
#ifndef ADVECTION_CELLULAR_FLOW_FUNCTOR_HPP_
#define ADVECTION_CELLULAR_FLOW_FUNCTOR_HPP_

#include "UTILS_ALL"
#include <Kokkos_Core.hpp>

template <typename sc_t, typename state_t = void>
class CellularFlow
{
  using this_t  = CellularFlow<sc_t, state_t>;
  using cell_adv_arr_t  = std::array<sc_t, 2>;
  using cell_state_arr_t  = std::array<sc_t, 3>;

public:
  static constexpr auto myPI_   = 3.14159265358979323846;
  // static constexpr auto two	= ::pressio::utils::constants::two<sc_t>();
  // static constexpr auto PI2_	= myPI_*two;

public:
  CellularFlow() = default;

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_adv_arr_t & vxy) const
  {
    vxy[0] = -myPI_*std::sin(myPI_*x)*std::cos(myPI_*y);
    vxy[1] =  myPI_*std::cos(myPI_*x)*std::sin(myPI_*y);
  }

  KOKKOS_INLINE_FUNCTION
  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const sc_t & u0,
		  const sc_t & u1,
		  const sc_t & u2,
		  sc_t & vx,
		  sc_t & vy) const
  {
    vx = -myPI_*std::sin(myPI_*x)*std::cos(myPI_*y);
    vy =  myPI_*std::cos(myPI_*x)*std::sin(myPI_*y);
  }
};

#endif
