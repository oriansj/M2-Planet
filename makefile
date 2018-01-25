# Prevent rebuilding
VPATH = bin:test:test/results

all: M2-Planet

CC=gcc
CFLAGS=-D_GNU_SOURCE -O0 -std=c99 -ggdb

M2-Planet: cc_reader.c cc_strings.c cc_core.c cc.c cc_types.c cc.h | bin
	$(CC) $(CFLAGS) cc_reader.c cc_strings.c cc_core.c cc.c cc_types.c cc.h -o bin/M2-Planet

M2-Planet-minimal: cc_reader.c cc_strings.c cc_core.c cc-minimal.c cc_types.c cc.h | bin
	$(CC) $(CFLAGS) cc_reader.c cc_strings.c cc_core.c cc-minimal.c cc_types.c cc.h -o bin/M2-Planet-minimal

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
	./test/test99/cleanup.sh

# Directories
bin:
	mkdir -p bin

results:
	mkdir -p test/results

# tests
test: test00-binary test01-binary test02-binary test03-binary test04-binary test05-binary test06-binary test07-binary test08-binary test09-binary test10-binary test99-binary | results
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

test99-binary: M2-Planet | results
	test/test99/hello.sh

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
