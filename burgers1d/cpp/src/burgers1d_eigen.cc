
#include "burgers1d_eigen.hpp"

void Burgers1dEigen::setup()
{
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
  state_.resize(Ncell_);
  state_.setConstant(one);
  JJ_.resize(Ncell_, Ncell_);
  tripletList_.resize( (Ncell_-1)*2 + 1 );
};


void Burgers1dEigen::velocity_impl(const state_type & u,
				   const scalar_type & t,
				   velocity_type & f) const
{
  constexpr auto one = ::pressio::utils::constants::one<sc_t>();
  constexpr auto two = ::pressio::utils::constants::two<sc_t>();
  constexpr auto oneHalf = one/two;
  // std::cout << "state in velocity" << std::endl;
  // std::cout << std::setprecision(14) << u << std::endl;

  f(0) = oneHalf * dxInv_ * (mu_(0)*mu_(0) - u(0)*u(0));
  for (int_t i=1; i<Ncell_; ++i){
    f(i) = oneHalf * dxInv_ * (u(i-1)*u(i-1) - u(i)*u(i));
  }
  for (int_t i=0; i<Ncell_; ++i){
    f(i) += mu_(1)*exp(mu_(2)*xGrid_(i));
  }
  // std::cout << "velo in velocity" << std::endl;
  // std::cout << std::setprecision(14) << f << std::endl;
}


void Burgers1dEigen::jacobian_impl(const state_type & u,
				   const scalar_type & t,
				   jacobian_type & J) const
{
  // std::cout << "state in jacob" << std::endl;
  // std::cout << std::setprecision(14) << u << std::endl;

  tripletList_[0] = Tr( 0, 0, -dxInv_*u(0));
  int_t k = 0;
  for (int_t i=1; i<Ncell_; ++i){
    tripletList_[++k] = Tr( i, i-1, dxInv_ * u(i-1) );
    tripletList_[++k] = Tr( i, i, -dxInv_ * u(i) );
  }
  J.setFromTriplets(tripletList_.begin(), tripletList_.end());

  // std::cout << "jacobian" << std::endl;
  // Eigen::MatrixXd jd(J);
  // std::cout << std::setprecision(14) << jd << std::endl;
}
