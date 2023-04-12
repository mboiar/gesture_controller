cmake_minimum_required(VERSION 3.0) 
PROJECT (drone-gesture-controller)
# set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)

# add_library(
#     Controller
#     src/controller.cpp
#     src/drone.cpp
#     src/face_detection.cpp
#     src/gesture_detection.cpp
# )
# target_include_directories(ControllerLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set( MAIN_SRC
    src/main.cpp
    src/controller.cpp
    src/drone.cpp
    src/face_detection.cpp
    src/gesture_detection.cpp
)
add_executable( drone_gesture_controller ${MAIN_SRC} )
# target_link_libraries( drone_gesture_controller LINK_PUBLIC ControllerLib)
target_link_libraries(drone_gesture_controller PUBLIC ${OpenCV_LIBS} spdlog::spdlog)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
link_directories( ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/Release)
