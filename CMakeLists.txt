cmake_minimum_required(VERSION 3.30)

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

# find_package(spdlog REQUIRED)
# find_package(OpenCV REQUIRED PATHS /usr/local/debug/)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(OpenCV_DIR "C:/Users/boiar/opencv_build_release/install")
    set(open62541_DIR "C:/Users/boiar/open62541/INSTALL_RELEASE/lib/cmake/open62541")
else()
    set(OpenCV_DIR "C:/opencv_build_debug/install")
    set(open62541_DIR "C:/Users/boiar/open62541/out/install/x64-Debug/lib/cmake/open62541")
endif()

find_package(OpenCV REQUIRED)
find_package(open62541 REQUIRED)


set( MAIN_SRC
    src/main.cpp
    src/controller.cpp
    src/device.cpp
    src/face_detection.cpp
    src/gesture_detection.cpp
        src/detection.cpp
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

include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)


add_executable( ${MAIN_EXEC} ${MAIN_SRC} )
# target_compile_options(${MAIN_EXEC} PRIVATE -Werror -Wall -Wextra)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS} ${open62541_INCLUDE_DIRS})
link_directories( ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(${MAIN_EXEC} PUBLIC ${OpenCV_LIBS} spdlog::spdlog open62541::open62541)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

