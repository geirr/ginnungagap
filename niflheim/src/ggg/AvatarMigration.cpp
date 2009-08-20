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

#include "Avatar.h"
#include "World.h"
#include "Uuid.h"
#include "MessageType.h"
#include "ObjectName.h"
#include "XdrSendBuffer.h"

#include <iostream>
using std::cerr; using std::endl;

using namespace ggg;

namespace niflheim
{
	XdrSendBuffer* Avatar::deflate()
	{
		int msg = MIGOBJ;
		int objectType = NIFLHEIM_AVATAR_OBJ;
		int active = 0;
		if (active_)
			active = 1;

		Uuid worldId = world_.objectId();
		NetAddr worldAddr = Ginnungagap::Instance()->nameService()->netAddr(worldId);

		Uuid viewId = view_.objectId();
		NetAddr viewAddr = Ginnungagap::Instance()->nameService()->netAddr(viewId);

		XdrSendBuffer* xdr = new XdrSendBuffer(INT*3 + OBJID*3 + NETADDR*2);
		*xdr << msg << objectType << this->objectId() << worldId << worldAddr << viewId << viewAddr << active;
		return xdr;
	}

	Avatar::Avatar(XdrReceiveBuffer* xdr)
	{
		Uuid objId;
		Uuid worldId;
		NetAddr worldAddr;
		Uuid viewId;
		NetAddr viewAddr;
		int active;
		*xdr >> objId >> worldId >> worldAddr >> viewId >> viewAddr >> active;
		this->setObjectId(objId);

		if (!(Ginnungagap::Instance()->nameService()->exists(worldId)))
		{
			Ginnungagap::Instance()->nameService()->updateNetAddr(worldId, worldAddr);
		}

		if (!(Ginnungagap::Instance()->nameService()->exists(viewId)))
		{
			Ginnungagap::Instance()->nameService()->updateNetAddr(viewId, viewAddr);
		}
			
		world_ = dist_ptr<World>(worldId);
		view_ = dist_ptr<View>(viewId);

		if (active == 0)
			active_ = false;
		else
			active_ = true;
	}
}

