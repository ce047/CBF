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

#include <cbf/spacenavi_reference.h>
#include <cbf/xml_factories.h>

namespace CBF {

#ifdef CBF_HAVE_XSD
	SpaceNaviReference::SpaceNaviReference(const SpaceNaviReferenceType &xml_instance) {
	}
	static XMLDerivedFactory<SpaceNaviReference, CBFSchema::SpaceNaviReference, Reference, CBFSchema::Reference> x;

#endif

} // namespace





