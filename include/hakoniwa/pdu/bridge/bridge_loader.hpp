#pragma once

#include "hakoniwa/pdu/bridge/bridge_core.hpp"
#include "hakoniwa/pdu/endpoint.hpp"
#include "hakoniwa/pdu/pdu_definition.hpp"
#include <string>
#include <memory>
#include <map>

namespace hako::pdu::bridge {

class BridgeLoader {
public:
    // Loads the config from a file, uses the provided endpoints and PDU definitions,
    // and returns a fully constructed BridgeCore.
    static std::unique_ptr<BridgeCore> load(
        const std::string& config_path,
        const std::map<std::string, std::shared_ptr<hakoniwa::pdu::Endpoint>>& endpoints,
        const std::map<std::string, std::shared_ptr<hakoniwa::pdu::PduDefinition>>& pdu_definitions
    );
};

} // namespace hako::pdu::bridge
