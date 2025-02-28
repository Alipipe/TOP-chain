// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xvnetwork/xvhost_face.h"

using top::vnetwork::xcluster_address_t;
using top::vnetwork::xmessage_ready_callback_t;
using top::vnetwork::xmessage_t;
using top::vnetwork::xvnode_address_t;

NS_BEG3(top, tests, vnetwork)

class xtop_dummy_vhost : public top::vnetwork::xvhost_face_t {
public:
    XDECLARE_DEFAULTED_DEFAULT_CONSTRUCTOR(xtop_dummy_vhost);
    XDECLARE_DELETED_COPY_DEFAULTED_MOVE_SEMANTICS(xtop_dummy_vhost);
    XDECLARE_DEFAULTED_OVERRIDE_DESTRUCTOR(xtop_dummy_vhost);

    void register_message_ready_notify(xvnode_address_t const &, xmessage_ready_callback_t) override {}

    void unregister_message_ready_notify(xvnode_address_t const &) override {}

    void start() override {}

    void stop() override {}

    bool running() const noexcept override { return false; }

    void running(bool const) noexcept override {}

    void send(common::xnode_address_t const & src, common::xip2_t const & dst, xmessage_t const & message, std::error_code & ec) {}

    void broadcast(common::xnode_address_t const & src, common::xip2_t const & dst, xmessage_t const & message, std::error_code & ec) {}

    void send(xmessage_t const &, xvnode_address_t const &, xvnode_address_t const &, top::network::xtransmission_property_t const &) override { m_counter++; }

    void forward_broadcast_message(xmessage_t const &, xvnode_address_t const &, xvnode_address_t const &) override { m_counter++; }

    void broadcast_to_all(xmessage_t const &, xvnode_address_t const &, xvnode_address_t const &) override { m_counter++; }

    void broadcast(xmessage_t const &, xvnode_address_t const &) override { m_counter++; }

    common::xnetwork_id_t const & network_id() const noexcept override { return net_id; }

    common::xnode_id_t const & host_node_id() const noexcept override { return node_id; }

    xvnode_address_t parent_group_address(xvnode_address_t const &) const override {
        m_counter++;
        return {};
    }

    // std::unordered_map<common::xcluster_address_t, xgroup_update_result_t> build_vnetwork(xvnetwork_construction_data_t const &) override { return {}; }

    // std::unordered_map<common::xcluster_address_t, xgroup_update_result_t> build_vnetwork(data::election::xelection_result_store_t const &, common::xzone_id_t const &) override {
    //     return {};
    // }

    std::map<xvnode_address_t, xcrypto_key_t<pub>> crypto_keys(std::vector<xvnode_address_t> const & nodes) const { return {}; }

    std::map<common::xslot_id_t, data::xnode_info_t> members_info_of_group2(xcluster_address_t const &, common::xelection_round_t const &) const override {
        m_counter++;
        return {{}, {}};
    }

    common::xlogic_time_t last_logic_time() const noexcept override { return 0; }

    common::xnetwork_id_t const net_id{1};
    common::xnode_id_t const    node_id{"test1"};

    mutable int m_counter;
};
using xdummy_vhost_t = xtop_dummy_vhost;

extern xdummy_vhost_t xdummy_vhost;

NS_END3
