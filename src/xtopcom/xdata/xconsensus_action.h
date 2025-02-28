// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbase/xobject_ptr.h"
#include "xbasic/xbyte_buffer.h"
#include "xcommon/xaddress.h"
#include "xdata/xcons_transaction.h"
#include "xdata/xconsensus_action_fwd.h"
#include "xdata/xconsensus_action_stage.h"
#include "xdata/xtop_action.h"

#include <cstdint>
#include <string>

NS_BEG2(top, data)

template <xtop_action_type_t ActionTypeV>
class xtop_consensus_action : public xtop_action_t<ActionTypeV> {
public:
    xtop_consensus_action(xtop_consensus_action const &) = default;
    xtop_consensus_action & operator=(xtop_consensus_action const &) = default;
    xtop_consensus_action(xtop_consensus_action &&) = default;
    xtop_consensus_action & operator=(xtop_consensus_action &&) = default;
    ~xtop_consensus_action() override = default;

    explicit xtop_consensus_action(xobject_ptr_t<data::xcons_transaction_t> const & tx) noexcept;

    xconsensus_action_stage_t stage() const noexcept;
    common::xaccount_address_t from_address() const;
    common::xaccount_address_t to_address() const;
    common::xaccount_address_t contract_address() const;
    common::xaccount_address_t execution_address() const;
    uint64_t max_gas_amount() const;
    uint64_t nonce() const noexcept;
    std::string action_name() const;
    xbyte_buffer_t action_data() const;
};

NS_END2

#include "xconfig/xconfig_register.h"
#include "xconfig/xpredefined_configurations.h"
#include "xvledger/xvblock.h"

NS_BEG2(top, data)

template <xtop_action_type_t ActionTypeV>
xtop_consensus_action<ActionTypeV>::xtop_consensus_action(xobject_ptr_t<data::xcons_transaction_t> const & tx) noexcept : xtop_top_action<ActionTypeV>{ tx, tx->is_send_tx() ? static_cast<common::xlogic_time_t>((tx->get_transaction()->get_fire_timestamp() + tx->get_transaction()->get_expire_duration() + XGET_ONCHAIN_GOVERNANCE_PARAMETER(tx_send_timestamp_tolerance)) / XGLOBAL_TIMER_INTERVAL_IN_SECONDS) : common::xjudgement_day } {
}

template <xtop_action_type_t ActionTypeV>
xconsensus_action_stage_t xtop_consensus_action<ActionTypeV>::stage() const noexcept {
    auto const & tx = dynamic_xobject_ptr_cast<data::xcons_transaction_t>(this->m_action_src);
    assert(tx != nullptr);
    switch (tx->get_tx_subtype()) {  // NOLINT(clang-diagnostic-switch-enum)
    case base::enum_transaction_subtype_send:
        return xconsensus_action_stage_t::send;

    case base::enum_transaction_subtype_recv:
        return xconsensus_action_stage_t::recv;

    case base::enum_transaction_subtype_confirm:
        return xconsensus_action_stage_t::confirm;

    case base::enum_transaction_subtype_self:
        return xconsensus_action_stage_t::self;

    default:
        assert(false);
        return xconsensus_action_stage_t::invalid;
    }
}

template <xtop_action_type_t ActionTypeV>
common::xaccount_address_t xtop_consensus_action<ActionTypeV>::from_address() const {
    auto const & tx = dynamic_xobject_ptr_cast<data::xcons_transaction_t>(this->m_action_src);
    assert(tx != nullptr);
    return common::xaccount_address_t{ tx->get_source_addr() };
}

template <xtop_action_type_t ActionTypeV>
common::xaccount_address_t xtop_consensus_action<ActionTypeV>::to_address() const {
    auto const & tx = dynamic_xobject_ptr_cast<data::xcons_transaction_t>(this->m_action_src);
    assert(tx != nullptr);
    return common::xaccount_address_t{ tx->get_target_addr() };
}

template <xtop_action_type_t ActionTypeV>
common::xaccount_address_t xtop_consensus_action<ActionTypeV>::contract_address() const {
    auto const & tx = dynamic_xobject_ptr_cast<data::xcons_transaction_t>(this->m_action_src);
    assert(tx != nullptr);
    return common::xaccount_address_t{ tx->get_target_addr() };
}

template <xtop_action_type_t ActionTypeV>
common::xaccount_address_t xtop_consensus_action<ActionTypeV>::execution_address() const {
    auto const & tx = dynamic_xobject_ptr_cast<data::xcons_transaction_t>(this->m_action_src);
    assert(tx != nullptr);

    switch (stage()) {
    case xconsensus_action_stage_t::send:
        XATTRIBUTE_FALLTHROUGH;
    case xconsensus_action_stage_t::confirm:
        return common::xaccount_address_t{ tx->get_source_addr() };

    case xconsensus_action_stage_t::recv:
        return common::xaccount_address_t{ tx->get_target_addr() };

    case xconsensus_action_stage_t::self:
        return common::xaccount_address_t{ tx->get_target_addr() };

    default:
        assert(false);
        return common::xaccount_address_t{};
    }
}

template <xtop_action_type_t ActionTypeV>
uint64_t xtop_consensus_action<ActionTypeV>::max_gas_amount() const {
    auto const & tx = dynamic_xobject_ptr_cast<data::xcons_transaction_t>(this->m_action_src);
    assert(tx != nullptr);
    return XGET_ONCHAIN_GOVERNANCE_PARAMETER(tx_deposit_gas_exchange_ratio) * tx->get_transaction()->get_deposit(); // TODO free tgas is missing here.
}

template <xtop_action_type_t ActionTypeV>
uint64_t xtop_consensus_action<ActionTypeV>::nonce() const noexcept {
    auto const & tx = dynamic_xobject_ptr_cast<data::xcons_transaction_t>(this->m_action_src);
    assert(tx != nullptr);
    
    return tx->get_transaction()->get_tx_nonce();
}

template <xtop_action_type_t ActionTypeV>
std::string xtop_consensus_action<ActionTypeV>::action_name() const {
    auto const & tx = dynamic_xobject_ptr_cast<data::xcons_transaction_t>(this->m_action_src);
    assert(tx != nullptr);

    return tx->get_target_action().get_action_name();
}

template <xtop_action_type_t ActionTypeV>
xbyte_buffer_t xtop_consensus_action<ActionTypeV>::action_data() const {
    auto const & tx = dynamic_xobject_ptr_cast<data::xcons_transaction_t>(this->m_action_src);
    assert(tx != nullptr);

    return { std::begin(tx->get_target_action().get_action_param()), std::end(tx->get_target_action().get_action_param()) };
}

NS_END2
