message(STATUS "--------------------- DCM ---------------------")

add_library(DCM_LIB STATIC ${CMAKE_CURRENT_LIST_DIR}/src/Empty.cpp)

set(BOOST_ROOT "C:\\dev\\boost_1_83_0")

target_link_libraries(DCM_LIB PUBLIC dcm)

add_subdirectory(ext/dcm)
target_include_directories(DCM_LIB PUBLIC "${CMAKE_CURRENT_LIST_DIR}/ext/dcm/")