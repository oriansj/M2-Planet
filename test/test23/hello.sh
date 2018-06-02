#! /bin/sh
set -x
# Build the test
./bin/M2-Planet -f test/functions/exit.c \
	-f test/functions/file.c \
	-f test/functions/file_print.c \
	-f test/functions/malloc.c \
	-f test/functions/calloc.c \
	-f test/functions/match.c \
	-f test/functions/numerate_number.c \
	-f test/functions/string.c \
	-f test/test23/M1-macro.c \
	-o test/test23/M1-macro.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f test/functions/libc-core.M1 \
	-f test/test23/M1-macro.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test23/M1-macro.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 \
	-f test/test23/M1-macro.hex2 \
	--LittleEndian \
	--Architecture 1 \
	--BaseAddress 0x8048000 \
	-o test/results/test23-binary \
	--exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test23-binary --version 2>&1 )
	[ 0 = $? ] || exit 4
	[ "$out" = "M1 0.3" ] || exit 5

	# Verify that the resulting file works
	./test/results/test23-binary -f \
		test/common_x86/x86_defs.M1 \
		-f test/functions/libc-core.M1 \
		-f test/test21/test.M1 \
		--LittleEndian \
		--Architecture 1 \
		-o test/test23/proof || exit 6

	out=$(sha256sum -c test/test23/proof.answer)
	[ "$out" = "test/test23/proof: OK" ] || exit 7
fi
exit 0
