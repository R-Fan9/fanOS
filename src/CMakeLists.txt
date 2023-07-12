
cmake_minimum_required(VERSION 3.12)
project(x86OS LANGUAGES C ASM)

set(CMAKE_ASM_COMPILER nasm)
set(CMAKE_ASM_FLAGS "-f elf32")

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "-m32 -fno-PIC -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -Wall -Wextra -Werror -g -c")

add_executable(KRNL.SYS)

file(GLOB_RECURSE C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

target_sources(KRNL.SYS PRIVATE ${C_SOURCES})

target_sources(KRNL.SYS PRIVATE kernel_entry.asm)

target_include_directories(KRNL.SYS PRIVATE include/)
