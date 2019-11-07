
#ifndef MATMATCPP_INPUT_HPP_
#define MATMATCPP_INPUT_HPP_

#include <iostream>
#include <sstream>
#include <chrono>

struct InputParser{
  using scalar_t = double;

  int32_t numDofs_  = 0;
  int32_t romSize_  = 0;
  int32_t numRepli_ = 0;

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
	if (col1 == "numDofs"){
	  numDofs_ = std::stoi(col2);
	}
	if (col1 == "romSize"){
	  romSize_ = std::stoi(col2);
	}
	if (col1 == "numReplicas"){
	  numRepli_ = std::stoi(col2);
	}
      }
    source.close();

    std::cout << "numDofs = "		<< numDofs_	<< " \n"
	      << "romSize = "		<< romSize_     << " \n"
	      << "numRepl = "		<< numRepli_	<< " \n"
	      << std::endl;

    if (numDofs_==0){
      std::cerr << "Invalid # dofs " << std::endl;
      return 1;
    }
    if (romSize_==0){
      std::cerr << "Invalid romSize" << std::endl;
      return 1;
    }

    // // TO DO MAT MAT PRODUCT, I NEED TO HAVE rows of B = cols of A
    // if (matBrows_ != matAcols_){
    //   std::cerr << "Cannot do A*B if # rows in B != # cols in A" << std::endl;
    //   return 1;
    // }

    return 0;
  }
};

#endif
