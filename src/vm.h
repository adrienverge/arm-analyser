/**
 * @file    vm.h
 * @author  Adrien Vergé <adrien.verge@polymtl.ca>
 * @date    March 2013
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This file sets up a "Virtual Machine" capable of loading a binary file, load
 * its executable sections and looking up for instructions or any data in the
 * virtual address space of the program.
 */

#if !defined(VM_H)
#define VM_H

#include <gelf.h>
#include <libelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"

#define NAMES_LENGTH	64

#define ELF_PTABLE_TYPE_NAME(val)	(val==PT_NULL?"PT_NULL":val==PT_LOAD?\
	"PT_LOAD":val==PT_DYNAMIC?"PT_DYNAMIC":val==PT_INTERP?"PT_INTERP":val==\
	PT_NOTE?"PT_NOTE":val==PT_SHLIB?"PT_SHLIB":val==PT_PHDR?"PT_PHDR":val==\
	PT_LOPROC?"PT_LOPROC":val==PT_HIPROC?"PT_HIPROC":"unknown")
#define ELF_PTABLE_FLAGS_R_NAME(val)	(val&PF_R?"R":"-")
#define ELF_PTABLE_FLAGS_W_NAME(val)	(val&PF_W?"W":"-")
#define ELF_PTABLE_FLAGS_X_NAME(val)	(val&PF_X?"X":"-")

#define ELF_HEADER_MACHINE_NAME(val)	(val==EM_NONE?"EM_NONE":val==EM_M32?\
	"EM_M32":val==EM_SPARC?"EM_SPARC":val==EM_386?"EM_386":val==EM_68K?"EM_68K"\
	:val==EM_88K?"EM_88K":val==EM_860?"EM_860":val==EM_MIPS?"EM_MIPS":val==\
	EM_PARISC?"EM_PARISC":val==EM_SPARC32PLUS?"EM_SPARC32PLUS":val==EM_PPC?\
	"EM_PPC":val==EM_PPC64?"EM_PPC64":val==EM_S390?"EM_S390":val==EM_ARM?\
	"EM_ARM":val==EM_SH?"EM_SH":val==EM_SPARCV9?"EM_SPARCV9":val==EM_IA_64?\
	"EM_IA_64":val==EM_X86_64?"EM_X86_64":val==EM_VAX?"EM_VAX":"unknown")

#define ELF_SYMTABLE_STT_NAME(val)	(val==STT_NOTYPE?"STT_NOTYPE":val==\
	STT_OBJECT?"STT_OBJECT":val==STT_FUNC?"STT_FUNC":val==STT_SECTION?\
	"STT_SECTION":val==STT_FILE?"STT_FILE":val==STT_LOPROC?"STT_LOPROC":val==\
	STT_HIPROC?"STT_HIPROC":val==STB_LOCAL?"STB_LOCAL":val==STB_GLOBAL?\
	"STB_GLOBAL":val==STB_WEAK?"STB_WEAK":val==STB_LOPROC?"STB_LOPROC":val==\
	STB_HIPROC?"STB_HIPROC":"unknown")

struct vm_elf_section {
	vmptr_t offset;
	vmptr_t vaddr;
	size_t size;
	void *map_addr;
};

struct vm_symbol {
	vmptr_t addr;
	char name[NAMES_LENGTH];
};

struct vm_program {
	int fd;
	Elf *elf;
	struct vm_elf_section *sections;
	struct vm_symbol *symbols;
	Elf32_Addr entrypoint;
};

struct vm_program *vm_open_program(const char *filename);
void vm_close_program(struct vm_program *program);

int vm_get_symbol_name(struct vm_program *program, vmptr_t addr, char *name);
int vm_get_symbol_addr(struct vm_program *program, const char *name,
	vmptr_t *addr);
void vm_dump_symbols(struct vm_program *program);

uint32_t vm_read_instruction(struct vm_program *program,vmptr_t vaddr);

#endif
