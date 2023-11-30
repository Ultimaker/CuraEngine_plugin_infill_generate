cmake_minimum_required(VERSION 3.23)
project(curaengine_plugin_infill_generate)

find_package(curaengine_grpc_definitions REQUIRED)
find_package(asio-grpc REQUIRED)
find_package(Boost REQUIRED)
find_package(spdlog REQUIRED)
find_package(docopt REQUIRED)
find_package(range-v3 REQUIRED)
find_package(clipper REQUIRED)
find_package(ctre REQUIRED)
find_package(semver REQUIRED)

add_executable(curaengine_plugin_infill_generate src/main.cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_include_directories(curaengine_plugin_infill_generate
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(curaengine_plugin_infill_generate PUBLIC asio-grpc::asio-grpc curaengine_grpc_definitions::curaengine_grpc_definitions boost::boost clipper::clipper ctre::ctre spdlog::spdlog docopt_s range-v3::range-v3 semver::semver)

