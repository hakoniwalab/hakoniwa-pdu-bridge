#pragma once

#include "hakoniwa/pdu/bridge/bridge_types.hpp" // For hako::pdu::bridge::PduKey
#include "hakoniwa/pdu/bridge/pdu_transfer_policy.hpp"
#include "hakoniwa/pdu/endpoint.hpp" // Actual Endpoint class
#include "hakoniwa/pdu/endpoint_types.hpp" // For hakoniwa::pdu::PduKey
#include "hakoniwa/pdu/pdu_definition.hpp" // For hakoniwa::pdu::PduDefinition

#include <memory>
#include <cstdint>
#include <chrono>
#include <vector> // For temporary buffer in transfer()

namespace hako::pdu::bridge {

class TransferPdu {
public:
    TransferPdu(
        const hako::pdu::bridge::PduKey& config_key, // The PduKey from bridge.json
        std::shared_ptr<IPduTransferPolicy> policy,
        hakoniwa::pdu::PduDefinition* pdu_definition, // Needed to get PDU size
        hakoniwa::pdu::Endpoint* src,
        hakoniwa::pdu::Endpoint* dst
    );

    void set_active(bool is_active);
    void set_epoch(uint64_t epoch);

    // Attempts to transfer data based on the policy.
    void try_transfer(std::chrono::steady_clock::time_point now);

private:
    hako::pdu::bridge::PduKey           config_pdu_key_; // PDU key from bridge.json
    hakoniwa::pdu::PduKey               endpoint_pdu_key_; // PDU key for endpoint API
    std::shared_ptr<IPduTransferPolicy> policy_;
    hakoniwa::pdu::PduDefinition*       pdu_definition_;
    hakoniwa::pdu::Endpoint*            src_endpoint_;
    hakoniwa::pdu::Endpoint*            dst_endpoint_;

    bool is_active_ = false;
    uint64_t owner_epoch_ = 0;

    void transfer();
    bool accept_epoch(uint64_t pdu_epoch);
};

} // namespace hako::pdu::bridge

