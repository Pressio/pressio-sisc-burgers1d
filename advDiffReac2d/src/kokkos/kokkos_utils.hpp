
#ifndef KOKKOS_UTILS_HPP_
#define KOKKOS_UTILS_HPP_

#include "UTILS_ALL"
#include "CONTAINERS_ALL"

template <typename phi_t>
auto readBasis(std::string filename, int32_t romSize)
  -> phi_t
{
  std::vector<std::vector<double>> A0;
  ::pressio::utils::readAsciiMatrixStdVecVec(filename, A0, romSize);
  const size_t nrows = A0.size();
  const size_t ncols = A0[0].size();

  phi_t phi_d("phi", nrows, ncols);

  // create a host view that mirrors the device passed in
  using view_h_t = typename ::pressio::containers::details::traits<phi_t>::host_mirror_t;
  view_h_t phi_h("phi_h", nrows, ncols);
  for (size_t i=0; i<nrows; i++)
  {
    for (size_t j=0; j<ncols; j++)
      phi_h(i,j) = A0[i][j];
  }
  // deep copy to device
  Kokkos::deep_copy(*phi_d.data(), phi_h);
  return phi_d;
}


template <typename int_t>
void readSmToFmGIDsMappingFile(std::string filename,
			       std::vector<std::array<int_t,2>> & A){
  std::ifstream file(filename);
  std::string line;
  std::array<int_t, 2> lineGIDs;
  while (getline(file, line)){
    std::istringstream ss(line);
    int_t entry;
    int_t j = 0;
    while (ss >> entry){
      lineGIDs[j] = entry;
      j++;
    }
    A.emplace_back(lineGIDs);
  }

  // for (auto & it : A){
  //   for (auto & it2 : it){
  //     std::cout << " " << it2;
  //   }
  //   std::cout << std::endl;
  // }
}


template <typename phi_d_t, typename app_t>
::pressio::containers::MultiVector<phi_d_t>
extractSampleMeshRows(const ::pressio::containers::MultiVector<phi_d_t> & phi0,
		      const InputParser & parser,
		      const app_t & appObj)
{
  // I do all operations on the host, since this is just extracting some
  // rows from the phi0 passed in, and then create a new phi1 on the device.
  // Note that phi0 is here a pressio wrapper

  using int_t = typename app_t::index_t;

  // number of basis vectors
  const auto numBasis = phi0.numVectors();

  // create a host mirror for phi0 since phi0 is a wrapper of device view
  using phi_h_t = typename phi_d_t::HostMirror;
  phi_h_t phi0_h("phi0_h", phi0.length(), phi0.numVectors());

  // copy phi0 from device to host
  Kokkos::deep_copy(phi0_h, *phi0.data());

  // load the sample mesh to full mesh GIDs mapping
  std::vector< std::array<int_t,2> > smToFmGidMap;
  readSmToFmGIDsMappingFile(parser.gidMapFileName_, smToFmGidMap);

  // get a reference to the HOST mesh graph from the app
  const auto & graph_h = appObj.viewGraphHost();

  // number of rows for the sample mesh basis matrix
  const int_t numRowsSMBasis = smToFmGidMap.size()*app_t::numSpecies_;

  // create the native HOST and device view to store the sample-mesh basis vectors
  phi_h_t phi1_h("phi1_h", numRowsSMBasis, numBasis);

  // loop over HOST sample mesh cells
  for (size_t iPt=0; iPt < smToFmGidMap.size(); ++iPt)
  {
    // get sample mesh GID of the current cell
    const auto & cellGIDsm = smToFmGidMap[iPt][0];
    // get the GID in the full mesh of this cell
    const auto & cellGIDfm = smToFmGidMap[iPt][1];

    // find which index in the state vector is associated with the
    // first species in this sample mesh cell
    const auto c0StateIndex_sm  = app_t::numSpecies_ * cellGIDsm;

    // find which index in the state vector is associated with the
    // first species in the full mesh cell
    const auto c0StateIndex_fm  = app_t::numSpecies_ * cellGIDfm;

    // std::cout << std::endl;
    // std::cout << iPt << " " << c0StateIndex_sm << " " << c0StateIndex_fm << " \n";
    for (auto iDof=0; iDof<app_t::numSpecies_; iDof++){
      //std::cout << c0StateIndex_sm+iDof << " ";

      for (auto j=0; j<numBasis; j++){
	phi1_h(c0StateIndex_sm + iDof, j) = phi0_h(c0StateIndex_fm + iDof,j);
	//std::cout << std::setprecision(15) << phi1_h(c0StateIndex_sm+iDof, j) << " ";
      }
      //std::cout << "\n";
    }
  }

  // create the device view to store the sample-mesh basis vectors
  phi_d_t phi1_d("phi1_d", numRowsSMBasis, numBasis);

  // now that basis is stored in the host view, deep copy to device
  Kokkos::deep_copy(phi1_d, phi1_h);

  // wrap with pressio multivector and return
  pressio::containers::MultiVector<phi_d_t> result(phi1_d);
  return result;
}


#endif
