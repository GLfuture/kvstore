cmake_minimum_required(VERSION 3.6)
project(kvstore-cli)
set(CMAKE_INSTALL_PREFIX ../..)
set(SRC_LIST kvstore-cli.cc config.cc)
include_directories(../include)
include_directories(../../thirdparty)
add_executable(kvstore-cli ${SRC_LIST})


target_link_directories(kvstore-cli PRIVATE ../lib)
target_link_libraries(kvstore-cli PUBLIC yaml-cpp spdlog)

install(TARGETS kvstore-cli DESTINATION sbin)