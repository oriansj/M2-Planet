#! /bin/sh
set -ex
# Build the test
bin/M2-Planet -f test/functions/file.c \
	-f test/functions/malloc.c \
	-f test/functions/calloc.c \
	-f test/functions/exit.c \
	-f test/functions/match.c \
	-f test/functions/numerate_number.c \
	-f test/test19/getopt.c \
	-o test/test19/getopt.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f test/functions/libc-core.M1 \
	-f test/test19/getopt.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test19/getopt.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test19/getopt.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test19-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the resulting file works
	./test/results/test19-binary -f test/test19/input -o test/test19/proof || exit 4
	out=$(sha256sum -c test/test19/proof.answer)
	[ "$out" = "test/test19/proof: OK" ] || exit 5
fi
exit 0
