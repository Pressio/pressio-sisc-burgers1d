
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "SOLVERS_NONLINEAR"
#include "ROM_LSPG"
#include <chrono>
#include "utils.hpp"
#include "burgers1d_input_parser.hpp"
#include "burgers1d_eigen.hpp"
#include "eigen_observer.hpp"

int main(int argc, char *argv[])
{
  using fom_t		= Burgers1dEigen;
  using scalar_t	= typename fom_t::scalar_type;

  // eigen native column vector
  using eig_dyn_vec	= Eigen::Matrix<scalar_t, -1, 1>;
  // wrap it
  using lspg_state_t	= pressio::containers::Vector<eig_dyn_vec>;
  using native_dmat_t	= typename fom_t::dmatrix_type;

  using eig_dyn_mat	= Eigen::Matrix<scalar_t, -1, -1>;
  using decoder_jac_t	= pressio::containers::MultiVector<eig_dyn_mat>;
  using decoder_t	= pressio::rom::LinearDecoder<decoder_jac_t>;
  using hessian_t	= pressio::containers::Matrix<eig_dyn_mat>;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  // parse input file
  InputParser parser;
  int32_t err = parser.parse(argc, argv);
  if (err == 1) return 1;

  // store inputs
  const auto numCell	= parser.numCell_;
  const auto dt		= parser.dt_;
  const auto finalT	= parser.finalT_;
  const auto observerOn = parser.observerOn_;
  const auto Nsteps	= static_cast<int32_t>(finalT/dt);
  const auto romSize	= parser.romSize_;
  const auto basisFileName = parser.basisFileName_;

  // initial time
  constexpr auto t0 = zero;

  // app object
  fom_t appobj(numCell);

  // store modes computed before from file
  // store basis vectors into native format
  const auto phiNative = readBasis<scalar_t, int32_t, native_dmat_t>(basisFileName,
								     romSize);
  // wrap native basis with a pressio wrapper
  const decoder_jac_t phi(phiNative);
  const int32_t numBasis = phi.numVectors();
  if( numBasis != romSize ) return 0;

  // create decoder obj
  decoder_t decoderObj(phi);

  // the reference state = initial condition
  typename fom_t::state_type yRef(numCell);
  yRef.setConstant(one);

  // define ROM state
  lspg_state_t yROM(romSize);
  yROM.putScalar(zero);

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  // define LSPG type
  constexpr auto ode_case  = pressio::ode::ImplicitEnum::Euler;
  using lspg_problem_type = pressio::rom::DefaultLSPGTypeGenerator<
				fom_t, ode_case, decoder_t, lspg_state_t>;
  using lspg_generator     = pressio::rom::LSPGUnsteadyProblemGenerator<lspg_problem_type>;
  lspg_generator lspgProblem(appobj, yRef, decoderObj, yROM, t0);

  // linear solver
  //using solver_tag   = pressio::solvers::linear::iterative::LSCG;
  //using linear_solver_t = pressio::solvers::iterative::EigenIterative<solver_tag, hessian_t>;
  using solver_tag   = pressio::solvers::linear::direct::PartialPivLU;
  using linear_solver_t = pressio::solvers::direct::EigenDirect<solver_tag, hessian_t>;
  linear_solver_t linSolverObj;

  // non-linear solver
  using lspg_stepper_t = typename lspg_problem_type::lspg_stepper_t;
  using gnsolver_t   = pressio::solvers::iterative::GaussNewton<lspg_stepper_t, linear_solver_t>;
  gnsolver_t solver(lspgProblem.stepperObj_, yROM, linSolverObj);
  solver.setTolerance(1e-13);
  solver.setMaxIterations(20);

  // integrate in time
  pressio::ode::integrateNSteps(lspgProblem.stepperObj_, yROM, t0, dt, Nsteps, solver);

  // Record run time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
	    << std::fixed << std::setprecision(10)
	    << elapsed.count() << std::endl;

  {
    // compute the fom corresponding to our rom final state
    const auto yFomFinal = lspgProblem.yFomReconstructor_(yROM);
    // print reconstructed fom state
    std::ofstream file;
    file.open("yFomReconstructed.txt");
    for(size_t i=0; i < yFomFinal.size(); i++){
      file << std::setprecision(15) << yFomFinal[i] << std::endl;
    }
    file.close();
  }
  {
    std::cout << "Printing first 5 elements of gen coords" << std::endl;
    for (int32_t i=0; i<5; ++i)
      std::cout << (*yROM.data())[i] << std::endl;

    // print generalized coords
    std::ofstream file;
    file.open("final_generalized_coords.txt");
    for(size_t i=0; i < yROM.size(); i++){
      file << std::setprecision(15) << yROM[i] << std::endl;
    }
    file.close();
  }

  return 0;
}
