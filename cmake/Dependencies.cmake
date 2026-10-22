# cmake/Dependencies.cmake
include(FetchContent)

# 1. Vulkan SDK
set(VULKAN_SDK_PATH "C:/VulkanSDK/1.4.363.0")
if(DEFINED ENV{VULKAN_SDK} AND EXISTS "$ENV{VULKAN_SDK}")
    set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
endif()

set(ENV{VULKAN_SDK} "${VULKAN_SDK_PATH}")
set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/Include" CACHE PATH "Vulkan Include Dir" FORCE)
set(Vulkan_LIBRARY "${VULKAN_SDK_PATH}/Lib/vulkan-1.lib" CACHE FILEPATH "Vulkan Library" FORCE)

find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan Include: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Found Vulkan Library: ${Vulkan_LIBRARIES}")

# Vulkan glslc shader compiler
if(EXISTS "${VULKAN_SDK_PATH}/Bin/glslc.exe")
    set(GLSLC_EXECUTABLE "${VULKAN_SDK_PATH}/Bin/glslc.exe" CACHE FILEPATH "glslc compiler" FORCE)
else()
    find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "${VULKAN_SDK_PATH}/bin" "${VULKAN_SDK_PATH}/Bin")
endif()
message(STATUS "Found glslc: ${GLSLC_EXECUTABLE}")

# 2. GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

# 3. GLM (Header-only mathematics)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)

# 4. Dear ImGui (docking branch)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
    GIT_SHALLOW TRUE
)

# 5. ImGuizmo (3D Transform Gizmo) - Source-only fetch (do not run its CMakeLists)
FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# 6. nlohmann/json (Modern JSON for C++)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

message(STATUS "Fetching dependencies...")
FetchContent_MakeAvailable(glfw glm imgui nlohmann_json)

FetchContent_GetProperties(imguizmo)
if(NOT imguizmo_POPULATED)
    FetchContent_Populate(imguizmo)
endif()

# Create ImGui static library with GLFW and Vulkan backends
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    ${imguizmo_SOURCE_DIR}/src/ImGuizmo.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imguizmo_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(imgui_lib PUBLIC
    glfw
    Vulkan::Vulkan
)
