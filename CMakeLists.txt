cmake_minimum_required(VERSION 2.8.8)

# Project
set(LIBNAME modulight)
project(${LIBNAME})

# Let's set build type to Debug if is unset
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to 'Debug' as none was specified.")
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Modulight files
include_directories(include)

file(GLOB common_sources src/common/*.cpp)
file(GLOB master_sources src/master/*.cpp)
file(GLOB module_sources src/module/*.cpp)

file(GLOB common_headers include/modulight/common/*.hpp)
file(GLOB master_headers include/modulight/master/*.hpp)
file(GLOB module_headers include/modulight/module/*.hpp)
file(GLOB base_headers include/modulight/*.hpp)

# Let's create a shared library
add_library(
	${LIBNAME} SHARED
	${common_sources}
	${master_sources}
	${module_sources}
)

# MPI
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
target_link_libraries(${LIBNAME} ${MPI_LIBRARIES})

if(MPI_COMPILE_FLAGS)
	set_target_properties(${LIBNAME} PROPERTIES 
					      COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(${LIBNAME} PROPERTIES
						LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

# ZeroMQ
set(CMAKE_MODULE_PATH /home/carnivor/proj/ter/modulight/cmake)
find_package(ZeroMQ REQUIRED)
include_directories(${ZEROMQ_INCLUDE_DIR})
target_link_libraries(${LIBNAME} ${ZEROMQ_LIBRARIES})

# ZeroMQ, cpp binding (header only)
find_package(ZeroMQCpp REQUIRED)
include_directories(${ZEROMQCPP_INCLUDE_DIR})

# Qt
find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

qt5_use_modules(${LIBNAME} Xml Network)

# Installation
install(TARGETS ${LIBNAME} DESTINATION lib)

install(FILES ${common_headers} DESTINATION include/modulight/common)
install(FILES ${master_headers} DESTINATION include/modulight/master)
install(FILES ${module_headers} DESTINATION include/modulight/module)
install(FILES ${base_headers} DESTINATION include/modulight)

install(FILES cmake/FindModulight.cmake DESTINATION share/cmake/Modulight)
install(FILES cmake/Macros.cmake DESTINATION share/cmake/Modulight)
