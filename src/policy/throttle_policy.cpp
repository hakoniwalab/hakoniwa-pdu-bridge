#include "hakoniwa/pdu/bridge/policy/throttle_policy.hpp"

namespace hako::pdu::bridge {

ThrottlePolicy::ThrottlePolicy(std::chrono::milliseconds interval)
    : interval_(interval), last_transfer_time_(std::chrono::steady_clock::time_point::min()) {}

bool ThrottlePolicy::should_transfer(std::chrono::steady_clock::time_point now) {
    if ((now - last_transfer_time_) >= interval_) {
        return true;
    }
    return false;
}

void ThrottlePolicy::on_transferred(std::chrono::steady_clock::time_point now) {
    last_transfer_time_ = now;
}

} // namespace hako::pdu::bridge
