# mkk7
An utility to turn a Z80 binary file into a loadable "cassette tape" file (.k7 extension) for the VG5000.

The BASIC ROM of this machine is able to load chunks of raw data into the RAM. mkk7 does the necessary wrapping to:
- prepend a compatible K7 file header
- insert a BASIC Call instruction to kickstart the program
- compute the checksum

The load location is in the BASIC program area (TXTTAB=0x49FC); so that the Call trampoline is interpreted as a normal program, and the binary is appended with a tunable offset (default is to reserve 16 bytes for the trampoline).

# toy example: hello.s

As an example, `hello.s` and a simple Makefile are provided. 

Required tools: sdcc, binutils

In order to build `hello.k7`, run: `make PROJ=hello`

To run the k7 file, start a VG5000 emulator (example: [DCVG5K](http://dcvg5k.free.fr/)), load the k7 file and type `CLOAD<enter>`

# usage
```
mkk7 [-l <load_addr>] [-e <entry>] [-n <name>] <binary> <k7>
```

`load_addr` is the address of the first byte of the binary file, once loaded to RAM. It defaults to 0x4A0C.

`entry` is the absolute location of the instruction to jump to after loading. Defaults to the load address (aka first byte of the binary).

`name` is the K7 file name as writen in the header.

# memory layout

```
     0x49FC +----txttab-----+
            | 10 CALL xxxxx | __
            | ...padding... |   |
  load_addr +---------------+   |
   [0x4A0C] |  binary file  |   |
            |      .  entry | <-
                   .
            |      .        |
            +---------------+
```

# file layout
```
         +----- [D3]x10-------+
         | header(name,       |
         |   size, type,      |
         |   entry, run loc   |
         |   cksum, ...)      |
         +----- [D6]x10 ------+
         | content of         |
         | memory layout      |
         |       ...          |
         +----- [00]x10 ------+
