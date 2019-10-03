
#ifndef BURGERS1DCPP_INPUT_HPP_
#define BURGERS1DCPP_INPUT_HPP_

#include <sstream>

struct InputParser{
  using scalar_t = double;

  std::string fileName_ = "empty";
  int32_t numCell_ = {};
  scalar_t dt_	        = {};
  scalar_t finalT_	= {};
  int32_t observerOn_	= 0;
  int32_t snapshotsFreq_    = {}; // freq of collecting snapshots
  std::string shapshotsFileName_ = "empty";
  std::string basisFileName_ = "empty";

  int32_t romOn_	= 0;
  int32_t romSize_ = {};

  InputParser() = default;
  ~InputParser() = default;

  int32_t parse(int32_t argc, char *argv[])
  {
    if (argc != 2){
      std::cerr << "Usage: " << argv[0] << " inputFile " << std::endl;
      std::cerr << "invalid number of argments" << std::endl;
      return 1;
    }

    const std::string inputFile = argv[1];

    std::ifstream source;
    source.open( inputFile, std::ios_base::in);
    std::string line;
    while (std::getline(source, line) )
      {
	//make a stream for the line itself
	std::istringstream in(line);
	// tmp variable to store each entry of the file
	std::string col1, col2;
	// column 0, index
	in >> col1;
	in >> col2;
	if (col1 == "numCell"){
	  numCell_	= std::stoi(col2);
	}
	if (col1 == "dt"){
	  dt_ = std::stod(col2);
	}
	if (col1 == "finalTime"){
	  finalT_ = std::stod(col2);
	}

	if (col1 == "observerOn"){
	  observerOn_ = std::stoi(col2);
	}
	if (observerOn_==1){
	  if (col1 == "shapshotsFreq"){
	    snapshotsFreq_ = std::stoi(col2);
	  }
	  if (col1 == "shapshotsFileName"){
	    shapshotsFileName_ = col2;
	  }
	  if (col1 == "basisFileName"){
	    basisFileName_ = col2;
	  }
	}

	if (col1 == "romOn"){
	  romOn_ = std::stoi(col2);
	}
	if (romOn_==1){
	  if (col1 == "romSize"){
	    romSize_ = std::stoi(col2);
	  }
	  if (col1 == "basisFileName"){
	    basisFileName_ = col2;
	  }
	}
      }
    source.close();

    std::cout << "Ncell = "		<< numCell_		<< " \n"
	      << "dt = "		<< dt_			<< " \n"
	      << "finalT = "		<< finalT_		<< " \n"
	      << "observerOn "		<< observerOn_		<< " \n"
	      << "shapshotsFileName_ = "<< shapshotsFileName_	<< " \n"
	      << "snapshotsFreq_ = "	<< snapshotsFreq_	<< " \n"
	      << "romOn_ = "		<< romOn_		<< " \n"
	      << "romSize_ = "		<< romSize_		<< " \n"
	      << "basisFileName_ = "	<< basisFileName_
	      << std::endl;

    if (numCell_==0){
      std::cerr << "Invalid numCell" << std::endl;
      return 1;
    }
    if (dt_==0.){
      std::cerr << "Invalid dt" << std::endl;
      return 1;
    }
    if (finalT_==0.){
      std::cerr << "Invalid finalT" << std::endl;
      return 1;
    }

    if (observerOn_==1){
      if (snapshotsFreq_==0){
    	std::cerr << "Invalid snapshotsFreq" << std::endl;
    	return 1;
      }
      if (shapshotsFileName_=="empty"){
    	std::cerr << "Invalid shapshotsFileName" << std::endl;
    	return 1;
      }
      if (basisFileName_=="empty"){
    	std::cerr << "Invalid basisFileName" << std::endl;
    	return 1;
      }
    }

    if (romOn_==1){
      if (romSize_==0){
    	std::cerr << "Invalid rom size" << std::endl;
    	return 1;
      }
      if (basisFileName_=="empty"){
    	std::cerr << "Invalid basisFileName" << std::endl;
    	return 1;
      }
    }

    return 0;
  }
};

#endif
