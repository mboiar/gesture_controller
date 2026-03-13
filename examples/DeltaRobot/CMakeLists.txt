set(NAME DeltaRobotExample)

add_executable(${NAME})

target_sources(${NAME} PRIVATE
    ${SOFTWARE_SRC}
)

target_include_directories(${NAME} PRIVATE ${SOFTWARE_SRC_PATH})
target_compile_definitions(${NAME} PRIVATE UA_ENABLE_AMALGAMATION)

target_include_directories(${NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
link_directories( ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(${NAME} PUBLIC ${OpenCV_LIBS} spdlog::spdlog)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
