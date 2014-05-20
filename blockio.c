/* BLOCKIO.C - Routines to read/write blocks of bits from/to a text file. */

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
 *  */

/* Copyright (c) 2000 by Radford M. Neal 
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


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "blockio.h"
#include "crc.h"
#include "version.h"
#include "int2bin.h"

/* READ A BLOCK OF BITS.  The bits must be given as '0' or '1' characters,
   with whitespace allowed (but not required) between bits.  Returns 0 if
   a block is read successfully, and EOF if eof or an error occurs.  If
   EOF is returned, a warning will be printed if a partial block had already
   been read. */

int blockio_read
( FILE *f,    /* File to read from */
  char *b,    /* Place to store bits read */
  int l       /* Length of block */
)
{
  int i, c;

  for (i = 0; i<l; i++)
  { 
    do
    { c = getc(f);
      if (c==EOF) 
      { if (i>0)
        { fprintf(stderr,
           "Warning: Short block (%d long) at end of input file ignored\n",i);
        }
        return EOF;
      }
    } while (c==' ' || c=='\t' || c=='\n' || c=='\r');

    if (c!='0' && c!='1')
    { fprintf(stderr,"Bad character in binary file (not '0' or '1')\n");
      exit(1);
    }
    
    b[i] = c=='1';
  }
  return 0;
}

/* READ A BLOCK OF BITS IN BINARY MODE. Returns 0 if a block is read successfully, 
   and EOF if eof or an error occurs.  If EOF is returned, last position in block
   will be recorded */

int blockio_read_bin
( FILE *f,    /* File to read from */
  char *b,    /* Place to store bits read */
  int l,       /* Length of block in bits (multiple of 8)*/
  int *last_pos	/* Record the last position before EOF */
)
{
  int i,j,n;
  unsigned char* buffer;
  buffer =(unsigned char*)malloc(sizeof(char)+1); 

  for (i = 0; i<l/CHAR_BIT; i++)
  { 
    n=fread(buffer, sizeof(char), 1, f);
    if (feof( f )) 
    { 
      *last_pos=(i)*CHAR_BIT;
      return EOF;
    }

    for ( j = CHAR_BIT - 1; j >= 0; j-- )
    {
        b[i*CHAR_BIT+j] = (*buffer % 2) + '0'=='1';
        *buffer /= 2;
    }
    
  }
  /*fprintf(stderr,"l:%d\nSize of block:%d\nblock:\n%s\n",l,sizeof(b),b);*/
  return 0;
}

/* WRITE A BLOCK OF BITS IN BINARY MODE. Returns 0 if a block is read successfully, 
   and EOF if eof or an error occurs.  If EOF is returned, last position in block
   will be recorded */

int blockio_write_bin
( FILE *f,    /* File to write */
  char *bin,    /* Block of data to be written */
  int l      /* Length of block in bits (multiple of 8)*/
)
{
  int i,n,b, k, m;
  int len;
  unsigned char* buffer;
  buffer =(unsigned char*)malloc(sizeof(char)+1); 

  for(k = 0; k < strlen(bin); k+=sizeof(char)*8) 
  {
    *buffer=0;
    for (i=0; i<sizeof(char)*8;i++){
	    n = (bin[k+i] - '0'); // char to numeric value
	    if ((n > 1) || (n < 0)) 
	    {
	      puts("\n\n ERROR! BINARY has only 1 and 0!\n");
	      return (-1);
	    }

	    // 1 2 4 8 16 32 64 ... place-values, reversed here
	    b = pow( 2, sizeof(char)*8-1-i );
	    // sum it up
	    *buffer = *buffer + n * b;
    }
    fwrite(buffer, sizeof(char), 1, f);
  }

  return 0;
}


/* WRITE A BLOCK OF BITS.  Bits are written as '0' and '1' characters, with
   no spaces between them, followed by a newline. */

void blockio_write
( FILE *f,     /* File to write to */
  char *b,     /* Block of bits to write */
  int l        /* Length of block */
)
{ 
  int i,temp_byte;
  char binary_crc[80];
  crc_t crc;

  temp_byte=0;

  crc = crc_init();
  fprintf(f, "\t<Data>");

  for (i = 0; i<l; i++)
  { if (b[i]!=0 && b[i]!=1) abort();
    putc("01"[b[i]],f);
    temp_byte = (temp_byte << 1) | (b[i] != 0);
    
    if ((i+1)%8==0){
	crc = crc_update(crc, (unsigned char *)&temp_byte, 1);      
	temp_byte=0;
    }
  }
  fprintf(f, "</Data>\n");
  
  crc = crc_finalize(crc);
  int2bin(crc,binary_crc);
  fprintf(f, "\t<Footer>\n\t\t<Data_Checksum>%s</Data_Checksum>\n\t\t<Version>%s</Version>\n\t</Footer>\n</Block>\n",binary_crc,version_3prime);
}

void blockio_write_nocrc
( FILE *f,     /* File to write to */
  char *b,     /* Block of bits to write */
  int l        /* Length of block */
)
{ 
  int i;

  for (i = 0; i<l; i++)
  { if (b[i]!=0 && b[i]!=1) abort();
    putc("01"[b[i]],f);
  }

  putc('\n',f);
}
