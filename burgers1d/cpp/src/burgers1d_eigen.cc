
#include "burgers1d_eigen.hpp"

void
Burgers1dEigen::setup(){
  dx_ = (xR_ - xL_)/static_cast<scalar_type>(Ncell_);
  dxInv_ = 1.0/dx_;

  // grid
  xGrid_.resize(Ncell_);
  for (ui_t i=0; i<Ncell_; ++i)
    xGrid_(i) = dx_*i + dx_*0.5;

  // init condition
  U_.resize(Ncell_);
  U_.setConstant(static_cast<scalar_type>(1));
  JJ_.resize(Ncell_, Ncell_);
};

void
Burgers1dEigen::velocity(const state_type & u,
			 const scalar_type & t,
			 velocity_type & f) const{

  f(0) = 0.5 * dxInv_ * (mu_(0)*mu_(0) - u(0)*u(0));
  for (ui_t i=1; i<Ncell_; ++i){
    f(i) = 0.5 * dxInv_ * (u(i-1)*u(i-1) - u(i)*u(i));
  }
  for (ui_t i=0; i<Ncell_; ++i){
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
			 jacobian_type & J) const{

  //evaluate jacobian
  if (J.rows() == 0 || J.cols()==0 ){
    J.resize(Ncell_, Ncell_);
  }

  tripletList.clear();
  tripletList.push_back( Tr( 0, 0, -dxInv_*u(0)) );
  for (ui_t i=1; i<Ncell_; ++i){
    tripletList.push_back( Tr( i, i-1, dxInv_ * u(i-1) ) );
    tripletList.push_back( Tr( i, i, -dxInv_ * u(i) ) );
  }
  J.setFromTriplets(tripletList.begin(), tripletList.end());
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
