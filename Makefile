# MAKEFILE for DNA data storage codec

# Copyright (c) 2014 by Allen Yu
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
#  of the Software, and to permit persons to whom the Software is furnished to do so,
#  subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
# USE OR OTHER DEALINGS IN THE SOFTWARE.


COMPILE = cc -g -c -O3    # Command to compile a module from .c to .o
LINK =    cc          # Command to link a program
LIBXML = /usr/include/libxml2

# MAKE ALL THE MAIN PROGRAMS.  First makes the modules used.

progs:	modules
	$(COMPILE) rand-src.c
	$(LINK) rand-src.o rand.o open.o -lm -o rand-src
	$(COMPILE) encode.c
	$(LINK) encode.o int2bin.o crc.o mod2sparse.o mod2dense.o mod2convert.o \
	   enc.o rcode.o rand.o alloc.o intio.o blockio.o open.o -lm -o encode
	$(COMPILE) transmit.c
	$(LINK) transmit.o channel.o rand.o open.o -lm -o transmit
	$(COMPILE) decode.c
	$(LINK) decode.o crc.o int2bin.o channel.o mod2sparse.o mod2dense.o mod2convert.o \
	   enc.o check.o \
	   rcode.o rand.o alloc.o intio.o blockio.o dec.o open.o -lm -o decode
	$(COMPILE) extract.c
	$(LINK) extract.o crc.o int2bin.o mod2sparse.o mod2dense.o mod2convert.o \
	   rcode.o alloc.o intio.o blockio.o open.o -lm -o extract
	$(COMPILE) verify.c
	$(LINK) verify.o crc.o int2bin.o mod2sparse.o mod2dense.o mod2convert.o check.o \
	   rcode.o alloc.o intio.o blockio.o open.o -lm -o verify
	$(COMPILE) DNAIO.c -I$(LIBXML) -lxml2
	$(LINK) DNAIO.o open.o crc.o bin2dec.o int2bin.o str_match.o xml.o -I$(LIBXML) -lxml2 -lm -o DNAIO


# MAKE THE MODULES USED BY THE PROGRAMS.

modules:
	$(COMPILE) rcode.c
	$(COMPILE) channel.c
	$(COMPILE) dec.c
	$(COMPILE) enc.c
	$(COMPILE) alloc.c
	$(COMPILE) bin2dec.c
	$(COMPILE) str_match.c
	$(COMPILE) crc.c int2bin.c blockio.c
	$(COMPILE) intio.c
	$(COMPILE) check.c
	$(COMPILE) open.c
	$(COMPILE) mod2dense.c
	$(COMPILE) mod2sparse.c
	$(COMPILE) mod2convert.c
	$(COMPILE) distrib.c
	$(COMPILE) crc.c
	$(COMPILE) xml.c -I$(LIBXML) -lxml2
	$(COMPILE) int2bin.c	
	$(COMPILE) -DRAND_FILE=\"`pwd`/randfile\" rand.c


# CLEAN UP ALL PROGRAMS AND REMOVE ALL FILES PRODUCED BY TESTS AND EXAMPLES.

clean:
	rm -f	core *.o ex-*.* test-file \
		rand-src encode DNAIO transmit decode extract verify 
