cmake_minimum_required(VERSION 3.15)
project(PhysicsApp)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. ШУКАЄМО БІБЛІОТЕКИ (vcpkg підставить їх сам)
find_package(SFML COMPONENTS Graphics Window System Audio REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ImGui-SFML REQUIRED)

# 2. ОПИСУЄМО ВИКОНУВАНИЙ ФАЙЛ
add_executable(MyApp base.cpp)

# 3. ПІДКЛЮЧАЄМО БІБЛІОТЕКИ ДО ПРОЄКТУ
target_link_libraries(MyApp PRIVATE 
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    SFML::Audio
    GLEW::GLEW
    OpenGL::GL
    ImGui-SFML::ImGui-SFML
)

# 4. КОПІЮВАННЯ РЕСУРСІВ (Шейдери, шрифти)
# Це важливо: щоб при запуску програма бачила свої файли у папці build
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")