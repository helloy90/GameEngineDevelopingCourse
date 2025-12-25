# get all files
file(GLOB
	PRIVATE_FILES
	private/*.cpp
	private/*.h
)

file(GLOB
	PUBLIC_FILES
	public/*.h
)

if (WINDOWS)
	set(VULKAN_LIBS_SEARCH_PATHS "${CMAKE_LIBRARY_PATH}/Vulkan/Lib")
	set(VULKAN_INCLUDE_DIRS "${CMAKE_LIBRARY_PATH}/Vulkan/Include")

	find_library(Vulkan_LIBRARY
		NAMES vulkan-1 vulkan
		PATHS 
			${VULKAN_LIBS_SEARCH_PATHS}
	)

	find_library(Vulkan_dxc_LIBRARY
		NAMES dxcompiler
		PATHS
			${VULKAN_LIBS_SEARCH_PATHS}
	)

	if (Vulkan_dxc_LIBRARY AND NOT TARGET dxc_lib)
    add_library(dxc_lib STATIC IMPORTED)
    set_property(TARGET dxc_lib
      PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${VULKAN_INCLUDE_DIRS}")
    set_property(TARGET dxc_lib APPEND
      PROPERTY
        IMPORTED_CONFIGURATIONS Release)
    set_property(TARGET dxc_lib APPEND
      PROPERTY
        IMPORTED_LOCATION_RELEASE "${Vulkan_dxc_LIBRARY}")
  endif()

endif()

if (WINDOWS)
	file(GLOB
		PLATFORM_PRIVATE_FILES
		private/Windows/*.cpp
		private/Windows/*.h
	)
endif()

# Add library
include(GenerateExportHeader)
add_library(VulkanRHI SHARED)
generate_export_header(VulkanRHI EXPORT_FILE_NAME export.h EXPORT_MACRO_NAME VULKAN_API)

set_target_properties(VulkanRHI PROPERTIES FOLDER "Engine")

target_compile_definitions(VulkanRHI
	PRIVATE
		Vulkan_EXPORTS
		UNICODE)

# Set Sources
target_sources(VulkanRHI
	PRIVATE ${PRIVATE_FILES}
	PRIVATE ${PUBLIC_FILES}
	PRIVATE ${PLATFORM_PRIVATE_FILES}
)

# include dir
target_include_directories(VulkanRHI 
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../RHI/public
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private/Windows
	PRIVATE ${CMAKE_LIBRARY_PATH}/VulkanMemoryAllocator/include
	PRIVATE ${VULKAN_INCLUDE_DIRS}
)

target_link_libraries(VulkanRHI PRIVATE
	Math
	RenderCore
	Core
)

if (WINDOWS)
	target_link_libraries(VulkanRHI PRIVATE
		${Vulkan_LIBRARY}
		dxc_lib
	)
	
	target_compile_definitions(VulkanRHI PRIVATE
		VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
		VULKAN_HPP_STORAGE_SHARED
		VULKAN_HPP_STORAGE_SHARED_EXPORT
		VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
		VULKAN_HPP_NO_EXCEPTIONS
		VMA_STATIC_VULKAN_FUNCTIONS=0
		VMA_DYNAMIC_VULKAN_FUNCTIONS=1
	)
endif()


target_precompile_headers(VulkanRHI
	PRIVATE
		<source_location>
		<ranges>
		<fstream>
		<unordered_map>
		<unordered_set>
		<string_view>
		<vector>
		<string>
		<cassert>
		<format>
		<functional>
		<thread>
		<mutex>
		<condition_variable>
		<tuple>
		<utility>
		<cstdint>
		<limits>
		<filesystem>
		<variant>
		<stack>
)