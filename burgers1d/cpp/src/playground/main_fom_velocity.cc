#define EIGEN_USE_BLAS
#define EIGEN_USE_LAPACKE

#include "CONTAINERS_ALL"
#include "ODE_ALL"
#include "burgers1d_eigen.hpp"
#include "burgers1d_input_parser.hpp"
#include "eigen_observer.hpp"

int main(int argc, char *argv[])
{
  // types
  using app_t		= Burgers1dEigen;
  using scalar_t	= typename app_t::scalar_type;
  using app_state_t	= typename app_t::state_type;
  using app_jacob_t	= typename app_t::jacobian_type;
  using app_dmat_t	= typename app_t::eig_dense_mat;
  using ode_state_t	= pressio::containers::Vector<app_state_t>;

  constexpr auto zero = ::pressio::utils::constants::zero<scalar_t>();
  constexpr auto one  = ::pressio::utils::constants::one<scalar_t>();

  // parse input file
  InputParser parser;
  int err = parser.parse(argc, argv);
  if (err == 1) return 1;

  // create app object
  app_t appObj(parser.numCell_);
  // set initial state to dummy, all ones
  app_state_t x0n(parser.numCell_);
  x0n.setConstant(one);

  // app_jacob_t J(parser.numCell_, parser.numCell_);
  // appObj.jacobian(x0n, zero, J);

  const int32_t numRounds = 5000;
  const int32_t romSize = 100;

  app_dmat_t phi(parser.numCell_, romSize);
  phi.setConstant(one);
  app_state_t a(romSize);
  app_state_t a1(parser.numCell_);

  //--- do one for warm up ---
  auto f = appObj.velocity(x0n, zero);
  a  = phi.transpose() * f;
  a1 = phi * a;

  //--- do many runs ---
  auto startTime = std::chrono::high_resolution_clock::now();
  for (auto i=0; i<numRounds; ++i){
    appObj.velocity(x0n, zero, f);
    a  = phi.transpose() * f;
    a1 = phi * a;
    //appObj.jacobian(x0n, zero, J);
  }

  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  std::cout << "Elapsed time: "
  	    << std::fixed << std::setprecision(10)
  	    << elapsed.count() << std::endl;

  std::cout << "Time per round: "
  	    << std::fixed << std::setprecision(10)
  	    << elapsed.count()/(double)numRounds << std::endl;

  return 0;
}
