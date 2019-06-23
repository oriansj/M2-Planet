# M2-Planet
The PLAtform NEutral Transpiler, when combined with mescc-tools;
allows one to compile a subset of the C language into working binaries
with introspective steps inbetween.

A lovely set of examples of M2-Planet programs are in tests but the most
surprising part of all M2-Planet can self-host M2-Planet.

Further more M2-Planet is cross-platform and self-hosting across platforms
with fully deterministic builds enabling prefect reproducibility.

To bootstrap from assembly simple run ./bootstrap-x86.sh after cd into the
bootstrap directory (the only requirement is mescc-tools to be installed)

Or if you have no binaries to bootstrap from please use mescc-tools-seed;
which will not only bootstrap mescc-tools but also M2-Planet