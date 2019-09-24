
#ifndef ADR2D_INPUT_HPP_
#define ADR2D_INPUT_HPP_

#include <sstream>

struct InputParser
{
  using scalar_t = double;
  using step_t   = int32_t;

  std::string problemName_	= "empty";
  std::string meshFileName_	= "empty";
  std::string gidMapFileName_	= "empty";

  std::string odeStepperName_	= {};
  scalar_t dt_			= {};
  scalar_t finalT_		= {};
  step_t NSteps_		= {};

  scalar_t D_			= {};
  scalar_t K_			= {};

  int observerOn_		= 0;
  int snapshotsFreq_		= {}; // freq of collecting snapshots
  std::string shapshotsFileName_= "empty";
  std::string basisFileName_	= "empty";
  int romOn_			= 0;
  int32_t romSize_		= {};

public:
  InputParser() = default;
  ~InputParser() = default;

  int parse(int argc, char *argv[])
  {
    if (argc != 2){
      std::cerr << "invalid number of argments" << std::endl;
      std::cerr << "Usage: " << argv[0] << " inputFile " << std::endl;
      return 1;
    }

    // which input file to read
    const std::string inputFile = argv[1];

    std::ifstream source;
    source.open( inputFile, std::ios_base::in);
    std::string line;
    while (std::getline(source, line) )
    {
      //make a stream for the line itself
      std::istringstream in(line);
      // tmp variables to store each entry in each line of the file
      std::string col1, col2;
      // read line
      in >> col1; in >> col2;

      if (col1 == "problemName")
	problemName_ = col2;

      if (col1 == "meshFileName")
	meshFileName_ = col2;

      if (col1 == "smToFmGIDMappingFileName")
	gidMapFileName_ = col2;

      if (col1 == "odeStepperName")
	odeStepperName_ = col2;

      if (col1 == "dt")
	dt_ = std::stod(col2);

      if (col1 == "finalTime")
	finalT_ = std::stod(col2);

      // compute num steps
      NSteps_ = static_cast<step_t>(finalT_/dt_);

      if (col1 == "diffusion")
	D_ = std::stod(col2);

      if (col1 == "chemReaction")
	K_ = std::stod(col2);


      if (col1 == "observerOn")
	observerOn_ = std::stoi(col2);

      if (observerOn_==1){
	if (col1 == "shapshotsFreq")
	  snapshotsFreq_ = std::stoi(col2);

	if (col1 == "shapshotsFileName")
	  shapshotsFileName_ = col2;

	if (col1 == "basisFileName")
	  basisFileName_ = col2;
      }

      if (col1 == "romOn")
	romOn_ = std::stoi(col2);

      if (romOn_==1){
	if (col1 == "romSize")
	  romSize_ = std::stoi(col2);

	if (col1 == "basisFileName")
	  basisFileName_ = col2;
      }
    }
    source.close();

    std::cout << problemName_ << std::endl;
    std::cout << meshFileName_ << std::endl;
    std::cout << gidMapFileName_ << std::endl;
    std::cout << "stepper = "		<< odeStepperName_	<< " \n"
	      << "dt = "		<< dt_			<< " \n"
	      << "finalT = "		<< finalT_		<< " \n"
	      << "numSteps = "		<< NSteps_		<< " \n"
	      << "D = "			<< D_			<< " \n"
	      << "K = "			<< K_			<< " \n"
	      << "observerOn "		<< observerOn_		<< " \n"
	      << "shapshotsFileName_ = "<< shapshotsFileName_	<< " \n"
	      << "snapshotsFreq_ = "	<< snapshotsFreq_	<< " \n"
	      << "romOn_ = "		<< romOn_		<< " \n"
	      << "romSize_ = "		<< romSize_		<< " \n"
	      << "basisFileName_ = "	<< basisFileName_
	      << std::endl;

    return checkArgsValidity();

  } //end parse

private:

  int checkArgsValidity()
  {
    if(problemName_ != "chemABC" and problemName_ != "ms")
    {
      std::cerr << "Invalid problemName, choose chemABC or ms" << std::endl;
      return 1;
    }

    if (meshFileName_=="empty"){
      std::cerr << "Empty meshFileName" << std::endl;
      return 1;
    }

    if (odeStepperName_ != "RungeKutta4"  and
	odeStepperName_ != "bdf1"){
      // std::cerr << "Invalid odeStepperName" << std::endl;
      // std::cerr << "odeStepperName must be: Euler, or RungeKutta4, bdf1" << std::endl;
    }

    if (dt_<=0.){
      std::cerr << "Cannot have dt <= 0" << std::endl;
      return 1;
    }
    if (finalT_<=0.){
      std::cerr << "Cannot have finalT <= 0" << std::endl;
      return 1;
    }
    if (D_<0.){
      std::cerr << "Cannot have negative diffusion coeff" << std::endl;
      return 1;
    }
    if (K_<0.){
      std::cerr << "Cannot have negative chem coeff" << std::endl;
      return 1;
    }

    if (observerOn_==1)
    {
      if (snapshotsFreq_<=0){
    	std::cerr << "cannot have snapshotsFreq <=0 " << std::endl;
    	return 1;
      }
      if (shapshotsFileName_=="empty"){
    	std::cerr << "Empty shapshotsFileName" << std::endl;
    	return 1;
      }
      if (basisFileName_=="empty"){
    	std::cerr << "Empty basisFileName" << std::endl;
    	return 1;
      }
    }

    if (romOn_==1){
      if (romSize_<=0){
    	std::cerr << "Cannot have rom size <=0 " << std::endl;
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
