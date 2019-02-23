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

M2-Planet: bin results
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
test: test00-binary \
	test01-binary \
	test02-binary \
	test03-binary \
	test04-binary \
	test05-binary \
	test06-binary \
	test07-binary \
	test08-binary \
	test09-binary \
	test10-binary \
	test11-binary \
	test12-binary \
	test13-binary \
	test14-binary \
	test15-binary \
	test16-binary \
	test17-binary \
	test18-binary \
	test19-binary \
	test20-binary \
	test21-binary \
	test22-binary \
	test23-binary \
	test24-binary \
	test99-binary \
	test100-binary | results
	sha256sum -c test/test.answers

test00-binary: M2-Planet | results
	test/test00/hello.sh

test01-binary: M2-Planet | results
	test/test01/hello.sh

test02-binary: M2-Planet | results
	test/test02/hello.sh

test03-binary: M2-Planet | results
	test/test03/hello.sh

test04-binary: M2-Planet | results
	test/test04/hello.sh

test05-binary: M2-Planet | results
	test/test05/hello.sh

test06-binary: M2-Planet | results
	test/test06/hello.sh

test07-binary: M2-Planet | results
	test/test07/hello.sh

test08-binary: M2-Planet | results
	test/test08/hello.sh

test09-binary: M2-Planet | results
	test/test09/hello.sh

test10-binary: M2-Planet | results
	test/test10/hello.sh

test11-binary: M2-Planet | results
	test/test11/hello.sh

test12-binary: M2-Planet | results
	test/test12/hello.sh

test13-binary: M2-Planet | results
	test/test13/hello.sh

test14-binary: M2-Planet | results
	test/test14/hello.sh

test15-binary: M2-Planet | results
	test/test15/hello.sh

test16-binary: M2-Planet | results
	test/test16/hello.sh

test17-binary: M2-Planet | results
	test/test17/hello.sh

test18-binary: M2-Planet | results
	test/test18/hello.sh

test19-binary: M2-Planet | results
	test/test19/hello.sh

test20-binary: M2-Planet | results
	test/test20/hello.sh

test21-binary: M2-Planet | results
	test/test21/hello.sh

test22-binary: M2-Planet | results
	test/test22/hello.sh

test23-binary: M2-Planet | results
	test/test23/hello.sh

test24-binary: M2-Planet | results
	test/test24/hello.sh

test99-binary: M2-Planet | results
	test/test99/hello.sh

test100-binary: M2-Planet | results
	test/test100/hello.sh

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
