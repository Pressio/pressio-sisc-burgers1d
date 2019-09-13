
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "SOLVERS_NONLINEAR"
#include "adr2d_eigen.hpp"
#include "input_parser.hpp"
#include "eigen_observer.hpp"
#include "advection_cellular_flow_functor.hpp"
#include "source_term_chemABC_functor.hpp"
#include "advection_manuf_sol_functor.hpp"
#include "source_term_manuf_sol_functor.hpp"

struct MSTypes{
  using scalar_t	= double;

  using src_fnct_t	= ManufacturedSolutionSource<scalar_t>;
  using adv_fnct_t	= ManufacturedSolutionAdvection<scalar_t>;

  using app_t		= Adr2dEigen<src_fnct_t, adv_fnct_t>;
  using app_state_t	= typename app_t::state_type;
  using app_velo_t	= typename app_t::velocity_type;
  using app_jacob_t	= typename app_t::jacobian_type;
  using ode_state_t	= pressio::containers::Vector<app_state_t>;
  using ode_f_t		= pressio::containers::Vector<app_velo_t>;
  using ode_r_t		= pressio::containers::Vector<app_velo_t>;
  using ode_j_t		= pressio::containers::Matrix<app_jacob_t>;

  using obs_t		= EigenObserver<ode_state_t>;
};


struct ChemTypes{
  using scalar_t	= double;

  using src_fnct_t	= ChemistryABCSource<scalar_t>;
  using adv_fnct_t	= CellularFlow<scalar_t>;

  using app_t		= Adr2dEigen<src_fnct_t, adv_fnct_t>;
  using app_state_t	= typename app_t::state_type;
  using app_velo_t	= typename app_t::velocity_type;
  using app_jacob_t	= typename app_t::jacobian_type;
  using ode_state_t	= pressio::containers::Vector<app_state_t>;
  using ode_f_t		= pressio::containers::Vector<app_velo_t>;
  using ode_r_t		= pressio::containers::Vector<app_velo_t>;
  using ode_j_t		= pressio::containers::Matrix<app_jacob_t>;

  using obs_t		= EigenObserver<ode_state_t>;
};


template <typename app_t>
void printMeshCoordsToFile(const app_t & appObj){
  const auto X = appObj.getX();
  const auto Y = appObj.getY();
  std::ofstream file; file.open("xy.txt");
  for(auto i=0; i < X.size(); i++){
    file << std::setprecision(14) << X[i] << " " << Y[i];
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



template <typename pt>
void runBDF1(const InputParser & parser,
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
  constexpr auto ode_case = pressio::ode::ImplicitEnum::Euler;
  using stepper_t = pressio::ode::ImplicitStepper<
    ode_case, typename pt::ode_state_t, typename pt::ode_r_t,
    typename pt::ode_j_t, typename pt::app_t>;
  stepper_t stepperObj(x, appObj);

  // define linear solver
  // using lin_solver_t = pressio::solvers::direct::EigenDirect<
  //   pressio::solvers::linear::direct::ColPivHouseholderQR, typename pt::ode_j_t>;
  using lin_solver_t = pressio::solvers::iterative::EigenIterative<
    pressio::solvers::linear::iterative::Bicgstab, typename pt::ode_j_t>;
  lin_solver_t linearSolverObj;

  // define non-linear solver
  using non_lin_solver_t = pressio::solvers::NewtonRaphson<scalar_t, lin_solver_t>;
  non_lin_solver_t solverObj(linearSolverObj);
  // by default, newton raphson exits when norm of correction is below tolerance
  solverObj.setMaxIterations(10);
  solverObj.setTolerance(1e-13);

  // integrate in time
  if (parser.observerOn_ == 1)
  {
    // create observer
    typename pt::obs_t Obs(parser.NSteps_, stateSize, x, parser.snapshotsFreq_);

    // do time integration
    pressio::ode::integrateNSteps(stepperObj, x, zero,
				  parser.dt_, parser.NSteps_, Obs,
				  solverObj);

    // print snapshots to file
    Obs.printSnapshotsToFile(parser.shapshotsFileName_);

    // print mesh
    printMeshCoordsToFile(appObj);

    // do SVD
    doSVD(Obs, parser.basisFileName_);
  }
  else{
    pressio::ode::integrateNSteps(stepperObj, x, zero,
				  parser.dt_, parser.NSteps_,
				  solverObj);
  }

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
	    << std::fixed << std::setprecision(10)
	    << elapsed.count() << std::endl;

}//end runBDF1



int main(int argc, char *argv[]){

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
    if (odeStepperName.compare("bdf1") == 0 ){
      runBDF1<pt>(parser, srcFunctor, advFunctor);
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
    if (odeStepperName.compare("bdf1") == 0 ){
      runBDF1<pt>(parser, srcFunctor, advFunctor);
    }
  }
  else{
    std::cout << "Invalid problemName in input file " << std::endl;
  }

  return 0;
}
