add_executable(client
        src/main_app.cpp
        src/client.cpp
)

target_include_directories(client
          PRIVATE
              ${PROJECT_SOURCE_DIR}
              ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(client
          PRIVATE
              genproto_lib
)
