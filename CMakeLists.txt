cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0015 NEW)
cmake_policy(SET CMP0072 OLD)

project(BaseCode)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

execute_process(COMMAND ln -s ../shaders)

set(appName BaseCode)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLUT_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories("${CMAKE_SOURCE_DIR}/glm")
include_directories("${CMAKE_SOURCE_DIR}/imgui")
include_directories("${CMAKE_SOURCE_DIR}/imgui/backends")

link_directories(${OPENGL_LIBRARY_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
link_directories(${GLEW_LIBRARY_DIRS})

add_executable(${appName} imgui/imgui.h imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_glut.h imgui/backends/imgui_impl_glut.cpp imgui/backends/imgui_impl_opengl3.h imgui/backends/imgui_impl_opengl3.cpp
PLYReader.h PLYReader.cpp TriangleMesh.h TriangleMesh.cpp Camera.h Camera.cpp Scene.h Scene.cpp Shader.h Shader.cpp ShaderProgram.h ShaderProgram.cpp Application.h Application.cpp main.cpp)
target_link_libraries(${appName} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES})

add_executable(MeshSimplifier TriangleMesh.cpp ShaderProgram.cpp Shader.cpp PLYReader.cpp PLYWriter.cpp MeshSimplifier.cpp Octree.cpp)
target_link_libraries(MeshSimplifier ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES} Eigen3::Eigen) 

add_executable(VisibilityPrecomputation VisibilityPrecomputation.cpp)