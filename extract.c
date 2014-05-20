/* EXTRACT.C - Extract message bits from coded blocks. */

/* Copyright (c) 2000, 2001 by Radford M. Neal 
 * Modified 2011 by Allen Yu
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

#include "alloc.h"
#include "blockio.h"
#include "open.h"
#include "mod2sparse.h"
#include "mod2dense.h"
#include "mod2convert.h"
#include "rcode.h"
#include "version.h"
#include "int2bin.h"

void usage(void);


/* MAIN PROGRAM. */

int main
( int argc,
  char **argv
)
{
  char *gen_file, *coded_file, *ext_file;
  FILE *codef, *extf;
  char *cblk;
  int i;

  /* Look at arguments. */

  if (!(gen_file = argv[1])
   || !(coded_file = argv[2])
   || !(ext_file = argv[3])
   || argv[4])
  { usage();
  }

  if ((strcmp(gen_file,"-")==0) + (strcmp(coded_file,"-")==0) > 1)
  { fprintf(stderr,"Can't read more than one stream from standard input\n");
    exit(1);
  }

  /* Read generator matrix file, up to the point of finding out which
     are the message bits. */
  
  read_gen(gen_file,1,1);

  /* Open decoded file. */

  codef = open_file_std(coded_file,"r");
  if (codef==NULL)
  { fprintf(stderr,"Can't open coded file: %s\n",coded_file);
    exit(1);
  }

  /* Open file to write extracted message bits to. */

  extf = open_file_std(ext_file,"wb");
  if (extf==NULL)
  { fprintf(stderr,"Can't create file for extracted bits: %s\n",ext_file);
    exit(1);
  }

  cblk = chk_alloc (N, sizeof *cblk);

  char block[N+1024];

  /* Read block from coded file. */
  if (blockio_read(codef,cblk,N)==EOF){fprintf(stderr,"Error reading decoded file!\n");}

  for (;;)
  { 

    /* Extract message bits and write to file, followed by newline to mark
       block boundary. */

    for (i = M; i<N; i++)
    { block[i-M]="01"[cblk[cols[i]]];
    }
   
    block[N-M]='\0';
    
    /* Check if last block */
    if (blockio_read(codef,cblk,N)==EOF) {
	char temp_str[strlen(version_terminator)];
        char *p;
        //remove terminator padding
	for (i=strlen(version_terminator);i>0;i--){
		strncpy(temp_str,version_terminator,i);
		//Try to find from longest to shortest
		if ((p=strstr(block,temp_str)) != NULL)
		{
			p[0]='\0';
			fprintf(stderr,"Successfully trimmed padding terminator!\n");
			break;
		}
	}
        /* Write buffered block */
        blockio_write_bin (extf, block, strlen(block)/8);
	break;
    }
    /* Write buffered block */
    blockio_write_bin (extf, block, strlen(block)/8);
  }

  if (ferror(extf) || fclose(extf)!=0)
  { fprintf(stderr,"Error writing extracted data to %s\n",ext_file);
    exit(1);
  }

  return 0;
}


/* PRINT USAGE MESSAGE AND EXIT. */

void usage(void)
{ fprintf(stderr,
    "Usage: extract gen-file decoded-file extracted-file\n");
  exit(1);
}
