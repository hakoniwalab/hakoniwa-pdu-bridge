#include "hakoniwa/pdu/bridge/policy/immediate_policy.hpp"

namespace hako::pdu::bridge {

bool ImmediatePolicy::should_transfer(std::chrono::steady_clock::time_point /* now */) {
    // For immediate policy, the answer is always yes when checked upon PDU update.
    return true;
}

void ImmediatePolicy::on_transferred(std::chrono::steady_clock::time_point /* now */) {
    // No state needs to be updated.
}

} // namespace hako::pdu::bridge
