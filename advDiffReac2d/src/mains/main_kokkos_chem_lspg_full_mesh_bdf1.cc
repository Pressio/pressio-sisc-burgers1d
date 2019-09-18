
// pressio headers
#include "UTILS_ALL"
#include "CONTAINERS_ALL"
#include "ODE_IMPLICIT"
#include "ODE_INTEGRATORS"
#include "SOLVERS_NONLINEAR"
#include "ROM_LSPG"
// local headers
#include "adr2d_kokkos.hpp"
#include "input_parser.hpp"
#include "advection_cellular_flow_functor.hpp"
#include "source_term_chemABC_functor.hpp"
#include "kokkos_utils.hpp"

int main(int argc, char *argv[]){
Kokkos::initialize (argc, argv);
{
  /*----------------------
   * PARSER
   ----------------------*/
  InputParser parser;
  int err = parser.parse(argc, argv);
  if (err == 1) return 1;

  const auto romSize	= parser.romSize_;
  const auto basisFileName = parser.basisFileName_;

  /*----------------------
   * TYPES
   ----------------------*/
  using scalar_t	  = double;

  using fom_t		  = Adr2dKokkos<ChemistryABCSource, CellularFlow>;

  // get the source and advection functor types from the app
  using src_fnct_t	  = typename fom_t::source_functor_t;
  using adv_fnct_t	  = typename fom_t::advection_functor_t;

  // grab the native types for both device(d) and host(h)
  using native_state_d_t  = typename fom_t::state_type_d_t;
  using native_state_h_t  = typename fom_t::state_type_h_t;
  using native_mv_d_t     = typename fom_t::mv_d_t;
  using native_mv_h_t	  = typename fom_t::mv_h_t;

  // the lspg state type on the DEVICE(d)
  using lspg_state_d_t	  = pressio::containers::Vector<native_state_d_t>;

  // decoder jacobian types for both device(d) and host(h)
  using decoder_jac_d_t   = pressio::containers::MultiVector<native_mv_d_t>;
  using decoder_jac_h_t	  = pressio::containers::MultiVector<native_mv_h_t>;
  // device decoder type
  using decoder_d_t	  = pressio::rom::LinearDecoder<decoder_jac_d_t>;

  // the hessian is used only if LSPG is solved using NormalEquations
  using lspg_hessian_t  = pressio::containers::Matrix<native_mv_d_t>;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();

  /*----------------------
   * CREATE APP OBJECT
   ----------------------*/
  // functors for advection and source
  adv_fnct_t advFunctor;
  src_fnct_t srcFunctor(parser.K_);

  // app object
  fom_t appObj(parser.meshFileName_, srcFunctor, advFunctor, parser.D_);
  // get the number of total dofs
  const auto stateSize = appObj.getStateSize();

  // the device reference state = all zeros
  // just need to initialize, since by default kokkos zeros out
  native_state_d_t yRef("yRef", stateSize);

  /*----------------------
   * CREATE LINEAR DECODER
   ----------------------*/
  // create the device decoder jacobian (i.e. the basis)
  decoder_jac_d_t phi("phi", stateSize, romSize);
  // fill the device basis
  readBasis(basisFileName, romSize, *phi.data());
  const auto numBasis = phi.numVectors();
  if( numBasis != romSize ) return 0;

  // create decoder obj
  decoder_d_t decoderObj(phi);

  /*----------------------
   * DEFINE LSPG PROBLEM
   ----------------------*/
  // start timer: we do it here because we do not count reading the basis
  auto startTime = std::chrono::high_resolution_clock::now();

  // device ROM state: this is zero initialized, like we want.
  lspg_state_d_t xROM("xROM", romSize);

  // define LSPG type
  constexpr auto ode_case  = pressio::ode::ImplicitEnum::Euler;
  using lspg_problem_type  = pressio::rom::DefaultLSPGTypeGenerator<
  				fom_t, ode_case, decoder_d_t, lspg_state_d_t>;
  using lspg_stepper_t	   = typename lspg_problem_type::lspg_stepper_t;
  using lspg_generator	   = pressio::rom::LSPGUnsteadyProblemGenerator<lspg_problem_type>;
  lspg_generator lspgProblem(appObj, yRef, decoderObj, xROM, zero);

  // --- linear solver ---
  using solver_tag	   = pressio::solvers::linear::direct::getrs;
  using linear_solver_t	   = pressio::solvers::direct::KokkosDirect<solver_tag, lspg_hessian_t>;
  linear_solver_t linSolverObj;

  // --- normal-equations-based GaussNewton solver ---
  using gnsolver_t   = pressio::solvers::iterative::GaussNewton<lspg_stepper_t,	linear_solver_t>;
  gnsolver_t solver(lspgProblem.stepperObj_, xROM, linSolverObj);
  solver.setTolerance(1e-13);
  solver.setMaxIterations(10);

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

  /*----------------------
   * FINISH UP
   ----------------------*/
  {
    // create a host mirror for the device ROM coords
    native_state_h_t xROMh("xROMh", romSize);
    Kokkos::deep_copy(xROMh, *xROM.data());

    // print generalized coordinates
    std::ofstream file;
    file.open("final_generalized_coords.txt");
    for(auto i=0; i < romSize; i++){
      file << std::setprecision(14) << xROMh(i) << std::endl;
    }
    file.close();
  }
  {
    // reconstruct the FOM state on the device using the final generalized coordinates
    const auto xFomFinal_d = lspgProblem.yFomReconstructor_(xROM);

    // create a host mirror for the device reconstructed FOM state
    native_state_h_t xH("xH", stateSize);
    Kokkos::deep_copy(xH, *xFomFinal_d.data());

    // print reconstructed host fom state
    std::ofstream file;
    file.open("xFomReconstructed.txt");
    for(auto i=0; i < stateSize; i++){
      file << std::setprecision(15) << xH(i) << std::endl;
    }
    file.close();
  }

}//end kokkos scope
return 0;
}
