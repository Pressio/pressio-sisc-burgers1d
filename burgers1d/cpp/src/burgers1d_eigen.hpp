
#ifndef BURGERS1D_EIGEN_HPP_
#define BURGERS1D_EIGEN_HPP_

#include "Eigen/Dense"
#include "Eigen/SparseCore"
#include <iostream>
#include "UTILS_ALL"

class Burgers1dEigen{
  using int_t	= int32_t;
  using sc_t	= double;
  using Tr	= Eigen::Triplet<sc_t>;

  // dynamic column vector
  using eigVec	= Eigen::Matrix<sc_t, -1, 1>;
  using muVec	= Eigen::Vector3d;

public:
  static constexpr auto spmat_layout = Eigen::ColMajor;

  //// Eigen SparseMatrix has to have a signed integer type, use index_t
  using eig_sp_mat   = Eigen::SparseMatrix<sc_t, spmat_layout, int_t>;
  using eig_dense_mat = Eigen::Matrix<sc_t, -1, -1, Eigen::ColMajor>;

  using scalar_type	= sc_t;
  using state_type	= eigVec;
  using velocity_type	= state_type;
  using jacobian_type	= eig_sp_mat;

  // for some reason, the best outcome is when the sparse is row-major
  // and the dense matrix is colmajor
  using dmatrix_type	= eig_dense_mat;

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
    velocity_type f(Ncell_);
    this->velocity_impl(u, t, f);
    return f;
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
		     const dmatrix_type & B,
		     const scalar_type & t,
		     dmatrix_type & A) const{
    this->jacobian_impl(u, t, JJ_);
    A = JJ_ * B;
  }

  dmatrix_type applyJacobian(const state_type & u,
			     const dmatrix_type & B,
			     const scalar_type & t) const{
    dmatrix_type A( u.size(), B.cols() );
    this->applyJacobian(u, B, t, A);
    return A;
  }

private:
  void setup();

  void velocity_impl(const state_type & u,
		     const scalar_type & t,
		     velocity_type & f) const
  {
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

  template <
    typename _jac_type = jacobian_type,
    typename std::enable_if<
      std::is_same<
	_jac_type, eig_dense_mat
	>::value
      >::type * = nullptr
    >
  void jacobian_impl(const state_type & u,
		     const scalar_type & t,
		     _jac_type & jac) const
  {
    jac(0,0) = -dxInv_*u(0);
    for (int_t i=1; i<Ncell_; ++i){
      jac(i, i-1) = dxInv_ * u(i-1);
      jac(i, i) = -dxInv_ * u(i);
    }
  }

  template <
    typename _jac_type = jacobian_type,
    typename std::enable_if<
      std::is_same<
	_jac_type, eig_sp_mat
	>::value
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

private:
  const scalar_type xL_ = 0.0;		// left side of domain
  const scalar_type xR_ = 100.0;	// right side of domain
  muVec mu_ = {5., 0.02, 0.02};		// parameters
  int_t Ncell_ = {};			// # of cells
  scalar_type dx_ = {};			// cell size
  scalar_type dxInv_ = {};		// inv of cell size
  eigVec xGrid_ = {};			// mesh points coordinates

  mutable state_type state_ = {};		// state vector
  mutable jacobian_type JJ_ = {};	// jacobian matrix
  mutable std::vector<Tr> tripletList_ = {};

};//end class

#endif
