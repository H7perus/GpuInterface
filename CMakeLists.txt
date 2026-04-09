file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.ixx" "*.slang")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

add_library(GpuInterface SHARED 
	${SOURCES}
)

target_compile_definitions(GpuInterface PRIVATE GPUI_DLL_EXPORTS)
set_target_properties(GpuInterface PROPERTIES FOLDER Engine)

set_target_properties(GpuInterface PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

# Specify the include directories so that other projects can use this module's headers

target_link_libraries(GpuInterface PUBLIC SDL3::SDL3)

target_link_libraries(GpuInterface PRIVATE
    $<$<CONFIG:Debug>:$ENV{VULKAN_SDK}/Lib/shaderc_combinedd.lib>
    $<$<CONFIG:Release>:$ENV{VULKAN_SDK}/Lib/shaderc_combined.lib>
)

set(Vulkan_LIBRARY "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")
set(Vulkan_INCLUDE_DIR "$ENV{VULKAN_SDK}/Include")
find_package(Vulkan REQUIRED)

message(STATUS "Vulkan_FOUND: ${Vulkan_FOUND}")
message(STATUS "Vulkan_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan_LIBRARIES: ${Vulkan_LIBRARIES}")

target_link_libraries(GpuInterface PUBLIC ${Vulkan_LIBRARIES} vk-bootstrap::vk-bootstrap slang::slang)



target_include_directories(GpuInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(GpuInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Adding include: ${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(GpuInterface PUBLIC ${Vulkan_INCLUDE_DIRS})

target_include_directories(GpuInterface PUBLIC 
    ${vk-bootstrap_SOURCE_DIR}/src
)






