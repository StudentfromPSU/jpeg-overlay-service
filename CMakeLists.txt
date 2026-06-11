cmake_minimum_required(VERSION 3.20)

project(grpc_server)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_DIR ${PROJECT_SOURCE_DIR}/proto)
set(GEN_DIR ${PROJECT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${GEN_DIR})

set(PROTO_FILE ${PROTO_DIR}/hello.proto)

get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

add_custom_command(
    OUTPUT
        ${GEN_DIR}/${PROTO_NAME}.pb.cc
        ${GEN_DIR}/${PROTO_NAME}.pb.h
        ${GEN_DIR}/${PROTO_NAME}.grpc.pb.cc
        ${GEN_DIR}/${PROTO_NAME}.grpc.pb.h

    COMMAND protobuf::protoc
    ARGS
        --proto_path=${PROTO_DIR}
        --cpp_out=${GEN_DIR}
        --grpc_out=${GEN_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        ${PROTO_FILE}

    DEPENDS ${PROTO_FILE}
)

add_library(genproto_lib
    ${GEN_DIR}/${PROTO_NAME}.pb.cc
    ${GEN_DIR}/${PROTO_NAME}.grpc.pb.cc
)

target_include_directories(genproto_lib
    PUBLIC ${GEN_DIR}
)

target_link_libraries(genproto_lib
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc++
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/server/server.cpp
    src/service/hello_service.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE genproto_lib
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()