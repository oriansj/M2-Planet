#! /bin/sh
set -ex
# Build the test
bin/M2-Planet-gcc -f test/functions/file.c \
	-f test/functions/malloc.c \
	-f test/functions/calloc.c \
	-f test/functions/exit.c \
	-f test/functions/match.c \
	-f test/functions/numerate_number.c \
	-f test/functions/file_print.c \
	-f cc.h \
	-f test/functions/string.c \
	-f cc_reader.c \
	-f cc_strings.c \
	-f cc_types.c \
	-f functions/require_match.c \
	-f cc_core.c \
	-f cc.c \
	-o test/test100/cc.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f test/functions/libc-core.M1 \
	-f test/test100/cc.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test100/cc.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test100/cc.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test100-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the resulting file works
	./test/results/test100-binary -f test/functions/file.c \
		-f test/functions/malloc.c \
		-f test/functions/calloc.c \
		-f test/functions/exit.c \
		-f test/functions/match.c \
		-f test/functions/numerate_number.c \
		-f test/functions/file_print.c \
		-f cc.h \
		-f test/functions/string.c \
		-f cc_reader.c \
		-f cc_strings.c \
		-f cc_types.c \
		-f functions/require_match.c \
		-f cc_core.c \
		-f cc.c \
		-o test/test100/proof || exit 4

	out=$(sha256sum -c test/test100/proof.answer)
	[ "$out" = "test/test100/proof: OK" ] || exit 5
	cp test/results/test100-binary bin/M2-Planet
else
	cp bin/M2-Planet-gcc bin/M2-Planet
fi
exit 0
