cmake_minimum_required(VERSION 3.16)
project(kvstore)
set(CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX ../..)
include_directories(../../thirdparty)
include_directories(../include)
set(SRC_LIST main.cc app.cc config.cc fileio.cc)
add_executable(kvstore ${SRC_LIST})
target_link_directories(kvstore PRIVATE ../lib)
target_link_libraries(kvstore PUBLIC reactor yaml-cpp spdlog)
install(TARGETS kvstore DESTINATION sbin)


