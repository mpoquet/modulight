# - Find zeromq libraries
# This module finds zeromq if it is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
# ZEROMQ_FOUND - have the zeromq libs been found
# ZEROMQ_LIBRARIES - path to the zeromq library
# ZEROMQ_INCLUDE_DIRS - path to where zmq.h is found

find_library(ZEROMQ_LIBRARY
        NAMES libzmq zmq
        PATHS
		/usr/lib
		/usr/local/lib
		${ZEROMQ_PREFIX}
		${ZEROMQ_PREFIX}/lib
		${ZEROMQ_LIBRARIES}
)

find_path(ZEROMQ_INCLUDE_DIR
        NAMES zmq.h
        PATHS
        /usr/include
		/usr/local/include
		${ZEROMQ_PREFIX}
		${ZEROMQ_PREFIX}/include
		${ZEROMQ_INCLUDE_DIRS}
)

set(ZEROMQ_INCLUDE_DIRS "${ZEROMQ_INCLUDE_DIR}")
set(ZEROMQ_LIBRARIES "${ZEROMQ_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZeroMQ DEFAULT_MSG ZEROMQ_LIBRARIES ZEROMQ_INCLUDE_DIRS)

mark_as_advanced(ZEROMQ_LIBRARY ZEROMQ_INCLUDE_DIR)
