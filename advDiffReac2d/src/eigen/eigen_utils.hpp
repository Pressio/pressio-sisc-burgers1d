
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

template <typename T = double>
pressio::containers::MultiVector<Eigen::MatrixXd>
convertFromVVecToMultiVec(const std::vector<std::vector<T>> & A0)
{
  const size_t nrows = A0.size();
  const size_t ncols = A0[0].size();
  pressio::containers::MultiVector<Eigen::MatrixXd> ADW(nrows, ncols);
  for (int i=0; i<nrows; i++){
    for (int j=0; j<ncols; j++)
      ADW(i,j) = A0[i][j];
  }
  return ADW;
}

template <typename T>
pressio::containers::MultiVector<Eigen::MatrixXd>
readBasis(std::string filename, T romSize)
{
  std::vector<std::vector<double>> A0;
  readAsciiMatrixStdVecVec(filename, A0, romSize);
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
    int j = 0;
    while (ss >> entry){
      lineGIDs[j] = entry;
      j++;
    }
    A.emplace_back(lineGIDs);
  }

  for (auto & it : A){
    for (auto & it2 : it){
      std::cout << " " << it2;
    }
    std::cout << std::endl;
  }
}


template <typename int_t, typename phi_t, typename app_t>
pressio::containers::MultiVector<Eigen::MatrixXd>
extractSampleMeshRows(const phi_t & phi0,
		      const InputParser & parser,
		      const app_t & appObj)
{
  std::vector< std::array<int_t,2> > smToFmGidMap;
  readSmToFmGIDsMappingFile(parser.gidMapFileName_, smToFmGidMap);

  // get a reference to the mesh graph from the app
  const auto & graph = appObj.viewGraph();

  const auto numBasis = phi0.numVectors();

  // number of rows for the sample mesh basis matrix
  const int_t numRowsSMBasis = smToFmGidMap.size()*app_t::numSpecies_;

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

    std::cout << std::endl;
    std::cout << iPt << " " << c0StateIndex_sm << " " << c0StateIndex_fm << " \n";
    for (auto iDof=0; iDof<app_t::numSpecies_; iDof++){
      std::cout << c0StateIndex_sm+iDof << " ";
      for (auto j=0; j<numBasis; j++)
      {
	phi1n(c0StateIndex_sm + iDof, j) = phi0(c0StateIndex_fm + iDof,j);

	std::cout << std::setprecision(15) << phi1n(c0StateIndex_sm+iDof, j) << " ";
      }
      std::cout << "\n";
    }
  }
  pressio::containers::MultiVector<Eigen::MatrixXd> result(phi1n);
  return result;
}


#endif
