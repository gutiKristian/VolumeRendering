project(App CXX)

add_executable(App
 "src/Application.cpp"
)

target_compile_definitions(App
	PRIVATE
	"_CRT_SECURE_NO_WARNINGS"

	PUBLIC
	$<$<CONFIG:Debug>:SS_DEBUG>
	$<$<CONFIG:Release>:SS_RELEASE>

	$<$<CONFIG:RelWithDebInfo>:SS_REL_DEB_INFO>
)

set(BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/)

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/imgui "vendor/imgui")
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/glm "vendor/glm")

target_link_libraries(App PUBLIC IMGUI_LIB GLM_LIB)

ConfigureProject()

if (EMSCRIPTEN)
	target_compile_definitions(App PRIVATE PLATFORM_WEB)
elseif(WIN32)
	target_compile_definitions(App PRIVATE PLATFORM_WINDOWS)
elseif(APPLE)
	target_compile_definitions(App PRIVATE PLATFORM_MAC)
elseif(UNIX)
	target_compile_definitions(App PRIVATE PLATFORM_LINUX)
endif()

set_property(TARGET App PROPERTY CXX_STANDARD 20)

target_include_directories(App PUBLIC ${WEBGPU_LIB_SOURCE_DIR}/src)
target_link_libraries(App PUBLIC WEBGPU_LIB)
