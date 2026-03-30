# lib/ — shared model + serialization library

add_library(bitbridge_lib STATIC
        model/ServiceNode.cpp
        model/HealthCheckConfig.cpp
        model/ConnectionConfig.cpp
        model/LoadBalancerConfig.cpp
        serialization/YamlConfigSerializer.cpp
)

target_include_directories(bitbridge_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/model
        ${CMAKE_CURRENT_SOURCE_DIR}/serialization
)

target_link_libraries(bitbridge_lib PUBLIC
        yaml-cpp::yaml-cpp
)