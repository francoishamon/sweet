/*
 * SWE_Plane_TS_ln_erk.hpp
 *
 *  Created on: 29 May 2017
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 */

#ifndef SRC_PROGRAMS_SWE_SPHERE_TS_INTERFACE_HPP_
#define SRC_PROGRAMS_SWE_SPHERE_TS_INTERFACE_HPP_

#include <limits>
#include <sweet/SimulationVariables.hpp>
#include <sweet/sphere/SphereData.hpp>
#include <sweet/sphere/SphereOperators.hpp>


class SWE_Sphere_TS_interface
{
public:
	virtual void run_timestep(
			SphereData &io_h,	///< prognostic variables
			SphereData &io_u,	///< prognostic variables
			SphereData &io_v,	///< prognostic variables

			double &o_dt,				///< time step restriction
			double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp,
			double i_max_simulation_time = std::numeric_limits<double>::infinity()
	) = 0;
};

#endif /* SRC_PROGRAMS_SWE_PLANE_REXI_SWE_PLANE_TS_LN_ERK_HPP_ */
