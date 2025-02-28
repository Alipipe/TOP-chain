// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * @file xvhost_face.h
 * @brief interface definitions for xvhost class
 */

#pragma once

#include "xbasic/xrunnable.h"
#include "xcommon/xlogic_time.h"
#include "xcommon/xmessage_category.h"
#include "xcommon/xnode_info.h"
#include "xdata/xelect_transaction.hpp"
#include "xdata/xelection/xelection_result_store.h"
#include "xdata/xnode_info.h"
#include "xnetwork/xmessage_transmission_property.h"
#include "xvnetwork/xaddress.h"
#include "xvnetwork/xmessage.h"
#include "xvnetwork/xmessage_ready_callback.h"
#include "xvnetwork/xvhost_face_fwd.h"
#include "xvnetwork/xvnetwork_driver_face.h"

#include <memory>
#include <vector>

NS_BEG2(top, vnetwork)

#ifndef VHOST_METRICS
#define VHOST_METRICS 0
#endif

// using callback_registration_token = xid_t<xmessage_ready_callback_t, std::size_t>;
/**
 * @brief xtop_vhost_face (or xvhost_face_t) is a virtual host delegate.
 *        when the election module runs, it will assign the physical node
 *        into different clusters at the same election round.  In such a
 *        situation, the upper modules face at least two problems:
 *             1. election round change which means the virtual network changes
 *             2. the physical node has more than two virtual network address
 *        When virtual network changes, it can be treated as different virtual
 *        networks.  In order to make the upper modules work correctly, in the
 *        new virtual network, the modules need to register notifications.
 *        But such kind of registration is in the same style.
 *        Meanwhile, a physical node stays in different clusters, communication
 *        becomes a little bit hard.  Upper modules need to know which virtual
 *        address should be selected as the source of the message to be sent.
 *
 *        Thus, a virtual host delegate is designed to help handling these issues.
 */
class xtop_vhost_face : public xbasic_runnable_t<xtop_vhost_face> {
public:
    xtop_vhost_face() = default;
    xtop_vhost_face(xtop_vhost_face const &) = delete;
    xtop_vhost_face & operator=(xtop_vhost_face const &) = delete;
    xtop_vhost_face(xtop_vhost_face &&) = default;
    xtop_vhost_face & operator=(xtop_vhost_face &&) = default;
    virtual ~xtop_vhost_face() = default;

    /**
     * @brief Register the message for a specified virtual address node.
     * @param vaddr The virtual address that registered for handling the
     *              message that sent to this virtual address.
     * @param cb The message callback.
     */
    virtual void register_message_ready_notify(xvnode_address_t const & vaddr, xmessage_ready_callback_t cb) = 0;

    /**
     * @brief Un-register the message notify for a vitual address.
     * @param vaddr The virtual address that don't need to monitor the message sent to it.
     */
    virtual void unregister_message_ready_notify(xvnode_address_t const & vaddr) = 0;

    // virtual
    // void
    // handshake(xvnode_address_t const & src, std::vector<xvnode_address_t> const & seeds) = 0;

    /**
     * @brief Send message.
     * @param message The message to be sent.
     * @param src The send address of the message.
     * @param dst The receive address of the message.
     * @param transmission_property Transmission property.
     */
    virtual void send(xmessage_t const & message,
                      xvnode_address_t const & src,
                      xvnode_address_t const & dst,
                      network::xtransmission_property_t const & transmission_property = {}) = 0;

    // virtual
    // void
    // send(xmessage_t const & message,
    //     xvnode_address_t const & src,
    //     xvnode_address_t const & dst,
    //     network::xtransmission_property_t const & transmission_property = {},
    //     std::error_code & ec) = 0;

    /**
     * @brief Send message to all nodes with src & dst specified.
              The receiver itself makes the decision if the dst doesn't match
              its address.
     * @param message The message to be broadcasted.
     * @param src The send address of the message.
     * @param dst The receive address of the message.  Usually the boradcast
     *            message should be sent to the address representing the whole
     *            network.  But the reality is the new joining node which doesn't
     *            have the ability to get the virtual network topology info.  It
     *            needs to broadcast the join message to the REC cluster.  Only
     *            REC nodes will handle this message.
     */
    virtual void broadcast_to_all(xmessage_t const & message, xvnode_address_t const & src, xvnode_address_t const & dst) = 0;

    /**
     * @brief Broadcast the message to the network that holds the src.
     * @param message The message to be broadcast.
     * @param src The src address of the broadcast message.
     */
    virtual void broadcast(xmessage_t const & message, xvnode_address_t const & src) = 0;

    virtual void forward_broadcast_message(xmessage_t const & message, xvnode_address_t const & src, xvnode_address_t const & dst) = 0;

    virtual void send(common::xnode_address_t const & src, common::xip2_t const & dst, xmessage_t const & message, std::error_code & ec) = 0;
    virtual void broadcast(common::xnode_address_t const & src, common::xip2_t const & dst, xmessage_t const & message, std::error_code & ec) = 0;

    /**
     * @brief Get the network info object associated with this virtual host object.
     * @return The network info object.
     */
    virtual common::xnetwork_id_t const & network_id() const noexcept = 0;

    /**
     * @brief Get the node id associated with this virtual host object.
     * @return The node id.
     */
    virtual common::xnode_id_t const & host_node_id() const noexcept = 0;

    /**
     * @brief Get the node infos of specified group
     * 
     * @param group_addr The cluster address of target group
     * @param election_round The election round of target group
     * @return std::map<common::xslot_id_t, data::xnode_info_t> 
     */
    virtual std::map<common::xslot_id_t, data::xnode_info_t> members_info_of_group2(xcluster_address_t const & group_addr, common::xelection_round_t const & election_round) const = 0;

    /**
     * @brief Get the parent node info of specified child node
     * 
     * @param child_addr the child node address
     * @return xvnode_address_t 
     */
    virtual xvnode_address_t parent_group_address(xvnode_address_t const & child_addr) const = 0;

    /**
     * @brief Get the pub_key of vnodes
     * 
     * @param nodes nodes' vector
     * @return std::map<xvnode_address_t, xcrypto_key_t<pub>> 
     */
    virtual std::map<xvnode_address_t, xcrypto_key_t<pub>> crypto_keys(std::vector<xvnode_address_t> const & nodes) const = 0;

    /**
     * @brief Get the logic chain time
     * 
     * @return common::xlogic_time_t 
     */
    virtual common::xlogic_time_t last_logic_time() const noexcept = 0;
};

NS_END2
