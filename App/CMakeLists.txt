project(App CXX)

add_executable(App
	"src/Main.cpp"
	"src/Application.h"
	"src/Application.cpp"
	"src/Camera.h"
	"src/Camera.cpp"
	"src/Shader.h"
	"src/ImGuiLayer.h"
	"src/ImGuiLayer.cpp"
		
	"src/renderer/Texture.h"
	"src/renderer/Texture.cpp"
	"src/renderer/VertexBuffer.h"
	"src/renderer/VertexBuffer.cpp"
	"src/renderer/UniformBuffer.h"
	"src/renderer/UniformBuffer.cpp"
	"src/renderer/RenderPipeline.h"
	"src/renderer/RenderPipeline.cpp"
	"src/renderer/StorageBuffer.h"
	"src/renderer/StorageBuffer.cpp"
	"src/renderer/Sampler.h"
	"src/renderer/Sampler.cpp"
	"src/renderer/PipelineBuilder.h"
	"src/renderer/PipelineBuilder.cpp"
	"src/renderer/BindGroup.h"
	"src/renderer/BindGroup.cpp"
	"src/renderer/IndexBuffer.h"
	"src/renderer/IndexBuffer.cpp"
	
	 "src/dicom/DcmImpl.cpp"
	 "src/dicom/DcmImpl.h"
	 "src/dicom/FileDataType.h"
	 "src/dicom/FileReader.cpp"
	 "src/dicom/FileReader.h"
	 "src/dicom/VolumeFile.cpp"
	 "src/dicom/VolumeFile.h"
 "src/tf/LinearInterpolation.h" "src/dicom/DatImpl.h" "src/dicom/DatImpl.cpp")

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
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/dcm "vendor/dcm")
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/implot "vendor/implot")

set(BOOST_ROOT "C:\\dev\\Boost\\boost_1_83_0")
find_package(Boost REQUIRED COMPONENTS system filesystem)

target_link_libraries(App PUBLIC IMGUI_LIB IMPLOT_LIB GLM_LIB DCM_LIB ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

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
