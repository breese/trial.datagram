set(TRIAL_DATAGRAM_EXTERNAL_ROOT "${TRIAL_DATAGRAM_ROOT}/external")
set(TRIAL_DATAGRAM_EXTERNAL_BUILD_DIR "${TRIAL_DATAGRAM_BUILD_DIR}/external")

###############################################################################
# Trial.Net
###############################################################################

if (NOT DEFINED TRIAL_NET_ROOT)
  set(TRIAL_NET_ROOT "${TRIAL_DATAGRAM_EXTERNAL_ROOT}/trial.net/cmake")
endif()
add_subdirectory("${TRIAL_NET_ROOT}" "${TRIAL_DATAGRAM_BUILD_DIR}/trial.net" EXCLUDE_FROM_ALL)
add_library(Trial::Net INTERFACE IMPORTED GLOBAL)
target_link_libraries(Trial::Net INTERFACE trial-net)
