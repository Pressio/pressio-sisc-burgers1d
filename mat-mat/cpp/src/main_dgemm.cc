
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

  // parse input file
  InputParser parser;
  int32_t err = parser.parse(argc, argv);
  if (err == 1) return 1;

  const auto numDofs = parser.numDofs_;
  const auto romSize = parser.romSize_;

  //
  // mimic the case for jacobian being dense and phi being dense
  //

  eig_dyn_mat A(numDofs, numDofs);
  //A = eig_dyn_mat::Random(rA, cA);
  A.setConstant(1);

  eig_dyn_mat B(numDofs, romSize);
  B.setConstant(2);

  eig_dyn_mat C(numDofs, romSize);
  Eigen::VectorXd f(numDofs);
  //volatile double f[numDofs];
  Eigen::VectorXd f2(numDofs); f2.setConstant(4);

  auto startTime = std::chrono::high_resolution_clock::now();
  for (auto i=1; i<=parser.numRepli_; ++i){
    //std::cout << i << std::endl;
    //C = A.transpose() * B;

    for (int i=0; i<numDofs; ++i){
      f[i] += 0.005 * std::exp(2.)*std::sin(3.14);
    }
    for (int i=0; i<numDofs; ++i){
      f[i] += f2[i];
    }

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

  constexpr auto two = static_cast<scalar_t>(2);
  const scalar_t gflop = 0.0; //(two*numDofs*numDofs*romSize)*1e-9;
  std::cout << "GFlopPerItem: "
	    << std::fixed << std::setprecision(5)
	    << gflop << std::endl;
  std::cout << "GFlop/sec: "
	    << std::fixed << std::setprecision(5)
	    << gflop/eT_avg << std::endl;

  return 0;
}
