
#include "CONTAINERS_ALL"
#include "ODE_INTEGRATORS"
#include "ODE_IMPLICIT"
#include "SOLVERS_NONLINEAR"
#include "burgers1d_eigen.hpp"
#include "burgers1d_input_parser.hpp"
#include "eigen_observer.hpp"
#include <chrono>

int main(int argc, char *argv[]){
  // type aliasing
  using app_t		= Burgers1dEigen;
  using scalar_t	= typename app_t::scalar_type;
  using app_state_t	= typename app_t::state_type;
  using app_velo_t	= typename app_t::velocity_type;
  using app_jacob_t	= typename app_t::jacobian_type;

  using ode_state_t	= pressio::containers::Vector<app_state_t>;
  using ode_r_t		= pressio::containers::Vector<app_velo_t>;
  using ode_j_t		= pressio::containers::Matrix<app_jacob_t>;

  constexpr auto zero	= ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one	= ::pressio::utils::constants::one<scalar_t>();

  // parse input file
  InputParser parser;
  int err = parser.parse(argc, argv);
  if (err == 1) return 1;

  // store cmd line args
  const auto numCell = parser.numCell_;
  const auto dt = parser.dt_;
  const auto finalT = parser.finalT_;
  const auto observerOn = parser.observerOn_;
  const auto Nsteps = static_cast<unsigned int>(finalT/dt);

  // create app object
  app_t appObj(numCell);

  // initial state
  app_state_t x0n(numCell);
  x0n.setConstant(one);
  // wrap init state
  ode_state_t x(x0n);

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  // define stepper
  constexpr auto ode_case = pressio::ode::ImplicitEnum::Euler;
  using stepper_t = pressio::ode::ImplicitStepper<
    ode_case, ode_state_t, ode_r_t, ode_j_t, app_t>;
  stepper_t stepperObj(x, appObj);

  // define linear solver
  using lin_solver_t = pressio::solvers::iterative::EigenIterative<
    pressio::solvers::linear::iterative::Bicgstab, ode_j_t>;
  lin_solver_t linearSolverObj;

  // define non-linear solver
  using non_lin_solver_t = pressio::solvers::NewtonRaphson<stepper_t, lin_solver_t, scalar_t>;
  non_lin_solver_t solverObj(stepperObj, x, linearSolverObj);
  // by default, newton raphson exits when norm of correction is below tolerance
  solverObj.setTolerance(1e-13);
  solverObj.setMaxIterations(10);

  // integrate in time
  if (observerOn == 1){
    // construct observer, pass y which now contains init condition
    using obs_t = EigenObserver<ode_state_t>;
    obs_t Obs(Nsteps, numCell, x, parser.snapshotsFreq_);
    pressio::ode::integrateNSteps(stepperObj, x, zero, dt, Nsteps, Obs, solverObj);

    // print snapshots to file
    Obs.printSnapshotsToFile(parser.shapshotsFileName_);

    // compute SVD
    const auto & S = Obs.viewSnapshots();
    // do SVD and compute basis
    Eigen::JacobiSVD<typename obs_t::matrix_t> svd(S, Eigen::ComputeThinU);
    const auto U = svd.matrixU();

    std::cout << "Print basis to file" << std::endl;
    printEigenDMatrixToFile(parser.basisFileName_, U);
    std::cout << "Done with basis" << std::endl;
  }
  else{
    pressio::ode::integrateNSteps(stepperObj, x, zero, dt, Nsteps, solverObj);
  }

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
	    << std::fixed << std::setprecision(10)
	    << elapsed.count() << std::endl;

  printEigenVectorToFile("yFom.txt", *x.data());

  return 0;
}
