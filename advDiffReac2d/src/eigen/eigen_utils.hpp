
#ifndef EIGEN_UTILS_HPP_
#define EIGEN_UTILS_HPP_

#include "UTILS_ALL"
#include "CONTAINERS_ALL"

template <typename T = double>
pressio::containers::MultiVector<Eigen::MatrixXd>
convertFromVVecToMultiVec(const std::vector<std::vector<T>> & A0)
{
  using eig_mat = Eigen::Matrix<T, -1, -1>;

  const size_t nrows = A0.size();
  const size_t ncols = A0[0].size();
  pressio::containers::MultiVector<eig_mat> ADW(nrows, ncols);

  for (size_t i=0; i<nrows; i++){
    for (size_t j=0; j<ncols; j++)
      ADW(i,j) = A0[i][j];
  }
  return ADW;
}


template <typename T>
pressio::containers::MultiVector<Eigen::MatrixXd>
readBasis(std::string filename, T romSize)
{
  std::vector<std::vector<double>> A0;
  ::pressio::utils::readAsciiMatrixStdVecVec(filename, A0, romSize);
  // read basis into a MultiVector
  auto phi = convertFromVVecToMultiVec(A0);
  //  phi.data()->Print(std::cout);
  return phi;
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


template <typename phi_t, typename app_t>
pressio::containers::MultiVector<Eigen::MatrixXd>
extractSampleMeshRows(const phi_t & phi0,
		      const InputParser & parser,
		      const app_t & appObj)
{
  using int_t = typename app_t::index_t;

  std::vector< std::array<int_t,2> > smToFmGidMap;
  readSmToFmGIDsMappingFile(parser.gidMapFileName_, smToFmGidMap);

  // get a reference to the mesh graph from the app
  const auto & graph = appObj.viewGraph();

  const auto numBasis = phi0.numVectors();

  // number of rows for the sample mesh basis matrix
  const auto numRowsSMBasis = smToFmGidMap.size()*app_t::numSpecies_;

  // create the native eigen matrix to store the sample-mesh bases
  Eigen::MatrixXd phi1n(numRowsSMBasis, numBasis);

  // loop over sample mesh cells holding the state
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
	phi1n(c0StateIndex_sm + iDof, j) = phi0(c0StateIndex_fm + iDof,j);
	//std::cout << std::setprecision(15) << phi1n(c0StateIndex_sm+iDof, j) << " ";
      }
      //std::cout << "\n";
    }
  }
  pressio::containers::MultiVector<Eigen::MatrixXd> result(phi1n);
  return result;
}


#endif
