cmake_minimum_required(VERSION 3.23)
project(curaengine_plugin_infill_generate)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

find_package(curaengine_grpc_definitions REQUIRED)
find_package(asio-grpc REQUIRED)
find_package(Boost REQUIRED)
find_package(spdlog REQUIRED)
find_package(docopt REQUIRED)
find_package(range-v3 REQUIRED)
find_package(clipper REQUIRED)
find_package(ctre REQUIRED)
find_package(neargye-semver REQUIRED)
find_package(standardprojectsettings REQUIRED)
add_executable(curaengine_plugin_infill_generate src/main.cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if( supported )
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET curaengine_plugin_infill_generate PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${error}>")
endif()

set(HDRS include/plugin/broadcast.h
        include/plugin/cmdline.h
        include/plugin/handshake.h
        include/plugin/metadata.h
        include/plugin/generate.h
        include/plugin/plugin.h
        include/plugin/settings.h)

add_library(curaengine_plugin_infill_generate_lib INTERFACE ${HDRS})

use_threads(curaengine_plugin_infill_generate_lib)
target_link_libraries(curaengine_plugin_infill_generate_lib INTERFACE asio-grpc::asio-grpc curaengine_grpc_definitions::curaengine_grpc_definitions boost::boost clipper::clipper ctre::ctre spdlog::spdlog docopt_s range-v3::range-v3 neargye-semver::neargye-semver)
target_include_directories(curaengine_plugin_infill_generate_lib
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(curaengine_plugin_infill_generate PUBLIC curaengine_plugin_infill_generate_lib)

