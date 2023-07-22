cmake_minimum_required(VERSION 3.0)

set(PROJECT_ID gesture_controller)
PROJECT (${PROJECT_ID})

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# add_compile_options(-Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wconversion)

set(USE_ASAN ON)
set(BUILD_TESTS ON)

if ((DEFINED USE_ASAN) AND (USE_ASAN STREQUAL "ON"))
    message(STATUS "Using AddressSanitizer (ASan).")
    if (UNIX)
        # message(STATUS "ASAN_OPTIONS = $ENV{ASAN_OPTIONS}")
        # set(ENV{ASAN_OPTIONS} allocator_may_return_null=1)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif(UNIX)
endif()

find_package(spdlog REQUIRED)
find_package(OpenCV REQUIRED)

set( MAIN_SRC
    src/main.cpp
    src/controller.cpp
    src/drone.cpp
    src/face_detection.cpp
    src/gesture_detection.cpp
)
set(MAIN_EXEC ${PROJECT_ID}__main)

# cmrc_add_resource_library(models ALIAS models::rc NAMESPACE models
#     resources/models/haarcascade_frontalface_default.xml
#     resources/models/resnet18.onnx
# )

# cmrc_add_resource_library(test_res ALIAS test_res::rc NAMESPACE test_res
#     test/data/test.png
#     test/data/test_face.jpg
#     resources/models/haarcascade_frontalface_default.xml
#     resources/models/resnet18.onnx
# )

add_executable( ${MAIN_EXEC} ${MAIN_SRC} )
target_link_libraries(${MAIN_EXEC} PUBLIC ${OpenCV_LIBS} spdlog::spdlog)
target_compile_options(${MAIN_EXEC} PRIVATE -Werror -Wall -Wextra)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
link_directories( ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if ((DEFINED BUILD_TESTS) AND (BUILD_TESTS STREQUAL "ON"))
    include(FetchContent)
    FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()

    set(SOURCES_FILES_TESTS
        test/test_gesture_detection.cpp
        test/test_face_detection.cpp
        )
    set( TEST_SRC
        src/controller.cpp
        src/drone.cpp
        src/face_detection.cpp
        src/gesture_detection.cpp
    )
    set(EXEC_TEST ${PROJECT_ID}__test)
    add_executable(${EXEC_TEST} ${TEST_SRC} ${SOURCES_FILES_TESTS} test/main_gtest.cpp)
    target_link_libraries(${EXEC_TEST} PRIVATE GTest::gtest_main ${OpenCV_LIBS} spdlog::spdlog)
    include(GoogleTest)
    gtest_discover_tests(${EXEC_TEST})
endif()