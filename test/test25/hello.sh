#! /bin/sh
set -x
# Build the test
./bin/M2-Planet -f functions/exit.c \
	-f functions/file.c \
	-f functions/file_print.c \
	-f functions/malloc.c \
	-f functions/calloc.c \
	-f functions/match.c \
	-f functions/numerate_number.c \
	-f functions/fork.c \
	-f functions/execve.c \
	-f test/test25/kaem.c \
	--debug \
	-o test/test25/kaem.M1 || exit 1

# Build debug footer
blood-elf -f test/test25/kaem.M1 \
	-o test/test25/kaem-footer.M1 || exit 2

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f functions/libc-core.M1 \
	-f test/test25/kaem.M1 \
	-f test/test25/kaem-footer.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test25/kaem.hex2 || exit 3

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386-debug.hex2 \
	-f test/test25/kaem.hex2 \
	--LittleEndian \
	--Architecture 1 \
	--BaseAddress 0x8048000 \
	-o test/results/test25-binary \
	--exec_enable || exit 4

exit 0
