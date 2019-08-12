
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "../adr2d_eigen.hpp"
#include "../input_parser.hpp"
#include "../observers/eigen_observer.hpp"
#include "../functors/source_term_functors.hpp"
#include "../functors/advection_field_functors.hpp"

// constexpr bool do_print = true;

// struct Observer{
//   Observer() = default;

//   template <typename T>
//   void operator()(size_t step, double t, const T & y)
//   {
//     if (do_print){

//       if (step % 20 == 0){
//     	std::ofstream file;
//     	file.open( "sol_" + std::to_string(step) + ".txt" );
//     	for(auto i=0; i < y.size(); i++){
//     	  file << std::fixed << std::setprecision(14) << y[i] ;
//     	  file << std::endl;
//     	}
//     	file.close();
//        }
//     }
//   }
// };

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
  const auto Nsteps	= static_cast<unsigned int>(finalT/dt);

  // types
  using scalar_t	= double;
  using src_fnct_t	= ManufacturedSolutionSource<void, scalar_t>;
  using adv_fnct_t	= ManufacturedSolutionAdvection<void, scalar_t>;
  using app_t		= Adr2dEigen<src_fnct_t, adv_fnct_t>;
  using app_state_t	= typename app_t::state_type;
  using app_velo_t	= typename app_t::velocity_type;
  using ode_state_t	= pressio::containers::Vector<app_state_t>;
  using ode_f_t		= pressio::containers::Vector<app_velo_t>;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  // instantiate functors for advection and source term
  adv_fnct_t advFunctor;
  src_fnct_t srcFunctor(diffusion);

  // create app object
  app_t appObj(meshFile, srcFunctor,  advFunctor, diffusion);

  // initial state and wrapper of it
  auto & x0n = appObj.getState();
  ode_state_t x(x0n);

  // if (do_print){
  //   auto X = appObj.getX();
  //   auto Y = appObj.getY();
  //   std::ofstream file; file.open( "xy.txt" );
  //   for(auto i=0; i < X.size(); i++){
  //     file << std::setprecision(14)
  // 	   << X[i] << " " << Y[i];
  //     file << std::endl;
  //   }
  //   file.close();
  // }

  // define stepper
  constexpr auto ode_case = pressio::ode::ExplicitEnum::RungeKutta4;
  using stepper_t = pressio::ode::ExplicitStepper<
    ode_case, ode_state_t, ode_f_t, scalar_t, app_t>;
  stepper_t stepperObj(x, appObj);

  // Observer Obs;
  // pressio::ode::integrateNSteps(stepperObj, x, zero, dt, Nsteps, Obs);

  // // integrate in time
  // if (observerOn == 1){
  //   //construct observer, pass init condition
  //   using obs_t = EigenObserver<ode_state_t>;
  //   obs_t Obs(Nsteps, numCell, x, parser.snapshotsFreq_);
  //   pressio::ode::integrateNSteps(stepperObj, x, zero, dt, Nsteps, Obs);

  //   // print snapshots to file
  //   Obs.printSnapshotsToFile(parser.shapshotsFileName_);

  //   // compute SVD
  //   const auto & S = Obs.viewSnapshots();
  //   // do SVD and compute basis
  //   Eigen::JacobiSVD<typename obs_t::matrix_t> svd(S, Eigen::ComputeThinU);
  //   const auto U = svd.matrixU();

  //   std::cout << "Print basis to file" << std::endl;
  //   std::ofstream file;
  //   file.open(parser.basisFileName_);
  //   for (auto i=0; i<U.rows(); i++){
  //     for (auto j=0; j<U.cols(); j++){
  // 	file << std::fixed << std::setprecision(15) << U(i,j) << " ";
  //     }
  //     file << std::endl;
  //   }
  //   file.close();
  //   std::cout << "Done with basis" << std::endl;
  // }
  // else{
  //   pressio::ode::integrateNSteps(stepperObj, x, zero, dt, Nsteps);
  // }

  // // Record end time
  // auto finishTime = std::chrono::high_resolution_clock::now();
  // std::chrono::duration<double> elapsed = finishTime - startTime;
  // std::cout << "Elapsed time: " <<
  //   std::fixed << std::setprecision(10) <<
  //   elapsed.count() << std::endl;

  // // print solution
  // std::cout << std::setprecision(14) << *x.data() << std::endl;

  return 0;
}
