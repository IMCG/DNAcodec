#!/bin/bash
if [ "$#" -ne 2 ]; then
        echo "Converts DNA in single-line fasta format to binary file"
        echo "usage: ./run_decode.sh source output"
        exit 1
fi

./DNAIO -c $1 - |./decode ECC.pchk - - bsc 0.09 prprp -100|./extract ECC.gen - -|lzma -d -c >$2
