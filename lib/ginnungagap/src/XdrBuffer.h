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

#ifndef GINNUNGAGAP_XDRBUFFER_H
#define GINNUNGAGAP_XDRBUFFER_H

#include "Uuid.h"

#include <rpc/xdr.h>
#include <string>

namespace ggg
{
	enum xdrSize
	{
		NETADDR = 8,
		INT = 4,
		OBJID = 64,
		UUID = 64
	};

	class XdrBuffer
	{
		public:
			XdrBuffer();
			virtual ~XdrBuffer();

			XDR* xdrStream();
			char* xdrBuffer() const;

			void writeToFile(const std::string& fileName) const;

			unsigned int bufferSize() const;

		protected:
			char* xdrBuffer_;
			unsigned int bufferSize_;
			XDR xdrStream_;

	};
}

#endif
