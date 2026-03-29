# core/ — TODO: Load Balancer Engine 
add_executable(bit_bridge_lb
    main.cpp
)

target_include_directories(bit_bridge_lb PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(bit_bridge_lb PRIVATE
    bitbridge_lib
)