/* ENCODE.C - Encode message blocks. */

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>    

#include "rand.h"
#include "alloc.h"
#include "blockio.h"
#include "open.h"
#include "mod2sparse.h"
#include "mod2dense.h"
#include "mod2convert.h"
#include "rcode.h"
#include "enc.h"
#include "int2bin.h"
#include "crc.h"
#include "version.h"

void usage(void);


/* MAIN PROGRAM. */

int main
( int argc,
  char **argv
)
{
  crc_t crc;
  char *source_file, *encoded_file, *temp_file;
  char *pchk_file, *gen_file;
  mod2dense *u, *v;

  FILE *srcf, *encf;
  char *sblk, *cblk, *chks;
  char ch;
  char block_pos[80],header_crc[80];
  int i, n;
  int last_pos=0;
  int k;
  int fz; //file_size
  char terminator[4097]=version_terminator;
  

  /* Get current date */
  char date[30];
  size_t date_i;
  struct tm tim;
  time_t now;
  now = time(NULL);
  tim = *(localtime(&now));
  date_i = strftime(date,30,"%b %d, %Y; %H:%M:%S",&tim);

  
  /* Look at arguments. */

  if (!(pchk_file = argv[1])
   || !(gen_file = argv[2])
   || !(source_file = argv[3])
   || !(encoded_file = argv[4])
   || argv[5])
  { usage();
  }

  if ((strcmp(pchk_file,"-")==0) 
    + (strcmp(gen_file,"-")==0) 
    + (strcmp(source_file,"-")==0) > 1)
  { fprintf(stderr,"Can't read more than one stream from standard input\n");
    exit(1);
  }

  /* Read parity check file */

  read_pchk(pchk_file);

  if (N<=M)
  { fprintf(stderr,
 "Can't encode if number of bits (%d) not greater than number of checks (%d)\n",
      N,M);
    exit(1);
  }

  /* Read generator matrix file. */

  read_gen(gen_file,0,0);

  /* Allocate needed space. */

  if (type=='d')
  { u = mod2dense_allocate(N-M,1);
    v = mod2dense_allocate(M,1);
  }

  if (type=='m')
  { u = mod2dense_allocate(M,1);
    v = mod2dense_allocate(M,1);
  }

  /* Open source file. */

  srcf = open_file_std(source_file,"rb");
  if (srcf==NULL)
  { fprintf(stderr,"Can't open source file: %s\n",source_file);
    exit(1);
  }

  
  /* Create temp encoded output file. */
  temp_file=(char*)chk_alloc(strlen(encoded_file)+10, sizeof(char));
  strcat(temp_file,encoded_file);
  strcat(temp_file,".tmp");
  encf = open_file_std(temp_file,"w");
  if (encf==NULL)
  { fprintf(stderr,"Can't create file for temp encoded data: %s\n",temp_file);
    exit(1);
  }
  
  sblk = chk_alloc (N-M, sizeof *sblk);
  cblk = chk_alloc (N, sizeof *cblk);
  chks = chk_alloc (M, sizeof *chks);

  /* Prepare the double terminator */ 
  for (k=0;k<=(N-M)/190;k++){
    strcat(terminator, version_terminator);
  } 
  /* Encode successive blocks. */
  for (n = 0; ; n++)
  { 
    /* Read block from source file. */
    if (blockio_read_bin(srcf,sblk,N-M,&last_pos)==EOF) 
    { /* Pad the short block with double terminator seq */
	for (k=0;k+last_pos<N-M;k++){
	  sblk[last_pos+k]=terminator[k]=='1';
	}	
    }

    /* Compute encoded block. */

    switch (type)
    { case 's':
      { sparse_encode (sblk, cblk);
        break;
      }
      case 'd':
      { dense_encode (sblk, cblk, u, v);
        break;
      }
      case 'm':
      { mixed_encode (sblk, cblk, u, v);
        break;
      }
    }

    /* Check that encoded block is a code word. */

    mod2sparse_mulvec (H, cblk, chks);

    for (i = 0; i<M; i++) 
    { if (chks[i]==1)
      { fprintf(stderr,"Output block %d is not a code word!  (Fails check %d)\n",n,i);
        abort(); 
      }
    }
    /* Write block header */
    int2bin_evenpad(n,block_pos);
    crc = crc_init();
    crc = crc_update(crc, (unsigned char *)block_pos, strlen(block_pos));
    crc = crc_finalize(crc);
    int2bin(crc,header_crc);
    fprintf(encf,"<Block>\n\t<Header>\n\t\t<Version>%s</Version>\n\t\t<Position>%s</Position>\n\t\t<Header_Checksum>%s</Header_Checksum>\n\t</Header>\n",version_5prime,block_pos,header_crc);

    /* Write encoded block to encoded output file. */

    blockio_write(encf,cblk,N);

    /* Break if last block is the last block */
    if (feof(srcf)){
	fz=ftell(srcf);
	break;
    }
  }
  fprintf(encf,"</Blocks>\n</root>");
  fprintf(stderr,
    "Encoded %d blocks, source block size %d, encoded block size %d\nPosition %d to %d of the last block was padded with double terminator\n",n+1,N-M,N,last_pos,N-M);

  if (ferror(encf) || fclose(encf)!=0)
  { fprintf(stderr,"Error writing encoded blocks to %s\n",strcat(encoded_file,".tmp"));
    exit(1);
  }
  
  /* Write meta-information */
  fclose(srcf);
   
  srcf = open_file_std(temp_file,"r");
  if (srcf==NULL)
  { fprintf(stderr,"Can't open temp file: %s\n",temp_file);
    exit(1);
  }

  encf = open_file_std(encoded_file,"w");
  if (encf==NULL)
  { fprintf(stderr,"Can't create file for encoded data: %s\n",encoded_file);
    exit(1);
  }

  fprintf(encf, "<?xml version='1.0'?>\n<root>\n<Meta>\n\t<Source_file>%s</Source_file>\n\t<File_size>%d</File_size>\n\t<Num_Blocks>%d</Num_Blocks>\n\t<Last_pos>%d</Last_pos>\n\t<Date>%s</Date>\n</Meta>\n<Blocks>\n",source_file,fz,n+1,last_pos,date);
  
  /* Copy temp file to the new file */
  while(1)
    {
       ch = fgetc(srcf);

       if(ch==EOF)
          break;
       else
          putc(ch,encf);
    }

  if (ferror(encf) || fclose(encf)!=0)
  { fprintf(stderr,"Error writing encoded blocks to %s\n",temp_file);
    exit(1);
  }
  if( remove( temp_file ) == -1 )
        fprintf(stderr,"Error deleting temp file %s\n",temp_file);
        exit(1);
  
  return 0;
}


/* PRINT USAGE MESSAGE AND EXIT. */

void usage(void)
{ fprintf(stderr,
   "Usage:  encode pchk-file gen-file source-file encoded-file\n");
  exit(1);
}
