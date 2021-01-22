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

TMPDIR="test/test0022/tmp-aarch64"
mkdir -p ${TMPDIR}

# Build the test
bin/M2-Planet \
	--architecture aarch64 \
	-f M2libc/aarch64/Linux/unistd.h \
	-f M2libc/stdlib.c \
	-f M2libc/aarch64/Linux/fcntl.h \
	-f M2libc/stdio.c \
	-f functions/in_set.c \
	-f functions/numerate_number.c \
	-f functions/file_print.c \
	-f test/test0022/continue.c \
	--debug \
	-o ${TMPDIR}/continue.M1 \
	|| exit 1

# Build debug footer
blood-elf \
	--64 \
	-f ${TMPDIR}/continue.M1 \
	--entry _start \
	-o ${TMPDIR}/continue-footer.M1 \
	|| exit 2

# Macro assemble with libc written in M1-Macro
M1 \
	-f M2libc/aarch64/aarch64_defs.M1 \
	-f M2libc/aarch64/libc-full.M1 \
	-f ${TMPDIR}/continue.M1 \
	-f ${TMPDIR}/continue-footer.M1 \
	--little-endian \
	--architecture aarch64 \
	-o ${TMPDIR}/continue.hex2 \
	|| exit 3

# Resolve all linkages
hex2 \
	-f M2libc/aarch64/ELF-aarch64-debug.hex2 \
	-f ${TMPDIR}/continue.hex2 \
	--little-endian \
	--architecture aarch64 \
	--base-address 0x400000 \
	-o test/results/test0022-aarch64-binary \
	|| exit 4

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "aarch64" ]
then
	. ./sha256.sh
	# Verify that the resulting file works
	./test/results/test0022-aarch64-binary >| test/test0022/proof
	[ 0 = $? ] || exit 5
	out=$(sha256_check test/test0022/proof.answer)
	[ "$out" = "test/test0022/proof: OK" ] || exit 6
fi
exit 0
