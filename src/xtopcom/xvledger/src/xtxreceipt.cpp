// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string>
#include "xvledger/xtxreceipt.h"
#include "xvledger/xvblockbuild.h"
#include "xvledger/xvcontract.h"
#include "xmetrics/xmetrics.h"
#include "xutility/xhash.h"

namespace top
{
    namespace base
    {
        REG_CLS(xtx_receipt_t);

        xtx_receipt_t::xtx_receipt_t()
        : m_tx_action({},{},{},"invalid") {  // TODO(jimmy)
            XMETRICS_GAUGE(metrics::dataobject_tx_receipt_t, 1);
        }
        xtx_receipt_t::xtx_receipt_t(const base::xvaction_t & txaction, base::xvqcert_t* prove_cert, const std::string & path, enum_xprove_cert_type type)
        : m_tx_action(txaction) {
            m_tx_action_prove = make_object_ptr<xprove_cert_t>(prove_cert, type, path);
            XMETRICS_GAUGE(metrics::dataobject_tx_receipt_t, 1);
        }

        xtx_receipt_t::~xtx_receipt_t() {
            XMETRICS_GAUGE(metrics::dataobject_tx_receipt_t, -1);
        }

        int32_t xtx_receipt_t::do_write(base::xstream_t & stream) {
            const int32_t begin_size = stream.size();
            std::string action_bin;
            m_tx_action.serialize_to(action_bin);
            stream.write_compact_var(action_bin);
            m_tx_action_prove->serialize_to(stream);
            return (stream.size() - begin_size);
        }
        int32_t xtx_receipt_t::do_read(base::xstream_t & stream) {
            const int32_t begin_size = stream.size();
            std::string action_bin;
            stream.read_compact_var(action_bin);
            m_tx_action.serialize_from(action_bin);
            m_tx_action_prove = make_object_ptr<xprove_cert_t>();
            m_tx_action_prove->serialize_from(stream);
            return (begin_size - stream.size());
        }

        std::string xtx_receipt_t::merkle_path_to_string(const base::xmerkle_path_256_t & path) {
            base::xstream_t _stream(base::xcontext_t::instance());
            path.serialize_to(_stream);
            std::string _path_bin = std::string((char *)_stream.data(), _stream.size());
            xassert(!_path_bin.empty());
            return _path_bin;
        }

        bool xtx_receipt_t::is_valid() const {
            if (get_tx_subtype() != base::enum_transaction_subtype_send && get_tx_subtype() != base::enum_transaction_subtype_recv) {
                xerror("xtx_receipt_t::is_valid not send or recv tx");
                return false;
            }

            if (m_tx_action_prove == nullptr) {
                xerror("xtx_receipt_t::is_valid prove cert null");
                return false;
            }

            if (!m_tx_action_prove->is_valid()) {
                xerror("xtx_receipt_t::is_valid prove invalid");
                return false;
            }

            if (m_tx_action_prove->get_prove_type() != enum_xprove_cert_type_unit_justify && m_tx_action_prove->get_prove_type() != enum_xprove_cert_type_table_justify) {
                xerror("xtx_receipt_t::is_valid prove type invalid");
                return false;
            }

            std::string _merkle_leaf;
            m_tx_action.serialize_to(_merkle_leaf);

            std::string root_hash = m_tx_action_prove->get_prove_root_hash();
            const std::string & _merkle_path_bin = m_tx_action_prove->get_prove_path();

            if (!_merkle_path_bin.empty()) {
                base::xmerkle_path_256_t path;
                base::xstream_t _stream(base::xcontext_t::instance(), (uint8_t *)_merkle_path_bin.data(), (uint32_t)_merkle_path_bin.size());
                int32_t ret = path.serialize_from(_stream);
                if (ret <= 0) {
                    xerror("xprove_cert_t::is_valid deserialize merkle path fail. ret=%d", ret);
                    return false;
                }
                base::xmerkle_t<utl::xsha2_256_t, uint256_t> merkle;
                if (!merkle.validate_path(_merkle_leaf, root_hash, path.get_levels())) {
                    xerror("xprove_cert_t::is_valid check merkle path fail.");
                    return false;
                }
            } else {
                if (_merkle_leaf != root_hash) {
                    xerror("xprove_cert_t::is_valid check prove object not equal with root fail");
                    return false;
                }
            }
            return true;
        }

        std::string xtx_receipt_t::get_tx_result_property(const std::string & key) const {
            const std::map<std::string,std::string>* map_ptr = m_tx_action.get_method_result()->get_map<std::string>();
            if (map_ptr != nullptr) {
                auto iter = map_ptr->find(key);
                if (iter != map_ptr->end()) {
                    return iter->second;
                }
                return {};
            }
            xassert(false);
            return {};
        }

        std::string xtx_receipt_t::get_contract_address() const {
            return xvcontract_t::get_contract_address(m_tx_action.get_contract_uri());
        }


        //----------------------------------------xtxreceipt_build_t-------------------------------------//
        std::vector<xfull_txreceipt_t> xtxreceipt_build_t::create_all_txreceipts(xvblock_t* commit_block, xvblock_t* cert_block, const std::vector<xvaction_t> & actions) {
            if (commit_block == nullptr || cert_block == nullptr) {
                xassert(false);
                return {};
            }
            if ( (cert_block->get_justify_cert_hash() != commit_block->get_input_root_hash())
                || (commit_block->get_height() + 2 != cert_block->get_height())
                || (commit_block->get_account() != cert_block->get_account())
                || (commit_block->get_block_level() != enum_xvblock_level_table && commit_block->get_block_level() != enum_xvblock_level_unit) ) {
                xassert(false);
                return {};
            }
            // get all leafs firstly for performance
            std::vector<std::string> all_leafs = xvblockmaker_t::get_input_merkle_leafs(commit_block->get_input());

            // #3 calc leaf path and make rceipt
            std::vector<xfull_txreceipt_t> txreceipts;
            for (auto & action : actions) {
                xmerkle_path_256_t hash_path;
                if (false == xvblockmaker_t::calc_merkle_path(all_leafs, action, hash_path)) {
                    xassert(false);
                    return {};
                }
                std::string path_bin = xtx_receipt_t::merkle_path_to_string(hash_path);
                enum_xprove_cert_type prove_type = commit_block->get_block_level() == enum_xvblock_level_table ? enum_xprove_cert_type_table_justify : enum_xprove_cert_type_unit_justify;
                xtx_receipt_t* _receipt = new xtx_receipt_t(action, cert_block->get_cert(), path_bin, prove_type);
                xtx_receipt_ptr_t _receipt_ptr;
                _receipt_ptr.attach(_receipt);
                std::string orgtx_bin = commit_block->get_input()->query_resource(action.get_org_tx_hash());
                xfull_txreceipt_t full_txreceipt(_receipt_ptr, orgtx_bin);  // orgtx_bin may be empty
                txreceipts.push_back(full_txreceipt);
            }
            xdbg("xtxreceipt_build_t::create_all_txreceipts,block=%s,receipts=%zu,allleafs=%zu,",
                commit_block->dump().c_str(), txreceipts.size(), all_leafs.size());
            return txreceipts;
        }

        std::vector<xfull_txreceipt_t> xtxreceipt_build_t::create_all_txreceipts(xvblock_t* commit_block, xvblock_t* cert_block) {
            if (commit_block == nullptr) {
                xassert(false);
                return {};
            }

            // #1 get send/recv action leafs, which need make txreceipt
            std::vector<xvaction_t> receipt_actions;
            auto & all_entitys = commit_block->get_input()->get_entitys();
            for (auto & entity : all_entitys) {
                // it must be xinentitys
                xvinentity_t* _inentity = dynamic_cast<xvinentity_t*>(entity);
                if (_inentity == nullptr) {
                    xassert(false);
                    return {};
                }
                auto & all_actions = _inentity->get_actions();
                for (auto & action : all_actions) {
                    if (false == action.get_org_tx_hash().empty()) {
                        enum_transaction_subtype _actionid = (enum_transaction_subtype)action.get_org_tx_action_id();
                        // only send and recv action need make receipt
                        if (_actionid == base::enum_transaction_subtype_send || _actionid == base::enum_transaction_subtype_recv) {
                            receipt_actions.push_back(action);
                        }
                    }
                }
            }
            if (receipt_actions.empty()) {
                return {};
            }

            return create_all_txreceipts(commit_block, cert_block, receipt_actions);
        }

        xfull_txreceipt_ptr_t xtxreceipt_build_t::create_one_txreceipt(xvblock_t* commit_block, xvblock_t* cert_block, const std::string & txhash) {
            if (commit_block == nullptr) {
                xassert(false);
                return nullptr;
            }

            // find txactin by txhash
            std::vector<xvaction_t> receipt_actions;
            auto & all_entitys = commit_block->get_input()->get_entitys();
            for (auto & entity : all_entitys) {
                // it must be xinentitys
                xvinentity_t* _inentity = dynamic_cast<xvinentity_t*>(entity);
                if (_inentity == nullptr) {
                    xassert(false);
                    return nullptr;
                }
                auto & all_actions = _inentity->get_actions();
                for (auto & action : all_actions) {
                    if (txhash == action.get_org_tx_hash()) {
                        receipt_actions.push_back(action);
                        break;
                    }
                }
            }
            if (receipt_actions.size() != 1) {  // not find txhash
                xassert(false);
                return nullptr;
            }

            std::vector<xfull_txreceipt_t> full_txreceipts = create_all_txreceipts(commit_block, cert_block, receipt_actions);
            if (full_txreceipts.size() != 1) {  // not find txhash
                xassert(false);
                return nullptr;
            }
            xfull_txreceipt_ptr_t txreceipt_ptr = std::make_shared<xfull_txreceipt_t>(full_txreceipts[0].get_txreceipt() ,full_txreceipts[0].get_tx_org_bin());
            return txreceipt_ptr;
        }

    }  // namespace base
}  // namespace top
