/*
 * SWE_Plane_TS_l_erk_n_erk.cpp
 *
 *  Created on: 29 May 2017
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 *
 *  Changelog:
 *  	2017-05-29: Based on source swe_plane_rexi.cpp
 *					which was also written by Pedro Peixoto
 */

#include "SWE_Plane_TS_l_erk_n_erk.hpp"





void SWE_Plane_TS_l_erk_n_erk::euler_timestep_update_linear(
		const PlaneData &i_h,	///< prognostic variables
		const PlaneData &i_u,	///< prognostic variables
		const PlaneData &i_v,	///< prognostic variables

		PlaneData &o_h_t,	///< time updates
		PlaneData &o_u_t,	///< time updates
		PlaneData &o_v_t,	///< time updates

		double i_simulation_timestamp
)
{
	/*
	 * linearized non-conservative (advective) formulation:
	 *
	 * h_t = -h0*u_x - h0*v_ym
	 * u_t = -g * h_x + f*v
	 * v_t = -g * h_y - f*u
	 */
	o_h_t = -(op.diff_c_x(i_u) + op.diff_c_y(i_v))*simVars.sim.h0;
	o_u_t = -simVars.sim.gravitation*op.diff_c_x(i_h) + simVars.sim.f0*i_v;
	o_v_t = -simVars.sim.gravitation*op.diff_c_y(i_h) - simVars.sim.f0*i_u;
}



void SWE_Plane_TS_l_erk_n_erk::euler_timestep_update_nonlinear(
		const PlaneData &i_h,	///< prognostic variables
		const PlaneData &i_u,	///< prognostic variables
		const PlaneData &i_v,	///< prognostic variables

		PlaneData &o_h_t,	///< time updates
		PlaneData &o_u_t,	///< time updates
		PlaneData &o_v_t,	///< time updates

		double i_simulation_timestamp
)
{
	/*
	 * non-conservative (advective) formulation:
	 *
	 *	h_t = -(u*h)_x - (v*h)_y
	 *	u_t = -g * h_x - u * u_x - v * u_y + f*v
	 *	v_t = -g * h_y - u * v_x - v * v_y - f*u
	 */
	o_h_t = -op.diff_c_x(i_u*i_h) - op.diff_c_y(i_v*i_h);
	o_u_t = -i_u*op.diff_c_x(i_u) - i_v*op.diff_c_y(i_u);
	o_v_t = -i_u*op.diff_c_x(i_v) - i_v*op.diff_c_y(i_v);
}




void SWE_Plane_TS_l_erk_n_erk::euler_timestep_update(
		const PlaneData &i_h,	///< prognostic variables
		const PlaneData &i_u,	///< prognostic variables
		const PlaneData &i_v,	///< prognostic variables

		PlaneData &o_h_t,	///< time updates
		PlaneData &o_u_t,	///< time updates
		PlaneData &o_v_t,	///< time updates

		double &o_dt,			///< time step restriction
		double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp
)
{
	if (i_fixed_dt <= 0)
		FatalError("SWE_Plane_TS_l_erk_n_erk: Only constant time step size allowed");

	o_dt = i_fixed_dt;

	PlaneData h_linear_dt(op.planeDataConfig);
	PlaneData u_linear_dt(op.planeDataConfig);
	PlaneData v_linear_dt(op.planeDataConfig);

	euler_timestep_update_linear(
			o_h_t, o_u_t, o_v_t,
			h_linear_dt, u_linear_dt, v_linear_dt,
			i_simulation_timestamp
		);

	PlaneData h_nonlinear_dt(op.planeDataConfig);
	PlaneData u_nonlinear_dt(op.planeDataConfig);
	PlaneData v_nonlinear_dt(op.planeDataConfig);

	euler_timestep_update_nonlinear(
			o_h_t, o_u_t, o_v_t,
			h_nonlinear_dt, u_nonlinear_dt, v_nonlinear_dt,
			i_simulation_timestamp
		);

	o_h_t = i_fixed_dt*(h_linear_dt + h_nonlinear_dt);
	o_u_t = i_fixed_dt*(u_linear_dt + u_nonlinear_dt);
	o_v_t = i_fixed_dt*(v_linear_dt + v_nonlinear_dt);
}



void SWE_Plane_TS_l_erk_n_erk::run_timestep(
		PlaneData &io_h,	///< prognostic variables
		PlaneData &io_u,	///< prognostic variables
		PlaneData &io_v,	///< prognostic variables

		double &o_dt,			///< time step restriction
		double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp,
		double i_max_simulation_time
)
{

	// standard time stepping
	timestepping_rk.run_timestep(
			this,
			&SWE_Plane_TS_l_erk_n_erk::euler_timestep_update,	///< pointer to function to compute euler time step updates
			io_h, io_u, io_v,
			o_dt,
			i_fixed_dt,
			timestepping_order,
			i_simulation_timestamp,
			i_max_simulation_time
		);
}





/*
 * Setup
 */
void SWE_Plane_TS_l_erk_n_erk::setup(
		int i_order	///< order of RK time stepping method
)
{
	timestepping_order = i_order;
	timestepping_rk.setupBuffers(op.planeDataConfig, timestepping_order);

	if (simVars.disc.use_staggering)
		FatalError("SWE_Plane_TS_l_erk_n_erk: Staggering not supported");
}


SWE_Plane_TS_l_erk_n_erk::SWE_Plane_TS_l_erk_n_erk(
		SimulationVariables &i_simVars,
		PlaneOperators &i_op
)	:
		simVars(i_simVars),
		op(i_op)
{
	setup(simVars.disc.timestepping_order);
}



SWE_Plane_TS_l_erk_n_erk::~SWE_Plane_TS_l_erk_n_erk()
{
}
