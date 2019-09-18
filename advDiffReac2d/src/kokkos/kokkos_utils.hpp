
#ifndef KOKKOS_UTILS_HPP_
#define KOKKOS_UTILS_HPP_

#include "UTILS_ALL"
#include "CONTAINERS_ALL"

// template <typename T = int>
// void readMatrixFromFile(std::string filename,
// 			std::vector<std::vector<double>> & A0,
// 			T ncols){

//   std::ifstream source;
//   source.open( filename, std::ios_base::in);
//   std::string line, colv;
//   std::vector<double> tmpv(ncols);
//   while (std::getline(source, line) ){
//     std::istringstream in(line);
//     for (int i=0; i<ncols; i++){
//       in >> colv;
//       tmpv[i] = atof(colv.c_str());
//     }
//     A0.emplace_back(tmpv);
//   }
//   source.close();
// }

template <typename phi_d_t>
void readBasis(std::string filename, int romSize, phi_d_t phi_d)
{
  std::vector<std::vector<double>> A0;
  ::pressio::utils::readAsciiMatrixStdVecVec(filename, A0, romSize);
  const size_t nrows = A0.size();
  const size_t ncols = A0[0].size();
  std::cout << nrows << " " << ncols << std::endl;

  // create a host view that mirrors the device passed in
  using view_h_t = typename phi_d_t::HostMirror;
  view_h_t phi_h("phi_h", nrows, ncols);
  for (size_t i=0; i<nrows; i++)
  {
    for (size_t j=0; j<ncols; j++)
      phi_h(i,j) = A0[i][j];
  }
  // deep copy to device
  Kokkos::deep_copy(phi_d, phi_h);
}

#endif
