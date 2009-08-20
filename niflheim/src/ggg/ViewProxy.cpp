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

#include "ViewProxy.h"

#include "ObjectName.h"
#include "MessageType.h"
#include "NameService.h"
#include "Ginnungagap.h"
#include "CommunicationSocketManager.h"

#include <iostream>
using std::endl;
using std::cerr;
using std::vector;
using std::pair;

using niflheim::WorldObjectInfo;
using niflheim::AvatarsView;

namespace ggg
{
	typedef vector< WorldObjectInfo >::const_iterator cObjItr_t;

	ViewProxy::ViewProxy(const Uuid& objectId)
	{
		objectType_ = NIFLHEIM_VIEW_OBJ;
		object_ = this;
		this->setObjectId(objectId);
		sendNeed();
	}

	ViewProxy::~ViewProxy()
	{
		sendDontNeed();
	}

	/* Event */
	void ViewProxy::updateView(const AvatarsView& avatarsView)
	{
		/* Hack: If we are a server or proxy, and don't already have a connection, the client have probably disconnected */
		if ((Ginnungagap::Instance()->appType() == SERVER) || (Ginnungagap::Instance()->appType() == PROXY))
		{
			if (!(Ginnungagap::Instance()->haveConnectionTo(Ginnungagap::Instance()->nameService()->netAddr(this->objectId()))))
			{
				return;
			}
		}

		XdrSendBuffer* xdr = makeEventMsg(0, INT*5 + (INT*3)*avatarsView.worldObjects.size());
		
		pair<int, int> vtlc = avatarsView.visibleTopLeftCorner;
		pair<int, int> vlrc = avatarsView.visibleLowerRightCorner;
		int size = avatarsView.worldObjects.size();
		
		*xdr << vtlc.first << vtlc.second << vlrc.first << vlrc.second << size;
		int wot, x, y;
		for (cObjItr_t wo = avatarsView.worldObjects.begin(); wo != avatarsView.worldObjects.end(); ++wo)
		{
			x = wo->position.first;
			y = wo->position.second;
			*xdr << x << y;
			wot = wo->worldObjectType;
			*xdr << wot;
		}

		sendEvent(xdr);
	}
}

