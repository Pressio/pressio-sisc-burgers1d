
#ifndef ADVECTION_FOUR_EDDY_MODEL_FUNCTOR_HPP_
#define ADVECTION_FOUR_EDDY_MODEL_FUNCTOR_HPP_

#include "UTILS_ALL"
#include <Kokkos_Core.hpp>

template <typename sc_t, typename state_t = void>
class FourEddyModel
{
  using this_t  = FourEddyModel<sc_t, state_t>;
  using cell_adv_arr_t  = std::array<sc_t, 2>;
  using cell_state_arr_t  = std::array<sc_t, 3>;

  // number of eddies
  static constexpr auto numEddies_ = 4;

  // scaling factor for eddy strength
  const sc_t A_ = 15.;

  const std::array<sc_t, numEddies_> eddiesOx_   = {{0.75, 0.25,  0.35,  0.85}};
  const std::array<sc_t, numEddies_> eddiesOy_   = {{0.25, 0.25, 0.7, 0.8}};
  const std::array<sc_t, numEddies_> eddiesR_    = {{0.1, 0.05, 0.07, 0.085}};
  const std::array<int,  numEddies_> eddiesSign_ = {{1, -1, -1, -1}};
  // store 1/2R_i^2 for each i
  std::array<sc_t, numEddies_> twoRiSqInv_;

public:
  static constexpr auto zero	= ::pressio::utils::constants::zero<sc_t>();
  static constexpr auto one	= ::pressio::utils::constants::one<sc_t>();
  static constexpr auto two	= ::pressio::utils::constants::two<sc_t>();

public:
  FourEddyModel(){
    for (auto i=0; i<numEddies_; ++i){
      twoRiSqInv_[i] = one/(two*eddiesR_[i]*eddiesR_[i]);
    }
  }

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_adv_arr_t & vxy) const
  {
    // compute vx
    vxy[0] = zero;
    vxy[1] = zero;
    for (auto i=0; i<numEddies_; ++i){
      auto dx = x - eddiesOx_[i];
      auto dy = y - eddiesOy_[i];
      auto expV = (dx*dx + dy*dy) * twoRiSqInv_[i];
      vxy[0] +=  eddiesSign_[i] * std::exp(-expV) * dy;
      vxy[1] += -eddiesSign_[i] * std::exp(-expV) * dx;
    }
    vxy[0] *= A_;
    vxy[1] *= A_;
  }

  KOKKOS_INLINE_FUNCTION
  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const sc_t & u0,
		  const sc_t & u1,
		  const sc_t & u2,
		  sc_t & vx,
		  sc_t & vy) const
  {
    // compute vx
    vx = zero;
    vy = zero;
    for (auto i=0; i<numEddies_; ++i){
      auto dx = x - eddiesOx_[i];
      auto dy = y - eddiesOy_[i];
      auto expV = (dx*dx + dy*dy) * twoRiSqInv_[i];
      vx +=  eddiesSign_[i] * std::exp(-expV) * dy;
      vy += -eddiesSign_[i] * std::exp(-expV) * dx;
    }
    vx *= A_;
    vy *= A_;
  }
};

#endif
