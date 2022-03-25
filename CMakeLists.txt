cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

project(VK)

link_directories(/usr/local/lib)

include_directories(${PROJECT_SOURCE_DIR}/include /usr/local/include)

link_libraries(vulkan glfw)

add_subdirectory(src)
