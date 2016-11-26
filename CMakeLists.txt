cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
file(GLOB cio_SRC
    "*.h"
    "*.cpp"
)
add_library(fix_slab fix_slab.h)
target_link_libraries(fix_slab rt)
add_executable(cio ${cio_SRC})