
#ifndef SOURCE_TERM_CHEMABC_FUNCTOR_HPP_
#define SOURCE_TERM_CHEMABC_FUNCTOR_HPP_

#include "MPL_ALL"
#include "UTILS_ALL"
#include "CONTAINERS_VECTOR"
#include <Kokkos_Core.hpp>

template <typename sc_t, typename state_t = void>
class ChemistryABCSource
{
  using this_t		  = ChemistryABCSource<sc_t, state_t>;
  using cell_src_arr_t	  = std::array<sc_t, 3>;
  using cell_src_jac_t    = std::array<cell_src_arr_t, 3>;
  using cell_state_arr_t  = std::array<sc_t, 3>;

public:
  sc_t K_ = {};
  // radius where source for second species is active
  static constexpr auto radius_ = 0.15;
  const std::array<sc_t,2> origin_ = {{0.55, 0.35}};

public:
  static constexpr auto zero	= ::pressio::utils::constants::zero<sc_t>();
  static constexpr auto four	= ::pressio::utils::constants::four<sc_t>();
  static constexpr auto myPI_	  = 3.14159265358979323846;
  static constexpr auto PI4_	  = this_t::four * myPI_;

public:
  ChemistryABCSource(sc_t K) : K_{K}{};

  KOKKOS_INLINE_FUNCTION
  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const sc_t & u0,
		  const sc_t & u1,
		  const sc_t & u2,
		  sc_t & src0,
		  sc_t & src1,
		  sc_t & src2) const
  {
    // account for chemistry
    src0 = -K_*u0*u1;
    src1 = -K_*u0*u1;
    src2 = K_*u0*u1 - K_*u2;

    // species source only acts if t<1.0
    if (t<1.0){
      src0 += std::sin(PI4_*x)*std::cos(PI4_*y);
      const auto dx = x - origin_[0];
      const auto dy = y - origin_[1];
      const auto distance = dx*dx + dy*dy;
      src1 += ( std::sqrt(distance) <= radius_) ? 0.05 : 0.0;
    }
  }

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_src_arr_t & src) const
  {
    (*this)(x, y, t, u[0], u[1], u[2], src[0], src[1], src[2]);
  }


  KOKKOS_INLINE_FUNCTION
  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const sc_t & u0,
		  const sc_t & u1,
		  const sc_t & u2,
		  sc_t & srcJ00, sc_t & srcJ01, sc_t & srcJ02,
		  sc_t & srcJ10, sc_t & srcJ11, sc_t & srcJ12,
		  sc_t & srcJ20, sc_t & srcJ21, sc_t & srcJ22) const
  {
    // d_src0/dc0 d_src0/dc1 d_src0/dc2
    srcJ00 = -K_*u1; srcJ01 = -K_*u0; srcJ02 = zero;
    // d_src1/dc0 d_src1/dc1 d_src1/dc2
    srcJ10 = -K_*u1; srcJ11 = -K_*u0; srcJ12 = zero;
    // d_src2/dc0 d_src2/dc1 d_src2/dc2
    srcJ20 =  K_*u1; srcJ21 =  K_*u0; srcJ22 = -K_;
  }

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_src_jac_t & srcJ) const
  {
    (*this)(x, y, t, u[0], u[1], u[2],
	    srcJ[0][0], srcJ[0][1], srcJ[0][2],
	    srcJ[1][0], srcJ[1][1], srcJ[1][2],
	    srcJ[2][0], srcJ[2][1], srcJ[2][2]);

    // // d_src0/dc0 d_src0/dc1 d_src0/dc2
    // srcJ[0][0] = -K_*u[1]; srcJ[0][1] = -K_*u[0]; srcJ[0][2] = zero;
    // // d_src1/dc0 d_src1/dc1 d_src1/dc2
    // srcJ[1][0] = -K_*u[1]; srcJ[1][1] = -K_*u[0]; srcJ[1][2] = zero;
    // // d_src2/dc0 d_src2/dc1 d_src2/dc2
    // srcJ[2][0] =  K_*u[1]; srcJ[2][1] =  K_*u[0]; srcJ[2][2] = -K_;
  }
};

#endif
