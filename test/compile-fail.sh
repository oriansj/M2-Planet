#!/usr/bin/env sh

set -e

for file in ./test/compile-fail/*.c; do

	if bin/M2-Planet "${file}" > /dev/null 2>&1; then
		echo "${file} returned success but needs to be compile failure"
		exit 1
	fi

done
