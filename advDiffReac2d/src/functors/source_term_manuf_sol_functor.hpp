
#ifndef SOURCE_TERM_MANUF_SOLUTION_FUNCTOR_HPP_
#define SOURCE_TERM_MANUF_SOLUTION_FUNCTOR_HPP_

#include "MPL_ALL"
#include "UTILS_ALL"
#include "CONTAINERS_VECTOR"
#include <Kokkos_Core.hpp>

template <typename sc_t, typename state_t = void>
class ManufacturedSolutionSource
{
  /*
   * this source was manufactured from a solution
   * c0(x,y,t) = t * sin(2 pi x) * cos(4 pi y)
   * c1(x,y,t) = t * sin(4 pi x) * sin(4 pi y)
   * c2(x,y,t) = t * sin(8 pi x) * cos(2 pi y)

   * for adr2d problem with pbc and chemistry c0 + c1 -> c2
   */

  using this_t		  = ManufacturedSolutionSource<sc_t, state_t>;
  using cell_src_arr_t	  = std::array<sc_t, 3>;
  using cell_src_jac_t    = std::array<cell_src_arr_t, 3>;
  using cell_state_arr_t  = std::array<sc_t, 3>;

  const sc_t D_ = {};

  static constexpr auto zero	= ::pressio::utils::constants::zero<sc_t>();
  static constexpr auto two	= ::pressio::utils::constants::two<sc_t>();
  static constexpr auto four	= ::pressio::utils::constants::four<sc_t>();
  static constexpr auto eight	= this_t::four * this_t::two;

  static constexpr auto myPI_		= 3.14159265358979323846;
  static constexpr auto PI2_		= this_t::two * myPI_;
  static constexpr auto PI4_		= this_t::two * PI2_;
  static constexpr auto PI8_		= this_t::two * PI4_;
  static constexpr auto fourPISq_	= this_t::four * myPI_*myPI_;
  static constexpr auto fourSqPISq_	= this_t::four*this_t::four*myPI_*myPI_;
  static constexpr auto eightSqPISq_	= this_t::eight*this_t::eight*myPI_*myPI_;

public:
  ManufacturedSolutionSource() = delete;
  ManufacturedSolutionSource(sc_t D) : D_{D}{};

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
    const auto sin2pix = std::sin(this_t::PI2_*x);
    const auto cos2pix = std::cos(this_t::PI2_*x);
    const auto sin2piy = std::sin(this_t::PI2_*y);
    const auto cos2piy = std::cos(this_t::PI2_*y);

    const auto sin4pix = std::sin(this_t::PI4_*x);
    const auto cos4pix = std::cos(this_t::PI4_*x);
    const auto sin4piy = std::sin(this_t::PI4_*y);
    const auto cos4piy = std::cos(this_t::PI4_*y);

    const auto sin8pix = std::sin(this_t::PI8_*x);
    const auto cos8pix = std::cos(this_t::PI8_*x);

    const auto dc0dt = sin2pix*cos4piy;
    const auto dc1dt = sin4pix*sin4piy;
    const auto dc2dt = sin8pix*cos2piy;

    const auto dc0dx   =  this_t::PI2_	       * cos2pix * t * cos4piy;
    const auto dc0dxSq = -this_t::fourPISq_    * sin2pix * t * cos4piy;
    const auto dc0dy   = -this_t::PI4_	       * sin2pix * t * sin4piy;
    const auto dc0dySq = -this_t::fourSqPISq_  * sin2pix * t * cos4piy;
    src0 = dc0dt + dc0dx + dc0dy -D_*dc0dxSq - D_*dc0dySq;

    const auto dc1dx   =  this_t::PI4_	       * cos4pix * t * sin4piy;
    const auto dc1dxSq = -this_t::fourSqPISq_  * sin4pix * t * sin4piy;
    const auto dc1dy   = this_t::PI4_	       * sin4pix * t * cos4piy;
    const auto dc1dySq = -this_t::fourSqPISq_  * sin4pix * t * sin4piy;
    src1 = dc1dt + dc1dx + dc1dy -D_*dc1dxSq - D_*dc1dySq;

    const auto dc2dx   = this_t::PI8_	       * cos8pix * t * cos2piy;
    const auto dc2dxSq = -this_t::eightSqPISq_ * sin8pix * t * cos2piy;
    const auto dc2dy   = -this_t::PI2_	       * sin8pix * t * sin2piy;
    const auto dc2dySq = -this_t::fourPISq_    * sin8pix * t * cos2piy;
    src2 = dc2dt + dc2dx + dc2dy -D_*dc2dxSq - D_*dc2dySq;
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
    srcJ00 = srcJ01 = srcJ02 = this_t::zero;
    srcJ10 = srcJ11 = srcJ12 = this_t::zero;
    srcJ20 = srcJ21 = srcJ22 = this_t::zero;
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
  }
};

#endif
