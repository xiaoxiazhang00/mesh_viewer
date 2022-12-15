cmake_minimum_required(VERSION 3.22)
project(mesh_preview)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

set(LIB_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/lib)
add_library(imgui STATIC
        ${LIB_FOLDER}/imgui/imgui.cpp
        ${LIB_FOLDER}/imgui/imgui_draw.cpp
        ${LIB_FOLDER}/imgui/imgui_demo.cpp
        ${LIB_FOLDER}/imgui/imgui_tables.cpp
        ${LIB_FOLDER}/imgui/imgui_widgets.cpp)

include_directories(${LIB_FOLDER}/imgui/ ${LIB_FOLDER}/imgui/backends)
add_executable(mesh_preview main.cpp tiny_obj_loader.cc Shader.cpp Shader.h ${LIB_FOLDER}/imgui/backends/imgui_impl_opengl3.cpp ${LIB_FOLDER}/imgui/backends/imgui_impl_glfw.cpp)
target_link_libraries(mesh_preview glfw OpenGL::GL GLEW::GLEW imgui)

#target_include_directories(mesh_preview imgui)
#target_link_directories(mesh_preview imgui)