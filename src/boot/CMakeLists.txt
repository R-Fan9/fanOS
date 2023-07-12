cmake_minimum_required(VERSION 3.12)
project(x86OS LANGUAGES ASM)

set(CMAKE_ASM_COMPILER nasm)
set(CMAKE_ASM_FLAGS "-f bin")

add_executable(Boot.bin)
add_executable(KRNLDR.SYS)

target_sources(Boot.bin PRIVATE boot.asm)
target_sources(KRNLDR.SYS PRIVATE stage2.asm)

target_include_directories(KRNLDR.SYS PRIVATE include/)
