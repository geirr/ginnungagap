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

#ifndef GINNUNGAGAP_COMMUNICATIONSOCKET_H
#define GINNUNGAGAP_COMMUNICATIONSOCKET_H

#include "Socket.h"
#include "NetAddr.h"
#include "BufferStructures.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

#include <queue>

namespace ggg
{
	class XdrSendBuffer;
	class XdrBuffer;

	class CommunicationSocket : public Socket
	{
		public:
			/* Outgoing */
			CommunicationSocket(const NetAddr& otherEnd);
			/* Incoming */
			CommunicationSocket(int socketDescriptor);
			~CommunicationSocket();

			friend CommunicationSocket& operator<<(CommunicationSocket& to, XdrBuffer& from);

			NetAddr otherEnd() const;
			void setOtherEnd(const NetAddr& otherEnd);

			void sendXdrBuffer(XdrBuffer* xdrBuffer);
			void verify();

			OutputBuffer*& outputBuffer();
			InputBuffer*& inputBuffer();
			std::queue<OutputBuffer*>*& outputBufferQueue();
			void lockOutputBufferQueue();
			void unlockOutputBufferQueue();

		private:
			bool verified_;
			NetAddr otherEnd_;

			OutputBuffer* outputBuffer_;
			InputBuffer* inputBuffer_;
			std::queue<OutputBuffer*>* outputBufferQueue_;
			pthread_mutex_t outputBufferQueueLock_;

			friend class Ginnungagap;
			friend class MessageHandler;

			int send(const char* buffer, const int bufferLength);
			int recv(char* buffer, const int bufferLength);
			void sendAll(const char* buffer, const int bufferLength);
	};
}

#endif

