#include "hakoniwa/pdu/bridge/bridge_core.hpp"
#include <thread>
#include <chrono>

namespace hako::pdu::bridge {

BridgeCore::BridgeCore(const std::string& node_name) 
    : node_name_(node_name), is_running_(false) {}

void BridgeCore::add_connection(std::unique_ptr<BridgeConnection> connection) {
    connections_.push_back(std::move(connection));
}

void BridgeCore::run() {
    if (is_running_.exchange(true)) {
        // Already running in another thread.
        return;
    }

    while (is_running_) {
        auto now = std::chrono::steady_clock::now();
        for (auto& connection : connections_) {
            connection->step(now);
        }

        // The sleep duration determines the resolution of the bridge.
        // For ticker policies, this should be small enough not to miss ticks.
        // 1 millisecond is a common choice for high-resolution timers.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void BridgeCore::stop() {
    is_running_ = false;
}

} // namespace hako::pdu::bridge
