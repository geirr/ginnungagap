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

#ifndef GINNUNGAGAP_DIST_PTR_H
#define GINNUNGAGAP_DIST_PTR_H

#include "Ginnungagap.h"
#include "Object.h"
#include "Uuid.h"
#include "NameService.h"
#include "ProxyFactory.h"

#include <iostream>

namespace ginnungagap
{
	template <class T> class dist_ptr
	{
		public:
			dist_ptr() : objectOrProxy_(0), objectId_(Uuid("00000000-0000-0000-0000-000000000000")), isProxy_(false), tmpProxyForMigratingObject_(false) { } 

			dist_ptr(T* object) : objectOrProxy_(object), isProxy_(false), tmpProxyForMigratingObject_(false)
			{
				if (object->hasObjectId())
				{
					objectId_ = object->objectId();
				}
				else
				{
					object->generateObjectId();
					objectId_ = object->objectId();
				}
			}

			dist_ptr(const Uuid& objectId) : objectOrProxy_(0), objectId_(objectId), isProxy_(false), tmpProxyForMigratingObject_(false)
			{
				/* fill in the objectOrProxy pointer
				 * create a proxy object if object is remote */
				updatePointer();
			}

			dist_ptr(const dist_ptr<T>& otherPtr)
			{
				objectId_ = otherPtr.objectId_;
				objectOrProxy_ = otherPtr.objectOrProxy_;
				isProxy_ = otherPtr.isProxy_;
				tmpProxyForMigratingObject_ = false;
				if (isProxy_)
					Ginnungagap::Instance()->nameService()->increaseProxyCount(objectId_);
			}

			~dist_ptr()
			{
				if (isProxy_)
					Ginnungagap::Instance()->nameService()->decreaseProxyCount(objectId_);
			}

			void operator=(const dist_ptr<T>& otherPtr)
			{
				objectId_ = otherPtr.objectId_;
				objectOrProxy_ = otherPtr.objectOrProxy_;
				isProxy_ = otherPtr.isProxy_;
				tmpProxyForMigratingObject_ = false;
				if (isProxy_)
					Ginnungagap::Instance()->nameService()->increaseProxyCount(objectId_);
			}

			void operator=(T* object)
			{
				if (object->hasObjectId())
				{
					objectId_ = object->objectId();
				}
				else
				{
					object->generateObjectId();
					objectId_ = object->objectId();
				}
				objectOrProxy_ = object;
				isProxy_ = false;
				tmpProxyForMigratingObject_ = false;
			}

			T* operator->()
			{
				updatePointer();
				return objectOrProxy_;
			}

			T& operator*()
			{
				updatePointer();
				return *objectOrProxy_;
			}

			bool operator==(const dist_ptr<T>& other_ptr) const
			{
				return (objectId_ == other_ptr.objectId_);
			}

			bool operator<(const dist_ptr<T>& other_ptr) const
			{
				return (objectId_ < other_ptr.objectId_);
			}

			bool isNull() const
			{
				return (objectId_ == Uuid("00000000-0000-0000-0000-000000000000"));
			}

			Uuid objectId() const
			{
				return objectId_;
			}

			void reset()
			{
				if (isProxy_)
					Ginnungagap::Instance()->nameService()->decreaseProxyCount(objectId_);
				objectId_ = Uuid("00000000-0000-0000-0000-000000000000");
				objectOrProxy_ = 0;
				isProxy_ = false;
			}

		private:
			void updatePointer()
			{
				NameService* ns = Ginnungagap::Instance()->nameService();
				/* If it's not in the Name Service, then this should act
				 * like a normal pointer. */
				if (ns->exists(objectId_))
				{
					if (ns->isLocal(objectId_))
					{
						if (tmpProxyForMigratingObject_)
						{
							delete objectOrProxy_;
							tmpProxyForMigratingObject_ = false;
						}
						/* This object might just have become local, so we fetch it again */
						objectOrProxy_ = dynamic_cast<T*>(ns->getLocalObjectOrProxy(objectId_));

						if (isProxy_)
						{
							ns->decreaseProxyCount(objectId_);
							isProxy_ = false;
						}

						/* TODO: Need to check if object is in migrating state */
						/* Update: fixed for events... rmi between migrating objects not supported atm */
						if (objectOrProxy_->objectState() == Object::MIGRATING)
						{
							/* Get a tmp proxy */
							objectOrProxy_ = dynamic_cast<T*>(Ginnungagap::Instance()->proxyFactory()->createProxy<T>(objectId_));
							tmpProxyForMigratingObject_	= true;
						}
					}
					else
					{
						if (tmpProxyForMigratingObject_)
						{
							delete objectOrProxy_;
							objectOrProxy_ = 0;
							tmpProxyForMigratingObject_ = false;
						}
						/* Object is remote, need to check if ns have a proxy */ 
						
						if (objectOrProxy_ == 0 || isProxy_ == false)
						{
							/* we dont have one, need to get a pointer to the proxy, if the ns have one */
							if (ns->getLocalObjectOrProxy(objectId_) != 0)
							{
								objectOrProxy_ = dynamic_cast<T*>(ns->getLocalObjectOrProxy(objectId_));
								ns->increaseProxyCount(objectId_);
								isProxy_ = true;
							}
							/* It does not, create one, and return a pointer to it */
							else
							{
								objectOrProxy_ = dynamic_cast<T*>(Ginnungagap::Instance()->proxyFactory()->createProxy<T>(objectId_));
								ns->setLocalObjectOrProxy(objectOrProxy_);
								ns->increaseProxyCount(objectId_);
								isProxy_ = true;
							}
						}
						else
						{
							/* Need to check it ns have the same proxy... */
							if (ns->getLocalObjectOrProxy(objectId_) != 0)
							{
								T* testProxy = dynamic_cast<T*>(ns->getLocalObjectOrProxy(objectId_));
								if (testProxy != objectOrProxy_)
								{
									/* We need to change to the new proxy */
									objectOrProxy_ = dynamic_cast<T*>(ns->getLocalObjectOrProxy(objectId_));
									ns->increaseProxyCount(objectId_);
									isProxy_ = true;
								}
							}
							else
							{
								/* ns do not have a proxy, even we think we do, create a new */
								objectOrProxy_ = dynamic_cast<T*>(Ginnungagap::Instance()->proxyFactory()->createProxy<T>(objectId_));
								ns->setLocalObjectOrProxy(objectOrProxy_);
								ns->increaseProxyCount(objectId_);
								isProxy_ = true;
							}
						}
					}
				}
			}

			T* objectOrProxy_;
			Uuid objectId_;
			bool isProxy_;
			bool tmpProxyForMigratingObject_;

	};
}

#endif

