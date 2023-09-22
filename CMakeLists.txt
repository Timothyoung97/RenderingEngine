cmake_minimum_required(VERSION 3.12)
project(RenderingEngine)

set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES "RenderingEngine/*cpp" "RenderingEngine/*h" "RenderingEngine/*hpp" "external/imgui/*h" "external/imgui/*cpp")
add_executable(RenderingEngine ${SOURCES})

target_include_directories(RenderingEngine PUBLIC "${CMAKE_CURRENT_LIST_DIR}/RenderingEngine")
target_include_directories(RenderingEngine PUBLIC "${CMAKE_CURRENT_LIST_DIR}/external/SDL2/include")
target_include_directories(RenderingEngine PUBLIC "${CMAKE_CURRENT_LIST_DIR}/external/assimp/include")
target_include_directories(RenderingEngine PUBLIC "${CMAKE_CURRENT_LIST_DIR}/external/stb/include")
target_include_directories(RenderingEngine PUBLIC "${CMAKE_CURRENT_LIST_DIR}/external/spdlog")
target_include_directories(RenderingEngine PUBLIC "${CMAKE_CURRENT_LIST_DIR}/external/imgui")

target_link_libraries(RenderingEngine ${CMAKE_CURRENT_LIST_DIR}/external/SDL2/lib/x64/SDL2.lib)
target_link_libraries(RenderingEngine ${CMAKE_CURRENT_LIST_DIR}/external/assimp/lib/assimp-vc143-mt.lib)
target_link_libraries(RenderingEngine d3d11 dxgi d3dcompiler)

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    target_compile_options(RenderingEngine PRIVATE "/MP")
endif()

target_compile_definitions(RenderingEngine PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RenderingEngine)