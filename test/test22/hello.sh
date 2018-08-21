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
	-f functions/stat.c \
	-f test/test22/hex2_linker.c \
	--debug \
	-o test/test22/hex2_linker.M1 || exit 1

# Build debug footer
blood-elf -f test/test22/hex2_linker.M1 \
	-o test/test22/hex2_linker-footer.M1 || exit 2

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f functions/libc-core.M1 \
	-f test/test22/hex2_linker.M1 \
	-f test/test22/hex2_linker-footer.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test22/hex2_linker.hex2 || exit 3

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386-debug.hex2 \
	-f test/test22/hex2_linker.hex2 \
	--LittleEndian \
	--Architecture 1 \
	--BaseAddress 0x8048000 \
	-o test/results/test22-binary \
	--exec_enable || exit 4

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test22-binary --version 2>&1 )
	[ 0 = $? ] || exit 5
	[ "$out" = "hex2 0.3" ] || exit 6

	# Verify that the resulting file works
	./test/results/test22-binary -f test/common_x86/ELF-i386-debug.hex2 \
	-f test/test22/test.hex2 \
	--LittleEndian \
	--Architecture 1 \
	--BaseAddress 0x8048000 \
	-o test/test22/proof || exit 7
	out=$(sha256sum -c test/test22/proof.answer)
	[ "$out" = "test/test22/proof: OK" ] || exit 8
fi
exit 0
