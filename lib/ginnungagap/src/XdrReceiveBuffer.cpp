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

#include "XdrReceiveBuffer.h"

#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>

using std::ifstream;
using std::ios;
using std::cerr;
using std::endl;

namespace ginnungagap
{
	XdrReceiveBuffer::XdrReceiveBuffer(unsigned int bufferSize)
	{
		bufferSize_ = bufferSize;

		xdrBuffer_ = new char[bufferSize_];
		memset(xdrBuffer_, 0, bufferSize_);

		xdrmem_create(&xdrStream_, xdrBuffer_, bufferSize_, XDR_DECODE);
	}
	XdrReceiveBuffer::XdrReceiveBuffer(const std::string& fileName)
	{
		ifstream inFile(fileName.c_str(), ios::in | ios::binary);
		bool_t ret;

		char* bufferForSize = new char[4];
		memset(bufferForSize, 0, 4);
		inFile.read(bufferForSize, 4);
		XDR size;
		xdrmem_create(&size, bufferForSize, 4, XDR_DECODE);

		ret = xdr_u_int(&size, &bufferSize_);
		if (ret != 1)
			cerr << "Error reading size from XdrReceiveBuffer" << endl;

		xdrBuffer_ = new char[bufferSize_];
		memset(xdrBuffer_, 0, bufferSize_);

		inFile.read(xdrBuffer_, bufferSize_);

		xdrmem_create(&xdrStream_, xdrBuffer_, bufferSize_, XDR_DECODE);
	}

	XdrReceiveBuffer::XdrReceiveBuffer(const XdrReceiveBuffer& xdrBuffer)
	{
		bufferSize_ = xdrBuffer.bufferSize();
		xdrBuffer_ = new char[bufferSize_];
		memcpy(xdrBuffer_, xdrBuffer.xdrBuffer(), bufferSize_);
		xdrmem_create(&xdrStream_, xdrBuffer_, bufferSize_, XDR_DECODE);
	}

	XdrReceiveBuffer::XdrReceiveBuffer(const XdrBuffer& xdrBuffer)
	{
		bufferSize_ = xdrBuffer.bufferSize();
		xdrBuffer_ = new char[bufferSize_];
		memcpy(xdrBuffer_, xdrBuffer.xdrBuffer(), bufferSize_);
		xdrmem_create(&xdrStream_, xdrBuffer_, bufferSize_, XDR_DECODE);
	}

	XdrReceiveBuffer& operator>>(XdrReceiveBuffer& xdrReciveBuffer, int& integer)
	{
		bool_t ret = xdr_int(xdrReciveBuffer.xdrStream(), &integer);
		if (ret != 1)
			cerr << "Error reading int from XdrReceiveBuffer" << endl;
		return xdrReciveBuffer;
	}

	XdrReceiveBuffer& operator>>(XdrReceiveBuffer& xdrReciveBuffer, Uuid& uuid)
	{
		//uuid_t = unsigned char[16]
		const u_char* const_uuid_ = (u_char*) uuid.getUuidPtr();
		u_char* uuid_ = const_cast<u_char*>(const_uuid_);
		for (int i = 0; i != 16; i++)
		{
			int ret = xdr_u_char(xdrReciveBuffer.xdrStream(), &uuid_[i]);
			if (ret != 1)
				cerr << "Error reading Uuid from XdrReceiveBuffer (i=" << i << ")" << endl;
		}
		return xdrReciveBuffer;
	}

	XdrReceiveBuffer& operator>>(XdrReceiveBuffer& xdrReciveBuffer, NetAddr& netAddr)
	{
		struct sockaddr_in* sockaddr = new struct sockaddr_in;
		bool_t ret;
		ret = xdr_u_int(xdrReciveBuffer.xdrStream(), &(sockaddr->sin_addr.s_addr));
		if (ret != 1)
			cerr << "Error reading ip from XdrReceiveBuffer" << endl;
		ret = xdr_u_short(xdrReciveBuffer.xdrStream(), &(sockaddr->sin_port));
		if (ret != 1)
			cerr << "Error reading port from XdrReceiveBuffer" << endl;
		netAddr.setAddress(*sockaddr);
		delete sockaddr;
		return xdrReciveBuffer;
	}
}

