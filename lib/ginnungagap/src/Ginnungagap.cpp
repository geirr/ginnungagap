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

#include "Ginnungagap.h"
#include "MessageHandler.h"
#include "CommunicationSocketManager.h"
#include "NameService.h"
#include "MigrationService.h"
#include "NetworkGraph.h"
#include "XdrSendBuffer.h"
#include "XdrReceiveBuffer.h"
#include "ProxyFactory.h"
#include "ObjectFactory.h"
#include "Skeleton.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <ctime>
#include <unistd.h>

#include <algorithm>
#include <vector>
#include <list>
#include <map>

using std::cerr;
using std::endl;

using std::cout; using std::ofstream; using std::string; using std::istringstream; using std::ifstream; using std::stringstream;

using std::list;

extern "C" void* C_messageHandlerWithCoreSelection(void*);
extern "C" void* C_messageHandler(void*);
extern "C" void* C_dataSender(void*);
extern "C" void* C_dataAndConnectionsReceiver(void*);
extern "C" void* C_latencyMeasurer(void*);

void* C_messageHandlerWithCoreSelection(void*)
{
	ggg::Ginnungagap::Instance()->messageHandlerWithCoreSelection();
	return 0;
}

void* C_messageHandler(void*)
{
	ggg::Ginnungagap::Instance()->messageHandler();
	return 0;
}

void* C_dataSender(void*)
{
	ggg::Ginnungagap::Instance()->dataSender();
	return 0;
}

void* C_dataAndConnectionsReceiver(void*)
{
	ggg::Ginnungagap::Instance()->dataAndConnectionsReceiver();
	return 0;
}

void* C_latencyMeasurer(void*)
{
	ggg::Ginnungagap::Instance()->latencyMeasurer();
	return 0;
}

namespace ggg
{
	typedef std::list<NetAddr>::iterator spItr_t;

	Ginnungagap* Ginnungagap::pInstance_ = 0;

	Ginnungagap* Ginnungagap::Initialize(const AppType& appType, const unsigned int& maxConnections, const NetAddr& netAddr, const NetAddr& mainServer, const double& coreSelThreshold, const unsigned int& coreSelInterval)
	{
		if (!pInstance_)
		{
			pInstance_ = new Ginnungagap(appType, maxConnections, netAddr, mainServer, coreSelThreshold, coreSelInterval);
			return pInstance_;
		}
		else
		{
			cerr << "Error: Can only call initialize once." << endl;
			exit(1);
		}
	}

	Ginnungagap* Ginnungagap::Instance()
	{
		if (!pInstance_)
		{
			cerr << "Error: Need to initialize Ginnungagap first." << endl;
			exit(1);
		}
		return pInstance_;
	}

	Ginnungagap::Ginnungagap(const AppType& appType, const unsigned int& maxConnections, const NetAddr& netAddr, const NetAddr& mainServer, const double& coreSelThreshold, const unsigned int& coreSelInterval) :
		appType_(appType), coreSelectionThreshold_(coreSelThreshold), coreSelInterval_(coreSelInterval), maxConnections_(maxConnections), netAddr_(netAddr), mainServer_(mainServer)
	{
		if ((inEpollDescriptor_ = epoll_create(maxConnections_)) == -1)
			perror("Error creating in epoll descriptor");
		if ((outEpollDescriptor_ = epoll_create(maxConnections_)) == -1)
			perror("Error creating out epoll descriptor");
		pthread_mutex_init(&messageQueueLock_, 0);
		pthread_cond_init(&messageQueueHasItems_, 0);
		pthread_mutex_init(&rmiRetValXdrBufferLock_, 0);
		pthread_cond_init(&rmiRetValXdrBufferNotEmpty_, 0);
		pthread_mutex_init(&closedSocketsLock_, 0);
		rmiRetValXdrBuffer_ = 0;
		nameService_ = new NameService;
		communicationSocketManager_ = new CommunicationSocketManager;
		migrationService_ = new MigrationService;
		messageHandler_ =  new MessageHandler;
		proxyFactory_ = new ProxyFactory;
		objectFactory_ = new ObjectFactory;
		skeleton_ = new Skeleton;
		if (appType_ == CLIENT)
		{
			networkGraph_ = 0;
			addServerOrProxy(mainServer_);
		}
		else if (appType_ == SERVER)
		{
			networkGraph_ = new NetworkGraph;
			networkGraph_->addServer(netAddr);
		}
		else /* PROXY */
		{
			networkGraph_ = new NetworkGraph;
			networkGraph_->addServer(netAddr);
			networkGraph_->addServer(mainServer);
		}

		/* open logfile */
		time_t curr;
		time(&curr);
		tm local = *(localtime(&curr));
		char hostname[50];
		int retVal = gethostname(hostname, 50);
		if (retVal != 0)
		{
			cerr << "Error getting hostname for logfile." << endl; 
			exit(1);
		}
		string logfileName = "log_";
		logfileName += hostname;
		logfileName += "_";
		stringstream tmp_time;
		tmp_time << local.tm_mday;
		tmp_time << "_";
		tmp_time << (local.tm_mon + 1);
		tmp_time << "_";
		tmp_time << local.tm_hour;
		tmp_time << "_";
		tmp_time << local.tm_min;
		tmp_time << "_";
		tmp_time << local.tm_sec;
		logfileName += tmp_time.str();
		logfile_.open(logfileName.c_str()); 
		if(!logfile_)
		{ 
			cerr << "Cannot open file for loging." << endl; 
			exit(1); 
		}
	}
	Ginnungagap::~Ginnungagap()
	{
		close(inEpollDescriptor_);
		close(outEpollDescriptor_);
		pthread_mutex_destroy(&messageQueueLock_);
		pthread_cond_destroy(&messageQueueHasItems_);
		pthread_mutex_destroy(&rmiRetValXdrBufferLock_);
		pthread_cond_destroy(&rmiRetValXdrBufferNotEmpty_);
		pthread_mutex_destroy(&closedSocketsLock_);
		delete communicationSocketManager_;
		delete migrationService_;
		delete messageHandler_;
		delete proxyFactory_;
		delete objectFactory_;
		delete skeleton_;
		delete networkGraph_;
		logfile_.close();
	}

	void Ginnungagap::writeToLog(const string& logMsg)
	{
		struct timeval currTime;
		int retValue = gettimeofday(&currTime, 0);
		if (retValue != 0) {
			cerr << "Error with gettimeofday" << endl;
			exit(0);
		}

		stringstream tmp_time;
		tmp_time << currTime.tv_sec;
		tmp_time << ":";
		tmp_time << currTime.tv_usec;
		tmp_time << ": ";
		logfile_ << tmp_time.str();
		logfile_ << logMsg << endl;
	}

	void Ginnungagap::startDataAndConnectionReceiverThread()
	{
		if (pthread_create(&dataAndConnectionsReceiverThread_, 0, C_dataAndConnectionsReceiver, 0))
			cerr << "Error creating dataSender thread." << endl;
	}

	void Ginnungagap::startMessageHandlerThread()
	{
		if (pthread_create(&messageHandlerThread_, 0, C_messageHandler, 0))
			cerr << "Error creating messageHandler thread." << endl;
	}

	void Ginnungagap::startMessageHandlerWithCoreSelectionThread()
	{
		if (pthread_create(&messageHandlerWithCoreSelectionThread_, 0, C_messageHandlerWithCoreSelection, 0))
			cerr << "Error creating messageHandlerWithCoreSelection thread." << endl;
	}

	void Ginnungagap::startDataSenderThread()
	{
		if (pthread_create(&dataSenderThread_, 0, C_dataSender, 0))
			cerr << "Error creating dataSender thread." << endl;
	}

	void Ginnungagap::startLatencyMesurerThread()
	{
		if (pthread_create(&latencyMeasurerThread_, 0, C_latencyMeasurer, 0))
			cerr << "Error creating latencyMeasurer thread." << endl;
	}

	NameService* Ginnungagap::nameService()
	{
		return nameService_;
	}

	MigrationService* Ginnungagap::migrationService()
	{
		return migrationService_;
	}

	CommunicationSocketManager* Ginnungagap::communicationSocketManager()
	{
		return communicationSocketManager_;
	}

	NetworkGraph* Ginnungagap::networkGraph()
	{
		return networkGraph_;
	}

	ProxyFactory* Ginnungagap::proxyFactory()
	{
		return proxyFactory_;
	}

	ObjectFactory* Ginnungagap::objectFactory()
	{
		return objectFactory_;
	}

	Skeleton* Ginnungagap::skeleton()
	{
		return skeleton_;
	}

	NetAddr Ginnungagap::netAddr() const
	{
		return netAddr_;
	}

	NetAddr Ginnungagap::mainServer() const
	{
		return mainServer_;
	}

	AppType Ginnungagap::appType() const
	{
		return appType_;
	}

	double Ginnungagap::coreSelectionThreshold() const
	{
		return coreSelectionThreshold_;
	}

	void Ginnungagap::addSocketToDataReceiver(const Socket* socket)
 	{
		struct epoll_event ev;
		ev.data.ptr = static_cast<void*>(const_cast<Socket*>(socket));
		ev.events = EPOLLIN;
		int ret = epoll_ctl(inEpollDescriptor_, EPOLL_CTL_ADD, socket->socketDescriptor(), &ev);
		if (ret != 0)
		{
			if (errno != EEXIST)
				perror("Error adding file descriptor to in epoll list");
		}
	}

	void Ginnungagap::addSocketToDataSender(CommunicationSocket* socket)
	{
		struct epoll_event ev;
		ev.data.ptr = static_cast<void*>(const_cast<CommunicationSocket*>(socket));
		ev.events = EPOLLOUT;
		int ret = epoll_ctl(outEpollDescriptor_, EPOLL_CTL_ADD, socket->socketDescriptor(), &ev);
		if (ret != 0)
		{
			if (errno != EEXIST)
				perror("Error adding file descriptor to out epoll list");
		}
	}

	void Ginnungagap::removeSocketFromDataSender(CommunicationSocket* socket)
	{
		epoll_ctl(outEpollDescriptor_, EPOLL_CTL_DEL, socket->socketDescriptor(), 0);
	}

	void Ginnungagap::removeSocketFromDataReceiver(CommunicationSocket* socket)
	{
		epoll_ctl(inEpollDescriptor_, EPOLL_CTL_DEL, socket->socketDescriptor(), 0);
	}

	XdrReceiveBuffer* Ginnungagap::getRmiRetVal(const Uuid& objectId, const NetAddr& netAddr)
	{
		XdrReceiveBuffer* retXdrBuffer;
		pthread_mutex_lock(&rmiRetValXdrBufferLock_);
		if (rmiRetValXdrBuffer_ == 0)
		{
			pthread_cond_wait(&rmiRetValXdrBufferNotEmpty_, &rmiRetValXdrBufferLock_);
		}
		retXdrBuffer = rmiRetValXdrBuffer_;
		rmiRetValXdrBuffer_ = 0;
		pthread_mutex_unlock(&rmiRetValXdrBufferLock_);
		return retXdrBuffer;
	}

	void Ginnungagap::giveRmiRetVal(XdrReceiveBuffer* xdrBuffer)
	{
		pthread_mutex_lock(&rmiRetValXdrBufferLock_);
		rmiRetValXdrBuffer_ = xdrBuffer;
		pthread_mutex_unlock(&rmiRetValXdrBufferLock_);
		pthread_cond_broadcast(&rmiRetValXdrBufferNotEmpty_);
	}

	void Ginnungagap::removeSocketFromManager(const CommunicationSocket* commSock)
	{
		communicationSocketManager_->removeSocket(commSock);
	}

	void Ginnungagap::sendXdrBuffer(const NetAddr& netAddr, XdrBuffer* xdrBuffer)
	{
		CommunicationSocket* commSock = communicationSocketManager_->getCommunicationSocket(netAddr);
		commSock->sendXdrBuffer(xdrBuffer);
	}

	bool Ginnungagap::isClosed(CommunicationSocket* commSock)
	{
		typedef std::set<CommunicationSocket*>::const_iterator citr_t;
		pthread_mutex_lock(&closedSocketsLock_);
		citr_t citer = closedSockets_.find(commSock);
		pthread_mutex_unlock(&closedSocketsLock_);
		if (citer == closedSockets_.end())
			return false;
		return true;
	}

	void Ginnungagap::addClosedSocket(CommunicationSocket* commSock)
	{
		pthread_mutex_lock(&closedSocketsLock_);
		closedSockets_.insert(commSock);
		pthread_mutex_unlock(&closedSocketsLock_);
	}

	void Ginnungagap::removeClosedSocket(CommunicationSocket* commSock)
	{
		typedef std::set<CommunicationSocket*>::iterator itr_t;
		pthread_mutex_lock(&closedSocketsLock_);
		itr_t iter = closedSockets_.find(commSock);
		if (iter != closedSockets_.end())
			closedSockets_.erase(iter);
		pthread_mutex_unlock(&closedSocketsLock_);
	}

	bool Ginnungagap::haveConnectionTo(const NetAddr& netAddr)
	{
		return communicationSocketManager_->exists(netAddr);
	}

	void Ginnungagap::removeClientOrServer(const NetAddr& netAddr)
	{
		//networkGraph_->removeClientOrServer(netAddr);
		migrationService_->deleteClientFromAllGroups(netAddr);
	}

	typedef list<NetAddr>::const_iterator cSPLItr_t;
	typedef list<NetAddr>::iterator sPLItr_t;

	void Ginnungagap::addServerOrProxy(const NetAddr& netAddr)
	{
		for (cSPLItr_t sPLItr = serverAndProxies_.begin(); sPLItr != serverAndProxies_.end(); ++sPLItr)
		{
			if (*sPLItr == netAddr)
				return;
		}
		serverAndProxies_.push_back(netAddr);
	}

	void Ginnungagap::sendMainServerInit()
	{
		XdrSendBuffer* mainInit = new XdrSendBuffer(INT*2);
		int msgType = MAININIT;
		int appType = appType_;
		*mainInit << msgType << appType;
		sendXdrBuffer(mainServer_, mainInit);
	}

	void Ginnungagap::sendProxyInit()
	{
		XdrSendBuffer* proxyInit = new XdrSendBuffer(INT*2);
		int msgType = PROXYINIT;
		int appType = appType_;
		*proxyInit << msgType << appType;
		sendXdrBuffer(mainServer_, proxyInit);
	}

	void Ginnungagap::sendNewProxyInfo()
	{
		list<NetAddr> proxies = networkGraph_->getServerList();
		int msgType = PROXYINFO;
		/* dont include ourself */
		int numberOfProxies = proxies.size() - 1;
		XdrSendBuffer* proxyInfo = new XdrSendBuffer(INT*2 + NETADDR*numberOfProxies);
		*proxyInfo << msgType << numberOfProxies;

		for (spItr_t spItr = proxies.begin(); spItr != proxies.end(); ++spItr)
		{
			if (!(*spItr == netAddr_))
			{
				*proxyInfo << *spItr;
			}
		}

		/* tell the clients */
		list<NetAddr> clients = networkGraph_->getClientList();
		for (spItr_t spItr = clients.begin(); spItr != clients.end(); ++spItr)
		{
			XdrSendBuffer* copy = new XdrSendBuffer(*proxyInfo);
			sendXdrBuffer(*spItr, copy);
		}

		/* tell the proxies */
		for (spItr_t spItr = proxies.begin(); spItr != proxies.end(); ++spItr)
		{
			/* dont tell ourself */
			if (!(*spItr == netAddr_))
			{
				XdrSendBuffer* copy = new XdrSendBuffer(*proxyInfo);
				sendXdrBuffer(*spItr, copy);
			}
		}
		delete proxyInfo;
	}

	void Ginnungagap::sendProxyInfo(const NetAddr& to)
	{
		list<NetAddr> proxies = networkGraph_->getServerList();
		int msgType = PROXYINFO;
		/* dont include ourself */
		int numberOfProxies = proxies.size() - 1;
		XdrSendBuffer* proxyInfo = new XdrSendBuffer(INT*2 + NETADDR*numberOfProxies);
		*proxyInfo << msgType << numberOfProxies;

		for (spItr_t spItr = proxies.begin(); spItr != proxies.end(); ++spItr)
		{
			if (!(*spItr == netAddr_))
			{
				*proxyInfo << *spItr;
			}
		}
		sendXdrBuffer(to, proxyInfo);
	}

	void Ginnungagap::readProxyInfo(XdrReceiveBuffer* xdrBuffer)
	{
		int numberOfProxies;
		*xdrBuffer >> numberOfProxies;
		list<NetAddr> newProxies;
		for (int i = 0; i != numberOfProxies; ++i)
		{
			NetAddr newServerOrProxy;
			*xdrBuffer >> newServerOrProxy;
			if (appType_ == CLIENT)
			{
				bool exists = false;
				for (spItr_t spItr = serverAndProxies_.begin(); spItr != serverAndProxies_.end(); ++spItr)
				{
					if (*spItr == newServerOrProxy)
					{
						exists = true;
						break;
					}
				}
				if (!exists)
				{
					serverAndProxies_.push_back(newServerOrProxy);
					newProxies.push_back(newServerOrProxy);
				}
			}
			else /* PROXY */
			{
				if (!(newServerOrProxy == netAddr_))
				{
					Ginnungagap::Instance()->networkGraph()->addServer(newServerOrProxy);
				}
			}
		}
		if (appType_ == CLIENT)
		{
			/* Send an init to all new proxies */
			XdrSendBuffer* proxyInit = new XdrSendBuffer(INT);
			int msg = PROXYINIT;
			*proxyInit << msg;
			for (list<NetAddr>::const_iterator proxy = newProxies.begin(); proxy != newProxies.end(); ++proxy)
			{
				XdrSendBuffer* copy = new XdrSendBuffer(*proxyInit);
				sendXdrBuffer(*proxy, copy);
			}
		}
	}

	void Ginnungagap::dataAndConnectionsReceiver()
	{
		listenSocket_ = new ListenSocket(netAddr_.port());
		addSocketToDataReceiver(listenSocket_);
		struct epoll_event* events_ = new struct epoll_event[maxConnections_];
		while (1)
		{
			int ret = epoll_wait(inEpollDescriptor_, events_, maxConnections_, -1); 
			if (ret == -1)
				perror("Error polling epoll descriptor");
			for(int i = 0; i < ret; i++)
			{
				Socket* socket = static_cast<Socket*>(events_[i].data.ptr);
				if (socket->socketDescriptor() == listenSocket_->socketDescriptor())
				{
					listenSocket_->acceptConnection();
				}
				else if (events_[i].events & EPOLLIN)
				{
					CommunicationSocket* commSock = static_cast<CommunicationSocket*>(socket);
					InputBuffer*& inputBuffer = commSock->inputBuffer();
					bool ewouldblock = false;
					if (inputBuffer == 0)
					{
						inputBuffer =  new InputBuffer;
						inputBuffer->xdr = new XdrReceiveBuffer(4);
						inputBuffer->totalRecived = 0;
						inputBuffer->isSizeBuffer = true;
					}

					bool commSockClosed = false;

					if (inputBuffer->isSizeBuffer)
					{
						int recived = -1;
						while (inputBuffer->totalRecived != inputBuffer->xdr->bufferSize())
						{
							recived = commSock->recv(inputBuffer->xdr->xdrBuffer()+inputBuffer->totalRecived, inputBuffer->xdr->bufferSize()-inputBuffer->totalRecived);
							if (recived == -1)
							{
								if (errno == EWOULDBLOCK)
								{
									ewouldblock = true;
									break;
								}
								else
								{
									perror("Socket read error");
									break;
								}
							}
							else if (recived == 0)
							{
								commSockClosed = true;
								removeClientOrServer(commSock->otherEnd());
								removeClientOrServer(commSock->otherEnd());
								break;
							}
							inputBuffer->totalRecived += recived;
						}

						if (!commSockClosed)
						{
							int bufferSize;
							*(inputBuffer->xdr) >> bufferSize;
							delete inputBuffer->xdr;
							inputBuffer->xdr = new XdrReceiveBuffer(bufferSize);
							inputBuffer->totalRecived = 0;
							inputBuffer->isSizeBuffer = false;
						}
					}

					if (!commSockClosed && !ewouldblock && inputBuffer != 0 && inputBuffer->isSizeBuffer == false)
					{
						int recived = -1;
						while (inputBuffer->totalRecived != inputBuffer->xdr->bufferSize())
						{
							recived = commSock->recv(inputBuffer->xdr->xdrBuffer()+inputBuffer->totalRecived, inputBuffer->xdr->bufferSize()-inputBuffer->totalRecived);
							if (recived == -1)
							{
								if (errno == EWOULDBLOCK)
								{
									ewouldblock = true;
									break;
								}
								else
								{
									perror("Socket read error");
									break;
								}
							}
							else if (recived == 0)
							{
								commSockClosed = true;
								removeClientOrServer(commSock->otherEnd());
								break;
							}
							inputBuffer->totalRecived += recived;
						}
					}

					if (commSockClosed)
					{
						addClosedSocket(commSock);
						removeSocketFromDataSender(commSock);
						removeSocketFromDataReceiver(commSock);
						Message* msg = new Message;
						msg->xdr = 0;
						msg->commSock = commSock;
						msg->messageType = DELCOMMSOCK;
						pthread_mutex_lock(&messageQueueLock_);
						messageQueue_.push(msg);
						pthread_mutex_unlock(&messageQueueLock_);
						pthread_cond_broadcast(&messageQueueHasItems_);
						break;
					}

					if (!ewouldblock)
					{
						Message* msg = new Message;
						msg->xdr = inputBuffer->xdr;
						msg->commSock = commSock;
						msg->messageType = NONE;

						pthread_mutex_lock(&messageQueueLock_);
						messageQueue_.push(msg);
						pthread_mutex_unlock(&messageQueueLock_);
						pthread_cond_broadcast(&messageQueueHasItems_);

						delete inputBuffer;
						inputBuffer = 0;
					}
				}
				else if (events_[i].events & EPOLLHUP)
				{
					CommunicationSocket* commSock = static_cast<CommunicationSocket*>(socket);
					delete commSock;
				}
				else if (events_[i].events & EPOLLPRI)
				{
				}
				else if (events_[i].events & EPOLLERR)
				{
					cerr << "EPOLLERR recived..." << endl;
				}
			}
		}
	}

	void Ginnungagap::messageHandler()
	{
		if (appType_ != SERVER)
		{
			sendMainServerInit();
		}

		Message* message = 0;

		while (1)
		{
			pthread_mutex_lock(&messageQueueLock_);
			while (messageQueue_.empty())
			{
				pthread_cond_wait(&messageQueueHasItems_, &messageQueueLock_);
			}
			message = messageQueue_.front();
			messageQueue_.pop();
			pthread_mutex_unlock(&messageQueueLock_);

			messageHandler_->handleMessage(message);
			message = 0;
		}
	}

	void Ginnungagap::messageHandlerWithCoreSelection()
	{
		if (appType_ != SERVER)
		{
			sendMainServerInit();
		}

		Message* message = 0;

		/* seconds for cond_wait to wait */
		int retValue;
		struct timespec ts;
		struct timeval tp;

		/* Set time to wait */
		retValue = gettimeofday(&tp, 0);
		if (retValue != 0) {
			cerr << "Error with gettimeofday" << endl;
			exit(0);
		}
		ts.tv_sec = tp.tv_sec;
		ts.tv_nsec = tp.tv_usec * 1000;
		ts.tv_sec += coreSelInterval_;

		while (1)
		{
			pthread_mutex_lock(&messageQueueLock_);
			while (messageQueue_.empty())
			{
				retValue = pthread_cond_timedwait(&messageQueueHasItems_, &messageQueueLock_, &ts);
				if (retValue == ETIMEDOUT)
				{
					/* Set new time to wait */
					retValue = gettimeofday(&tp, 0);
					if (retValue != 0) {
						cerr << "Error with gettimeofday" << endl;
						exit(0);
					}
					ts.tv_sec = tp.tv_sec;
					ts.tv_nsec = tp.tv_usec * 1000;
					ts.tv_sec += coreSelInterval_;

					/* Perform core selection, and migrate groups if necessary */
					migrationService_->doCoreSelectionOnAllGroups();
				}
			}
			message = messageQueue_.front();
			messageQueue_.pop();
			pthread_mutex_unlock(&messageQueueLock_);

			messageHandler_->handleMessage(message);
			message = 0;
		}
	}

	void Ginnungagap::dataSender()
	{
		struct epoll_event* events_ = new struct epoll_event[maxConnections_];
		while (1)
		{
			int ret = epoll_wait(outEpollDescriptor_, events_, maxConnections_, -1); 
			if (ret == -1)
				perror("Error polling epoll descriptor");
			for(int i = 0; i < ret; i++)
			{
				CommunicationSocket* commSock = static_cast<CommunicationSocket*>(events_[i].data.ptr);
				OutputBuffer*& outputBuffer = commSock->outputBuffer();
				std::queue<OutputBuffer*>*& outputBufferQueue = commSock->outputBufferQueue();
				if (events_[i].events & EPOLLOUT)
				{
					if (outputBuffer == 0)
					{
						commSock->lockOutputBufferQueue();
						if (outputBufferQueue != 0)
						{
							if (outputBufferQueue->empty())
							{
								delete outputBufferQueue;
								outputBufferQueue = 0;
								removeSocketFromDataSender(commSock);
								break;
							}
							else
							{
								outputBuffer = outputBufferQueue->front();
								outputBufferQueue->pop();
							}
						}
						else
						{
							removeSocketFromDataSender(commSock);
							break;
						}
						commSock->unlockOutputBufferQueue();
					}

					if (outputBuffer->totalSent != outputBuffer->xdr->bufferSize())
					{
						int sent = -1;
						sent = commSock->send(outputBuffer->xdr->xdrBuffer()+outputBuffer->totalSent, outputBuffer->xdr->bufferSize()-outputBuffer->totalSent);
						if (sent == -1)
						{
							if (errno == EWOULDBLOCK)
							{
							}
							else
							{
								cerr << "Socket send error" << endl;
							}
						}
						outputBuffer->totalSent += sent;
					}
					if (outputBuffer->totalSent == outputBuffer->xdr->bufferSize())
					{
						delete outputBuffer->xdr;
						delete outputBuffer;
						outputBuffer = 0;

						commSock->lockOutputBufferQueue();
						if (outputBufferQueue == 0 || (outputBufferQueue->empty() || !(commSock->verified_)))
						{
							removeSocketFromDataSender(commSock);
						}
						commSock->unlockOutputBufferQueue();
					}
				}
				else if (events_[i].events & EPOLLHUP)
				{
					addClosedSocket(commSock);
					removeSocketFromDataSender(commSock);
					removeSocketFromDataReceiver(commSock);
					Message* msg = new Message;
					msg->xdr = 0;
					msg->commSock = commSock;
					msg->messageType = DELCOMMSOCK;
					pthread_mutex_lock(&messageQueueLock_);
					messageQueue_.push(msg);
					pthread_mutex_unlock(&messageQueueLock_);
					pthread_cond_broadcast(&messageQueueHasItems_);
				}
				else if (events_[i].events & EPOLLERR)
				{
					cerr << "Out: EPOLLERR recived..." << endl;
				}
			}
		}
	}

	void Ginnungagap::latencyMeasurer()
	{
		typedef std::map<NetAddr, int>::iterator latItr_t;

		/* Wait for proxies and server list to update */
		sleep(10);

		while (1)
		{
			std::map< NetAddr, std::vector<int> > latencies;
			std::map<NetAddr, int> latency;

			stringstream logMsg;
			logMsg << "LatencyMeasurer: calling external ping program";
			Ginnungagap::Instance()->writeToLog(logMsg.str());

			for (spItr_t itr = serverAndProxies_.begin(); itr != serverAndProxies_.end(); ++itr)
			{
				char cping [30];
				string command = "./ping.py ";
				command += itr->addressString();
				FILE* pingResultFilePtr = popen(command.c_str(), "r");
				fgets(cping, 30, pingResultFilePtr);
				pclose(pingResultFilePtr);
				istringstream issping;
				issping.str(cping);
				int ping;
				issping >> ping;
				latency[*itr] = ping;
			}

			int numberOfNodes = serverAndProxies_.size();
			XdrSendBuffer* latInfo = new XdrSendBuffer(INT*2 + INT*numberOfNodes + NETADDR*numberOfNodes);
			int msgType = LATINFO;
			*latInfo << msgType << numberOfNodes;

			for (latItr_t laten = latency.begin(); laten != latency.end(); ++laten)
			{
				*latInfo << laten->first << laten->second;
				logMsg.str("");
				logMsg << "LatencyMeasurer: latency to node: " << laten->first.addressString() << " is: " << laten->second;
				Ginnungagap::Instance()->writeToLog(logMsg.str());
			}

			/* Send the results to all nodes */
			for (spItr_t itr = serverAndProxies_.begin(); itr != serverAndProxies_.end(); ++itr)
			{
				/* copy xdrbuffer */
				XdrSendBuffer* xdr = new XdrSendBuffer(*latInfo);
				Ginnungagap::Instance()->sendXdrBuffer(*itr, xdr);
			}

			delete latInfo;

			/* Take a break =) */
			sleep(300);
		}
	}
}

