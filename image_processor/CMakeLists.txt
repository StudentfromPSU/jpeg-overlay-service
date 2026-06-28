cmake_minimum_required(VERSION 3.20)

project(image_processor)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(OpenCV REQUIRED)


set(SOURCES
    src/ImageValidator.cpp 
    src/ImageCodec.cpp
    src/CaptionDrawer.cpp
    src/ImageProcessor.cpp
)

set(HEADERS
    include/image_processor/Exceptions.h
    include/image_processor/ImageValidator.h
    include/image_processor/ImageCodec.h
    include/image_processor/CaptionDrawer.h
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

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_SOURCES
    tests/ImageValidatorTests.cpp
    tests/ImageCodecTests.cpp
    tests/CaptionDrawerTests.cpp
    tests/ImageProcessorTests.cpp
)

add_executable(image_processor_tests
    ${TEST_SOURCES}
)

target_link_libraries(image_processor_tests
    PRIVATE
        image_processor
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(image_processor_tests)

add_custom_command(TARGET image_processor_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_images
        $<TARGET_FILE_DIR:image_processor_tests>/test_images
)