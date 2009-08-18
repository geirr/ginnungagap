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

#ifndef TESTRMI_H
#define TESTRMI_H

#include "Object.h"
#include "XdrReceiveBuffer.h"

#include <vector>

class TestObject : public ginnungagap::Object
{
	public:
		TestObject() :  tmp_(0), data_(std::vector<int>(0)) {}
		TestObject(int data) : tmp_(0), data_(std::vector<int>(data, data)) {}

		/* RMI reachable functions */
		virtual void void_void();
		virtual void void_int(int one);
		virtual int int_void();
		virtual int int_int(int one);
		virtual int int_intInt(int one, int two);
		virtual int int_intIntInt(int one, int two, int three);
		virtual int int_vectorOfInts(const std::vector<int>& vectorOfInts);

		/* mingration */
		ginnungagap::XdrSendBuffer* deflate();
		TestObject(ginnungagap::XdrReceiveBuffer* xdr);

	private:
		int tmp_;
		std::vector<int> data_;
};

#endif

