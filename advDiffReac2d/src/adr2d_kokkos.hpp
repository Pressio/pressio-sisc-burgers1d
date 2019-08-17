
#ifndef ADR2D_KOKKOS_HPP_
#define ADR2D_KOKKOS_HPP_

#include "UTILS_ALL"
#include <Kokkos_Core.hpp>
#include <KokkosSparse_CrsMatrix.hpp>
#include "KokkosSparse_spmv.hpp"
#include "adr2d_velocity_functor.hpp"
#include "adr2d_jacobian_functor.hpp"

template<
  template<typename, typename> class source_functor,
  template<typename, typename> class advection_functor
  >
class Adr2dKokkos
{
  static constexpr int numSpecies_{3};

  using this_t	= Adr2dKokkos<source_functor, advection_functor>;
  using ui_t	= unsigned int;
  using sc_t	= double;
  // the type to represent global indices, for enumerating dofs and cells
  using gid_t   = ui_t;

  // static constants
  static constexpr auto zero	= ::pressio::utils::constants::zero<sc_t>();
  static constexpr auto one	= ::pressio::utils::constants::one<sc_t>();
  static constexpr auto two	= ::pressio::utils::constants::two<sc_t>();
  static constexpr auto three	= ::pressio::utils::constants::three<sc_t>();
  static constexpr auto four	= ::pressio::utils::constants::four<sc_t>();
  static constexpr auto oneHalf = one/two;

  // aliases for layouts and exe space
  using klr = Kokkos::LayoutRight;
  using kll = Kokkos::LayoutLeft;
  using exe_space = Kokkos::DefaultExecutionSpace;

  // --------------------------------------------------------------
  // aliases for 1d Kokkos view with left layout on device and host
  using k1d_ll_d_t = Kokkos::View<sc_t*, kll, exe_space>;
  using k1d_ll_h_t = k1d_ll_d_t::host_mirror_type;
  // host and device should have same layout
  using k1ll_h_layout = typename k1d_ll_h_t::traits::array_layout;
  using k1ll_d_layout = typename k1d_ll_d_t::traits::array_layout;
  static_assert( std::is_same<k1ll_h_layout, k1ll_d_layout>::value,
		 "Layout for d and h (mirrorw) 1d view is not the same");

  // --------------------------------------------------------------
  // aliases for 2d Kokkos view with left layout on device and host
  using k2d_ll_d_t = Kokkos::View<sc_t**, kll, exe_space>;
  using k2d_ll_h_t = k2d_ll_d_t::host_mirror_type;
  // host and device have same layout
  using k2ll_h_layout = typename k2d_ll_h_t::traits::array_layout;
  using k2ll_d_layout = typename k2d_ll_d_t::traits::array_layout;
  static_assert( std::is_same<k2ll_h_layout, k2ll_d_layout>::value,
		 "Layout for d and h (mirrorw) 2d view is not the same");

  // --------------------------------------------------------------
  // the cell connectivity types, for both host and device:
  // we know at compile time that the connectivity matrix has 5 columns
  using connectivity_d_t  = Kokkos::View<gid_t*[5], kll, exe_space>;
  using connectivity_h_t  = connectivity_d_t::host_mirror_type;

  // --------------------------------------------------------------
  // the coordinates type:
  // we know at compile time that coords matrix has 2 columns
  using coords_d_t  = Kokkos::View<sc_t*[2], kll, exe_space>;
  using coords_h_t  = coords_d_t::host_mirror_type;

public:
  using kcrs_mat_t		= KokkosSparse::CrsMatrix<sc_t, gid_t, exe_space>;
  using state_type_d_t		= k1d_ll_d_t;
  using state_type_h_t		= k1d_ll_h_t;
  using velocity_type_d_t	= state_type_d_t;
  using velocity_type_h_t	= state_type_h_t;
  using mv_d_t			= k2d_ll_d_t;
  using mv_h_t			= k2d_ll_h_t;

  using scalar_type	= sc_t;
  using state_type	= state_type_d_t;
  using velocity_type	= state_type_d_t;
  using jacobian_type   = kcrs_mat_t;
  using mvec_t		= mv_d_t;

  using source_functor_t    = source_functor<scalar_type, void>;
  using advection_functor_t = advection_functor<scalar_type, void>;

public:
  Adr2dKokkos() = delete;

  Adr2dKokkos(const std::string & meshFile,
	      source_functor_t & srcFnctIn,
  	      advection_functor_t & advFnctIn,
	      scalar_type diffusion)
    : meshFile_{meshFile},
      srcFnct_{srcFnctIn},
      advFnct_{advFnctIn},
      D_{diffusion},
      graph_h_{"graph_h", 1}, // we don't know size yet so set 1
      graph_d_{"graph_d", 1},
      coords_h_{"coords_h", 1},
      coords_d_{"coords_d", 1},
      state_{"state", 1}
  {
    this->readMesh();
    this->computeCoefficients();
  }

  ~Adr2dKokkos() = default;

public:
  size_t getStateSize() const{
    return numDof_;
  }

  const state_type & getState() const{
    return state_;
  }

  void velocity(const state_type  & u,
		const scalar_type & t,
		velocity_type	  & f) const{
    this->velocity_impl(u, t, f);
  }

  velocity_type velocity(const state_type  & u,
			 const scalar_type & t) const{
    velocity_type f("f", numDof_r_);
    this->velocity_impl(u, t, f);
    return f;
  }

  void jacobian(const state_type & u,
		const scalar_type t,
		jacobian_type & jac) const{
    this->jacobian_impl(u, t, jac);
  }

  jacobian_type jacobian(const state_type & u,
			 const scalar_type t) const
  {
    // this code basically creates the graph of the Jacobian only

    // TODO: make sure we do this only once,
    // if/when the jacobian needs to be recomputed,
    // use the method above that accepts a Jacobian as positional argument

    // each row of the jacobian has 7 non zeros
    constexpr int nonZerosPerRow = 7;

    const gid_t numRows = numDof_r_;
    const gid_t numCols = numDof_;
    const gid_t numEnt  = numRows * nonZerosPerRow;

    state_type_h_t u_h = Kokkos::create_mirror_view(u);

    // create data on device that we need to fill to create Jacobian
    typename jacobian_type::row_map_type::non_const_type ptr ("ptr", numRows+1);
    typename jacobian_type::index_type::non_const_type ind ("ind", numEnt);
    typename jacobian_type::values_type val ("val", numEnt);

    {
      // create host mirros of these
      typename jacobian_type::row_map_type::HostMirror ptr_h = Kokkos::create_mirror_view (ptr);
      typename jacobian_type::index_type::HostMirror   ind_h = Kokkos::create_mirror_view (ind);
      typename jacobian_type::values_type::HostMirror  val_h = Kokkos::create_mirror_view (val);

      // first, fill in how many elements per row
      ptr_h[0] = 0;
      for (gid_t iRow = 0; iRow < numRows; ++iRow) {
	ptr_h[iRow+1] = ptr_h[iRow] + nonZerosPerRow;
      }
      Kokkos::deep_copy(ptr, ptr_h);

      // set the column index for each entry of the Jacobian
      // we have 7 non zeros per row
      gid_t k = -1;
      for (gid_t rPt=0; rPt < numGpt_r_; ++rPt)
      {
	// gID of this cell
	const auto & cellGID_ = graph_h_(rPt,0);

	// given a cell, compute index in u of the first dof, i.e. c0
	const auto uIndex = cellGID_*numSpecies_;

	// the gids of the neighboring cells (we assume 2nd ordered)
	const auto & westCellGid  = graph_h_(rPt,1);
	const auto & northCellGid = graph_h_(rPt,2);
	const auto & eastCellGid  = graph_h_(rPt,3);
	const auto & southCellGid = graph_h_(rPt,4);

	// given this cell, find index of state for species 0 at WEST cell
	const auto uWestIndex  = westCellGid*numSpecies_;
	// given this cell, find index of state value at NORTH cell
	const auto uNorthIndex = northCellGid*numSpecies_;
	// given this cell, find index of state value at EAST cell
	const auto uEastIndex  = eastCellGid*numSpecies_;
	// given this cell, find index of state value at SOUTH cell
	const auto uSouthIndex = southCellGid*numSpecies_;

	// for every grid point, I have 3 degrees of freedom
	// deal with dof 0
	ind_h[++k] = uIndex;
	ind_h[++k] = uIndex+1;
	ind_h[++k] = uIndex+2;
	ind_h[++k] = uWestIndex;
	ind_h[++k] = uNorthIndex;
	ind_h[++k] = uEastIndex;
	ind_h[++k] = uSouthIndex;

	// deal with dof 1
	ind_h[++k] = uIndex+1;
	ind_h[++k] = uIndex;
	ind_h[++k] = uIndex+2;
	ind_h[++k] = uWestIndex+1;
	ind_h[++k] = uNorthIndex+1;
	ind_h[++k] = uEastIndex+1;
	ind_h[++k] = uSouthIndex+1;

	// deal with dof 2
	ind_h[++k] = uIndex+2;
	ind_h[++k] = uIndex;
	ind_h[++k] = uIndex+1;
	ind_h[++k] = uWestIndex+2;
	ind_h[++k] = uNorthIndex+2;
	ind_h[++k] = uEastIndex+2;
	ind_h[++k] = uSouthIndex+2;
      }
    }

    jacobian_type J("J", numRows, numCols, numEnt, val, ptr, ind);
    this->jacobian(u, t, J);
    return J;
  }


  void applyJacobian(const state_type & u,
  		     const mv_d_t & B,
  		     scalar_type t,
  		     mv_d_t & A) const
  {
    auto JJ = this->jacobian(u, t);
    constexpr auto zero = ::pressio::utils::constants::zero<sc_t>();
    constexpr auto one = ::pressio::utils::constants::one<sc_t>();
    const char ct = 'N';
    KokkosSparse::spmv(&ct, one, JJ, B, zero, A);
  }


  mv_d_t applyJacobian(const state_type & u,
  		     const mvec_t & B,
  		     scalar_type t) const
  {
    mv_d_t A("AA", numDof_r_, B.extent(1) );
    this->applyJacobian(u, B, t, A);
    return A;
  }

private:
  void readMesh()
  {
    std::ifstream foundFile(meshFile_);
    if(!foundFile){
      std::cout << "meshfile not found" << std::endl;
      exit(EXIT_FAILURE);
    }

    std::ifstream source;
    source.open( meshFile_, std::ios_base::in);
    std::string line;
    gid_t count=-1;
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

	  // since graph has fix num of cols, this only resizes rows
	  Kokkos::resize( graph_h_, numGpt_r_);
	  Kokkos::resize( graph_d_, numGpt_r_);
	}
	else if (colVal == "numStatePts"){
	  ss >> colVal;
	  numGpt_ = std::stoi(colVal);
	  numDof_   = numGpt_ * this_t::numSpecies_;
	  std::cout << "numGpt = " << numGpt_ << " "
		    << "numDof = " << numDof_ << std::endl;

	  Kokkos::resize( state_, numDof_);
	  // since coords has fix num of cols, this only resizes the rows
	  Kokkos::resize( coords_h_, numGpt_);
	  Kokkos::resize( coords_d_, numGpt_);
	}
	else{
	  ++count;
	  // store first value and its coords
	  auto thisGid = std::stoi(colVal);
	  graph_h_(count, 0) = thisGid;
	  ss >> colVal; coords_h_(thisGid, 0) = std::stod(colVal);
	  ss >> colVal; coords_h_(thisGid, 1) = std::stod(colVal);
	  // store others on same row
	  for (auto i=1; i<=4; ++i){
	    ss >> colVal; thisGid = std::stoi(colVal);
	    graph_h_(count, i) = thisGid;
	    ss >> colVal; coords_h_(thisGid,0) = std::stod(colVal);
	    ss >> colVal; coords_h_(thisGid,1) = std::stod(colVal);
	  }
	}
      }
    source.close();

    // for (auto i=0; i<numGpt_r_; ++i){
    //   for (auto j=0; j<5; ++j){
    // 	auto gid = graph_h_(i,j);
    //     std::cout << " " << graph_h_(i,j) << " " << coords_h_(gid, 0) << " ";
    //   }
    //   std::cout << std::endl;
    // }

    // we initialized the graph on the host, so deep copy to device
    Kokkos::deep_copy(graph_d_,  graph_h_);
    Kokkos::deep_copy(coords_d_, coords_h_);

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
		     velocity_type     & f) const
  {
    using func_t = VelocityFunctor<
      state_type, velocity_type, coords_d_t, connectivity_d_t,
      advection_functor_t, source_functor_t, scalar_type, numSpecies_>;

    func_t F(t, u, f, coords_d_, graph_d_, advFnct_, srcFnct_,
	     dxSqInv_, dySqInv_, dx2Inv_, dy2Inv_, DovDxSq_, DovDySq_, FDcoeff1_);

    // launch pfor as large as number of cells where we need to evaluate velocity
    Kokkos::parallel_for(numGpt_r_, F);
    exe_space().fence();
  }//end velocity_impl


  void jacobian_impl(const state_type  & u,
		     const scalar_type & t,
		     jacobian_type     & J) const
  {
    using policy_type = Kokkos::RangePolicy<exe_space, gid_t>;

    using func_t = JacobianFunctor<
      state_type, jacobian_type, coords_d_t, connectivity_d_t,
      advection_functor_t, source_functor_t, scalar_type, numSpecies_>;

    // construct functor
    func_t F(t, u, J, coords_d_, graph_d_, advFnct_, srcFnct_,
	     dxSqInv_, dySqInv_, dx2Inv_, dy2Inv_, DovDxSq_, DovDySq_, FDcoeff1_);

    // launch pfor as large as number of cells where we need to evaluate velocity
    Kokkos::parallel_for( policy_type(0, numGpt_r_), F);
    exe_space().fence();
  }//end jacobian_impl


private:
  // name of mesh file
  const std::string meshFile_ = {};

  // functor to evaluate source term
  source_functor_t & srcFnct_;
  // functor to evaluate advection field
  advection_functor_t & advFnct_;

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

  /*
    graph: contains a list such that
    1 0 3 2 -1

    first col: contains GIDs of cells where we want velocity
    1,2,3,4 col: contains GIDs of neighboring cells needed for stencil
  		 the order of the neighbors is: east, north, west, south
   */
  connectivity_h_t graph_h_ = {};
  connectivity_d_t graph_d_ = {};

  // note that dof refers to the degress of freedom,
  // which is NOT same as grid points. for this problem,
  // the dof = numSpecies_ * number_of_unknown_grid_points
  // _r_ stands for velocity
  gid_t numGpt_	  = {};
  gid_t numDof_	  = {};
  gid_t numGpt_r_ = {};
  gid_t numDof_r_ = {};

  // the coords of all cells centers
  coords_h_t coords_h_ = {};
  coords_d_t coords_d_ = {};

  // state_ has size = numGpt_ * numSpecies_
  // because we have multiple dofs per cell
  mutable state_type state_ = {};

};//end class

#endif
