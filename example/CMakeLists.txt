# Async example

add_executable(async_echo_server
  async_echo_server.cpp
)
target_link_libraries(async_echo_server ${TRIAL_DATAGRAM_DEPENDENT_LIBRARIES})

add_executable(async_echo_client
  async_echo_client.cpp
)
target_link_libraries(async_echo_client ${TRIAL_DATAGRAM_DEPENDENT_LIBRARIES})

# Coroutine example

add_executable(spawn_echo_client
  spawn_echo_client.cpp
)
target_link_libraries(spawn_echo_client ${TRIAL_DATAGRAM_DEPENDENT_LIBRARIES})

# Target

add_custom_target(example
  DEPENDS
  async_echo_server
  async_echo_client
  spawn_echo_client)
