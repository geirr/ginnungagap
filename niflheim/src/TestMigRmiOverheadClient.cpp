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
#include <sstream>
#include <errno.h>

#include <fstream>

using std::cout; using std::endl; using std::pair; using std::cerr; using std::ofstream; using std::string;
using std::stringstream;


using ggg::Ginnungagap; using ggg::dist_ptr; using ggg::NetAddr;
using ggg::Uuid; using ggg::CLIENT;

int main(int argc, char* argv[])
{
	NetAddr thisAddr(argv[1]);
	NetAddr mainServAddr(argv[2]);

	/* Initialize Server */
	Ginnungagap::Initialize(CLIENT, 10, thisAddr, mainServAddr);

	/* Start threads */
	Ginnungagap::Instance()->startDataAndConnectionReceiverThread();
	Ginnungagap::Instance()->startDataSenderThread();
	Ginnungagap::Instance()->startMessageHandlerThread();

	/* Start looking for the test object at the main server */
	Ginnungagap::Instance()->nameService()->addRemoteObject(Uuid("00000000-0000-0000-0000-000000000001"), mainServAddr);
	dist_ptr<TestObject> test = dist_ptr<TestObject>(Uuid("00000000-0000-0000-0000-000000000001"));

	test->void_void();

	struct timeval before;
	struct timeval after;
	stringstream time;

	int retValue;

	//int testNumber = 10000;
	//for (int i = 0; i != testNumber; ++i)
	while(true)
	{
		retValue = gettimeofday(&before, 0);
		if (retValue != 0) {
			cerr << "Error with gettimeofday" << endl;
			exit(0);
		}

		test->void_void();

		retValue = gettimeofday(&after, 0);
		if (retValue != 0) {
			cerr << "Error with gettimeofday" << endl;
			exit(0);
		}

		time.str("");
		time << "void_void took: ";
		time << (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
		Ginnungagap::Instance()->writeToLog(time.str());
		usleep(100000);
	}
}

