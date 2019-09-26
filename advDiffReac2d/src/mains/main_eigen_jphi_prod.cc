
// local headers
#include "adr2d_eigen.hpp"
#include "input_parser.hpp"
#include "advection_cellular_flow_functor.hpp"
#include "source_term_chemABC_functor.hpp"
#include "eigen_utils.hpp"
#include "eigen_observer.hpp"

int main(int argc, char *argv[]){

  /*----------------------
   * PARSER
   ----------------------*/
  InputParser parser;
  int err = parser.parse(argc, argv);
  if (err == 1) return 1;

  /*----------------------
   * TYPES
   ----------------------*/
  using scalar_t	= double;

  using src_fnct_t	= ChemistryABCSource<scalar_t>;
  using adv_fnct_t	= CellularFlow<scalar_t>;

  using fom_t		= Adr2dEigen<src_fnct_t, adv_fnct_t>;
  using native_state_t  = typename fom_t::state_type;
  using native_dmat_t	= typename fom_t::dmatrix_type;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  std::cout << "Eigen threads=" << Eigen::nbThreads() << std::endl;

  /*----------------------
   * CREATE APP OBJECT
   ----------------------*/
  // here we do everything as if we were setting up a real rom problem,
  // but then we use the Jacobian and basis matrix to repeat mat-mat prodducts
  // to test the performacne

  adv_fnct_t advFunctor;
  src_fnct_t srcFunctor(parser.K_);

  // app object
  fom_t appObj(parser.meshFileName_, srcFunctor, advFunctor, parser.D_);
  // get the number of total dofs
  const auto stateSize = appObj.getStateSize();

  native_state_t yRef(stateSize);
  // set to one, just for fun
  yRef.setConstant(one);

  /*----------------------
   * CREATE RANDOM basis vectors
   * since we do not care about numbers here
   ----------------------*/
  native_dmat_t phi(stateSize, parser.romSize_);
  phi = native_dmat_t::Random(phi.rows(), phi.cols());
  const auto numBasis = phi.cols();
  if( numBasis != parser.romSize_ ) return 1;

  // start timer: we do it here because we do not count reading the basis
  auto startTime = std::chrono::high_resolution_clock::now();

  // native_dmat_t phi2( phi.cols(), 200);
  // phi2 = native_dmat_t::Random(phi.cols(), 200);
  // native_dmat_t C( phi.rows(), phi2.cols() );
  // for (auto i=0; i<20; ++i){
  //   std::cout << i << std::endl;
  //   C = phi * phi2;
  // }

  // native_dmat_t A( 4000, 1000 );
  // native_dmat_t B( 1000, 500 );
  // native_dmat_t C( 4000, 500 );
  // for (auto i=0; i<20; ++i){
  //   std::cout << i << std::endl;
  //   C = A * B;
  // }

  // do product of Jacobian times phi, i.e. sparse time dense
  // this is how it is done inside the app
  const auto JJ = appObj.jacobian(yRef, zero);
  native_dmat_t A( JJ.rows(), phi.cols() );
  for (auto i=0; i<20; ++i){
    std::cout << i << std::endl;
    A = JJ * phi;
  }

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
  	    << std::fixed << std::setprecision(10)
  	    << elapsed.count() << std::endl;

  // // print summary from timers
  // #ifdef HAVE_TEUCHOS_TIMERS
  // pressio::utils::TeuchosPerformanceMonitor::stackedTimersReportSerial();
  // #endif

  return 0;
}
