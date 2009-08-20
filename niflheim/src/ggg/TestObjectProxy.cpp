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

#include "TestObjectProxy.h"
#include "XdrSendBuffer.h"
#include "XdrReceiveBuffer.h"
#include "ObjectName.h"
#include "MessageType.h"
#include "NameService.h"
#include "Ginnungagap.h"

#include <iostream>

using std::endl;
using std::cerr;

namespace ggg
{
	TestObjectProxy::TestObjectProxy(const Uuid& objectId)
	{
		objectType_ = TESTOBJECT_OBJ;
		object_ = this;
		this->setObjectId(objectId);
		sendNeed();
	}

	TestObjectProxy::~TestObjectProxy()
	{
		sendDontNeed();
	}

	void TestObjectProxy::void_void()
	{
		XdrSendBuffer* xdr = makeRmiMsg(0);
		sendAndGetRetVal(xdr);
	}

	void TestObjectProxy::void_int(int one)
	{
		XdrSendBuffer* xdr = makeRmiMsg(1, INT);
		*xdr << one;
		sendAndGetRetVal(xdr);
	}

	int TestObjectProxy::int_void()
	{
		XdrSendBuffer* xdr = makeRmiMsg(2);
		XdrReceiveBuffer* xdrReciveBuffer = sendAndGetRetVal(xdr);
		int retVal;
		*xdrReciveBuffer >> retVal;
		delete xdrReciveBuffer;
		return retVal;
	}

	int TestObjectProxy::int_int(int one)
	{
		XdrSendBuffer* xdr = makeRmiMsg(3, INT);
		*xdr << one;
		XdrReceiveBuffer* xdrReciveBuffer = sendAndGetRetVal(xdr);
		int retVal;
		*xdrReciveBuffer >> retVal;
		delete xdrReciveBuffer;
		return retVal;
	}

	int TestObjectProxy::int_intInt(int one, int two)
	{
		XdrSendBuffer* xdr = makeRmiMsg(4, INT*2);
		*xdr << one << two;
		XdrReceiveBuffer* xdrReciveBuffer = sendAndGetRetVal(xdr);
		int retVal;
		*xdrReciveBuffer >> retVal;
		delete xdrReciveBuffer;
		return retVal;
	}

	int TestObjectProxy::int_intIntInt(int one, int two, int three)
	{
		XdrSendBuffer* xdr = makeRmiMsg(5, INT*3);
		*xdr << one << two << three;
		XdrReceiveBuffer* xdrReciveBuffer = sendAndGetRetVal(xdr);
		int retVal;
		*xdrReciveBuffer >> retVal;
		delete xdrReciveBuffer;
		return retVal;
	}

	int TestObjectProxy::int_vectorOfInts(const std::vector<int>& vectorOfInts)
	{
		int sizeOfVector = vectorOfInts.size();
		XdrSendBuffer* xdr = makeRmiMsg(6, INT + INT*sizeOfVector);
		*xdr << sizeOfVector;
		int tmp;
		for (std::vector<int>::const_iterator itr = vectorOfInts.begin(); itr != vectorOfInts.end(); ++itr)
		{
			tmp = *itr;
			*xdr << tmp;
		}
		XdrReceiveBuffer* xdrReciveBuffer = sendAndGetRetVal(xdr);
		int retVal;
		*xdrReciveBuffer >> retVal;
		delete xdrReciveBuffer;
		return retVal;
	}
}

