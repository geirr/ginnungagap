
#include "TestObject.h"

#include <iostream>

#include <string>
#include <errno.h>

#include <fstream>

using namespace std;

int main(int argc, char* argv[])
{
	TestObject* test = new TestObject();

	string whatToTest = argv[1];

	struct timeval before;
	struct timeval after;

	test->void_void();

	int ret;
	int retValue;

	int testNumber = 10000;

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
	return 0;
}
