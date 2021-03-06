/*
 * SemiLangrangian.hpp
 *
 *  Created on: 5 Dec 2015
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk> Schreiber <schreiberx@gmail.com>
 */
#ifndef SRC_INCLUDE_SWEET_PLANEDATASEMILAGRANGIAN_HPP_
#define SRC_INCLUDE_SWEET_PLANEDATASEMILAGRANGIAN_HPP_

#include <sweet/plane/Convert_PlaneData_to_ScalarDataArray.hpp>
#include <sweet/plane/Convert_ScalarDataArray_to_PlaneData.hpp>
#include "PlaneData.hpp"
#include "PlaneDataSampler.hpp"
#include <sweet/ScalarDataArray.hpp>
#include <sweet/plane/Staggering.hpp>

class PlaneDataSemiLagrangian
{
	PlaneDataSampler sample2D;
	PlaneDataConfig *planeDataConfig;

public:
	PlaneDataSemiLagrangian()	:
		planeDataConfig(nullptr)
	{
	}


	void setup(
		double i_domain_size[2],
		PlaneDataConfig *i_planeDataConfig
	)
	{
		planeDataConfig = i_planeDataConfig;
		sample2D.setup(i_domain_size, planeDataConfig);
	}


	/**
	 * Stable extrapolation Two-Time-Level Scheme, Mariano Hortal,
	 *     Development and testing of a new two-time-level semi-lagrangian scheme (settls) in the ECMWF forecast model.
	 * Quaterly Journal of the Royal Meterological Society
	 *
	 * r_d = r_a - dt/2 * (2 * v_n(r_d) - v_{n-1}(r_d) + v_n(r_a))
	 *
	 * v^{iter} := (dt*v_n - dt*0.5*v_{n-1})
	 * r_d = r_a - dt/2 * v_n(r_d) - v^{iter}(r_d)
	 */
	void compute_departure_points_settls(
			PlaneData* i_velocity_field_t_prev[2],	///< velocity field at time n-1
			PlaneData* i_velocity_field_t[2],		///< velocity field at time n

			ScalarDataArray* i_pos_arrival[2],		///< position at time n+1
			double i_dt,							///< time step size

			ScalarDataArray* o_pos_departure[2],	///< departure points at time n,
			double *i_staggering = nullptr			///< staggering (ux, uy, vx, vy)
	)
	{
		if (i_staggering == nullptr)
		{
			static double constzerostuff[4] = {0,0,0,0};
			i_staggering = constzerostuff;
		}


		std::size_t num_points = i_pos_arrival[0]->number_of_elements;

		/**
		 * Convert velocity components at previous departure points
		 * to ScalarDataArray
		 */

		ScalarDataArray vx_n_prev = Convert_PlaneData_To_ScalarDataArray::physical_convert(*i_velocity_field_t_prev[0]);
		ScalarDataArray vy_n_prev = Convert_PlaneData_To_ScalarDataArray::physical_convert(*i_velocity_field_t_prev[1]);

		ScalarDataArray vx_n = Convert_PlaneData_To_ScalarDataArray::physical_convert(*i_velocity_field_t[0]);
		ScalarDataArray vy_n = Convert_PlaneData_To_ScalarDataArray::physical_convert(*i_velocity_field_t[1]);

		ScalarDataArray &rx_a = *i_pos_arrival[0];
		ScalarDataArray &ry_a = *i_pos_arrival[1];

		ScalarDataArray &rx_d = *o_pos_departure[0];
		ScalarDataArray &ry_d = *o_pos_departure[1];


		double dt = i_dt;

		ScalarDataArray vx_iter(num_points);
		ScalarDataArray vy_iter(num_points);

		vx_iter = dt * vx_n - dt*0.5 * vx_n_prev;
		vy_iter = dt * vy_n - dt*0.5 * vy_n_prev;

		ScalarDataArray rx_d_new(num_points);
		ScalarDataArray ry_d_new(num_points);

		ScalarDataArray rx_d_prev = rx_a;
		ScalarDataArray ry_d_prev = ry_a;	// TODO: is rx_a correct or should it be ry_a?

		//PlaneData* r_d[2] = {&rx_d, &ry_d};

		// initialize departure points with arrival points
		rx_d = rx_a;
		ry_d = ry_a;

		int iters;
		for (iters = 0; iters < 10; iters++)
		{
			// r_d = r_a - dt/2 * v_n(r_d) - v^{iter}(r_d)
			rx_d_new = rx_a - dt*0.5 * vx_n - sample2D.bilinear_scalar(
					Convert_ScalarDataArray_to_PlaneData::convert(vx_iter, planeDataConfig),
					rx_d, ry_d,
					i_staggering[0], i_staggering[1]
			);

			ry_d_new = ry_a - dt*0.5 * vy_n - sample2D.bilinear_scalar(
					Convert_ScalarDataArray_to_PlaneData::convert(vy_iter, planeDataConfig),
					rx_d, ry_d, i_staggering[2], i_staggering[3]);

			double diff = (rx_d_new - rx_d_prev).reduce_maxAbs() + (ry_d_new - ry_d_prev).reduce_maxAbs();
			rx_d_prev = rx_d_new;
			ry_d_prev = ry_d_new;

			for (	std::size_t i = 0;
					i < num_points;
					i++
			)
			{
				rx_d.scalar_data[i] = sample2D.wrapPeriodic(rx_d_new.scalar_data[i], sample2D.domain_size[0]);
				ry_d.scalar_data[i] = sample2D.wrapPeriodic(ry_d_new.scalar_data[i], sample2D.domain_size[1]);
			}

			if (diff < 1e-8)
				break;
		}
	}

	/**
	 * Stable extrapolation Two-Time-Level Scheme, Mariano Hortal,
	 *     Development and testing of a new two-time-level semi-lagrangian scheme (settls) in the ECMWF forecast model.
	 * Quaterly Journal of the Royal Meterological Society
	 *
	 * r_d = r_a - dt/2 * (2 * v_n(r_d) - v_{n-1}(r_d) + v_n(r_a))
	 *
	 * v^{iter} := (dt*v_n - dt*0.5*v_{n-1})
	 * r_d = r_a - dt/2 * v_n(r_d) - v^{iter}(r_d)
	 */
	void semi_lag_departure_points_settls(
			const PlaneData &i_u_prev,	// Velocities at time t-1
			const PlaneData &i_v_prev,
			const PlaneData &i_u, 		// Velocities at time t
			const PlaneData &i_v,

			const ScalarDataArray &i_posx_a,	// Position of arrival points x / y
			const ScalarDataArray &i_posy_a,

			double i_dt,				///< time step size
			ScalarDataArray &o_posx_d, 	///< Position of departure points x / y
			ScalarDataArray &o_posy_d,

			const Staggering &i_staggering	///< staggering, if any (ux, uy, vx, vy)
	)
	{
		std::size_t num_points = i_posx_a.number_of_elements;

		ScalarDataArray u_prev = Convert_PlaneData_To_ScalarDataArray::physical_convert(i_u_prev);
		ScalarDataArray v_prev = Convert_PlaneData_To_ScalarDataArray::physical_convert(i_v_prev);

		ScalarDataArray u = Convert_PlaneData_To_ScalarDataArray::physical_convert(i_u);
		ScalarDataArray v = Convert_PlaneData_To_ScalarDataArray::physical_convert(i_v);

		//local dt
		double dt = i_dt;

		//Velocity for iterations
		ScalarDataArray u_iter = dt * u - dt*0.5 * u_prev;
		ScalarDataArray v_iter = dt * v - dt*0.5 * v_prev;

		//Departure point tmp
		ScalarDataArray rx_d_new(num_points);
		ScalarDataArray ry_d_new(num_points);

		//Previous departure point
		ScalarDataArray rx_d_prev = i_posx_a;
		ScalarDataArray ry_d_prev = i_posy_a;

		// initialize departure points with arrival points
		o_posx_d = i_posx_a;
		o_posy_d = i_posy_a;

		int iters = 0;
		for (; iters < 10; iters++)
		{
			//std::cout<<iters<<std::endl;
			// r_d = r_a - dt/2 * v_n(r_d) - v^{iter}(r_d)
			rx_d_new = i_posx_a - dt*0.5 * u - sample2D.bilinear_scalar(
					Convert_ScalarDataArray_to_PlaneData::convert(u_iter, planeDataConfig),
					o_posx_d, o_posy_d, i_staggering.u[0], i_staggering.u[1]
			);
			ry_d_new = i_posy_a - dt*0.5 * v - sample2D.bilinear_scalar(
					Convert_ScalarDataArray_to_PlaneData::convert(v_iter, planeDataConfig),
					o_posx_d, o_posy_d, i_staggering.v[0], i_staggering.v[1]
			);

			double diff = (rx_d_new - rx_d_prev).reduce_maxAbs() + (ry_d_new - ry_d_prev).reduce_maxAbs();
			rx_d_prev = rx_d_new;
			ry_d_prev = ry_d_new;

			for (std::size_t i = 0; i < num_points; i++)
			{
				o_posx_d.scalar_data[i] = sample2D.wrapPeriodic(rx_d_new.scalar_data[i], sample2D.domain_size[0]);
				o_posy_d.scalar_data[i] = sample2D.wrapPeriodic(ry_d_new.scalar_data[i], sample2D.domain_size[1]);
			}

			if (diff < 1e-8)
			   break;
		}
	}
};

#endif /* SRC_INCLUDE_SWEET_PLANEDATASEMILAGRANGIAN_HPP_ */
