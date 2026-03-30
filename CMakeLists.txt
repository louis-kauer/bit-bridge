cmake_minimum_required(VERSION 3.20)
project(bit_bridge)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Shared dependency ---
find_package(yaml-cpp REQUIRED)

# --- Modules ---
add_subdirectory(lib)
add_subdirectory(core)

# UI and tests are optional — skip in CI if wxWidgets not available
find_package(wxWidgets COMPONENTS net core base)
if (wxWidgets_FOUND)
    if (wxWidgets_USE_FILE)
        include(${wxWidgets_USE_FILE})
    endif ()
    find_package(tomlplusplus REQUIRED)
    add_subdirectory(ui)
endif ()

# --- Tests (optional — skip in CI if not available) ---
enable_testing()
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
find_package(GTest QUIET)
if (GTest_FOUND)
    add_subdirectory(tests)
endif ()

# --- ASan ---
if (ENABLE_ASAN)
    foreach (target bit_bridge bit_bridge_lb bit_bridge_tests)
        if (TARGET ${target})
            target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address)
        endif ()
    endforeach ()
endif ()