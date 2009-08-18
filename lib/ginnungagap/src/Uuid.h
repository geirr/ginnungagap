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

#ifndef GINNUNGAGAP_OBJECTID_H
#define GINNUNGAGAP_OBJECTID_H

#include <uuid/uuid.h>
#include <string>

namespace ginnungagap
{
	class Uuid
	{
		public:
			Uuid();
			Uuid(std::string);
			std::string getHexString() const;
			bool operator==(const Uuid& uuid) const;
			bool operator!=(const Uuid& uuid) const;
			bool operator<(const Uuid& uuid) const;
			const uuid_t* getUuidPtr() const;
		private:
			uuid_t uuid_;
	};
}

#endif
