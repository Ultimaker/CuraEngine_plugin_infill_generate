cmake_minimum_required(VERSION 3.25)
project(curaengine_plugin_infill_generate)

find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)
find_package(docopt REQUIRED)
find_package(asio-grpc REQUIRED)
find_package(range-v3 REQUIRED)
find_package(clipper REQUIRED)
find_package(ctre REQUIRED)

asio_grpc_protobuf_generate(PROTOS "${GRPC_PROTOS}"
        IMPORT_DIRS ${GRPC_IMPORT_DIRS}
        OUT_VAR "ASIO_GRPC_PLUGIN_PROTO_SOURCES"
        OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
        GENERATE_GRPC GENERATE_MOCK_CODE)

add_executable(curaengine_plugin_infill_generate ${PROTO_SRCS} ${ASIO_GRPC_PLUGIN_PROTO_SOURCES} src/main.cpp)

target_include_directories(curaengine_plugin_infill_generate
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        ${CMAKE_CURRENT_BINARY_DIR}/generated
        )

target_link_libraries(curaengine_plugin_infill_generate PUBLIC clipper::clipper ctre::ctre asio-grpc::asio-grpc protobuf::libprotobuf boost::boost spdlog::spdlog docopt_s range-v3::range-v3)

