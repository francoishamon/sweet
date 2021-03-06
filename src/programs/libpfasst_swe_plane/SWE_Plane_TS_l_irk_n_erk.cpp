/*
 * SWE_Plane_TS_l_irk_n_erk.cpp
 *
 *  Created on: 29 May 2017
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 *
 *  Changelog:
 *  	2017-05-29: Based on source swe_plane_rexi.cpp
 *					which was also written by Pedro Peixoto
 */

#include "SWE_Plane_TS_l_irk_n_erk.hpp"







/*
 * Main routine for method to be used in case of finite differences
 */
void SWE_Plane_TS_l_irk_n_erk::euler_timestep_update_nonlinear(
		const PlaneData &i_h,	///< prognostic variables
		const PlaneData &i_u,	///< prognostic variables
		const PlaneData &i_v,	///< prognostic variables

		PlaneData &o_h_t,	///< time updates
		PlaneData &o_u_t,	///< time updates
		PlaneData &o_v_t	///< time updates
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


void SWE_Plane_TS_l_irk_n_erk::run_timestep(
		PlaneData &io_h,	///< prognostic variables
		PlaneData &io_u,	///< prognostic variables
		PlaneData &io_v,	///< prognostic variables

		double &o_dt,			///< time step restriction
		double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp,
		double i_max_simulation_time
)
{
	if (i_fixed_dt <= 0)
		FatalError("SWE_Plane_TS_l_irk_n_erk: Only constant time step size allowed");

	if (i_simulation_timestamp + i_fixed_dt > i_max_simulation_time)
		i_fixed_dt = i_max_simulation_time-i_simulation_timestamp;

	o_dt = i_fixed_dt;


	PlaneData h_linear_t1 = io_h;
	PlaneData u_linear_t1 = io_u;
	PlaneData v_linear_t1 = io_v;

	ts_l_irk.run_timestep(
			h_linear_t1, u_linear_t1, v_linear_t1,
			o_dt,
			i_fixed_dt,
			i_simulation_timestamp,
			i_max_simulation_time
		);

	// compute non-linear tendencies at half time step
	PlaneData h_dt_nonlinear(op.planeDataConfig);
	PlaneData u_dt_nonlinear(op.planeDataConfig);
	PlaneData v_dt_nonlinear(op.planeDataConfig);

	// standard time stepping
	euler_timestep_update_nonlinear(
			io_h, io_u, io_v,
			h_dt_nonlinear, u_dt_nonlinear, v_dt_nonlinear
		);

	io_h = h_linear_t1 + h_dt_nonlinear*i_fixed_dt;
	io_u = u_linear_t1 + u_dt_nonlinear*i_fixed_dt;
	io_v = v_linear_t1 + v_dt_nonlinear*i_fixed_dt;
}



/*
 * Setup
 */
void SWE_Plane_TS_l_irk_n_erk::setup(
		int i_l_order,
		int i_n_order
)
{
	timestepping_order_linear = i_l_order;
	ts_l_irk.setup(timestepping_order_linear);

	if (timestepping_order_linear != 1)
		FatalError("SWE_Plane_TS_l_irk_n_erk: Only 1st order TS supported with this implementation");

	timestepping_order_nonlinear = i_l_order;
	timestepping_rk.setupBuffers(op.planeDataConfig, timestepping_order_nonlinear);
}


SWE_Plane_TS_l_irk_n_erk::SWE_Plane_TS_l_irk_n_erk(
		SimulationVariables &i_simVars,
		PlaneOperators &i_op
)	:
		simVars(i_simVars),
		op(i_op),
		ts_l_irk(simVars, op)
{
	setup(simVars.disc.timestepping_order, simVars.disc.timestepping_order2);
	//ts_l_irk.setup(simVars.disc.timestepping_order);
}



SWE_Plane_TS_l_irk_n_erk::~SWE_Plane_TS_l_irk_n_erk()
{
}

