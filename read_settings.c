/*
 * This file is part of Efiedit.
 * Copyright (c) 2016 Juri Vitali <juri@dividebyzero.it>
 * 
 * Efiedit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <iconv.h>
#include <stdint.h>

#include "efiedit.h"

#define MAXLEN 2048

/* List boot entries */
int list_entries(void)
{
	return( system( efibootmgr ) );
}

/* Print boot_index entry name and associated additional data */
int print_entry(unsigned int boot_index)
{
	char filename [128];
	FILE* fd;
	
	int16_t content[MAXLEN] = { 0 };
	wchar_t output[MAXLEN] = { 0 };
	
	/* Boot entry filename */
	snprintf(filename, 128, "%s/Boot%04x-%s", efi_path, boot_index, boot_uid);
	
	/* Read file content in binary mode */
	fd = fopen(filename, "rb");
	if(fd==NULL){
		perror(filename);
		return -1;
	}
	fread(content, sizeof(int16_t), MAXLEN, fd);
	fclose(fd);
	
	/* The first 4 words (8 byte) contain various flags and are skipped,
	 * while the next word is the size (in bytes) of the data after the
	 * null-terminated string containing the bootentry label.
	 * The remaining bytes contain the remaining data.
	 * 
	 * | Flags (8 bytes) | Size (2B) | Label + \nul | Data (<Size> bytes) |+
	 * +| Additional data (Kernel cmdline) | */
	 
	int skip = content[4];
	
	/* Additional data is in UCS-2 format, and need to be converted. */
	char* inBuf = (char *)content;
	char* outBuf = (char *)output;
	size_t inLen, outLen;
	inLen = sizeof(content);
	outLen= sizeof(output);
	//inLen = outLen = MAXLEN;
	
	iconv_t conv = iconv_open("WCHAR_T//IGNORE", "UCS-2");
	if( iconv(conv, &inBuf, &inLen, &outBuf, &outLen) == -1 ){
		perror("iconv");
		return -2;
	}
	iconv_close(conv);
	
	/* Find the label string ending */
	wchar_t *endlabel = wcschr(output+5, L'\0');
	
	wchar_t label_name[MAXLEN], options[MAXLEN];
	
	wcscpy(label_name, output+5);
	
	/* Additional data begins skip bytes (skip/2 words)
	 * after the label string ending */
	wcscpy(options, endlabel + skip/2 + 1);
	
	printf("Bootentry %04x: label=%ls, options=%ls\n", boot_index, label_name, options);
	
	return 0;
}

int check_existing(unsigned bootnum)
{
	/* Returns 0 if the argument exists and is a regular file, nonzero otherwise */
	char cmd[MAXLEN];
	int ret = snprintf(cmd, MAXLEN, "/bin/test -e %s/Boot%04x-%s", efi_path, bootnum, boot_uid);
	if(ret < 0 || ret > MAXLEN){
		printf("snprintf failed\n");
		return -2;
	}
	
	return( system(cmd) );
}

/* Run efibootmgr without options to get current boot configuration,
 * then invoke grep and cut to cut out the boot order string. */
int read_bootorder(char* str, int len)
{
	FILE* pipe;
	char cmd[MAXLEN];
	
	if(str==NULL){ return -1; }
	
	/* Craft command string, to be passed to the shell */
	int ret = snprintf(cmd, MAXLEN, "%s | grep BootOrder | cut -d' ' -f2", efibootmgr);
	if(ret < 0 || ret > MAXLEN){
		printf("snprintf failed\n");
		return -2;
	}
	
	/* Open a pipe... */
	if( (pipe = popen(cmd, "r")) == NULL ){
		perror(cmd);
		return -2;
	}
	
	/* ...and read out the result. */
	fread(str, sizeof(char), len, pipe);
	if( feof(pipe) ){ return 0; }
	else return( ferror(pipe) );
}
