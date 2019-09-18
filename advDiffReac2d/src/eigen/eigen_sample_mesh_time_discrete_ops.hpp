
#ifndef EIGEN_SAMPLE_MESH_TIME_DISCRETE_OPS_HPP_
#define EIGEN_SAMPLE_MESH_TIME_DISCRETE_OPS_HPP_

#include <Eigen/Core>

template <typename fom_t>
struct time_discrete_ops
{
  using scalar_t	 = typename fom_t::scalar_type;
  using residual_t	 = typename fom_t::state_type;
  using state_t		 = typename fom_t::state_type;
  using velocity_t	 = typename fom_t::state_type;
  using dmatrix_t	 = typename fom_t::dmatrix_type;
  using connectivity_t	 = typename fom_t::connectivity_t;

  const int numSpecies_ = fom_t::numSpecies_;
  const connectivity_t & graph_;

  // this is because below we unroll the loop for Dofs
  // so it is hardwired
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

    // loop over the mesh cells where the residual needs to be computed.
    // for the sample mesh, this is only a subset of the mesh and this info
    // is contained within the graph
    for (size_t iPt=0; iPt < graph_.size(); ++iPt)
    {
      // get sample mesh GID of the current cell. This GID identifies this cell
      // within all the cells in the sample mesh.
      const auto & cellGID  = graph_[iPt][0];

      /* Remember that for sample mesh, we always have the state vector to be
       * larger than the residual vectors because to compute the residual at
       * a given cell, we need the state at neighbording points.
       * The number of neighboring points where we need the state is determined
       * by the stencil chosen for the numerical method.
       * Why? becaue assume that we have a 2nd FD stencil and a sample mesh with
       * 4 total cells, i.e. something like:
       *       o
       *    o  x  o
       *       o
       * where o=cell with state values only, while x=cell with state and residual.
       * Suppose that the cells are enumerate as:
       *       4
       *    1  2  3
       *       0
       * And suppose we have 2 dofs for each cell, then the state vector is:
       *
       *  state = [ c0_0, c1_0, c0_1, c1_1, c0_2, c1_2, c0_3, c1_3, c0_4, c1_4 ]
       *
       * where ci_j = the i-th dof at the j cell, and the residual vector is:
       *
       * R = [ r0_2, r1_2 ]
       *
       * where ri_j = the residual for the i-th dof at the j cell.
       *
       * Keeping this in mind, given a cell where I want to compute residual at,
       * I need to find, for each dof in this cell, the corresponding index in
       * the residual vector and those in the state vector.
      */
      // find which index in the RESIDUAL vector is associated with the
      // first species in this cell
      const auto rIndex = iPt*numSpecies_;

      // find which index in the STATE vector is associated with the
      // first species in this cell
      const auto stateIndex = cellGID*numSpecies_;

      R[rIndex]	  = xn[stateIndex]   - xnm1[stateIndex]   - dt*R[rIndex];
      R[rIndex+1] = xn[stateIndex+1] - xnm1[stateIndex+1] - dt*R[rIndex+1];
      R[rIndex+2] = xn[stateIndex+2] - xnm1[stateIndex+2] - dt*R[rIndex+2];
    }
  }

  void time_discrete_jacobian(dmatrix_t & jphi,
			      const dmatrix_t & phi,
			      scalar_t prefactor,
			      scalar_t dt) const
  {
    // loop over cells where residual needs to be computed
    for (size_t iPt=0; iPt < graph_.size(); ++iPt)
    {
      // get GID of the current residual cell
      const auto & cellGID  = graph_[iPt][0];

      // find which index in the RESIDUAL vector is associated with the
      // first species in this cell
      const auto rIndex = iPt*numSpecies_;

      // find which index in the STATE vector is associated with the
      // first species in this cell
      const auto stateIndex = cellGID*numSpecies_;

      for (auto j=0; j<jphi.cols(); j++){
	jphi(rIndex, j)   = phi(stateIndex, j)   - prefactor*dt*jphi(rIndex,j);
	jphi(rIndex+1, j) = phi(stateIndex+1, j) - prefactor*dt*jphi(rIndex+1,j);
	jphi(rIndex+2, j) = phi(stateIndex+2, j) - prefactor*dt*jphi(rIndex+2,j);
      }
    }
  }
};

#endif
