# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Define the project name
project(NeuroVisualizer VERSION 1.0)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory( vendor/spdlog)

find_package( OpenGL REQUIRED )
include_directories( ${OPENGL_INCLUDE_DIRS} )


add_subdirectory( vendor/glfw )
include_directories("${GLFW_SOURCE_DIR}/deps")
    set( GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h" )
if(MSVC)
    add_compile_options("/utf-8")
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()

set(IMGUI_SOURCES
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE INCS "${INCLUDE_DIR}/*.h")
file(GLOB_RECURSE SRCS "${SOURCE_DIR}/*.cpp")

add_executable( ${PROJECT_NAME} ${SRCS} 
    ${IMGUI_SOURCES} vendor/glad/src/glad.c )
# Add vendor include directories to find header files
target_include_directories(${PROJECT_NAME} PUBLIC
    # User's own include folder
    ${INCLUDE_DIR}
    
    vendor/glfw/include
    vendor/spdlog/include
    vendor/glad/include
    vendor/glm
    vendor/imgui
)

# Link the required libraries
target_link_libraries(${PROJECT_NAME} 
    ${OPENGL_LIBRARIES} 
    glfw 
    spdlog::spdlog
    #$<$<BOOL:${MINGW}>:ws2_32>
)

