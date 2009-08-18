#include "XdrSendBuffer.h"

#include <iostream>

using namespace ginnungagap;
using namespace std;

int main()
{
	cout << "oh: " << INT*4 << endl;
	XdrSendBuffer* test = new XdrSendBuffer(INT*4);
	int en = 0xfffffff;
	int to = 0xaaaaaaaa;
	int tre = 0xbbbbbbbb;
	int fire = 0x12345678;
	*test << en << to << tre << fire;
	XdrSendBuffer* test2 = new XdrSendBuffer(INT*8);
	int xx = 0xdddddddd;
	int xxx = 0xeeeeeeee;
	*test2 << xx;
	test2->insert(test, INT);
	*test2 << xxx;
	test->writeToFile("bahaha");
	test2->writeToFile("bahaha2");
	usleep(1000);
}
