#! /bin/sh
set -ex
# Build the test
bin/M2-Planet test/test0/cc500.c test/test0/cc0.M1 || exit 1
# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 -f test/common_x86/libc.M1 -f test/test0/cc0.M1 --LittleEndian --Architecture 1 -o test/test0/cc0.hex2 || exit 2
# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test0/cc0.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test0-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the compiled program can compile itself
	./test/results/test0-binary < test/test0/cc500.c >| test/test0/cc1 || exit 4
	out=$(sha256sum -c test/test0/proof0.answer)
	[ "$out" = "test/test0/cc1: OK" ] || exit 5

	# Make it executable
	exec_enable test/test0/cc1

	# Verify that the result of it compiling itself can compile itself
	./test/test0/cc1 < test/test0/cc500.c >| test/test0/cc2 || exit 6
	out=$(sha256sum -c test/test0/proof1.answer)
	[ "$out" = "test/test0/cc2: OK" ] || exit 7
fi
exit 0
