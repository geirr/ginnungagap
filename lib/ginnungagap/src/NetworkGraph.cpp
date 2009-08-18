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

#include "NetworkGraph.h"
#include "Ginnungagap.h"

#include <map>
#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

using std::multimap; using std::pair; using std::list; using std::make_pair; using std::stringstream;

namespace ginnungagap
{
	void NetworkGraph::addServer(const NetAddr& netAddr)
	{
		list<NetAddr> servers = getServerList();
		list<NetAddr>::const_iterator serverItr;
		for (serverItr = servers.begin(); serverItr != servers.end(); ++serverItr)
		{
			if (netAddr == *serverItr)
			{
				return;
			}
		}
		Node newServer = boost::add_vertex(network_);
		network_[newServer].netAddr = netAddr;
		serverAndProxies_.push_back(newServer);
		list<Node>::const_iterator client_it;
		for(client_it = clients_.begin(); client_it != clients_.end(); ++client_it)
		{
			Link newLink = boost::add_edge(*client_it, newServer, network_);
			network_[newLink.first].weight = 0;
		}
	}

	void NetworkGraph::addClient(const NetAddr& netAddr)
	{
		Node newClient = boost::add_vertex(network_);
		network_[newClient].netAddr = netAddr;
		clients_.push_back(newClient);
		list<Node>::const_iterator server_it;
		for(server_it = serverAndProxies_.begin(); server_it != serverAndProxies_.end(); ++server_it)
		{
			Link newLink = boost::add_edge(*server_it, newClient, network_);
			network_[newLink.first].weight = 0;
		}
	}

	void NetworkGraph::removeClientOrServer(const NetAddr& netAddr)
	{
		Node node;
		bool ret = getNode(netAddr, node);
		if (ret)
		{
			remove_vertex(node, network_);
		}
	}

	list<NetAddr> NetworkGraph::getClientList() const
	{
		list<NetAddr> clientList;
		list<Node>::const_iterator client_it;
		for(client_it = clients_.begin(); client_it != clients_.end(); ++client_it)
		{
			clientList.push_back(network_[*client_it].netAddr);
		}
		return clientList;
	}

	list<NetAddr> NetworkGraph::getServerList() const
	{
		list<NetAddr> serverList;
		list<Node>::const_iterator server_it;
		for(server_it = serverAndProxies_.begin(); server_it != serverAndProxies_.end(); ++server_it)
		{
			serverList.push_back(network_[*server_it].netAddr);
		}
		return serverList;
	}

	bool NetworkGraph::getNode(const NetAddr& netAddr, Node& node) const
	{
		typedef boost::graph_traits<Graph>::vertex_iterator node_itr_t;
		pair<node_itr_t, node_itr_t> nodes = vertices(network_);
		node_itr_t node_itr = nodes.first;
		node_itr_t nodes_end = nodes.second;
		while (node_itr != nodes_end)
		{
			if(network_[*node_itr].netAddr == netAddr)
			{
				node = *node_itr;
				return true;
			}
			++node_itr;
		}
		return false;
	}

	bool NetworkGraph::updateLink(const NetAddr& netAddr1, const NetAddr& netAddr2, const double& weight)
	{
		Node node1, node2;
		bool ret;
		ret = getNode(netAddr1, node1);
		if (!ret)
			return false;
		ret = getNode(netAddr2, node2);
		if (!ret)
			return false;

		Link link = edge(node1, node2, network_);
		if (!(link.second))
		{
			return false;
		}
		network_[link.first].weight = weight;
		return true;
	}

	pair<NetAddr, double> NetworkGraph::findBestCore(const list<Node>& clients) const
	{
		double thisWeight = 0;
		list<Node>::const_iterator server_it;
		multimap<double, Node> sumWeightMap;


		for(server_it = serverAndProxies_.begin(); server_it != serverAndProxies_.end(); ++server_it)
		{
			double sumWeight = 0;
			list<Node>::const_iterator client_it;
			for(client_it = clients.begin(); client_it != clients.end(); ++client_it)
			{
				if(*server_it == *client_it)
				{
					continue;
				}
				Link link = edge(*server_it, *client_it, network_);
				sumWeight += network_[link.first].weight;			
			}

			if (Ginnungagap::Instance()->netAddr() == network_[*server_it].netAddr)
			{
				thisWeight = sumWeight;			
			}

			sumWeightMap.insert(pair<double, Node>(sumWeight, *server_it));
		}
		multimap<double, Node>::iterator logMmIt;
		for (logMmIt = sumWeightMap.begin(); logMmIt != sumWeightMap.end(); ++logMmIt)
		{
			stringstream logMsg;
			logMsg << "NetworkGraph: proxy/server : " << network_[logMmIt->second].netAddr.addressString() << " have a total weight of: " << logMmIt->first;
			Ginnungagap::Instance()->writeToLog(logMsg.str());
		}

		multimap<double, Node>::iterator mmIt = sumWeightMap.begin();
		return make_pair(network_[mmIt->second].netAddr, (thisWeight - mmIt->first)/clients.size());
	}

	pair<NetAddr, double> NetworkGraph::findBestCore(const list<NetAddr>& clients) const
	{
		stringstream logMsg;
		logMsg << "NetworkGraph: performing core selection on clients: ";
		Ginnungagap::Instance()->writeToLog(logMsg.str());

		Node node;
		bool ret;
		list<Node> clientNodes;
		typedef list<NetAddr>::const_iterator cClientsItr_t;
		for (cClientsItr_t itr = clients.begin(); itr != clients.end(); ++itr)
		{
			ret = getNode(*itr, node);
			/* skip deleted nodes... */
			if (ret)
			{
				clientNodes.push_back(node);
				logMsg.str("");
				logMsg << "NetworkGraph: client: " << itr->addressString();
				Ginnungagap::Instance()->writeToLog(logMsg.str());
			}
		}
		logMsg.str("");
		logMsg << "NetworkGraph: Total number of clients is: " << clientNodes.size();
		Ginnungagap::Instance()->writeToLog(logMsg.str());
		return findBestCore(clientNodes);
	}
}

