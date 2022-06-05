cmake_minimum_required(VERSION 3.17)

if(DEFINED ENV{CMAKE_PACKAGES} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "!!!CMAKE_PACKAGES NOT set! $ENV{CMAKE_PACKAGES}")
  set(CMAKE_TOOLCHAIN_FILE "$ENV{CMAKE_PACKAGES}"
    CACHE STRING ""
  )

else()
  message(STATUS "!!!CMAKE_PACKAGES set! ${CMAKE_TOOLCHAIN_FILE}")
endif()


project(D3D_12_Renderer)
set(CMAKE_CXX_STANDARD 20)

find_package(assimp CONFIG REQUIRED)

# file(GLOB project_SRC "src/**.h" "src/**.cpp" "src/core/*.h" "src/core/*.cpp")



add_executable(D3D_12_Renderer WIN32 
  src/main.cpp 
  src/Util.cpp
  src/core/D3DContext.cpp 
  src/core/GpuBuffer.cpp 
  src/core/UploadBuffer.cpp
  src/core/SwapChain.cpp
  src/core/SwapChainBuffer.cpp
  src/core/Display.cpp
  src/core/DepthBuffer.cpp
)

add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)

#include_directories(${PROJECT_SOURCE_DIR}/dep/include)

target_link_libraries(D3D_12_Renderer D3DCompiler.lib)
target_link_libraries(D3D_12_Renderer d3d12.lib)
target_link_libraries(D3D_12_Renderer dxgi.lib)
target_link_libraries(D3D_12_Renderer Dxguid.lib)
target_link_libraries(D3D_12_Renderer assimp::assimp)
# target_link_libraries(D3D_12_Renderer PRIVATE assimp::assimp)
# -DCMAKE_TOOLCHAIN_FILE=C:/Users/rpg59/dev/cpp/packages/vcpkg/scripts/buildsystems/vcpkg.cmake"

INCLUDE_DIRECTORIES(
  C:/Users/rpg59/dev/cpp/packages/vcpkg/installed/x64-windows/include/
)

# copy shaders
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/PS.hlsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/VS.hlsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders/)


