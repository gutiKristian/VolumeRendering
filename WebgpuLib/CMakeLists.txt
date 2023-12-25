project(WEBGPU_LIB CXX)

set(WEBGPU_LIB_Sources
	"src/Base/Base.h"
	"src/Base/Assert.h"
	"src/Base/Window.h"
	"src/Base/Event.h"
	"src/Base/Utils.h"
	"src/Base/Timestep.h"
	"src/Base/Timer.h"
	"src/Base/Log.h"
	"src/Base/Log.cpp"
	"src/Base/Logger.h"
	"src/Base/Logger.cpp"
	"src/Base/Filesystem.h"
	"src/Base/Filesystem.cpp"
	"src/Base/GraphicsContext.h"
	"src/Base/GraphicsContext.cpp"
)

if(EMSCRIPTEN)
	set(WEBGPU_LIB_PlatformSources
		"src/Platform/Web/WebUtils.cpp"
		"src/Platform/Web/WebWindow.cpp"
		"src/Platform/Web/WebGraphicsContext.cpp"
		"src/Platform/Web/WebFilesystem.cpp"
	)
else()
	set(WEBGPU_LIB_PlatformSources
		"src/Platform/Native/NativeUtils.cpp"
		"src/Platform/Native/NativeWindow.cpp"
		"src/Platform/Native/NativeGraphicsContext.cpp"
		"src/Platform/Native/NativeFilesystem.cpp"
	)
endif()

add_library(WEBGPU_LIB STATIC ${WEBGPU_LIB_Sources} ${WEBGPU_LIB_PlatformSources})

if (NOT EMSCRIPTEN)
	add_subdirectory(vendor/glfw)
	add_subdirectory(vendor/dawn)

	target_link_libraries(WEBGPU_LIB PUBLIC GLFW_LIB DAWN_LIB)
else()
	set(GLOBAL CMAKE_EXECUTABLE_SUFFIX ".html")

	set_property(GLOBAL PROPERTY EMSCRIPTEN_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}
		--shell-file ${CMAKE_SOURCE_DIR}/src/web/shell.html
		-lglfw
		-s USE_WEBGPU=1
		-s USE_GLFW=3
		-s WASM=1
		-s ASYNCIFY
		-s ALLOW_MEMORY_GROWTH
		-std=c++20")

	target_include_directories(WEBGPU_LIB PUBLIC "${CMAKE_CURRENT_LIST_DIR}/vendor/emsdk/ext/upstream/emscripten/cache/sysroot/include")

	set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES} ${CMAKE_CURRENT_LIST_DIR}/vendor/emsdk/ext/upstream/emscripten/cache/sysroot/include")
endif()

macro(ConfigureProject)
	if (EMSCRIPTEN)
		set(CMAKE_EXECUTABLE_SUFFIX ".html")

		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}
		--shell-file ${CMAKE_SOURCE_DIR}/src/web/shell.html
		-lglfw
		-s USE_WEBGPU=1
		-s USE_GLFW=3
		-s WASM=1
		-s ASYNCIFY
		-s ALLOW_MEMORY_GROWTH
		--preload-file ${CMAKE_SOURCE_DIR}/App/assets/shaders/@assets/
		-std=c++20")

		configure_file( ${CMAKE_SOURCE_DIR}/src/web/shell.css ${CMAKE_CURRENT_BINARY_DIR}/shell.css COPYONLY)
	endif()
endmacro()

target_compile_definitions(WEBGPU_LIB
	PRIVATE
	"_CRT_SECURE_NO_WARNINGS"

	PUBLIC
	$<$<CONFIG:Debug>:CONFIG_DEBUG>
	$<$<CONFIG:Release>:CONFIG_RELEASE>

	$<$<CONFIG:RelWithDebInfo>:CONFIG_RELEASE>
)

if (EMSCRIPTEN)
	target_compile_definitions(WEBGPU_LIB PRIVATE PLATFORM_WEB)
elseif(WIN32)
	target_compile_definitions(WEBGPU_LIB PRIVATE PLATFORM_WINDOWS)
elseif(APPLE)
	target_compile_definitions(WEBGPU_LIB PRIVATE PLATFORM_MAC)
elseif(UNIX)
	target_compile_definitions(WEBGPU_LIB PRIVATE PLATFORM_LINUX)
endif()

if (NOT EMSCRIPTEN)
	get_property(assets_path GLOBAL PROPERTY ASSETS_PATH_PROP)
	target_compile_definitions(WEBGPU_LIB PRIVATE ASSETS_PATH="${assets_path}")
endif()

target_include_directories(WEBGPU_LIB PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)

set_property(TARGET WEBGPU_LIB PROPERTY CXX_STANDARD 20)
