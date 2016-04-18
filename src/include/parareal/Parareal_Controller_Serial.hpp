/*
 * PararealController.hpp
 *
 *  Created on: 11 Apr 2016
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 */

#ifndef SRC_INCLUDE_PARAREAL_PARAREAL_CONTROLLER_SERIAL_HPP_
#define SRC_INCLUDE_PARAREAL_PARAREAL_CONTROLLER_SERIAL_HPP_


#include <parareal/Parareal_ConsolePrefix.hpp>
#include <parareal/Parareal_SimulationInstance.hpp>
#include <parareal/Parareal_SimulationVariables.hpp>
#include <iostream>
#include <fstream>
#include <string>


/**
 * This class takes over the control and
 * calls methods offered via PararealSimulation.
 *
 * \param t_SimulationInstance	class which implements the Parareal_SimulationInstance interfaces
 */
template <class t_SimulationInstance>
class Parareal_Controller_Serial
{
	/**
	 * Array with instantiations of PararealSimulations
	 */
	t_SimulationInstance *simulationInstances = nullptr;

	/**
	 * Pointers to interfaces of simulationInstances
	 * This helps to clearly separate between the allocation of the simulation classes and the parareal interfaces.
	 */
	Parareal_SimulationInstance **parareal_simulationInstances = nullptr;

	/**
	 * Pointer to parareal simulation variables.
	 * These variables are used as a singleton
	 */
	PararealSimulationVariables *pVars;

	/**
	 * Class which helps prefixing console output
	 */
	Parareal_ConsolePrefix CONSOLEPREFIX;

public:
	Parareal_Controller_Serial()
	{
	}

	~Parareal_Controller_Serial()
	{
		cleanup();
	}

	void cleanup()
	{
		if (simulationInstances != nullptr)
		{
			delete [] simulationInstances;
			delete [] parareal_simulationInstances;
		}
	}

	void setup(
			PararealSimulationVariables *i_pararealSimVars
	)
	{
		cleanup();


		pVars = i_pararealSimVars;

		if (!pVars->enabled)
			return;

		if (pVars->coarse_slices <= 0)
		{
			std::cerr << "Invalid number of coarse slices" << std::endl;
			exit(1);
		}

		// allocate raw simulation instances
		simulationInstances = new t_SimulationInstance[pVars->coarse_slices];

		parareal_simulationInstances = new Parareal_SimulationInstance*[pVars->coarse_slices];

		CONSOLEPREFIX.start("[MAIN] ");
		std::cout << "Resetting simulation instances" << std::endl;

		// convert to pararealsimulationInstances to get Parareal interfaces
		for (int k = 0; k < pVars->coarse_slices; k++)
		{
			CONSOLEPREFIX.start(k);
			parareal_simulationInstances[k] = &(Parareal_SimulationInstance&)(simulationInstances[k]);
			simulationInstances[k].reset();
		}

		CONSOLEPREFIX.start("[MAIN] ");
		std::cout << "Setup time frames" << std::endl;

		/*
		 * SETUP time frame
		 */
		// size of coarse time step
		double coarse_timestep_size = i_pararealSimVars->max_simulation_time / pVars->coarse_slices;

		CONSOLEPREFIX.start(0);
		parareal_simulationInstances[0]->sim_set_timeframe(0, coarse_timestep_size);

		for (int k = 1; k < pVars->coarse_slices-1; k++)
		{
			CONSOLEPREFIX.start(k);
			parareal_simulationInstances[k]->sim_set_timeframe(coarse_timestep_size*k, coarse_timestep_size*(k+1));
		}

		CONSOLEPREFIX.start(pVars->coarse_slices-1);
		parareal_simulationInstances[pVars->coarse_slices-1]->sim_set_timeframe(i_pararealSimVars->max_simulation_time-coarse_timestep_size, i_pararealSimVars->max_simulation_time);


		/*
		 * Setup first simulation instance
		 */
		CONSOLEPREFIX.start(0);
		parareal_simulationInstances[0]->sim_setup_initial_data();

		CONSOLEPREFIX.end();
	}

	void run()
	{

		CONSOLEPREFIX.start("[MAIN] ");
		std::cout << "Initial propagation" << std::endl;

		/**
		 * Initial propagation
		 */
		CONSOLEPREFIX.start(0);
		parareal_simulationInstances[0]->run_timestep_coarse();
		for (int i = 1; i < pVars->coarse_slices; i++)
		{
			CONSOLEPREFIX.start(i-1);
			PararealData &tmp = parareal_simulationInstances[i-1]->get_data_timestep_coarse();

				// use coarse time step output data as initial data of next coarse time step
			CONSOLEPREFIX.start(i);
			parareal_simulationInstances[i]->sim_set_data(tmp);

			// run coarse time step
			parareal_simulationInstances[i]->run_timestep_coarse();
		}



		/**
		 * We run as much Parareal iterations as there are coarse slices
		 */
		for (int k = 0; k < pVars->coarse_slices; k++)
		{
			CONSOLEPREFIX.start("[MAIN] ");
			std::cout << "Iteration Nr. " << k << std::endl;
			/*
			 * All the following loop should start with 0.
			 * For debugging reasons, we leave it here at 0
			 */
			int start = 0;
//			int start = k;

			/**
			 * Fine time stepping
			 */
			for (int i = start; i < pVars->coarse_slices; i++)
			{
				CONSOLEPREFIX.start(i);
				parareal_simulationInstances[i]->run_timestep_fine();
			}


			/**
			 * Compute difference between coarse and fine solution
			 */
			for (int i = start; i < pVars->coarse_slices; i++)
			{
				CONSOLEPREFIX.start(i);
				parareal_simulationInstances[i]->compute_difference();
			}


			/**
			 * 1) Coarse time stepping
			 * 2) Compute output + convergence check
			 * 3) Forward to next frame
			 */
			for (int i = start; i < pVars->coarse_slices; i++)
			{
				CONSOLEPREFIX.start(i);
				parareal_simulationInstances[i]->run_timestep_coarse();

				// compute convergence
				double convergence = parareal_simulationInstances[i]->compute_output_data(true);
				std::cout << "                        iteration " << k << ", time slice " << i << ", convergence: " << convergence << std::endl;

				// forward to next time slice if it exists
				if (i < pVars->coarse_slices-1)
				{
					CONSOLEPREFIX.start(i);
					PararealData &tmp = parareal_simulationInstances[i]->get_output_data();

					CONSOLEPREFIX.start(i+1);
					parareal_simulationInstances[i+1]->sim_set_data(tmp);
				}
			}

		}


		CONSOLEPREFIX.end();
	}
};





#endif /* SRC_INCLUDE_PARAREAL_PARAREAL_CONTROLLER_SERIAL_HPP_ */