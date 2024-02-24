message(STATUS "--------------------- DCM ---------------------")

add_library(DCM_LIB STATIC ${CMAKE_CURRENT_LIST_DIR}/src/Empty.cpp)

# Attempt to find Boost in default paths
find_package(Boost)

# If Boost was not found, set the path manually
if(NOT Boost_FOUND)
	message(STATUS "Boost not found, setting path manually")
    set(BOOST_ROOT "C:\\dev\\boost_1_83_0")
    find_package(Boost)
endif()

target_link_libraries(DCM_LIB PUBLIC dcm)

add_subdirectory(ext/dcm)
target_include_directories(DCM_LIB PUBLIC "${CMAKE_CURRENT_LIST_DIR}/ext/dcm/")