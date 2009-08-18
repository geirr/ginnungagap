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

#include "XdrBuffer.h"

#include <rpc/xdr.h>
#include <fstream>
#include <iostream>
#include <iomanip>

using std::ofstream;
using std::ios;
using std::string;

using std::cerr;
using std::endl;

namespace ginnungagap
{
	XdrBuffer::XdrBuffer() {}
	XdrBuffer::~XdrBuffer()
	{
		xdr_destroy(&xdrStream_);
		delete [] xdrBuffer_;
	}

	XDR* XdrBuffer::xdrStream()
	{
		return &xdrStream_;
	}

	char* XdrBuffer::xdrBuffer() const
	{
		return xdrBuffer_;
	}

	void XdrBuffer::writeToFile(const string& fileName) const
	{
		ofstream outFile(fileName.c_str(), ios::out | ios::binary);
		outFile.write(xdrBuffer_, bufferSize_);
		outFile.close();
	}

	unsigned int XdrBuffer::bufferSize() const
	{
		return bufferSize_;
	}
}

