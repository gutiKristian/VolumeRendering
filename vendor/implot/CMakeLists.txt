add_library(IMPLOT_LIB STATIC
	# Implot
	"${CMAKE_CURRENT_LIST_DIR}/ext/implot/implot.h"
	"${CMAKE_CURRENT_LIST_DIR}/ext/implot/implot.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ext/implot/implot_internal.h"
	"${CMAKE_CURRENT_LIST_DIR}/ext/implot/implot_items.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ext/implot/implot_demo.cpp"
)

if (NOT EMSCRIPTEN)
	target_link_libraries(IMPLOT_LIB PUBLIC DAWN_LIB GLFW_LIB)
endif()

target_include_directories(IMPLOT_LIB PUBLIC "${CMAKE_CURRENT_LIST_DIR}/ext/implot" PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../imgui/ext/imgui")
