#! /bin/sh
set -ex
# Build the test
bin/M2-Planet test/test1/for.c test/test1/for.M1 || exit 1
# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 -f test/common_x86/libc.M1 -f test/test1/for.M1 --LittleEndian --Architecture 1 -o test/test1/for.hex2 || exit 2
# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test1/for.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test1-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the resulting file works
	./test/results/test1-binary >| test/test1/proof || exit 4
	out=$(sha256sum -c test/test1/proof.answer)
	[ "$out" = "test/test1/proof: OK" ] || exit 5
fi
exit 0
