#! /bin/sh
set -ex
# Build the test
bin/M2-Planet -f test/functions/file.c \
	-f test/functions/putchar.c \
	-f test/test16/file_write.c \
	-o test/test16/file_write.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f test/functions/libc-core.M1 \
	-f test/test16/file_write.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test16/file_write.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test16/file_write.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test16-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the resulting file works
	./test/results/test16-binary test/test16/proof || exit 4
	out=$(sha256sum -c test/test16/proof.answer)
	[ "$out" = "test/test16/proof: OK" ] || exit 5
fi
exit 0
