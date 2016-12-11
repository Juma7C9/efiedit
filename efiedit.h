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

#ifndef _EFIEDIT_H
#define _EFIEDIT_H

/* --------------------------- Write flags --------------------------- */
#define PRESERVE_EXISTING 1
#define PRESERVE_ORDER 2

/* ------------------ Program configuration options ------------------ */

const char* efibootmgr; /* "/usr/bin/efibootmgr */
const char* efi_path; /* "/sys/firmware/efi/efivars" */
const char* boot_uid; /* "8be4df61-93ca-11d2-aa0d-00e098032b8c" */

struct arguments {
	/* Program mode */
	int list;
	int read;
	int write;
	/* Options passed to efibootmgr */
	unsigned bootnum;
	char* disk;
	unsigned part;
	char* loader;
	char* label;
	char* cmdline;
	int write_flags;
	char* bootorder;
};

/* ---------------------- Function prototypes ------------------------ */
 
/* read_settings.c */
int list_entries(void);
int read_bootorder(char* str, int len);
int print_entry(unsigned int);
int check_existing(unsigned int);

/* write_settings.c */
int write_bootentry(struct arguments* args);
int delete_bootentry(unsigned int bootnum);
int set_bootorder(const char* str);

#endif
