/* DNAIO.C - Routines to convert quarternary coded DNA to/from a text file. */

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> 
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <libxml/encoding.h>

#include "open.h"
#include "version.h"
#include "int2bin.h"
#include "bin2dec.h"
#include "crc.h"
#include "str_match.h"
#include "xml.h"

#define MY_ENCODING "ISO-8859-1"

#ifdef LIBXML_READER_ENABLED

/**
 * DNA2bin:
 * @src: the input string
 * @dest: the output string
 *
 * Convert DNA sequence to binary string
 */
static char DNA2bin ( char *src, char *dest ){
	int i=0;

	for( i = 0; src[i] != '\0'; i++)
	{
		switch (src[i])
		{
			case 'A':
				{
					dest[i*2]='0';
					dest[i*2+1]='0';
					break;
				}
			case 'T':
				{
					dest[i*2]='0';
					dest[i*2+1]='1';
					break;
				}
			case 'C':
				{
					dest[i*2]='1';
					dest[i*2+1]='0';
					break;
				}
			case 'G':
				{
					dest[i*2]='1';
					dest[i*2+1]='1';
					break;
				}
			default:
				{
					fprintf(stderr,"Incorrect base %c in source file!\n",src[i]);
					exit(1);
				}
		}
	}
	dest[i*2]= '\0';
}

/**
 * bin2DNA:
 * @src: the input string
 * @dest: the output string
 *
 * Convert binary string to DNA sequence
 */
static char bin2DNA ( char *src, char *dest ){
	int i, flag;

	for( i = 0; src[i] != '\0'; i+=2)
	{
		flag=0;
		if (src[i]!='0' && src[i]!='1' && src[i+1]!='0' && src[i+1]!='1')
		{
			fprintf(stderr, "Bad character %c%c in binary file (not '0' or '1')\n",src[i],src[i+1]);
			exit(1);
		}

		flag+=2*(src[i]=='1');
		flag+=src[i+1]=='1';

		dest[i/2]="ATCG"[flag];
	}
	dest[i/2]='\0';
}


/**
 * parse_DNA_blocks:
 * @source_file: the input file
 * @output_file: the output file
 *
 * Parse the assembled DNA blocks, remove the version tags as well as
 * recombination repeats, check the CRC signature, and write to binary blocks
 */
static void parse_DNA_blocks ( char *source_file, char *output_file){
	char temp_fname[32];
	char temp_str[65536];
	char temp_bin[65536];
	char block_pos[80],header_crc[80],header_crc2[80],data_crc[80],data_crc2[80];

	char *p;

	FILE *srcf, *encf, *tmpf;
	char c;
	int i, j=0, n,k, line=0,last_addr=0 ;
	crc_t crc;

	/* Open source file. */
	srcf = open_file_std(source_file,"r");
	if (srcf==NULL)
	{ fprintf(stderr,"Can't open source file: %s\n",source_file);
	exit(1);
	}

	/* Create output file. */
	encf = open_file_std(output_file,"w");
	if (encf==NULL)
	{ fprintf(stderr,"Can't create file for encoded data: %s\n",output_file);
	exit(1);
	}

	while(!feof( srcf ))
	{
		line++;

		//Skip the header line of fasta
		if(fgets(temp_str,65536,srcf)==NULL) continue;
		else if (temp_str[0] == '\n') continue;
		else if (temp_str[0] != '>'){
			fprintf(stderr,"Error in fasta format\n");
		}

		//Convert consensus DNA to binary, sorted by address
		if(fgets(temp_str,65536,srcf)!=NULL) {
			//Remove newline character
			temp_str[strlen(temp_str)-1]='\0';
			//Check if version tags match
			char c[strlen(version_5prime_DNA)];
			int dist5=0, dist3=0;
			strncpy(c,temp_str,strlen(version_5prime_DNA));
			c[strlen(version_5prime_DNA)]='\0';
			dist5=ldistance((char *)c,(char *)version_5prime_DNA);
			if (dist5<=1){
				//Truncate the 5' version tag
				memmove(temp_str,temp_str+strlen(version_5prime_DNA),strlen(temp_str)+1);
			}

			strncpy(c,temp_str+strlen(temp_str)-strlen(version_5prime_DNA),strlen(version_5prime_DNA));

			c[strlen(version_5prime_DNA)]='\0';
			dist3=ldistance((char *)c,(char *)version_3prime_DNA);
			if (dist3<=1){
				//Truncate the 3' version tag
				temp_str[strlen(temp_str)-strlen(version_5prime_DNA)]='\0';
			}

			/*
			//remove all repeats --Todo: Should allow rearrangement
			if ((p=strstr(temp_str,repeat1)) != NULL)
			{
				fprintf(stderr,"\tRepeat 1 found and removed!\n");
				memmove(p,p+strlen(repeat1), strlen(p+strlen(repeat1))+1);
			}else{
				fprintf(stderr,"\tExact match of Repeat 1 not found, ambigously removed!\n");
				memmove(temp_str+header_lgth,temp_str+header_lgth+strlen(repeat1), strlen(temp_str)+1);
			}
			if ((p=strstr(temp_str,repeat2)) != NULL)
			{
				fprintf(stderr,"\tRepeat 2 found and removed!\n");
				memmove(p,p+strlen(repeat2), strlen(p+strlen(repeat2))+1);
			}else{
				fprintf(stderr,"\tExact match of Repeat 2 not found, ambigously removed!\n");
				memmove(temp_str+repeat2_pos,temp_str+repeat2_pos+strlen(repeat2), strlen(temp_str)+1);
			}
			if ((p=strstr(temp_str,repeat3)) != NULL)
			{
				fprintf(stderr,"\tRepeat 3 found and removed!\n");
				memmove(p,p+strlen(repeat3), strlen(p+strlen(repeat3))+1);
			}else{
				fprintf(stderr,"\tExact match of Repeat 3 not found, ambigously removed!\n");
				temp_str[strlen(temp_str)-strlen(repeat3)]='\0';
			}
			*/

			//Convert Nuc to Bin
			if (dist5<=1 && dist3<=1){
				i=0;
				while (temp_str[i]!='\0'){
					switch (temp_str[i])
					{ case 'A':
					{ temp_bin[i*2]='0';
					temp_bin[i*2+1]='0';
					break;
					}
					case 'T':
						{ temp_bin[i*2]='0';
						temp_bin[i*2+1]='1';
						break;
						}
					case 'C':
						{ temp_bin[i*2]='1';
						temp_bin[i*2+1]='0';
						break;
						}
					case 'G':
						{ temp_bin[i*2]='1';
						temp_bin[i*2+1]='1';
						break;
						}
					default:
						{ fprintf(stderr,"Incorrect base %c in source file!\n",temp_str[i]);
						exit(1);
						}
					}
					i++;
				}
			}else{
				fprintf(stderr,"Incorrect version tags in source file! %d %d\n",dist5,dist3);
				exit(1);
			}
			temp_bin[i*2+2]='\0';

			//Try to match the block position to its CRC signature
			int correct_header=-1;
			for (j=1;j<16;j++){
				strncpy(block_pos,temp_bin,j);
				block_pos[j]='\0';
				strncpy(header_crc,temp_bin+j,32);
				header_crc[32]='\0';
				crc = crc_init();
				crc = crc_update(crc, (unsigned char *)block_pos, strlen(block_pos));
				crc = crc_finalize(crc);
				int2bin(crc,header_crc2);
				if (strcmp(header_crc,header_crc2)==0){
					fprintf(stderr,"Correct header checksum found at block %d! \n",line);
					correct_header=bin2dec(block_pos);
					break;
				}
			}
			//Correct header is found
			if (correct_header>=0){
				if (correct_header>last_addr){last_addr=correct_header;}

				//truncate the header information
				memmove(temp_bin,temp_bin+strlen(block_pos)+32,strlen(temp_bin)+1);

				//Remove the data checksum --Todo: rearrange blocks if data checksum mismatch
				temp_bin[strlen(temp_bin)-32]='\n';
				temp_bin[strlen(temp_bin)-31]='\0';

				//Write the block
				fputs(temp_bin, encf);
				if (ferror(encf))
				{ fprintf(stderr,"Error writing block output file\n");
				exit(1);
				}else{ fprintf(stderr,"\tBlock %d data extracted!\n",line);}

			}
			else{
				fprintf(stderr,"Can't find proper address at line %d!\n",line);
				exit(1);
			}
		}
	}

	if (fclose(encf)!=0) fprintf(stderr,"Error closing output file!\n");
}

static void convert_xml(char *source_file, char *output_file, int mode)
{
	FILE *encf;

	// XML handling routine
	LIBXML_TEST_VERSION

	xmlTextReaderPtr reader;
	xmlTextWriterPtr writer;

	const xmlChar *name, *value;
	xmlChar *tmp;

	//array to hold multiple meta tags and values
	char meta_tags[64][32], meta_values[64][1024];

	// Data for each block
	char header_version[65], pos[34], header_checksum[34], data[2049], data_checksum[34], footer_version[65];
	char buf[4097];
	int i, type, ret, rc; //ret for xmlTextReader, rc for xmlTextWriter

	/* Create XML output only if mode is -b or -d */
	if (mode==1 || mode==2){
		/* Create a new XmlWriter for output_file, with no compression. */
		writer = xmlNewTextWriterFilename(output_file, 0);
		if (writer == NULL) {
			fprintf(stderr,"testXmlwriterFilename: Error creating the xml writer\n");
			return;
		}

		/* Start the document with the xml default for the version,
		 * encoding ISO 8859-1 and the default for the standalone
		 * declaration. */
		rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
		if (rc < 0) {
			fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartDocument\n");
			return;
		}

		/* Start an element named "root" */
		rc = xmlTextWriterStartElement(writer, BAD_CAST "root");
		if (rc < 0) {
			fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
			return;
		}

		/* Start an element named "Meta" as child of root. */
		rc = xmlTextWriterStartElement(writer, BAD_CAST "Meta");
		if (rc < 0) {
			fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
			return;
		}
	}else{
		/* Create output file. */
		encf = open_file_std(output_file,"w");
		if (encf==NULL)
		{ fprintf(stderr,"Can't create file for encoded data: %s\n",output_file);
		exit(1);
		}
	}

	reader = xmlReaderForFile(source_file, NULL, 0);
	if (reader != NULL) {
		ret = xmlTextReaderRead(reader);
		while (ret == 1) {
			parse_node(reader, &name, &value, &type);

			//Write the Meta information parsed from source file, if output is xml
			if ((mode==1 || mode==2) && (!xmlStrcmp(name, (const xmlChar *)"Meta"))) {
				parse_meta(reader, meta_tags, meta_values);
				for (i=0; meta_tags[i][0] != '\0'; i++)
				{
					fprintf(stderr,"%s:\t%s\n", meta_tags[i], meta_values[i]);
					rc = xmlTextWriterWriteElement(writer, BAD_CAST meta_tags[i],BAD_CAST meta_values[i]);
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
						return;
					}
				}

				/* Close the element named Meta. */
				rc = xmlTextWriterEndElement(writer);
				if (rc < 0) {
					fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
					return;
				}
				/* Start an element named "Blocks" as child of root. */
				rc = xmlTextWriterStartElement(writer, BAD_CAST "Blocks");
				if (rc < 0) {
					fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
					return;
				}
			}
			else if ((!xmlStrcmp(name, (const xmlChar *)"Block"))) {

				if (mode==1 || mode==2){
					/* Start an element named "Block" as child of Blocks. */
					rc = xmlTextWriterStartElement(writer, BAD_CAST "Block");
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
						return;
					}
				}

				parse_block(reader, header_version, pos, header_checksum, data, data_checksum, footer_version);

				/* From DNA to binary. */
				if (mode==1){
					DNA2bin(header_version, buf);
					strncpy(header_version, buf, 65);
					DNA2bin(pos, buf);
					strncpy(pos, buf, 34);
					DNA2bin(header_checksum, buf);
					strncpy(header_checksum, buf, 34);
					DNA2bin(data, buf);
					strncpy(data, buf, 2049);
					DNA2bin(data_checksum, buf);
					strncpy(data_checksum, buf, 34);
					DNA2bin(footer_version, buf);
					strncpy(footer_version, buf, 65);
				}
				/* From binary to DNA. */
				else if (mode==2 || mode==3){
					bin2DNA(header_version, buf);
					strncpy(header_version, buf, 65);
					bin2DNA(pos, buf);
					strncpy(pos, buf, 34);
					bin2DNA(header_checksum, buf);
					strncpy(header_checksum, buf, 34);
					bin2DNA(data, buf);
					strncpy(data, buf, 2049);
					bin2DNA(data_checksum, buf);
					strncpy(data_checksum, buf, 34);
					bin2DNA(footer_version, buf);
					strncpy(footer_version, buf, 65);
				}


				if (mode==3){
					/* Create fasta output */
					fprintf(encf,">%s\n%s%s%s%s%s%s\n",pos,header_version,pos,header_checksum,data,data_checksum,footer_version);
				}
				else if (mode==1 || mode==2){
					/* Write XML output */
					/* Start an element named "Header" as child of Block. */
					rc = xmlTextWriterStartElement(writer, BAD_CAST "Header");
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
						return;
					}

					/* Write an element named "Version" as child of Header. */
					rc = xmlTextWriterWriteElement(writer, BAD_CAST "Version",BAD_CAST header_version);
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
						return;
					}

					/* Write an element named "Position" as child of Header. */
					rc = xmlTextWriterWriteElement(writer, BAD_CAST "Position",BAD_CAST pos);
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
						return;
					}

					/* Write an element named "Header_Checksum" as child of Header. */
					rc = xmlTextWriterWriteElement(writer, BAD_CAST "Header_Checksum",BAD_CAST header_checksum);
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
						return;
					}

					/* Close the element named Header. */
					rc = xmlTextWriterEndElement(writer);
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
						return;
					}

					/* Write an element named "Data" as child of Block. */
					rc = xmlTextWriterWriteElement(writer, BAD_CAST "Data",BAD_CAST data);
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
						return;
					}

					/* Start an element named "Footer" as child of Block. */
					rc = xmlTextWriterStartElement(writer, BAD_CAST "Footer");
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
						return;
					}

					/* Write an element named "Footer_Checksum" as child of Footer. */
					rc = xmlTextWriterWriteElement(writer, BAD_CAST "Data_Checksum",BAD_CAST data_checksum);
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
						return;
					}

					/* Write an element named "Version" as child of Header. */
					rc = xmlTextWriterWriteElement(writer, BAD_CAST "Version",BAD_CAST footer_version);
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
						return;
					}

					/* Close the element named Footer. */
					rc = xmlTextWriterEndElement(writer);
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
						return;
					}

					/* Close the element named Block. */
					rc = xmlTextWriterEndElement(writer);
					if (rc < 0) {
						fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
						return;
					}
				}
			}
			ret = xmlTextReaderRead(reader);
		}

		if (mode==1 || mode==2){
			/* Here we could close the elements because we do not want to
			 * write any other elements. */
			rc = xmlTextWriterEndDocument(writer);
			if (rc < 0) {
				fprintf(stderr,"testXmlwriterFilename: Error at xmlTextWriterEndDocument\n");
				return;
			}

			xmlFreeTextWriter(writer);
		}
		else if (mode==3)
		{
			if (ferror(encf) || fclose(encf)!=0)
			{ fprintf(stderr,"Error closing output file\n");
			exit(1);
			}
		}

		xmlFreeTextReader(reader);
		if (ret != 0) {
			fprintf(stderr, "%s : failed to parse\n", source_file);
		}
	} else {
		fprintf(stderr, "Unable to open %s\n", source_file);
	}

	xmlCleanupParser();
}

static void usage(void)
{ fprintf(stderr,
		  "Usage:  DNAIO -b|-d|-f|-c source-file output-file\n\n-b Converts from DNA XML to binary XML\n-d Converts from binary XML to DNA XML\n-f Converts from binary XML to DNA fasta\n-c Converts from DNA fasta to binary blocks\n");
exit(1);
}

/* MAIN PROGRAM. */
int main ( int argc,  char **argv)
{
	char *source_file, *output_file;
	int mode=0;

	/* Look at arguments. */
	if (!(source_file = argv[2])
		|| !(output_file = argv[3])
		|| argv[4])
	{ usage();
	}

	if (strcmp(argv[1],"-b")!=0 && strcmp(argv[1],"-d")!=0 && strcmp(argv[1],"-f")!=0 && strcmp(argv[1],"-c")!=0)
	{ usage();
	}

	if (strcmp(argv[1],"-b")==0){
		mode=1;
	}else if (strcmp(argv[1],"-d")==0){
		mode=2;
	}else if (strcmp(argv[1],"-f")==0){
		mode=3;
	}else if (strcmp(argv[1],"-c")==0){
		mode=4;
	}

	if (mode==4){
		parse_DNA_blocks ( source_file, output_file);
	}
	else{
		convert_xml(source_file, output_file, mode);
	}

	return(0);
}

#else
int main(void) {
    fprintf(stderr, "XInclude support not compiled in\n");
    exit(1);
}
#endif
