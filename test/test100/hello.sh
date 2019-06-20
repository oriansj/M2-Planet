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

set -ex
# Build using seed if possible
if [ -f bin/M2-Planet-seed ]
then
	[ ! -f test/results ] && mkdir -p test/results
	cp bin/M2-Planet-seed bin/M2-Planet

	if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "amd64" ]
	then
		./test/test100/hello-amd64.sh
		mv test/results/test100-amd64-binary bin/M2-Planet
	elif [ "$(get_machine ${GET_MACHINE_FLAGS})" = "x86" ]
	then
		./test/test100/hello-x86.sh
		mv test/results/test100-x86-binary bin/M2-Planet
	elif [ "$(get_machine ${GET_MACHINE_FLAGS})" = "armv7l" ]
	then
		./test/test100/hello-armv7l.sh
		mv test/results/test100-armv7l-binary bin/M2-Planet
	fi

else
[ -z "${CC+x}" ] && export CC=gcc
[ -z "${CFLAGS+x}" ] && export CFLAGS=" -D_GNU_SOURCE -O0 -std=c99 -ggdb"

${CC} ${CFLAGS} \
	functions/match.c \
	functions/in_set.c \
	functions/numerate_number.c \
	functions/file_print.c \
	functions/number_pack.c \
	functions/fixup.c \
	functions/string.c \
	cc_reader.c \
	cc_strings.c \
	cc_types.c \
	cc_core.c \
	cc.c \
	cc.h \
	gcc_req.h \
	-o bin/M2-Planet || exit 5
fi

exit 0
