# Efiedit
Efiedit is an efibootmgr wrapper to inspect and edit EFI boot entries.
It extends efibootmgr adding the possibility to inspect the content of existing boot entries,
and to read the option from a config file, enabling an easier managenment of boot option,
in a bootloader-y fashion.

## Requirements
This program should run on any GNU/Linux system, and maybe on other Unices (need to be tested).
* A POSIX-compliant system
* [EFIVARS-FS enabled in kernel](https://wiki.archlinux.org/index.php/Unified_Extensible_Firmware_Interface#Linux_Kernel_Config_options_for_UEFI)
* [efibootmgr](https://github.com/rhinstaller/efibootmgr)

## Contents
* Source files:
  * main.{c,h}, efiedit.h, read_settings.c, write_settings.c: main program source files;
  * ini.{c,h}: [config file parser, by Ben Hoyt](https://github.com/benhoyt/inih);
  * Makefile;
 * example.conf: an overview of various configuration options;
 * COPYING, LICENSE.GPLv3: legal notices, to be preserved in binary distributions (at least COPYING).
 
## Installation
Simply run `make` in the directory containing the source files.

## Usage
The various options may be passed directly by command line, or may be included in configuration files.
The options are parsed in the order they are passed on commandline or written in configuration files,
and if they are specified more than once later directives overwrite previous ones.
This means that if a configuration file path is passed to the program before other options, the latter
take precedence over the former, and viceversa.
 
The commandline options are:
* Actions:
  * `-S|--show`: Show current bootentries. Equivalent to `efibootmgr`.
  * `-R|--read[=NUM]`: Reads bootentry NUM from EFIVARS-FS path, and prints its label (name) and additional options (cmdline).
  * `-W|--write[=NUM]`: Writes bootentry NUM (must be between 0 and FFFF, and may be specified on config file).
      Equivalent to `echo CMDLINE | iconv -t ucs2 | efibootmgr -c -b NUM -d DISK -p PART -l LOADER -L LABEL -@ -`
  * `-O|--boot-order NUM[,NUM[,NUM...]]`: Sets boot order. May be specified alone or together the `-W` switch.
      Equivalent to `bootmgr -o NUM[,NUM[,NUM...]]`
  * `-C|--config=FILE`: load and the specified file. Allowed keys are specified in the table below.
* Write options:
  * `-d|--disk=STRING`: Select disk the bootloader is on. Default is `/dev/sda`.
  * `-p|--part=NUM`: Select the EFI partition number. Default is `1`.
  * `-l|--loader=STRING`: Select loader path, in EFI format. Default is `\EFI\grub\grubx64.efi`.
  * `-L|--label=STRING`: Set bootentry label name. Default `Grub2 loader`.
  * `-c|--cmdline=STRING`: Pass `STRING` as additional data, to be passed to the bootloader.
      If the loader is the Linux kernel itself used as EFIStub, this data becomes its commandline.
  * `-e|--preserve-existing`: If the selected bootentry altready exist, exit and do not overwrite it.
  * `-o|--preserve-order`: By default, if no explicit bootorder is specified `efibootmgr` sets the newly
      added entry as the first one. Pass this switch to restore the previous order.

## Options overview
|Commanline switch|Config file section|Config file key|
|-------|--------|---------|
|`-S\|--show`| *NONE* | *NONE* |
|`-R\|--read[=NUM]`| *NONE* | *NONE* |
|`-W\|--write[=NUM]`| *NONE* | *NONE* |
|`-O\|--boot-order NUM[,NUM[,NUM...]]` | `General` | `BootOrder = NUM[,NUM[,NUM...]]`|
|`-d\|--disk=STRING` | `BootEntry` | `Disk = STRING` |
|`-p\|--part=NUM` | `BootEntry` | `Partition = NUM` |
|`-l\|--loader=STRING` | `BootEntry` | `Loader = STRING` |
|`-L\|--label=STRING` | `Bootentry` | `Label = STRING` |
|`-c\|--cmdline=STRING` | `BootEntry` | `Cmdline = STRING` |
|`-e\|--preserve-existring` | *NONE* | *NONE* |
|`-o\|--preserve-order` | `General` | `Order = Preserve` |
|`-C\|--config` | `General` | `Include = FILE` |
| *NONE* | `General` | `Efibootmgr = PATH/TO/EFIBOOTMGR/BINARY` |
| *NONE* | `General` | `EfivarsPath = PATH/TO/EFIVARS/DIR` |
| *NONE* | `General` | `BootUid = BOOTENTRY-UID` |

## How to contribute
This program is free software, so feel free to use, experiment and modify it.
If you add some improvements, or find some bugs, you can open a pull request here
or simply send me an email, and I will be more than happy for your contribution :)
  
