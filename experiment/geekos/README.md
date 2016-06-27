# geekos

## Configure

### Install

#### nasm
```bash
$> cd /opt
$> sudo tar -xvf nasm-2.08.02.tar.gz
$> cd nasm-2.08.02
$> sudo ./configure
$> sudo make
$> sudo make install
```

#### bochs
```bash
$> cd /opt
$> sudo tar -xvf bochs-2.6.8.tar.gz
$> cd bochs-2.6.8
$> sudo ./configure --enable-gdb-stub --enable-disasm
$> sudo make
$> sudo make install
```

### Build
Because my computer's architecture is X86-64, I have to add **-m32** and **-m elf_i386** arguments to build these X86-32 codes.
```bash
$> cd project[0-4]/build
$> make clean
$> make depend
$> make all
```

### Run
project[0-4]
```bash
$> make run
```
boot
```bash
$> cd boot
$> bochs -f bochsrc
```

### Debug
Before runnig the follow command, please open the comment **#gdbstub: enabled=1, port=1234, text_base=0, data_base=0, bss_base=0**
```bash
$> make debug
```
You can set the breakpoints by adding **break function's name** to **gdbinfo** file.
