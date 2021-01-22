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

TMPDIR="test/test0018/tmp-knight-native"
mkdir -p ${TMPDIR}

# Build the test
bin/M2-Planet \
	--architecture knight-native \
	-f test/common_knight/functions/file-native.c \
	-f test/common_knight/functions/malloc.c \
	-f functions/calloc.c \
	-f functions/match.c \
	-f test/test0018/math.c \
	--bootstrap-mode \
	-o ${TMPDIR}/math.M1 \
	|| exit 1

# Macro assemble with libc written in M1-Macro
M1 \
	-f test/common_knight/knight-native_defs.M1 \
	-f test/common_knight/libc-native-file.M1 \
	-f ${TMPDIR}/math.M1 \
	--BigEndian \
	--architecture knight-native \
	-o ${TMPDIR}/math.hex2 \
	|| exit 2

# Resolve all linkages
hex2 \
	-f ${TMPDIR}/math.hex2 \
	--BigEndian \
	--architecture knight-native \
	--BaseAddress 0x00 \
	-o test/results/test0018-knight-native-binary \
	|| exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "knight*" ]
then
	. ./sha256.sh
	# Verify that the resulting file works
	./test/results/test0018-knight-native-binary >| test/test0018/proof || exit 4
	out=$(sha256_check test/test0018/proof.answer)
	[ "$out" = "test/test0018/proof: OK" ] || exit 5
fi
exit 0
