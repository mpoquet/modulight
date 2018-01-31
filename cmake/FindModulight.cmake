# - Find modulight libraries
# This module finds modulight if it is installed and determines where the
# include files and libraries are. This code sets the following variables:
#
# MODULIGHT_FOUND - have the modulight libs been found
# MODULIGHT_LIBRARIES - path to the modulight library
# MODULIGHT_INCLUDE_DIRS - path to where modulight include directory is found

# Finding modulight
find_library(MODULIGHT_LIBRARY
        NAMES libmodulight modulight
        PATHS
		/usr/lib
		/usr/local/lib
		${MODULIGHT_PREFIX}
		${MODULIGHT_PREFIX}/lib
		${MODULIGHT_LIBRARIES}
)

find_path(MODULIGHT_INCLUDE_DIR
        NAMES modulight/module.hpp
        PATHS
        /usr/include
		/usr/local/include
		${MODULIGHT_PREFIX}
		${MODULIGHT_PREFIX}/include
		${MODULIGHT_INCLUDE_DIRS}
)

set(MODULIGHT_INCLUDE_DIRS "${MODULIGHT_INCLUDE_DIR}")
set(MODULIGHT_LIBRARIES "${MODULIGHT_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Modulight DEFAULT_MSG MODULIGHT_LIBRARIES MODULIGHT_INCLUDE_DIRS)

mark_as_advanced(MODULIGHT_LIBRARY MODULIGHT_INCLUDE_DIR)

macro(use_modulight target)
	# Modulight
	include_directories(${MODULIGHT_INCLUDE_DIRS})
	target_link_libraries(${target} ${MODULIGHT_LIBRARIES})
	
	# MPI
	find_package(MPI REQUIRED)
	include_directories(${MPI_INCLUDE_PATH})
	target_link_libraries(${target} ${MPI_LIBRARIES})
	
	if(MPI_COMPILE_FLAGS)
		set_target_properties(${target} PROPERTIES 
							  COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
	endif()

	if(MPI_LINK_FLAGS)
	  set_target_properties(${target} PROPERTIES
							LINK_FLAGS "${MPI_LINK_FLAGS}")
	endif()
	
	# ZeroMQ
	find_package(ZeroMQ REQUIRED)
	include_directories(${ZEROMQ_INCLUDE_DIR})
	target_link_libraries(${target} ${ZEROMQ_LIBRARIES})
	
	# ZeroMQ, C++ binding
	find_package(ZeroMQCpp REQUIRED)
	include_directories(${ZEROMQCPP_INCLUDE_DIR})
	
	# Qt 5
	find_package(Qt5Core REQUIRED)
	find_package(Qt5Xml REQUIRED)
	find_package(Qt5Network REQUIRED)

	qt5_use_modules(${target} Xml Network)
endmacro()
