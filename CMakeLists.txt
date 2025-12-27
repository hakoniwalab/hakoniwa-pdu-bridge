cmake_minimum_required(VERSION 3.16)
project(hakoniwa-pdu-bridge CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(include)
# This allows includes like #include "hakoniwa/pdu/endpoint.hpp"
include_directories(hakoniwa-pdu-endpoint/include)

# The submodule 'hakoniwa-pdu-endpoint' will fetch nlohmann_json.
# We add the subdirectory first, making its targets (including fetched ones) available.
add_subdirectory(hakoniwa-pdu-endpoint)

# Find all our source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add the executable
add_executable(hakoniwa-pdu-bridge ${SOURCES})

# Explicitly add the hakoniwa core include directory for this project
if(APPLE)
  target_include_directories(hakoniwa-pdu-bridge
    PRIVATE
      /usr/local/hakoniwa/include/hakoniwa
  )
endif()

# Link our bridge executable against the endpoint library and nlohmann_json.
# The 'nlohmann_json::nlohmann_json' target is available because the subdirectory fetched it.
target_link_libraries(hakoniwa-pdu-bridge PRIVATE hakoniwa_pdu_endpoint nlohmann_json::nlohmann_json)
