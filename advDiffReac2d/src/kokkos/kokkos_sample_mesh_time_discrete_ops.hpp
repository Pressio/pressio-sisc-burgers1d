
#ifndef KOKKOS_SAMPLE_MESH_TIME_DISCRETE_OPS_HPP_
#define KOKKOS_SAMPLE_MESH_TIME_DISCRETE_OPS_HPP_

template <
  typename view_t, typename graph_t, typename sc_t, int numSpecies
  >
struct TimeDiscreteEulerFunctor
{
  graph_t graph_ = {};
  view_t R_      = {};
  view_t xn_     = {};
  view_t xnm1_   = {};
  sc_t dt_       = {};

  TimeDiscreteEulerFunctor(graph_t graph,
			   view_t R, view_t xn,
			   view_t xnm1, sc_t dt)
    : graph_{graph}, R_{R}, xn_{xn}, xnm1_{xnm1}, dt_{dt}{}

  KOKKOS_INLINE_FUNCTION
  void operator() (const int iPt) const
  {
    const auto & cellGID  = graph_(iPt,0);
    const auto rIndex = iPt*numSpecies;
    const auto stateIndex = cellGID*numSpecies;
    R_[rIndex]	 = xn_[stateIndex]   - xnm1_[stateIndex]   - dt_*R_[rIndex];
    R_[rIndex+1] = xn_[stateIndex+1] - xnm1_[stateIndex+1] - dt_*R_[rIndex+1];
    R_[rIndex+2] = xn_[stateIndex+2] - xnm1_[stateIndex+2] - dt_*R_[rIndex+2];
  }
};



template <
  typename dmat_t, typename graph_t, typename sc_t, int numSpecies
  >
struct TimeDiscreteJacobianFunctor
{
  graph_t graph_  = {};
  dmat_t jphi_	  = {};
  dmat_t phi_	  = {};
  sc_t prefactor_ = {};
  sc_t dt_	  = {};

  TimeDiscreteJacobianFunctor(graph_t graph,
			      dmat_t jphi,
			      dmat_t phi,
			      sc_t prefactor,
			      sc_t dt)
    : graph_{graph}, jphi_{jphi}, phi_{phi}, prefactor_{prefactor}, dt_{dt}{}

  KOKKOS_INLINE_FUNCTION
  void operator() (const int iPt) const
  {
    const auto & cellGID  = graph_(iPt,0);
    const auto rIndex = iPt*numSpecies;
    const auto stateIndex = cellGID*numSpecies;

    for (auto j=0; j<jphi_.extent(1); j++){
      jphi_(rIndex, j)   = phi_(stateIndex, j)   - prefactor_*dt_*jphi_(rIndex,j);
      jphi_(rIndex+1, j) = phi_(stateIndex+1, j) - prefactor_*dt_*jphi_(rIndex+1,j);
      jphi_(rIndex+2, j) = phi_(stateIndex+2, j) - prefactor_*dt_*jphi_(rIndex+2,j);
    }
  }
};



template <typename fom_t>
struct time_discrete_ops
{
  // remember that what we do here is on the DEVICE

  using scalar_t	 = typename fom_t::scalar_type;
  using residual_t	 = typename fom_t::state_type;
  using state_t		 = typename fom_t::state_type;
  using velocity_t	 = typename fom_t::state_type;
  using dmatrix_t	 = typename fom_t::mvec_t;
  using connectivity_t	 = typename fom_t::connectivity_d_t;

  const connectivity_t & graph_;

  // because below we unroll the loop for Dofs so it is hardwired
  static_assert( fom_t::numSpecies_==3 );

  // constructor
  time_discrete_ops(const connectivity_t & graph)
    : graph_{graph}{}

  void time_discrete_euler(residual_t & R,
  			   const state_t & xn,
  			   const state_t & xnm1,
  			   scalar_t dt) const
  {
    /*
     * On input:
     * - R	= contains the application velocity, i.e. f(...)
     * - xn	= the state at time step n
     * - xnm1	= state at time step n-1
     * Note that:
     * (a) R and y (might) have diffferent sizes! this is the whole point of doing this.
     * (b) xn and xnm1 have for sure the same size
     * this method is supposed to compute the residual for Euler backward, i.e.:
     *		R = xn - xnm1 - dr*R
     *
     * On exit, R contains the residual for BDF1
     */
    using fnctr_t = TimeDiscreteEulerFunctor<state_t, connectivity_t, scalar_t, fom_t::numSpecies_>;
    fnctr_t F(graph_, R, xn, xnm1, dt);
    Kokkos::parallel_for(graph_.extent(0), F);
  }

  void time_discrete_jacobian(dmatrix_t & jphi,
  			      const dmatrix_t & phi,
  			      scalar_t prefactor,
  			      scalar_t dt) const
  {
    using fnctr_t = TimeDiscreteJacobianFunctor<dmatrix_t, connectivity_t, scalar_t, fom_t::numSpecies_>;
    fnctr_t F(graph_, jphi, phi, prefactor, dt);
    Kokkos::parallel_for(graph_.extent(0), F);
  }

};

#endif
