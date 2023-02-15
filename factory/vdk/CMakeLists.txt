cmake_minimum_required(VERSION 2.8.9)

add_subdirectory(librpmb)
add_subdirectory(test)

project(libvdk)
set(LIB_SRC src/RecoverySystem.cpp src/Build.cpp)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER arm-xilinx-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER arm-xilinx-linux-gnueabi-g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
include_directories(include)
include_directories(librpmb/include)
link_libraries(rpmb)
add_library(vdk SHARED ${LIB_SRC})

set(TARGET_CROSS_SYSTEM_DIR "${PROJECT_SOURCE_DIR}/../../system")
install(TARGETS vdk DESTINATION ${TARGET_CROSS_SYSTEM_DIR}/usr/lib)
