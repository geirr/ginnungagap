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

#include <Ginnungagap.h>
#include <NameService.h>
#include <dist_ptr.h>

#include "TestObject.h"

#include <pthread.h>
#include <string>
#include <iostream>
#include <errno.h>

using std::cerr;
using std::endl;
using std::string;

using ggg::Ginnungagap;
using ggg::NetAddr;
using ggg::Uuid;
using ggg::PROXY;

int main(int argc, char* argv[])
{
	/* get the NetAddr of the server */
	NetAddr netAddr(argv[1]);
	NetAddr server(argv[2]);

	/* Initialize Server with our NetAddr */
	Ginnungagap::Initialize(PROXY, 50, netAddr, server);

	/* Start necessary threads */
	Ginnungagap::Instance()->startDataSenderThread();
	Ginnungagap::Instance()->startDataAndConnectionReceiverThread();
	Ginnungagap::Instance()->messageHandler();
	return 1;
}

