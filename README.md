# mybios

A from-scratch x86 BIOS implementation targeting legacy PC hardware (386/486 era), written in C and NASM assembly.

## Background

This project began as an effort to revive a motherboard whose BIOS had been
corrupted, with no ROM dumps available online at the time. The chipset
initialization code for the UMC UM82C480 was reverse engineered by fuzzing
registers in order to bring the board back to life. The project has since grown
into a more general custom BIOS implementation supporting multiple chipsets and
has successfully booted on some Pentium and even a Pentium Pro motherboard,
albeit without full DRAM, cache controller, and bus initialization on those
platforms.

## Supported Chipsets

- **UMC UM82C480** - 486-era chipset (reverse engineered initialization)
- **Intel 440FX** - Pentium-era PCI chipset
- **SiS 8C460** - DRAM configuration and memory detection
- **Null** - Minimal stub for testing

## Features

- Complete POST (Power-On Self Test) sequence with POST codes
- Real mode and protected mode operation with mode switching
- BIOS interrupt services: INT 13h (disk), INT 16h (keyboard), INT 15h (extended services), INT 1Ah (time), and more
- ATA/IDE disk driver with CHS and LBA addressing
- PCI bus enumeration and configuration
- Keyboard and timer (PIT 8254) interrupt handling
- VGA option ROM initialization
- MBR boot support
- Serial port debug output

## Building

Requires an i386 cross-compiler toolchain and NASM:

```
make
```

The default build produces a 64KB ROM image (`bios.bin`). Edit the `Makefile` to select the target chipset.

## Testing

The BIOS can be tested under QEMU or Bochs:

```
# QEMU
./qemu.sh

# Bochs
bochs -f .bochsrc
```

## License

This project is provided as-is for educational and research purposes.
