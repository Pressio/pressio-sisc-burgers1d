
#include "CONTAINERS_ALL"
#include "ODE_EXPLICIT"
#include "ODE_INTEGRATORS"
#include "adr2d_kokkos.hpp"
#include "input_parser.hpp"
#include "kokkos_observer.hpp"
#include "advection_cellular_flow_functor.hpp"
#include "source_term_chemABC_functor.hpp"
#include "advection_manuf_sol_functor.hpp"
#include "source_term_manuf_sol_functor.hpp"


struct MSTypes{
  using scalar_t	= double;

  using app_t		= Adr2dKokkos<ManufacturedSolutionSource, ManufacturedSolutionAdvection>;

  // get the source and advection functor types from the app
  using src_fnct_t	= typename app_t::source_functor_t;
  using adv_fnct_t	= typename app_t::advection_functor_t;

  using app_state_t	= typename app_t::state_type;
  using app_velo_t	= typename app_t::velocity_type;
  //using app_jacob_t	= typename app_t::jacobian_type;
  using ode_state_t	= pressio::containers::Vector<app_state_t>;
  using ode_f_t		= pressio::containers::Vector<app_velo_t>;
  using ode_r_t		= pressio::containers::Vector<app_velo_t>;
  //using ode_j_t		= pressio::containers::Matrix<app_jacob_t>;

  using obs_t		= KokkosObserver<ode_state_t>;
};


struct ChemTypes{
  using scalar_t	= double;

  using app_t		= Adr2dKokkos<ChemistryABCSource, CellularFlow>;

  // get the source and advection functor types from the app
  using src_fnct_t	= typename app_t::source_functor_t;
  using adv_fnct_t	= typename app_t::advection_functor_t;

  using app_state_t	= typename app_t::state_type;
  using app_velo_t	= typename app_t::velocity_type;
  //using app_jacob_t	= typename app_t::jacobian_type;
  using ode_state_t	= pressio::containers::Vector<app_state_t>;
  using ode_f_t		= pressio::containers::Vector<app_velo_t>;
  using ode_r_t		= pressio::containers::Vector<app_velo_t>;
  //using ode_j_t		= pressio::containers::Matrix<app_jacob_t>;

  using obs_t		= KokkosObserver<ode_state_t>;
};


template <typename app_t>
void printMeshCoordsToFile(const app_t & appObj){
  const auto coords = appObj.getCoordsHost();
  std::ofstream file; file.open("xy.txt");
  for(auto i=0; i < coords.extent(0); i++){
    file << std::setprecision(14) << coords(i,0) << " " << coords(i,1);
    file << std::endl;
  }
  file.close();
}


template <typename obs_t>
void doSVD(const obs_t & Obs,
	   std::string basisFileName)
{
  // compute SVD
  const auto & S = Obs.viewSnapshots();
  // do SVD and compute basis
  Eigen::JacobiSVD<typename obs_t::matrix_t> svd(S, Eigen::ComputeThinU);
  const auto U = svd.matrixU();

  std::cout << "Print basis to file" << std::endl;
  std::ofstream file; file.open(basisFileName);
  for (auto i=0; i<U.rows(); i++){
    for (auto j=0; j<U.cols(); j++){
      file << std::fixed << std::setprecision(15) << U(i,j) << " ";
    }
    file << std::endl;
  }
  file.close();
  std::cout << "Done with basis" << std::endl;
}


template <typename pt>
void runRungeKutta4(const InputParser & parser,
		    const typename pt::src_fnct_t & srcFunctor,
		    const typename pt::adv_fnct_t & advFunctor)
{
  using scalar_t = typename pt::scalar_t;
  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();

  // create app object
  typename pt::app_t appObj(parser.meshFileName_, srcFunctor, advFunctor, parser.D_);
  const auto stateSize = appObj.getStateSize();
  const auto & x0n = appObj.getState();

  // initial state
  typename pt::ode_state_t x(x0n);

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  // define stepper
  constexpr auto ode_case = pressio::ode::ExplicitEnum::RungeKutta4;
  using stepper_t = pressio::ode::ExplicitStepper<
    ode_case, typename pt::ode_state_t, typename pt::ode_f_t,
    typename pt::scalar_t, typename pt::app_t>;
  stepper_t stepperObj(x, appObj);

  // integrate in time
  if (parser.observerOn_ == 1)
  {
    // create observer
    typename pt::obs_t Obs(parser.NSteps_, stateSize, x, parser.snapshotsFreq_);

    // do time integration
    pressio::ode::integrateNSteps(stepperObj, x, zero,
				  parser.dt_, parser.NSteps_, Obs);

    // print snapshots to file
    Obs.printSnapshotsToFile(parser.shapshotsFileName_);

    // print mesh
    printMeshCoordsToFile(appObj);

    // do SVD
    doSVD(Obs, parser.basisFileName_);
  }
  else{
    pressio::ode::integrateNSteps(stepperObj, x, zero,
				  parser.dt_, parser.NSteps_);
  }

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
	    << std::fixed << std::setprecision(10)
	    << elapsed.count() << std::endl;
}//end runRK4



int main(int argc, char *argv[]){
  std::cout << "Starting Kokkos::initialize" << std::endl;
  Kokkos::initialize (argc, argv);
  {
    // parse input file
    InputParser parser;
    int err = parser.parse(argc, argv);
    if (err == 1) return 1;

    // get the problem name and stepper
    const auto problemName    = parser.problemName_;
    const auto odeStepperName = parser.odeStepperName_;

    if ( problemName.compare("ms") == 0 ){
      // types
      using pt = MSTypes;

      // functors to use
      typename pt::adv_fnct_t advFunctor;
      typename pt::src_fnct_t srcFunctor(parser.D_);

      if (odeStepperName.compare("RungeKutta4") == 0 ){
	runRungeKutta4<pt>(parser, srcFunctor, advFunctor);
      }
    }
    else if ( problemName.compare("chemABC") == 0 ){
      // types
      using pt = ChemTypes;

      // functors to use
      typename pt::adv_fnct_t advFunctor;
      typename pt::src_fnct_t srcFunctor(parser.K_);

      if (odeStepperName.compare("RungeKutta4") == 0){
    	runRungeKutta4<pt>(parser, srcFunctor, advFunctor);
      }
    }
    else{
      std::cout << "Invalid problemName in input file " << std::endl;
    }
  }//end kokkos initialize

  return 0;
}
