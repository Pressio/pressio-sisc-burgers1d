
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "ROM_GALERKIN"
#include "utils.hpp"
#include "burgers1d_input_parser.hpp"
#include "burgers1d_eigen.hpp"
#include "eigen_observer.hpp"

int main(int argc, char *argv[]){
  using fom_t		= Burgers1dEigen;
  using sc_t		= typename fom_t::scalar_type;

  using eig_dyn_vec	= Eigen::Matrix<sc_t, -1, 1>;
  using rom_state_t	= pressio::containers::Vector<eig_dyn_vec>;

  using native_state_t	= typename fom_t::state_type;
  using native_dmat_t	= typename fom_t::dense_matrix_type;
  using decoder_jac_t	= pressio::containers::MultiVector<native_dmat_t>;
  using decoder_t	= pressio::rom::LinearDecoder<decoder_jac_t>;

  constexpr auto zero = ::pressio::utils::constants::zero<sc_t>();
  constexpr auto one  = ::pressio::utils::constants::one<sc_t>();

  // parse input file
  InputParser parser;
  int32_t err = parser.parse(argc, argv);
  if (err == 1) return 1;

  // app object
  fom_t appobj(parser.numCell_);

  // store modes computed before from file
  // store basis vectors into native format
  const auto phiNative = readBasis<sc_t, int32_t, native_dmat_t>(parser.basisFileName_, parser.romSize_);

  // wrap native basis with a pressio wrapper
  const decoder_jac_t phi(phiNative);
  const int32_t numBasis = phi.numVectors();
  if( numBasis != parser.romSize_ ) return 0;

  // create decoder obj
  decoder_t decoderObj(phi);

  // the reference state = initial condition
  native_state_t yRef(parser.numCell_);
  yRef.setConstant(one);

  // define ROM state
  rom_state_t yROM(parser.romSize_);
  yROM.putScalar(zero);

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();
  // define ROM problem
  constexpr auto ode_case  = pressio::ode::ExplicitEnum::RungeKutta4;
  using galerkin_t = pressio::rom::galerkin::DefaultProblemType<
    ode_case, rom_state_t, fom_t, decoder_t>;
  using galerkin_prob = pressio::rom::galerkin::ProblemGenerator<galerkin_t>;
  galerkin_prob galerkinProb(appobj, yRef, decoderObj, yROM, zero);

  // get stepper object
  auto & stepper = galerkinProb.getStepperRef();

  // integrate in time
  pressio::ode::integrateNSteps(stepper, yROM, zero,
  				parser.dt_, parser.numSteps_);

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
  	    << std::fixed << std::setprecision(10)
  	    << elapsed.count() << std::endl;

  // compute the fom corresponding to our rom final state
  const auto yFomFinal = galerkinProb.getFomStateReconstructorCRef()(yROM);
  printEigenVectorToFile("yFomReconstructed.txt", *yFomFinal.data());

  // print generalized coords
  printEigenVectorToFile("final_generalized_coords.txt", *yROM.data());

  return 0;
}
