# - Find the C++ binding of zeromq
# This module finds the C++ binding of zeromq if it is installed and determines where the
# include file is. This code sets the following variables:
#
# ZEROMQCPP_FOUND - have the c++ binding of zeromq has been found
# ZEROMQCPP_INCLUDE_DIRS - path to where zmq.hpp is found


find_path(ZEROMQCPP_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS
		/usr/include
		/usr/local/include
		${ZEROMQCPP_PREFIX}
		${ZEROMQCPP_PREFIX}/include
		${ZEROMQCPP_INCLUDE_DIR}
)

set(ZEROMQCPP_INCLUDE_DIRS "${ZEROMQCPP_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZeroMQCpp DEFAULT_MSG ZEROMQCPP_INCLUDE_DIRS) 

mark_as_advanced(ZEROMQCPP_INCLUDE_DIR)
