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

#include "XdrSendBuffer.h"

#include <iostream>
#include <string.h>

using std::cerr;
using std::endl;

namespace ginnungagap
{
	XdrSendBuffer::XdrSendBuffer(unsigned int bufferSize)
	{
		/* We want to store the size at the start */
		bufferSize_ = bufferSize + INT;
		xdrBuffer_ = new char[bufferSize_];
		memset(xdrBuffer_, 0, bufferSize_);
		xdrmem_create(&xdrStream_, xdrBuffer_, bufferSize_, XDR_ENCODE);

		/* Store size */
		bool_t ret = xdr_u_int(&xdrStream_, &bufferSize);
		if (ret != 1)
			cerr << "Error writing size to XdrSendBuffer" << endl;
	}

	XdrSendBuffer::XdrSendBuffer(int& msgType, const NetAddr& netAddr, XdrBuffer* xdrBuffer)
	{
		bufferSize_ = xdrBuffer->bufferSize() + NETADDR + INT;
		xdrBuffer_ = new char[bufferSize_];

		/* Make tmp xdr buffer with netAddr and new size */
		char* tmpBuffer = new char[NETADDR + INT*2];
		memset(tmpBuffer, 0, NETADDR + INT*2);
		XDR tmpXdr;
		xdrmem_create(&tmpXdr, tmpBuffer, NETADDR + INT*2, XDR_ENCODE);

		/* Store size */
		unsigned int size = bufferSize_ - INT;
		bool_t ret = xdr_u_int(&tmpXdr, &size);
		if (ret != 1)
			cerr << "Error writing size to XdrSendBuffer" << endl;

		/* Store msg type */
		ret = xdr_int(&tmpXdr, &msgType);
		if (ret != 1)
			cerr << "Error writing msg type to XdrSendBuffer" << endl;

		/* Store netAddr */
		struct sockaddr_in* sockaddr = netAddr.address();
		ret = xdr_u_int(&tmpXdr, &(sockaddr->sin_addr.s_addr));
		if (ret != 1)
			cerr << "Error writing ip to XdrSendBuffer" << endl;
		ret = xdr_u_short(&tmpXdr, &(sockaddr->sin_port));
		if (ret != 1)
			cerr << "Error writing port to XdrSendBuffer" << endl;

		/* Copy netAddr and size to start of new buffer, then the old xdr buffer */
		memcpy(xdrBuffer_, tmpBuffer, NETADDR + INT*2);
		memcpy(xdrBuffer_ + INT*2 + NETADDR, xdrBuffer->xdrBuffer() + INT, xdrBuffer->bufferSize() - INT);

		xdrmem_create(&xdrStream_, xdrBuffer_, bufferSize_, XDR_ENCODE);

		xdr_destroy(&tmpXdr);
		delete[] tmpBuffer;
	}

	XdrSendBuffer::XdrSendBuffer(const XdrSendBuffer& xdrBuffer)
	{
		bufferSize_ = xdrBuffer.bufferSize();
		xdrBuffer_ = new char[bufferSize_];
		memcpy(xdrBuffer_, xdrBuffer.xdrBuffer(), bufferSize_);
		xdrmem_create(&xdrStream_, xdrBuffer_, bufferSize_, XDR_ENCODE);
	}

	XdrSendBuffer::XdrSendBuffer(XdrReceiveBuffer* xdrBuffer)
	{
		unsigned int bufferSize = xdrBuffer->bufferSize();
		bufferSize_ = bufferSize + INT;
		xdrBuffer_ = new char[bufferSize_];


		char* tmpBuffer = new char[INT];
		memset(tmpBuffer, 0, INT);
		XDR tmpXdr;
		xdrmem_create(&tmpXdr, tmpBuffer, INT, XDR_ENCODE);
		bool_t ret = xdr_u_int(&tmpXdr, &bufferSize);
		if (ret != 1)
			cerr << "Error writing size to XdrSendBuffer" << endl;


		memcpy(xdrBuffer_, tmpBuffer, INT);
		memcpy(xdrBuffer_+INT, xdrBuffer->xdrBuffer(), bufferSize);
		xdrmem_create(&xdrStream_, xdrBuffer_, bufferSize_, XDR_ENCODE);

		xdr_destroy(&tmpXdr);
		delete[] tmpBuffer;
	}

	void XdrSendBuffer::insert(XdrSendBuffer* xdrBuffer, int offset)
	{
		int currPos = xdr_getpos(&xdrStream_);
		memcpy(xdrBuffer_+currPos, xdrBuffer->xdrBuffer()+INT+offset, xdrBuffer->bufferSize()-offset-INT);
		xdr_setpos(&xdrStream_, currPos+xdrBuffer->bufferSize()-offset-INT);
	}

	XdrSendBuffer& operator<<(XdrSendBuffer& xdrSendBuffer, int& integer)
	{
		bool_t ret = xdr_int(xdrSendBuffer.xdrStream(), &integer);
		if (ret != 1)
			cerr << "Error writing int to XdrSendBuffer" << endl;
		return xdrSendBuffer;
	}
	XdrSendBuffer& operator<<(XdrSendBuffer& xdrSendBuffer, const Uuid& uuid)
	{
		/* uuid_t = unsigned char[16] */
		u_char* uuidPtr = (u_char*) uuid.getUuidPtr();
		for (int i = 0; i != 16; i++)
		{
			int ret = xdr_u_char(xdrSendBuffer.xdrStream(), &uuidPtr[i]);
			if (ret != 1)
				cerr << "Error writing Uuid to XdrSendBuffer" << endl;
		}
		return xdrSendBuffer;
	}
	XdrSendBuffer& operator<<(XdrSendBuffer& xdrSendBuffer, const NetAddr& netAddr)
	{
		struct sockaddr_in* sockaddr = netAddr.address();
		bool_t ret;
		ret = xdr_u_int(xdrSendBuffer.xdrStream(), &(sockaddr->sin_addr.s_addr));
		if (ret != 1)
			cerr << "Error writing ip to XdrSendBuffer" << endl;
		ret = xdr_u_short(xdrSendBuffer.xdrStream(), &(sockaddr->sin_port));
		if (ret != 1)
			cerr << "Error writing port to XdrSendBuffer" << endl;
		delete sockaddr;
		return xdrSendBuffer;
	}
}

