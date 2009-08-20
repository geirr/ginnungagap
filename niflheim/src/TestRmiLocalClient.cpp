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

#include <fstream>

using std::cout; using std::endl; using std::pair; using std::cerr; using std::ofstream; using std::string;

using ggg::Ginnungagap; using ggg::dist_ptr; using ggg::NetAddr;
using ggg::Uuid; using ggg::CLIENT;

int main(int argc, char* argv[])
{
	NetAddr thisAddr(argv[1]);
	NetAddr mainServAddr(argv[2]);

	string whatToTest = argv[3];

	/* Initialize Server */
	Ginnungagap::Initialize(CLIENT, 10, thisAddr, mainServAddr);

	/* Start threads */
	Ginnungagap::Instance()->startDataAndConnectionReceiverThread();
	Ginnungagap::Instance()->startDataSenderThread();
	Ginnungagap::Instance()->startMessageHandlerThread();

	/* Create a local object, and bind it to ns  */
	TestObject* testobj = new TestObject();
	testobj->setObjectId(Uuid("00000000-0000-0000-0000-000000000001"));
	Ginnungagap::Instance()->nameService()->bind(*testobj);

	dist_ptr<TestObject> test = dist_ptr<TestObject>(Uuid("00000000-0000-0000-0000-000000000001"));

	test->void_void();

	struct timeval before;
	struct timeval after;

	int ret;
	int retValue;

	int testNumber = 10000;

	/* just to make sure we have connected */
	sleep(5);

	if (whatToTest == "0")
	{
		ofstream void_void("void_void");
		if(!void_void) { 
			cout << "Cannot open file.\n"; 
			exit(0);
		} 
		for (int i = 0; i != testNumber; ++i)
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
			double time = (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
			void_void << time << endl;
		}
		void_void.close();
	}
	else if (whatToTest == "1")
	{
		ofstream void_int("void_int");
		if(!void_int) { 
			cout << "Cannot open file.\n"; 
			exit(0);
		} 
		for (int i = 0; i != testNumber; ++i)
		{
			retValue = gettimeofday(&before, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}

			test->void_int(0);

			retValue = gettimeofday(&after, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}
			double time = (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
			void_int << time << endl;
		}
		void_int.close();
	}
	else if (whatToTest == "2")
	{
		ofstream int_void("int_void");
		if(!int_void) { 
			cout << "Cannot open file.\n"; 
			exit(0);
		} 
		for (int i = 0; i != testNumber; ++i)
		{
			retValue = gettimeofday(&before, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}

			ret = test->int_void();

			retValue = gettimeofday(&after, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}
			double time = (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
			int_void << time << endl;
		}
		int_void.close();
	}
	else if (whatToTest == "3")
	{
		ofstream int_int("int_int");
		if(!int_int) { 
			cout << "Cannot open file.\n"; 
			exit(0);
		} 
		for (int i = 0; i != testNumber; ++i)
		{
			retValue = gettimeofday(&before, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}

			ret = test->int_int(1);

			retValue = gettimeofday(&after, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}
			double time = (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
			int_int << time << endl;
		}
		int_int.close();
	}
	else if (whatToTest == "4")
	{
		ofstream int_intInt("int_intInt");
		if(!int_intInt) { 
			cout << "Cannot open file.\n"; 
			exit(0);
		} 
		for (int i = 0; i != testNumber; ++i)
		{
			retValue = gettimeofday(&before, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}

			ret = test->int_intInt(1, 2);

			retValue = gettimeofday(&after, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}
			double time = (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
			int_intInt << time << endl;
		}
		int_intInt.close();
	}
	else if (whatToTest == "5")
	{
		ofstream int_intIntInt("int_intIntInt");
		if(!int_intIntInt) { 
			cout << "Cannot open file.\n"; 
			exit(0);
		} 
		for (int i = 0; i != testNumber; ++i)
		{
			retValue = gettimeofday(&before, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}

			ret = test->int_intIntInt(1, 2, 3);

			retValue = gettimeofday(&after, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}
			double time = (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
			int_intIntInt << time << endl;
		}
		int_intIntInt.close();
	}
	else if (whatToTest == "6")
	{
		ofstream vectorOfIntsX10("vectorOfIntsX10");
		if(!vectorOfIntsX10) { 
			cout << "Cannot open file.\n"; 
			exit(0);
		} 
		std::vector<int> intsX10(10, 4);
		for (int i = 0; i != intsX10.size(); ++i)
			intsX10[i] = i;
		for (int i = 0; i != testNumber; ++i)
		{
			retValue = gettimeofday(&before, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}

			ret = test->int_vectorOfInts(intsX10);

			retValue = gettimeofday(&after, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}
			double time = (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
			vectorOfIntsX10 << time << endl;
		}
		vectorOfIntsX10.close();
	}
	else if (whatToTest == "7")
	{
		ofstream vectorOfIntsX100("vectorOfIntsX100");
		if(!vectorOfIntsX100) { 
			cout << "Cannot open file.\n"; 
			exit(0);
		} 
		std::vector<int> intsX100(100, 6);
		for (int i = 0; i != intsX100.size(); ++i)
			intsX100[i] = i;
		for (int i = 0; i != testNumber; ++i)
		{
			retValue = gettimeofday(&before, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}

			ret = test->int_vectorOfInts(intsX100);

			retValue = gettimeofday(&after, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}
			double time = (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
			vectorOfIntsX100 << time << endl;
		}
		vectorOfIntsX100.close();
	}
	else if (whatToTest == "8")
	{
		ofstream vectorOfIntsX1000("vectorOfIntsX1000");
		if(!vectorOfIntsX1000) { 
			cout << "Cannot open file.\n"; 
			exit(0);
		} 
		std::vector<int> intsX1000(1000, 7);
		for (int i = 0; i != intsX1000.size(); ++i)
			intsX1000[i] = i;
		for (int i = 0; i != testNumber; ++i)
		{
			retValue = gettimeofday(&before, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}

			ret = test->int_vectorOfInts(intsX1000);

			retValue = gettimeofday(&after, 0);
			if (retValue != 0) {
				cerr << "Error with gettimeofday" << endl;
				exit(0);
			}
			double time = (after.tv_sec - before.tv_sec)*1000000 + (after.tv_usec - before.tv_usec);
			vectorOfIntsX1000 << time << endl;
		}
		vectorOfIntsX1000.close();
	}
}

