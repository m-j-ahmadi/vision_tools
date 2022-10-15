# cmake needs this line
cmake_minimum_required(VERSION 3.1)
set (OpenCV_DIR ${CMAKE_SOURCE_DIR}/dep/opencv)
#message(STATUS "MY_VARIABLE=${CMAKE_SOURCE_DIR}")
add_subdirectory(${CMAKE_SOURCE_DIR}/dep/json)
# Define project name
project(vision_tools)

set (nlohmann_json_DIR ${CMAKE_SOURCE_DIR}/build/dep/json)
#message(STATUS "json  dir: ${nlohmann_json_DIR}")
# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# Declare the executable target built from your sources
add_executable(vision_tools main.cpp)

# Link your application with OpenCV libraries
target_link_libraries(vision_tools PRIVATE ${OpenCV_LIBS} nlohmann_json::nlohmann_json)
