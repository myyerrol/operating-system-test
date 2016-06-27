include_directories(../../lib)

add_executable(shared_send shared_memory.h shared_send.c)
target_link_libraries(shared_send file_manager time_counter input_output)

add_executable(shared_recv shared_memory.h shared_recv.c)
target_link_libraries(shared_recv file_manager time_counter input_output)
