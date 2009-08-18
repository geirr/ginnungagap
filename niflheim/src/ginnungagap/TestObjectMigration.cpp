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

#include "TestObject.h"
#include "Uuid.h"
#include "MessageType.h"
#include "ObjectName.h"
#include "XdrSendBuffer.h"

#include <iostream>

using std::vector;
using namespace ginnungagap;

XdrSendBuffer* TestObject::deflate()
{
	int msg = MIGOBJ;
	int objectType = TESTOBJECT_OBJ;
	int size = INT*(2 + 1 + data_.size()) + OBJID;
	XdrSendBuffer* xdr = new XdrSendBuffer(size);
	int tmp = data_.size();
	*xdr << msg << objectType << this->objectId() << tmp;
	for (vector<int>::const_iterator dataItr = data_.begin(); dataItr != data_.end(); ++dataItr)
	{
		tmp = *dataItr;
		*xdr << tmp;
	}
	return xdr;
}

TestObject::TestObject(XdrReceiveBuffer* xdr)
{
	Uuid objId;
	*xdr >> objId;
	this->setObjectId(objId);
	int tmp, tmp2;
	*xdr >> tmp;
	data_.reserve(tmp);
	for (int i = 0; i != tmp; ++i)
	{
		*xdr >> tmp2;
		data_.push_back(tmp2);
	}
}

