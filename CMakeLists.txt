cmake_minimum_required(VERSION 3.0...3.25)
project(
    GLVSCode
    VERSION 0.0.2
    LANGUAGES CXX C
)
add_executable(GLVSCode 
    src/main.cpp
    C:/Users/ianku/CPPLibraries/glad.c
)

set(GLFW_DIR "C:/Users/ianku/CPPLibraries/glfw-3.4")

target_include_directories(GLVSCode PRIVATE
    ${GLFW_DIR}/include
    C:/Users/ianku/CPPLibraries/glm
)

target_link_libraries(GLVSCode PRIVATE
    ${GLFW_DIR}/lib/glfw3.lib
)

set_target_properties(GLVSCode PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)