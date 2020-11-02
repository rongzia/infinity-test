cmake_minimum_required(VERSION 3.14)
project(rdma C CXX)


SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE INTERNAL "compiler options" FORCE)
SET(CMAKE_CXX_FLAGS_RELEASE "-O0 -g" CACHE INTERNAL "compiler options" FORCE)

include_directories(include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR})


add_executable(server1 test/server1.cpp)
add_executable(client1 test/client1.cpp)
target_link_libraries(server1 libinfinity.a infinity ibverbs)
target_link_libraries(client1 libinfinity.a infinity ibverbs)

add_executable(read-write test/read-write.cpp)
target_link_libraries(read-write libinfinity.a infinity ibverbs)