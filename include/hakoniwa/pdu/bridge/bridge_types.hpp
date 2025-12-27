#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace hako::pdu::bridge {

// from transferPolicies
struct TransferPolicy {
    std::string type;
    std::optional<int> intervalMs;
};

// from nodes
struct Node {
    std::string id;
};

// from endpoints
struct EndpointDefinition {
    std::string id;
    std::string mode;
};

struct NodeEndpoints {
    std::string nodeId;
    std::vector<EndpointDefinition> endpoints;
};

// from wireLinks
struct WireLink {
    std::string from;
    std::string to;
};

// from pduKeyGroups
struct PduKey {
    std::string id;
    std::string robot_name;
    std::string pdu_name;
};

// from connections
struct ConnectionSource {
    std::string endpointId;
};

struct ConnectionDestination {
    std::string endpointId;
};

struct TransferPduConfig {
    std::string pduKeyGroupId;
    std::string policyId;
};

struct Connection {
    std::string id;
    std::string nodeId;
    ConnectionSource source;
    std::vector<ConnectionDestination> destinations;
    std::vector<TransferPduConfig> transferPdus;
};

// Root Configuration Object
struct BridgeConfig {
    std::string version;
    std::map<std::string, TransferPolicy> transferPolicies;
    std::vector<Node> nodes;
    std::vector<NodeEndpoints> endpoints;
    std::vector<WireLink> wireLinks;
    std::map<std::string, std::vector<PduKey>> pduKeyGroups;
    std::vector<Connection> connections;
};

} // namespace hako::pdu::bridge


