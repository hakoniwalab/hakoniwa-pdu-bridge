#include "hakoniwa/pdu/bridge/bridge_loader.hpp"
#include <iostream>
#include <signal.h>
#include <memory>
#include <thread>
#include <vector>

// Global pointer to the core for the signal handler
std::unique_ptr<hako::pdu::bridge::BridgeCore> g_core;

void signal_handler(int signum) {
    if (signum == SIGINT) {
        std::cout << "Interrupt signal (" << signum << ") received." << std::endl;
        if (g_core) {
            g_core->stop();
        }
    }
}

// Mock Endpoint for the sample. In a real application, this would be loaded
// from the hakoniwa-pdu-endpoint library and configured properly.
class MockEndpoint : public hakoniwa::pdu::Endpoint {
public:
    MockEndpoint(const std::string& name) : hakoniwa::pdu::Endpoint(name, HAKO_PDU_ENDPOINT_DIRECTION_INOUT) {}

    // Public method to allow setting the protected pdu_def_ member
    void set_pdu_def(std::shared_ptr<hakoniwa::pdu::PduDefinition> def) {
        pdu_def_ = def;
    }

    // Override open to prevent loading from files.
    HakoPduErrorType open(const std::string& config_path) override {
        // Do nothing, bypass file loading.
        return HAKO_PDU_ERR_OK;
    }
    // Override other virtual methods as needed for mock behavior.
    HakoPduErrorType close() noexcept override { return HAKO_PDU_ERR_OK; }
    HakoPduErrorType start() noexcept override { return HAKO_PDU_ERR_OK; }
    HakoPduErrorType stop() noexcept override { return HAKO_PDU_ERR_OK; }
    HakoPduErrorType is_running(bool& running) noexcept override { running = true; return HAKO_PDU_ERR_OK; }
    
    // Override low-level virtual send/recv for custom mock behavior.
    HakoPduErrorType recv(const hakoniwa::pdu::PduResolvedKey& pdu_key, std::span<std::byte> data, size_t& received_size) noexcept override {
        if (data.size() >= sizeof(uint64_t)) {
            static uint64_t epoch = 1; // Simulate changing data
            *reinterpret_cast<uint64_t*>(data.data()) = epoch++;
            received_size = data.size();
            return HAKO_PDU_ERR_OK;
        }
        return HAKO_PDU_ERR_INVALID_ARGUMENT;
    }
    HakoPduErrorType send(const hakoniwa::pdu::PduResolvedKey& pdu_key, std::span<const std::byte> data) noexcept override {
        std::cout << "MOCK(" << get_name() << ")::send() for channel " << pdu_key.channel_id << ", size: " << data.size() << std::endl;
        return HAKO_PDU_ERR_OK;
    }
};


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_bridge.json> [node_name]" << std::endl;
        return 1;
    }
    signal(SIGINT, signal_handler);

    std::string config_path = argv[1];
    std::string node_name = (argc > 2) ? argv[2] : "node1";


    try {
        // 1. Mock PDU definitions and Endpoints for this sample.
        // In a real application, these would be loaded and initialized properly.
        auto pdu_def_node1 = std::make_shared<hakoniwa::pdu::PduDefinition>();
        
        // Populate definitions to match simple-bridge.json, including channel IDs.
        pdu_def_node1->pdu_definitions_["Drone"]["pos"] = {"sensor_msgs/Twist", "pos", "pos", 1, 32, "shm"};
        pdu_def_node1->pdu_definitions_["Drone"]["motor"] = {"std_msgs/Float64", "motor", "motor", 2, 16, "shm"};
        pdu_def_node1->pdu_definitions_["Drone"]["hako_camera_data"] = {"sensor_msgs/Image", "hako_camera_data", "hako_camera_data", 3, 65536, "shm"};

        std::map<std::string, std::shared_ptr<hakoniwa::pdu::PduDefinition>> pdu_definitions;
        pdu_definitions[node_name] = pdu_def_node1;

        // Create mock endpoints
        auto ep_src = std::make_shared<MockEndpoint>("n1-epSrc");
        auto ep_dst = std::make_shared<MockEndpoint>("n1-epDst");

        // Set the pdu_def for the mock endpoints using the public method
        ep_src->set_pdu_def(pdu_def_node1);
        ep_dst->set_pdu_def(pdu_def_node1);
        
        std::map<std::string, std::shared_ptr<hakoniwa::pdu::Endpoint>> endpoints;
        endpoints["n1-epSrc"] = ep_src;
        endpoints["n1-epDst"] = ep_dst;


        // 2. Load the bridge core using the configuration and mocked objects
        g_core = hako::pdu::bridge::BridgeLoader::load(config_path, node_name, endpoints, pdu_definitions);

        std::cout << "Bridge core loaded for node " << node_name << ". Running... (Press Ctrl+C to stop)" << std::endl;
        g_core->run();
        std::cout << "Bridge core stopped." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
