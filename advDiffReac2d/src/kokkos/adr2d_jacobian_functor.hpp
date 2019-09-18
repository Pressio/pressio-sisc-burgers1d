
#ifndef ADR2D_JACOBIAN_FUNCTOR_HPP_
#define ADR2D_JACOBIAN_FUNCTOR_HPP_

#include "MPL_ALL"
#include "UTILS_ALL"
#include <Kokkos_Core.hpp>

template <
  typename state_t,
  typename jacobian_t,
  typename coords_t,
  typename graph_t,
  typename advection_functor,
  typename source_functor,
  typename sc_t,
  int numSpecies
  >
struct JacobianFunctor{

  using ord_t = typename jacobian_t::ordinal_type;

  sc_t t_			 = {};
  state_t u_			 = {};
  jacobian_t J_			 = {};
  coords_t coords_		 = {};
  graph_t graph_		 = {};
  advection_functor advFunctor_  = {};
  source_functor srcFunctor_	 = {};
  sc_t dxSqInv_{}; //1/dx2
  sc_t dySqInv_{};
  sc_t dx2Inv_{}; // 1/(2dx)
  sc_t dy2Inv_{};
  sc_t DovDxSq_{}; // D/dx^2
  sc_t DovDySq_{};
  sc_t FDcoeff1_ = {};

  JacobianFunctor(sc_t t,
		  state_t u,
		  jacobian_t J,
		  coords_t coords,
		  graph_t graph,
		  advection_functor advFunctor,
		  source_functor srcFunctor,
		  sc_t dxSqInv, sc_t dySqInv,
		  sc_t dx2Inv,  sc_t dy2Inv,
		  sc_t DovDxSq, sc_t DovDySq,
		  sc_t FDcoeff1)
    : t_{t},
      u_{u},
      J_{J},
      coords_{coords},
      graph_{graph},
      advFunctor_{advFunctor},
      srcFunctor_{srcFunctor},
      dxSqInv_{dxSqInv}, dySqInv_{dySqInv},
      dx2Inv_{dx2Inv}, dy2Inv_{dy2Inv},
      DovDxSq_{DovDxSq}, DovDySq_{DovDySq},
      FDcoeff1_{FDcoeff1}
  {}

  KOKKOS_INLINE_FUNCTION
  void operator() (const int & rPt) const
  {
    // gID of this cell
    const auto & cellGID_ = graph_(rPt,0);

    // local coordinates
    const auto & thisCellX = coords_(cellGID_, 0);
    const auto & thisCellY = coords_(cellGID_, 1);

    // given a cell, compute row index of the first dof, i.e. c0
    const auto rowIndex = rPt*numSpecies;

    // given a cell, compute index in u of the first dof, i.e. c0
    const auto uIndex = cellGID_*numSpecies;

    // store the state at this cell
    const auto & u0 = u_[uIndex];
    const auto & u1 = u_[uIndex+1];
    const auto & u2 = u_[uIndex+2];

    // compute local advection
    sc_t cellVx_    = {};
    sc_t cellVy_    = {};
    advFunctor_(thisCellX, thisCellY, t_, u0, u1, u2, cellVx_, cellVy_);
    // compute local source
    sc_t cellSrc00_  = {}; sc_t cellSrc01_  = {};  sc_t cellSrc02_  = {};
    sc_t cellSrc10_  = {}; sc_t cellSrc11_  = {};  sc_t cellSrc12_  = {};
    sc_t cellSrc20_  = {}; sc_t cellSrc21_  = {};  sc_t cellSrc22_  = {};
    srcFunctor_(thisCellX, thisCellY, t_, u0, u1, u2,
    		cellSrc00_, cellSrc01_, cellSrc02_,
    		cellSrc10_, cellSrc11_, cellSrc12_,
    		cellSrc20_, cellSrc21_, cellSrc22_);

    // compute the FD coefficients for this cell
    const auto FDcoeffWest_  =  cellVx_*dx2Inv_ + DovDxSq_;
    const auto FDcoeffNorth_ = -cellVy_*dy2Inv_ + DovDySq_;
    const auto FDcoeffEast_  = -cellVx_*dx2Inv_ + DovDxSq_;
    const auto FDcoeffSouth_ =  cellVy_*dy2Inv_ + DovDySq_;

    // the gids of the neighboring cells (we assume 2nd ordered)
    const auto & westCellGid  = graph_(rPt,1);
    const auto & northCellGid = graph_(rPt,2);
    const auto & eastCellGid  = graph_(rPt,3);
    const auto & southCellGid = graph_(rPt,4);

    // given this cell, find index of state for species 0 at WEST cell
    const auto uWestIndex  = westCellGid*numSpecies;
    // given this cell, find index of state value at NORTH cell
    const auto uNorthIndex = northCellGid*numSpecies;
    // given this cell, find index of state value at EAST cell
    const auto uEastIndex  = eastCellGid*numSpecies;
    // given this cell, find index of state value at SOUTH cell
    const auto uSouthIndex = southCellGid*numSpecies;

    constexpr int ncol = 7;
    ord_t cols[ncol];
    sc_t  vals[ncol];

    // deal with dof 0
    cols[0] = uIndex;		vals[0] = FDcoeff1_ + cellSrc00_;
    cols[1] = uIndex+1;		vals[1] = cellSrc01_;
    cols[2] = uIndex+2;		vals[2] = cellSrc02_;
    cols[3] = uWestIndex;	vals[3] = FDcoeffWest_;
    cols[4] = uNorthIndex;	vals[4] = FDcoeffNorth_;
    cols[5] = uEastIndex;	vals[5] = FDcoeffEast_;
    cols[6] = uSouthIndex;	vals[6] = FDcoeffSouth_;
    J_.replaceValues(rowIndex, cols, ncol, vals, false, true);

    // deal with dof 1
    cols[0] = uIndex+1;	vals[0] = FDcoeff1_ + cellSrc11_;
    cols[1] = uIndex;	vals[1] = cellSrc10_;
    cols[2] = uIndex+2; vals[2] = cellSrc12_;
    cols[3] = uWestIndex+1;  vals[3] = FDcoeffWest_;
    cols[4] = uNorthIndex+1; vals[4] = FDcoeffNorth_;
    cols[5] = uEastIndex+1;  vals[5] = FDcoeffEast_;
    cols[6] = uSouthIndex+1; vals[6] = FDcoeffSouth_;
    J_.replaceValues(rowIndex+1, cols, ncol, vals, false, true);

    // deal with dof 2
    cols[0] = uIndex+2;	vals[0] = FDcoeff1_ + cellSrc22_;
    cols[1] = uIndex;	vals[1] = cellSrc20_;
    cols[2] = uIndex+1; vals[2] = cellSrc21_;
    cols[3] = uWestIndex+2;  vals[3] = FDcoeffWest_;
    cols[4] = uNorthIndex+2; vals[4] = FDcoeffNorth_;
    cols[5] = uEastIndex+2;  vals[5] = FDcoeffEast_;
    cols[6] = uSouthIndex+2; vals[6] = FDcoeffSouth_;
    J_.replaceValues(rowIndex+2, cols, ncol, vals, false, true);
  }
};

#endif
