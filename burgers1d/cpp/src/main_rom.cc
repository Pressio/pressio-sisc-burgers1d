
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "SOLVERS_NONLINEAR"
#include "ROM_LSPG"
#include "utils.hpp"
#include "burgers1d_input_parser.hpp"
#include "burgers1d_eigen.hpp"
#include "eigen_observer.hpp"

int main(int argc, char *argv[]){
  using fom_t		= Burgers1dEigen;
  using scalar_t	= typename fom_t::scalar_type;

  using eig_dyn_vec	= Eigen::Matrix<scalar_t, -1, 1>;
  using lspg_state_t	= pressio::containers::Vector<eig_dyn_vec>;

  using eig_dyn_mat	= Eigen::Matrix<scalar_t, -1, -1>;
  using decoder_jac_t	= pressio::containers::MultiVector<eig_dyn_mat>;
  using decoder_t	= pressio::rom::LinearDecoder<decoder_jac_t>;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  // parse input file
  InputParser parser;
  int err = parser.parse(argc, argv);
  if (err == 1) return 1;

  // store inputs
  const auto numCell = parser.numCell_;
  const auto dt = parser.dt_;
  const auto finalT = parser.finalT_;
  const auto observerOn = parser.observerOn_;
  const auto Nsteps = static_cast<unsigned int>(finalT/dt);

  const auto romSize = parser.romSize_;
  const auto basisFileName = parser.basisFileName_;

  // app object
  fom_t appobj(numCell);

  // store modes computed before from file
  decoder_jac_t phi = readBasis(basisFileName, romSize, numCell);
  const int numBasis = phi.numVectors();
  if( numBasis != romSize ) return 0;

  // create decoder obj
  decoder_t decoderObj(phi);

  // for this problem, my reference state = initial state
  typename fom_t::state_type yRef(numCell);
  yRef.setConstant(one);

  // define ROM state
  lspg_state_t yROM(romSize);
  yROM.putScalar(zero);

  constexpr auto t0 = zero;

  // define LSPG type
  constexpr auto ode_case  = pressio::ode::ImplicitEnum::Euler;
  using lspg_problem_types = pressio::rom::DefaultLSPGTypeGenerator<
    fom_t, ode_case, decoder_t, lspg_state_t>;
  pressio::rom::LSPGUnsteadyProblemGenerator<lspg_problem_types> lspgProblem(
      appobj, yRef, decoderObj, yROM, t0);

  using lspg_stepper_t = typename lspg_problem_types::lspg_stepper_t;

  // linear solver
  using eig_dyn_mat  = Eigen::Matrix<scalar_t, -1, -1>;
  using hessian_t  = pressio::containers::Matrix<eig_dyn_mat>;
  using solver_tag   = pressio::solvers::linear::iterative::LSCG;
  using linear_solver_t = pressio::solvers::iterative::EigenIterative<solver_tag, hessian_t>;
  linear_solver_t linSolverObj;

  // GaussNewton solver
  // hessian comes up in GN solver, it is (J phi)^T (J phi)
  // rom is solved using eigen, hessian is wrapper of eigen matrix
  using eig_dyn_mat  = Eigen::Matrix<scalar_t, -1, -1>;
  using gnsolver_t   = pressio::solvers::iterative::GaussNewton<
    lspg_stepper_t, linear_solver_t>;
  gnsolver_t solver(lspgProblem.stepperObj_, yROM, linSolverObj);
  solver.setTolerance(1e-13);
  // I know this should converge in few iters every step
  solver.setMaxIterations(5);

  // integrate in time
  pressio::ode::integrateNSteps(lspgProblem.stepperObj_, yROM, t0, dt, Nsteps, solver);

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: " <<
    std::fixed << std::setprecision(10) <<
    elapsed.count() << std::endl;

  // compute the fom corresponding to our rom final state
  auto yFomFinal = lspgProblem.yFomReconstructor_(yROM);
  std::cout << *yFomFinal.data() << std::endl;

  // std::cout << checkStr <<  std::endl;
  return 0;
}




  // // this is a reproducing ROM test, so the final reconstructed state
  // // has to match the FOM solution obtained with euler, same time-step, for 10 steps
  // // const auto trueY = pressio::apps::test::Burg1DtrueImpEulerN20t010;
  // const auto trueY = pressio::apps::test::Burgers1dImpGoldStates<ode_case>::get(numCell, dt, 0.10);
  // for (auto i=0; i<yFomFinal.size(); i++)
  //   if (std::abs(yFomFinal[i] - trueY[i]) > 1e-10) checkStr = "FAILED";

  //   auto n1 = ::pressio::containers::ops::norm2(yFomFinal);
  //   std::cout << n1 << std::endl;
