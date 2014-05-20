/* DEC.H - Interface to decoding procedures. */

/* Copyright (c) 2014 by Allen Yu
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Copyright (c) 2000, 2001 by Radford M. Neal 
 *
 * Permission is granted for anyone to copy, use, modify, or distribute this
 * program and accompanying programs and documents for any purpose, provided 
 * this copyright notice is retained and prominently displayed, along with
 * a note saying that the original programs are available from Radford Neal's
 * web page, and note is made of any changes made to the programs.  The
 * programs and documents are distributed without any warranty, express or
 * implied.  As the programs were written for research purposes only, they have
 * not been tested to the degree that would be advisable in any important
 * application.  All use of these programs is entirely at the user's own risk.
 */


/* DECODING METHOD, ITS PARAMETERS, AND OTHER VARIABLES.  The global variables 
   declared here are located in dec.c. */

typedef enum 
{ Enum_block, Enum_bit, Prprp
} decoding_method;

extern decoding_method dec_method; /* Decoding method to use */

extern int table;	/* Trace option, 2 for a table of decoding details */
extern int block_no;	/* Number of current block, from zero */

extern int max_iter;	/* Maximum number of iteratons of decoding to do */
extern char *gen_file;	/* Generator file for Enum_block and Enum_bit */


/* PROCEDURES RELATING TO DECODING METHODS. */

void enum_decode_setup (void);
unsigned enum_decode (double *, char *, double *, int);

void prprp_decode_setup (void);
unsigned prprp_decode 
(mod2sparse *, double *, char *, char *, double *);

void initprp (mod2sparse *, double *, char *, double *);
void iterprp (mod2sparse *, double *, char *, double *);
