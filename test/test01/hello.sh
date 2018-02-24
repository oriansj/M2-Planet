#! /bin/sh
set -x
# Build the test
bin/M2-Planet -f test/functions/putchar.c \
	-f test/functions/exit.c \
	-f test/test01/library_call.c \
	-o test/test01/library_call.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f test/functions/libc-core.M1 \
	-f test/test01/library_call.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test01/library_call.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test01/library_call.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test01-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test01-binary 2>&1)
	[ 42 = $? ] || exit 3
	[ "$out" = "Hello mes" ] || exit 4
fi
exit 0
