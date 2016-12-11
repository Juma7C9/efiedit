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
#include <argp.h>
#include "ini.h"
#include "efiedit.h"
#include "main.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

struct arguments opts;

int main(int argc, char** argv)
{
	/* Options are read in the order they are passed to the program,
	 * and later options overwrite the previous ones with the same name.
	 * So, for example, if the -C switch is passed before the others options,
	 * the cmdline arguments overwrites the configuration file,
	 * otherwise is the configuration file that takes precedence over the
	 * commandline options. */
	
	/* Default options */
	efibootmgr = "/usr/bin/efibootmgr";
	efi_path = "/sys/firmware/efi/efivars";
	boot_uid = "8be4df61-93ca-11d2-aa0d-00e098032b8c";
	
	opts.read = opts.write = opts.list = 0;
	opts.bootnum = opts.write_flags = 0;
	opts.bootorder = NULL;
	opts.disk = "/dev/sda";
	opts.part = 1;
	opts.loader = "\\EFI\\grub\\grubx64.efi";
	opts.label = "Grub2 loader";
	opts.cmdline = "";

	/* Parse commandline; arguments are processed in the order the appear. */
	argp_parse(&argp, argc, argv, 0, NULL, &opts);
	
	/* Run the correct functions. Safeguards are in place
	 * so only one of the (R|W|S) action is set at a time.
	 * The (O) action is run inside the write block if (W) is set,
	 * separately otherwise. */
	if (opts.read){
		exit( print_entry(opts.bootnum) ); 
	} else if (opts.write){
		exit( write_bootentry(&opts) );
	} else if (opts.bootorder){
		exit( set_bootorder(opts.bootorder) );
	} else { 
		/* Default option: list entries. */
		exit(list_entries());
	}
}

/* Commandline options parser function. Called by argp_parse() */
/* Returns 0 on succees, nonzero on failure. */
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments* args = state->input;
	
	char* endptr;
	int ret;
	
	switch(key){
		case 'C':
			ret = config_parse(arg);
			if( ret < 0 ){
				argp_failure(state, ret, 0, "Error while parsing configuration.");
			}
			break;
		case 'S':
			args->list = 1;
			break;
		case 'R':
			args->read = 1;
			if( arg != NULL ) {
				args->bootnum = (unsigned)strtoul(arg, &endptr, 0);
				if( (endptr == arg) || (args->bootnum > 0xFFFF) ) {
					argp_error(state, "Bootnum '%s' invalid: must be a number between 0 and 0xFFFF.", arg);
					return ARGP_ERR_UNKNOWN;
				}
			}
			break;
		case 'W':
			args->write = 1;
			if( arg != NULL ) {
				args->bootnum = (unsigned)strtoul(arg, &endptr, 0);
				if( (endptr == arg) || (args->bootnum > 0xFFFF) ) {
					argp_error(state, "Bootnum '%s' invalid: must be a number between 0 and 0xFFFF.", arg);
					return ARGP_ERR_UNKNOWN;
				}
			}
			break;
		case 'O':
			args->bootorder = arg;
			break;
		case 'd':
			args->disk = arg;
			break;
		case 'p':
			if ( arg != NULL ) {
				args->part = (unsigned)strtoul(arg, &endptr, 0);
				if( endptr == arg ) {
					argp_error(state, "Invalid partition '%s': must be a number.", arg);
					return ARGP_ERR_UNKNOWN;
				}
			}
			break;
		case 'l':
			args->loader = arg;
			break;
		case 'L':
			args->label = arg;
			break;
		case 'c':
			args->cmdline = arg;
			break;
		case 'e':
			args->write_flags |= PRESERVE_EXISTING;
			break;
		case 'o':
			args->write_flags |= PRESERVE_ORDER;
			args->bootorder = NULL;
			break;
		case ARGP_KEY_ARG:
			argp_error(state, "Too many arguments.");
			return ARGP_ERR_UNKNOWN;
			break;
		case ARGP_KEY_END:
			if(args->write + args->read + args->list > 1){
				argp_error(state, "Only one of -S, -R and -W may be specified together.");
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/* Configuration file parser. Invoked by ini_parse(). */
/* Returns nonzero on success, 0 on failure. */
static int config_handler(void* options, const char* section, const char* name, const char* value)
{
	char* endptr;
	struct arguments* opts = (struct arguments*)options;
	
	if( MATCH("General", "Include") ){
		return( config_parse(value) );
	}
	else if( MATCH("General", "Efibootmgr") ){
		efibootmgr = strdup(value);
	}
	else if( MATCH("General", "EfivarsPath") ){
		efi_path = strdup(value);
	}
	else if( MATCH("General", "BootUid") ){
		boot_uid = strdup(value);
	}
	else if( MATCH("General", "BootOrder") ){
		if ( strcmp(value, "Preserve") == 0) {
			opts->bootorder=NULL;
			opts->write_flags |= PRESERVE_ORDER;
		} else {
			opts->bootorder = strdup(value);
		}
	}
	else if( MATCH("BootEntry", "Bootnum") ){
		opts->bootnum = (unsigned)strtoul(value, &endptr, 0);
		if( endptr == value ) {
			printf("Invalid value '%s' in option '%s' in section '%s': must be a number.\n", value, name, section);
			return 0;
		}
	}
	else if( MATCH("BootEntry", "Disk") ){
		opts->disk = strdup(value);
	}
	else if( MATCH("BootEntry", "Partition") ){
		opts->part = (unsigned)strtoul(value, &endptr, 0);
		if( endptr == value ) {
			printf("Invalid value '%s' in option '%s' in section '%s': must be a number.\n", value, name, section);
			return 0;
		}
	}
	else if( MATCH("BootEntry", "Loader") ){
		opts->loader = strdup(value);
	}
	else if( MATCH("BootEntry", "Label") ){
		opts->label = strdup(value);
	}
	else if( MATCH("BootEntry", "Cmdline") ){
		opts->cmdline = strdup(value);
	}
	else { printf("Ignored unknown option %s in section %s\n", name, section); }
	
	return 1;
}

/* Wrapper for ini_parse(). config_handler and opts are globally definded. */
int config_parse(const char* filename)
{
	int ret = ini_parse(filename, config_handler, &opts);
	if (ret < 0) {
        printf("Error: Can't load %s\n", filename);
    }
    return ret;
}
