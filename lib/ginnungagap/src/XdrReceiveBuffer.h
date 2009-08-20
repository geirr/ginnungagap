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

#ifndef GINNUNGAGAP_XDRRECIVEBUFFER_H
#define GINNUNGAGAP_XDRRECIVEBUFFER_H

#include "XdrBuffer.h"
#include "Uuid.h"
#include "NetAddr.h"

#include <string>

namespace ggg
{
	class XdrReceiveBuffer : public XdrBuffer
	{
		public:
			XdrReceiveBuffer(unsigned int bufferSize);
			XdrReceiveBuffer(const std::string& fileName);
			XdrReceiveBuffer(const XdrReceiveBuffer& xdrBuffer);
			XdrReceiveBuffer(const XdrBuffer& xdrBuffer);
			~XdrReceiveBuffer() {}

			friend XdrReceiveBuffer& operator>>(XdrReceiveBuffer& xdrReciveBuffer, int& integer);
			friend XdrReceiveBuffer& operator>>(XdrReceiveBuffer& xdrReciveBuffer, Uuid& uuid);
			friend XdrReceiveBuffer& operator>>(XdrReceiveBuffer& xdrReciveBuffer, NetAddr& uuid);
	};
}

#endif
