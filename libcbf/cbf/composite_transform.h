/* -*- mode: c-non-suck; -*- */

#ifndef CBF_COMPOSITE_TRANSFORM_HH
#define CBF_COMPOSITE_TRANSFORM_HH

#include <cbf/plugin_decl_macros.h>
#include <cbf/sensor_transform.h>

#include <vector>
#include <boost/numeric/ublas/matrix.hpp>

CBF_PLUGIN_PREAMBLE(CompositeSensorTransform)

namespace CBF {
	
	namespace ublas = boost::numeric::ublas;
	
	/**
		@brief This sensor transform allows to compose different sensor transforms into a single
		one. 
	
		If T1 and T2 are sensor transforms transforming to task space variables t1 and t2 
		respectively, then this sensor transform transforms to (t1' t2')', i.e. the column 
		vector composed of the components of t1 and t2.
	
		This allows e.g. composition of a position/orientation controller from a position
		and an orientation controller.

		Note that you are not limited to two transforms. But you can use an arbitrary
		positive non zero number of transforms...
	*/
	struct CompositeSensorTransform : public SensorTransform {
		CBF_PLUGIN_DECL_METHODS(CompositeSensorTransform)

		protected:
			std::vector<SensorTransformPtr> m_SensorTransforms;

	
		public:
			CompositeSensorTransform(std::vector<SensorTransformPtr> transforms = std::vector<SensorTransformPtr>()) {
				set_transforms(transforms);
			}

			virtual void set_transforms(std::vector<SensorTransformPtr> transforms);
		
			virtual unsigned resource_dim();

			virtual unsigned int task_dim();

			virtual void update();
	
			virtual void set_resource(ResourcePtr resource);
	};
	
	typedef boost::shared_ptr<CompositeSensorTransform> CompositeSensorTransformPtr;
	
} // Namespace

#endif