cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PROJECT_ID delta_robot_example)

PROJECT (${PROJECT_ID})

set(PROJECT_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(SOFTWARE_SRC_PATH ${PROJECT_BASE_PATH}/src)
file(GLOB_RECURSE SOFTWARE_SRC "${SOFTWARE_SRC_PATH}/*.*")


set(USE_ASAN ON)
set(BUILD_TESTS ON)

if ((DEFINED USE_ASAN) AND (USE_ASAN STREQUAL "ON"))
    message(STATUS "Using AddressSanitizer (ASan).")
    if (UNIX)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif(UNIX)
endif()

find_package(OpenCV REQUIRED)
find_package(open62541 REQUIRED)

include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

add_subdirectory(examples)