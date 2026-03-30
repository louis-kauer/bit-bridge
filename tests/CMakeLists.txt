# tests/ — all unit and integration tests
find_package(Boost REQUIRED CONFIG)

set(CORE_TEST_SOURCES
        core/state/ServiceStateTest.cpp
        core/state/ServicePoolTest.cpp
        core/routing/P2CStrategyTest.cpp
        core/routing/ConsistentHashStrategyTest.cpp
        core/proxy/SessionTest.cpp
        core/proxy/TcpProxyTest.cpp
        core/health/HealthCheckerTest.cpp
)

set(LIB_TEST_SOURCES
        lib/model/ServiceNodeTest.cpp
        lib/model/HealthCheckConfigTest.cpp
        lib/model/ConnectionConfigTest.cpp
        lib/model/LoadBalancerConfigTest.cpp
        lib/serialization/YamlConfigSerializerTest.cpp
)

set(CORE_SOURCES
        ${CMAKE_SOURCE_DIR}/core/state/ServiceState.cpp
        ${CMAKE_SOURCE_DIR}/core/state/ServicePool.cpp
        ${CMAKE_SOURCE_DIR}/core/routing/Xoshiro256.cpp
        ${CMAKE_SOURCE_DIR}/core/routing/P2CStrategy.cpp
        ${CMAKE_SOURCE_DIR}/core/routing/ConsistentHashStrategy.cpp
        ${CMAKE_SOURCE_DIR}/core/proxy/Session.cpp
        ${CMAKE_SOURCE_DIR}/core/proxy/TcpProxy.cpp
        ${CMAKE_SOURCE_DIR}/core/health/HealthChecker.cpp
)

add_executable(bit_bridge_tests
        ${LIB_TEST_SOURCES}
        ${CORE_TEST_SOURCES}
        ${CORE_SOURCES}
)

target_include_directories(bit_bridge_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/core/state
        ${CMAKE_SOURCE_DIR}/core/routing
        ${CMAKE_SOURCE_DIR}/core/health
        ${CMAKE_SOURCE_DIR}/core/proxy
)

target_link_libraries(bit_bridge_tests PRIVATE
        GTest::gtest_main
        bitbridge_lib
        Boost::boost
)

include(GoogleTest)
gtest_discover_tests(bit_bridge_tests)