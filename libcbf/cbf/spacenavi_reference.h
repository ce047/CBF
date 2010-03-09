/* -*- mode: c-non-suck; -*- */

#ifndef CBF_SPACENAVI_REFERENCE_HH
#define CBF_SPACENAVI_REFERENCE_HH

#include <stdexcept>

#include <spacenavi.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <boost/numeric/ublas/vector.hpp>

#include <cbf/reference.h>
#include <cbf/types.h>
#include <cbf/plugin_decl_macros.h>

CBF_PLUGIN_PREAMBLE(SpaceNaviReference)

namespace CBF {
	namespace ublas = boost::numeric::ublas;
	
	/** @brief: A reference that takes its information from a SpaceMouse */
	struct SpaceNaviReference : public Reference {
		CBF_PLUGIN_DECL_METHODS(SpaceNaviReference)
	
		SpaceNaviReference()
		{
			m_References.push_back(FloatVector(6));
			m_Device = snavi_open(NULL, O_NONBLOCK);
			if (m_Device == 0)
				throw std::runtime_error("Could not open SpaceMouse device");
		}
	
		virtual void update() {
			int ret = 0;
			bool got_event = false;
			/** get all events from the queue */
			while((ret = snavi_get_event(m_Device, &m_Event)) >= 0) {
				got_event = true;
			}
			if (got_event == true) {
	
			}
		}
	
		protected:
			snavi_event_t m_Event;
			void *m_Device;
	};
	
} // namespace

#endif