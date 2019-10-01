
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "ROM_GALERKIN"
#include <chrono>
#include "utils.hpp"
#include "burgers1d_input_parser.hpp"
#include "burgers1d_eigen.hpp"
#include "eigen_observer.hpp"

int main(int argc, char *argv[]){
  using fom_t		= Burgers1dEigen;
  using scalar_t	= typename fom_t::scalar_type;

  using eig_dyn_vec	= Eigen::Matrix<scalar_t, -1, 1>;
  using rom_state_t	= pressio::containers::Vector<eig_dyn_vec>;

  using eig_dyn_mat	= Eigen::Matrix<scalar_t, -1, -1>;
  using decoder_jac_t	= pressio::containers::MultiVector<eig_dyn_mat>;
  using decoder_t	= pressio::rom::LinearDecoder<decoder_jac_t>;

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

  // app object
  fom_t appobj(numCell);

  // store modes computed before from file
  // store basis vectors into native format
  const auto phiNative = readBasis<scalar_t, int32_t>(basisFileName, romSize);
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
  rom_state_t yROM(romSize);
  yROM.putScalar(zero);

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  // initial time
  constexpr auto t0 = zero;

  // define ROM problem
  constexpr auto ode_case  = pressio::ode::ExplicitEnum::RungeKutta4;
  using galerkin_t = pressio::rom::DefaultGalerkinExplicitTypeGenerator<
    ode_case, rom_state_t, fom_t, decoder_t>;
  pressio::rom::GalerkinProblemGenerator<galerkin_t> galerkinProb(
      appobj, yRef, decoderObj, yROM, t0);

  // integrate in time
  pressio::ode::integrateNSteps(galerkinProb.stepperObj_, yROM, t0, dt, Nsteps);

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: " <<
    std::fixed << std::setprecision(10) <<
    elapsed.count() << std::endl;

  std::cout << "Printing first 5 elements of gen coords" << std::endl;
  for (int32_t i=0; i<5; ++i)
    std::cout << (*yROM.data())[i] << std::endl;

  // // compute the fom corresponding to our rom final state
  // auto yFomFinal = romProblem.yFomReconstructor_(yROM);
  // std::cout << *yFomFinal.data() << std::endl;

  return 0;
}
