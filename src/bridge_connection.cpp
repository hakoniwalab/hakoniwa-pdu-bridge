#include "hakoniwa/pdu/bridge/bridge_connection.hpp"

namespace hako::pdu::bridge {

void BridgeConnection::add_transfer_pdu(std::unique_ptr<TransferPdu> pdu) {
    transfer_pdus_.push_back(std::move(pdu));
}

void BridgeConnection::step(std::chrono::steady_clock::time_point now) {
    for (auto& pdu : transfer_pdus_) {
        pdu->try_transfer(now);
    }
}

} // namespace hako::pdu::bridge
