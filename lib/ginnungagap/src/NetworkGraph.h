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

#ifndef GINNUNGAGAP_CORESELECTIONSERVICE_H
#define GINNUNGAGAP_CORESELECTIONSERVICE_H

#include "NetAddr.h"

#include <climits>
#include <iostream>
#include <utility>
#include <algorithm>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <set>

namespace ggg
{
	struct NodeProperties
	{
		NetAddr netAddr;
	};

	struct LinkProperties
	{
		double weight;
	};

	typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, NodeProperties, LinkProperties> Graph;
	typedef Graph::vertex_descriptor Node;
	typedef std::pair<Graph::edge_descriptor, bool> Link;

	class NetworkGraph
	{
		public:
			NetworkGraph() : network_(Graph(0)) {}

			void addServer(const NetAddr& netAddr);
			void addClient(const NetAddr& netAddr);

			void removeClientOrServer(const NetAddr& netAddr);

			std::list<NetAddr> getClientList() const;
			std::list<NetAddr> getServerList() const;

			bool getNode(const NetAddr& netAddr, Node& node) const;

			bool updateLink(const NetAddr& node1, const NetAddr& node2, const double& weight);

			/* returns best node, and gain over current node */
			std::pair<NetAddr, double> findBestCore(const std::list<Node>& clients) const;
			std::pair<NetAddr, double> findBestCore(const std::list<NetAddr>& clients) const;

		private:
			Graph network_;
			std::list<Node> serverAndProxies_;
			std::list<Node> clients_;
	};
}

#endif

