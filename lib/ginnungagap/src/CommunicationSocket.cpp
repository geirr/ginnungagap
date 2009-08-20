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

#include "CommunicationSocket.h"
#include "CommunicationSocketManager.h"
#include "Ginnungagap.h"
#include "MessageType.h"
#include "Version.h"
#include "XdrSendBuffer.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fcntl.h>

using std::cerr;
using std::endl;

namespace ggg
{
	CommunicationSocket::CommunicationSocket(const NetAddr& otherEnd) : otherEnd_(otherEnd)
	{
		if ((socketDescriptor_ = socket(PF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto)) == -1)
		{
			cerr << "CommunicationSocket error: socket" << endl;
		}
		if ((connect(socketDescriptor_, (struct sockaddr*) otherEnd_.address(), sizeof(struct sockaddr))) == -1)
		{
			cerr << "CommunicationSocket error: connect" << endl;
		}

		/* Set to non-blocking */
		int flags = fcntl(socketDescriptor_, F_GETFL);
		fcntl(socketDescriptor_, F_SETFL, flags | O_NONBLOCK);

		/* Create INIT message */
		int msgType = INIT;
		int majorVersion = MAJORVER;
		int minorVersion = MINORVER;
		XdrSendBuffer* xdrBuffer = new XdrSendBuffer(INT*3+NETADDR); 
		*xdrBuffer << msgType << majorVersion << minorVersion << Ginnungagap::Instance()->netAddr();
		outputBuffer_ = new OutputBuffer;
		outputBuffer_->xdr = xdrBuffer;
		outputBuffer_->totalSent = 0;
		inputBuffer_ = 0;

		verified_ = false;
		outputBufferQueue_ = 0;
		inputBuffer_ = 0;
		pthread_mutex_init(&outputBufferQueueLock_, 0);

		Ginnungagap::Instance()->communicationSocketManager()->addSocket(this);
		Ginnungagap::Instance()->addSocketToDataSender(this);
		Ginnungagap::Instance()->addSocketToDataReceiver(this);
	}

	CommunicationSocket::CommunicationSocket(int socketDescriptor)
	{
		socketDescriptor_ = socketDescriptor;

		/* Set to non-blocking */
		int flags = fcntl(socketDescriptor_, F_GETFL);
		fcntl(socketDescriptor_, F_SETFL, flags | O_NONBLOCK);

		otherEnd_ = NetAddr("0.0.0.0:0");
		outputBufferQueue_ = 0;
		outputBuffer_ = 0;
		inputBuffer_ = 0;
		verified_ = false;
		pthread_mutex_init(&outputBufferQueueLock_, 0);
		Ginnungagap::Instance()->addSocketToDataReceiver(this);
	}

	CommunicationSocket::~CommunicationSocket()
	{
		Ginnungagap::Instance()->removeSocketFromManager(this);
		Ginnungagap::Instance()->removeSocketFromDataSender(this);
		Ginnungagap::Instance()->removeClosedSocket(this);
		/* delete sendQueueBuffers */
		if (outputBufferQueue_ != 0)
		{
			/* delete all; */
			while (!(outputBufferQueue_->empty()))
			{
				delete outputBufferQueue_->front();
				outputBufferQueue_->pop();
			}
			delete outputBufferQueue_;
		}
		if (outputBuffer_ != 0)
		{
			delete outputBuffer_;
		}
		pthread_mutex_destroy(&outputBufferQueueLock_);
	}

	OutputBuffer*& CommunicationSocket::outputBuffer()
	{
		return outputBuffer_;
	}

	InputBuffer*& CommunicationSocket::inputBuffer()
	{
		return inputBuffer_;
	}

	std::queue<OutputBuffer*>*& CommunicationSocket::outputBufferQueue()
	{
		return outputBufferQueue_;
	}

	void CommunicationSocket::lockOutputBufferQueue()
	{
		pthread_mutex_lock(&outputBufferQueueLock_);
	}

	void CommunicationSocket::unlockOutputBufferQueue()
	{
		pthread_mutex_unlock(&outputBufferQueueLock_);
	}

	void CommunicationSocket::verify()
	{
		verified_ = true;
		Ginnungagap::Instance()->communicationSocketManager()->addSocket(this);
		if (outputBufferQueue_ != 0)
		{
			Ginnungagap::Instance()->addSocketToDataSender(this);
		}
	}

	void CommunicationSocket::sendXdrBuffer(XdrBuffer* xdrBuffer)
	{
		OutputBuffer* outputBuffer = new OutputBuffer;
		outputBuffer->xdr = xdrBuffer;
		outputBuffer->totalSent = 0;
		pthread_mutex_lock(&outputBufferQueueLock_);
		if (outputBufferQueue_ == 0)
		{
			outputBufferQueue_ = new std::queue<OutputBuffer*>;
			outputBufferQueue_->push(outputBuffer);
		}
		else
		{
			outputBufferQueue_->push(outputBuffer);
		}
		if (verified_)
			Ginnungagap::Instance()->addSocketToDataSender(this);
		pthread_mutex_unlock(&outputBufferQueueLock_);
	}

	int CommunicationSocket::send(const char* buffer, const int bufferLength)
	{
		int bytesSent = ::send(socketDescriptor_, buffer, bufferLength, 0);
		return bytesSent;
	}

	int CommunicationSocket::recv(char* buffer, const int bufferLength)
	{
		int recived = ::recv(socketDescriptor_, buffer, bufferLength, 0);
		return recived;
	}

	void CommunicationSocket::sendAll(const char* buffer, const int bufferLength)
	{
		int totalSent = 0;
		int bytesLeft = bufferLength;
		int bytesSent;

		while (totalSent < bytesLeft)
		{
			if (this == 0)
			{
				break;
			}
			bytesSent = ::send(socketDescriptor_, buffer+totalSent, bytesLeft, 0);
			if (bytesSent == -1)
			{
				break;
			}
			totalSent += bytesSent;
			bytesLeft -= bytesSent;
		}
	}

	CommunicationSocket& operator<<(CommunicationSocket& to, XdrBuffer& from)
	{
		to.sendAll(from.xdrBuffer(), from.bufferSize());
		return to;
	}

	NetAddr CommunicationSocket::otherEnd() const
	{
		return otherEnd_;
	}

	void CommunicationSocket::setOtherEnd(const NetAddr& otherEnd)
	{
		otherEnd_ = otherEnd;
	}
}

