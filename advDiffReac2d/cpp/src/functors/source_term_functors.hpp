
#ifndef SOURCE_TERMS_HPP_
#define SOURCE_TERMS_HPP_

#include "MPL_ALL"
#include "UTILS_ALL"
#include "CONTAINERS_VECTOR"
#include <Kokkos_Core.hpp>
#include <KokkosSparse_CrsMatrix.hpp>

template <typename state_t, typename sc_t>
class ManufacturedSolutionSource
{
  using this_t		  = ManufacturedSolutionSource<state_t, sc_t>;
  using dummy_t		  = int;
  using cell_src_arr_t	  = std::array<sc_t, 3>;
  using cell_src_jac_t    = std::array<cell_src_arr_t, 3>;
  using cell_state_arr_t  = std::array<sc_t, 3>;

public:
  sc_t D_ = {};

  typename std::conditional<
    ::pressio::containers::meta::is_vector_kokkos<state_t>::value,
    state_t, dummy_t>::type u_;

public:
  static constexpr auto zero	= ::pressio::utils::constants::zero<sc_t>();
  static constexpr auto two	= ::pressio::utils::constants::two<sc_t>();
  static constexpr auto four	= ::pressio::utils::constants::four<sc_t>();
  static constexpr auto eight	= this_t::four * this_t::two;

  static constexpr auto myPI_	  = 3.14159265358979323846;
  static constexpr auto PI2_	  = this_t::two  * myPI_;
  static constexpr auto PI4_	  = this_t::four * myPI_;
  static constexpr auto PI8_	  = this_t::two  * this_t::four * myPI_;
  static constexpr auto fourPISq_ = this_t::four * myPI_*myPI_;
  static constexpr auto fourSqPISq_ = this_t::four*this_t::four*myPI_*myPI_;
  static constexpr auto eightSqPISq_ = this_t::eight*this_t::eight*myPI_*myPI_;

public:
  ManufacturedSolutionSource(sc_t D) : D_{D}{};

  template <
    typename _state_t = state_t,
    ::pressio::mpl::enable_if_t<
      ::pressio::containers::meta::is_vector_kokkos<_state_t>::value
      > * = nullptr
    >
  ManufacturedSolutionSource(sc_t D, _state_t uIn) : u_{uIn}{}

  KOKKOS_INLINE_FUNCTION
  void operator() (const int & i) const {}

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_src_arr_t & src) const
  {
    /*
     * this source was manufactured from a solution
     * c1(x,y,t) = t * sin(2 pi x) * cos(4 pi y)
     * c2(x,y,t) = t * sin(4 pi x) * sin(4 pi y)
     * c3(x,y,t) = t * sin(8 pi x) * cos(2 pi y)
     and you need to set K_ = 0
    */

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

    const auto c1 = sin2pix*t*cos4piy;
    const auto c2 = sin4pix*t*sin4piy;
    const auto c3 = sin8pix*t*cos2piy;

    const auto dc1dt = sin2pix*cos4piy;
    const auto dc2dt = sin4pix*sin4piy;
    const auto dc3dt = sin8pix*cos2piy;

    const auto dc1dx   = this_t::PI2_ *cos2pix*t*cos4piy;
    const auto dc1dxSq = -this_t::fourPISq_ *sin2pix*t*cos4piy;
    const auto dc1dy   = -this_t::PI4_*sin2pix*t*sin4piy;
    const auto dc1dySq = -this_t::fourSqPISq_ *sin2pix*t*cos4piy;
    src[0] = dc1dt + dc1dx + dc1dy -D_*dc1dxSq - D_*dc1dySq;

    const auto dc2dx   = this_t::PI4_ *cos4pix*t*sin4piy;
    const auto dc2dxSq = -this_t::fourSqPISq_ *sin4pix*t*sin4piy;
    const auto dc2dy   = this_t::PI4_*sin4pix*t*cos4piy;
    const auto dc2dySq = -this_t::fourSqPISq_ *sin4pix*t*sin4piy;
    src[1] = dc2dt + dc2dx + dc2dy -D_*dc2dxSq - D_*dc2dySq;

    const auto dc3dx   = this_t::PI8_ *cos8pix*t*cos2piy;
    const auto dc3dxSq = -this_t::eightSqPISq_ *sin8pix*t*cos2piy;
    const auto dc3dy   = -this_t::PI2_*sin8pix*t*sin2piy;
    const auto dc3dySq = -this_t::fourPISq_ *sin8pix*t*cos2piy;
    src[2] = dc3dt + dc3dx + dc3dy -D_*dc3dxSq - D_*dc3dySq;
  }

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_src_jac_t & srcJ) const
  {
    for (auto & ir : srcJ)
      for (auto & ic : ir)
	ic = this_t::zero;
  }
};



template <typename state_t, typename sc_t>
class ChemistryABCSource
{
  using this_t		  = ChemistryABCSource<state_t, sc_t>;
  using dummy_t		  = int;
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

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_src_arr_t & src) const
  {
    // account for chemistry
    src[0] = -K_*u[0]*u[1];
    src[1] = -K_*u[0]*u[1];
    src[2] = K_*u[0]*u[1] - K_*u[2];

    // species source only acts if t<1.0
    if (t<1.0){
      src[0] += std::sin(PI4_*x)*std::cos(PI4_*y);
      const auto dx = x - origin_[0];
      const auto dy = y - origin_[1];
      const auto distance = dx*dx + dy*dy;
      src[1] += ( std::sqrt(distance) <= radius_) ? 0.05 : 0.0;
      // no source for last species
    }
  }

  void operator()(const sc_t & x,
		  const sc_t & y,
		  const sc_t & t,
		  const cell_state_arr_t & u,
		  cell_src_jac_t & srcJ) const
  {
    // d_src0/dc0 d_src0/dc1 d_src0/dc2
    srcJ[0][0] = -K_*u[1]; srcJ[0][1] = -K_*u[0]; srcJ[0][2] = zero;
    // d_src1/dc0 d_src1/dc1 d_src1/dc2
    srcJ[1][0] = -K_*u[1]; srcJ[1][1] = -K_*u[0]; srcJ[1][2] = zero;
    // d_src2/dc0 d_src2/dc1 d_src2/dc2
    srcJ[2][0] =  K_*u[1]; srcJ[2][1] =  K_*u[0]; srcJ[2][2] = -K_;
  }
};

#endif
