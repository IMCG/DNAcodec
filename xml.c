/* xml.c - Routines to parse xml files produced by DNAIO. */

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


#include <stdio.h>
#include <string.h>
#include <libxml/xmlreader.h>

#ifdef LIBXML_READER_ENABLED

/**
 * parse_node:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */

void parse_node(xmlTextReaderPtr reader,const xmlChar **name,const xmlChar **value, int *type) {
    *name = xmlTextReaderConstName(reader);
    if (*name == NULL)
    	*name = BAD_CAST "--";

    *value = xmlTextReaderConstValue(reader);
    *type = xmlTextReaderNodeType(reader);
}

void parse_meta(xmlTextReaderPtr reader,char meta_tags[64][32], char meta_values[64][1024])
{
	const xmlChar *name, *value;
	int type, ret, cur_tag=0;

	//Read the next element
	ret = xmlTextReaderRead(reader);
	parse_node(reader, &name, &value, &type);

	while (ret == 1 && (xmlStrcmp(name, (const xmlChar *)"Meta"))) {
		if (xmlStrcmp(name, (const xmlChar *)"#text")){
			strcpy(meta_tags[cur_tag], name);

			//Get the #text element value
			ret = xmlTextReaderRead(reader);
			parse_node(reader, &name, &value, &type);
			strcpy(meta_values[cur_tag], value);

			//Skip the closing tags
			while (ret == 1 && (xmlStrcmp(name, meta_tags[cur_tag])))
			{
				ret = xmlTextReaderRead(reader);
				parse_node(reader, &name, &value, &type);
			}
			cur_tag+=1;
		}

		//Read the next record
		ret = xmlTextReaderRead(reader);
		parse_node(reader, &name, &value, &type);

	}

	//Skip the closing meta tag
	ret = xmlTextReaderRead(reader);

	meta_tags[cur_tag][0] = '\0';
	meta_values[cur_tag][0] = '\0';

}

void parse_block(xmlTextReaderPtr reader,char *header_version, char *pos, char *header_checksum,
		char *data, char *data_checksum, char *footer_version)
{
	const xmlChar *name, *value;
	int type, ret;

	//Read the next element
	ret = xmlTextReaderRead(reader);
	parse_node(reader, &name, &value, &type);

	while (ret == 1 && (xmlStrcmp(name, (const xmlChar *)"Block"))) {
		if ((!xmlStrcmp(name, (const xmlChar *)"Header"))) {
			ret = xmlTextReaderRead(reader);
			parse_node(reader, &name, &value, &type);

			while (ret == 1 && (xmlStrcmp(name, (const xmlChar *)"Header"))) {
				if (!xmlStrcmp(name, (const xmlChar *)"Version")){
					//Get the #text element value
					ret = xmlTextReaderRead(reader);
					parse_node(reader, &name, &value, &type);
					strcpy(header_version, value);
					//Get pass the closing element
					ret = xmlTextReaderRead(reader);
				}
				else if (!xmlStrcmp(name, (const xmlChar *)"Position")){
					ret = xmlTextReaderRead(reader);
					parse_node(reader, &name, &value, &type);
					strcpy(pos, value);
					ret = xmlTextReaderRead(reader);
				}
				else if (!xmlStrcmp(name, (const xmlChar *)"Header_Checksum")){
					ret = xmlTextReaderRead(reader);
					parse_node(reader, &name, &value, &type);
					strcpy(header_checksum, value);
					ret = xmlTextReaderRead(reader);
				}
				ret = xmlTextReaderRead(reader);
				parse_node(reader, &name, &value, &type);
			}
		}
		else if ((!xmlStrcmp(name, (const xmlChar *)"Data"))) {
			ret = xmlTextReaderRead(reader);
			parse_node(reader, &name, &value, &type);
			strcpy(data, value);
			ret = xmlTextReaderRead(reader);
		}
		else if ((!xmlStrcmp(name, (const xmlChar *)"Footer"))) {
			ret = xmlTextReaderRead(reader);
			parse_node(reader, &name, &value, &type);
			while (ret == 1 && (xmlStrcmp(name, (const xmlChar *)"Footer"))) {
				if (!xmlStrcmp(name, (const xmlChar *)"Data_Checksum")){
					ret = xmlTextReaderRead(reader);
					parse_node(reader, &name, &value, &type);
					strcpy(data_checksum, value);
					ret = xmlTextReaderRead(reader);
				}
				else if (!xmlStrcmp(name, (const xmlChar *)"Version")){
					ret = xmlTextReaderRead(reader);
					parse_node(reader, &name, &value, &type);
					strcpy(footer_version, value);
					ret = xmlTextReaderRead(reader);
				}
				ret = xmlTextReaderRead(reader);
				parse_node(reader, &name, &value, &type);
			}
		}
		ret = xmlTextReaderRead(reader);
		parse_node(reader, &name, &value, &type);
	}
}

#else
int main(void) {
    fprintf(stderr, "XInclude support not compiled in\n");
    exit(1);
}
#endif
