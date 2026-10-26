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

#Vulkan is found in the engine root CMakeLists
target_link_libraries(GpuInterface PUBLIC Vulkan::Vulkan vk-bootstrap::vk-bootstrap slang::slang)



target_include_directories(GpuInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(GpuInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Adding include: ${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(GpuInterface PUBLIC ${Vulkan_INCLUDE_DIRS})

target_include_directories(GpuInterface PUBLIC 
    ${vk-bootstrap_SOURCE_DIR}/src
)






