cmake_minimum_required(VERSION 2.9) 
PROJECT (drone-gesture-control)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
find_package(OpenCV REQUIRED PATHS ~/build/ NO_DEFAULT_PATH)
set( MAIN_SRC
    src/main.cpp    
)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include )
include_directories( ${OpenCV_INCLUDE_DIRS} )
link_directories( ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable( main ${MAIN_SRC} )

target_link_libraries( main ${OpenCV_LIBS} )