#! /bin/sh
set -ex
# Build the test
bin/M2-Planet -f test/test11/break-do.c -o test/test11/break-do.M1 || exit 1
# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 -f test/common_x86/libc.M1 -f test/test11/break-do.M1 --LittleEndian --Architecture 1 -o test/test11/break-do.hex2 || exit 2
# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test11/break-do.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test11-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the resulting file works
	./test/results/test11-binary >| test/test11/proof || exit 4
	out=$(sha256sum -c test/test11/proof.answer)
	[ "$out" = "test/test11/proof: OK" ] || exit 5
fi
exit 0
