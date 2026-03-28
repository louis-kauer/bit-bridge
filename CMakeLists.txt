cmake_minimum_required(VERSION 3.20)
project(bit_bridge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- wxWidgets ---
find_package(wxWidgets REQUIRED COMPONENTS net core base)
if (wxWidgets_USE_FILE)
    include(${wxWidgets_USE_FILE})
endif ()

# --- yaml-cpp ---
find_package(yaml-cpp REQUIRED)

# --- tomlplusplus ---
find_package(tomlplusplus REQUIRED)

# --- Source files (without main.cpp for reuse in tests) ---
set(LIB_SOURCES
        src/model/ServiceNode.cpp
        src/model/HealthCheckConfig.cpp
        src/model/ConnectionConfig.cpp
        src/model/LoadBalancerConfig.cpp
        src/serialization/YamlConfigSerializer.cpp
        src/ui/AppSettings.cpp
        src/ui/BitBridgeApp.cpp
        src/ui/MainFrame.cpp
)

# --- Executable ---
add_executable(bit_bridge src/main.cpp ${LIB_SOURCES})

target_include_directories(bit_bridge PRIVATE
        ${CMAKE_SOURCE_DIR}/src/model
        ${CMAKE_SOURCE_DIR}/src/serialization
        ${CMAKE_SOURCE_DIR}/src/ui
)

target_link_libraries(bit_bridge PRIVATE
        ${wxWidgets_LIBRARIES}
        yaml-cpp::yaml-cpp
        tomlplusplus::tomlplusplus
)

# --- Tests ---
enable_testing()
find_package(GTest REQUIRED)

option(ENABLE_ASAN "Enable AddressSanitizer for tests" OFF)

set(TEST_SOURCES
        tests/model/ServiceNodeTest.cpp
        tests/model/HealthCheckConfigTest.cpp
        tests/model/ConnectionConfigTest.cpp
        tests/model/LoadBalancerConfigTest.cpp
        tests/serialization/YamlConfigSerializerTest.cpp
)

add_executable(bit_bridge_tests ${TEST_SOURCES} ${LIB_SOURCES})

target_include_directories(bit_bridge_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/model
        ${CMAKE_SOURCE_DIR}/src/serialization
        ${CMAKE_SOURCE_DIR}/src/ui
)

target_link_libraries(bit_bridge_tests PRIVATE
        GTest::gtest_main
        ${wxWidgets_LIBRARIES}
        yaml-cpp::yaml-cpp
        tomlplusplus::tomlplusplus
)

include(GoogleTest)
gtest_discover_tests(bit_bridge_tests)

if (ENABLE_ASAN)
    target_compile_options(bit_bridge PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(bit_bridge PRIVATE -fsanitize=address)
    target_compile_options(bit_bridge_tests PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(bit_bridge_tests PRIVATE -fsanitize=address)
endif ()