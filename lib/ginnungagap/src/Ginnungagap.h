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

#ifndef GINNUNGAGAP_GINNUNGAGAP_H
#define GINNUNGAGAP_GINNUNGAGAP_H

#include "ListenSocket.h"
#include "CommunicationSocket.h"
#include "Uuid.h"
#include "BufferStructures.h"
#include "MessageType.h"
#include "MigrationGroup.h"
#include "MessageHandler.h"

#include <pthread.h>
#include <sys/epoll.h>

#include <stack>
#include <queue>
#include <map>
#include <list>
#include <set>
#include <fstream>
#include <iostream>

namespace ggg
{
	class NameService;
	class MigrationService;
	class CommunicationSocketManager;
	class NetworkGraph;
	class XdrReceiveBuffer;
	class XdrSendBuffer;
	class Skeleton;
	class ProxyFactory;
	class ObjectFactory;

	enum AppType
	{
		SERVER,
		PROXY,
		CLIENT
	};

	class Ginnungagap
	{
		public:
			static Ginnungagap* Initialize(const AppType& appType, const unsigned int& maxConnections, const NetAddr& listenAddr, const NetAddr& mainServer, const double& coreSelThreshold = 10, const unsigned int& coreSelInterval = 60);
			static Ginnungagap* Instance();

			XdrReceiveBuffer* getRmiRetVal(const Uuid& objectId, const NetAddr& netAddr);
			void giveRmiRetVal(XdrReceiveBuffer* xdrBuffer);

			void sendXdrBuffer(const NetAddr& netAddr, XdrBuffer* xdrBuffer);

			NameService* nameService();
			MigrationService* migrationService();

			NetAddr netAddr() const;
			NetAddr mainServer() const;
			AppType appType() const;

			void sendMainServerInit();
			void sendProxyInit();

			void sendNewProxyInfo();
			void sendProxyInfo(const NetAddr& to);
			void readProxyInfo(XdrReceiveBuffer* xdrBuffer);

			void addMigrationGroup(MigrationGroup* mg);

			bool haveConnectionTo(const NetAddr& netAddr);

			double coreSelectionThreshold() const;

			void writeToLog(const std::string& logMsg);

			/* Functions to start new threads */
			void startDataAndConnectionReceiverThread();
			void startMessageHandlerThread();
			void startMessageHandlerWithCoreSelectionThread();
			void startDataSenderThread();
			void startLatencyMesurerThread();

			/* Need to be public because of c calling conventions 
			 * dont call these directly, but use the functions
			 * right above, as long as you want it in a new thread.
			 * Call these if you dont want to start another tread. */
			void dataAndConnectionsReceiver();
			void messageHandler();
			void messageHandlerWithCoreSelection();
			void dataSender();
			void latencyMeasurer();


		private:
			friend class CommunicationSocket;
			friend class MessageHandler;
			friend class MigrationGroup;
			template<class T> friend class dist_ptr;

			NetworkGraph* networkGraph();
			CommunicationSocketManager* communicationSocketManager();
			ProxyFactory* proxyFactory();
			ObjectFactory* objectFactory();
			Skeleton* skeleton();

			AppType appType_;

			/* Core selection parameters */
			const double coreSelectionThreshold_;
			const unsigned int coreSelInterval_;

			void removeSocketFromManager(const CommunicationSocket* commSock);
			void removeClientOrServer(const NetAddr& netAddr);

			const unsigned int maxConnections_;

			NetAddr netAddr_;
			NetAddr mainServer_;
			ListenSocket* listenSocket_;

			NameService* nameService_;
			CommunicationSocketManager* communicationSocketManager_;
			MigrationService* migrationService_;
			MessageHandler* messageHandler_;
			NetworkGraph* networkGraph_;
			ProxyFactory* proxyFactory_;
			ObjectFactory* objectFactory_;
			Skeleton* skeleton_;

			/* Logfile */
			std::ofstream logfile_;


			/* Threads */
			pthread_t messageHandlerWithCoreSelectionThread_;
			pthread_t messageHandlerThread_;
			pthread_t dataSenderThread_;
			pthread_t dataAndConnectionsReceiverThread_;
			pthread_t latencyMeasurerThread_;

			/* We don't want to use the closed sockets */
			std::set<CommunicationSocket*> closedSockets_;
			bool isClosed(CommunicationSocket* commSock);
			void addClosedSocket(CommunicationSocket* commSock);
			void removeClosedSocket(CommunicationSocket* commSock);
			pthread_mutex_t closedSocketsLock_;

			/* Epoll for dataAndConnectionReceiver */
			int inEpollDescriptor_;
			void addSocketToDataReceiver(const Socket* socket);
			void removeSocketFromDataReceiver(CommunicationSocket* socket);

			/* Epoll and synchronization for dataSender */
			int outEpollDescriptor_;
			void addSocketToDataSender(CommunicationSocket* socket);
			void removeSocketFromDataSender(CommunicationSocket* socket);

			/* Synchronization for messageHandler */
			std::queue<Message*> messageQueue_;
			pthread_mutex_t messageQueueLock_;
			pthread_cond_t messageQueueHasItems_;

			/* Synchronization for RMI return values */
			pthread_mutex_t rmiRetValXdrBufferLock_;
			pthread_cond_t rmiRetValXdrBufferNotEmpty_;
			XdrReceiveBuffer* rmiRetValXdrBuffer_;

			/* Used by clients for a list of all servers and proxies to ping.
			 * Used by server for a list of proxies */
			std::list<NetAddr> serverAndProxies_;
			void addServerOrProxy(const NetAddr& netAddr);

			/* Singleton pattern */
			Ginnungagap(const AppType& appType, const unsigned int& maxConnections, const NetAddr& listenAddr, const NetAddr& mainServer, const double& coreSelThreshold, const unsigned int& coreSelInterval);
			~Ginnungagap();
			Ginnungagap(const Ginnungagap&);
			Ginnungagap operator=(const Ginnungagap&);
			static Ginnungagap* pInstance_;
	};
}

#endif

