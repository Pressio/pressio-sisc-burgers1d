
#ifndef ADR2D_EIGEN_HPP_
#define ADR2D_EIGEN_HPP_

#include "UTILS_ALL"
#include "Eigen/Dense"
#include "Eigen/SparseCore"
#include <fstream>
#include <sstream>
#include <vector>

template<typename source_functor, typename advection_functor>
class Adr2dEigen
{
  using this_t	  = Adr2dEigen<source_functor, advection_functor>;
  using sc_t	  = double;
  using eigVec	  = Eigen::VectorXd;
  using Tr	  = Eigen::Triplet<sc_t>;
  using sc_arr3_t = std::array<sc_t, 3>;

  static constexpr auto zero	= ::pressio::utils::constants::zero<sc_t>();
  static constexpr auto one	= ::pressio::utils::constants::one<sc_t>();
  static constexpr auto two	= ::pressio::utils::constants::two<sc_t>();
  static constexpr auto oneHalf = one/two;

public:

  // the layout to use for SparseMatrix,
  // if using RowMajor, we can benefit for OpenMP quoting from website:
  // ***
  // Currently, the following algorithms can make use of multi-threading:
  // general dense matrix - matrix products
  // row-major-sparse * dense vector/matrix products
  // ConjugateGradient with Lower|Upper as the UpLo template parameter.
  // BiCGSTAB with a row-major sparse matrix format.
  // LeastSquaresConjugateGradient
  // **
  static constexpr auto spmat_layout = Eigen::RowMajor;

  // type to use for all indexing, has to be large enough
  // to support indexing fairly large systems
  using index_t  = int32_t;

  // type for the adjacency list for each mesh graph
  // we use an array of 5 indices since all graph nodes have
  // the same number of connections
  using node_al_t = std::array<index_t,5>;

  // number of dofs for each cell
  static constexpr index_t numSpecies_{3};

  // number of non zero entries for each jacobian row
  static constexpr int32_t nonZerosPerRowJ_ = 7;

  using scalar_type	= sc_t;
  using state_type	= eigVec;
  using velocity_type	= eigVec;
  // Eigen SparseMatrix has to have a signed integer type, use index_t
  using jacobian_type	= Eigen::SparseMatrix<sc_t, spmat_layout, index_t>;

  // for some reason, the best outcome is when the sparse is row-major
  // and the dense matrix is colmajor
  using dmatrix_type	= Eigen::Matrix<sc_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>;

  // type to represent connectivity
  using mesh_graph_t	= std::vector<node_al_t>;

public:
  Adr2dEigen() = delete;

  Adr2dEigen(const std::string & meshFile,
	     const source_functor & srcFnctIn,
	     const advection_functor & advFnctIn,
	     scalar_type diffusion)
    : meshFile_{meshFile},
      srcFnct_{srcFnctIn},
      advFnct_{advFnctIn},
      D_{diffusion}
  {
    this->readMesh();
    /* the jacobian has
     * num of rows = number of residuals dofs
     * num of cols = number of state dofs
     */
    J_.resize(numDof_r_, numDof_);
    tripletList.resize(nonZerosPerRowJ_*numDof_r_);

    // compute and store all coefficients for stencils
    this->computeCoefficients();
  }

  ~Adr2dEigen() = default;

public:
  size_t getStateSize() const{ return numDof_; }
  const state_type & getState() const{ return state_; }
  eigVec getX() const { return x_; }
  eigVec getY() const { return y_; }

  const mesh_graph_t & viewGraph() const{
    return graph_;
  }

  void velocity(const state_type  & u,
		const scalar_type & t,
		velocity_type	  & f) const{
    this->velocity_impl(u, t, f);
  }

  velocity_type velocity(const state_type  & u,
			 const scalar_type & t) const{
    // use member f_ here
    this->velocity_impl(u, t, f_);
    return f_;
  }

  void jacobian(const state_type  & u,
  		const scalar_type t,
		jacobian_type	  & jac) const{
    this->jacobian_impl(u, jac, t);
  }

  jacobian_type jacobian(const state_type  & u,
  			 const scalar_type   t) const{
    /* the jacobian has
     * num of rows = number of residuals dofs
     * num of cols = number of state dofs
     */
    // use member J_ here
    this->jacobian_impl(u, J_, t);
    return J_;
  }

  void applyJacobian(const state_type & u,
  		     const dmatrix_type & B,
  		     const scalar_type & t,
  		     dmatrix_type & A) const{
    this->jacobian_impl(u, J_, t);
    A = J_ * B;
  }

  dmatrix_type applyJacobian(const state_type & u,
  		     const dmatrix_type & B,
  		     const scalar_type & t) const{
    dmatrix_type A( numDof_r_, B.cols() );
    this->applyJacobian(u, B, t, A);
    return A;
  }

private:
  void readMesh(){
    typename Adr2dEigen::node_al_t lineGIDs;

    std::ifstream foundFile(meshFile_);
    if(!foundFile){
      std::cout << "meshfile not found" << std::endl;
      exit(EXIT_FAILURE);
    }

    std::ifstream source;
    source.open( meshFile_, std::ios_base::in);
    std::string line;
    while (std::getline(source, line) )
      {
	std::istringstream ss(line);
	std::string colVal;
	// get first column
	ss >> colVal;

	if (colVal == "dx"){
	  ss >> colVal;
	  dx_ = std::stod(colVal);
	  std::cout << "dx = " << dx_ << std::endl;
	}
	else if (colVal == "dy"){
	  ss >> colVal;
	  dy_ = std::stod(colVal);
	  std::cout << "dy = " << dy_ << std::endl;
	}
	else if (colVal == "numResidualPts"){
	  ss >> colVal;
	  numGpt_r_ = std::stoi(colVal);
	  numDof_r_ = numGpt_r_ * this_t::numSpecies_;
	  f_.resize(numDof_r_);
	  std::cout << "numGpt_r = " << numGpt_r_ << std::endl;
	  std::cout << "numDof_r = " << numDof_r_ << std::endl;
	}
	else if (colVal == "numStatePts"){
	  ss >> colVal;
	  numGpt_ = std::stoi(colVal);
	  numDof_   = numGpt_ * this_t::numSpecies_;
	  state_.resize(numDof_);
	  x_.resize(numGpt_);
	  y_.resize(numGpt_);
	  std::cout << "numGpt = " << numGpt_ << std::endl;
	  std::cout << "numDof = " << numDof_ << std::endl;
	}
	else{
	  // store first value and its coords
	  auto thisGid = std::stoi(colVal);
	  lineGIDs[0] = thisGid;
	  ss >> colVal; x_[thisGid] = std::stod(colVal);
	  ss >> colVal; y_[thisGid] = std::stod(colVal);

	  // store others on same row
	  for (auto i=1; i<=4; ++i){
	    ss >> colVal; thisGid = std::stoi(colVal);
	    lineGIDs[i] = thisGid;
	    ss >> colVal; x_[thisGid] = std::stod(colVal);
	    ss >> colVal; y_[thisGid] = std::stod(colVal);
	  }
	  graph_.emplace_back(lineGIDs);
	}
      }
    source.close();

    // // loop over cells where velocity needs to be computed
    // // i.e. over all cells where we want residual
    // for (index_t rPt=0; rPt < graph_.size(); ++rPt)
    // {
    //   // gID of this cell
    //   const auto & cellGID = graph_[rPt][0];

    //   // local coordinates
    //   const auto & thisCellX = x_[cellGID];
    //   const auto & thisCellY = y_[cellGID];

    //   // store the state at this cell
    //   cellState_[0] = 0.;
    //   cellState_[1] = 0.;
    //   cellState_[2] = 0.;

    //   advFnct_(thisCellX, thisCellY, 0., cellState_, cellAdv_);
    //   vx_[cellGID] = cellAdv_[0];
    //   vy_[cellGID] = cellAdv_[1];
    // }

    // for (auto & it : graph_){
    //   for (auto & it2 : it){
    //     std::cout << " " << it2;
    //   }
    //   std::cout << std::endl;
    // }
  }//end readMesh

  void computeCoefficients(){
    dxSqInv_ = this_t::one/(dx_*dx_);
    dySqInv_ = this_t::one/(dy_*dy_);
    dx2Inv_  = this_t::one/(this_t::two*dx_);
    dy2Inv_  = this_t::one/(this_t::two*dy_);
    DovDxSq_ = D_*dxSqInv_;
    DovDySq_ = D_*dySqInv_;
    FDcoeff1_ = -this_t::two*(DovDxSq_ + DovDySq_);
  }

  void velocity_impl(const state_type  & u,
		     const scalar_type & t,
		     velocity_type     & f) const{
    // remember that the size of the state vector u is not necessarily == f
    // f can be smaller than u when involving sample mesh

    // set f to zero
    f.setZero();

    /* the graph contains the connectivity for all the cells where I need
     * to copute the velocity. I can loop over it.
     */
    for (index_t vPt=0; vPt < numGpt_r_; ++vPt)
    {
      // ajacency list of this cell
      const auto & thisCellAdList = graph_[vPt];

      // gID of this cell
      const auto & cellGID = thisCellAdList[0];

      // local coordinates
      const auto & thisCellX = x_[cellGID];
      const auto & thisCellY = y_[cellGID];

      // given a cell, compute its index in f of the first dof, i.e. c0,
      // belonging to this cell
      const auto fIndex = vPt*numSpecies_;

      // given a cell, compute index in u of the first dof, i.e. c0
      const auto uIndex = cellGID*numSpecies_;

      // store the state at this cell
      cellState_[0] = u[uIndex];
      cellState_[1] = u[uIndex+1];
      cellState_[2] = u[uIndex+2];

      // compute local advection
      advFnct_(thisCellX, thisCellY, t, cellState_, cellAdv_);
      // compute local source
      srcFnct_(thisCellX, thisCellY, t, cellState_, cellSrc_);

      // compute the FD coefficients for this cell
      FDcoeffWest_  =  cellAdv_[0]*dx2Inv_ + DovDxSq_;
      FDcoeffNorth_ = -cellAdv_[1]*dy2Inv_ + DovDySq_;
      FDcoeffEast_  = -cellAdv_[0]*dx2Inv_ + DovDxSq_;
      FDcoeffSouth_ =  cellAdv_[1]*dy2Inv_ + DovDySq_;

      // the gids of the neighboring cells (we assume 2nd ordered)
      const auto & westCellGid  = thisCellAdList[1];
      const auto & northCellGid = thisCellAdList[2];
      const auto & eastCellGid  = thisCellAdList[3];
      const auto & southCellGid = thisCellAdList[4];

      // loop manually unrolled for species = 3
      // store the velocity
      f[fIndex]   = FDcoeff1_ * cellState_[0];
      f[fIndex+1] = FDcoeff1_ * cellState_[1];
      f[fIndex+2] = FDcoeff1_ * cellState_[2];
      // given this cell, find index of state for species 0 at WEST cell
      uWestIndex_	 = westCellGid*numSpecies_;
      // given this cell, find index of state value at NORTH cell
      uNorthIndex_	= northCellGid*numSpecies_;
      // given this cell, find index of state value at EAST cell
      uEastIndex_	 = eastCellGid*numSpecies_;
      // given this cell, find index of state value at SOUTH cell
      uSouthIndex_	= southCellGid*numSpecies_;
      // compute velocity
      f[fIndex] += FDcoeffWest_*u[uWestIndex_] + FDcoeffNorth_*u[uNorthIndex_]
      	        + FDcoeffEast_*u[uEastIndex_] + FDcoeffSouth_*u[uSouthIndex_]
      		+ cellSrc_[0];

      f[fIndex+1] += FDcoeffWest_*u[uWestIndex_+1] + FDcoeffNorth_*u[uNorthIndex_+1]
      		  + FDcoeffEast_*u[uEastIndex_+1] + FDcoeffSouth_*u[uSouthIndex_+1]
      		  + cellSrc_[1];

      f[fIndex+2] += FDcoeffWest_*u[uWestIndex_+2] + FDcoeffNorth_*u[uNorthIndex_+2]
      		  + FDcoeffEast_*u[uEastIndex_+2] + FDcoeffSouth_*u[uSouthIndex_+2]
      		  + cellSrc_[2];

    }// end vPt loop
  }//end velocity_impl


  void jacobian_impl(const state_type	& u,
  		     jacobian_type	& J,
		     const scalar_type	& t) const
  {
    J.setZero();

    // triplets is used to store a series of (row, col, value)
    // loop over cells where velocity needs to be computed
    // i.e. over all cells where we want residual
    index_t tripCount = -1;
    for (index_t vPt=0; vPt < graph_.size(); ++vPt)
    {
      // ajacency list of this cell
      const auto & thisCellAdList = graph_[vPt];

      // gID of this cell
      const auto & cellGID = thisCellAdList[0];

      // local coordinates
      const auto & thisCellX = x_[cellGID];
      const auto & thisCellY = y_[cellGID];

      // given a cell, compute row index of the first dof, i.e. c0
      const auto rowIndex = vPt*numSpecies_;

      // given a cell, compute index in u of the first dof, i.e. c0
      const auto uIndex = cellGID*numSpecies_;

      // store the state at this cell
      // store the state at this cell
      cellState_[0] = u[uIndex];
      cellState_[1] = u[uIndex+1];
      cellState_[2] = u[uIndex+2];

      // compute local advection
      advFnct_(thisCellX, thisCellY, t, cellState_, cellAdv_);
      // compute Jacobian of local source
      srcFnct_(thisCellX, thisCellY, t, cellState_, cellSrcJ_);

      // compute the FD coefficients for this cell
      FDcoeffWest_  =  cellAdv_[0]*dx2Inv_ + DovDxSq_;
      FDcoeffNorth_ = -cellAdv_[1]*dy2Inv_ + DovDySq_;
      FDcoeffEast_  = -cellAdv_[0]*dx2Inv_ + DovDxSq_;
      FDcoeffSouth_ =  cellAdv_[1]*dy2Inv_ + DovDySq_;

      // the gids of the neighboring cells
      const auto & westCellGid  = thisCellAdList[1];
      const auto & northCellGid = thisCellAdList[2];
      const auto & eastCellGid  = thisCellAdList[3];
      const auto & southCellGid = thisCellAdList[4];

      // contribution for ij
      tripletList[++tripCount] = Tr(rowIndex,   uIndex,   FDcoeff1_ + cellSrcJ_[0][0]);
      tripletList[++tripCount] = Tr(rowIndex+1, uIndex+1, FDcoeff1_ + cellSrcJ_[1][1]);
      tripletList[++tripCount] = Tr(rowIndex+2, uIndex+2, FDcoeff1_ + cellSrcJ_[2][2]);

      // deal with mixed terms, all involving state in this cell
      // dof 0
      tripletList[++tripCount] = Tr(rowIndex, uIndex+1, cellSrcJ_[0][1]);
      tripletList[++tripCount] = Tr(rowIndex, uIndex+2, cellSrcJ_[0][2]);
      // dof 1
      tripletList[++tripCount] = Tr(rowIndex+1, uIndex,   cellSrcJ_[1][0]);
      tripletList[++tripCount] = Tr(rowIndex+1, uIndex+2, cellSrcJ_[1][2]);
      // dof 2
      tripletList[++tripCount] = Tr(rowIndex+2, uIndex,   cellSrcJ_[2][0]);
      tripletList[++tripCount] = Tr(rowIndex+2, uIndex+1, cellSrcJ_[2][1]);

      // given this cell and DOF, find index of state value at WEST cell
      const auto uWestIndex  = westCellGid*numSpecies_;
      tripletList[++tripCount] = Tr(rowIndex,   uWestIndex,   FDcoeffWest_);
      tripletList[++tripCount] = Tr(rowIndex+1, uWestIndex+1, FDcoeffWest_);
      tripletList[++tripCount] = Tr(rowIndex+2, uWestIndex+2, FDcoeffWest_);

      // given this cell and DOF, find index of state value at NORTH cell
      const auto uNorthIndex = northCellGid*numSpecies_;
      tripletList[++tripCount] = Tr(rowIndex,   uNorthIndex,   FDcoeffNorth_);
      tripletList[++tripCount] = Tr(rowIndex+1, uNorthIndex+1, FDcoeffNorth_);
      tripletList[++tripCount] = Tr(rowIndex+2, uNorthIndex+2, FDcoeffNorth_);

      // given this cell and DOF, find index of state value at EAST cell
      const auto uEastIndex  = eastCellGid*numSpecies_;
      tripletList[++tripCount] = Tr(rowIndex,   uEastIndex,	  FDcoeffEast_);
      tripletList[++tripCount] = Tr(rowIndex+1, uEastIndex+1, FDcoeffEast_);
      tripletList[++tripCount] = Tr(rowIndex+2, uEastIndex+2, FDcoeffEast_);

      // given this cell and DOF, find index of state value at SOUTH cell
      const auto uSouthIndex = southCellGid*numSpecies_;
      tripletList[++tripCount] = Tr(rowIndex,   uSouthIndex,   FDcoeffSouth_);
      tripletList[++tripCount] = Tr(rowIndex+1, uSouthIndex+1, FDcoeffSouth_);
      tripletList[++tripCount] = Tr(rowIndex+2, uSouthIndex+2, FDcoeffSouth_);
    }// end vPt loop

    J.setFromTriplets(tripletList.begin(), tripletList.end());
    if ( !J.isCompressed() )
      J.makeCompressed();

  }// end jacobian_impl


private:

  // name of mesh file
  const std::string meshFile_ = {};

  // functor to evaluate source term
  const source_functor & srcFnct_;
  // functor to evaluate advection field
  const advection_functor & advFnct_;

  // diffusion
  const scalar_type D_ = this_t::zero;

  scalar_type dx_{};
  scalar_type dy_{};

  // 1/dx^2 and 1/dy^2
  scalar_type dxSqInv_{};
  scalar_type dySqInv_{};

  //1/(2 dx) and 1/(2 dy)
  scalar_type dx2Inv_{};
  scalar_type dy2Inv_{};

  // D/dx^2 and D/dy^2
  scalar_type DovDxSq_{};
  scalar_type DovDySq_{};

  // -2.0 * (D/dx^2 + D/dy^2)
  scalar_type FDcoeff1_{};

  // uij/2dx + D/dx^2
  mutable scalar_type FDcoeffWest_{};
  // -vij/2dy + D/dy^2
  mutable scalar_type FDcoeffNorth_{};
  // -uij/2dx + D/dx^2
  mutable scalar_type FDcoeffEast_{};
  // vij/2dy + D/dy^2
  mutable scalar_type FDcoeffSouth_{};

  /*
    graph: contains a list such that
    1 0 3 2 -1

    first col   : contains GIDs of cells where we want velocity
    col 1,2,3,4 : contains GIDs of neighboring cells needed for stencil
		  the order of the neighbors is: east, north, west, south
   */
  mesh_graph_t graph_ = {};

  mutable index_t uWestIndex_  = {};
  mutable index_t uNorthIndex_ = {};
  mutable index_t uEastIndex_  = {};
  mutable index_t uSouthIndex_ = {};

  // note that dof refers to the degress of freedom,
  // which is NOT same as grid points. for this problem,
  // the dof = numSpecies_ * number_of_unknown_grid_points
  // _r_ stands for velocity
  index_t numGpt_	  = {};
  index_t numDof_	  = {};
  index_t numGpt_r_ = {};
  index_t numDof_r_ = {};

  // x,y define the coords of all cells centers
  eigVec x_ = {};
  eigVec y_ = {};

  // triplet list to compute Jacobian
  mutable std::vector<Tr> tripletList = {};

  // state_ has size = numGpt_ * numSpecies_
  mutable state_type state_ = {};

  // f_ has size = numGpt_r_ * numSpecies_
  // because it might be evaluated at a subset of cells
  mutable velocity_type f_ {};

  // J_ has numDof_r_ rows and numDof_ cols
  mutable jacobian_type J_ {};

  // advection components at a given cell
  mutable std::array<scalar_type, 2> cellAdv_ = {};

  // source computed at a given cell
  mutable sc_arr3_t cellSrc_ = {};

  // jacobian of the source at a given cell
  mutable std::array<sc_arr3_t, numSpecies_> cellSrcJ_ = {};

  // the state at a given cell
  mutable std::array<scalar_type, numSpecies_> cellState_ = {};

};//end class

#endif
