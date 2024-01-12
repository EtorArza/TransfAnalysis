cmake_minimum_required(VERSION 2.8)

project(disc_analysis)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#Build command line app?
option (BUILD_TEST "Build normal app" OFF)
if(BUILD_TEST)
  add_definitions(-DBUILD_TEST)
endif()

# Set output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# mixmod
#include_directories(/usr/local/include)
#include_directories(/Users/toand/git/mivp/projects/instancespace/tools/libmixmod_3.2.2/INSTALL/include)
#link_directories(/Users/toand/git/mivp/projects/instancespace/tools/libmixmod_3.2.2/INSTALL/lib)

set(ALL_LIBS
    -lmixmod
    -lmixmod_newmat
  )

# Executable
add_executable(discLoss discLoss_mex.cpp)
target_link_libraries(discLoss ${ALL_LIBS})