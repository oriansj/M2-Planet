#! /bin/sh
set -x
# Build the test
./bin/M2-Planet -f test/functions/exit.c \
	-f test/functions/file.c \
	-f test/functions/file_print.c \
	-f test/functions/malloc.c \
	-f test/functions/calloc.c \
	-f test/functions/uname.c \
	-f test/test24/get_machine.c \
	--debug \
	-o test/test24/get_machine.M1 || exit 1

# Build debug footer
blood-elf -f test/test24/get_machine.M1 \
	-o test/test24/get_machine-footer.M1 || exit 2

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f test/functions/libc-core.M1 \
	-f test/test24/get_machine.M1 \
	-f test/test24/get_machine-footer.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test24/get_machine.hex2 || exit 3

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386-debug.hex2 \
	-f test/test24/get_machine.hex2 \
	--LittleEndian \
	--Architecture 1 \
	--BaseAddress 0x8048000 \
	-o test/results/test24-binary \
	--exec_enable || exit 4

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test24-binary 2>&1 )
	[ 0 = $? ] || exit 5
	[ "$out" = "x86_64" ] || exit 6
fi
exit 0
