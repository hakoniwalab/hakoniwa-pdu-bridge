#pragma once

#include "hakoniwa/pdu/bridge/transfer_pdu.hpp"
#include <vector>
#include <memory>
#include <chrono>

namespace hako::pdu::bridge {

class BridgeConnection {
public:
    BridgeConnection() = default;

    void add_transfer_pdu(std::unique_ptr<TransferPdu> pdu);

    void step(std::chrono::steady_clock::time_point now);

private:
    std::vector<std::unique_ptr<TransferPdu>> transfer_pdus_;
};

} // namespace hako::pdu::bridge


