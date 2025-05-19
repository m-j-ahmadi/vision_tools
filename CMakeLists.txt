# cmake needs this line
cmake_minimum_required(VERSION 3.1)
project(vision_tools VERSION 0.1.0)
include(ExternalProject)  # Include ExternalProject module
include(FindPkgConfig)
find_package(PkgConfig)

set(NLOHMANN_JSON_PATH ${PROJECT_SOURCE_DIR}/dep/json)

#Add nlohmann json lib to subdirectories
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${NLOHMANN_JSON_PATH})

find_package(OpenCV REQUIRED)

if(OpenCV_FOUND)
    message(STATUS "OpenCV library found: ${OpenCV_VERSION}")
    message(STATUS "OpenCV include path: ${OpenCV_INCLUDE_DIRS}")
    include_directories(${OpenCV_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "OpenCV library not found")
endif()


find_package(Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})

set(CPPCODEC_PATH ${PROJECT_SOURCE_DIR}/dep/cppcodec)
include_directories(${CPPCODEC_PATH})

if(Boost_FOUND)
    message(STATUS "Boost library found")   
   
else()
    message(FATAL_ERROR "Boost library not found")
endif()


# Gather source files
file(GLOB vision_tools_SRC "*.h" "*.cpp")
list(REMOVE_ITEM vision_tools_SRC ${CMAKE_CURRENT_SOURCE_DIR}/clients/http-client.cpp)

# Declare the executable targets built from your sources
add_executable(vision_tools main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/servers/sync-server.cpp  ${CMAKE_CURRENT_SOURCE_DIR}/servers/image-processor.cpp)
add_executable(client ${CMAKE_CURRENT_SOURCE_DIR}/clients/http-client.cpp)

# Link your application with OpenCV, , and Boost libraries
target_link_libraries(vision_tools PRIVATE ${OpenCV_LIBS}  ${Boost_LIBRARIES}  nlohmann_json::nlohmann_json)
target_link_libraries(client PRIVATE ${OpenCV_LIBS}  ${Boost_LIBRARIES} nlohmann_json::nlohmann_json)
