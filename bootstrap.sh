#! /usr/bin/env bash
set -ex

# Make the required bin directry
[ -d bin ] || mkdir bin

# Build debug footer
blood-elf -f seed.M1 \
	-o bin/seed-footer.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f functions/libc-core.M1 \
	-f seed.M1 \
	-f bin/seed-footer.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o bin/seed.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386-debug.hex2 \
	-f bin/seed.hex2 \
	--LittleEndian \
	--Architecture 1 \
	--BaseAddress 0x8048000 \
	-o bin/M2-Planet \
	--exec_enable || exit 2
