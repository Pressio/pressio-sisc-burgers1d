
#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <Eigen/Sparse>
#include "input_parser.hpp"
#include <random>

int main(int argc, char *argv[])
{
  using scalar_t = double;
  using int_t	= int32_t;
  using Tr	= Eigen::Triplet<scalar_t>;

  static constexpr auto spmat_layout = Eigen::ColMajor;
  using eig_sp_mat   = Eigen::SparseMatrix<scalar_t, spmat_layout, int_t>;

  static constexpr auto demat_layout = Eigen::ColMajor;
  using eig_de_mat   = Eigen::Matrix<scalar_t, -1, -1, demat_layout>;

  // parse input file
  InputParser parser;
  int_t err = parser.parse(argc, argv);
  if (err == 1) return 1;

  const auto rA = parser.matArows_;
  const auto cA = parser.matAcols_;
  const auto rB = parser.matBrows_;
  const auto cB = parser.matBcols_;

  // fill sparse matrix A to be pentadiagonal
  constexpr scalar_t Vdm1 = 343.232;
  constexpr scalar_t Vd   = 34.1232;
  constexpr scalar_t Vdp1 = 5654.55652;

  std::vector<Tr> tripletList_;
  tripletList_.push_back( Tr(0, 0, Vd) );
  tripletList_.push_back( Tr(0, 1, Vdp1) );
  for (int_t i=1; i<rA; ++i){
    tripletList_.push_back( Tr(i, i-1, Vdm1) );
    tripletList_.push_back( Tr(i, i, Vd) );
    if (i<rA-1)
      tripletList_.push_back( Tr(i, i+1, Vdp1) );
  }
  eig_sp_mat A(rA, cA);
  A.setFromTriplets(tripletList_.begin(), tripletList_.end());
  if ( !A.isCompressed() )
    A.makeCompressed();

  eig_de_mat B(rB, cB);
  //B = eig_de_mat::Random(rB, cB);
  B.setConstant(1);

  // Record start time
  auto startTime = std::chrono::high_resolution_clock::now();

  for (auto i=1; i<=parser.numRepli_; ++i){
    std::cout << i << std::endl;
    auto C = A.transpose() * B;
  }
  //std::cout << Eigen::MatrixXd(C) << std::endl;

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

  const scalar_t gflop = 0.0;
  std::cout << "GFlopPerItem: "
  	    << std::fixed << std::setprecision(5)
  	    << gflop << std::endl;
  std::cout << "GFlop/sec: "
  	    << std::fixed << std::setprecision(5)
  	    << gflop/eT_avg << std::endl;

  return 0;
}
