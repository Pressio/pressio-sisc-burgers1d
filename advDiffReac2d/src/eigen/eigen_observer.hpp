
#ifndef ADR2D_CPP_EIGEN_OBSERVER_HPP
#define ADR2D_CPP_EIGEN_OBSERVER_HPP

#include <type_traits>

template <
  typename state_t,
  bool subtract_ref_state = false,
  ::pressio::mpl::enable_if_t<
    ::pressio::containers::meta::is_vector_wrapper_eigen<state_t>::value
    > * = nullptr
  >
struct EigenObserver{
  using matrix_t = Eigen::MatrixXd;
  using int_t    = int32_t;
  using scalar_t = double;

  int_t numStateDof_ {};
  matrix_t A_;
  int_t count_ = 0;
  state_t xRef_;
  state_t xIncr_;
  int_t snapshotsFreq_ = {};

  EigenObserver(int_t Nsteps,
		int_t numStateDof,
		const state_t & xRef,
		int shapshotsFreq)
    : numStateDof_(numStateDof),
      xRef_(xRef),
      xIncr_(numStateDof),
      snapshotsFreq_(shapshotsFreq)
  {
    int_t numCols = 0;
    // make sure number of steps is divisible by sampling frequency
    if ( Nsteps % shapshotsFreq == 0)
      numCols = Nsteps/shapshotsFreq;
    else
      throw std::runtime_error("Snapshot frequency not a divisor of steps");

    //resize matrix
    A_.resize(numStateDof_, numCols);
  }

  template <
    bool _subtract_ref_state = subtract_ref_state,
    typename std::enable_if<_subtract_ref_state>::type * = nullptr
    >
  void operator()(int_t step, scalar_t t, const state_t & x)
  {
    if ( step % snapshotsFreq_ == 0 and step > 0){
      xIncr_ = x - xRef_;
      this->storeInColumn(xIncr_, count_);
      count_++;
    }
  }

  template <
    bool _subtract_ref_state = subtract_ref_state,
    typename std::enable_if<!_subtract_ref_state>::type * = nullptr
    >
  void operator()(int_t step, scalar_t t, const state_t & x)
  {
    if ( step % snapshotsFreq_ == 0 and step > 0){
      this->storeInColumn(x, count_);
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
    for (int_t i=0; i<numStateDof_; i++)
      A_(i, colIndex) = x(i);
  }
};

#endif
