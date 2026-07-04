cmake_minimum_required(VERSION 3.20)

project(image_processor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED)

set(SOURCES
    src/ImageProcessor.cpp
)

set(HEADERS
    include/image_processor/Exceptions.h
    include/image_processor/ImageProcessor.h
)

add_library(image_processor STATIC)

target_sources(image_processor
    PRIVATE
        ${SOURCES}
        ${HEADERS}
)

target_include_directories(image_processor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(image_processor
    PUBLIC
        ${OpenCV_LIBS}
)

if(MSVC)
    target_compile_options(image_processor PRIVATE /W4)
else()
    target_compile_options(image_processor PRIVATE -Wall -Wextra -Wpedantic)
endif()