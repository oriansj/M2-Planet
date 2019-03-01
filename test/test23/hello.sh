#! /bin/sh
## Copyright (C) 2017 Jeremiah Orians
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

set -x
# Build the test
./bin/M2-Planet --architecture x86 -f test/common_x86/functions/exit.c \
	-f test/common_x86/functions/file.c \
	-f functions/file_print.c \
	-f test/common_x86/functions/malloc.c \
	-f functions/calloc.c \
	-f functions/match.c \
	-f functions/in_set.c \
	-f functions/numerate_number.c \
	-f functions/string.c \
	-f test/test23/M1-macro.c \
	--debug \
	-o test/test23/M1-macro.M1 || exit 1

# Build debug footer
blood-elf -f test/test23/M1-macro.M1 \
	-o test/test23/M1-macro-footer.M1 || exit 2

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f test/common_x86/libc-core.M1 \
	-f test/test23/M1-macro.M1 \
	-f test/test23/M1-macro-footer.M1 \
	--LittleEndian \
	--architecture x86 \
	-o test/test23/M1-macro.hex2 || exit 3

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386-debug.hex2 \
	-f test/test23/M1-macro.hex2 \
	--LittleEndian \
	--architecture x86 \
	--BaseAddress 0x8048000 \
	-o test/results/test23-binary \
	--exec_enable || exit 4

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "x86" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test23-binary --version 2>&1 )
	[ 0 = $? ] || exit 5
	[ "$out" = "M1 0.3" ] || exit 6

	# Verify that the resulting file works
	./test/results/test23-binary -f \
		test/common_x86/x86_defs.M1 \
		-f test/common_x86/libc-core.M1 \
		-f test/test21/test.M1 \
		--LittleEndian \
		--architecture x86 \
		-o test/test23/proof || exit 7

	out=$(sha256sum -c test/test23/proof.answer)
	[ "$out" = "test/test23/proof: OK" ] || exit 8
fi
exit 0
