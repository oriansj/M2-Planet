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

TMPDIR="test/test0022/tmp-x86"
mkdir -p ${TMPDIR}

# Build the test
bin/M2-Planet \
	--architecture x86 \
	-f test/common_x86/functions/malloc.c \
	-f test/common_x86/functions/file.c \
	-f test/common_x86/functions/exit.c \
	-f functions/in_set.c \
	-f functions/numerate_number.c \
	-f functions/calloc.c \
	-f functions/file_print.c \
	-f test/test0022/continue.c \
	--debug \
	--bootstrap-mode \
	-o ${TMPDIR}/continue.M1 \
	|| exit 1

# Build debug footer
blood-elf \
	-f ${TMPDIR}/continue.M1 \
	--entry _start \
	-o ${TMPDIR}/continue-footer.M1 \
	|| exit 2

# Macro assemble with libc written in M1-Macro
M1 \
	-f test/common_x86/x86_defs.M1 \
	-f test/common_x86/libc-core.M1 \
	-f ${TMPDIR}/continue.M1 \
	-f ${TMPDIR}/continue-footer.M1 \
	--LittleEndian \
	--architecture x86 \
	-o ${TMPDIR}/continue.hex2 \
	|| exit 3

# Resolve all linkages
hex2 \
	-f test/common_x86/ELF-i386-debug.hex2 \
	-f ${TMPDIR}/continue.hex2 \
	--LittleEndian \
	--architecture x86 \
	--BaseAddress 0x8048000 \
	-o test/results/test0022-x86-binary \
	--exec_enable \
	|| exit 4

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "x86" ]
then
	. ./sha256.sh
	# Verify that the resulting file works
	./test/results/test0022-x86-binary >| test/test0022/proof
	[ 0 = $? ] || exit 5
	out=$(sha256_check test/test0022/proof.answer)
	[ "$out" = "test/test0022/proof: OK" ] || exit 6
fi
exit 0
