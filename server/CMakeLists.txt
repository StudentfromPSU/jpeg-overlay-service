add_executable(grpc_server
    src/main.cpp
    src/server.cpp
)

target_include_directories(grpc_server
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(grpc_server
    PRIVATE
        genproto_lib
        image_processor
)