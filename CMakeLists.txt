cmake_minimum_required(VERSION 3.15)
project(OpenGL)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory(external/glfw)


add_library(glm INTERFACE)
target_include_directories(glm INTERFACE external/glm)

# Створюємо наш виконуваний файл
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/glad.c
)


target_include_directories(${PROJECT_NAME} PRIVATE
        include
        external/glm
)


if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm dl)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm)
endif()