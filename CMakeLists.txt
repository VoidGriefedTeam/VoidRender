cmake_minimum_required(VERSION 4.4)

project(render
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Executable
# ------------------------------------------------------------

add_executable(render
    src/cube_gpu.cpp
)

# ------------------------------------------------------------
# Include directories
# ------------------------------------------------------------

target_include_directories(render PRIVATE
    "${CMAKE_SOURCE_DIR}/THIRD_PARTY/voidcl/include"
    "${CMAKE_SOURCE_DIR}/THIRD_PARTY/sgl/include/simplegl"
)

# ------------------------------------------------------------
# Libraries
# ------------------------------------------------------------

target_link_directories(render PRIVATE
    "${CMAKE_SOURCE_DIR}/THIRD_PARTY/voidcl/lib"
    "${CMAKE_SOURCE_DIR}/THIRD_PARTY/sgl/lib"
)

target_link_libraries(render PRIVATE
    VoidCLcompute
    simplegl
)

# ------------------------------------------------------------
# Copy DLLs to build directory
# ------------------------------------------------------------

add_custom_command(TARGET render POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/THIRD_PARTY/voidcl/bin"
        "${CMAKE_BINARY_DIR}"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/THIRD_PARTY/sgl/bin"
        "${CMAKE_BINARY_DIR}"

    COMMENT "Copying VoidCLcompute and SGL DLLs..."
)