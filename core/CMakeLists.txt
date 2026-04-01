# core/ — Load Balancer Engine
find_package(Boost REQUIRED CONFIG)

add_executable(bit_bridge_lb
        main.cpp
        state/ServiceState.cpp
        state/ServicePool.cpp
        routing/Xoshiro256.cpp
        routing/P2CStrategy.cpp
        routing/ConsistentHashStrategy.cpp
        proxy/Session.cpp
        proxy/TcpProxy.cpp
        health/HealthChecker.cpp
)

target_include_directories(bit_bridge_lb PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/state
        ${CMAKE_CURRENT_SOURCE_DIR}/routing
        ${CMAKE_CURRENT_SOURCE_DIR}/health
        ${CMAKE_CURRENT_SOURCE_DIR}/proxy
)

target_link_libraries(bit_bridge_lb PRIVATE
        bitbridge_lib
        Boost::boost
)