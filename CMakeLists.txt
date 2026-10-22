cmake_minimum_required(VERSION 3.22)
project(Vulkan3DEngine VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler flags for Clang & MSVC
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -Wno-invalid-offsetof
        -Wno-tautological-constant-out-of-range-compare
    )
elseif(MSVC)
    add_compile_options(/W4 /permissive-)
endif()

# Include third-party dependencies
include(cmake/Dependencies.cmake)

# Shader compilation
set(SHADER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/mesh.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/mesh.frag
)

set(SPV_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
file(MAKE_DIRECTORY ${SPV_OUTPUT_DIR})

set(SPV_OUTPUTS "")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPV_OUTPUT ${SPV_OUTPUT_DIR}/${FILE_NAME}.spv)
    list(APPEND SPV_OUTPUTS ${SPV_OUTPUT})

    add_custom_command(
        OUTPUT ${SPV_OUTPUT}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SPV_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILE_NAME} -> ${FILE_NAME}.spv"
        VERBATIM
    )
endforeach()

add_custom_target(CompileShaders ALL DEPENDS ${SPV_OUTPUTS})

# Core Engine Library
file(GLOB_RECURSE ENGINE_CORE_SOURCES
    "src/core/*.cpp"
    "src/reflection/*.cpp"
    "src/rhi/*.cpp"
    "src/scene/*.cpp"
    "src/physics/*.cpp"
    "src/audio/*.cpp"
    "src/project/*.cpp"
)

file(GLOB_RECURSE ENGINE_CORE_HEADERS
    "src/core/*.hpp"
    "src/reflection/*.hpp"
    "src/rhi/*.hpp"
    "src/scene/*.hpp"
    "src/physics/*.hpp"
    "src/audio/*.hpp"
    "src/project/*.hpp"
)

add_library(EngineCore STATIC
    ${ENGINE_CORE_SOURCES}
    ${ENGINE_CORE_HEADERS}
)

target_include_directories(EngineCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/miniaudio
    ${glm_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${nlohmann_json_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIRS}
    ${glfw_SOURCE_DIR}/include
)

target_link_libraries(EngineCore PUBLIC
    Vulkan::Vulkan
    glfw
    glm::glm
    nlohmann_json::nlohmann_json
)

# Engine Editor Executable
file(GLOB_RECURSE EDITOR_SOURCES
    "src/editor/*.cpp"
)

file(GLOB_RECURSE EDITOR_HEADERS
    "src/editor/*.hpp"
)

add_executable(EngineEditor
    src/main.cpp
    ${EDITOR_SOURCES}
    ${EDITOR_HEADERS}
)

target_include_directories(EngineEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imguizmo_SOURCE_DIR}/src
    ${glm_SOURCE_DIR}
)

target_link_libraries(EngineEditor PRIVATE
    EngineCore
    imgui_lib
    Vulkan::Vulkan
    glfw
)

add_dependencies(EngineEditor CompileShaders)

# Automated Test Suite Executable
add_executable(EngineTests
    tests/EngineTests.cpp
)

target_include_directories(EngineTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${glm_SOURCE_DIR}
    ${nlohmann_json_SOURCE_DIR}/include
)

target_link_libraries(EngineTests PRIVATE
    EngineCore
)

# Doxygen target
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()
