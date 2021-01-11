## Copyright (C) 2017 Jeremiah Orians
## Copyright (C) 2020-2021 deesix <deesix@tuta.io>
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

# Prevent rebuilding
VPATH = bin:test:test/results
PACKAGE = m2-planet

# C compiler settings
CC?=gcc
CFLAGS:=$(CFLAGS) -D_GNU_SOURCE -O0 -std=c99 -ggdb

all: M2-Planet

M2-Planet: bin results cc.h cc_reader.c cc_strings.c cc_types.c cc_core.c cc.c cc_globals.c cc_globals.h
	$(CC) $(CFLAGS) \
	functions/match.c \
	functions/in_set.c \
	functions/numerate_number.c \
	functions/file_print.c \
	functions/number_pack.c \
	functions/string.c \
	functions/require.c \
	functions/fixup.c \
	cc_reader.c \
	cc_strings.c \
	cc_types.c \
	cc_core.c \
	cc_macro.c \
	cc.c \
	cc.h \
	cc_globals.c \
	gcc_req.h \
	-o bin/M2-Planet

M2-minimal: bin results cc.h cc_reader.c cc_strings.c cc_types.c cc_core.c cc-minimal.c
	$(CC) $(CFLAGS) \
	functions/match.c \
	functions/in_set.c \
	functions/numerate_number.c \
	functions/file_print.c \
	functions/number_pack.c \
	functions/string.c \
	functions/require.c \
	functions/fixup.c \
	cc_reader.c \
	cc_strings.c \
	cc_types.c \
	cc_core.c \
	cc-minimal.c \
	cc.h \
	cc_globals.c \
	gcc_req.h \
	-o bin/M2-minimal

# Clean up after ourselves
.PHONY: clean
clean:
	rm -rf bin/ test/results/
	./test/cleanup_test.sh 0000
	./test/cleanup_test.sh 0001
	./test/cleanup_test.sh 0002
	./test/cleanup_test.sh 0003
	./test/cleanup_test.sh 0004
	./test/cleanup_test.sh 0005
	./test/cleanup_test.sh 0006
	./test/cleanup_test.sh 0007
	./test/cleanup_test.sh 0008
	./test/cleanup_test.sh 0009
	./test/cleanup_test.sh 0010
	./test/cleanup_test.sh 0011
	./test/cleanup_test.sh 0012
	./test/cleanup_test.sh 0013
	./test/cleanup_test.sh 0014
	./test/cleanup_test.sh 0015
	./test/cleanup_test.sh 0016
	./test/cleanup_test.sh 0017
	./test/cleanup_test.sh 0018
	./test/cleanup_test.sh 0019
	./test/cleanup_test.sh 0020
	./test/cleanup_test.sh 0021
	./test/cleanup_test.sh 0022
	./test/cleanup_test.sh 0023
	./test/cleanup_test.sh 0024
	./test/cleanup_test.sh 0025
	./test/cleanup_test.sh 0100
	./test/cleanup_test.sh 0101
	./test/cleanup_test.sh 0102
	./test/cleanup_test.sh 0103
	./test/cleanup_test.sh 0104
	./test/cleanup_test.sh 0105
	./test/cleanup_test.sh 0106
	./test/cleanup_test.sh 1000

# Directories
bin:
	mkdir -p bin

results:
	mkdir -p test/results

# tests
test: aarch64-tests amd64-tests knight-posix-tests knight-native-tests armv7l-tests x86-tests | results
	sha256sum -c test/test.answers

aarch64-tests: \
	test0000-aarch64-binary \
	test0001-aarch64-binary \
	test0002-aarch64-binary \
	test0003-aarch64-binary \
	test0004-aarch64-binary \
	test0005-aarch64-binary \
	test0006-aarch64-binary \
	test0007-aarch64-binary \
	test0008-aarch64-binary \
	test0009-aarch64-binary \
	test0010-aarch64-binary \
	test0011-aarch64-binary \
	test0012-aarch64-binary \
	test0013-aarch64-binary \
	test0014-aarch64-binary \
	test0015-aarch64-binary \
	test0016-aarch64-binary \
	test0017-aarch64-binary \
	test0018-aarch64-binary \
	test0019-aarch64-binary \
	test0020-aarch64-binary \
	test0021-aarch64-binary \
	test0022-aarch64-binary \
	test0023-aarch64-binary \
	test0024-aarch64-binary \
	test0025-aarch64-binary \
	test0100-aarch64-binary \
	test0101-aarch64-binary \
	test0102-aarch64-binary \
	test0103-aarch64-binary \
	test0104-aarch64-binary \
	test0105-aarch64-binary \
	test0106-aarch64-binary \
	test1000-aarch64-binary | results

amd64-tests: \
	test0000-amd64-binary \
	test0001-amd64-binary \
	test0002-amd64-binary \
	test0003-amd64-binary \
	test0004-amd64-binary \
	test0005-amd64-binary \
	test0006-amd64-binary \
	test0007-amd64-binary \
	test0008-amd64-binary \
	test0009-amd64-binary \
	test0010-amd64-binary \
	test0011-amd64-binary \
	test0012-amd64-binary \
	test0013-amd64-binary \
	test0014-amd64-binary \
	test0015-amd64-binary \
	test0016-amd64-binary \
	test0017-amd64-binary \
	test0018-amd64-binary \
	test0019-amd64-binary \
	test0020-amd64-binary \
	test0021-amd64-binary \
	test0022-amd64-binary \
	test0023-amd64-binary \
	test0024-amd64-binary \
	test0025-amd64-binary \
	test0100-amd64-binary \
	test0101-amd64-binary \
	test0102-amd64-binary \
	test0103-amd64-binary \
	test0104-amd64-binary \
	test0105-amd64-binary \
	test0106-amd64-binary \
	test1000-amd64-binary | results

knight-posix-tests: \
	test0000-knight-posix-binary \
	test0001-knight-posix-binary \
	test0002-knight-posix-binary \
	test0003-knight-posix-binary \
	test0004-knight-posix-binary \
	test0005-knight-posix-binary \
	test0006-knight-posix-binary \
	test0007-knight-posix-binary \
	test0008-knight-posix-binary \
	test0009-knight-posix-binary \
	test0010-knight-posix-binary \
	test0011-knight-posix-binary \
	test0012-knight-posix-binary \
	test0013-knight-posix-binary \
	test0014-knight-posix-binary \
	test0015-knight-posix-binary \
	test0016-knight-posix-binary \
	test0017-knight-posix-binary \
	test0018-knight-posix-binary \
	test0019-knight-posix-binary \
	test0020-knight-posix-binary \
	test0021-knight-posix-binary \
	test0022-knight-posix-binary \
	test0023-knight-posix-binary \
	test0024-knight-posix-binary \
	test0100-knight-posix-binary \
	test0101-knight-posix-binary \
	test0102-knight-posix-binary \
	test0103-knight-posix-binary \
	test0106-knight-posix-binary \
	test1000-knight-posix-binary | results

knight-native-tests: \
	test0000-knight-native-binary\
	test0001-knight-native-binary\
	test0002-knight-native-binary\
	test0003-knight-native-binary\
	test0004-knight-native-binary\
	test0005-knight-native-binary\
	test0006-knight-native-binary\
	test0007-knight-native-binary\
	test0008-knight-native-binary\
	test0009-knight-native-binary\
	test0010-knight-native-binary\
	test0011-knight-native-binary\
	test0012-knight-native-binary\
	test0013-knight-native-binary\
	test0017-knight-native-binary\
	test0018-knight-native-binary\
	test0020-knight-native-binary\
	test0024-knight-native-binary\
	test0106-knight-native-binary | results

armv7l-tests: \
	test0000-armv7l-binary \
	test0001-armv7l-binary \
	test0002-armv7l-binary \
	test0003-armv7l-binary \
	test0004-armv7l-binary \
	test0005-armv7l-binary \
	test0006-armv7l-binary \
	test0007-armv7l-binary \
	test0008-armv7l-binary \
	test0009-armv7l-binary \
	test0010-armv7l-binary \
	test0011-armv7l-binary \
	test0012-armv7l-binary \
	test0013-armv7l-binary \
	test0014-armv7l-binary \
	test0015-armv7l-binary \
	test0016-armv7l-binary \
	test0017-armv7l-binary \
	test0018-armv7l-binary \
	test0019-armv7l-binary \
	test0020-armv7l-binary \
	test0021-armv7l-binary \
	test0022-armv7l-binary \
	test0023-armv7l-binary \
	test0024-armv7l-binary \
	test0025-armv7l-binary \
	test0100-armv7l-binary \
	test0101-armv7l-binary \
	test0102-armv7l-binary \
	test0103-armv7l-binary \
	test0104-armv7l-binary \
	test0105-armv7l-binary \
	test0106-armv7l-binary \
	test1000-armv7l-binary | results

x86-tests: \
	test0000-x86-binary \
	test0001-x86-binary \
	test0002-x86-binary \
	test0003-x86-binary \
	test0004-x86-binary \
	test0005-x86-binary \
	test0006-x86-binary \
	test0007-x86-binary \
	test0008-x86-binary \
	test0009-x86-binary \
	test0010-x86-binary \
	test0011-x86-binary \
	test0012-x86-binary \
	test0013-x86-binary \
	test0014-x86-binary \
	test0015-x86-binary \
	test0016-x86-binary \
	test0017-x86-binary \
	test0018-x86-binary \
	test0019-x86-binary \
	test0020-x86-binary \
	test0021-x86-binary \
	test0022-x86-binary \
	test0023-x86-binary \
	test0024-x86-binary \
	test0025-x86-binary \
	test0100-x86-binary \
	test0101-x86-binary \
	test0102-x86-binary \
	test0103-x86-binary \
	test0104-x86-binary \
	test0105-x86-binary \
	test0106-x86-binary \
	test1000-x86-binary | results

test0000-aarch64-binary: M2-Planet | results
	test/test0000/hello-aarch64.sh

test0001-aarch64-binary: M2-Planet | results
	test/test0001/hello-aarch64.sh

test0002-aarch64-binary: M2-Planet | results
	test/test0002/hello-aarch64.sh

test0003-aarch64-binary: M2-Planet | results
	test/test0003/hello-aarch64.sh

test0004-aarch64-binary: M2-Planet | results
	test/test0004/hello-aarch64.sh

test0005-aarch64-binary: M2-Planet | results
	test/test0005/hello-aarch64.sh

test0006-aarch64-binary: M2-Planet | results
	test/test0006/hello-aarch64.sh

test0007-aarch64-binary: M2-Planet | results
	test/test0007/hello-aarch64.sh

test0008-aarch64-binary: M2-Planet | results
	test/test0008/hello-aarch64.sh

test0009-aarch64-binary: M2-Planet | results
	test/test0009/hello-aarch64.sh

test0010-aarch64-binary: M2-Planet | results
	test/test0010/hello-aarch64.sh

test0011-aarch64-binary: M2-Planet | results
	test/test0011/hello-aarch64.sh

test0012-aarch64-binary: M2-Planet | results
	test/test0012/hello-aarch64.sh

test0013-aarch64-binary: M2-Planet | results
	test/test0013/hello-aarch64.sh

test0014-aarch64-binary: M2-Planet | results
	test/test0014/hello-aarch64.sh

test0015-aarch64-binary: M2-Planet | results
	test/test0015/hello-aarch64.sh

test0016-aarch64-binary: M2-Planet | results
	test/test0016/hello-aarch64.sh

test0017-aarch64-binary: M2-Planet | results
	test/test0017/hello-aarch64.sh

test0018-aarch64-binary: M2-Planet | results
	test/test0018/hello-aarch64.sh

test0019-aarch64-binary: M2-Planet | results
	test/test0019/hello-aarch64.sh

test0020-aarch64-binary: M2-Planet | results
	test/test0020/hello-aarch64.sh

test0021-aarch64-binary: M2-Planet | results
	test/test0021/hello-aarch64.sh

test0022-aarch64-binary: M2-Planet | results
	test/test0022/hello-aarch64.sh

test0023-aarch64-binary: M2-Planet | results
	test/test0023/hello-aarch64.sh

test0024-aarch64-binary: M2-Planet | results
	test/test0024/hello-aarch64.sh

test0025-aarch64-binary: M2-Planet | results
	test/test0025/hello-aarch64.sh

test0100-aarch64-binary: M2-Planet | results
	test/test0100/hello-aarch64.sh

test0101-aarch64-binary: M2-Planet | results
	test/test0101/hello-aarch64.sh

test0102-aarch64-binary: M2-Planet | results
	test/test0102/hello-aarch64.sh

test0103-aarch64-binary: M2-Planet | results
	test/test0103/hello-aarch64.sh

test0104-aarch64-binary: M2-Planet | results
	test/test0104/hello-aarch64.sh

test0105-aarch64-binary: M2-Planet | results
	test/test0105/hello-aarch64.sh

test0106-aarch64-binary: M2-Planet | results
	test/test0106/hello-aarch64.sh

test1000-aarch64-binary: M2-Planet | results
	test/test1000/hello-aarch64.sh

test0000-amd64-binary: M2-Planet | results
	test/test0000/hello-amd64.sh

test0001-amd64-binary: M2-Planet | results
	test/test0001/hello-amd64.sh

test0002-amd64-binary: M2-Planet | results
	test/test0002/hello-amd64.sh

test0003-amd64-binary: M2-Planet | results
	test/test0003/hello-amd64.sh

test0004-amd64-binary: M2-Planet | results
	test/test0004/hello-amd64.sh

test0005-amd64-binary: M2-Planet | results
	test/test0005/hello-amd64.sh

test0006-amd64-binary: M2-Planet | results
	test/test0006/hello-amd64.sh

test0007-amd64-binary: M2-Planet | results
	test/test0007/hello-amd64.sh

test0008-amd64-binary: M2-Planet | results
	test/test0008/hello-amd64.sh

test0009-amd64-binary: M2-Planet | results
	test/test0009/hello-amd64.sh

test0010-amd64-binary: M2-Planet | results
	test/test0010/hello-amd64.sh

test0011-amd64-binary: M2-Planet | results
	test/test0011/hello-amd64.sh

test0012-amd64-binary: M2-Planet | results
	test/test0012/hello-amd64.sh

test0013-amd64-binary: M2-Planet | results
	test/test0013/hello-amd64.sh

test0014-amd64-binary: M2-Planet | results
	test/test0014/hello-amd64.sh

test0015-amd64-binary: M2-Planet | results
	test/test0015/hello-amd64.sh

test0016-amd64-binary: M2-Planet | results
	test/test0016/hello-amd64.sh

test0017-amd64-binary: M2-Planet | results
	test/test0017/hello-amd64.sh

test0018-amd64-binary: M2-Planet | results
	test/test0018/hello-amd64.sh

test0019-amd64-binary: M2-Planet | results
	test/test0019/hello-amd64.sh

test0020-amd64-binary: M2-Planet | results
	test/test0020/hello-amd64.sh

test0021-amd64-binary: M2-Planet | results
	test/test0021/hello-amd64.sh

test0022-amd64-binary: M2-Planet | results
	test/test0022/hello-amd64.sh

test0023-amd64-binary: M2-Planet | results
	test/test0023/hello-amd64.sh

test0024-amd64-binary: M2-Planet | results
	test/test0024/hello-amd64.sh

test0025-amd64-binary: M2-Planet | results
	test/test0025/hello-amd64.sh

test0100-amd64-binary: M2-Planet | results
	test/test0100/hello-amd64.sh

test0101-amd64-binary: M2-Planet | results
	test/test0101/hello-amd64.sh

test0102-amd64-binary: M2-Planet | results
	test/test0102/hello-amd64.sh

test0103-amd64-binary: M2-Planet | results
	test/test0103/hello-amd64.sh

test0104-amd64-binary: M2-Planet | results
	test/test0104/hello-amd64.sh

test0105-amd64-binary: M2-Planet | results
	test/test0105/hello-amd64.sh

test0106-amd64-binary: M2-Planet | results
	test/test0106/hello-amd64.sh

test1000-amd64-binary: M2-Planet | results
	test/test1000/hello-amd64.sh

test0000-knight-posix-binary: M2-Planet | results
	test/test0000/hello-knight-posix.sh

test0001-knight-posix-binary: M2-Planet | results
	test/test0001/hello-knight-posix.sh

test0002-knight-posix-binary: M2-Planet | results
	test/test0002/hello-knight-posix.sh

test0003-knight-posix-binary: M2-Planet | results
	test/test0003/hello-knight-posix.sh

test0004-knight-posix-binary: M2-Planet | results
	test/test0004/hello-knight-posix.sh

test0005-knight-posix-binary: M2-Planet | results
	test/test0005/hello-knight-posix.sh

test0006-knight-posix-binary: M2-Planet | results
	test/test0006/hello-knight-posix.sh

test0007-knight-posix-binary: M2-Planet | results
	test/test0007/hello-knight-posix.sh

test0008-knight-posix-binary: M2-Planet | results
	test/test0008/hello-knight-posix.sh

test0009-knight-posix-binary: M2-Planet | results
	test/test0009/hello-knight-posix.sh

test0010-knight-posix-binary: M2-Planet | results
	test/test0010/hello-knight-posix.sh

test0011-knight-posix-binary: M2-Planet | results
	test/test0011/hello-knight-posix.sh

test0012-knight-posix-binary: M2-Planet | results
	test/test0012/hello-knight-posix.sh

test0013-knight-posix-binary: M2-Planet | results
	test/test0013/hello-knight-posix.sh

test0014-knight-posix-binary: M2-Planet | results
	test/test0014/hello-knight-posix.sh

test0015-knight-posix-binary: M2-Planet | results
	test/test0015/hello-knight-posix.sh

test0016-knight-posix-binary: M2-Planet | results
	test/test0016/hello-knight-posix.sh

test0017-knight-posix-binary: M2-Planet | results
	test/test0017/hello-knight-posix.sh

test0018-knight-posix-binary: M2-Planet | results
	test/test0018/hello-knight-posix.sh

test0019-knight-posix-binary: M2-Planet | results
	test/test0019/hello-knight-posix.sh

test0020-knight-posix-binary: M2-Planet | results
	test/test0020/hello-knight-posix.sh

test0021-knight-posix-binary: M2-Planet | results
	test/test0021/hello-knight-posix.sh

test0022-knight-posix-binary: M2-Planet | results
	test/test0022/hello-knight-posix.sh

test0023-knight-posix-binary: M2-Planet | results
	test/test0023/hello-knight-posix.sh

test0024-knight-posix-binary: M2-Planet | results
	test/test0024/hello-knight-posix.sh

test0100-knight-posix-binary: M2-Planet | results
	test/test0100/hello-knight-posix.sh

test0101-knight-posix-binary: M2-Planet | results
	test/test0101/hello-knight-posix.sh

test0102-knight-posix-binary: M2-Planet | results
	test/test0102/hello-knight-posix.sh

test0103-knight-posix-binary: M2-Planet | results
	test/test0103/hello-knight-posix.sh

test0106-knight-posix-binary: M2-Planet | results
	test/test0106/hello-knight-posix.sh

test1000-knight-posix-binary: M2-Planet | results
	test/test1000/hello-knight-posix.sh

test0000-knight-native-binary: M2-Planet | results
	test/test0000/hello-knight-native.sh

test0001-knight-native-binary: M2-Planet | results
	test/test0001/hello-knight-native.sh

test0002-knight-native-binary: M2-Planet | results
	test/test0002/hello-knight-native.sh

test0003-knight-native-binary: M2-Planet | results
	test/test0003/hello-knight-native.sh

test0004-knight-native-binary: M2-Planet | results
	test/test0004/hello-knight-native.sh

test0005-knight-native-binary: M2-Planet | results
	test/test0005/hello-knight-native.sh

test0006-knight-native-binary: M2-Planet | results
	test/test0006/hello-knight-native.sh

test0007-knight-native-binary: M2-Planet | results
	test/test0007/hello-knight-native.sh

test0008-knight-native-binary: M2-Planet | results
	test/test0008/hello-knight-native.sh

test0009-knight-native-binary: M2-Planet | results
	test/test0009/hello-knight-native.sh

test0010-knight-native-binary: M2-Planet | results
	test/test0010/hello-knight-native.sh

test0011-knight-native-binary: M2-Planet | results
	test/test0011/hello-knight-native.sh

test0012-knight-native-binary: M2-Planet | results
	test/test0012/hello-knight-native.sh

test0013-knight-native-binary: M2-Planet | results
	test/test0013/hello-knight-native.sh

test0017-knight-native-binary: M2-Planet | results
	test/test0017/hello-knight-native.sh

test0018-knight-native-binary: M2-Planet | results
	test/test0018/hello-knight-native.sh

test0020-knight-native-binary: M2-Planet | results
	test/test0020/hello-knight-native.sh

test0024-knight-native-binary: M2-Planet | results
	test/test0024/hello-knight-native.sh

test0106-knight-native-binary: M2-Planet | results
	test/test0106/hello-knight-native.sh

test0000-armv7l-binary: M2-Planet | results
	test/test0000/hello-armv7l.sh

test0001-armv7l-binary: M2-Planet | results
	test/test0001/hello-armv7l.sh

test0002-armv7l-binary: M2-Planet | results
	test/test0002/hello-armv7l.sh

test0003-armv7l-binary: M2-Planet | results
	test/test0003/hello-armv7l.sh

test0004-armv7l-binary: M2-Planet | results
	test/test0004/hello-armv7l.sh

test0005-armv7l-binary: M2-Planet | results
	test/test0005/hello-armv7l.sh

test0006-armv7l-binary: M2-Planet | results
	test/test0006/hello-armv7l.sh

test0007-armv7l-binary: M2-Planet | results
	test/test0007/hello-armv7l.sh

test0008-armv7l-binary: M2-Planet | results
	test/test0008/hello-armv7l.sh

test0009-armv7l-binary: M2-Planet | results
	test/test0009/hello-armv7l.sh

test0010-armv7l-binary: M2-Planet | results
	test/test0010/hello-armv7l.sh

test0011-armv7l-binary: M2-Planet | results
	test/test0011/hello-armv7l.sh

test0012-armv7l-binary: M2-Planet | results
	test/test0012/hello-armv7l.sh

test0013-armv7l-binary: M2-Planet | results
	test/test0013/hello-armv7l.sh

test0014-armv7l-binary: M2-Planet | results
	test/test0014/hello-armv7l.sh

test0015-armv7l-binary: M2-Planet | results
	test/test0015/hello-armv7l.sh

test0016-armv7l-binary: M2-Planet | results
	test/test0016/hello-armv7l.sh

test0017-armv7l-binary: M2-Planet | results
	test/test0017/hello-armv7l.sh

test0018-armv7l-binary: M2-Planet | results
	test/test0018/hello-armv7l.sh

test0019-armv7l-binary: M2-Planet | results
	test/test0019/hello-armv7l.sh

test0020-armv7l-binary: M2-Planet | results
	test/test0020/hello-armv7l.sh

test0021-armv7l-binary: M2-Planet | results
	test/test0021/hello-armv7l.sh

test0022-armv7l-binary: M2-Planet | results
	test/test0022/hello-armv7l.sh

test0023-armv7l-binary: M2-Planet | results
	test/test0023/hello-armv7l.sh

test0024-armv7l-binary: M2-Planet | results
	test/test0024/hello-armv7l.sh

test0025-armv7l-binary: M2-Planet | results
	test/test0025/hello-armv7l.sh

test0100-armv7l-binary: M2-Planet | results
	test/test0100/hello-armv7l.sh

test0101-armv7l-binary: M2-Planet | results
	test/test0101/hello-armv7l.sh

test0102-armv7l-binary: M2-Planet | results
	test/test0102/hello-armv7l.sh

test0103-armv7l-binary: M2-Planet | results
	test/test0103/hello-armv7l.sh

test0104-armv7l-binary: M2-Planet | results
	test/test0104/hello-armv7l.sh

test0105-armv7l-binary: M2-Planet | results
	test/test0105/hello-armv7l.sh

test0106-armv7l-binary: M2-Planet | results
	test/test0106/hello-armv7l.sh

test1000-armv7l-binary: M2-Planet | results
	test/test1000/hello-armv7l.sh

test0000-x86-binary: M2-Planet | results
	test/test0000/hello-x86.sh

test0001-x86-binary: M2-Planet | results
	test/test0001/hello-x86.sh

test0002-x86-binary: M2-Planet | results
	test/test0002/hello-x86.sh

test0003-x86-binary: M2-Planet | results
	test/test0003/hello-x86.sh

test0004-x86-binary: M2-Planet | results
	test/test0004/hello-x86.sh

test0005-x86-binary: M2-Planet | results
	test/test0005/hello-x86.sh

test0006-x86-binary: M2-Planet | results
	test/test0006/hello-x86.sh

test0007-x86-binary: M2-Planet | results
	test/test0007/hello-x86.sh

test0008-x86-binary: M2-Planet | results
	test/test0008/hello-x86.sh

test0009-x86-binary: M2-Planet | results
	test/test0009/hello-x86.sh

test0010-x86-binary: M2-Planet | results
	test/test0010/hello-x86.sh

test0011-x86-binary: M2-Planet | results
	test/test0011/hello-x86.sh

test0012-x86-binary: M2-Planet | results
	test/test0012/hello-x86.sh

test0013-x86-binary: M2-Planet | results
	test/test0013/hello-x86.sh

test0014-x86-binary: M2-Planet | results
	test/test0014/hello-x86.sh

test0015-x86-binary: M2-Planet | results
	test/test0015/hello-x86.sh

test0016-x86-binary: M2-Planet | results
	test/test0016/hello-x86.sh

test0017-x86-binary: M2-Planet | results
	test/test0017/hello-x86.sh

test0018-x86-binary: M2-Planet | results
	test/test0018/hello-x86.sh

test0019-x86-binary: M2-Planet | results
	test/test0019/hello-x86.sh

test0020-x86-binary: M2-Planet | results
	test/test0020/hello-x86.sh

test0021-x86-binary: M2-Planet | results
	test/test0021/hello-x86.sh

test0022-x86-binary: M2-Planet | results
	test/test0022/hello-x86.sh

test0023-x86-binary: M2-Planet | results
	test/test0023/hello-x86.sh

test0024-x86-binary: M2-Planet | results
	test/test0024/hello-x86.sh

test0025-x86-binary: M2-Planet | results
	test/test0025/hello-x86.sh

test0100-x86-binary: M2-Planet | results
	test/test0100/hello-x86.sh

test0101-x86-binary: M2-Planet | results
	test/test0101/hello-x86.sh

test0102-x86-binary: M2-Planet | results
	test/test0102/hello-x86.sh

test0103-x86-binary: M2-Planet | results
	test/test0103/hello-x86.sh

test0104-x86-binary: M2-Planet | results
	test/test0104/hello-x86.sh

test0105-x86-binary: M2-Planet | results
	test/test0105/hello-x86.sh

test0106-x86-binary: M2-Planet | results
	test/test0106/hello-x86.sh

test1000-x86-binary: M2-Planet | results
	test/test1000/hello-x86.sh

# Generate test answers
.PHONY: Generate-test-answers
Generate-test-answers:
	sha256sum test/results/* >| test/test.answers

DESTDIR:=
PREFIX:=/usr/local
bindir:=$(DESTDIR)$(PREFIX)/bin
.PHONY: install
install: M2-Planet
	mkdir -p $(bindir)
	cp $^ $(bindir)

###  dist
.PHONY: dist

COMMIT=$(shell git describe --dirty)
TARBALL_VERSION=$(COMMIT:Release_%=%)
TARBALL_DIR:=$(PACKAGE)-$(TARBALL_VERSION)
TARBALL=$(TARBALL_DIR).tar.gz
# Be friendly to Debian; avoid using EPOCH
MTIME=$(shell git show HEAD --format=%ct --no-patch)
# Reproducible tarball
TAR_FLAGS=--sort=name --mtime=@$(MTIME) --owner=0 --group=0 --numeric-owner --mode=go=rX,u+rw,a-s

$(TARBALL):
	(git ls-files					\
	    --exclude=$(TARBALL_DIR);			\
	    echo $^ | tr ' ' '\n')			\
	    | tar $(TAR_FLAGS)				\
	    --transform=s,^,$(TARBALL_DIR)/,S -T- -cf-	\
	    | gzip -c --no-name > $@

dist: $(TARBALL)
