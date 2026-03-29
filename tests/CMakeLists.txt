# tests/ — all unit and integration tests

set(TEST_SOURCES
        lib/model/ServiceNodeTest.cpp
        lib/model/HealthCheckConfigTest.cpp
        lib/model/ConnectionConfigTest.cpp
        lib/model/LoadBalancerConfigTest.cpp
        lib/serialization/YamlConfigSerializerTest.cpp
)

add_executable(bit_bridge_tests ${TEST_SOURCES})

target_include_directories(bit_bridge_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/ui
)

target_link_libraries(bit_bridge_tests PRIVATE
        GTest::gtest_main
        bitbridge_lib
        ${wxWidgets_LIBRARIES}
        tomlplusplus::tomlplusplus
)

include(GoogleTest)
gtest_discover_tests(bit_bridge_tests)