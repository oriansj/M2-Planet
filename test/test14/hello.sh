#! /bin/sh
set -ex
# Build the test
bin/M2-Planet -f functions/putchar.c \
	-f test/test14/basic_args.c \
	-o test/test14/basic_args.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f functions/libc-core.M1 \
	-f test/test14/basic_args.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test14/basic_args.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test14/basic_args.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test14-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the resulting file works
	./test/results/test14-binary 314 1 5926 5 35897 932384626 43 383279 50288 419 71693 99375105 820974944 >| test/test14/proof || exit 4
	out=$(sha256sum -c test/test14/proof.answer)
	[ "$out" = "test/test14/proof: OK" ] || exit 5
fi
exit 0
