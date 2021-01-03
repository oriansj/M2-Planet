#! /bin/sh
## Copyright (C) 2017 Jeremiah Orians
## Copyright (C) 2020 deesix <deesix@tuta.io>
## This file is part of M2-Planet.
##
## M2-Planet is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## M2-Planet is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with M2-Planet.  If not, see <http://www.gnu.org/licenses/>.

set -ex
# Build the test
./bin/M2-Planet --architecture aarch64 \
	-f test/common_aarch64/functions/file.c \
	-f test/common_aarch64/functions/malloc.c \
	-f functions/calloc.c \
	-f test/common_aarch64/functions/exit.c \
	-f functions/match.c \
	-f functions/in_set.c \
	-f functions/numerate_number.c \
	-f functions/file_print.c \
	-f functions/number_pack.c \
	-f functions/string.c \
	-f functions/require.c \
	-f test/common_aarch64/functions/fixup.c \
	-f cc.h \
	-f cc_globals.c \
	-f cc_reader.c \
	-f cc_strings.c \
	-f cc_types.c \
	-f cc_core.c \
	-f cc.c \
	--debug \
	--bootstrap-mode \
	-o test/test1000/cc.M1 || exit 1

# Build debug footer
blood-elf --64 -f test/test1000/cc.M1 \
	--entry _start \
	-o test/test1000/cc-footer.M1 || exit 2

# Macro assemble with libc written in M1-Macro
M1 -f test/common_aarch64/aarch64_defs.M1 \
	-f test/common_aarch64/libc-core.M1 \
	-f test/test1000/cc.M1 \
	-f test/test1000/cc-footer.M1 \
	--LittleEndian \
	--architecture aarch64 \
	-o test/test1000/cc.hex2 || exit 3

# Resolve all linkages
hex2 -f test/common_aarch64/ELF-aarch64-debug.hex2 \
	-f test/test1000/cc.hex2 \
	--LittleEndian \
	--architecture aarch64 \
	--BaseAddress 0x400000 \
	-o test/results/test1000-aarch64-binary --exec_enable || exit 4

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "aarch64" ]
then
	# Verify that the resulting file works
	./test/results/test1000-aarch64-binary --architecture x86 \
		-f test/common_x86/functions/file.c \
		-f test/common_x86/functions/malloc.c \
		-f functions/calloc.c \
		-f test/common_x86/functions/exit.c \
		-f functions/match.c \
		-f functions/in_set.c \
		-f functions/numerate_number.c \
		-f functions/file_print.c \
		-f functions/number_pack.c \
		-f functions/string.c \
		-f functions/require.c \
		-f cc.h \
		-f cc_globals.c \
		-f cc_reader.c \
		-f cc_strings.c \
		-f cc_types.c \
		-f cc_core.c \
		-f cc.c \
		--bootstrap-mode \
		-o test/test1000/proof || exit 5

	. ./sha256.sh
	out=$(sha256_check test/test1000/proof.answer)
	[ "$out" = "test/test1000/proof: OK" ] || exit 6
	[ ! -e bin/M2-Planet ] && mv test/results/test1000-x86-binary bin/M2-Planet
fi
exit 0
