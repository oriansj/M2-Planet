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

set -ex

TMPDIR="test/test0025/tmp-x86"
mkdir -p ${TMPDIR}

# Build the test
bin/M2-Planet \
	--architecture x86 \
	-f M2libc/x86/Linux/unistd.h \
	-f M2libc/stdlib.c \
	-f M2libc/x86/Linux/fcntl.h \
	-f M2libc/stdio.c \
	-f test/test0025/array.c \
	--debug \
	-o ${TMPDIR}/array.M1 \
	|| exit 1

# Build debug footer
blood-elf \
	-f ${TMPDIR}/array.M1 \
	--entry _start \
	-o ${TMPDIR}/array-footer.M1 \
	|| exit 2

# Macro assemble with libc written in M1-Macro
M1 \
	-f M2libc/x86/x86_defs.M1 \
	-f M2libc/x86/libc-full.M1 \
	-f ${TMPDIR}/array.M1 \
	-f ${TMPDIR}/array-footer.M1 \
	--LittleEndian \
	--architecture x86 \
	-o ${TMPDIR}/array.hex2 \
	|| exit 2

# Resolve all linkages
hex2 \
	-f M2libc/x86/ELF-x86-debug.hex2 \
	-f ${TMPDIR}/array.hex2 \
	--LittleEndian \
	--architecture x86 \
	--BaseAddress 0x8048000 \
	-o test/results/test0025-x86-binary \
	|| exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "x86" ]
then
	. ./sha256.sh
	# Verify that the resulting file works
	./test/results/test0025-x86-binary 2>| test/test0025/proof || exit 4
	out=$(sha256_check test/test0025/proof.answer)
	[ "$out" = "test/test0025/proof: OK" ] || exit 5
fi
exit 0
