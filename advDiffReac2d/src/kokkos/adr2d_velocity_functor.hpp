
#ifndef ADR2D_VELOCITY_FUNCTOR_HPP_
#define ADR2D_VELOCITY_FUNCTOR_HPP_

#include "MPL_ALL"
#include "UTILS_ALL"
#include <Kokkos_Core.hpp>

template <
  typename state_t,
  typename velocity_t,
  typename coords_t,
  typename graph_t,
  typename advection_functor,
  typename source_functor,
  typename sc_t,
  int32_t numSpecies
  >
struct VelocityFunctor{

  sc_t t_			 = {};
  state_t u_			 = {};
  velocity_t f_			 = {};
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

  VelocityFunctor(sc_t t,
		  state_t u,
		  velocity_t f,
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
      f_{f},
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
  void operator() (const int32_t & rPt) const
  {
    // gID of this cell
    const auto & cellGID_ = graph_(rPt,0);

    // local coordinates
    const auto & thisCellX = coords_(cellGID_, 0);
    const auto & thisCellY = coords_(cellGID_, 1);

    // given a cell, compute index in f of the first dof, i.e. c0
    const auto fIndex = rPt*numSpecies;

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
    sc_t cellSrc0_  = {};
    sc_t cellSrc1_  = {};
    sc_t cellSrc2_  = {};
    srcFunctor_(thisCellX, thisCellY, t_, u0, u1, u2, cellSrc0_, cellSrc1_, cellSrc2_);

    // std::cout << cellGID_ << " "
    // 	      << thisCellX << " "
    // 	      << thisCellY << " "
    // 	      << cellVx_ << " "
    // 	      << cellVy_ << " "
    // 	      << cellSrc0_ << " "
    // 	      << cellSrc1_ << " "
    // 	      << cellSrc2_ << std::endl;

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

    // store the velocity
    f_(fIndex)   = FDcoeff1_ * u0;
    f_(fIndex+1) = FDcoeff1_ * u1;
    f_(fIndex+2) = FDcoeff1_ * u2;
    // given this cell, find index of state for species 0 at WEST cell
    const auto uWestIndex_  = westCellGid*numSpecies;
    // given this cell, find index of state value at NORTH cell
    const auto uNorthIndex_ = northCellGid*numSpecies;
    // given this cell, find index of state value at EAST cell
    const auto uEastIndex_  = eastCellGid*numSpecies;
    // given this cell, find index of state value at SOUTH cell
    const auto uSouthIndex_ = southCellGid*numSpecies;
    //compute velocity
    f_(fIndex) += FDcoeffWest_*u_(uWestIndex_) + FDcoeffNorth_*u_(uNorthIndex_)
      + FDcoeffEast_*u_(uEastIndex_) + FDcoeffSouth_*u_(uSouthIndex_)
      + cellSrc0_;

    f_(fIndex+1) += FDcoeffWest_*u_(uWestIndex_+1) + FDcoeffNorth_*u_(uNorthIndex_+1)
      + FDcoeffEast_*u_(uEastIndex_+1) + FDcoeffSouth_*u_(uSouthIndex_+1)
      + cellSrc1_;

    f_(fIndex+2) += FDcoeffWest_*u_(uWestIndex_+2) + FDcoeffNorth_*u_(uNorthIndex_+2)
      + FDcoeffEast_*u_(uEastIndex_+2) + FDcoeffSouth_*u_(uSouthIndex_+2)
      + cellSrc2_;
  }
};

#endif
