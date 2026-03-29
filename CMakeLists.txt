cmake_minimum_required(VERSION 3.20)
project(bit_bridge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies ---
find_package(wxWidgets REQUIRED COMPONENTS net core base)
if (wxWidgets_USE_FILE)
    include(${wxWidgets_USE_FILE})
endif ()

find_package(yaml-cpp REQUIRED)
find_package(tomlplusplus REQUIRED)

# --- Modules ---
add_subdirectory(lib)
add_subdirectory(ui)
add_subdirectory(core)

# --- Tests ---
enable_testing()
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
add_subdirectory(tests)

# --- ASan ---
if (ENABLE_ASAN)
    foreach (target bit_bridge bit_bridge_lb bit_bridge_tests)
        if (TARGET ${target})
            target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address)
        endif ()
    endforeach ()
endif ()