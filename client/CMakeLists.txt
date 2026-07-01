add_executable(client
    src/main.cpp
)

target_include_directories(client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(client
    PRIVATE
        genproto_lib
)