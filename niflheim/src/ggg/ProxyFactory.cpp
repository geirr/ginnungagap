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

#include "ProxyFactory.h"

#include "Object.h"

#include "World.h"
#include "WorldProxy.h"

#include "Avatar.h"
#include "AvatarProxy.h"

#include "View.h"
#include "ViewProxy.h"

#include "Goldmine.h"
#include "GoldmineProxy.h"

namespace ggg
{
	template <> Object* ProxyFactory::createProxy<niflheim::World>(const Uuid& objectId)
	{
		return new WorldProxy(objectId);
	}

	template <> Object* ProxyFactory::createProxy<niflheim::Avatar>(const Uuid& objectId)
	{
		return new AvatarProxy(objectId);
	}

	template <> Object* ProxyFactory::createProxy<niflheim::View>(const Uuid& objectId)
	{
		return new ViewProxy(objectId);
	}

	template <> Object* ProxyFactory::createProxy<niflheim::Goldmine>(const Uuid& objectId)
	{
		return new GoldmineProxy(objectId);
	}
}

