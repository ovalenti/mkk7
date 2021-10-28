# mkk7
An utility to turn a Z80 binary file into a loadable K7 for the VG5000.

The BASIC ROM of this machine is able to load chunks of raw data into the RAM. mkk7 does the necessary wrapping to:
- prepend a compatible K7 file header
- insert a BASIC Call instruction to kickstart the program
- compute the checksum

The default load location is in the BASIC program area (TXTTAB=0x49FC); so that the Call trampoline is interpreted as a normal program, and the binary is appended with a tunable offset (default is to reserve 16 bytes for the trampoline).

# usage
```
mkk7 [-l <load_addr>] [-e <entry>] [-n <name>] <binary> <k7>
```

`load_addr` is the address of the first byte of the binary file, once loaded to RAM. It defaults to 0x4A0C.

`entry` is the absolute location of the instruction to jump to after loading.

`name` is the K7 file name as writen in the header.
