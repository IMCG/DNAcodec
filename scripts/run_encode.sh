#!/bin/bash
if [ "$#" -ne 2 ]; then
	echo "Converts source file to encoded DNA in single-line fasta format"
	echo "usage: ./run_encode.sh source output"
	exit 1
fi

lzma -z -9 -c $1|./encode ECC.pchk ECC.gen - -|./DNAIO -f - $2
