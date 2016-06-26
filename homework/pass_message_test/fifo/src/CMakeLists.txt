include_directories(../../lib)

add_executable(fifo_send fifo_send.h fifo_send.c)
target_link_libraries(fifo_send file_manager time_counter input_output)

add_executable(fifo_recv fifo_recv.h fifo_recv.c)
target_link_libraries(fifo_recv file_manager time_counter input_output)
