#!/usr/bin/env sh

set -e

for file in ./test/compile-fail/*.c; do

	set +e
	bin/M2-Planet --expand-includes "${file}" > /dev/null 2>&1
	rc=$?
	set -e

	if [ "${rc}" -eq 0 ]; then
		echo "${file} returned success but needs to be compile failure"
		exit 1
	fi

	if [ "${rc}" -eq 139 ]; then
		echo "${file} crashed with SIGSEGV"
		exit 1
	fi

done
