add_library(GLM_LIB STATIC ${CMAKE_CURRENT_LIST_DIR}/src/Empty.cpp)

target_include_directories(GLM_LIB PUBLIC "${CMAKE_CURRENT_LIST_DIR}/ext/glm/")
