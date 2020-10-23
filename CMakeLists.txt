cmake_minimum_required(VERSION 3.17)
project(D3D_12_Renderer)
set(CMAKE_CXX_STANDARD 20)

file(GLOB project_SRC "src/**.h" "src/**.cpp" "src/core/*.h" "src/core/*.cpp")

add_executable(D3D_12_Renderer WIN32 ${project_SRC} src/Util.h src/Util.cpp src/core/Scene.cpp src/core/Scene.h)

add_definitions(-DUNICODE -D_UNICODE)

#include_directories(${PROJECT_SOURCE_DIR}/dep/include)

target_link_libraries(D3D_12_Renderer d3d12.lib)
target_link_libraries(D3D_12_Renderer dxgi.lib)