cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

project(MYVK)

# add dependency
find_package(glog 0.6.0 REQUIRED)
set(WITH_CUSTOM_PREFIX true)

link_directories(/usr/local/lib)

include_directories(${PROJECT_SOURCE_DIR}/include /usr/local/include)

link_libraries(vulkan glfw glog::glog)

add_subdirectory(src)
