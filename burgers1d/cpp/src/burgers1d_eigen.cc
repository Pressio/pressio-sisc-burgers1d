
#include "burgers1d_eigen.hpp"

void
Burgers1dEigen::setup(){
  constexpr auto one = ::pressio::utils::constants::one<sc_t>();
  constexpr auto two = ::pressio::utils::constants::two<sc_t>();
  constexpr auto oneHalf = one/two;

  dx_ = (xR_ - xL_)/static_cast<scalar_type>(Ncell_);
  dxInv_ = one/dx_;

  // grid
  xGrid_.resize(Ncell_);
  for (int_t i=0; i<Ncell_; ++i)
    xGrid_(i) = dx_*static_cast<sc_t>(i) + dx_*oneHalf;

  // init condition
  U_.resize(Ncell_);
  U_.setConstant(static_cast<scalar_type>(1));
  JJ_.resize(Ncell_, Ncell_);

  tripletList_.resize( (Ncell_-1)*2 + 1 );
};

void
Burgers1dEigen::velocity(const state_type & u,
			 const scalar_type & t,
			 velocity_type & f) const{
  constexpr auto one = ::pressio::utils::constants::one<sc_t>();
  constexpr auto two = ::pressio::utils::constants::two<sc_t>();
  constexpr auto oneHalf = one/two;

  f(0) = oneHalf * dxInv_ * (mu_(0)*mu_(0) - u(0)*u(0));
  for (int_t i=1; i<Ncell_; ++i){
    f(i) = oneHalf * dxInv_ * (u(i-1)*u(i-1) - u(i)*u(i));
  }
  for (int_t i=0; i<Ncell_; ++i){
    f(i) += mu_(1)*exp(mu_(2)*xGrid_(i));
  }
}

Burgers1dEigen::velocity_type
Burgers1dEigen::velocity(const state_type & u,
			 const scalar_type & t) const{
  velocity_type f(Ncell_);
  this->velocity(u, t, f);
  return f;
}

void
Burgers1dEigen::jacobian(const state_type & u,
			 const scalar_type & t,
			 jacobian_type & J) const
{
  tripletList_[0] = Tr( 0, 0, -dxInv_*u(0));
  int_t k = 0;
  for (int_t i=1; i<Ncell_; ++i){
    tripletList_[++k] = Tr( i, i-1, dxInv_ * u(i-1) );
    tripletList_[++k] = Tr( i, i, -dxInv_ * u(i) );
  }
  J.setFromTriplets(tripletList_.begin(), tripletList_.end());
}

Burgers1dEigen::jacobian_type
Burgers1dEigen::jacobian(const state_type & u,
			 const scalar_type & t) const{
  jacobian_type JJ(u.size(), u.size());
  this->jacobian(u, t, JJ);
  return JJ;
}

void
Burgers1dEigen::applyJacobian(const state_type & y,
			      const mv_t & B,
			      const scalar_type & t,
			      mv_t & A) const{
  jacobian(y, t, JJ_);
  A = JJ_ * B;
}

Burgers1dEigen::mv_t
Burgers1dEigen::applyJacobian(const state_type & y,
			      const mv_t & B,
			      const scalar_type & t) const{
  mv_t A( y.size(), B.cols() );
  applyJacobian(y, B, t, A);
  return A;
}
