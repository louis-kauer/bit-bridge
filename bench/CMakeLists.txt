# bench/ — C++ benchmark driver for bit_bridge_lb
find_package(Boost REQUIRED CONFIG)

add_executable(bit_bridge_bench main.cpp)

target_link_libraries(bit_bridge_bench PRIVATE Boost::boost)

target_compile_features(bit_bridge_bench PRIVATE cxx_std_23)