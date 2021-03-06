/*
 * rexi.cpp
 *
 *  Created on: 2 Aug 2015
 *      Author: Martin Schreiber <schreiberx@gmail.com>
 */

#include <iostream>
#include <rexi/GaussianApproximation.hpp>
#include <rexi/Phi1Approximation.hpp>
#include <rexi/REXI.hpp>
#include <sweet/SimulationVariables.hpp>
#include "../include/sweet/plane/PlaneDataComplex.hpp"

int main(
		int i_argc,
		char *const i_argv[]
)
{
	SimulationVariables simVars;
	if (!simVars.setupFromMainParameters(i_argc, i_argv))
	{
		return -1;
	}

	double max_error_threshold = 1e-9;
	double max_error_threshold_machine = 1e-12;


#if 0
	if (1)
	{
		std::cout << "******************************************************" << std::endl;
		std::cout << "REXI real: Test for partition of unity" << std::endl;
		std::cout << "******************************************************" << std::endl;

		for (double h = 0.2; h >= 0.05; h *= 0.5)
		{
			for (int M = 256; M < 1024; M *= 2)
			{
				REXI rexi(h, M, 0, false);

				// REXI approximates the interval [-M*h;M*h] but gets inaccurate close to the interval boundaries
				double start = -M*h*0.95;
				double end = -start;
				double step_size = 0.0001;

				double max_error = 0;

				for (double x = start; x < end; x += step_size)
				{
	//				std::cout << std::endl;
	//				std::cout << "x: " << x << std::endl;
	//				std::cout << "exp(ix): " << rexi.eval_e_ix(x).real() << std::endl;
	//				std::cout << "approx. exp(ix): " << rexi.approx_e_ix(x).real() << std::endl;

					double correct = rexi.eval_e_ix(x).real();
					double approx = rexi.approx_e_ix_returnReal(x);

					if (std::abs(approx) > 1.0)
					{
						std::cerr << "approx value " << approx << " not bounded by unity (just a warning and not a problem)" << std::endl;
//						exit(-1);
					}
					double error_real = std::abs(correct - approx);

	//				std::cout << x << ": " << error_real << std::endl;
					max_error = std::max(max_error, error_real);
				}

				std::cout << "max_error: " << max_error << " for h " << h << " and M " << M << std::endl;

				if (std::abs(max_error) > max_error_threshold)
				{
					std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
					exit(-1);
				}
			}
		}
	}
#endif

#if 0
	if (1)
	{
		std::cout << "******************************************************" << std::endl;
		std::cout << "EVALUATING GAUSSIAN APPROXIMATION (real)" << std::endl;
		std::cout << "******************************************************" << std::endl;
		GaussianApproximation ga;

		for (double h = 0.2; h > 0.001; h *= 0.5)
		{
			double start = -100.0;
			double end = 100.0;
			double step_size = 0.001;

			double max_error = 0;

			for (double x = start; x < end; x += step_size)
			{
				double error = std::abs(ga.evalGaussian(x, h) - ga.approxGaussian(x, h));
				max_error = std::max(max_error, error);
			}

			std::cout << "max_error: " << max_error << " for h " << h << std::endl;

			if (std::abs(max_error) > max_error_threshold)
			{
				std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
				exit(-1);
			}
		}
	}
#endif

#if 1
	if (1)
	{
		std::cout << "******************************************************" << std::endl;
		std::cout << "EVALUATING Phi1 APPROXIMATION with approx Gaussian (approx_e_ix)" << std::endl;
		std::cout << "******************************************************" << std::endl;

		for (double h = 0.2; h > 0.005; h *= 0.5)
		{
			int M = 32/h;
			std::cout << "Setting up scalars..." << std::flush;
			Phi1Approximation ea(h, M);
			std::cout << "OK" << std::endl;

			double start = -M*h*0.95;
			double end = -start;
			double step_size = 0.01;

			double max_error = 0;

			std::cout << "Testing interval [" << start << ", " << end << "] with step size " << step_size << std::endl;
			for (double x = start; x < end; x += step_size)
			{
				double error = std::abs(ea.eval(x) - ea.approx(x));
				max_error = std::max(max_error, error);
			}

			std::cout << "max_error: " << max_error << " for h " << h << " and M " << M << std::endl;

			if (std::abs(max_error) > max_error_threshold)
			{
				std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
				exit(-1);
			}
		}
	}
#endif

#if 0
	if (1)
	{
		std::cout << "******************************************************" << std::endl;
		std::cout << "REXI complex" << std::endl;
		std::cout << "******************************************************" << std::endl;

		for (double h = 0.2; h > 0.005; h *= 0.5)
		{
			int M = 256/h;

			REXI rexi(h, M, 0, false);

			double start = -M*h*0.95;
			double end = -start;
			double step_size = 0.01;

			double max_error = 0;

			for (double x = start; x < end; x += step_size)
			{
//				std::cout << "x: " << x << std::endl;
//				std::cout << "exp(ix): " << rexi.eval_e_ix(x) << std::endl;
//				std::cout << "approx. exp(ix): " << rexi.approx_e_ix(x) << std::endl;
				double error = std::abs(rexi.eval_e_ix(x) - rexi.approx_e_ix(x));
				max_error = std::max(max_error, error);
			}

			std::cout << "max_error: " << max_error << " for h " << h << " and M " << M << std::endl;

			if (std::abs(max_error) > max_error_threshold)
			{
				std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
				exit(-1);
			}
		}
	}
#endif

#if 0
	if (1)
	{
		std::cout << "******************************************************" << std::endl;
		std::cout << "REXI real" << std::endl;
		std::cout << "******************************************************" << std::endl;

//		for (int k = 0; k < 1; k++)
		{
			// using only half of the poles does not work for e(ix)
			bool half = false;
			for (double h = 0.2; h > 0.005; h *= 0.5)
			{
				int M = 256/h;

				REXI rexi(h, M, 0, half);

				double start = -M*h*0.95;
				double end = -start;
				double step_size = 0.01;

				double max_error = 0;

				for (double x = start; x < end; x += step_size)
				{
	//				std::cout << std::endl;
	//				std::cout << "x: " << x << std::endl;
	//				std::cout << "exp(ix): " << rexi.eval_e_ix(x).real() << std::endl;
	//				std::cout << "approx. exp(ix): " << rexi.approx_e_ix(x).real() << std::endl;
					double error_real = std::abs(rexi.eval_e_ix(x).real() - rexi.approx_e_ix_returnReal(x));
					max_error = std::max(max_error, error_real);
				}

				std::cout << "max_error: " << max_error << " for h " << h << " and M " << M << std::endl;

				if (std::abs(max_error) > max_error_threshold)
				{
					std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
					exit(-1);
				}
			}
			}
	}
#endif


#if 0
	if (1)
	{
		std::cout << "******************************************************" << std::endl;
		std::cout << "REXI imag" << std::endl;
		std::cout << "******************************************************" << std::endl;


//		for (int k = 0; k < 2; k++)
		{
			// halving does only work for linear operator
			//bool half = k;
			bool half = false;
			for (double h = 1; h > 0.005; h *= 0.5)
			{
				int M = 256.0/h;

				REXI rexi(h, M, 0, half);

				double start = -M*h*0.95;
				double end = -start;
				double step_size = 0.01;

				double max_error = 0;

				for (double x = start; x < end; x += step_size)
				{
					double error_imag = std::abs(rexi.eval_e_ix(x).imag() - rexi.approx_e_ix_returnImag(x));
					max_error = std::max(max_error, error_imag);
				}

				std::cout << "max_error: " << max_error << " for h " << h << " and M " << M << std::endl;

				if (std::abs(max_error) > max_error_threshold)
				{
					std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
					exit(-1);
				}
			}
		}
	}
#endif

#if 0
	if (0)
	{
		std::size_t res[2] = {128, 128};

		double eta_bar = simVars.setup.h0;
		double g = simVars.sim.g;

		PlaneDataComplex eta0(simVars.disc.res);
		PlaneDataComplex u0(simVars.disc.res);
		PlaneDataComplex v0(simVars.disc.res);

		PlaneDataComplex op_diff_c_x(simVars.disc.res), op_diff_c_y(simVars.disc.res);
		PlaneDataComplex op_diff2_c_x(simVars.disc.res), op_diff2_c_y(simVars.disc.res);

		op_diff_c_x.op_setup_diff_x(simVars.sim.domain_size);
		op_diff_c_y.op_setup_diff_y(simVars.sim.domain_size);

		op_diff2_c_x.op_setup_diff2_x(simVars.sim.domain_size);
		op_diff2_c_y.op_setup_diff2_y(simVars.sim.domain_size);


		for (double h = 2; h > 0.01; h *= 0.5)
		{
			int M = 256/h;

			double tau = h*20.0;
			double f = simVars.sim.f0;

			std::cout << "REXI setup: M=" << M << ", h=" << h << ", tau=" << tau << ", f=" << f << std::endl;

			REXI rexi;
			rexi.setup(0, h, M);

			std::size_t N = rexi.alpha.size();
			for (std::size_t n = 0; n < N; n++)
			{
				// load alpha (a) and scale by inverse of tau
				std::complex<double> alpha = rexi.alpha[n];

				/*
				 * VALIDATION CHECK FOR SOLVER (partly based on Pedros suggestion)
				 *
				 * we test for a valid spectral solver for the side constraints
				 * 		g=1, h0=1, u=0, v=0, f=0
				 *
				 * As a result, we assume
				 * 		\eta := exp(i*(k1*x+k2*y))
				 *
				 * To test the D^2 operator, we can compute
				 * 		D^2 \eta = -(k1^2 + k2^2) \eta
				 *
				 * Then, we can evaluate the following expression for \eta:
				 * 		(\alpha^2 - D^2) \eta
				 * 			= (\alpha^2 + (k1^2 + k2^2)) \eta
				 * 			= (\alpha^2 + |k|^2) \eta
				 *
				 * Given the side constraints, the solver for \eta is given by
				 * 		(\alpha^2 - D^2) \eta = \alpha^2 / alpha \eta_0
				 *
				 * Given the above specialization on \eta, we can solve for \eta_0:
				 *		\eta_0 = (\alpha^2 + |k|^2) / \alpha^2 * alpha * \eta
				 *		       = (1.0 + |k|^2 / \alpha^2) * alpha * \eta
				 *		       = (alpha + |k|^2 / \alpha) * \eta
				 */
				{
					std::complex<double> alpha = rexi.alpha[n];

					PlaneDataComplex test_eta0(simVars.disc.res);
					PlaneDataComplex test_eta(simVars.disc.res);
					PlaneDataComplex test_diff1(simVars.disc.res);
					PlaneDataComplex test_diff2(simVars.disc.res);

					PlaneDataComplex test_step1(simVars.disc.res);
					PlaneDataComplex test_step2(simVars.disc.res);
					PlaneDataComplex test_step3(simVars.disc.res);

					double k1 = 2.0*M_PIl*1.0;
					double k2 = 2.0*M_PIl*2.0;

					for (std::size_t j = 0; j < simVars.disc.res[1]; j++)
					{
						for (std::size_t i = 0; i < simVars.disc.res[0]; i++)
						{
							double x = ((double)i+0.5)/(double)simVars.disc.res[0];
							double y = ((double)j+0.5)/(double)simVars.disc.res[1];

							double exponent = x*k1 + y*k2;
							std::complex<double> eta_val(cos(exponent), sin(exponent));

							// test function
							// \eta := exp(i*(k1*x+k2*y))
							test_eta.p_physical_set(j, i, eta_val);

							double sim_size_scale_x = 1.0/(double)simVars.sim.domain_size[0];
							double sim_size_scale_y = 1.0/(double)simVars.sim.domain_size[1];
							std::complex<double> eta_fac = (alpha*alpha + (k1*k1*sim_size_scale_x*sim_size_scale_x + k2*k2*sim_size_scale_y*sim_size_scale_y))/(alpha*alpha) * alpha;
							test_eta0.p_physical_set(j, i, eta_val * eta_fac);

							// \eta_x + \eta_y = (i k1 + i k2) \eta
							std::complex<double> test_d1_fac(0, (k1*sim_size_scale_x + k2*sim_size_scale_y));
							test_diff1.p_physical_set(j, i, eta_val * test_d1_fac);

							// D^2 \eta = -(k1^2 + k2^2) \eta
							std::complex<double> test_d2_fac(-(k1*k1*sim_size_scale_x*sim_size_scale_x + k2*k2*sim_size_scale_y*sim_size_scale_y), 0);
							test_diff2.p_physical_set(j, i, eta_val * test_d2_fac);

							std::complex<double> test_step1_fac(alpha*alpha + (k1*k1*sim_size_scale_x*sim_size_scale_x + k2*k2*sim_size_scale_y*sim_size_scale_y));
//							std::complex<double> test_step1_fac((k1*k1 + k2*k2));
							test_step1.p_physical_set(j, i, eta_val*test_step1_fac);

							std::complex<double> test_step2_fac(alpha*alpha);
							test_step2.p_physical_set(j, i, eta_val*test_step2_fac);

							std::complex<double> test_step3_fac(alpha*alpha + (k1*k1*sim_size_scale_x*sim_size_scale_x + k2*k2*sim_size_scale_y*sim_size_scale_y));
							test_step3.p_physical_set(j, i, eta_val*test_step3_fac);
						}
					}

					{
						// Test LHS with alpha=0, step 1
//						PlaneDataComplex lhs = ((-1.0*(op_diff2_c_x + op_diff2_c_y)).addScalar_Spec(alpha*alpha))(test_eta.toSpec());
						PlaneDataComplex lhs = ((-1.0*(op_diff2_c_x + op_diff2_c_y)))(test_eta.toSpec());
						lhs = lhs + test_eta.toSpec()*(alpha*alpha);
						double error_0 = (lhs.toCart()-test_step1).reduce_norm2_quad()/(simVars.disc.res[0]*simVars.disc.res[1]);
						std::cout << "ERROR 0 step1: " << error_0 << std::endl;

						if (std::abs(error_0) > max_error_threshold_machine)
						{
							std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
							exit(-1);
						}
					}


					{
						// Test LHS with alpha and by multiplying in, step 2
						PlaneDataComplex lhs = test_eta.toSpec()*(alpha*alpha);
						double error_0 = (lhs.toCart()-test_step2).reduce_norm2_quad()/(simVars.disc.res[0]*simVars.disc.res[1]);
						std::cout << "ERROR 0 step2: " << error_0 << std::endl;

						if (std::abs(error_0) > max_error_threshold_machine)
						{
							std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
							exit(-1);
						}
					}

					{
						// Test LHS with alpha and by joining operators, step 3
						// NOTE!!!! Here we use addScalar_Cart, since \eta has to be scaled by alpha.
						// If we would use addScalar_Spec, only the zero mode would be scaled which would be PLAIN WRONG!!!
						PlaneDataComplex lhs = ((-1.0*(op_diff2_c_x + op_diff2_c_y)).addScalar_Cart(alpha*alpha))(test_eta.toSpec());
	//					PlaneDataComplex lhs = ((-1.0*(op_diff2_c_x + op_diff2_c_y)))(test_eta.toSpec())
	//											 + test_eta.toSpec()*(alpha*alpha);

						double error_0 = (lhs.toCart()-test_step1).reduce_norm2_quad()/(simVars.disc.res[0]*simVars.disc.res[1]);
						std::cout << "ERROR 0 step3: " << error_0 << std::endl;

						if (std::abs(error_0) > max_error_threshold_machine)
						{
							std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
							exit(-1);
						}
					}


					{
						// NOTE!!!!
						// Here we use addScalar_Cart, since \eta has to be scaled by alpha, see comment above for 'step 3'
						// If we would use addScalar_Spec, only the zero mode would be scaled
						PlaneDataComplex lhs = (-1.0*(op_diff2_c_x + op_diff2_c_y)).addScalar_Cart(alpha*alpha);
						PlaneDataComplex rhs =	test_eta0.toSpec()*alpha;
						PlaneDataComplex eta = rhs.spec_div_element_wise(lhs).toCart();

						// Test SOLVER
						double error_1 = (eta - test_eta).reduce_norm2_quad()/(simVars.disc.res[0]*simVars.disc.res[1]);
						std::cout << "ERROR 1: " << error_1 << std::endl;

						if (std::abs(error_1) > max_error_threshold_machine)
						{
							std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
							exit(-1);
						}
					}

					{
						// Test DIFF
						double error_2 = ((op_diff_c_x + op_diff_c_y)(test_eta.toSpec()).toCart()-test_diff1).reduce_norm2_quad()/(simVars.disc.res[0]*simVars.disc.res[1]);
						std::cout << "ERROR 2: " << error_2 << std::endl;

						if (std::abs(error_2) > max_error_threshold_machine)
						{
							std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
							exit(-1);
						}
					}

					{
						// Test DIFF2
						double error_3 = ((op_diff2_c_x + op_diff2_c_y)(test_eta.toSpec()).toCart()-test_diff2).reduce_norm2_quad()/(simVars.disc.res[0]*simVars.disc.res[1]);
						std::cout << "ERROR 3: " << error_3 << std::endl;

						if (std::abs(error_3) > max_error_threshold_machine)
						{
							std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
							exit(-1);
						}
					}

					{
						// Test inverse of DIFF2
						double error_3b = (test_diff2.toSpec().spec_div_element_wise(op_diff2_c_x + op_diff2_c_y).toCart() - test_eta).reduce_norm2_quad()/(simVars.disc.res[0]*simVars.disc.res[1]);
						std::cout << "ERROR 3b: " << error_3b << std::endl;

						if (std::abs(error_3b) > max_error_threshold_machine)
						{
							std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
							exit(-1);
						}
					}

					{
						std::complex<double> test_value(0.7, -2.0);
						PlaneDataComplex test_a(simVars.disc.res);

						// test add scalar spec
						test_a.set_all(test_value);
						double error_4 = (test_a.toSpec().addScalar_Spec(-test_value)).toCart().reduce_norm2_quad()/(simVars.disc.res[0]*simVars.disc.res[1]);
						std::cout << "ERROR 4: " << error_4 << std::endl;

						if (std::abs(error_4) > max_error_threshold_machine)
						{
							std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
							exit(-1);
						}
					}

					{
						std::complex<double> test_value(0.7, -2.0);
						PlaneDataComplex test_a(simVars.disc.res);

						// test add scalar cart
						test_a.set_all(test_value);
						double error_5 = (test_a.addScalar_Cart(-test_value)).reduce_norm2_quad()/(simVars.disc.res[0]*simVars.disc.res[1]);
						std::cout << "ERROR 5: " << error_5 << std::endl;

						if (std::abs(error_5) > max_error_threshold_machine)
						{
							std::cerr << "MAX ERROR THRESHOLD EXCEEDED!" << std::endl;
							exit(-1);
						}
					}
				}
			}
		}
	}
#endif


	return 0;
}
