
#ifndef EIGEN_UTILS_HPP_
#define EIGEN_UTILS_HPP_

#include "CONTAINERS_ALL"

template <typename T = int>
void readAsciiMatrixStdVecVec(std::string filename,
			      std::vector<std::vector<double>> & A0,
			      T ncols)
{
  assert( A0.empty() );
  std::ifstream source;
  source.open( filename, std::ios_base::in);
  std::string line, colv;
  std::vector<double> tmpv(ncols);
  while (std::getline(source, line) ){
    std::istringstream in(line);
    for (int i=0; i<ncols; i++){
      in >> colv;
      tmpv[i] = atof(colv.c_str());
    }
    A0.emplace_back(tmpv);
  }
  source.close();
}

template <typename T = int>
auto convertFromVVecToMultiVec(const std::vector<std::vector<double>> & A0,
			       T nrows, T ncols)
  -> pressio::containers::MultiVector<Eigen::MatrixXd>{

  pressio::containers::MultiVector<Eigen::MatrixXd> ADW(nrows, ncols);
  for (int i=0; i<nrows; i++){
    for (int j=0; j<ncols; j++)
      ADW(i,j) = A0[i][j];
  }
  return ADW;
}

template <typename T>
auto readBasis(std::string filename,
	       T romSize, T numCell)
  ->pressio::containers::MultiVector<Eigen::MatrixXd>
{
  std::vector<std::vector<double>> A0;
  readAsciiMatrixStdVecVec(filename, A0, romSize);
  // read basis into a MultiVector
  auto phi = convertFromVVecToMultiVec(A0, numCell, romSize);
  //  phi.data()->Print(std::cout);
  return phi;
}

#endif
