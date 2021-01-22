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

TMPDIR="test/test0015/tmp-x86"
mkdir -p ${TMPDIR}

# Build the test
bin/M2-Planet \
	--architecture x86 \
	-f M2libc/x86/Linux/unistd.h \
	-f M2libc/stdlib.c \
	-f M2libc/x86/Linux/fcntl.h \
	-f M2libc/stdio.c \
	-f test/test0015/file_read.c \
	-o ${TMPDIR}/file_read.M1 \
	|| exit 1

# Macro assemble with libc written in M1-Macro
M1 \
	-f M2libc/x86/x86_defs.M1 \
	-f M2libc/x86/libc-full.M1 \
	-f ${TMPDIR}/file_read.M1 \
	--little-endian \
	--architecture x86 \
	-o ${TMPDIR}/file_read.hex2 \
	|| exit 2

# Resolve all linkages
hex2 \
	-f M2libc/x86/ELF-x86.hex2 \
	-f ${TMPDIR}/file_read.hex2 \
	--little-endian \
	--architecture x86 \
	--base-address 0x8048000 \
	-o test/results/test0015-x86-binary \
	|| exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "x86" ]
then
	. ./sha256.sh
	# Verify that the resulting file works
	./test/results/test0015-x86-binary test/test0015/file_read.c >| test/test0015/proof || exit 4
	out=$(sha256_check test/test0015/proof.answer)
	[ "$out" = "test/test0015/proof: OK" ] || exit 5
fi
exit 0
