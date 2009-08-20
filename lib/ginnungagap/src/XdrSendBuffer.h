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

#ifndef GINNUNGAGAP_XDRSENDBUFFER_H
#define GINNUNGAGAP_XDRSENDBUFFER_H

#include "XdrBuffer.h"
#include "XdrReceiveBuffer.h"
#include "Uuid.h"
#include "NetAddr.h"

namespace ggg
{
	class XdrSendBuffer : public XdrBuffer
	{
		public:
			XdrSendBuffer(unsigned int bufferSize);
			XdrSendBuffer(int& msgType, const NetAddr& netAddr, XdrBuffer* xdrBuffer);
			XdrSendBuffer(const XdrSendBuffer& xdrBuffer);
			XdrSendBuffer(XdrReceiveBuffer* xdrBuffer);
			~XdrSendBuffer() {}

			void insert(XdrSendBuffer* xdrBuffer, int offset = 0);

			friend XdrSendBuffer& operator<<(XdrSendBuffer& xdrSendBuffer, int& integer);
			friend XdrSendBuffer& operator<<(XdrSendBuffer& xdrSendBuffer, const Uuid& uuid);
			friend XdrSendBuffer& operator<<(XdrSendBuffer& xdrSendBuffer, const NetAddr& netAddr);
	};
}

#endif
