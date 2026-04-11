cmake_minimum_required(VERSION 3.10)
project(SFML_Physics_App)

# Вимикаємо попередження про OpenGL (CMP0072)
set(CMAKE_POLICY_DEFAULT_CMP0072 NEW)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Пошук пакетів
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# 2. Список файлів для ImGui (ядро + місток)
# Припускаємо, що imgui-SFML.cpp лежить у корені, а ядро - у папці imgui/
set(IMGUI_SOURCES 
    imgui/imgui.cpp 
    imgui/imgui_draw.cpp 
    imgui/imgui_widgets.cpp 
    imgui/imgui_tables.cpp
    imgui-SFML.cpp
)

# 3. Створення виконуваного файлу
# Додаємо base.cpp ТА всі файли ImGui
add_executable(MyApp base.cpp ${IMGUI_SOURCES})

# 4. Шляхи до заголовних файлів (.h)
# Додаємо корінь проекту та папку imgui, щоб #include "imgui.h" працював
target_include_directories(MyApp PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${GLEW_INCLUDE_DIRS}
)

# 5. Копіювання шейдерів
# Це перенесе папку shaders у build/shaders при кожній збірці
set(SHADERS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADERS_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

# Команда для копіювання всієї папки
file(COPY ${SHADERS_SOURCE_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf" 
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
# 6. Лінкування бібліотек
# Використовуємо сучасні назви мішеней (targets)
target_link_libraries(MyApp PRIVATE 
    sfml-graphics 
    sfml-window 
    sfml-system 
    sfml-audio
    ${GLEW_LIBRARIES}
    OpenGL::GL
)