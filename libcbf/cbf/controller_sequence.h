/*
    This file is part of CBF.

    CBF is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    CBF is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CBF.  If not, see <http://www.gnu.org/licenses/>.


    Copyright 2009, 2010 Florian Paul Schmidt
*/

/* -*- mode: c-non-suck; -*- */

#ifndef CBF_CONTROLLER_SEQUENCE_HH
#define CBF_CONTROLLER_SEQUENCE_HH

#include <cbf/config.h>
#include <cbf/controller.h>
#include <cbf/namespace.h>

#include <vector>
#include <boost/shared_ptr.hpp>

namespace CBFSchema { class ControllerSequence; }

namespace CBF {
	
	/**
		@brief A controller that runs a sequence of controllers.
	
		This controller can be used to execute sequences of controllers. Each controller in the sequence is run until finished. When the last controller finished, the ControllerSequence finished, too.
	*/
	struct ControllerSequence : public Controller {
		ControllerSequence (const CBFSchema::ControllerSequence &xml_instance, ObjectNamespacePtr object_namespace);

		std::vector<ControllerPtr> m_Controllers;
		std::vector<ControllerPtr>::iterator m_Iterator;
	
		ControllerSequence(
			std::vector<ControllerPtr> controllers = std::vector<ControllerPtr>()
		) {
			init(controllers);
		}

		virtual void init(
			std::vector<ControllerPtr> controllers
		) {
			m_Controllers = controllers;
			reset();
		}

		virtual bool step();
	
		virtual bool finished();
	
		/**
			Resets this controller, so it can start from the beginning again. Note that this
			is also done when finished() is true.. So calling step() after this controller 
			finished just restarts it..
		*/
		virtual void reset() {
			m_Iterator = m_Controllers.begin();
		}
	};
	
	typedef boost::shared_ptr<ControllerSequence> ControllerSequencePtr;
	

} // namespace

#endif
