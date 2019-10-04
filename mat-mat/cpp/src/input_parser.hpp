
#ifndef MATMATCPP_INPUT_HPP_
#define MATMATCPP_INPUT_HPP_

#include <iostream>
#include <sstream>
#include <chrono>

struct InputParser{
  using scalar_t = double;

  int32_t matArows_ = 0;
  int32_t matAcols_ = 0;
  int32_t matBrows_ = 0;
  int32_t matBcols_ = 0;
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
	if (col1 == "matArows"){
	  matArows_ = std::stoi(col2);
	}
	if (col1 == "matAcols"){
	  matAcols_ = std::stoi(col2);
	}
	if (col1 == "matBrows"){
	  matBrows_ = std::stoi(col2);
	}
	if (col1 == "matBcols"){
	  matBcols_ = std::stoi(col2);
	}
	if (col1 == "numReplicas"){
	  numRepli_ = std::stoi(col2);
	}
      }
    source.close();

    std::cout << "A rows = "		<< matArows_	<< " \n"
	      << "A cols = "		<< matAcols_	<< " \n"
	      << "B rows = "		<< matBrows_	<< " \n"
	      << "B cols = "		<< matBcols_	<< " \n"
	      << "numRepl = "		<< numRepli_	<< " \n"
	      << std::endl;

    if (matArows_==0){
      std::cerr << "Invalid # rows for A" << std::endl;
      return 1;
    }
    if (matAcols_==0){
      std::cerr << "Invalid # cols for A" << std::endl;
      return 1;
    }
    if (matBrows_==0){
      std::cerr << "Invalid # rows for B" << std::endl;
      return 1;
    }
    if (matBcols_==0){
      std::cerr << "Invalid # cols for B" << std::endl;
      return 1;
    }

    // TO DO MAT MAT PRODUCT, I NEED TO HAVE rows of B = cols of A
    if (matBrows_ != matAcols_){
      std::cerr << "Cannot do A*B if # rows in B != # cols in A" << std::endl;
      return 1;
    }

    return 0;
  }
};

#endif
