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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <iconv.h>

#include "efiedit.h"

#define MAXLEN 1024

/* Pass parameters to efibootmgr to write out the selected boot entry */
int write_bootentry(struct arguments* args)
{
	unsigned bootnum = args->bootnum;
	char* device = args->disk;
	unsigned part = args->part;
	char* loader = args->loader;
	char* label = args->label;
	char* bootparams = args->cmdline;
	int flags = args->write_flags;
	char* bootorder = args->bootorder;
	
	FILE* pipe;
	char cmd[MAXLEN];
	int ret;

	char* output = malloc(MAXLEN*sizeof(char));
	char* inBuf = (char *)bootparams;
	char* outBuf = output;
	size_t bpLen, inLen, outLen;
	inLen = bpLen = strlen(inBuf);
	outLen = MAXLEN;
	char* orig_bootorder;
	
	if(bootnum > 0xFFFF){
		printf("Bootnum %X invalid: must be between 0 and FFFF\n", bootnum);
		return -1;
	}
	
	/* By default the last written entry becomes the default one.
	 * Using this flag we can control and deactivate this behaviour. */
	if( (flags & PRESERVE_ORDER) && (bootorder == NULL) )
	{
		orig_bootorder = calloc(MAXLEN, sizeof(char));
		if( (ret = read_bootorder(orig_bootorder, MAXLEN)) )
		{
			printf("Cannot read original bootorder\n");
			return ret;
		}
	}
	
	/* Check if the selected bootentry does altready exist, and if 
	 * it does and the PRESERVE_EXISTING flag is not set, it is deleted
	 * in order to be rewritten. Otherwise, if the flag is set, return
	 * with an error message. */
	if( !check_existing(bootnum) ){
		if( flags & PRESERVE_EXISTING ){
			printf("Entry altready exists and preserve-existing option is enabled.\n");
			return -1;
		}
		else if( delete_bootentry(bootnum) ){
			printf("Failed to delete existing bootentry no. %04x\n", bootnum);
			return -2;
		}
	}
	
	/* Craft efibootmgr configuration string */
	ret = snprintf(cmd, MAXLEN, "%s -c -b %04x -d %s -p %d -l \"%s\" -L \"%s\" -@ -",
								efibootmgr, bootnum, device, part, loader, label);

	if(ret < 0 || ret > MAXLEN){
		printf("snprintf failed\n");
		return -2;
	}
	
	/* Additional data needs to be passed separately in UCS-2 format,
	 * we do it using a pipe. */
	if( (pipe = popen(cmd, "w")) == NULL) {
		perror(cmd);
	}
	
	/* Convert data to UCS-2 format using iconv */
	iconv_t conv = iconv_open("UCS-2", "ASCII");
	if( iconv(conv, &inBuf, &inLen, &outBuf, &outLen) == -1 ){
		perror("iconv: ");
		free(output);
		return -2;
	}
	iconv_close(conv);
	
	/* Write data to pipe */
	fwrite(output, sizeof(char), bpLen*2, pipe);
	
	free(output);
	
	ret = pclose(pipe);
	
	/* If the PRESERVE_ORDER flag is set, or bootorder in not NULL,
	 * and efibootmgr returned without errors, restore original or custom bootorder. */
	if( !ret )
	{
		if( (flags & PRESERVE_ORDER) || (bootorder != NULL) )
		{
			if ( bootorder == NULL ) {
				ret = set_bootorder(orig_bootorder);
				free(orig_bootorder);
			} else {
				ret = set_bootorder(bootorder);
			}
		}
		return ret;
	}
	else return ret;
}

/* If a bootentry altready exists, it needs to be deleted first to be modified */
int delete_bootentry(unsigned int bootnum){
	/* Returns 0 if the deletion suceeded, nonzero otherwise */
	char cmd[MAXLEN];
	int ret = snprintf(cmd, MAXLEN, "%s -q -B -b %04x", efibootmgr, bootnum);
	if(ret < 0 || ret > MAXLEN){
		printf("snprintf failed\n");
		return -2;
	}
	return( system(cmd) );
}

/* Write bootorder passed in string (in format NUM,NUM,...) */
int set_bootorder(const char* string){
	char cmd[MAXLEN];
	int ret = snprintf(cmd, MAXLEN, "%s -q -o %s", efibootmgr, string);
		if(ret < 0 || ret > MAXLEN)
		{
			printf("snprintf failed\n");
			return -2;
		}
		return( system(cmd) );
}
