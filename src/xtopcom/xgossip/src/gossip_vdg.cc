// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xgossip/include/gossip_vdg.h"

#include "xgossip/include/gossip_utils.h"
#include "xgossip/include/mesages_with_bloomfilter.h"
#include "xpbase/base/kad_key/kadmlia_key.h"

#include <cinttypes>

namespace top {
namespace gossip {

GossipVerifiableDirectedGraph::GossipVerifiableDirectedGraph(transport::TransportPtr transport_ptr) : GossipInterface{transport_ptr} {
}
GossipVerifiableDirectedGraph::~GossipVerifiableDirectedGraph() {
}

void GossipVerifiableDirectedGraph::Broadcast(transport::protobuf::RoutingMessage & message, kadmlia::ElectRoutingTablePtr & routing_table) {
    CheckDiffNetwork(message);

    auto gossip_max_hop_num = kGossipDefaultMaxHopNum;
    if (message.gossip().max_hop_num() > 0) {
        gossip_max_hop_num = message.gossip().max_hop_num();
    }
    if (gossip_max_hop_num <= message.hop_num()) {
        xdbg("msg_hash:%u msg_type:%d hop_num:%d larger than gossip_max_hop_num:%d", message.msg_hash(), message.type(), message.hop_num(), gossip_max_hop_num);
        return;
    }

    MessageKey msg_key(message.msg_hash());
    if (MessageWithBloomfilter::Instance()->StopGossip(msg_key, 1)) {
        xdbg("stop gossip at message %u", message.msg_hash());
        return;
    }

    auto kad_key_ptr = base::GetKadmliaKey(message.des_node_id());

    std::vector<top::kadmlia::NodeInfoPtr> select_nodes;
    CalcNeighborhood(message, routing_table, select_nodes);
    if (select_nodes.empty()) {
        xdbg("stop broadcast, select_nodes empty,msg_hash:%u msg_type:%d hop_num:%d", message.msg_hash(), message.type(), message.hop_num());
        return;
    }

    Send(message, select_nodes);
}

void GossipVerifiableDirectedGraph::CalcNeighborhood(transport::protobuf::RoutingMessage & message,
                                                     kadmlia::ElectRoutingTablePtr & routing_table,
                                                     std::vector<top::kadmlia::NodeInfoPtr> & select_nodes) {
    std::unordered_map<std::string, kadmlia::NodeInfoPtr> const nodes_map = routing_table->nodes();
    std::vector<std::string> stable_xip2 = routing_table->get_stable_xip2();
    std::mt19937 prng{message.msg_hash()};

    for (auto _s : stable_xip2) {
        xdbg("%s", _s.c_str());
    }

    auto group_size = stable_xip2.size();
    for (std::size_t v_index = 0; v_index < group_size; ++v_index) {
        auto const r = std::uniform_int_distribution<std::size_t>{v_index, group_size - 1}(prng);
        std::swap(stable_xip2[v_index], stable_xip2[r]);
    }


    for (auto _s : stable_xip2) {
        xdbg("%s", _s.c_str());
    }

    std::size_t located = 0;
    for (auto const & v_xip2 : stable_xip2) {
        if (v_xip2 == routing_table->get_local_node_info()->kad_key()) {
            xdbg("self: %s", routing_table->get_local_node_info()->kad_key().c_str());
            break;
        }
        located++;
    }

    std::set<std::string> select_xip;
    select_xip.insert(stable_xip2[(located + 17) % group_size]);
    select_xip.insert(stable_xip2[(located + 47) % group_size]);
    select_xip.insert(stable_xip2[(located + 131) % group_size]);
    select_xip.insert(stable_xip2[(located + 307) % group_size]);

    for (auto _s : select_xip) {
        xdbg("%s", _s.c_str());
    }

    for (auto const & s_xip : select_xip) {
        select_nodes.push_back(nodes_map.at(s_xip));
    }

    return;
}

}  // namespace gossip
}  // namespace top
