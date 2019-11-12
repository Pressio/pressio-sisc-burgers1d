
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "SOLVERS_NONLINEAR"
#include "ROM_LSPG_UNSTEADY"
#include <chrono>
#include "utils.hpp"
#include "burgers1d_input_parser.hpp"
#include "burgers1d_eigen.hpp"
#include "eigen_observer.hpp"

int main(int argc, char *argv[])
{
  using fom_t		= Burgers1dEigen;
  using scalar_t	= typename fom_t::scalar_type;

  // eigen native
  using eig_dyn_vec	= Eigen::Matrix<scalar_t, -1, 1>;
  using eig_dyn_mat	= Eigen::Matrix<scalar_t, -1, -1>;

  using lspg_state_t	= pressio::containers::Vector<eig_dyn_vec>;

  using native_state_t	= typename fom_t::state_type;
  using native_dmat_t	= typename fom_t::dense_matrix_type;
  using decoder_jac_t	= pressio::containers::MultiVector<native_dmat_t>;
  using decoder_t	= pressio::rom::LinearDecoder<decoder_jac_t>;
  using hessian_t	= pressio::containers::Matrix<eig_dyn_mat>;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  // parse input file
  InputParser parser;
  int32_t err = parser.parse(argc, argv);
  if (err == 1) return 1;

  // app object
  fom_t appobj(parser.numCell_);

  // store modes computed before from file
  // store basis vectors into native format
  const auto phiNative = readBasis<scalar_t, int32_t, native_dmat_t>(parser.basisFileName_, parser.romSize_);
  // wrap native basis with a pressio wrapper
  const decoder_jac_t phi(phiNative);
  const int32_t numBasis = phi.numVectors();
  if( numBasis != parser.romSize_ ) return 0;

  // create decoder obj
  decoder_t decoderObj(phi);

  // the reference state = initial condition, all ones
  native_state_t yRef(parser.numCell_);
  yRef.setConstant(one);

  // define ROM state
  lspg_state_t yROM(parser.romSize_);
  yROM.putScalar(zero);

  auto startTime = std::chrono::high_resolution_clock::now();

  // define LSPG type
  constexpr auto ode_case  = pressio::ode::ImplicitEnum::Euler;
  using lspg_problem = pressio::rom::LSPGUnsteadyProblem<pressio::rom::DefaultLSPGUnsteady,
  							 ode_case, fom_t, lspg_state_t, decoder_t>;
  using lspg_stepper_t = typename lspg_problem::lspg_stepper_t;
  lspg_problem lspgProblem(appobj, yRef, decoderObj, yROM, zero);

  // linear solver
  using solver_tag   = pressio::solvers::linear::direct::PartialPivLU;
  using linear_solver_t = pressio::solvers::direct::EigenDirect<solver_tag, hessian_t>;
  linear_solver_t linSolverObj;

  // non-linear solver
  using lspg_stepper_t = typename lspg_problem::lspg_stepper_t;
  using gnsolver_t   = pressio::solvers::iterative::GaussNewton<lspg_stepper_t, linear_solver_t>;
  gnsolver_t solver(lspgProblem.getStepperRef(), yROM, linSolverObj);
  solver.setMaxIterations(20);
  solver.setTolerance(1e-13);

  // get stepper object
  auto & stepper = lspgProblem.getStepperRef();

  // integrate in time
  pressio::ode::integrateNSteps(stepper, yROM, zero, parser.dt_, parser.numSteps_, solver);

  // Record run time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: " << std::fixed << std::setprecision(10)
  	    << elapsed.count() << std::endl;

  // compute the fom corresponding to our rom final state
  const auto yFomFinal = lspgProblem.getFomStateReconstructorCRef()(yROM);
  printEigenVectorToFile("yFomReconstructed.txt", *yFomFinal.data());

  // print generalized coords
  printEigenVectorToFile("final_generalized_coords.txt", *yROM.data());

  return 0;
}



// // print summary from timers
// #ifdef HAVE_TEUCHOS_TIMERS
// pressio::utils::TeuchosPerformanceMonitor::stackedTimersReportSerial();
// #endif
