cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

project(VK)

# add dependency
include(FetchContent)
FetchContent_Declare(
  glog
  GIT_REPOSITORY https://github.com/google/glog.git
  GIT_TAG v0.5.0
)

set(WITH_CUSTOM_PREFIX true)

FetchContent_MakeAvailable(glog)

link_directories(/usr/local/lib)

include_directories(${PROJECT_SOURCE_DIR}/include /usr/local/include)

link_libraries(vulkan glfw glog)

add_subdirectory(src)
