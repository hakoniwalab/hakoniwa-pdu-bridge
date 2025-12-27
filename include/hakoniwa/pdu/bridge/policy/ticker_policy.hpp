#pragma once

#include "hakoniwa/pdu/bridge/pdu_transfer_policy.hpp"
#include <chrono>

namespace hako::pdu::bridge {

class TickerPolicy : public IPduTransferPolicy {
public:
    explicit TickerPolicy(std::chrono::milliseconds interval);
    ~TickerPolicy() = default;

    bool should_transfer(std::chrono::steady_clock::time_point now) override;
    void on_transferred(std::chrono::steady_clock::time_point now) override;

private:
    std::chrono::milliseconds interval_;
    std::chrono::steady_clock::time_point next_tick_time_;
    bool initialized_;
};

} // namespace hako::pdu::bridge


