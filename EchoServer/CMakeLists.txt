cmake_minimum_required(VERSION 3.2)

project(EchoServer)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

set(EchoServer_SRC src/MyEpoll.c src/EchoServer.c)
set(EchoServerNonBlock_SRC src/MyEpoll.c src/EchoServerNonBlock.c)

add_executable(EchoServerEpoll ${EchoServer_SRC})
add_executable(EchoServerEpollNonBlock ${EchoServerNonBlock_SRC})


target_link_libraries(EchoServerEpoll unp)
target_link_libraries(EchoServerEpollNonBlock unp)

