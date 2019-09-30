
#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "UTILS_ALL"
#include "CONTAINERS_ALL"

template <typename sc_t, typename int_t, typename dmat_t = Eigen::MatrixXd>
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

// template <typename T = int32_t>
// void readAsciiMatrixStdVecVec(std::string filename,
// 			      std::vector<std::vector<double>> & A0,
// 			      T ncols)
// {
//   assert( A0.empty() );
//   std::ifstream source;
//   source.open( filename, std::ios_base::in);
//   std::string line, colv;
//   std::vector<double> tmpv(ncols);
//   while (std::getline(source, line) ){
//     std::istringstream in(line);
//     for (int32_t i=0; i<ncols; i++){
//       in >> colv;
//       tmpv[i] = atof(colv.c_str());
//     }
//     A0.emplace_back(tmpv);
//   }
//   source.close();
// }

// template <typename T = int32_t>
// auto convertFromVVecToMultiVec(const std::vector<std::vector<double>> & A0,
// 			       T nrows, T ncols)
//   -> pressio::containers::MultiVector<Eigen::MatrixXd>{

//   pressio::containers::MultiVector<Eigen::MatrixXd> ADW(nrows, ncols);
//   for (int32_t i=0; i<nrows; i++){
//     for (int32_t j=0; j<ncols; j++)
//       ADW(i,j) = A0[i][j];
//   }
//   return ADW;
// }

// template <typename T = int32_t>
// auto readBasis(std::string filename,
// 	       T romSize, T numCell)
//   ->pressio::containers::MultiVector<Eigen::MatrixXd>
// {
//   std::vector<std::vector<double>> A0;
//   readAsciiMatrixStdVecVec(filename, A0, romSize);
//   // read basis to a MultiVector
//   auto phi = convertFromVVecToMultiVec(A0, numCell, romSize);
//   //  phi.data()->Print(std::cout);
//   return phi;
// }

#endif
