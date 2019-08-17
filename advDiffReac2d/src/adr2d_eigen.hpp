
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
  using this_t	= Adr2dEigen<source_functor, advection_functor>;
  using ui_t	= unsigned int;
  using sc_t	= double;
  using eigVec	= Eigen::VectorXd;
  using mv_t	= Eigen::MatrixXd;
  using Tr	= Eigen::Triplet<sc_t>;
  using sc_arr3_t = std::array<sc_t, 3>;

  // the type to represent global indices
  using gid_t   = ui_t;
  // array of 5 gids
  using gid_arr5_t = std::array<gid_t,5>;

public:
  using scalar_type	= sc_t;
  using state_type	= eigVec;
  using velocity_type	= eigVec;
  using jacobian_type	= Eigen::SparseMatrix<sc_t, Eigen::RowMajor, int32_t>;

  // type to represent connectivity
  using connectivity_t	= std::vector<gid_arr5_t>;

  static constexpr auto zero	= ::pressio::utils::constants::zero<scalar_type>();
  static constexpr auto one	= ::pressio::utils::constants::one<scalar_type>();
  static constexpr auto two	= ::pressio::utils::constants::two<scalar_type>();
  static constexpr auto three	= ::pressio::utils::constants::three<scalar_type>();
  static constexpr auto four	= ::pressio::utils::constants::four<scalar_type>();
  static constexpr auto oneHalf = one/two;

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
    this->computeCoefficients();
  }

  ~Adr2dEigen() = default;

public:
  size_t getStateSize() const{
    return numDof_;
  }

  const state_type & getState() const{
    return state_;
  }

  eigVec getX() const {
    return x_;
  }

  eigVec getY() const {
    return y_;
  }

  void velocity(const state_type  & u,
		const scalar_type & t,
		velocity_type	  & f) const{
    this->velocity_impl(u, t, f);
  }

  velocity_type velocity(const state_type  & u,
			 const scalar_type & t) const{
    velocity_type f(numDof_r_);
    this->velocity_impl(u, t, f);
    return f;
  }

  void jacobian(const state_type  & u,
  		jacobian_type	  & jac,
  		const scalar_type t) const{
    this->jacobian_impl(u, jac, t);
  }

  jacobian_type jacobian(const state_type  & u,
  			 const scalar_type   t) const{
    /* the jacobian has
     * num of rows = number of residuals dofs
     * num of cols = number of state dofs
     */
    jacobian_type J(numDof_r_, numDof_);
    this->jacobian_impl(u, J, t);
    return J;
  }

  void applyJacobian(const state_type & y,
  		     const mv_t & B,
  		     const scalar_type & t,
  		     mv_t & A) const{
    /* the jacobian has
     * num of rows = number of residuals dofs
     * num of cols = number of state dofs
     */
    jacobian_type J(numDof_r_, numDof_);
    this->jacobian_impl(y, J, t);
    A = J * B;
  }

  mv_t applyJacobian(const state_type & y,
  		     const mv_t & B,
  		     const scalar_type & t) const{
    mv_t A( numDof_r_, B.cols() );
    this->applyJacobian(y, B, t, A);
    return A;
  }

private:
  void readMesh(){
    typename Adr2dEigen::gid_arr5_t lineGIDs;

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
	  std::cout << "numGpt_r = " << numGpt_r_ << " "
		    << "numDof_r = " << numDof_r_ << std::endl;
	}
	else if (colVal == "numStatePts"){
	  ss >> colVal;
	  numGpt_ = std::stoi(colVal);
	  numDof_   = numGpt_ * this_t::numSpecies_;
	  std::cout << "numGpt = " << numGpt_ << " "
		    << "numDof = " << numDof_ << std::endl;

	  x_.resize(numGpt_);
	  y_.resize(numGpt_);
	  state_.resize(numDof_);
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
    // for (gid_t rPt=0; rPt < graph_.size(); ++rPt)
    // {
    //   // gID of this cell
    //   const auto & cellGID_ = graph_[rPt][0];

    //   // local coordinates
    //   const auto & thisCellX = x_[cellGID_];
    //   const auto & thisCellY = y_[cellGID_];

    //   // store the state at this cell
    //   cellState_[0] = 0.;
    //   cellState_[1] = 0.;
    //   cellState_[2] = 0.;

    //   advFnct_(thisCellX, thisCellY, 0., cellState_, cellAdv_);
    //   vx_[cellGID_] = cellAdv_[0];
    //   vy_[cellGID_] = cellAdv_[1];
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
    // remember that the size of u not necessarily == f
    // f can be smaller than u when involving sample mesh

    // set to zero
    f.setZero();

    // loop over cells where velocity needs to be computed
    // i.e. over all cells where we want residual
    for (gid_t rPt=0; rPt < numGpt_r_; ++rPt)
    {
      // gID of this cell
      const auto & cellGID_ = graph_[rPt][0];

      // local coordinates
      const auto & thisCellX = x_[cellGID_];
      const auto & thisCellY = y_[cellGID_];

      // given a cell, compute index in f of the first dof, i.e. c0
      const auto fIndex = rPt*numSpecies_;

      // given a cell, compute index in u of the first dof, i.e. c0
      const auto uIndex = cellGID_*numSpecies_;

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
      const auto & westCellGid  = graph_[rPt][1];
      const auto & northCellGid = graph_[rPt][2];
      const auto & eastCellGid  = graph_[rPt][3];
      const auto & southCellGid = graph_[rPt][4];

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

    }// end rPt loop
  }//end velocity_impl


  void jacobian_impl(const state_type	& u,
  		     jacobian_type	& J,
		     const scalar_type	& t) const
  {
    // triplets is used to store a series of (row, col, value)
    // has to be cleared becuase we append to it while computing
    tripletList.clear();

    // loop over cells where velocity needs to be computed
    // i.e. over all cells where we want residual
    for (gid_t rPt=0; rPt < graph_.size(); ++rPt)
    {
      // gID of this cell
      const auto & cellGID_ = graph_[rPt][0];

      // local coordinates
      const auto & thisCellX = x_[cellGID_];
      const auto & thisCellY = y_[cellGID_];

      // given a cell, compute row index of the first dof, i.e. c0
      const auto rowIndex = rPt*numSpecies_;

      // given a cell, compute index in u of the first dof, i.e. c0
      const auto uIndex = cellGID_*numSpecies_;

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
      const auto & westCellGid  = graph_[rPt][1];
      const auto & northCellGid = graph_[rPt][2];
      const auto & eastCellGid  = graph_[rPt][3];
      const auto & southCellGid = graph_[rPt][4];

      // contribution for ij
      tripletList.push_back( Tr(rowIndex,   uIndex,   FDcoeff1_ + cellSrcJ_[0][0]) );
      tripletList.push_back( Tr(rowIndex+1, uIndex+1, FDcoeff1_ + cellSrcJ_[1][1]) );
      tripletList.push_back( Tr(rowIndex+2, uIndex+2, FDcoeff1_ + cellSrcJ_[2][2]) );

      // deal with mixed terms, all involving state in this cell
      // dof 0
      tripletList.push_back( Tr(rowIndex, uIndex+1, cellSrcJ_[0][1]) );
      tripletList.push_back( Tr(rowIndex, uIndex+2, cellSrcJ_[0][2]) );
      // dof 1
      tripletList.push_back( Tr(rowIndex+1, uIndex,   cellSrcJ_[1][0]) );
      tripletList.push_back( Tr(rowIndex+1, uIndex+2, cellSrcJ_[1][2]) );
      // dof 2
      tripletList.push_back( Tr(rowIndex+2, uIndex,   cellSrcJ_[2][0]) );
      tripletList.push_back( Tr(rowIndex+2, uIndex+1, cellSrcJ_[2][1]) );

      // given this cell and DOF, find index of state value at WEST cell
      const auto uWestIndex  = westCellGid*numSpecies_;
      tripletList.push_back( Tr(rowIndex,   uWestIndex,   FDcoeffWest_) );
      tripletList.push_back( Tr(rowIndex+1, uWestIndex+1, FDcoeffWest_) );
      tripletList.push_back( Tr(rowIndex+2, uWestIndex+2, FDcoeffWest_) );

      // given this cell and DOF, find index of state value at NORTH cell
      const auto uNorthIndex = northCellGid*numSpecies_;
      tripletList.push_back( Tr(rowIndex,   uNorthIndex,   FDcoeffNorth_) );
      tripletList.push_back( Tr(rowIndex+1, uNorthIndex+1, FDcoeffNorth_) );
      tripletList.push_back( Tr(rowIndex+2, uNorthIndex+2, FDcoeffNorth_) );

      // given this cell and DOF, find index of state value at EAST cell
      const auto uEastIndex  = eastCellGid*numSpecies_;
      tripletList.push_back( Tr(rowIndex,   uEastIndex,	  FDcoeffEast_) );
      tripletList.push_back( Tr(rowIndex+1, uEastIndex+1, FDcoeffEast_) );
      tripletList.push_back( Tr(rowIndex+2, uEastIndex+2, FDcoeffEast_) );

      // given this cell and DOF, find index of state value at SOUTH cell
      const auto uSouthIndex = southCellGid*numSpecies_;
      tripletList.push_back( Tr(rowIndex,   uSouthIndex,   FDcoeffSouth_) );
      tripletList.push_back( Tr(rowIndex+1, uSouthIndex+1, FDcoeffSouth_) );
      tripletList.push_back( Tr(rowIndex+2, uSouthIndex+2, FDcoeffSouth_) );
    }// end rPt loop

    J.setFromTriplets(tripletList.begin(), tripletList.end());

  }// end jacobian_impl


private:
  static constexpr int numSpecies_{3};

  // name of mesh file
  const std::string meshFile_ = {};

  // functor to evaluate source term
  const source_functor & srcFnct_;
  // functor to evaluate advection field
  const advection_functor & advFnct_;

  // diffusion
  const scalar_type D_ = this_t::zero;

  const scalar_type Lx_{1.};
  const scalar_type Ly_{1.};
  const std::array<scalar_type,2> xAxisLim_{{0., Lx_}};
  const std::array<scalar_type,2> yAxisLim_{{0., Ly_}};

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

    first col: contains GIDs of cells where we want velocity
    1,2,3,4 col: contains GIDs of neighboring cells needed for stencil
		 the order of the neighbors is: east, north, west, south
   */
  connectivity_t graph_ = {};

  mutable gid_t uWestIndex_  = {};
  mutable gid_t uNorthIndex_ = {};
  mutable gid_t uEastIndex_  = {};
  mutable gid_t uSouthIndex_ = {};

  // note that dof refers to the degress of freedom,
  // which is NOT same as grid points. for this problem,
  // the dof = numSpecies_ * number_of_unknown_grid_points
  // _r_ stands for velocity
  gid_t numGpt_	  = {};
  gid_t numDof_	  = {};
  gid_t numGpt_r_ = {};
  gid_t numDof_r_ = {};

  // x,y define the coords of all cells centers
  eigVec x_ = {};
  eigVec y_ = {};

  // triplet list to compute Jacobian
  mutable std::vector<Tr> tripletList = {};

  // state_ has size = numGpt_ * numSpecies_
  // because we have multiple dofs per cell
  mutable state_type state_ = {};

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
