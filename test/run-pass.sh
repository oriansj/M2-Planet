#!/usr/bin/env sh

set -ex

ARCH="$1"
. test/env.inc.sh
TMPDIR="test/run-pass/tmp/${ARCH}"

mkdir -p "${TMPDIR}"

for file in ./test/run-pass/*.c; do
	base_name="$(basename ${file})"

	# Build the test
	bin/M2-Planet \
	--architecture "${ARCH}" \
	--debug \
	"${file}" \
	-o "${TMPDIR}/${base_name}.M1" \
	|| exit 1

	# Build debug footer
	blood-elf \
		${BLOOD_ELF_WORD_SIZE_FLAG} \
		-f "${TMPDIR}/${base_name}.M1" \
		${ENDIANNESS_FLAG} \
		--entry _start \
		-o "${TMPDIR}/${base_name}-footer.M1" \
		|| exit 2

	# Macro assemble with libc written in M1-Macro
	M1 \
		-f M2libc/${ARCH}/${ARCH}_defs.M1 \
		-f M2libc/${ARCH}/libc-core.M1 \
		-f "${TMPDIR}/${base_name}.M1" \
		-f "${TMPDIR}/${base_name}-footer.M1" \
		${ENDIANNESS_FLAG} \
		--architecture ${ARCH} \
		-o "${TMPDIR}/${base_name}.hex2" \
		|| exit 2

	# Resolve all linkages
	hex2 \
		-f M2libc/${ARCH}/ELF-${ARCH}-debug.hex2 \
		-f "${TMPDIR}/${base_name}.hex2" \
		${ENDIANNESS_FLAG} \
		--architecture ${ARCH} \
		--base-address ${BASE_ADDRESS} \
		-o test/results/${base_name}-${ARCH}-binary \
		|| exit 3

	# Ensure binary works if host machine supports test
	if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "${ARCH}" ] || [ -n "${GET_MACHINE_OVERRIDE_ALWAYS_RUN+x}" ]
	then
		. ./sha256.sh
		# Verify that the resulting file works
		./test/results/${base_name}-${ARCH}-binary || exit $?
	fi

done

