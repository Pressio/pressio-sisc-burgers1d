
#include "CONTAINERS_ALL"
#include "ODE_EXPLICIT"
#include "ODE_INTEGRATORS"
#include "adr2d_kokkos.hpp"
#include "input_parser.hpp"
#include "advection_cellular_flow_functor.hpp"
#include "source_term_chemABC_functor.hpp"

int main(int argc, char *argv[]){
  Kokkos::initialize (argc, argv);
  {
    // parse input file
    InputParser parser;
    int err = parser.parse(argc, argv);
    if (err == 1) return 1;

    const auto meshFile		= parser.meshFileName_;
    const auto diffusion	= parser.D_;
    const auto chemReact	= parser.K_;
    const auto dt		= parser.dt_;
    const auto finalT		= parser.finalT_;
    const auto observerOn	= parser.observerOn_;
    const auto Nsteps		= parser.NSteps_;

    // types
    using scalar_t	= double;
    using app_t		= Adr2dKokkos<ChemistryABCSource, CellularFlow>;
    using app_state_t	= typename app_t::state_type;
    using app_velo_t	= typename app_t::velocity_type;
    using ode_state_t	= pressio::containers::Vector<app_state_t>;
    using ode_f_t	= pressio::containers::Vector<app_velo_t>;

    // get the source and advection functor types from the app
    using src_fnct_t	= typename app_t::source_functor_t;
    using adv_fnct_t    = typename app_t::advection_functor_t;

    // Record start time
    auto startTime = std::chrono::high_resolution_clock::now();

    // instantiate functors to use
    adv_fnct_t advFunctor;
    src_fnct_t srcFunctor(chemReact);

    // create app object
    app_t appObj(meshFile, srcFunctor,  advFunctor, diffusion);

    // initial state
    auto & x0n = appObj.getState();
    ode_state_t x(x0n);

    // define stepper
    constexpr auto ode_case = pressio::ode::ExplicitEnum::RungeKutta4;
    using stepper_t = pressio::ode::ExplicitStepper<
      ode_case, ode_state_t, ode_f_t, scalar_t, app_t>;
    stepper_t stepperObj(x, appObj);

    constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
    pressio::ode::integrateNSteps(stepperObj, x, zero, dt, Nsteps);

    // Record end time
    auto finishTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finishTime - startTime;
    std::cout << "Elapsed time: "
	      << std::fixed << std::setprecision(10)
	      << elapsed.count() << std::endl;

    // print final state
    using state_type_h = typename app_state_t::HostMirror;
    const auto stateSize = appObj.getStateSize();
    state_type_h xH("xH", stateSize);
    Kokkos::deep_copy(xH, *x.data());

    std::ofstream file; file.open("final_state.txt");
    for(auto i=0; i < stateSize; i++){
      file << std::setprecision(14) << xH(i) << std::endl;
    }
    file.close();

  }//end kokkos scope
  return 0;
}
