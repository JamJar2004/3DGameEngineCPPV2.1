cmake_minimum_required(VERSION 4.0)
project(3DGameEngineCPPV2_1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/Engine/*.cpp"
)

file(GLOB_RECURSE ECS_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/ECS/*.cpp"
)

file(GLOB_RECURSE OPENGL_RENDERER_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/OpenGLRenderer/*.cpp"
)

add_library(EngineLib STATIC
        ${ENGINE_SOURCES}
        ${OPENGL_RENDERER_SOURCES}
        ${ECS_SOURCES}
)

target_include_directories(EngineLib PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
)

add_executable(3DGameEngineCPPV2_1 src/NightmareMaze/Main.cpp)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

target_link_libraries(EngineLib PRIVATE assimp::assimp SDL2::SDL2 SDL2_mixer GLEW::GLEW OpenGL::GL)
target_link_libraries(3DGameEngineCPPV2_1 PUBLIC EngineLib)