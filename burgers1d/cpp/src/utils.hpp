
#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "UTILS_ALL"
#include "CONTAINERS_ALL"
#include <chrono>

template <typename sc_t, typename int_t, typename dmat_t>
dmat_t readBasis(std::string filename, int_t romSize)
{
  // create an empty matrix using stl
  std::vector<std::vector<sc_t>> A0;

  // read file into this matrix
  ::pressio::utils::readAsciiMatrixStdVecVec<int_t, sc_t>(filename, A0, romSize);

  // store data into eigen format
  dmat_t eigM( A0.size(), A0.begin()->size() );
  for (int_t i=0; i<eigM.rows(); i++){
    for (int_t j=0; j<eigM.cols(); j++)
      eigM(i,j) = A0[i][j];
  }

  return eigM;
}


void printEigenDMatrixToFile(std::string fileName,
			     const Eigen::MatrixXd & A)
{
  using int_t = int32_t;

  std::ofstream file;
  file.open(fileName);
  for (int_t i=0; i<A.rows(); i++){
    for (int_t j=0; j<A.cols(); j++){
      file << std::fixed
	   << std::setprecision(15)
	   << A(i,j) << " ";
    }
    file << std::endl;
  }
  file.close();
}


#endif
