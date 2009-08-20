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

#ifndef NIFLHEIM_WORLDOBJECTS_H
#define NIFLHEIM_WORLDOBJECTS_H

namespace niflheim
{
	enum WorldObjectType
	{
		AVATAR,
		GOLDMINE,
		VOID,
		NOTHING,
		BUFFER_AVATAR,
		BUFFER_GOLDMINE,
		BUFFER_VOID,
		BUFFER_NOTHING,
		SAVED
	};

	struct WorldObjectInfo
	{
		WorldObjectType worldObjectType;
		std::pair<int, int> position;
	};
}

#endif
