/*
 * This file is part of Efiedit.
 * Copyright (c) 2016, 2020 Juri Vitali <juri@dividebyzero.it>
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

/* ----------------------- Argp data structures ---------------------- */
const char* argp_program_version = "Efiedit 1.0.1";
static char doc[] ="Program to read and write EFI boot entries, "
					"using efibootmgr and efivars FS\v"
					"Options are aquired in the order they are parsed, by the program, so "
					"later options overwrite the previous ones with the same name.\n"
					"So, for example, if the -C switch is passed before the others options, "
					"the cmdline arguments take precedence over the configuration file, "
					"otherwise is the configuration file that takes precedence over the "
					"commandline options.";
					
static char args_doc[] = "[-W|--write] NUM [WRITE OPTIONS...]\n[-R|--read] NUM\n"
						 "[-O|--boot-order] NUM[,NUM[,NUM...]]\n[-S|--show]";
const char* argp_program_bug_address = "<juri@dividebyzero.it";

/* Program cmdline switches, passed to argp_parse */
static struct argp_option options[] =
{
  {0, 0, 0, 0, "General options:", 1 },
  {"config", 'C', "FILE", 0, "Load config file"},
  {"show", 'S', 0, 0, "Show current boot entries"},
  {"read", 'R', "XXXX", OPTION_ARG_OPTIONAL, "Read BootXXXX (hex)"},
  {"write", 'W', "XXXX", OPTION_ARG_OPTIONAL, "Write BootXXXX (hex)"},
  {"boot-order", 'O', "NUM[,NUM[,NUM...]]", 0, "Use custom bootorder instead of the current or the default one."},
  {0, 0, 0, 0, "WRITE OPTIONS:", 10 },
  {"disk", 'd', "DISK", 0, "Disk containing loader (Default: '/dev/sda')",},
  {"part", 'p', "NUM", 0, "Partition containing loader (Default: '1')"},
  {"loader", 'l', "PATH", 0, "Bootloader location, in EFI path format (Default '\\EFI\\grub\\grubx64.efi')"},
  {"label", 'L', "STRING", 0, "Boot entry label name (Default: 'Grub2 loader')"},
  {"cmdline", 'c', "STRING", 0, "Kernel cmdline"},
  {"preserve-existing", 'e', 0, 0, "Do not overwrite existing boot entries"},
  {"preserve-order", 'o', 0, 0, "Preserve existing order"},
  {0, 0, 0, 0, "OPTIONS:", -1 },
  {0}
};

/* Commandline parsing functions */
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static struct argp argp = {options, parse_opt, args_doc, doc};

/* Config file parsing functions */
int config_parse(const char* filename);
static int config_handler(void* options, const char* section, const char* name, const char* value);
