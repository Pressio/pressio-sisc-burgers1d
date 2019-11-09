
#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "ROM_GALERKIN"
#include "utils.hpp"
#include "burgers1d_input_parser.hpp"
#include "burgers1d_eigen.hpp"
#include "eigen_observer.hpp"

int main(int argc, char *argv[]){
  using fom_t		= Burgers1dEigen;
  using scalar_t	= typename fom_t::scalar_type;

  using eig_dyn_vec	= Eigen::Matrix<scalar_t, -1, 1>;
  using rom_state_t	= pressio::containers::Vector<eig_dyn_vec>;

  using native_state_t	= typename fom_t::state_type;
  using native_dmat_t	= typename fom_t::dmatrix_type;
  using decoder_jac_t	= pressio::containers::MultiVector<native_dmat_t>;
  using decoder_t	= pressio::rom::LinearDecoder<decoder_jac_t>;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  // parse input file
  InputParser parser;
  int32_t err = parser.parse(argc, argv);
  if (err == 1) return 1;

  // app object
  fom_t appobj(parser.numCell_);

  // store modes computed before from file
  // store basis vectors into native format
  const auto phiNative = readBasis<scalar_t, int32_t, native_dmat_t>(parser.basisFileName_,
  								     parser.romSize_);

  //const native_dmat_t phiNative(parser.numCell_, parser.romSize_);
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
  yROM.putScalar(0.);

  //-------------------------------------------------------------
  yROM.putScalar(1.);
  int32_t nRounds = 1;
  auto startTime = std::chrono::high_resolution_clock::now();
  ::pressio::containers::Vector<native_state_t> yFom(parser.numCell_); yFom.putScalar(1.);
  ::pressio::containers::Vector<native_state_t> f(parser.numCell_); f.putScalar(0.);
  for (auto i=0; i<nRounds; ++i){
    //*yROM.data() = phi.data()->transpose() * (*yFom.data());
    decoderObj.applyMapping(yROM, yFom);
    //appobj.velocity(*yFom.data(), 0.0, *f.data());
    //appobj.velocityEmpty(*yFom.data(), 0.0, *f.data());
  }

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
  	    << std::fixed << std::setprecision(10)
  	    << elapsed.count() << std::endl;

  std::cout << "time per iter: "
  	    << std::fixed << std::setprecision(10)
  	    << elapsed.count()/(double)nRounds << std::endl;

    std::ofstream file;
    file.open("yFomReconstructed.txt");
    for(size_t i=0; i < yFom.size(); i++){
      file << std::setprecision(15) << yFom[i] << std::endl;
    }
    file.close();

  //-------------------------------------------------------------

  // // Record start time
  // auto startTime = std::chrono::high_resolution_clock::now();
  // // define ROM problem
  // constexpr auto ode_case  = pressio::ode::ExplicitEnum::RungeKutta4;
  // using galerkin_t = pressio::rom::DefaultGalerkinExplicitTypeGenerator<ode_case, rom_state_t, fom_t, decoder_t>;
  // using galerkin_prob = pressio::rom::GalerkinProblemGenerator<galerkin_t>;
  // galerkin_prob galerkinProb(appobj, yRef, decoderObj, yROM, zero);

  // // get stepper object
  // auto & stepper = galerkinProb.getStepperRef();

  // // integrate in time
  // pressio::ode::integrateNSteps(stepper, yROM, zero, parser.dt_, parser.numSteps_);

  // // Record end time
  // auto finishTime = std::chrono::high_resolution_clock::now();
  // std::chrono::duration<double> elapsed = finishTime - startTime;
  // std::cout << "Elapsed time: "
  // 	    << std::fixed << std::setprecision(10)
  // 	    << elapsed.count() << std::endl;
  // {
  //   // compute the fom corresponding to our rom final state
  //   const auto yFomFinal = galerkinProb.getFomStateReconstructorCRef()(yROM);
  //   std::ofstream file;
  //   file.open("yFomReconstructed.txt");
  //   for(size_t i=0; i < yFomFinal.size(); i++){
  //     file << std::setprecision(15) << yFomFinal[i] << std::endl;
  //   }
  //   file.close();
  // }
  // {
  //   // print generalized coords
  //   std::ofstream file;
  //   file.open("final_generalized_coords.txt");
  //   for(size_t i=0; i < yROM.size(); i++){
  //     file << std::setprecision(17) << yROM[i] << std::endl;
  //   }
  //   file.close();
  // }

  return 0;
}
