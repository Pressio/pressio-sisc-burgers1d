
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "SOLVERS_NONLINEAR"
#include "burgers1d_eigen.hpp"

int main(int argc, char *argv[]){
  // type aliasing
  using app_t		= Burgers1dEigen;
  using scalar_t	= typename app_t::scalar_type;
  using app_state_t	= typename app_t::state_type;
  using app_rhs_t	= typename app_t::velocity_type;
  using app_jacob_t	= typename app_t::jacobian_type;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  // create app object
  constexpr int Ncell = 100;
  app_t appObj(Ncell);

  // initial state
  app_state_t y0n(Ncell);
  y0n.setConstant(one);

  // types for ode
  using ode_state_t = pressio::containers::Vector<app_state_t>;
  using ode_res_t   = pressio::containers::Vector<app_rhs_t>;
  using ode_jac_t   = pressio::containers::Matrix<app_jacob_t>;

  ode_state_t y(y0n);
  constexpr auto ode_case = pressio::ode::ImplicitEnum::Euler;
  using stepper_t = pressio::ode::ImplicitStepper<
    ode_case, ode_state_t, ode_res_t, ode_jac_t, app_t>;
  stepper_t stepperObj(y, appObj);

  // define solver
  using lin_solver_t = pressio::solvers::iterative::EigenIterative<
    pressio::solvers::linear::iterative::Bicgstab, ode_jac_t>;
  pressio::solvers::NewtonRaphson<scalar_t, lin_solver_t> solverO;
  solverO.setMaxIterations(10);
  // by default, newton raphson exits when norm of correction is below tolerance
  solverO.setTolerance(1e-12);

  // integrate in time
  const scalar_t fint = 35;
  const scalar_t dt = 0.01;
  const auto Nsteps = static_cast<unsigned int>(fint/dt);
  pressio::ode::integrateNSteps(stepperObj, y, zero, dt, Nsteps, solverO);
  std::cout << std::setprecision(14) << *y.data();
  // {
  //   using namespace pressio::apps::test;
  //   checkSol(y, Burgers1dImpGoldStates<ode_case>::get(Ncell, dt, fint));
  // }

  return 0;
}
