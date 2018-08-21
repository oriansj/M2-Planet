#! /bin/sh
set -x
# Build the test
./bin/M2-Planet -f functions/exit.c \
	-f functions/file.c \
	-f functions/file_print.c \
	-f functions/malloc.c \
	-f functions/calloc.c \
	-f functions/match.c \
	-f test/test21/blood-elf.c \
	--debug \
	-o test/test21/blood-elf.M1 || exit 1

# Build debug footer
blood-elf -f test/test21/blood-elf.M1 \
	-o test/test21/blood-elf-footer.M1 || exit 2

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f functions/libc-core.M1 \
	-f test/test21/blood-elf.M1 \
	-f test/test21/blood-elf-footer.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test21/blood-elf.hex2 || exit 3

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386-debug.hex2 \
	 -f test/test21/blood-elf.hex2 \
	 --LittleEndian \
	 --Architecture 1 \
	 --BaseAddress 0x8048000 \
	 -o test/results/test21-binary \
	 --exec_enable || exit 4

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test21-binary --version 2>&1 )
	[ 0 = $? ] || exit 5
	[ "$out" = "blood-elf 0.1
(Basically Launches Odd Object Dump ExecutabLe Files" ] || exit 6

	# Verify that the resulting file works
	./test/results/test21-binary -f test/test21/test.M1 -o test/test21/proof || exit 7
	out=$(sha256sum -c test/test21/proof.answer)
	[ "$out" = "test/test21/proof: OK" ] || exit 8
fi
exit 0
