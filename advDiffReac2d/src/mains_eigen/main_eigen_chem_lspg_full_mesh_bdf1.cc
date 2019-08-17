
#include "CONTAINERS_ALL"
#include "ODE_IMPLICIT"
#include "ODE_INTEGRATORS"
#include "SOLVERS_NONLINEAR"
#include "ROM_LSPG"
#include "adr2d_eigen.hpp"
#include "input_parser.hpp"
#include "eigen_observer.hpp"
#include "advection_cellular_flow_functor.hpp"
#include "source_term_chemABC_functor.hpp"
#include "eigen_utils.hpp"

int main(int argc, char *argv[]){
  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  /*----------------------
   * PARSER
   ----------------------*/
  InputParser parser;
  int err = parser.parse(argc, argv);
  if (err == 1) return 1;

  const auto meshFile	= parser.meshFileName_;
  const auto diffusion	= parser.D_;
  const auto chemReact	= parser.K_;
  const auto dt		= parser.dt_;
  const auto finalT	= parser.finalT_;
  const auto observerOn = parser.observerOn_;
  const auto Nsteps	= parser.NSteps_;
  const auto romSize	= parser.romSize_;
  const auto basisFileName = parser.basisFileName_;

  /*----------------------
   * TYPES
   ----------------------*/
  using scalar_t	= double;
  using src_fnct_t	= ChemistryABCSource<scalar_t>;
  using adv_fnct_t	= CellularFlow<scalar_t>;
  using fom_t		= Adr2dEigen<src_fnct_t, adv_fnct_t>;
  using native_state_t  = typename fom_t::state_type;

  using eig_dyn_vec	= Eigen::Matrix<scalar_t, -1, 1>;
  using eig_dyn_mat	= Eigen::Matrix<scalar_t, -1, -1>;

  using lspg_state_t	= pressio::containers::Vector<eig_dyn_vec>;
  using decoder_jac_t	= pressio::containers::MultiVector<eig_dyn_mat>;
  using decoder_t	= pressio::rom::LinearDecoder<decoder_jac_t>;

  /*----------------------
   * CREATE APP OBJECT
   ----------------------*/
  // functors
  adv_fnct_t advFunctor;
  src_fnct_t srcFunctor(chemReact);

  // app object
  fom_t appObj(meshFile, srcFunctor, advFunctor, diffusion);
  // get the number of total dofs
  const auto stateSize = appObj.getStateSize();

  // the reference state = initial condition
  native_state_t yRef(stateSize);
  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  yRef.setConstant(zero);

  /*----------------------
   * CREATE LINEAR DECODER
   ----------------------*/
  // store modes from file
  decoder_jac_t phi = readBasis<unsigned int>(basisFileName, romSize, stateSize);
  const int numBasis = phi.numVectors();
  if( numBasis != romSize ) return 0;

  // create decoder obj
  decoder_t decoderObj(phi);

  /*----------------------
   * DEFINE LSPG PROBLEM
   ----------------------*/
  // ROM state
  lspg_state_t xROM(romSize);
  xROM.putScalar(zero);

  // define LSPG type
  constexpr auto ode_case  = pressio::ode::ImplicitEnum::Euler;
  using lspg_problem_type = pressio::rom::DefaultLSPGTypeGenerator<
    fom_t, ode_case, decoder_t, lspg_state_t /* ud_ops */>;
  using lspg_stepper_t = typename lspg_problem_type::lspg_stepper_t;
  using lspg_generator = pressio::rom::LSPGUnsteadyProblemGenerator<lspg_problem_type>;
  lspg_generator lspgProblem(appObj, yRef, decoderObj, xROM, zero);

  // linear solver
  using eig_dyn_mat  = Eigen::Matrix<scalar_t, -1, -1>;
  using hessian_t  = pressio::containers::Matrix<eig_dyn_mat>;
  using solver_tag   = pressio::solvers::linear::iterative::LSCG;
  using linear_solver_t = pressio::solvers::iterative::EigenIterative<solver_tag, hessian_t>;
  linear_solver_t linSolverObj;

  // GaussNewton solver
  using gnsolver_t   = pressio::solvers::iterative::GaussNewton<lspg_stepper_t,
  								linear_solver_t>;
  gnsolver_t solver(lspgProblem.stepperObj_, xROM, linSolverObj);
  solver.setTolerance(1e-13);
  // this should converge in few iters every step
  solver.setMaxIterations(10);

  /*----------------------
   * INTEGRATE
   ----------------------*/
  pressio::ode::integrateNSteps(lspgProblem.stepperObj_, xROM, zero, dt, Nsteps, solver);


  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
  	    << std::fixed << std::setprecision(10)
  	    << elapsed.count() << std::endl;

  /*----------------------
   * FINISH UP
   ----------------------*/
  // compute the fom corresponding to our rom final state
  const auto xFomFinal = lspgProblem.yFomReconstructor_(xROM);

  // print mesh coords
  {
    const auto X = appObj.getX();
    const auto Y = appObj.getY();
    std::ofstream file; file.open("xy.txt");
    for(auto i=0; i < X.size(); i++){
      file << std::setprecision(14) << X[i] << " " << Y[i];
      file << std::endl;
    }
    file.close();
  }

  // print reconstructed fom state
  {
    std::ofstream file;
    file.open("xFomReconstructed.txt");
    for(auto i=0; i < xFomFinal.size(); i++){
      file << std::setprecision(15) << xFomFinal[i] << std::endl;
    }
    file.close();
  }

  // // // print solution
  // // std::cout << std::setprecision(14) << *x.data() << std::endl;

  return 0;
}
