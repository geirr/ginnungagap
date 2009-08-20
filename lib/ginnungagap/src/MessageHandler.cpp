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

#include "MessageHandler.h"
#include "MessageType.h"
#include "NameService.h"
#include "Skeleton.h"
#include "ObjectFactory.h"
#include "MigrationService.h"
#include "Ginnungagap.h"
#include "Version.h"
#include "XdrReceiveBuffer.h"
#include "XdrSendBuffer.h"
#include "CommunicationSocketManager.h"
#include "NetworkGraph.h"
#include "NetAddr.h"
#include "Uuid.h"
#include "Object.h"

#include <iostream>
#include <sstream>

using std::cerr; using std::endl; using std::list; using std::stringstream;

namespace ggg
{
	typedef list<Uuid>::iterator idListItr_t;
	enum InitAckTypes
	{
		OK,
		DOUBLECONN,
		VERSIONERR
	};


	void MessageHandler::handleMessage(Message* message)
	{
		currentMessage_ = message;

		if (currentMessage_->messageType == DELCOMMSOCK || !(Ginnungagap::Instance()->isClosed(message->commSock)))
		{
			XdrReceiveBuffer* xdrReciveBuffer = currentMessage_->xdr;
			int msgType;
			if (currentMessage_->messageType == DELCOMMSOCK)
			{
				msgType = DELCOMMSOCK;
			}
			else
			{
				*xdrReciveBuffer >> msgType;
			}
			switch (msgType)
			{
				case INIT:
					if (verbose_)
						cerr << "MessageHandler: got INIT" << endl;
					this->handleInit(xdrReciveBuffer);
					break;
				case INITACK:
					if (verbose_)
						cerr << "MessageHandler: got INITACK" << endl;
					this->handleInitAck(xdrReciveBuffer);
					break;
				case MAININIT:
					if (verbose_)
						cerr << "MessageHandler: got MAININIT" << endl;
					this->handleMainInit(xdrReciveBuffer);
					break;
				case PROXYINFO:
					if (verbose_)
						cerr << "MessageHandler: got PROXYINFO" << endl;
					this->handleProxyInfo(xdrReciveBuffer);
					break;
				case PROXYINIT:
					if (verbose_)
						cerr << "MessageHandler: got PROXYINIT" << endl;
					this->handleProxyInit(xdrReciveBuffer);
					break;
				case NEED:
					if (verbose_)
						cerr << "MessageHandler: got NEED" << endl;
					this->handleNeedObject(xdrReciveBuffer);
					break;
				case DONTNEED:
					if (verbose_)
						cerr << "MessageHandler: got DONTNEED" << endl;
					this->handleDontNeedObject(xdrReciveBuffer);
					break;
				case RMI:
					if (verbose_)
						cerr << "MessageHandler: got RMI" << endl;
					this->handleRmi(xdrReciveBuffer);
					break;
				case EVENT:
					if (verbose_)
						cerr << "MessageHandler: got EVENT" << endl;
					this->handleEvent(xdrReciveBuffer);
					break;
				case FORWARDRMI:
					if (verbose_)
						cerr << "MessageHandler: got FORWARDRMI" << endl;
					this->handleForwardedRmi(xdrReciveBuffer);
					break;
				case RMIACKRETVAL:
					if (verbose_)
						cerr << "MessageHandler: got RMIACKRETVAL" << endl;
					this->handleRmiAckAndReturnValue(xdrReciveBuffer);
					break;
				case MIGOBJ:
					if (verbose_)
						cerr << "MessageHandler: got MIGOBJ" << endl;
					this->handleIncomingMigration(xdrReciveBuffer);
					break;
				case MIGACK:
					if (verbose_)
						cerr << "MessageHandler: got MIGACK" << endl;
					this->handleMigrationAck(xdrReciveBuffer);
					break;
				case NSUPDATE:
					if (verbose_)
						cerr << "MessageHandler: got NSUPDATE" << endl;
					this->handleNameServiceUpdate(xdrReciveBuffer);
					break;
				case LATINFO:
					if (verbose_)
						cerr << "MessageHandler: got LATINFO" << endl;
					this->handleLatencyInformation(xdrReciveBuffer);
					break;
				case MIGGRP:
					if (verbose_)
						cerr << "MessageHandler: got MIGGRP" << endl;
					this->handleMigrationGroup(xdrReciveBuffer);
					break;
				case MIGGRPACK:
					if (verbose_)
						cerr << "MessageHandler: got MIGGRPACK" << endl;
					this->handleMigrationGroupAck(xdrReciveBuffer);
					break;
				case INITCHOBJGRP:
					if (verbose_)
						cerr << "MessageHandler: got INITCHOBJGRP" << endl;
					this->handleInitChangeObjectGroup(xdrReciveBuffer);
					break;
				case ICOGACKISHERE:
					if (verbose_)
						cerr << "MessageHandler: got ICOGACKISHERE" << endl;
					this->handleICOGIsHere(xdrReciveBuffer);
					break;
				case ICOGACKISNOTHERE:
					if (verbose_)
						cerr << "MessageHandler: got ICOGACKISNOTHERE" << endl;
					this->handleICOGNotHere(xdrReciveBuffer);
					break;
				case OBJGRPINFO:
					if (verbose_)
						cerr << "MessageHandler: got OBJGRPINFO" << endl;
					this->handleGroupInfoForObject(xdrReciveBuffer);
					break;
				case DELCOMMSOCK:
					if (verbose_)
						cerr << "MessageHandler: got DELCOMMSOCK" << endl;
					delete currentMessage_->commSock;
					break;
				default:
					cerr << "Error reading message from XdrBuffer" << endl;
					break;
			}
		}
		delete message;
	}

	void MessageHandler::handleInit(XdrReceiveBuffer* xdrBuffer)
	{
		int major;
		int minor;
		NetAddr netAddr;
		*xdrBuffer >> major >> minor >> netAddr;
		delete xdrBuffer;

		XdrSendBuffer* xdrSendBuffer = new XdrSendBuffer(INT*2+NETADDR);
		int msgType = INITACK;
		int ackType;
		*xdrSendBuffer << msgType << Ginnungagap::Instance()->netAddr();

		if (major != MAJORVER && minor != MINORVER)
		{
			cerr << "Incompatible application versions" << endl;
			ackType = VERSIONERR;
		}
		else
		{
			currentMessage_->commSock->setOtherEnd(netAddr);
			currentMessage_->commSock->verify();
			Ginnungagap::Instance()->communicationSocketManager()->addSocket(currentMessage_->commSock);
			ackType = OK;
		}

		*xdrSendBuffer << ackType;

		currentMessage_->commSock->outputBuffer_ = new OutputBuffer;
		currentMessage_->commSock->outputBuffer_->xdr = xdrSendBuffer;
		currentMessage_->commSock->outputBuffer_->totalSent = 0;
		Ginnungagap::Instance()->addSocketToDataSender(currentMessage_->commSock);
	}

	void MessageHandler::handleInitAck(XdrReceiveBuffer* xdrBuffer)
	{
		NetAddr netAddr;
		int ackType;
		*xdrBuffer >> netAddr >> ackType;
		delete xdrBuffer;

		if (ackType == OK)
		{
			currentMessage_->commSock->verify();
		}
		else if (ackType == DOUBLECONN)
		{
			cerr << "Got doubleconn" << endl;
			/* TODO */
			exit(0);
		}
		else if (ackType == VERSIONERR)
		{
			cerr << "Got version error" << endl;
			exit(0);
		}
		else
		{
			cerr << "Unknown error" << endl;
			exit(0);
		}
	}

	void MessageHandler::handleMainInit(XdrReceiveBuffer* xdrBuffer)
	{
		int appType;
		*xdrBuffer >> appType;

		stringstream logMsg;
		if (appType == PROXY)
		{
			Ginnungagap::Instance()->networkGraph()->addServer(currentMessage_->commSock->otherEnd());
			Ginnungagap::Instance()->sendNewProxyInfo();
			logMsg << "New proxy connected: " << currentMessage_->commSock->otherEnd().addressString();
		}
		else if (appType == CLIENT)
		{
			Ginnungagap::Instance()->networkGraph()->addClient(currentMessage_->commSock->otherEnd());
			Ginnungagap::Instance()->sendProxyInfo(currentMessage_->commSock->otherEnd());
			logMsg << "New client connected: " << currentMessage_->commSock->otherEnd().addressString();
		}
		Ginnungagap::Instance()->writeToLog(logMsg.str());
	}

	void MessageHandler::handleProxyInfo(XdrReceiveBuffer* xdrBuffer)
	{
		Ginnungagap::Instance()->readProxyInfo(xdrBuffer);
	}

	void MessageHandler::handleProxyInit(XdrReceiveBuffer* xdrBuffer)
	{
		Ginnungagap::Instance()->networkGraph()->addClient(currentMessage_->commSock->otherEnd());
		delete xdrBuffer;
	}

	void MessageHandler::handleNeedObject(XdrReceiveBuffer* xdrBuffer)
	{
		NetAddr client;
		Uuid objectId;
		*xdrBuffer >> client >> objectId;
		delete xdrBuffer;
		if (!(Ginnungagap::Instance()->nameService()->isLocal(objectId)))
		{
			XdrSendBuffer* newXdr = new XdrSendBuffer(INT + NETADDR + OBJID);
			int msg = NEED;
			*newXdr << msg << client << objectId;
			Ginnungagap::Instance()->sendXdrBuffer(Ginnungagap::Instance()->nameService()->netAddr(objectId), newXdr);
			return;
		}
		MigrationGroup* mg = Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectId);
		if (mg != 0)
			mg->addClientNeedObject(client, objectId);
	}

	void MessageHandler::handleDontNeedObject(XdrReceiveBuffer* xdrBuffer)
	{
		NetAddr client;
		Uuid objectId;
		*xdrBuffer >> client >> objectId;
		delete xdrBuffer;
		if (!(Ginnungagap::Instance()->nameService()->isLocal(objectId)))
		{
			XdrSendBuffer* newXdr = new XdrSendBuffer(INT + NETADDR + OBJID);
			int msg = DONTNEED;
			*newXdr << msg << client << objectId;
			Ginnungagap::Instance()->sendXdrBuffer(Ginnungagap::Instance()->nameService()->netAddr(objectId), newXdr);
			return;
		}
		MigrationGroup* mg = Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectId);
		if (mg != 0)
			mg->removeClientNeedObject(objectId, client);
	}

	void MessageHandler::handleRmi(XdrReceiveBuffer* xdrBuffer)
	{
		int objectName;
		Uuid objectId;		

		*xdrBuffer >> objectName >> objectId;

		NameService* ns = Ginnungagap::Instance()->nameService();

		//stringstream logMsg;
		//logMsg << "MessageHandler: Got RMI from node " << currentMessage_->commSock->otherEnd().addressString() << " for object " << objectId.getHexString();
		//Ginnungagap::Instance()->writeToLog(logMsg.str());

		if (!(ns->isLocal(objectId)))
		{
			//logMsg.str("");
			//logMsg << "MessageHandler: Object is not local, forwarding rmi to new host: ";
			//logMsg << ns->netAddr(objectId).addressString();
			//Ginnungagap::Instance()->writeToLog(logMsg.str());

			int msgType = FORWARDRMI;
			XdrSendBuffer* xdrSendBuffer = new XdrSendBuffer(msgType, currentMessage_->commSock->otherEnd(), xdrBuffer);
			delete xdrBuffer;
			Ginnungagap::Instance()->sendXdrBuffer(ns->netAddr(objectId), xdrSendBuffer);
		}
		else
		{
			Object* object = ns->getLocalObjectOrProxy(objectId);
			if (object->objectState() == Object::NORMAL)
			{
				//logMsg.str("");
				//logMsg << "MessageHandler: Object is local, and in NORMAL state, performing RMI";
				//Ginnungagap::Instance()->writeToLog(logMsg.str());
				XdrSendBuffer* xdrRet = Ginnungagap::Instance()->skeleton()->handleRmi(objectName, objectId, object, xdrBuffer);
				if (xdrRet != 0)
				{
					currentMessage_->commSock->sendXdrBuffer(xdrRet);
				}
			}
			else
			{
				//logMsg.str("MessageHandler: Object is local, and in MIGRATING state, making FORWARDRMI message");
				//Ginnungagap::Instance()->writeToLog(logMsg.str());
				int msgType = FORWARDRMI;
				XdrSendBuffer* xdrSendBuffer = new XdrSendBuffer(msgType, currentMessage_->commSock->otherEnd(), xdrBuffer);
				delete xdrBuffer;
				Ginnungagap::Instance()->migrationService()->putXdrBufferInQueue(objectId, xdrSendBuffer);
			}
		}

	}

	void MessageHandler::handleEvent(XdrReceiveBuffer* xdrBuffer)
	{
		int objectName;
		Uuid objectId;		
		*xdrBuffer >> objectName >> objectId;
		NameService* ns = Ginnungagap::Instance()->nameService();

		//stringstream logMsg;
		//logMsg << "MessageHandler: Got EVENT from node " << currentMessage_->commSock->otherEnd().addressString() << " for object " << objectId.getHexString();
		//Ginnungagap::Instance()->writeToLog(logMsg.str());

		if (!(ns->isLocal(objectId)))
		{
			/* send a ns update to the sender, and forward the event to the new node */
			XdrSendBuffer* nsUpdate = ns->getNsRecordUpdate(objectId);
			XdrSendBuffer* sendXdr = new XdrSendBuffer(xdrBuffer);
			Ginnungagap::Instance()->sendXdrBuffer(ns->netAddr(objectId), sendXdr);
			currentMessage_->commSock->sendXdrBuffer(nsUpdate);

			//logMsg.str("");
			//logMsg << "MessageHandler: Object is not local, forwarding event, and sending new addr: ";
			//logMsg << ns->netAddr(objectId).addressString();
			//Ginnungagap::Instance()->writeToLog(logMsg.str());

			return;
		}
		else
		{
			Object* object = ns->getLocalObjectOrProxy(objectId);
			if (object->objectState() == Object::NORMAL)
			{
				//logMsg.str("");
				//logMsg << "MessageHandler: Object is local, and in NORMAL state, performing EVENT";
				//Ginnungagap::Instance()->writeToLog(logMsg.str());
				Ginnungagap::Instance()->skeleton()->handleEvent(objectName, objectId, object, xdrBuffer);
			}
			else
			{
				//logMsg.str("");
				//logMsg << "MessageHandler: Object is local, and in MIGRATING state, buffering EVENT and will forward";
				//Ginnungagap::Instance()->writeToLog(logMsg.str());
				XdrSendBuffer* xdrSendBuffer = new XdrSendBuffer(xdrBuffer);
				delete xdrBuffer;
				Ginnungagap::Instance()->migrationService()->putXdrBufferInQueue(objectId, xdrSendBuffer);
			}
		}
	}

	void MessageHandler::handleForwardedRmi(XdrReceiveBuffer* xdrBuffer)
	{
		NetAddr netAddr;
		*xdrBuffer >> netAddr;

		int objectName;
		*xdrBuffer >> objectName;

		Uuid objectId;		
		*xdrBuffer >> objectId;

		NameService* ns = Ginnungagap::Instance()->nameService();

		//stringstream logMsg;
		//logMsg << "MessageHandler: Got FORWARDRMI from node " << currentMessage_->commSock->otherEnd().addressString();
		//logMsg << " for object " << objectId.getHexString();
		//logMsg << " originally from node: " << netAddr.addressString();
		//Ginnungagap::Instance()->writeToLog(logMsg.str());

		if (!(ns->isLocal(objectId)))
		{
			/* This usually shouldn't happend, but the object might have
			 * migrated again, before we got this. Check name service, and
			 * send this wherever we think the object resides */
			XdrSendBuffer* sendXdr = new XdrSendBuffer(xdrBuffer);
			Ginnungagap::Instance()->sendXdrBuffer(ns->netAddr(objectId), sendXdr);
			XdrSendBuffer* nsUpdate = ns->getNsRecordUpdate(objectId);
			currentMessage_->commSock->sendXdrBuffer(nsUpdate);

			//logMsg.str("");
			//logMsg << "MessageHandler: Object is not local, forwarding to: ";
			//logMsg << ns->netAddr(objectId).addressString();
			//Ginnungagap::Instance()->writeToLog(logMsg.str());

		}
		else
		{
			Object* object = ns->getLocalObjectOrProxy(objectId);
			if (object->objectState() == Object::NORMAL)
			{
				//logMsg.str("");
				//logMsg << "MessageHandler: Object is local, performing RMI";
				//Ginnungagap::Instance()->writeToLog(logMsg.str());

				/* Get RMIRETVAL - true indicates that our NetAddr should be piggybacked, so the client can update its name service */
				XdrSendBuffer* xdrRet = Ginnungagap::Instance()->skeleton()->handleRmi(objectName, objectId, object, xdrBuffer, true);
				if (xdrRet != 0)
				{
					CommunicationSocket* commSock = Ginnungagap::Instance()->communicationSocketManager()->getCommunicationSocket(netAddr);
					commSock->sendXdrBuffer(xdrRet);
				}
			}
			else
			{
				//logMsg.str("");
				//logMsg << "MessageHandler: (xxx) Object is local, but in MIGRATING state, buffering FORWARDRMI msg";
				//Ginnungagap::Instance()->writeToLog(logMsg.str());
				XdrSendBuffer* sendXdr = new XdrSendBuffer(xdrBuffer);
				Ginnungagap::Instance()->migrationService()->putXdrBufferInQueue(objectId, sendXdr);
				delete xdrBuffer;
			}
		}
	}

	void MessageHandler::handleRmiAckAndReturnValue(XdrReceiveBuffer* xdrBuffer)
	{
		Ginnungagap::Instance()->giveRmiRetVal(xdrBuffer);
	}

	void MessageHandler::handleIncomingMigration(XdrReceiveBuffer* xdrBuffer)
	{
		int objectName;
		*xdrBuffer >> objectName;
		Object* object = Ginnungagap::Instance()->objectFactory()->createObject(objectName, xdrBuffer);

		/* The factory doesn't delte the buffer anymore (because of mig grps)  */
		delete xdrBuffer;

		Ginnungagap::Instance()->nameService()->bind(*object);

		if (Ginnungagap::Instance()->appType() != SERVER && !(currentMessage_->commSock->otherEnd() == Ginnungagap::Instance()->mainServer()))
		{
			XdrSendBuffer* nsUpdate = Ginnungagap::Instance()->nameService()->getNsRecordUpdate(object->objectId());
			Ginnungagap::Instance()->sendXdrBuffer(Ginnungagap::Instance()->mainServer(), nsUpdate);
		}

		int msg = MIGACK;
		XdrSendBuffer* xdrSendBuffer = new XdrSendBuffer(OBJID + INT);
		*xdrSendBuffer << msg << object->objectId();
		currentMessage_->commSock->sendXdrBuffer(xdrSendBuffer);

		MigrationGroup* mg = Ginnungagap::Instance()->migrationService()->getMigrationGroup(object->objectId());
		if (mg != 0)
			mg->tryToActivateIncoming(object->objectId());

		stringstream logMsg;
		logMsg << "MessageHandler: Got incomming object " << object->objectId().getHexString();
		Ginnungagap::Instance()->writeToLog(logMsg.str());
	}

	void MessageHandler::handleMigrationAck(XdrReceiveBuffer* xdrBuffer)
	{
		Uuid objectId;
		*xdrBuffer >> objectId;
		delete xdrBuffer;

		NetAddr ackEnd = currentMessage_->commSock->otherEnd();

		stringstream logMsg;
		logMsg << "MessageHandler: Got MIGACK from node " << currentMessage_->commSock->otherEnd().addressString() << " for object " << objectId.getHexString();
		Ginnungagap::Instance()->writeToLog(logMsg.str());

		Ginnungagap::Instance()->migrationService()->updateNameServiceAndForwardRmiMessages(ackEnd, objectId);
	}

	void MessageHandler::handleNameServiceUpdate(XdrReceiveBuffer* xdrBuffer)
	{
		stringstream logMsg;
		logMsg << "MessageHandler: Got nsupdate";
		Ginnungagap::Instance()->writeToLog(logMsg.str());
		Ginnungagap::Instance()->nameService()->updateNsRecord(xdrBuffer);
	}

	void MessageHandler::handleLatencyInformation(XdrReceiveBuffer* xdrBuffer)
	{
		int numberOfNodes;
		*xdrBuffer >> numberOfNodes;
		list<NetAddr> servers;
		bool anyZeros = false;
		for (int i = 0; i != numberOfNodes; ++i)
		{
			NetAddr netAddr;
			int latency;
			*xdrBuffer >> netAddr >> latency;
			servers.push_front(netAddr);
			if (latency == 0)
				anyZeros = true;
			Ginnungagap::Instance()->networkGraph()->updateLink(netAddr, currentMessage_->commSock->otherEnd(), latency);
		}
		if (anyZeros)
		{
			/* Reset this nodes weights, to dont skew the result of core selection */
			for (list<NetAddr>::const_iterator netAddrItr = servers.begin(); netAddrItr != servers.end(); ++netAddrItr)
			{
				Ginnungagap::Instance()->networkGraph()->updateLink(*netAddrItr, currentMessage_->commSock->otherEnd(), 0);
			}
		}
		delete xdrBuffer;
	}

	void MessageHandler::handleMigrationGroup(XdrReceiveBuffer* xdrBuffer)
	{
		Uuid groupId;
		*xdrBuffer >> groupId;

		stringstream logMsg;
		logMsg << "MessageHandler: Got MIGGRP: " << groupId.getHexString();
		Ginnungagap::Instance()->writeToLog(logMsg.str());

		MigrationGroup* mg = new MigrationGroup(groupId);
		Ginnungagap::Instance()->migrationService()->addMigrationGroup(mg);


		int numberOfObjects;
		int tmpNumber;
		int tmpNumber2;
		Uuid tmpObjId;
		NetAddr tmpNetAddr;

		*xdrBuffer >> numberOfObjects;
		for (int i = 0; i != numberOfObjects; ++i)
		{
			*xdrBuffer >> tmpObjId;
			mg->addObject(tmpObjId);
		}

		*xdrBuffer >> tmpNumber;
		for (int i = 0; i != tmpNumber; ++i)
		{
			*xdrBuffer >> tmpObjId;
			*xdrBuffer >> tmpNumber2;
			for (int j = 0; j != tmpNumber2; ++j)
			{
				*xdrBuffer >> tmpNetAddr;
				mg->addClientNeedObject(tmpNetAddr, tmpObjId);
			}
		}

		*xdrBuffer >> tmpNumber;
		for (int i = 0; i != tmpNumber; ++i)
		{
			*xdrBuffer >> tmpObjId;
			*xdrBuffer >> tmpNumber2;
			for (int j = 0; j != tmpNumber2; ++j)
			{
				*xdrBuffer >> tmpNetAddr;
				mg->addObjectNeedClient(tmpNetAddr, tmpObjId);
			}
		}

		/* Get objects */
		int objectName;
		for (int i = 0; i != numberOfObjects; ++i)
		{
			*xdrBuffer >> objectName;
			Object* object = Ginnungagap::Instance()->objectFactory()->createObject(objectName, xdrBuffer);
			Ginnungagap::Instance()->nameService()->bind(*object);
			if (Ginnungagap::Instance()->appType() != SERVER && !(currentMessage_->commSock->otherEnd() == Ginnungagap::Instance()->mainServer()))
			{
				XdrSendBuffer* nsUpdate = Ginnungagap::Instance()->nameService()->getNsRecordUpdate(object->objectId());
				Ginnungagap::Instance()->sendXdrBuffer(Ginnungagap::Instance()->mainServer(), nsUpdate);
			}
		}

		delete xdrBuffer;

		logMsg.str("");
		logMsg << "MessageHandler: MIGGRP done reading info: " << groupId.getHexString();
		Ginnungagap::Instance()->writeToLog(logMsg.str());

		/* Send miggrp ack */
		XdrSendBuffer* migGrpAck = new XdrSendBuffer(INT+UUID);
		int msgType = MIGGRPACK;
		*migGrpAck << msgType << groupId;
		currentMessage_->commSock->sendXdrBuffer(migGrpAck);

		mg->activateGroup();
	}

	void MessageHandler::handleMigrationGroupAck(XdrReceiveBuffer* xdrBuffer)
	{
		Uuid groupId;
		*xdrBuffer >> groupId;
		/* Update ns and forward buffers for all objects in grp */
		Ginnungagap::Instance()->migrationService()->getMigrationGroupById(groupId)->forwardAllMsgsAndUpdateAllNs(currentMessage_->commSock->otherEnd());

		/* Don't need the group anymore, can delete it */
		Ginnungagap::Instance()->migrationService()->deleteMigrationGroup(groupId);
		delete xdrBuffer;

		stringstream logMsg;
		logMsg << "MessageHandler: Got MIGGRPACK from node " << currentMessage_->commSock->otherEnd().addressString() << " for group " << groupId.getHexString();
		Ginnungagap::Instance()->writeToLog(logMsg.str());

	}

	void MessageHandler::handleInitChangeObjectGroup(XdrReceiveBuffer* xdrBuffer)
	{
		Uuid objectToMove;
		Uuid objectInNewGroup;
		*xdrBuffer >> objectToMove >> objectInNewGroup;

		/* if objectInNewGroup is local, deactivate group, and get the new object here */
		/* if objectInNewGroup isn't local, send new location back */
		if (Ginnungagap::Instance()->nameService()->isLocal(objectInNewGroup))
		{
			MigrationGroup* mg = Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectInNewGroup);
			if (mg != 0)
				mg->waitFor(objectToMove);
			int msg = ICOGACKISHERE;
			XdrSendBuffer* xdr = new XdrSendBuffer(INT + OBJID*2);
			*xdr << msg << objectToMove << objectInNewGroup;
			currentMessage_->commSock->sendXdrBuffer(xdr);
		}
		else
		{
			NetAddr newAddr = Ginnungagap::Instance()->nameService()->netAddr(objectInNewGroup);
			int msg = ICOGACKISNOTHERE;
			XdrSendBuffer* xdr = new XdrSendBuffer(INT + OBJID*2 + NETADDR);
			*xdr << msg << objectToMove << objectInNewGroup << newAddr;
			currentMessage_->commSock->sendXdrBuffer(xdr);
		}

		delete xdrBuffer;
	}

	void MessageHandler::handleICOGNotHere(XdrReceiveBuffer* xdrBuffer)
	{
		Uuid objectToMove;
		Uuid objectInNewGroup;
		NetAddr newAddr;

		*xdrBuffer >> objectToMove >> objectInNewGroup >> newAddr;

		Ginnungagap::Instance()->nameService()->updateNetAddr(objectInNewGroup, newAddr);
		Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectToMove)->moveObjectToGroupWithMainObject(objectToMove, objectInNewGroup);
		MigrationGroup* mg = Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectToMove);
		if (mg != 0)
			mg->moveObjectToGroupWithMainObject(objectToMove, objectInNewGroup);

		delete xdrBuffer;
	}

	void MessageHandler::handleICOGIsHere(XdrReceiveBuffer* xdrBuffer)
	{
		Uuid objectToMove;
		Uuid objectInNewGroup;

		*xdrBuffer >> objectToMove >> objectInNewGroup;

		MigrationGroup* mg = Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectToMove);
		if (mg != 0)
			mg->moveObjectToGroupWithMainObjectAfterConfimedDestination(objectToMove, objectInNewGroup);

		delete xdrBuffer;
	}

	void MessageHandler::handleGroupInfoForObject(XdrReceiveBuffer* xdrBuffer)
	{
		Uuid objectToMove;
		Uuid objectInNewGroup;

		*xdrBuffer >> objectToMove >> objectInNewGroup;

		MigrationGroup* mg = Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectInNewGroup);
		if (mg != 0)
			mg->addObject(objectToMove);

		int tmpSize;
		NetAddr tmpNetAddr;

		*xdrBuffer >> tmpSize;
		for (int i = 0; i != tmpSize; ++i)
		{
			*xdrBuffer >> tmpNetAddr;
			mg = Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectToMove);
			if (mg != 0)
				mg->addClientNeedObject(tmpNetAddr, objectToMove);
		}

		*xdrBuffer >> tmpSize;
		for (int i = 0; i != tmpSize; ++i)
		{
			*xdrBuffer >> tmpNetAddr;
			mg = Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectToMove);
			if (mg != 0)
				mg->addObjectNeedClient(tmpNetAddr, objectToMove);
		}

		Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectToMove)->tryToActivateIncoming(objectToMove);
		mg = Ginnungagap::Instance()->migrationService()->getMigrationGroup(objectToMove);
		if (mg != 0)
			mg->tryToActivateIncoming(objectToMove);

		delete xdrBuffer;
	}
}

