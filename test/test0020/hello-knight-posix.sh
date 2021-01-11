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

TMPDIR="test/test0020/tmp-knight-posix"
mkdir -p ${TMPDIR}

# Build the test
bin/M2-Planet \
	--architecture knight-posix \
	-f test/common_knight/functions/putchar.c \
	-f test/common_knight/functions/exit.c \
	-f test/common_knight/functions/malloc.c \
	-f test/test0020/struct.c \
	--bootstrap-mode \
	-o ${TMPDIR}/struct.M1 \
	|| exit 1

# Macro assemble with libc written in M1-Macro
M1 \
	-f test/common_knight/knight_defs.M1 \
	-f test/common_knight/libc-core.M1 \
	-f ${TMPDIR}/struct.M1 \
	--BigEndian \
	--architecture knight-posix \
	-o ${TMPDIR}/struct.hex2 \
	|| exit 2

# Resolve all linkages
hex2 \
	-f test/common_knight/ELF-knight.hex2 \
	-f ${TMPDIR}/struct.hex2 \
	--BigEndian \
	--architecture knight-posix \
	--BaseAddress 0x00 \
	-o test/results/test0020-knight-posix-binary \
	--exec_enable \
	|| exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "knight*" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test0020-knight-posix-binary 2>&1 )
	[ 20 = $? ] || exit 4
	[ "$out" = "35419896642975313541989657891634" ] || exit 5
fi
exit 0
