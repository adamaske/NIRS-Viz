# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.15)

# Define the project name
project(NeuroVisualizer VERSION 1.0)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")

find_package( OpenGL REQUIRED )
include_directories( ${OPENGL_INCLUDE_DIRS} )


include_directories("${GLFW_SOURCE_DIR}/deps")
    set( GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h" )
if(MSVC)
    add_compile_options("/utf-8")
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()


add_subdirectory( vendor/glfw )

set(IMGUI_SOURCES
    ${VENDOR_DIR}/imgui/imgui.cpp
    ${VENDOR_DIR}/imgui/imgui_draw.cpp
    ${VENDOR_DIR}/imgui/imgui_tables.cpp
    ${VENDOR_DIR}/imgui/imgui_widgets.cpp
    ${VENDOR_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${VENDOR_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)


add_subdirectory( vendor/spdlog)

add_library(assimp SHARED IMPORTED)
set_target_properties(assimp PROPERTIES
    IMPORTED_LOCATION "${VENDOR_DIR}/assimp/bin/assimp.dll"
    IMPORTED_IMPLIB "${VENDOR_DIR}/assimp/lib/assimp-vc143-mt.lib"
    #INTERFACE_INCLUDE_DIRECTORIES "${VENDOR_DIR}/assimp/include/"
)

set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
file(GLOB_RECURSE DATA_SHARE "${DATA_DIR}/*")
set(INSTALL_FILES ${DATA_SHARE})

# Install the listed files to a 'data' subdirectory under the install prefix
install(
    FILES ${INSTALL_FILES}
    DESTINATION "share/my_project_name/data"
)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE INCS "${INCLUDE_DIR}/*.h")
file(GLOB_RECURSE SRCS "${SOURCE_DIR}/*.cpp")

add_executable( ${PROJECT_NAME} ${SRCS} ${IMGUI_SOURCES} vendor/glad/src/glad.c )

# Include the subdirectory and use the target HighFive.
#add_subdirectory(${VENDOR_DIR}/HighFive INTERFACE)

# Add vendor include directories to find header files
target_include_directories(${PROJECT_NAME} PUBLIC
    ${INCLUDE_DIR}
    ${VENDOR_DIR}/glfw/include
    ${VENDOR_DIR}/spdlog/include
    ${VENDOR_DIR}/glad/include
    ${VENDOR_DIR}/glm
    ${VENDOR_DIR}/imgui
    ${VENDOR_DIR}/assimp/include
    ${VENDOR_DIR}/HighFive/include

)
 

# Link the required libraries 
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OPENGL_LIBRARIES} 
    glfw 
    $<$<BOOL:${MINGW}>:ws2_32> 
    spdlog::spdlog 
    assimp
    #HighFive
)
