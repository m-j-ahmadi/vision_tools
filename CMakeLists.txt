# cmake needs this line
cmake_minimum_required(VERSION 3.1)
project(vision_tools VERSION 0.1.0)
include(ExternalProject)  # Include ExternalProject module
include(FindPkgConfig)
find_package(PkgConfig)

set(OPENCV_BIN_INSTALL_PATH ${PROJECT_SOURCE_DIR}/build/opencv)
# Set paths for submodules
set(OPENCV_PATH ${PROJECT_SOURCE_DIR}/dep/opencv)
# set(JSON_PATH ${PROJECT_SOURCE_DIR}/dep/json)

# Add subdirectories for submodules
add_subdirectory(${OPENCV_PATH})
# add_subdirectory(${JSON_PATH})

# Set the path to OpenCVConfig.cmake
set(OpenCV_DIR ${PROJECT_SOURCE_DIR}/build/)
find_package(OpenCV REQUIRED)

if(OpenCV_FOUND)
    message(STATUS "OpenCV library found: ${OpenCV_VERSION}")
    message(STATUS "OpenCV include path: ${OpenCV_INCLUDE_DIRS}")
    include_directories(${OpenCV_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "OpenCV library not found")
endif()

set(BOOST_PATH ${PROJECT_SOURCE_DIR}/dep/boost)
add_subdirectory(${BOOST_PATH})

set(Boost_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dep/boost/libs/config/include)
set(BOOST_LIBRARYDIR ${BOOST_PATH}/stage/lib)

# Find and include Boost
find_package(Boost REQUIRED)
if(Boost_FOUND)
    message(STATUS "Boost library found")
    message(STATUS "Boost include path: ${Boost_INCLUDE_DIRS}")
    # include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost library not found")
endif()


# Gather source files
file(GLOB vision_tools_SRC "*.h" "*.cpp")
list(REMOVE_ITEM vision_tools_SRC ${CMAKE_CURRENT_SOURCE_DIR}/clients/http-client.cpp)

# Declare the executable targets built from your sources
add_executable(vision_tools main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/servers/sync-server.cpp)
add_executable(client ${CMAKE_CURRENT_SOURCE_DIR}/clients/http-client.cpp)

# Link your application with OpenCV, , and Boost libraries
target_link_libraries(vision_tools PRIVATE ${OpenCV_LIBS}  ${Boost_LIBRARIES})
target_link_libraries(client PRIVATE ${OpenCV_LIBS}  ${Boost_LIBRARIES})
