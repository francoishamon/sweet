/*
 * SphereData_To_ScalarDataArray.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 */

#ifndef SRC_INCLUDE_SWEET_SPHERE_CONVERT_SPHEREDATA_TO_PLANEDATA_HPP_
#define SRC_INCLUDE_SWEET_SPHERE_CONVERT_SPHEREDATA_TO_PLANEDATA_HPP_

#include <sweet/sphere/SphereData.hpp>
#include <sweet/plane/PlaneData.hpp>

class Convert_SphereData_To_PlaneData
{
public:
	static
	PlaneData physical_convert(
			const SphereData &i_sphereData,
			PlaneDataConfig *i_planeDataConfig
	)
	{
		assert(i_sphereData.sphereDataConfig->physical_num_lon == (int)i_planeDataConfig->physical_res[0]);
		assert(i_sphereData.sphereDataConfig->physical_num_lat == (int)i_planeDataConfig->physical_res[1]);
		assert((int)i_planeDataConfig->physical_array_data_number_of_elements == i_sphereData.sphereDataConfig->physical_array_data_number_of_elements);

		i_sphereData.request_data_physical();

		PlaneData out(i_planeDataConfig);

#if SWEET_THREADING
#pragma omp parallel for
#endif


#if SPHERE_DATA_GRID_LAYOUT	== SPHERE_DATA_LAT_CONTINUOUS

		for (int i = 0; i < i_sphereData.sphereDataConfig->physical_num_lon; i++)
			for (int j = 0; j < i_sphereData.sphereDataConfig->physical_num_lat; j++)
				out.physical_space_data[(i_sphereData.sphereDataConfig->physical_num_lat-1-j)*i_sphereData.sphereDataConfig->physical_num_lon + i] = i_sphereData.physical_space_data[i*i_sphereData.sphereDataConfig->physical_num_lat + j];
#else
		for (int j = 0; j < i_sphereData.sphereDataConfig->physical_num_lat; j++)
			for (int i = 0; i < i_sphereData.sphereDataConfig->physical_num_lon; i++)
				out.physical_space_data[(i_sphereData.sphereDataConfig->physical_num_lat-1-j)*i_sphereData.sphereDataConfig->physical_num_lon + i] = i_sphereData.physical_space_data[j*i_sphereData.sphereDataConfig->physical_num_lon + i];
#endif

#if SWEET_USE_PLANE_SPECTRAL_SPACE
		out.physical_space_data_valid = true;
		out.spectral_space_data_valid = false;
#endif

		return out;
	}
};



#endif /* SRC_INCLUDE_SWEET_SPHERE_CONVERT_SPHEREDATA_TO_SCALARDATAARRAY_HPP_ */
