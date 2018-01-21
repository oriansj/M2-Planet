#! /bin/sh
set -x
# Build the test
bin/M2-Planet -f test/test0/return.c -o test/test0/return.M1 || exit 1
# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 -f test/common_x86/libc.M1 -f test/test0/return.M1 --LittleEndian --Architecture 1 -o test/test0/return.hex2 || exit 2
# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test0/return.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test0-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the compiled program returns the correct result
	./test/results/test0-binary
	[ 42 = $? ] || exit 3
fi
exit 0
