cmake_minimum_required(VERSION 3.2)
project(VCIO_proto)
set(GPP_COMPILE_FLAGS -g)

set(GPP_LINK_FLAGS -pthread)

set(SYS_LIBS -lrt)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GPP_COMPILE_FLAGS}" )

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GPP_LINK_FLAGS}" )

set(MAIN_FILES

  main_test.cpp

  cio.cpp

  fileDescriptor.cpp

  fix_slab.cpp

  HashManager.cpp

  InodeManager.cpp

  RegSlabManager.cpp

  regular_slab.cpp)

add_executable(main ${MAIN_FILES})

target_link_libraries(main ${SYS_LIBS})