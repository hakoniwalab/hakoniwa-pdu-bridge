#include "hakoniwa/pdu/bridge/bridge_loader.hpp"
#include "hakoniwa/pdu/bridge/bridge_types.hpp"
#include "hakoniwa/pdu/bridge/policy/immediate_policy.hpp"
#include "hakoniwa/pdu/bridge/policy/throttle_policy.hpp"
#include "hakoniwa/pdu/bridge/policy/ticker_policy.hpp"

#include <nlohmann/json.hpp> // nlohmann/json

#include <fstream>
#include <stdexcept>

namespace hako::pdu::bridge {

    using json = nlohmann::json;

    // JSON parsing helpers for BridgeConfig DTOs
    void from_json(const json& j, TransferPolicy& p) {
        j.at("type").get_to(p.type);
        if (j.contains("intervalMs")) {
            p.intervalMs = j.at("intervalMs").get<int>();
        }
    }
    void from_json(const json& j, Node& n) {
        j.at("id").get_to(n.id);
    }
    void from_json(const json& j, EndpointDefinition& e) {
        j.at("id").get_to(e.id);
        j.at("mode").get_to(e.mode);
    }
    void from_json(const json& j, NodeEndpoints& n) {
        j.at("nodeId").get_to(n.nodeId);
        j.at("endpoints").get_to(n.endpoints);
    }
    void from_json(const json& j, WireLink& w) {
        j.at("from").get_to(w.from);
        j.at("to").get_to(w.to);
    }
    void from_json(const json& j, PduKey& p) {
        j.at("id").get_to(p.id);
        j.at("robot_name").get_to(p.robot_name);
        j.at("pdu_name").get_to(p.pdu_name);
    }
    void from_json(const json& j, ConnectionSource& s) {
        j.at("endpointId").get_to(s.endpointId);
    }
    void from_json(const json& j, ConnectionDestination& d) {
        j.at("endpointId").get_to(d.endpointId);
    }
    void from_json(const json& j, TransferPduConfig& t) {
        j.at("pduKeyGroupId").get_to(t.pduKeyGroupId);
        j.at("policyId").get_to(t.policyId);
    }
    void from_json(const json& j, Connection& c) {
        j.at("id").get_to(c.id);
        j.at("nodeId").get_to(c.nodeId);
        j.at("source").get_to(c.source);
        j.at("destinations").get_to(c.destinations);
        j.at("transferPdus").get_to(c.transferPdus);
    }
    void from_json(const json& j, BridgeConfig& b) {
        j.at("version").get_to(b.version);
        j.at("transferPolicies").get_to(b.transferPolicies);
        j.at("nodes").get_to(b.nodes);
        j.at("endpoints").get_to(b.endpoints);
        if (j.contains("wireLinks")) {
            j.at("wireLinks").get_to(b.wireLinks);
        }
        j.at("pduKeyGroups").get_to(b.pduKeyGroups);
        j.at("connections").get_to(b.connections);
    }

    static BridgeConfig parse_config_from_file(const std::string& config_path) {
        std::ifstream ifs(config_path);
        if (!ifs.is_open()) {
            throw std::runtime_error("BridgeLoader: Failed to open config file: " + config_path);
        }
        json j;
        ifs >> j;
        return j.get<BridgeConfig>();
    }

    std::unique_ptr<BridgeCore> BridgeLoader::load(
        const std::string& config_path,
        const std::string& node_name,
        const std::map<std::string, std::shared_ptr<hakoniwa::pdu::Endpoint>>& endpoints,
        const std::map<std::string, std::shared_ptr<hakoniwa::pdu::PduDefinition>>& pdu_definitions)
    {
        auto config = parse_config_from_file(config_path);
        auto core = std::make_unique<BridgeCore>(node_name);

        // 1. Instantiate policies
        std::map<std::string, std::shared_ptr<IPduTransferPolicy>> policy_map;
        for (const auto& pair : config.transferPolicies) {
            const auto& id = pair.first;
            const auto& policy_def = pair.second;
            if (policy_def.type == "immediate") {
                policy_map[id] = std::make_shared<ImmediatePolicy>();
            } else if (policy_def.type == "throttle") {
                if (!policy_def.intervalMs) throw std::runtime_error("throttle policy needs intervalMs");
                policy_map[id] = std::make_shared<ThrottlePolicy>(std::chrono::milliseconds(*policy_def.intervalMs));
            } else if (policy_def.type == "ticker") {
                if (!policy_def.intervalMs) throw std::runtime_error("ticker policy needs intervalMs");
                policy_map[id] = std::make_shared<TickerPolicy>(std::chrono::milliseconds(*policy_def.intervalMs));
            }
        }

        // 2. Create Connections and TransferPdus
        for (const auto& conn_def : config.connections) {
            if (conn_def.nodeId != node_name) {
                continue; // Skip connections not intended for this node
            }
            auto connection = std::make_unique<BridgeConnection>(conn_def.nodeId);
            
            auto src_ep_it = endpoints.find(conn_def.source.endpointId);
            if (src_ep_it == endpoints.end()) throw std::runtime_error("Source endpoint not found: " + conn_def.source.endpointId);
            auto* src_ep = src_ep_it->second.get();
            
            auto pdu_def_it = pdu_definitions.find(conn_def.nodeId);
             if (pdu_def_it == pdu_definitions.end()) throw std::runtime_error("PduDefinition not found for node: " + conn_def.nodeId);
            auto* pdu_def = pdu_def_it->second.get();

            for (const auto& dest_def : conn_def.destinations) {
                auto dst_ep_it = endpoints.find(dest_def.endpointId);
                if (dst_ep_it == endpoints.end()) throw std::runtime_error("Destination endpoint not found: " + dest_def.endpointId);
                auto* dst_ep = dst_ep_it->second.get();

                for (const auto& trans_pdu_def : conn_def.transferPdus) {
                    auto policy = policy_map.at(trans_pdu_def.policyId);
                    const auto& pdu_keys = config.pduKeyGroups.at(trans_pdu_def.pduKeyGroupId);

                    for (const auto& pdu_key_def : pdu_keys) {
                        auto transfer_pdu = std::make_unique<TransferPdu>(pdu_key_def, policy, pdu_def, src_ep, dst_ep);
                        connection->add_transfer_pdu(std::move(transfer_pdu));
                    }
                }
            }
            core->add_connection(std::move(connection));
        }

        return core;
    }

} // namespace hako::pdu::bridge
