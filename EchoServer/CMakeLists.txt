cmake_minimum_required(VERSION 3.2)

project(EchoServer)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

set(EchoServer_SRC src/MyEpoll.c src/EchoServer.c)
set(EchoServerNonBlock_SRC src/MyEpoll.c src/EchoServerNonBlock.c)
set(EchoServerSelect_SRC src/EchoServerSelect.c)
set(EchoServerFork_SRC src/EchoServerFork.c)

add_executable(EchoServerEpoll ${EchoServer_SRC})
add_executable(EchoServerEpollNonBlock ${EchoServerNonBlock_SRC})
add_executable(EchoServerSelect ${EchoServerSelect_SRC})
add_executable(EchoServerFork ${EchoServerFork_SRC})

target_link_libraries(EchoServerEpoll unp)
target_link_libraries(EchoServerEpollNonBlock unp)
target_link_libraries(EchoServerSelect unp)
target_link_libraries(EchoServerFork unp)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
