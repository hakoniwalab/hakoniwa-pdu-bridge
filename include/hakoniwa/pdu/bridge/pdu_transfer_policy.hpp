#pragma once

#include <chrono>

namespace hako::pdu::bridge {

class IPduTransferPolicy {
public:
    virtual ~IPduTransferPolicy() = default;

    // Checks if a transfer should occur at the given time.
    virtual bool should_transfer(std::chrono::steady_clock::time_point now) = 0;

    // Notifies the policy that a transfer has occurred.
    virtual void on_transferred(std::chrono::steady_clock::time_point now) = 0;
};

} // namespace hako::pdu::bridge


