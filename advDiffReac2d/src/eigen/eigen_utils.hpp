
#ifndef EIGEN_UTILS_HPP_
#define EIGEN_UTILS_HPP_

#include "UTILS_ALL"
#include "CONTAINERS_ALL"


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


// template <typename phi_t, typename app_t>
// pressio::containers::MultiVector<Eigen::MatrixXd>
// extractSampleMeshRows(const phi_t & phi0,
// 		      const InputParser & parser,
// 		      const app_t & appObj)
// {
//   using int_t = typename app_t::index_t;

//   std::vector< std::array<int_t,2> > smToFmGidMap;
//   readSmToFmGIDsMappingFile(parser.gidMapFileName_, smToFmGidMap);

//   // get a reference to the mesh graph from the app
//   const auto & graph = appObj.viewGraph();

//   const auto numBasis = phi0.numVectors();

//   // number of rows for the sample mesh basis matrix
//   const auto numRowsSMBasis = smToFmGidMap.size()*app_t::numSpecies_;

//   // create the native eigen matrix to store the sample-mesh bases
//   Eigen::MatrixXd phi1n(numRowsSMBasis, numBasis);

//   // loop over sample mesh cells holding the state
//   for (size_t iPt=0; iPt < smToFmGidMap.size(); ++iPt)
//   {
//     // get sample mesh GID of the current cell
//     const auto & cellGIDsm = smToFmGidMap[iPt][0];
//     // get the GID in the full mesh of this cell
//     const auto & cellGIDfm = smToFmGidMap[iPt][1];

//     // find which index in the state vector is associated with the
//     // first species in this sample mesh cell
//     const auto c0StateIndex_sm  = app_t::numSpecies_ * cellGIDsm;

//     // find which index in the state vector is associated with the
//     // first species in the full mesh cell
//     const auto c0StateIndex_fm  = app_t::numSpecies_ * cellGIDfm;

//     // std::cout << std::endl;
//     // std::cout << iPt << " " << c0StateIndex_sm << " " << c0StateIndex_fm << " \n";
//     for (auto iDof=0; iDof<app_t::numSpecies_; iDof++){
//       //std::cout << c0StateIndex_sm+iDof << " ";

//       for (auto j=0; j<numBasis; j++){
// 	phi1n(c0StateIndex_sm + iDof, j) = phi0(c0StateIndex_fm + iDof,j);
// 	//std::cout << std::setprecision(15) << phi1n(c0StateIndex_sm+iDof, j) << " ";
//       }
//       //std::cout << "\n";
//     }
//   }
//   pressio::containers::MultiVector<Eigen::MatrixXd> result(phi1n);
//   return result;
// }


#endif
