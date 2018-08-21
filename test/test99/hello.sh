#! /bin/sh
set -ex
# Build the test
bin/M2-Planet -f functions/putchar.c \
	-f functions/getchar.c \
	-f functions/exit.c \
	-f functions/malloc.c \
	-f test/test99/cc500.c \
	-o test/test99/cc0.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f functions/libc-core.M1 \
	-f test/test99/cc0.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test99/cc0.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test99/cc0.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test99-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the compiled program can compile itself
	./test/results/test99-binary < test/test99/cc500.c >| test/test99/cc1 || exit 4
	out=$(sha256sum -c test/test99/proof0.answer)
	[ "$out" = "test/test99/cc1: OK" ] || exit 5

	# Make it executable
	exec_enable test/test99/cc1

	# Verify that the result of it compiling itself can compile itself
	./test/test99/cc1 < test/test99/cc500.c >| test/test99/cc2 || exit 6
	out=$(sha256sum -c test/test99/proof1.answer)
	[ "$out" = "test/test99/cc2: OK" ] || exit 7
fi
exit 0
