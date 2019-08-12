
#ifndef ADR2D_CPP_INPUT_HPP_
#define ADR2D_CPP_INPUT_HPP_

#include <sstream>

struct InputParser{
  using scalar_t = double;

  std::string meshFileName_	= "empty";

  scalar_t dt_			= {};
  scalar_t finalT_		= {};
  scalar_t D_			= {};
  scalar_t K_			= {};

  int observerOn_		= 0;
  int snapshotsFreq_		= {}; // freq of collecting snapshots
  std::string shapshotsFileName_= "empty";
  std::string basisFileName_	= "empty";
  int romOn_			= 0;
  unsigned int romSize_		= {};

public:
  InputParser() = default;
  ~InputParser() = default;

  int parse(int argc, char *argv[])
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

	if (col1 == "meshFileName"){
	  meshFileName_	= col2;
	}

	if (col1 == "dt"){
	  dt_ = std::stod(col2);
	}
	if (col1 == "finalTime"){
	  finalT_ = std::stod(col2);
	}
	if (col1 == "diffusion"){
	  D_ = std::stod(col2);
	}
	if (col1 == "chemReaction"){
	  K_ = std::stod(col2);
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

    std::cout << meshFileName_ << " "
	      << dt_ << " "
	      << finalT_ << " "
	      << D_ << " "
	      << K_ << " "
	      << observerOn_ << " "
	      << shapshotsFileName_ << " "
	      << snapshotsFreq_ << " "
	      << romOn_ << " "
	      << romSize_ << " "
	      << basisFileName_
	      << std::endl;

    if (meshFileName_=="empty"){
      std::cerr << "Invalid meshFileName" << std::endl;
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
    if (D_<=0.){
      std::cerr << "Invalid diffusion coeff" << std::endl;
      return 1;
    }
    if (K_<=0.){
      std::cerr << "Invalid chem coeff" << std::endl;
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
