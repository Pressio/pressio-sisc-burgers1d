
#ifndef ADVECTION_FIELD_HPP_
#define ADVECTION_FIELD_HPP_

#include "UTILS_ALL"
#include <Kokkos_Core.hpp>
#include <KokkosSparse_CrsMatrix.hpp>

template <typename state_t, typename sc_t>
class ManufacturedSolutionAdvection
{
  using this_t  = ManufacturedSolutionAdvection<state_t, sc_t>;
  using dummy_t = int;
  using cell_adv_arr_t  = std::array<sc_t, 2>;
  using cell_state_arr_t  = std::array<sc_t, 3>;

public:
  typename std::conditional<
    ::pressio::containers::meta::is_vector_kokkos<state_t>::value,
    state_t, dummy_t>::type u_;

public:
  ManufacturedSolutionAdvection() = default;

  template <
    typename _state_t = state_t,
    ::pressio::mpl::enable_if_t<
      ::pressio::containers::meta::is_vector_kokkos<_state_t>::value
      > * = nullptr
    >
  ManufacturedSolutionAdvection(_state_t uIn) : u_{uIn}{}

  KOKKOS_INLINE_FUNCTION
  void operator() (const int & i) const {}

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_adv_arr_t & vxy) const
  {
    vxy[0] = 1.0;
    vxy[1] = 1.0;
  }
};



template <typename state_t, typename sc_t>
class FourEddyModel
{
  using this_t  = FourEddyModel<state_t, sc_t>;
  using dummy_t = int;
  using cell_adv_arr_t  = std::array<sc_t, 2>;
  using cell_state_arr_t  = std::array<sc_t, 3>;

  // number of eddies
  static constexpr auto numEddies_ = 4;

  // scaling factor for eddy strength
  const sc_t A_ = 15.;

  const std::array<sc_t, numEddies_> eddiesOx_   = {{0.75, 0.25,  0.35,  0.85}};
  const std::array<sc_t, numEddies_> eddiesOy_   = {{0.25, 0.25, 0.7, 0.8}};
  const std::array<sc_t, numEddies_> eddiesR_    = {{0.1, 0.05, 0.07, 0.085}};
  const std::array<int,  numEddies_> eddiesSign_ = {{1, -1, -1, -1}};
  // store 1/2R_i^2 for each i
  std::array<sc_t, numEddies_> twoRiSqInv_;

public:
  static constexpr auto zero	= ::pressio::utils::constants::zero<sc_t>();
  static constexpr auto one	= ::pressio::utils::constants::one<sc_t>();
  static constexpr auto two	= ::pressio::utils::constants::two<sc_t>();

public:
  FourEddyModel(){
    for (auto i=0; i<numEddies_; ++i){
      twoRiSqInv_[i] = one/(two*eddiesR_[i]*eddiesR_[i]);
    }
  }

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_adv_arr_t & vxy) const
  {
    // compute vx
    vxy[0] = zero;
    vxy[1] = zero;
    for (auto i=0; i<numEddies_; ++i){
      auto dx = x - eddiesOx_[i];
      auto dy = y - eddiesOy_[i];
      auto expV = (dx*dx + dy*dy) * twoRiSqInv_[i];
      vxy[0] +=  eddiesSign_[i] * std::exp(-expV) * dy;
      vxy[1] += -eddiesSign_[i] * std::exp(-expV) * dx;
    }
    vxy[0] *= A_;
    vxy[1] *= A_;
  }
};



template <typename state_t, typename sc_t>
class CellularFlow
{
  using this_t  = CellularFlow<state_t, sc_t>;
  using dummy_t = int;
  using cell_adv_arr_t  = std::array<sc_t, 2>;
  using cell_state_arr_t  = std::array<sc_t, 3>;

public:
  static constexpr auto myPI_   = 3.14159265358979323846;

public:
  CellularFlow() = default;

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_adv_arr_t & vxy) const
  {
    vxy[0] = -myPI_*std::sin(myPI_*x)*std::cos(myPI_*y);
    vxy[1] = myPI_*std::cos(myPI_*x)*std::sin(myPI_*y);
  }
};

#endif
