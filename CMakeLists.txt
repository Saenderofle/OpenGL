cmake_minimum_required(VERSION 3.15)
project(OpenGL C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Підключаємо оригінальну бібліотеку GLFW
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory(external/glfw)

# 2. Налаштування GLM
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE external/glm)

# =========================================================================
# ЗАВДАННЯ 2.3.1: Три текстуровані прямокутники (Hw_2_3_1.cpp)
# =========================================================================
add_executable(Task_2_3_1
        src/Hw_2_3_1.cpp
        src/glad.c
)
target_include_directories(Task_2_3_1 PRIVATE include external/glm)
target_link_libraries(Task_2_3_1 PRIVATE glfw glm)


# =========================================================================
# ЗАВДАННЯ 2.3.2: Текстурований квадрат з обертанням і 60 FPS (Hw_2_3_2.cpp)
# =========================================================================
add_executable(Task_2_3_2
        src/Hw_2_3_2.cpp
        src/glad.c
)
target_include_directories(Task_2_3_2 PRIVATE include external/glm)
target_link_libraries(Task_2_3_2 PRIVATE glfw glm)