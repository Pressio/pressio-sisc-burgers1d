
#ifndef ADVECTION_MANUF_SOL_FUNCTOR_HPP_
#define ADVECTION_MANUF_SOL_FUNCTOR_HPP_

#include "UTILS_ALL"
#include <Kokkos_Core.hpp>

template <typename sc_t, typename state_t = void>
class ManufacturedSolutionAdvection
{
  using this_t		  = ManufacturedSolutionAdvection<sc_t, state_t>;
  using cell_adv_arr_t    = std::array<sc_t, 2>;
  using cell_state_arr_t  = std::array<sc_t, 3>;

public:
  ManufacturedSolutionAdvection() = default;

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_adv_arr_t & vxy) const
  {
    constexpr auto one	= ::pressio::utils::constants::one<sc_t>();
    vxy[0] = one;
    vxy[1] = one;
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
    constexpr auto one	= ::pressio::utils::constants::one<sc_t>();
    vx = one;
    vy = one;
  }
};

#endif
