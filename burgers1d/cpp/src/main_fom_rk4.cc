
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "burgers1d_eigen.hpp"
#include "burgers1d_input_parser.hpp"
#include "eigen_observer.hpp"
#include <chrono>

int main(int argc, char *argv[]){
  // type aliasing
  using app_t		= Burgers1dEigen;
  using scalar_t	= typename app_t::scalar_type;
  using app_state_t	= typename app_t::state_type;
  using ode_state_t	= pressio::containers::Vector<app_state_t>;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  // parse input file
  InputParser parser;
  int err = parser.parse(argc, argv);
  if (err == 1) return 1;

  // store cmd line args
  const auto numCell = parser.numCell_;
  const auto dt = parser.dt_;
  const auto finalT = parser.finalT_;
  const auto observerOn = parser.observerOn_;
  const auto Nsteps = static_cast<unsigned int>(finalT/dt);

  // create app object
  app_t appObj(numCell);

  // initial state
  app_state_t x0n(numCell);
  x0n.setConstant(one);
  // wrap init state
  ode_state_t x(x0n);

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  // define stepper
  constexpr auto ode_case = pressio::ode::ExplicitEnum::RungeKutta4;
  using stepper_t = pressio::ode::ExplicitStepper<ode_case, ode_state_t, scalar_t, app_t>;
  stepper_t stepperObj(x, appObj);

  // integrate in time
  if (observerOn == 1){
    // construct observer, pass y which now contains init condition
    using obs_t = EigenObserver<ode_state_t>;
    obs_t Obs(Nsteps, numCell, x, parser.snapshotsFreq_);
    pressio::ode::integrateNSteps(stepperObj, x, zero, dt, Nsteps, Obs);

    // print snapshots to file
    Obs.printSnapshotsToFile(parser.shapshotsFileName_);

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
    pressio::ode::integrateNSteps(stepperObj, x, zero, dt, Nsteps);
  }

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: " <<
    std::fixed << std::setprecision(10) <<
    elapsed.count() << std::endl;

  // // print solution
  // std::cout << std::setprecision(14) << *x.data() << std::endl;

  return 0;
}
