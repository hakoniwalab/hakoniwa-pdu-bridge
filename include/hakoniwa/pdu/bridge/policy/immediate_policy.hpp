#pragma once

#include "hakoniwa/pdu/bridge/pdu_transfer_policy.hpp"

namespace hako::pdu::bridge {

class ImmediatePolicy : public IPduTransferPolicy {
public:
    ImmediatePolicy() = default;
    ~ImmediatePolicy() = default;

    bool should_transfer(std::chrono::steady_clock::time_point now) override;
    void on_transferred(std::chrono::steady_clock::time_point now) override;
};

} // namespace hako::pdu::bridge


