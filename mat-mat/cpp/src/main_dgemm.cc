
#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <Eigen/Dense>
#include "input_parser.hpp"
#include <random>

int main(int argc, char *argv[])
{
  /*----------------------
   * Dense Dense product
   ----------------------*/
  using scalar_t = double;
  constexpr auto layout = Eigen::ColMajor;
  using eig_dyn_mat	= Eigen::Matrix<scalar_t, -1, -1, layout>;

  constexpr auto two = static_cast<scalar_t>(2);

  // parse input file
  InputParser parser;
  int32_t err = parser.parse(argc, argv);
  if (err == 1) return 1;

  const auto rA = parser.matArows_;
  const auto cA = parser.matAcols_;
  const auto rB = parser.matBrows_;
  const auto cB = parser.matBcols_;

  eig_dyn_mat A(rA, cA);
  //A = eig_dyn_mat::Random(rA, cA);
  A.setConstant(1);

  eig_dyn_mat B(rB, cB);
  //B = eig_dyn_mat::Random(rB, cB);
  B.setConstant(2);

  eig_dyn_mat C(rA, cB);

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  for (auto i=1; i<=parser.numRepli_; ++i){
    std::cout << i << std::endl;
    C = A * B;
  }

  // Record run time
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  const auto elapsedTime = elapsed.count();
  const auto eT_avg = (elapsedTime)/parser.numRepli_;

  std::cout << "Elapsed time: "
	    << std::fixed << std::setprecision(10)
	    << elapsedTime << std::endl;
  std::cout << "Avg time per run: "
	    << std::fixed << std::setprecision(10)
	    << eT_avg << std::endl;

  const scalar_t gflop = (two*rA*cA*cB)*1e-9;
  std::cout << "GFlopPerItem: "
	    << std::fixed << std::setprecision(5)
	    << gflop << std::endl;
  std::cout << "GFlop/sec: "
	    << std::fixed << std::setprecision(5)
	    << gflop/eT_avg << std::endl;

  return 0;
}
