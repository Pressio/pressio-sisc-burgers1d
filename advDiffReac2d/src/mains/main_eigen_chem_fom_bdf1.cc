
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "SOLVERS_NONLINEAR"
#include "../adr2d_eigen.hpp"
#include "../input_parser.hpp"
#include "../observers/eigen_observer.hpp"
#include "../functors/source_term_functors.hpp"
#include "../functors/advection_field_functors.hpp"

int main(int argc, char *argv[]){

  // parse input file
  InputParser parser;
  int err = parser.parse(argc, argv);
  if (err == 1) return 1;

  const auto meshFile	= parser.meshFileName_;
  const auto diffusion	= parser.D_;
  const auto chemReact	= parser.K_;
  const auto dt		= parser.dt_;
  const auto finalT	= parser.finalT_;
  const auto observerOn = parser.observerOn_;
  const auto Nsteps	= parser.numSteps_;

  // types
  using scalar_t	= double;
  using src_fnct_t	= ChemistryABCSource<void, scalar_t>;
  using adv_fnct_t	= CellularFlow<void, scalar_t>;
  using app_t		= Adr2dEigen<src_fnct_t, adv_fnct_t>;
  using app_state_t	= typename app_t::state_type;
  using app_velo_t	= typename app_t::velocity_type;
  using app_jacob_t	= typename app_t::jacobian_type;
  using ode_state_t	= pressio::containers::Vector<app_state_t>;
  using ode_r_t		= pressio::containers::Vector<app_velo_t>;
  using ode_j_t		= pressio::containers::Matrix<app_jacob_t>;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  // functors to use
  adv_fnct_t advFunctor;
  src_fnct_t srcFunctor(chemReact);

  // create app object
  app_t appObj(meshFile, srcFunctor, advFunctor, diffusion);

  // initial state
  auto & x0n = appObj.getState();
  ode_state_t x(x0n);

  // define stepper
  constexpr auto ode_case = pressio::ode::ImplicitEnum::Euler;
  using stepper_t = pressio::ode::ImplicitStepper<
    ode_case, ode_state_t, ode_r_t, ode_j_t, app_t>;
  stepper_t stepperObj(x, appObj);

  // define solver
  using lin_solver_t = pressio::solvers::iterative::EigenIterative<
    pressio::solvers::linear::iterative::LSCG, ode_j_t>;
  pressio::solvers::NewtonRaphson<scalar_t, lin_solver_t> solverO;
  solverO.setMaxIterations(10);
  // by default, newton raphson exits when norm of correction is below tolerance
  solverO.setTolerance(1e-13);

  // integrate in time
  if (observerOn == 1){
    const auto stateSize = appObj.getStateSize();
    using obs_t = EigenObserver<ode_state_t>;
    obs_t Obs(Nsteps, stateSize, x, parser.snapshotsFreq_);
    pressio::ode::integrateNSteps(stepperObj, x, zero, dt, Nsteps, Obs, solverO);

    // print snapshots to file
    Obs.printSnapshotsToFile(parser.shapshotsFileName_);

    // print mesh
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

    // compute SVD
    const auto & S = Obs.viewSnapshots();
    // do SVD and compute basis
    Eigen::JacobiSVD<typename obs_t::matrix_t> svd(S, Eigen::ComputeThinU);
    const auto U = svd.matrixU();

    std::cout << "Print basis to file" << std::endl;
    std::ofstream file;
    file.open(parser.basisFileName_);
    for (auto i=0; i<U.rows(); i++){
      for (auto j=0; j<U.cols(); j++){
  	file << std::fixed << std::setprecision(15) << U(i,j) << " ";
      }
      file << std::endl;
    }
    file.close();
    std::cout << "Done with basis" << std::endl;
  }
  else{
    pressio::ode::integrateNSteps(stepperObj, x, zero, dt, Nsteps, solverO);
  }

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
	    << std::fixed << std::setprecision(10)
	    << elapsed.count() << std::endl;

  // // print solution
  // std::cout << std::setprecision(14) << *x.data() << std::endl;

  return 0;
}
