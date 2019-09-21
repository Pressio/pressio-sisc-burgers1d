
#include "CONTAINERS_ALL"
#include "ODE_IMPLICIT"
#include "ODE_INTEGRATORS"
#include "QR_BASIC"
#include "SOLVERS_NONLINEAR"
#include "ROM_LSPG"
#include "adr2d_eigen.hpp"
#include "input_parser.hpp"
#include "eigen_observer.hpp"
#include "advection_cellular_flow_functor.hpp"
#include "source_term_chemABC_functor.hpp"
#include "eigen_utils.hpp"

//this contains the custom ops to enable sample mesh
#include "eigen_sample_mesh_time_discrete_ops.hpp"


int main(int argc, char *argv[]){

  /*----------------------
   * PARSER
   ----------------------*/
  InputParser parser;
  int err = parser.parse(argc, argv);
  if (err == 1) return 1;

  /*----------------------
   * TYPES
   ----------------------*/
  using scalar_t	= double;

  using src_fnct_t	= ChemistryABCSource<scalar_t>;
  using adv_fnct_t	= CellularFlow<scalar_t>;

  using fom_t		= Adr2dEigen<src_fnct_t, adv_fnct_t>;
  using native_state_t  = typename fom_t::state_type;
  using native_dmat_t	= typename fom_t::dmatrix_type;

  using eig_dyn_vec	= Eigen::Matrix<scalar_t, Eigen::Dynamic, 1>;

  using lspg_state_t	= pressio::containers::Vector<eig_dyn_vec>;
  using decoder_jac_t	= pressio::containers::MultiVector<native_dmat_t>;
  using decoder_t	= pressio::rom::LinearDecoder<decoder_jac_t>;

  // the hessian is used only if LSPG is solved using NormalEquations
  using eig_dyn_mat	= Eigen::Matrix<scalar_t, Eigen::Dynamic, Eigen::Dynamic>;
  using lspg_hessian_t	= pressio::containers::Matrix<eig_dyn_mat>;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();

  /*----------------------
   * CREATE APP OBJECT
   ----------------------*/
  // functors for advection and source
  adv_fnct_t advFunctor;
  src_fnct_t srcFunctor(parser.K_);

  // app object (this will load the mesh)
  fom_t appObj(parser.meshFileName_, srcFunctor, advFunctor, parser.D_);
  // get the number of total dofs
  const auto stateSize = appObj.getStateSize();

  // the reference state = all zeros
  native_state_t yRef(stateSize);
  yRef.setConstant(zero);

  /*----------------------
   * READ BASIS VECTORS
   ----------------------*/
  // this loads the full basis, i.e. wrt the full mesh was used

  const decoder_jac_t phi0 = readBasis<unsigned int>(parser.basisFileName_, parser.romSize_);
  const auto numBasis = phi0.numVectors();
  if( numBasis != parser.romSize_ ) return 1;

  /*----------------------
   * CREATE DECODER
   ----------------------*/
  /* when using the sample mesh, it means that I don't have all the cells of the domain.
   * Therefore, I need to extract from the basis vectors only the entries
   * relative to the sample mes. Specifically, we want all the ROWs corresponding
   * to the sample mesh cells where we have state.
   * The decoder will then be constructed using this subset of the basis vectors.
   * To find out the rows I need to extract, I use the mesh. */

  // extract subset of basis
  const decoder_jac_t phi1 = extractSampleMeshRows(phi0, parser, appObj);

  // create decoder obj
  decoder_t decoderObj(phi1);

  /*----------------------
   * DEFINE LSPG PROBLEM
   ----------------------*/
  // start timer: we do it here because we do not count reading the basis
  auto startTime = std::chrono::high_resolution_clock::now();

  // define user-defined ops object: this is needed because
  // behind the scenes pressio does not know if you are using sample mesh or not
  using time_discrete_ops_t = time_discrete_ops<fom_t>;
  time_discrete_ops_t tdOps(appObj.viewGraph());

  // ROM state
  lspg_state_t xROM(parser.romSize_);
  xROM.putScalar(zero);

  // define LSPG type
  constexpr auto ode_case  = pressio::ode::ImplicitEnum::Euler;
  using lspg_problem_type  = pressio::rom::DefaultLSPGTypeGenerator<
    fom_t, ode_case, decoder_t, lspg_state_t, time_discrete_ops_t>;
  using lspg_stepper_t	   = typename lspg_problem_type::lspg_stepper_t;
  using lspg_generator     = pressio::rom::LSPGUnsteadyProblemGenerator<lspg_problem_type>;
  lspg_generator lspgProblem(appObj, yRef, decoderObj, xROM, zero, tdOps);

  // --- linear solver ---
  using solver_tag	   = pressio::solvers::linear::direct::HouseholderQR;
  using linear_solver_t	   = pressio::solvers::direct::EigenDirect<solver_tag, lspg_hessian_t>;
  linear_solver_t linSolverObj;

  // --- normal-equations-based GaussNewton solver ---
  using gnsolver_t   = pressio::solvers::iterative::GaussNewton<lspg_stepper_t, linear_solver_t>;
  gnsolver_t solver(lspgProblem.stepperObj_, xROM, linSolverObj);
  solver.setTolerance(1e-13);
  solver.setMaxIterations(20);

  /*----------------------
   * INTEGRATE
   ----------------------*/
  pressio::ode::integrateNSteps(lspgProblem.stepperObj_, xROM, zero,
  				parser.dt_, parser.NSteps_, solver);

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
  	    << std::fixed << std::setprecision(10)
  	    << elapsed.count() << std::endl;

  // // print summary from timers
  // #ifdef HAVE_TEUCHOS_TIMERS
  // pressio::utils::TeuchosPerformanceMonitor::stackedTimersReportSerial();
  // #endif

  /*----------------------
   * FINISH UP
   ----------------------*/
  {
    // print generalized coordinates
    std::ofstream file;
    file.open("final_generalized_coords.txt");
    for(auto i=0; i < xROM.size(); i++){
      file << std::setprecision(14) << xROM[i] << std::endl;
    }
    file.close();
  }
  {
    // print sample mesh coords
    const auto X = appObj.getX();
    const auto Y = appObj.getY();
    std::ofstream file; file.open("xy.txt");
    for(auto i=0; i < X.size(); i++){
      file << std::setprecision(14) << X[i] << " " << Y[i];
      file << std::endl;
    }
    file.close();
  }
  {
    // compute the fom corresponding to our rom final state
    const auto xFomFinal = lspgProblem.yFomReconstructor_(xROM);

    // print reconstructed fom state over sample mesh
    std::ofstream file;
    file.open("xFomReconstructed.txt");
    for(auto i=0; i < xFomFinal.size(); i++){
      file << std::setprecision(15) << xFomFinal[i] << std::endl;
    }
    file.close();
  }

  // print reconstructed fom state over full mesh
  {
    //I dont need to add the yRef because that is zero anyway
    const auto xFomFullMesh = ::pressio::containers::ops::product(phi0, xROM);
    std::ofstream file;
    file.open("xFomReconstructedFM.txt");
    for(auto i=0; i < xFomFullMesh.size(); i++){
      file << std::setprecision(15) << xFomFullMesh[i] << std::endl;
    }
    file.close();
  }

  // // // print solution
  // // std::cout << std::setprecision(14) << *x.data() << std::endl;
  return 0;
}
