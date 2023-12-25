add_library(IMGUI_LIB STATIC
	"${CMAKE_CURRENT_LIST_DIR}/ext/imgui/imgui.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ext/imgui/imgui_draw.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ext/imgui/imgui_tables.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ext/imgui/imgui_widgets.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ext/imgui/backends/imgui_impl_glfw.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ext/imgui/backends/imgui_impl_wgpu.cpp"
)

if (NOT EMSCRIPTEN)
	target_link_libraries(IMGUI_LIB PUBLIC DAWN_LIB GLFW_LIB)
endif()

target_include_directories(IMGUI_LIB PUBLIC "${CMAKE_CURRENT_LIST_DIR}/ext/" PUBLIC "${CMAKE_CURRENT_LIST_DIR}/ext/imgui/")
