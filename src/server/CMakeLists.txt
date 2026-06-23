add_executable(grpc_server
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    server.cpp
)

target_include_directories(grpc_server
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(grpc_server
    PRIVATE
        genproto_lib
)