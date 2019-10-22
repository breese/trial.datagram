find_package(Boost 1.55.0 COMPONENTS coroutine context thread)
if (NOT ${Boost_FOUND})
  message(FATAL_ERROR "${Boost_ERROR_REASON}")
endif()

# Async example

add_executable(async_echo_server
  async_echo_server.cpp
)
target_link_libraries(async_echo_server trial-datagram Boost::boost ${Boost_LIBRARIES})

add_executable(async_echo_client
  async_echo_client.cpp
)
target_link_libraries(async_echo_client trial-datagram Boost::boost ${Boost_LIBRARIES})

# Coroutine example

#add_executable(spawn_echo_client
#  spawn_echo_client.cpp
#  )
#target_compile_options(spawn_echo_client PRIVATE -DBOOST_COROUTINES_NO_DEPRECATION_WARNING)
#target_link_libraries(spawn_echo_client trial-datagram Boost::boost ${Boost_LIBRARIES})

# Target

add_custom_target(example
  DEPENDS
  async_echo_server
  async_echo_client
  #spawn_echo_client
    )
