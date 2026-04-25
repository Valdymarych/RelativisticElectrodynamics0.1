cmake_minimum_required(VERSION 3.15)
project(PhysicsApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML COMPONENTS Graphics Window System Audio REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ImGui-SFML REQUIRED)

add_executable(MyApp base.cpp)

target_link_libraries(MyApp PRIVATE 
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    SFML::Audio
    GLEW::GLEW
    OpenGL::GL
    ImGui-SFML::ImGui-SFML
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")