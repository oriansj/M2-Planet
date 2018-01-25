#! /bin/sh
set -ex
# Build the test
bin/M2-Planet -f test/test06/for.c -o test/test06/for.M1 || exit 1
# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 -f test/common_x86/libc.M1 -f test/test06/for.M1 --LittleEndian --Architecture 1 -o test/test06/for.hex2 || exit 2
# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test06/for.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test06-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the resulting file works
	./test/results/test06-binary >| test/test06/proof || exit 4
	out=$(sha256sum -c test/test06/proof.answer)
	[ "$out" = "test/test06/proof: OK" ] || exit 5
fi
exit 0
