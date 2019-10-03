
#ifndef BURGERS1D_CPP_EIGEN_OBSERVER_HPP
#define BURGERS1D_CPP_EIGEN_OBSERVER_HPP

template <typename state_t>
struct EigenObserver
{
  using scalar_t = double;
  using matrix_t = Eigen::Matrix<scalar_t, -1, -1>;
  using int_t    = int32_t;

  int_t numCell_ {};
  matrix_t A_;
  int_t count_ = 0;
  state_t xRef_;
  state_t xIncr_;
  int snapshotsFreq_ = {};

  EigenObserver(int_t Nsteps, int_t numCell,
		const state_t & xRef, int shapshotsFreq)
    : numCell_(numCell),
      xRef_(xRef),
      xIncr_(numCell),
      snapshotsFreq_(shapshotsFreq)
  {
    int_t numCols = 0;
    // make sure number of steps is divisible by sampling frequency
    if ( Nsteps % shapshotsFreq == 0)
      numCols = Nsteps/shapshotsFreq;
    else
      throw std::runtime_error("Snapshot frequency not a divisor of steps");

    //resize matrix
    A_.resize(numCell_, numCols);
  }

  void operator()(int_t step, scalar_t t, const state_t & x)
  {
    // we do not keep the step = 0
    if ( step % snapshotsFreq_ == 0 and step > 0){
      xIncr_ = x - xRef_;
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
    for (int_t i=0; i<A_.rows(); i++){
      for (int_t j=0; j<A_.cols(); j++){
	file << std::fixed
	     << std::setprecision(15)
	     << A_(i,j) << " ";
      }
      file << std::endl;
    }
    file.close();
  }

private:
  void storeInColumn(const state_t & x, int_t colIndex){
    for (int_t i=0; i<numCell_; i++)
      A_(i, colIndex) = x(i);
  }
};

#endif
