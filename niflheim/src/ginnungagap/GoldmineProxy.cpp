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

#include "GoldmineProxy.h"

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

namespace ginnungagap
{
	GoldmineProxy::GoldmineProxy(const Uuid& objectId)
	{
		objectType_ = NIFLHEIM_GOLDMINE_OBJ;
		object_ = this;
		this->setObjectId(objectId);
		sendNeed();
	}

	GoldmineProxy::~GoldmineProxy()
	{
		sendDontNeed();
	}
}

