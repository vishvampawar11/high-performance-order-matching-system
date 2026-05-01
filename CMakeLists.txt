cmake_minimum_required(VERSION 3.15)

project(HighPerformanceOrderMatchingSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SOURCES
    src/*.cpp
)

set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_dx11.cpp
    imgui/backends/imgui_impl_win32.cpp
)

list(APPEND SOURCES ${IMGUI_SOURCES})

add_executable(order_matching_system ${SOURCES})

find_package(PostgreSQL REQUIRED)

find_path(PQXX_INCLUDE_DIR pqxx/pqxx)
find_library(PQXX_LIBRARY pqxx)

target_include_directories(order_matching_system PRIVATE
    ${PQXX_INCLUDE_DIR}
    ${PostgreSQL_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/imgui
    ${PROJECT_SOURCE_DIR}/imgui/backends
)

# Link libraries (KEYWORD SIGNATURE — consistent)
target_link_libraries(order_matching_system PRIVATE
    ${PQXX_LIBRARY}
    ${PostgreSQL_LIBRARIES}
)

if (WIN32)
    target_link_libraries(order_matching_system PRIVATE ws2_32 d3d11 dxgi d3dcompiler dwmapi)
endif()

set_target_properties(order_matching_system PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build
)
