cmake_minimum_required(VERSION 2.8.9)
project(librpmb)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER arm-xilinx-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER arm-xilinx-linux-gnueabi-g++)
set(LIBRPMB_SRC aes.c mmcwrapper.c hmac_sha2.c sha2.c)
add_definitions(-D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
include_directories(include)
add_library(rpmb STATIC ${LIBRPMB_SRC})
