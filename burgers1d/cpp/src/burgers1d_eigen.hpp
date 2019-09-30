
#ifndef BURGERS1D_EIGEN_HPP_
#define BURGERS1D_EIGEN_HPP_

#include "Eigen/Dense"
#include "Eigen/SparseCore"
#include <iostream>
#include "UTILS_ALL"

class Burgers1dEigen{
  using int_t	= int32_t;
  using sc_t	= double;
  using eigVec	= Eigen::VectorXd;
  using muVec	= Eigen::Vector3d;
  using mv_t	= Eigen::MatrixXd;
  using eigSM	= Eigen::SparseMatrix<sc_t, Eigen::RowMajor, int_t>;
  using Tr	= Eigen::Triplet<sc_t>;

public:
  using scalar_type	= sc_t;
  using state_type	= eigVec;
  using velocity_type	= eigVec;
  using jacobian_type	= eigSM;

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
		velocity_type & rhs) const;

  velocity_type velocity(const state_type & u,
			 const scalar_type & t) const;

  void jacobian(const state_type & u,
		const scalar_type & t,
		jacobian_type & jac) const;

  jacobian_type jacobian(const state_type & u,
			 const scalar_type & t) const;

  void applyJacobian(const state_type & y,
		     const mv_t & B,
		     const scalar_type & t,
		     mv_t & A) const;

  mv_t applyJacobian(const state_type & y,
		     const mv_t & B,
		     const scalar_type & t) const;

private:
  void setup();

private:
  const scalar_type xL_ = 0.0;		// left side of domain
  const scalar_type xR_ = 100.0;	// right side of domain
  muVec mu_ = {5., 0.02, 0.02};		// parameters
  int_t Ncell_ = {};			// # of cells
  scalar_type dx_ = {};			// cell size
  scalar_type dxInv_ = {};		// inv of cell size
  eigVec xGrid_ = {};			// mesh points coordinates
  mutable state_type U_ = {};		// state vector
  mutable jacobian_type JJ_ = {};	// jacobian matrix
  mutable std::vector<Tr> tripletList_ = {};

};//end class

#endif
