#! /bin/sh
## Copyright (C) 2017 Jeremiah Orians
## Copyright (C) 2021 deesix <deesix@tuta.io>
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

TMPDIR="test/test0100/tmp-amd64"
mkdir -p ${TMPDIR}

# Build the test
bin/M2-Planet \
	--architecture amd64 \
	-f M2libc/amd64/Linux/unistd.h \
	-f M2libc/stdlib.c \
	-f M2libc/amd64/Linux/fcntl.h \
	-f M2libc/stdio.c \
	-f functions/file_print.c \
	-f functions/match.c \
	-f test/test0100/blood-elf.c \
	--debug \
	-o ${TMPDIR}/blood-elf.M1 \
	|| exit 1

# Build debug footer
blood-elf \
	--64 \
	-f ${TMPDIR}/blood-elf.M1 \
	--entry _start \
	-o ${TMPDIR}/blood-elf-footer.M1 \
	|| exit 2

# Macro assemble with libc written in M1-Macro
M1 \
	-f M2libc/amd64/amd64_defs.M1 \
	-f M2libc/amd64/libc-full.M1 \
	-f ${TMPDIR}/blood-elf.M1 \
	-f ${TMPDIR}/blood-elf-footer.M1 \
	--little-endian \
	--architecture amd64 \
	-o ${TMPDIR}/blood-elf.hex2 \
	|| exit 3

# Resolve all linkages
hex2 \
	-f M2libc/amd64/ELF-amd64-debug.hex2 \
	-f ${TMPDIR}/blood-elf.hex2 \
	--little-endian \
	--architecture amd64 \
	--base-address 0x00600000 \
	-o test/results/test0100-amd64-binary \
	|| exit 4

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "amd64" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test0100-amd64-binary --version 2>&1 )
	[ 0 = $? ] || exit 5
	[ "$out" = "blood-elf 0.1
(Basically Launches Odd Object Dump ExecutabLe Files" ] || exit 6

	. ./sha256.sh
	# Verify that the resulting file works
	./test/results/test0100-amd64-binary -f test/test0100/test.M1 -o test/test0100/proof || exit 7
	out=$(sha256_check test/test0100/proof.answer)
	[ "$out" = "test/test0100/proof: OK" ] || exit 8
fi
exit 0
