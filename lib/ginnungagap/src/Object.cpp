/************************************************************************
*                                                                       *
*  Copyright (C) 2009 by Geir Erikstad                                  *
*  geirr@baldr.no                                                       *
*                                                                       *
*  This file is part of Ginnungagap.                                    *
*                                                                       *
*  Ginnungagap is free software: you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by *
*  the Free Software Foundation, either version 2 of the License, or    *
*  (at your option) any later version.                                  *
*                                                                       *
*  Ginnungagap is distributed in the hope that it will be useful,       *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         *
*  GNU General Public License for more details.                         *
*                                                                       *
*  You should have received a copy of the GNU General Public License    *
*  along with Ginnungagap. If not, see <http://www.gnu.org/licenses/>.  *
*                                                                       *
************************************************************************/

#include "Object.h"
#include <iostream>

namespace ggg
{
	Object::~Object() {}

	void Object::generateObjectId()
	{
		objectId_ = Uuid();
	}

	void Object::setObjectId(const Uuid& objectId)
	{
		objectId_ = objectId;
	}

	Uuid Object::objectId() const
	{
		return objectId_;
	}

	bool Object::hasObjectId() const
	{
		if (objectId_ != Uuid("00000000-0000-0000-0000-000000000000"))
			return true;
		return false;
	}

	Object::ObjectState Object::objectState() const
	{
		return objectState_;
	}

	void Object::setObjectState(const ObjectState& objectState)
	{
		objectState_ = objectState;
	}
}

