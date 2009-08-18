/************************************************************************
*                                                                       *
*  Copyright (C) 2009 by Geir Erikstad                                  *
*  geirr@baldr.no                                                       *
*                                                                       *
*  This file is part of Niflheim.                                       *
*                                                                       *
*  Niflheim is free software: you can redistribute it and/or modify     *
*  it under the terms of the GNU General Public License as published by *
*  the Free Software Foundation, either version 2 of the License, or    *
*  (at your option) any later version.                                  *
*                                                                       *
*  Niflheim is distributed in the hope that it will be useful,          *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         *
*  GNU General Public License for more details.                         *
*                                                                       *
*  You should have received a copy of the GNU General Public License    *
*  along with Niflheim. If not, see <http://www.gnu.org/licenses/>.     *
*                                                                       *
************************************************************************/

#ifndef GINNUNGAGAP_TESTRMIPROXY_H
#define GINNUNGAGAP_TESTRMIPROXY_H

#include <Proxy.h>

#include "TestObject.h"
#include "Uuid.h"

namespace ginnungagap
{
	class TestObjectProxy : public Proxy, public TestObject
	{
		public:
			TestObjectProxy(const Uuid& objectId);
			~TestObjectProxy();

			/* RMI callabe functions */
			void void_void();
			void void_int(int one);
			int int_void();
			int int_int(int one);
			int int_intInt(int one, int two);
			int int_intIntInt(int one, int two, int three);
			int int_vectorOfInts(const std::vector<int>& vectorOfInts);
	};
}

#endif

