
#include "burgers1d_eigen.hpp"

void Burgers1dEigen::setup()
{
  constexpr auto zero = ::pressio::utils::constants::zero<sc_t>();
  constexpr auto one = ::pressio::utils::constants::one<sc_t>();
  constexpr auto two = ::pressio::utils::constants::two<sc_t>();
  constexpr auto oneHalf = one/two;

  dx_ = (xR_ - xL_)/static_cast<scalar_type>(Ncell_);
  dxInv_ = one/dx_;

  // grid
  xGrid_.resize(Ncell_);
  expVec_.resize(Ncell_);
  for (int_t i=0; i<Ncell_; ++i){
    xGrid_(i) = dx_*static_cast<sc_t>(i) + dx_*oneHalf;
    expVec_(i) = mu_(1) * std::exp(mu_(2)*xGrid_(i));
  }

  // init condition
  state_.resize(Ncell_); state_.setConstant(one);
  f_.resize(Ncell_); f_.setConstant(zero);

  JJ_.resize(Ncell_, Ncell_);
  tripletList_.resize( (Ncell_-1)*2 + 1 );
};
