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

TMPDIR="test/test0003/tmp-knight-native"
mkdir -p ${TMPDIR}

# Build the test
bin/M2-Planet \
	--architecture knight-native \
	-f test/common_knight/functions/putchar-native.c \
	-f test/common_knight/functions/exit-native.c \
	-f test/test0003/constant.c \
	-o ${TMPDIR}/constant.M1 \
	|| exit 1

# Macro assemble with libc written in M1-Macro
M1 \
	-f test/common_knight/knight-native_defs.M1 \
	-f test/common_knight/libc-native.M1 \
	-f ${TMPDIR}/constant.M1 \
	--BigEndian \
	--architecture knight-native \
	-o ${TMPDIR}/constant.hex2 \
	|| exit 2

# Resolve all linkages
hex2 \
	-f ${TMPDIR}/constant.hex2 \
	--BigEndian \
	--architecture knight-native \
	--BaseAddress 0x00 \
	-o test/results/test0003-knight-native-binary \
	|| exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "knight*" ]
then
	# Verify that the compiled program returns the correct result
	out=$(./test/results/test0003-knight-native-binary 2>&1 )
	[ 42 = $? ] || exit 4
	[ "$out" = "Hello mes" ] || exit 5
fi
exit 0
