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
./bin/M2-Planet --architecture armv7l -f test/common_armv7l/functions/exit.c \
	-f test/common_armv7l/functions/file.c \
	-f functions/file_print.c \
	-f test/common_armv7l/functions/malloc.c \
	-f functions/calloc.c \
	-f functions/match.c \
	-f test/test21/blood-elf.c \
	--debug \
	-o test/test21/blood-elf.M1 || exit 1

# Build debug footer
blood-elf -f test/test21/blood-elf.M1 \
	-o test/test21/blood-elf-footer.M1 || exit 2

# Macro assemble with libc written in M1-Macro
M1 -f test/common_armv7l/armv7l_defs.M1 \
	-f test/common_armv7l/libc-core.M1 \
	-f test/test21/blood-elf.M1 \
	-f test/test21/blood-elf-footer.M1 \
	--LittleEndian \
	--architecture armv7l \
	-o test/test21/blood-elf.hex2 || exit 3

# Resolve all linkages
hex2 -f test/common_armv7l/ELF-armv7l-debug.hex2 \
	 -f test/test21/blood-elf.hex2 \
	 --LittleEndian \
	 --architecture armv7l \
	 --BaseAddress 0x10000 \
	 -o test/results/test21-armv7l-binary \
	 --exec_enable || exit 4

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "armv7l" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test21-armv7l-binary --version 2>&1 )
	[ 0 = $? ] || exit 5
	[ "$out" = "blood-elf 0.1
(Basically Launches Odd Object Dump ExecutabLe Files" ] || exit 6

	. ./sha256.sh
	# Verify that the resulting file works
	./test/results/test21-armv7l-binary -f test/test21/test.M1 -o test/test21/proof || exit 7
	out=$(sha256_check test/test21/proof.answer)
	[ "$out" = "test/test21/proof: OK" ] || exit 8
fi
exit 0
