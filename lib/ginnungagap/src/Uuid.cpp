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

#include "Uuid.h"

#include <string>

using std::string;

namespace ggg
{
	Uuid::Uuid()
	{
		uuid_generate(uuid_);
	}

	Uuid::Uuid(std::string uuidStr)
	{
		uuid_parse(uuidStr.c_str(), uuid_);
	}

	const uuid_t* Uuid::getUuidPtr() const
	{
		return &uuid_;
	}

	std::string Uuid::getHexString() const
	{
		std::string retStr;
		char* char_retStr = new char[37]; //36 bytes + trailing \0
		uuid_unparse(uuid_, char_retStr);
		retStr = char_retStr;
		return retStr;
	}

	bool Uuid::operator==(const Uuid& uuid) const
	{
		if (uuid_compare(uuid_, *(uuid.getUuidPtr())) == 0)
			return true;
		return false;
	}

	bool Uuid::operator!=(const Uuid& uuid) const
	{
		if (uuid_compare(uuid_, *(uuid.getUuidPtr())) == 0)
			return false;
		return true;
	}

	bool Uuid::operator<(const Uuid& uuid) const
	{
		if (uuid_compare(uuid_, *(uuid.getUuidPtr())) < 0)
			return true;
		return false;
	}
}

