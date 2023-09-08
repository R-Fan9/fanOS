# x86OS

A x86 (32 bits) operating system from scratch

## Description

### Project Structure

* /boot: source code for x86OS bootloader
* /boot/build: object files for the compiled boot code 
* /src: source code for x86OS
* /build: object files for the compiled src code 
* /bin: contains the floppy disk - OS.bin binary
* /target: linker scripts to combine object files to create an executable program

### Current State

#### Boot.bin
* 512 bytes bootsector
* Loads KRNLDR.SYS (kernel loader) into memory using BIOS int 0x13

#### KRNLDR.SYS
* Get a map of available RAM on the machine using BIOS int 0x15 function 0xE820
* Install Global Descriptor Table (GDT)
* Enable A20 gate (to access memory beyond 1MB) using BIOS int 0x15 function 0x2401
* Loads KRNL.SYS into memory using BIOS int 0x13
* Switch from real mode (16 bits) to protected mode (32 bits)
* Copy KRNL.SYS (kernel) from real mode address to protected mode address 0x100000
* Jump to kernel 

#### KRNL.SYS
* Initialize physical memory manager using the memory map information obtained from the bootloader
* Set up Global Descriptor Table (GDT), Interrupt Descriptor Table (IDT), Programmable Interrupt Controller (PIC), and Task State Segment (TSS)
* Set up software and hardware interrupts handlers (i.e. page fault handler, system call dispatcher, timer (PIT), keyboard and floppy disk controller)
* Initialize floppy disk controller with Direct Memory Access (DMA), file system implemented with FAT12 format
* Deinitialize memory region where the BIOS, bootloader, memory map and the kernel reside
* Initialize virtual memory manager with paging enabled and kernel mapped to address 0xC0000000 (higher half kernel)
* Enter user space

## Getting Started

### Prerequsites

* nasm
* build-essential
* qemu-system
* mtools
* make

### Installing

#### Linux
```
sudo apt-get install nasm build-essential qemu-system mtools make
```

### Executing x86OS

```
make run
```

## TODO(s)

* Implement write & save file(s) functionalities
* Implement process management (i.e. IPC, task scheduling, multithreading, etc)
* Implement more C standard library functions & date types (i.e. type conversion, type abstraction, etc)
* Implement network functionalities
* Allocate memory space from hard disk when encounter page fault (#PF) due to out of RAM
* Utilize Unified Extensible Firmware Interface (UEFI) to replace BIOS firmware, along with 64 bits long mode enabled (could be a version 2 of x86OS)
* Develop a simple Graphical User Interface (GUI)
* Enable x86OS to run successfully on bochs emulator

## Acknowledgments

* [BrokenThorn Entertainment](http://www.brokenthorn.com/Resources/OSDevIndex.html)
* [OSDev Wiki](https://wiki.osdev.org/Expanded_Main_Page)
* [Queso Fuego](https://www.youtube.com/@QuesoFuego)
