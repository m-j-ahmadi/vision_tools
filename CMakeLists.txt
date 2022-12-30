# cmake needs this line
cmake_minimum_required(VERSION 3.1)
project(vision_tools VERSION 0.1.0)
include(FindPkgConfig)
find_package(PkgConfig) 

#Making sure that compiler output is always colorful
option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()

#add_subdirectory(${CMAKE_SOURCE_DIR}/dep/opencv)
set (OpenCV_DIR ${CMAKE_SOURCE_DIR}/dep/opencv/build/)
#message(STATUS "MY_VARIABLE=${CMAKE_SOURCE_DIR}")
add_subdirectory(${CMAKE_SOURCE_DIR}/dep/json)
#add_subdirectory(${CMAKE_SOURCE_DIR}/dep/boost EXCLUDE_FROM_ALL)
#set(BOOST_ROOT ${CMAKE_SOURCE_DIR}/dep/boost/)
set(BOOST_ROOT /home/mohamad/Codes/boost_1_81_0)
#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/boost_lib/)
#set(Boost_USE_MULTITHREADED ON)  
#set(Boost_USE_STATIC_RUNTIME OFF)
set (nlohmann_json_DIR ${CMAKE_SOURCE_DIR}/build/dep/json)
#set (Boost_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/buil/dep/boost/libs/)
#message(STATUS "json  dir: ${nlohmann_json_DIR}")
# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
find_package(OpenCV REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Boost REQUIRED)
if(Boost_FOUND)
    MESSAGE( STATUS " ******************************************Setting up boost." )
    MESSAGE( STATUS "---------------------  ${Boost_INCLUDE_DIRS}" )
    include_directories(${Boost_INCLUDE_DIRS})
endif()

#include_directories(${Boost_INCLUDE_DIRS})
# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them



file(GLOB vision_tools_SRC
     "*.h"
     "*.cpp"
)

list(REMOVE_ITEM vision_tools_SRC ${CMAKE_CURRENT_SOURCE_DIR}/clients/http-client.cpp)

# Declare the executable target built from your sources
add_executable(vision_tools main.cpp)
add_executable(client ${CMAKE_CURRENT_SOURCE_DIR}/clients/http-client.cpp)
# Link your application with OpenCV libraries
target_link_libraries(vision_tools PRIVATE ${OpenCV_LIBS} nlohmann_json::nlohmann_json ${Boost_LIBRARIES})
