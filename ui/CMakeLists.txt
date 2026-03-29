# ui/ — wxWidgets configuration tool
add_executable(bit_bridge
    main.cpp
    BitBridgeApp.cpp
    MainFrame.cpp
    AppSettings.cpp
)

target_include_directories(bit_bridge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(bit_bridge PRIVATE
    bitbridge_lib
    ${wxWidgets_LIBRARIES}
    tomlplusplus::tomlplusplus
)