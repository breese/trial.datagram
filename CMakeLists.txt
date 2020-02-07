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

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
endif()

###############################################################################
# External packages
###############################################################################

add_subdirectory(external)

###############################################################################
# trial.datagram package
###############################################################################

add_library(trial-datagram INTERFACE)
target_include_directories(trial-datagram INTERFACE "${TRIAL_DATAGRAM_ROOT}/include")
target_link_libraries(trial-datagram INTERFACE Trial::Net)

# Examples
if (TRIAL_DATAGRAM_EXAMPLE)
  add_subdirectory(example EXCLUDE_FROM_ALL)
endif()
