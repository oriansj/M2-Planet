#! /bin/sh
set -x
# Build the test
bin/M2-Planet -f test/functions/putchar.c \
	-f test/functions/exit.c \
	-f test/functions/malloc.c \
	-f test/test10/nested_struct.c \
	-o test/test10/nested_struct.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f test/functions/libc-core.M1 \
	-f test/test10/nested_struct.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test10/nested_struct.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test10/nested_struct.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test10-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test10-binary 2>&1 )
	[ 12 = $? ] || exit 4
	[ "$out" = "35419896642975313541989657891634" ] || exit 5
fi
exit 0
