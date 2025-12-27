#pragma once

#include "hakoniwa/pdu/bridge/pdu_transfer_policy.hpp"
#include <chrono>

namespace hako::pdu::bridge {

class ThrottlePolicy : public IPduTransferPolicy {
public:
    explicit ThrottlePolicy(std::chrono::milliseconds interval);
    ~ThrottlePolicy() = default;

    bool should_transfer(std::chrono::steady_clock::time_point now) override;
    void on_transferred(std::chrono::steady_clock::time_point now) override;

private:
    std::chrono::milliseconds interval_;
    std::chrono::steady_clock::time_point last_transfer_time_;
};

} // namespace hako::pdu::bridge


