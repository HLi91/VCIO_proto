cmake_minimum_required(VERSION 3.2)
project(VCIO_proto)
set(GPP_COMPILE_FLAGS -g)

set(GPP_LINK_FLAGS -pthread)

set(SYS_LIBS -lrt)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GPP_COMPILE_FLAGS}" )

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GPP_LINK_FLAGS}" )

file(GLOB MAIN_FILES
    "*.h"
    "*.cpp"
)

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
 
# Link runTests with what we want to test and the GTest and pthread library
add_executable(runTests tests.cpp ${MAIN_FILES})
target_link_libraries(runTests  ${GTEST_LIBRARIES} ${SYS_LIBS} pthread)
