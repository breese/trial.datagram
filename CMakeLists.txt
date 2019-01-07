###############################################################################
#
# Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
###############################################################################

cmake_minimum_required(VERSION 3.1)
project(trial.datagram CXX)

set(TRIAL_DATAGRAM_EXAMPLE ON CACHE BOOL "Enable examples")

set(TRIAL_DATAGRAM_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(TRIAL_DATAGRAM_BUILD_DIR ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${TRIAL_DATAGRAM_BUILD_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${TRIAL_DATAGRAM_BUILD_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
endif()

###############################################################################
# Boost package
###############################################################################

find_package(Boost 1.55.0 COMPONENTS system)
if (NOT ${Boost_FOUND})
  message(FATAL_ERROR "${Boost_ERROR_REASON}")
endif()

include_directories(BEFORE ${Boost_INCLUDE_DIR})

set(TRIAL_DATAGRAM_DEPENDENT_LIBRARIES
  ${Boost_LIBRARIES})

###############################################################################
# trial.datagram package
###############################################################################

add_library(trial-datagram INTERFACE)
target_include_directories(trial-datagram INTERFACE "${TRIAL_DATAGRAM_ROOT}/include")

# Examples
if (TRIAL_DATAGRAM_EXAMPLE)
  add_subdirectory(example EXCLUDE_FROM_ALL)
endif()

add_subdirectory(doc EXCLUDE_FROM_ALL)
