
#ifndef ADR2D_CPP_KOKKOS_OBSERVER_HPP
#define ADR2D_CPP_KOKKOS_OBSERVER_HPP

#include <type_traits>
#include "MPL_ALL"
#include "CONTAINERS_ALL"

template <
  typename state_t,
  bool subtract_ref_state = false,
  ::pressio::mpl::enable_if_t<
    ::pressio::containers::meta::is_vector_wrapper_kokkos<state_t>::value
    > * = nullptr
  >
struct KokkosObserver{
  using matrix_t = Eigen::MatrixXd;
  using uint_t   = unsigned int;
  using scalar_t = double;

  uint_t numStateDof_ {};
  matrix_t A_;
  uint_t count_ = 0;
  state_t xRef_;
  state_t xIncr_;
  int snapshotsFreq_ = {};

  KokkosObserver(uint_t Nsteps,
		 uint_t numStateDof,
		 const state_t & xRef,
		 int shapshotsFreq)
    : numStateDof_(numStateDof),
      xRef_(xRef), // rem. that pressio kokkos wrapper DEEP copy constructs
      xIncr_("xIncr", numStateDof),
      snapshotsFreq_(shapshotsFreq)
  {
    uint_t numCols = 0;
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
  void operator()(uint_t step, scalar_t t, const state_t & x)
  {
    constexpr auto one	  = ::pressio::utils::constants::one<scalar_t>();
    constexpr auto negOne = -one;

    if ( step % snapshotsFreq_ == 0 and step > 0){
      //xIncr_ = x - xRef_;
      ::pressio::containers::ops::do_update(xIncr_, x, one, xRef_, negOne);
      this->storeInColumn(xIncr_, count_);
      count_++;
    }
  }

  template <
    bool _subtract_ref_state = subtract_ref_state,
    typename std::enable_if<!_subtract_ref_state>::type * = nullptr
    >
  void operator()(uint_t step, scalar_t t, const state_t & x)
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
    using kokkos_view_d_t = typename ::pressio::containers::details::traits<state_t>::wrapped_t;
    using kokkos_view_h_t = typename kokkos_view_d_t::host_mirror_type;

    //create a host view and deep copy
    kokkos_view_h_t xhv("xhv", numStateDof_);
    Kokkos::deep_copy(xhv, *x.data());

    for (uint_t i=0; i<numStateDof_; i++)
      A_(i, colIndex) = xhv(i);
  }
};

#endif
