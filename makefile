# Prevent rebuilding
VPATH = bin:test:test/results

all: M2-Planet

CC=gcc
CFLAGS=-D_GNU_SOURCE -O0 -std=c99 -ggdb

M2-Planet: cc_reader.c cc_strings.c cc_core.c cc.c cc.h | bin
	$(CC) $(CFLAGS) cc_reader.c cc_strings.c cc_core.c cc.c cc.h -o bin/M2-Planet

M2-Planet-minimal: cc_reader.c cc_strings.c cc_core.c cc-minimal.c cc.h | bin
	$(CC) $(CFLAGS) cc_reader.c cc_strings.c cc_core.c cc-minimal.c cc.h -o bin/M2-Planet-minimal

# Clean up after ourselves
.PHONY: clean
clean:
	rm -rf bin/ test/results/
	./test/test0/cleanup.sh
	./test/test1/cleanup.sh
	./test/test2/cleanup.sh
	./test/test3/cleanup.sh
	./test/test4/cleanup.sh
	./test/test99/cleanup.sh

# Directories
bin:
	mkdir -p bin

results:
	mkdir -p test/results

# tests
test: test0-binary test1-binary test2-binary test3-binary test4-binary test99-binary | results
	sha256sum -c test/test.answers

test0-binary: M2-Planet | results
	test/test0/hello.sh

test1-binary: M2-Planet | results
	test/test1/hello.sh

test2-binary: M2-Planet | results
	test/test2/hello.sh

test3-binary: M2-Planet | results
	test/test3/hello.sh

test4-binary: M2-Planet | results
	test/test4/hello.sh

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
