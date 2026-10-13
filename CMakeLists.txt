cmake_minimum_required(VERSION 4.4)   # must be <= the CMake you have installed (cmake --version)

project(render
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# ------------------------------------------------------------
# Output
# ------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")



# ------------------------------------------------------------
# Library
# ------------------------------------------------------------
add_library(render SHARED
    src/render.cpp
)

set_target_properties(render PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

include(GenerateExportHeader)
generate_export_header(render
    EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/generated/render_export.h"
)

# ------------------------------------------------------------
# Include directories
# ------------------------------------------------------------
target_include_directories(render
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>"
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/THIRD_PARTY/voidcl/include>"
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/THIRD_PARTY/sgl/include/simplegl>"
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/THIRD_PARTY/Glaze/glaze-9.0.0/include>"
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/THIRD_PARTY/mapfmt/include>"
        "$<INSTALL_INTERFACE:include>"
)

# ------------------------------------------------------------
# Libraries
# ------------------------------------------------------------
target_link_directories(render PRIVATE
    "${CMAKE_SOURCE_DIR}/THIRD_PARTY/voidcl/lib"
    "${CMAKE_SOURCE_DIR}/THIRD_PARTY/sgl/lib"
)

target_link_libraries(render
    PRIVATE
        VoidCLcompute
        simplegl
)

# ------------------------------------------------------------
# Example
# ------------------------------------------------------------

add_executable(voidrender_smoke WIN32
    examples/smoke/smoke.cpp 
    ) 

set_target_properties(voidrender_smoke PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_options(voidrender_smoke PRIVATE
    /ENTRY:mainCRTStartup
)

target_compile_features(voidrender_smoke PRIVATE cxx_std_23)
target_include_directories(voidrender_smoke PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include" 
    "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY/voidcl/include" 
    "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY/sgl/include/simplegl" 
    "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY/Glaze/glaze-9.0.0" 
    "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY/mapfmt/include" 
    ) 
target_link_libraries(voidrender_smoke PRIVATE render) 
set_target_properties(voidrender_smoke PROPERTIES RUNTIME_OUTPUT_DIRECTORY 
"${CMAKE_BINARY_DIR}/bin" 
)
add_custom_command(TARGET voidrender_smoke POST_BUILD
    COMMAND cmd /c "${CMAKE_BINARY_DIR}/bin/mapc.exe ${CMAKE_SOURCE_DIR}/examples/smoke/smoke_anim.map ${CMAKE_BINARY_DIR}/bin/smoke_anim.mapb"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/examples/smoke/smoke.config.json"
        "$<TARGET_FILE_DIR:voidrender_smoke>/smoke.config.json"
)


add_executable(game WIN32
    examples/game/main.cpp 
    ) 

set_target_properties(game PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_options(game PRIVATE
    /ENTRY:mainCRTStartup
)

target_compile_features(game PRIVATE cxx_std_23)
target_include_directories(game PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include" 
    "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY/voidcl/include" 
    "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY/sgl/include/simplegl" 
    "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY/Glaze/glaze-9.0.0" 
    "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY/mapfmt/include" 
    ) 
target_link_libraries(game PRIVATE render) 
set_target_properties(game PROPERTIES RUNTIME_OUTPUT_DIRECTORY 
"${CMAKE_BINARY_DIR}/bin" 
)

add_custom_command(TARGET game POST_BUILD
    COMMAND "${CMAKE_BINARY_DIR}/bin/mapc.exe"
        "${CMAKE_SOURCE_DIR}/examples/game/smoke_anim.map"
        "$<TARGET_FILE_DIR:game>/game_anim.mapb"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/examples/game/smoke.config.json"
        "$<TARGET_FILE_DIR:game>/game.config.json"

    COMMENT "Compiling game MAP2 assets..."
)


# ------------------------------------------------------------
# Copy dependency DLLs
# ------------------------------------------------------------
add_custom_command(TARGET render POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/THIRD_PARTY/voidcl/bin"
        "$<TARGET_FILE_DIR:render>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/THIRD_PARTY/sgl/bin"
        "$<TARGET_FILE_DIR:render>"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/THIRD_PARTY/mapfmt/bin"
        "$<TARGET_FILE_DIR:render>"

    COMMENT "Copying VoidCLcompute and SGL DLLs..."
)


# ------------------------------------------------------------
# Installation
# ------------------------------------------------------------

# Install the renderer DLL and both applications.
install(TARGETS render voidrender_smoke game
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)

# Install runtime dependencies.
install(FILES
    "${CMAKE_SOURCE_DIR}/THIRD_PARTY/voidcl/bin/voidclcompute.dll"
    "${CMAKE_SOURCE_DIR}/THIRD_PARTY/sgl/bin/simplegl.dll"
    DESTINATION bin
)

# Install the MAP2 compiler.
install(FILES
    "${CMAKE_BINARY_DIR}/bin/mapc.exe"
    DESTINATION bin
)

# Install public headers.
install(FILES
    "${CMAKE_SOURCE_DIR}/include/render.hpp"
    "${CMAKE_BINARY_DIR}/generated/render_export.h"
    DESTINATION include
)

# Install smoke demo assets.
install(FILES
    "${CMAKE_SOURCE_DIR}/examples/smoke/smoke.config.json"
    "${CMAKE_BINARY_DIR}/bin/smoke_anim.mapb"
    DESTINATION bin
)

# Install game demo assets.
install(FILES
    "${CMAKE_SOURCE_DIR}/examples/game/smoke.config.json"
    "${CMAKE_BINARY_DIR}/bin/game_anim.mapb"
    DESTINATION bin
)
