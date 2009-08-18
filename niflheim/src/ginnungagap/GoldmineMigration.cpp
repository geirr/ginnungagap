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

#include "Goldmine.h"
#include "World.h"
#include "Uuid.h"
#include "MessageType.h"
#include "ObjectName.h"
#include "XdrSendBuffer.h"

#include <iostream>
using std::cerr; using std::endl;

using namespace ginnungagap;

namespace niflheim
{
	XdrSendBuffer* Goldmine::deflate()
	{
		int msg = MIGOBJ;
		int objectType = NIFLHEIM_GOLDMINE_OBJ;

		Uuid worldId = world_.objectId();
		NetAddr worldAddr = Ginnungagap::Instance()->nameService()->netAddr(worldId);

		XdrSendBuffer* xdr = new XdrSendBuffer(INT*2 + OBJID*2 + NETADDR);
		*xdr << msg << objectType << this->objectId() << worldId << worldAddr;
		return xdr;
	}

	Goldmine::Goldmine(XdrReceiveBuffer* xdr)
	{
		Uuid objId;
		Uuid worldId;
		NetAddr worldAddr;
		*xdr >> objId >> worldId >> worldAddr;
		this->setObjectId(objId);

		if (!(Ginnungagap::Instance()->nameService()->exists(worldId)))
		{
			Ginnungagap::Instance()->nameService()->updateNetAddr(worldId, worldAddr);
		}

		world_ = dist_ptr<World>(worldId);
	}
}

