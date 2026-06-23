add_executable(greeter_client
    greeter_client.cpp
)

target_include_directories(greeter_client
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(greeter_client
    PRIVATE
        genproto_lib
)