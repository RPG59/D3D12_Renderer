cmake_minimum_required(VERSION 3.17)
project(D3D_12_Renderer)
set(CMAKE_CXX_STANDARD 20)

file(GLOB project_SRC "src/**.h" "src/**.cpp" "src/core/*.h" "src/core/*.cpp")

add_executable(D3D_12_Renderer WIN32 src/main.cpp src/core/D3DContext.h src/core/D3DContext.cpp src/Util.cpp src/Util.h)

add_definitions(-DUNICODE -D_UNICODE)

#include_directories(${PROJECT_SOURCE_DIR}/dep/include)

target_link_libraries(D3D_12_Renderer D3DCompiler.lib)
target_link_libraries(D3D_12_Renderer d3d12.lib)
target_link_libraries(D3D_12_Renderer dxgi.lib)
target_link_libraries(D3D_12_Renderer Dxguid.lib)


# copy shaders
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/PS.hlsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/build/shaders/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/VS.hlsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/build/shaders/)
