# pass_message_test

Enter project's directory.
```bash
$> cd pass_message_test
```

Build and generate executable files.
```bash
$> mkdir build
$> cd build
$> cmake ../
$> make all
```

Run the fifo program.
```bash
$> cd fifo/bin
$> ./fifo_send file_name_a
$> ./fifo_recv file_name_b
```

Run the shared_memory program.
```bash
$> cd shared_memory
$> ./shared_send file_name_a
$> ./shared_recv file_name_b
```
