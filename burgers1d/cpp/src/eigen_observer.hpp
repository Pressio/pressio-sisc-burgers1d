
#ifndef BURGERS1D_CPP_EIGEN_OBSERVER_HPP
#define BURGERS1D_CPP_EIGEN_OBSERVER_HPP

template <typename state_t>
struct EigenObserver{
  using matrix_t = Eigen::MatrixXd;
  using uint_t   = unsigned int;
  using scalar_t = double;

  uint_t numCell_ {};
  matrix_t A_;
  uint_t count_ = 0;
  state_t x0_;
  state_t xIncr_;
  int snapshotsFreq_ = {};

  EigenObserver(uint_t Nsteps, uint_t numCell,
	   const state_t & x0, int shapshotsFreq)
    : numCell_(numCell),
      x0_(x0),
      xIncr_(numCell),
      snapshotsFreq_(shapshotsFreq)
  {
    uint_t numCols = 0;
    // make sure number of steps is divisible by sampling frequency
    if ( Nsteps % shapshotsFreq == 0)
      numCols = Nsteps/shapshotsFreq;
    else
      throw std::runtime_error("Snapshot frequency not a divisor of steps");

    //resize matrix
    A_.resize(numCell_, numCols);
  }

  void operator()(uint_t step, scalar_t t, const state_t & x)
  {
    if ( step % snapshotsFreq_ == 0 and step > 0){
      xIncr_ = x - x0_;
      this->storeInColumn(xIncr_, count_);
      count_++;
    }
  }

  const matrix_t & viewSnapshots() const {
    return A_;
  }

  void printSnapshotsToFile(std::string fileName) const {
    std::ofstream file;
    file.open(fileName);
    for (uint_t i=0; i<A_.rows(); i++){
      for (uint_t j=0; j<A_.cols(); j++){
	file << std::fixed
	     << std::setprecision(15)
	     << A_(i,j) << " ";
      }
      file << std::endl;
    }
    file.close();
  }

private:
  void storeInColumn(const state_t & x, uint_t colIndex){
    for (uint_t i=0; i<numCell_; i++)
      A_(i, colIndex) = x(i);
  }
};

#endif
