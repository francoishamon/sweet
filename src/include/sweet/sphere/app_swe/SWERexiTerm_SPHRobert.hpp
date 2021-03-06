/*
 * SWE_REXI_SPH.hpp
 *
 *  Created on: 30 Aug 2016
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 */

#ifndef SRC_SWEREXI_SPHROBERT_HPP_
#define SRC_SWEREXI_SPHROBERT_HPP_

#include <complex>
#include <sweet/sphere/SphereData.hpp>
#include <sweet/sphere/SphereDataConfig.hpp>
#include <sweet/sphere/SphereDataPhysicalComplex.hpp>
#include <sweet/sphere/SphereOperators.hpp>
#include <sweet/sphere/SphereOperatorsComplex.hpp>
#include <sweet/sphere/SphereDataConfig.hpp>

#include <sweet/sphere/Convert_SphereData_to_SphereDataComplex.hpp>
#include <sweet/sphere/Convert_SphereDataComplex_to_SphereData.hpp>
#include <sweet/sphere/app_swe/SWESphBandedMatrixPhysicalComplex.hpp>



/**
 * REXI solver for SWE based on Robert function formulation
 */
class SWERexiTerm_SPHRobert
{
	/// SPH configuration
//	SphereDataConfig *sphereDataConfig;

	/// SPH configuration
	SphereDataConfig *sphereDataConfigSolver;

	/// Solver for given alpha
	SphBandedMatrixPhysicalComplex< std::complex<double> > sphSolverPhi;
	SphBandedMatrixPhysicalComplex< std::complex<double> > sphSolverVel;

	SphereOperators op;
	SphereOperatorsComplex opComplex;

	/// scalar in front of RHS
	std::complex<double> rhs_scalar;

	/// REXI alpha
	std::complex<double> alpha;

	/// REXI beta
	std::complex<double> beta;


	/// timestep size
	double timestep_size;

	/// earth radius
	double r;

	/// inverse of earth radius
	double inv_r;

	/// Coriolis omega
	double two_coriolis_omega;

	/// f0
	double f0;

	bool use_f_sphere;

	/// Average geopotential
	double gh;

	SphereDataPhysicalComplex mug;

public:
	SWERexiTerm_SPHRobert()	:
//		sphereDataConfig(nullptr),
		sphereDataConfigSolver(nullptr)
	{
	}




	/**
	 * Setup the SWE REXI solver with SPH
	 */
	void setup_vectorinvariant_progphivortdiv(
			SphereDataConfig *i_sphereDataConfigSolver,

			const std::complex<double> &i_alpha,
			const std::complex<double> &i_beta,

			double i_radius,
			double i_coriolis_omega,
			double i_f0,
			double i_avg_geopotential,
			double i_timestep_size,

			bool i_use_f_sphere
	)
	{
		sphereDataConfigSolver = i_sphereDataConfigSolver;

		use_f_sphere = i_use_f_sphere;
		timestep_size = i_timestep_size;

		alpha = i_alpha/timestep_size;
		beta = i_beta/timestep_size;

		r = i_radius;
		inv_r = 1.0/r;

		if (use_f_sphere)
			f0 = i_f0;
		else
			two_coriolis_omega = 2.0*i_coriolis_omega;

		gh = i_avg_geopotential;

		op.setup(sphereDataConfigSolver, r);
		opComplex.setup(sphereDataConfigSolver, r);


		if (!use_f_sphere)
		{
			sphSolverPhi.setup(sphereDataConfigSolver, 4);
			sphSolverPhi.solver_component_rexi_z1(	(alpha*alpha)*(alpha*alpha), r);
			sphSolverPhi.solver_component_rexi_z2(	2.0*two_coriolis_omega*two_coriolis_omega*alpha*alpha, r);
			sphSolverPhi.solver_component_rexi_z3(	(two_coriolis_omega*two_coriolis_omega)*(two_coriolis_omega*two_coriolis_omega), r);
			sphSolverPhi.solver_component_rexi_z4robert(	-gh*alpha*two_coriolis_omega, r);
			sphSolverPhi.solver_component_rexi_z5robert(	gh/alpha*two_coriolis_omega*two_coriolis_omega*two_coriolis_omega, r);
			sphSolverPhi.solver_component_rexi_z6robert(	gh*2.0*two_coriolis_omega*two_coriolis_omega, r);
			sphSolverPhi.solver_component_rexi_z7(	-gh*alpha*alpha, r);
			sphSolverPhi.solver_component_rexi_z8(	-gh*two_coriolis_omega*two_coriolis_omega, r);

			mug.setup(sphereDataConfigSolver);
			mug.physical_update_lambda_gaussian_grid(
				[&](double lon, double mu, std::complex<double> &o_data)
				{
					o_data = mu;
				}
			);
		}
	}



	/**
	 * Solve a REXI time step for the given initial conditions
	 */
	inline
	void solve_vectorinvariant_progphivortdiv(
			const SphereData &i_phi0,
			const SphereData &i_vort0,
			const SphereData &i_div0,

			SphereData &o_phi,
			SphereData &o_vort,
			SphereData &o_div
	)
	{
		const SphereDataComplex phi0 = Convert_SphereData_To_SphereDataComplex::physical_convert(i_phi0);
		const SphereDataComplex vort0 = Convert_SphereData_To_SphereDataComplex::physical_convert(i_vort0);
		const SphereDataComplex div0 = Convert_SphereData_To_SphereDataComplex::physical_convert(i_div0);

		SphereDataComplex phi(sphereDataConfigSolver);
		SphereDataComplex vort(sphereDataConfigSolver);
		SphereDataComplex div(sphereDataConfigSolver);

		if (use_f_sphere)
		{
			SphereDataComplex rhs = gh*(div0 - f0/alpha*vort0) + (alpha+f0*f0/alpha)*phi0;
			phi = rhs.spectral_solve_helmholtz(alpha*alpha + f0*f0, -gh, r);

			div = -1.0/gh*(phi0 - alpha*phi);
			vort = (1.0/alpha)*(vort0 + f0*(div));
		}
		else
		{
			SphereDataPhysicalComplex u0g(sphereDataConfigSolver);
			SphereDataPhysicalComplex v0g(sphereDataConfigSolver);
			opComplex.robert_vortdiv_to_uv(vort0, div0, u0g, v0g, r);

			SphereDataComplex rhs(sphereDataConfigSolver);

			SphereDataPhysicalComplex phi0g = phi0.getSphereDataPhysicalComplex();

			SphereDataPhysicalComplex Fc_k =
					two_coriolis_omega*inv_r*(
							-(-two_coriolis_omega*two_coriolis_omega*mug*mug + alpha*alpha)*u0g
							+ 2.0*alpha*two_coriolis_omega*mug*v0g
					);

			SphereDataPhysicalComplex foo =
					(gh*(div0.getSphereDataPhysicalComplex() - (1.0/alpha)*two_coriolis_omega*mug*vort0.getSphereDataPhysicalComplex())) +
					(alpha*phi0g + (1.0/alpha)*two_coriolis_omega*two_coriolis_omega*mug*mug*phi0g);

			SphereDataPhysicalComplex rhsg =
					alpha*alpha*foo +
					two_coriolis_omega*two_coriolis_omega*mug*mug*foo
					- (gh/alpha)*Fc_k;

			// convert to spectral space
			rhs = rhsg;

			phi = sphSolverPhi.solve(rhs);

			/*
			 * Solve without inverting a matrix
			 */
			SphereDataPhysicalComplex u0(sphereDataConfigSolver);
			SphereDataPhysicalComplex v0(sphereDataConfigSolver);

			opComplex.robert_vortdiv_to_uv(vort0, div0, u0, v0, r);


			SphereDataPhysicalComplex a(sphereDataConfigSolver);
			SphereDataPhysicalComplex b(sphereDataConfigSolver);

#if 1

			SphereDataPhysicalComplex gradu(sphereDataConfigSolver);
			SphereDataPhysicalComplex gradv(sphereDataConfigSolver);
			opComplex.robert_grad_to_vec(phi, gradu, gradv, r);
			a = u0 + gradu;
			b = v0 + gradv;

#else
			// LEAVE THIS CODE HERE!!!
			// THIS code resulted in the requirement of a mode extension of 2 additional modes
			// for the REXI solver!!!
            a = u0 + inv_r*opComplex.robert_grad_lon(phi).getSphereDataPhysicalComplex();
            b = v0 + inv_r*opComplex.robert_grad_lat(phi).getSphereDataPhysicalComplex();

#endif

			SphereDataPhysicalComplex k = (two_coriolis_omega*two_coriolis_omega*(mug*mug)+alpha*alpha);
			SphereDataPhysicalComplex u = (alpha*a - two_coriolis_omega*mug*(b))/k;
			SphereDataPhysicalComplex v = (two_coriolis_omega*mug*(a) + alpha*b)/k;

			opComplex.robert_uv_to_vortdiv(u, v, vort, div, r);
		}

		o_phi = Convert_SphereDataComplex_To_SphereData::physical_convert_real(phi * beta);
		o_vort = Convert_SphereDataComplex_To_SphereData::physical_convert_real(vort * beta);
		o_div = Convert_SphereDataComplex_To_SphereData::physical_convert_real(div * beta);
	}


};


#endif /* SRC_SWEREXI_SPHROBERT_HPP_ */
