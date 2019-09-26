
// // pressio headers
// #include "UTILS_ALL"
// #include "CONTAINERS_ALL"
// #include "ODE_IMPLICIT"
// #include "ODE_INTEGRATORS"
// #include "SOLVERS_NONLINEAR"
// #include "ROM_LSPG"
// local headers
#include "adr2d_kokkos.hpp"
#include "input_parser.hpp"
#include "advection_cellular_flow_functor.hpp"
#include "source_term_chemABC_functor.hpp"
#include "kokkos_utils.hpp"
#include <random>

int main(int argc, char *argv[]){
Kokkos::initialize (argc, argv);
{
  /*----------------------
   * PARSER
   ----------------------*/
  InputParser parser;
  int err = parser.parse(argc, argv);
  if (err == 1) return 1;

  const auto romSize	= parser.romSize_;

  /*----------------------
   * TYPES
   ----------------------*/
  using scalar_t	  = double;

  using fom_t		  = Adr2dKokkos<ChemistryABCSource, CellularFlow>;

  // get the source and advection functor types from the app
  using src_fnct_t	  = typename fom_t::source_functor_t;
  using adv_fnct_t	  = typename fom_t::advection_functor_t;

  // grab the native types for both device(d) and host(h)
  using native_state_d_t  = typename fom_t::state_type_d_t;
  using native_state_h_t  = typename fom_t::state_type_h_t;
  using native_mv_d_t     = typename fom_t::mv_d_t;
  using native_mv_h_t	  = typename fom_t::mv_h_t;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  /*----------------------
   * CREATE APP OBJECT
   ----------------------*/
  // here we do everything as if we were setting up a real rom problem,
  // but then we use the Jacobian and basis matrix to repeat mat-mat prodducts
  // to test the performacne

  adv_fnct_t advFunctor;
  src_fnct_t srcFunctor(parser.K_);
  fom_t appObj(parser.meshFileName_, srcFunctor, advFunctor, parser.D_);
  // get the number of total dofs
  const auto stateSize = appObj.getStateSize();

  // the device reference state = all zeros
  // just need to initialize, since by default kokkos zeros out
  native_state_d_t yRef("yRef", stateSize);
  {
    native_state_h_t yRefh("yRefh", stateSize);
    for (size_t i=0; i<stateSize; ++i)
      yRefh(i) = one;
    Kokkos::deep_copy(yRef, yRefh);
  }

  /*----------------------
   * CREATE RANDOM basis vectors
   * since we do not care about numbers here
   ----------------------*/
  native_mv_d_t phi("phi", stateSize, parser.romSize_);
  const auto numBasis = phi.extent(1);
  if( numBasis != romSize ) return 0;
  {
    const auto phiEig = Eigen::MatrixXd::Random(phi.extent(0), phi.extent(1));
    // using gen_t	   = std::mt19937;
    // using rand_distr_t = std::uniform_real_distribution<scalar_t>;
    // gen_t engine(473445);
    // rand_distr_t distr(zero, one);
    // auto genFnc = [&distr, &engine](){
    // 		   return distr(engine);
    // 		 };

    // fill randomly
    native_mv_h_t phi_h("phi_h", stateSize, parser.romSize_);
    for (size_t i=0; i<phi_h.extent(0); i++){
      for (size_t j=0; j<phi_h.extent(1); j++)
  	phi_h(i,j) = phiEig(i,j);
    }
    Kokkos::deep_copy(phi, phi_h);
  }

  // start timer: we do it here because we do not count reading the basis
  auto startTime = std::chrono::high_resolution_clock::now();

  // do product of Jacobian times phi, i.e. sparse time dense
  // this is how it is done inside the app
  const auto JJ = appObj.jacobian(yRef, zero);
  native_mv_d_t A("AA", stateSize, phi.extent(1));
  const char ct = 'N';
  for (auto i=0; i<20; ++i){
    std::cout << i << std::endl;
    KokkosSparse::spmv(&ct, one, JJ, phi, zero, A);
  }

  // Record end time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
  	    << std::fixed << std::setprecision(10)
  	    << elapsed.count() << std::endl;

}//end kokkos scope
return 0;
}
