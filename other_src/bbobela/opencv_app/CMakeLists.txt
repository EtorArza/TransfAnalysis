cmake_minimum_required(VERSION 2.8)

project(lda_dtree)

#OpenCV
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

#OpenMP
option (USE_OpenMP "Use OpenMP" OFF)
if(USE_OpenMP)
  find_package(OpenMP REQUIRED)
  if(OPENMP_FOUND)
    add_definitions(-DUSE_OpenMP)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

#Build command line app?
option (BUILD_TEST "Build normal app" OFF)
if(BUILD_TEST)
  add_definitions(-DBUILD_TEST)
endif()

# Set output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(ALL_LIBS
    ${OpenCV_LIBS}
  )

# Executable
add_executable(dtreeLoss dtreeLoss_mex.cpp)
target_link_libraries(dtreeLoss ${ALL_LIBS})