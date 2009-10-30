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


#include "ObjectFactory.h"

#include "Object.h"

#include "World.h"
#include "Avatar.h"

#include <iostream>

using std::cerr;
using std::endl;

namespace ggg
{
	Object* ObjectFactory::createObject(int objectName, XdrReceiveBuffer* xdr)
	{
		switch (objectName)
		{
			case NIFLHEIM_WORLD_OBJ:
				return new niflheim::World(xdr);
				break;
			case NIFLHEIM_AVATAR_OBJ:
				return new niflheim::Avatar(xdr);
				break;
			case NIFLHEIM_GOLDMINE_OBJ:
				return new niflheim::Goldmine(xdr);
				break;
			default:
				cerr << "Error creating object" << endl;
				break;
		}
		return 0;
	}
}

