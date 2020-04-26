
#ifndef BURGERS1D_EIGEN_HPP_
#define BURGERS1D_EIGEN_HPP_

#include "Eigen/Dense"
#include "Eigen/SparseCore"
#include <iostream>
#include <chrono>
#include "UTILS_ALL"

class Burgers1dEigen
{
  // Eigen matrices have to have a signed integer type, use int32_t
  using int_t	= int32_t;

  using sc_t	= double;
  using Tr	= Eigen::Triplet<sc_t>;
  // column vector in Eigen
  using eigVec	= Eigen::VectorXd;
  // muVec is a vector to store parameters
  using muVec	= Eigen::Vector3d;

public:
  // eigen sparse matrix
  static constexpr auto spmat_layout = Eigen::RowMajor;
  using eig_sparse_mat	  = Eigen::SparseMatrix<sc_t, spmat_layout, int_t>;

  // eigen dense matrix
  static constexpr auto dmat_layout = Eigen::ColMajor;
  using eig_dense_mat	  = Eigen::Matrix<sc_t, -1, -1, dmat_layout>;

  using scalar_type	  = sc_t;
  using state_type	  = eigVec;
  using velocity_type	  = state_type;
  using dense_matrix_type = eig_dense_mat;
#ifdef USE_DENSE
  using jacobian_type	  = eig_dense_mat;
#else
  using jacobian_type	  = eig_sparse_mat;
#endif

public:
  Burgers1dEigen(muVec params, int_t Ncell)
    : mu_(params), Ncell_(Ncell){
    setup();
  }

  explicit Burgers1dEigen(int_t Ncell)
    : Ncell_(Ncell){
    setup();
  }

  Burgers1dEigen() = delete;
  ~Burgers1dEigen() = default;

public:
  void velocity(const state_type & u,
		const scalar_type & t,
		velocity_type & f) const{
    this->velocity_impl(u, t, f);
  }

  velocity_type velocity(const state_type & u,
			 const scalar_type & t) const{
    this->velocity_impl(u, t, f_);
    return f_;
  }

  void jacobian(const state_type & u,
  		const scalar_type & t,
  		jacobian_type & jac) const{
    this->jacobian_impl(u, t, jac);
  }

  jacobian_type jacobian(const state_type & u,
  			 const scalar_type & t) const{
    this->jacobian_impl(u, t, JJ_);
    return JJ_;
  }

  void applyJacobian(const state_type & u,
  		     const dense_matrix_type & B,
  		     const scalar_type & t,
  		     dense_matrix_type & A) const{
    this->jacobian_impl(u, t, JJ_);
    A = JJ_ * B;
  }

  dense_matrix_type applyJacobian(const state_type & u,
				  const dense_matrix_type & B,
				  const scalar_type & t) const{
    dense_matrix_type A( u.size(), B.cols() );
    this->applyJacobian(u, B, t, A);
    return A;
  }

private:
  void setup(){
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

  void velocity_impl(const state_type & u,
		     const scalar_type & t,
		     velocity_type & f) const
  {
    constexpr auto one = ::pressio::utils::constants::one<sc_t>();
    constexpr auto two = ::pressio::utils::constants::two<sc_t>();
    constexpr auto oneHalf = one/two;

    const auto coeff = oneHalf * dxInv_;
    f(0) = coeff * (mu_(0)*mu_(0) - u(0)*u(0)) + expVec_(0);
    for (int_t i=1; i<Ncell_; ++i){
      f(i) = coeff * (u(i-1)*u(i-1) - u(i)*u(i)) + expVec_(i);
    }
  }

  // the sparse jacobian impl
  template <
    typename _jac_type = jacobian_type,
    typename std::enable_if<
      std::is_same<_jac_type, eig_sparse_mat>::value
      >::type * = nullptr
    >
  void jacobian_impl(const state_type & u,
  		     const scalar_type & t,
  		     _jac_type & jac) const
  {
    tripletList_[0] = Tr( 0, 0, -dxInv_*u(0));
    int_t k = 0;
    for (int_t i=1; i<Ncell_; ++i){
      tripletList_[++k] = Tr( i, i-1, dxInv_ * u(i-1) );
      tripletList_[++k] = Tr( i, i, -dxInv_ * u(i) );
    }
    jac.setFromTriplets(tripletList_.begin(), tripletList_.end());
    if ( !jac.isCompressed() )
      jac.makeCompressed();
  }

  // --------------------------------------------------------------
  /* dense jacobian impl is based on the layout */
  template <
    typename _jac_type = jacobian_type,
    typename std::enable_if<
      std::is_same<_jac_type, eig_dense_mat>::value and
      eig_dense_mat::IsRowMajor == 0
      >::type * = nullptr
    >
  void jacobian_impl(const state_type & u,
  		     const scalar_type & t,
  		     _jac_type & jac) const{
    for (int_t i=0; i<Ncell_-1; ++i){
      jac(i, i)   = -dxInv_ * u(i);
      jac(i+1, i) = dxInv_ * u(i);
    }
    jac(Ncell_-1,Ncell_-1) = -dxInv_*u(Ncell_-1);
  }

  template <
    typename _jac_type = jacobian_type,
    typename std::enable_if<
      std::is_same<_jac_type, eig_dense_mat>::value
      and eig_dense_mat::IsRowMajor == 1
      >::type * = nullptr
    >
  void jacobian_impl(const state_type & u,
  		     const scalar_type & t,
  		     _jac_type & jac) const{
    jac(0,0) = -dxInv_*u(0);
    for (int_t i=1; i<Ncell_; ++i){
      jac(i, i)   = -dxInv_ * u(i);
      jac(i, i-1) = dxInv_ * u(i-1);
    }
  }
  // --------------------------------------------------------------

private:
  const scalar_type xL_ = 0.0;		// left side of domain
  const scalar_type xR_ = 100.0;	// right side of domain
  muVec mu_ = {5., 0.02, 0.02};		// parameters
  int_t Ncell_ = {};			// # of cells
  scalar_type dx_ = {};			// cell size
  scalar_type dxInv_ = {};		// inv of cell size

  eigVec xGrid_ = {};			// mesh points coordinates
  eigVec expVec_ = {};			// mu(1)*exp(mu[2] * x)

  mutable state_type state_ = {};	// state vector
  mutable velocity_type f_ = {};	// velocity
  mutable jacobian_type JJ_ = {};	// jacobian matrix
  mutable std::vector<Tr> tripletList_ = {};

};//end class

#endif
