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

# Prevent rebuilding
VPATH = bin:test:test/results

all: M2-Planet

M2-Planet: bin results cc.h cc_reader.c cc_strings.c cc_types.c cc_core.c cc.c
	./test/test100/hello.sh

# Clean up after ourselves
.PHONY: clean
clean:
	rm -rf bin/ test/results/
	./test/test00/cleanup.sh
	./test/test01/cleanup.sh
	./test/test02/cleanup.sh
	./test/test03/cleanup.sh
	./test/test04/cleanup.sh
	./test/test05/cleanup.sh
	./test/test06/cleanup.sh
	./test/test07/cleanup.sh
	./test/test08/cleanup.sh
	./test/test09/cleanup.sh
	./test/test10/cleanup.sh
	./test/test11/cleanup.sh
	./test/test12/cleanup.sh
	./test/test13/cleanup.sh
	./test/test14/cleanup.sh
	./test/test15/cleanup.sh
	./test/test16/cleanup.sh
	./test/test17/cleanup.sh
	./test/test18/cleanup.sh
	./test/test19/cleanup.sh
	./test/test20/cleanup.sh
	./test/test21/cleanup.sh
	./test/test22/cleanup.sh
	./test/test23/cleanup.sh
	./test/test24/cleanup.sh
	./test/test99/cleanup.sh
	./test/test100/cleanup.sh

# Directories
bin:
	mkdir -p bin

results:
	mkdir -p test/results

# tests
test: test00-knight-posix-binary \
	test01-knight-posix-binary \
	test02-knight-posix-binary \
	test03-knight-posix-binary \
	test04-knight-posix-binary \
	test05-knight-posix-binary \
	test06-knight-posix-binary \
	test07-knight-posix-binary \
	test08-knight-posix-binary \
	test09-knight-posix-binary \
	test10-knight-posix-binary \
	test11-knight-posix-binary \
	test12-knight-posix-binary \
	test13-knight-posix-binary \
	test14-knight-posix-binary \
	test15-knight-posix-binary \
	test16-knight-posix-binary \
	test17-knight-posix-binary \
	test18-knight-posix-binary \
	test19-knight-posix-binary \
	test20-knight-posix-binary \
	test21-knight-posix-binary \
	test22-knight-posix-binary \
	test23-knight-posix-binary \
	test24-knight-posix-binary \
	test99-knight-posix-binary \
	test100-knight-posix-binary \
	test00-armv7l-binary \
	test01-armv7l-binary \
	test02-armv7l-binary \
	test03-armv7l-binary \
	test04-armv7l-binary \
	test05-armv7l-binary \
	test06-armv7l-binary \
	test07-armv7l-binary \
	test08-armv7l-binary \
	test09-armv7l-binary \
	test10-armv7l-binary \
	test11-armv7l-binary \
	test12-armv7l-binary \
	test13-armv7l-binary \
	test14-armv7l-binary \
	test15-armv7l-binary \
	test16-armv7l-binary \
	test17-armv7l-binary \
	test18-armv7l-binary \
	test19-armv7l-binary \
	test20-armv7l-binary \
	test21-armv7l-binary \
	test22-armv7l-binary \
	test23-armv7l-binary \
	test24-armv7l-binary \
	test25-armv7l-binary \
	test26-armv7l-binary \
	test00-x86-binary \
	test01-x86-binary \
	test02-x86-binary \
	test03-x86-binary \
	test04-x86-binary \
	test05-x86-binary \
	test06-x86-binary \
	test07-x86-binary \
	test08-x86-binary \
	test09-x86-binary \
	test10-x86-binary \
	test11-x86-binary \
	test12-x86-binary \
	test13-x86-binary \
	test14-x86-binary \
	test15-x86-binary \
	test16-x86-binary \
	test17-x86-binary \
	test18-x86-binary \
	test19-x86-binary \
	test20-x86-binary \
	test21-x86-binary \
	test22-x86-binary \
	test23-x86-binary \
	test24-x86-binary \
	test25-x86-binary \
	test26-x86-binary \
	test99-x86-binary \
	test100-x86-binary | results
	sha256sum -c test/test.answers

test00-knight-posix-binary: M2-Planet | results
	test/test00/hello-knight-posix.sh

test01-knight-posix-binary: M2-Planet | results
	test/test01/hello-knight-posix.sh

test02-knight-posix-binary: M2-Planet | results
	test/test02/hello-knight-posix.sh

test03-knight-posix-binary: M2-Planet | results
	test/test03/hello-knight-posix.sh

test04-knight-posix-binary: M2-Planet | results
	test/test04/hello-knight-posix.sh

test05-knight-posix-binary: M2-Planet | results
	test/test05/hello-knight-posix.sh

test06-knight-posix-binary: M2-Planet | results
	test/test06/hello-knight-posix.sh

test07-knight-posix-binary: M2-Planet | results
	test/test07/hello-knight-posix.sh

test08-knight-posix-binary: M2-Planet | results
	test/test08/hello-knight-posix.sh

test09-knight-posix-binary: M2-Planet | results
	test/test09/hello-knight-posix.sh

test10-knight-posix-binary: M2-Planet | results
	test/test10/hello-knight-posix.sh

test11-knight-posix-binary: M2-Planet | results
	test/test11/hello-knight-posix.sh

test12-knight-posix-binary: M2-Planet | results
	test/test12/hello-knight-posix.sh

test13-knight-posix-binary: M2-Planet | results
	test/test13/hello-knight-posix.sh

test14-knight-posix-binary: M2-Planet | results
	test/test14/hello-knight-posix.sh

test15-knight-posix-binary: M2-Planet | results
	test/test15/hello-knight-posix.sh

test16-knight-posix-binary: M2-Planet | results
	test/test16/hello-knight-posix.sh

test17-knight-posix-binary: M2-Planet | results
	test/test17/hello-knight-posix.sh

test18-knight-posix-binary: M2-Planet | results
	test/test18/hello-knight-posix.sh

test19-knight-posix-binary: M2-Planet | results
	test/test19/hello-knight-posix.sh

test20-knight-posix-binary: M2-Planet | results
	test/test20/hello-knight-posix.sh

test21-knight-posix-binary: M2-Planet | results
	test/test21/hello-knight-posix.sh

test22-knight-posix-binary: M2-Planet | results
	test/test22/hello-knight-posix.sh

test23-knight-posix-binary: M2-Planet | results
	test/test23/hello-knight-posix.sh

test24-knight-posix-binary: M2-Planet | results
	test/test24/hello-knight-posix.sh

test99-knight-posix-binary: M2-Planet | results
	test/test99/hello-knight-posix.sh

test100-knight-posix-binary: M2-Planet | results
	test/test100/hello-knight-posix.sh

test00-armv7l-binary: M2-Planet | results
	test/test00/hello-armv7l.sh

test01-armv7l-binary: M2-Planet | results
	test/test01/hello-armv7l.sh

test02-armv7l-binary: M2-Planet | results
	test/test02/hello-armv7l.sh

test03-armv7l-binary: M2-Planet | results
	test/test03/hello-armv7l.sh

test04-armv7l-binary: M2-Planet | results
	test/test04/hello-armv7l.sh

test05-armv7l-binary: M2-Planet | results
	test/test05/hello-armv7l.sh

test06-armv7l-binary: M2-Planet | results
	test/test06/hello-armv7l.sh

test07-armv7l-binary: M2-Planet | results
	test/test07/hello-armv7l.sh

test08-armv7l-binary: M2-Planet | results
	test/test08/hello-armv7l.sh

test09-armv7l-binary: M2-Planet | results
	test/test09/hello-armv7l.sh

test10-armv7l-binary: M2-Planet | results
	test/test10/hello-armv7l.sh

test11-armv7l-binary: M2-Planet | results
	test/test11/hello-armv7l.sh

test12-armv7l-binary: M2-Planet | results
	test/test12/hello-armv7l.sh

test13-armv7l-binary: M2-Planet | results
	test/test13/hello-armv7l.sh

test14-armv7l-binary: M2-Planet | results
	test/test14/hello-armv7l.sh

test15-armv7l-binary: M2-Planet | results
	test/test15/hello-armv7l.sh

test16-armv7l-binary: M2-Planet | results
	test/test16/hello-armv7l.sh

test17-armv7l-binary: M2-Planet | results
	test/test17/hello-armv7l.sh

test18-armv7l-binary: M2-Planet | results
	test/test18/hello-armv7l.sh

test19-armv7l-binary: M2-Planet | results
	test/test19/hello-armv7l.sh

test20-armv7l-binary: M2-Planet | results
	test/test20/hello-armv7l.sh

test21-armv7l-binary: M2-Planet | results
	test/test21/hello-armv7l.sh

test22-armv7l-binary: M2-Planet | results
	test/test22/hello-armv7l.sh

test23-armv7l-binary: M2-Planet | results
	test/test23/hello-armv7l.sh

test24-armv7l-binary: M2-Planet | results
	test/test24/hello-armv7l.sh

test25-armv7l-binary: M2-Planet | results
	test/test25/hello-armv7l.sh

test26-armv7l-binary: M2-Planet | results
	test/test26/hello-armv7l.sh

test00-x86-binary: M2-Planet | results
	test/test00/hello-x86.sh

test01-x86-binary: M2-Planet | results
	test/test01/hello-x86.sh

test02-x86-binary: M2-Planet | results
	test/test02/hello-x86.sh

test03-x86-binary: M2-Planet | results
	test/test03/hello-x86.sh

test04-x86-binary: M2-Planet | results
	test/test04/hello-x86.sh

test05-x86-binary: M2-Planet | results
	test/test05/hello-x86.sh

test06-x86-binary: M2-Planet | results
	test/test06/hello-x86.sh

test07-x86-binary: M2-Planet | results
	test/test07/hello-x86.sh

test08-x86-binary: M2-Planet | results
	test/test08/hello-x86.sh

test09-x86-binary: M2-Planet | results
	test/test09/hello-x86.sh

test10-x86-binary: M2-Planet | results
	test/test10/hello-x86.sh

test11-x86-binary: M2-Planet | results
	test/test11/hello-x86.sh

test12-x86-binary: M2-Planet | results
	test/test12/hello-x86.sh

test13-x86-binary: M2-Planet | results
	test/test13/hello-x86.sh

test14-x86-binary: M2-Planet | results
	test/test14/hello-x86.sh

test15-x86-binary: M2-Planet | results
	test/test15/hello-x86.sh

test16-x86-binary: M2-Planet | results
	test/test16/hello-x86.sh

test17-x86-binary: M2-Planet | results
	test/test17/hello-x86.sh

test18-x86-binary: M2-Planet | results
	test/test18/hello-x86.sh

test19-x86-binary: M2-Planet | results
	test/test19/hello-x86.sh

test20-x86-binary: M2-Planet | results
	test/test20/hello-x86.sh

test21-x86-binary: M2-Planet | results
	test/test21/hello-x86.sh

test22-x86-binary: M2-Planet | results
	test/test22/hello-x86.sh

test23-x86-binary: M2-Planet | results
	test/test23/hello-x86.sh

test24-x86-binary: M2-Planet | results
	test/test24/hello-x86.sh

test25-x86-binary: M2-Planet | results
	test/test25/hello-x86.sh

test26-x86-binary: M2-Planet | results
	test/test26/hello-x86.sh

test99-x86-binary: M2-Planet | results
	test/test99/hello-x86.sh

test100-x86-binary: M2-Planet | results
	test/test100/hello-x86.sh

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
