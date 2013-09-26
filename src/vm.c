/**
 * @file    vm.c
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

#include "common.h"
#include "vm.h"

static int vm_check_elf32bitarm(struct vm_program *program);
static int vm_load_sections_elf32bitarm(struct vm_program *program);

static int vm_load_section(struct vm_program *program, GElf_Shdr *shdr,
	Elf_Scn *scn);
static int vm_free_sections(struct vm_program *program);

static void vm_set_symbol_name(struct vm_program *program, vmptr_t addr,
	const char *name);

/**
 * Opens a binary file and creates a new vm_program corresponding to it.
 */
struct vm_program *vm_open_program(const char *filename)
{
	struct vm_program *program;

	// Initialize data structure
	program = malloc(sizeof(struct vm_program));
	if (program == NULL)
		FATAL_ERROR("malloc");
	memset(program, 0, sizeof(struct vm_program));

	LIST_INIT(program->sections);

	LIST_INIT(program->symbols);

	// Open file and make checks
	if (elf_version(EV_CURRENT) == EV_NONE)
		FATAL_ERROR("libelf initialization");

	program->fd = open(filename, O_RDONLY, 0);
	if (program->fd < 0)
		FATAL_ERROR("open");

	program->elf = elf_begin(program->fd, ELF_C_READ, NULL);
	if (program->elf == NULL)
		FATAL_ERROR("elf_begin");

	// Check if executable is of the right type
	vm_check_elf32bitarm(program);

	// Load exetutable sections into memory
	vm_load_sections_elf32bitarm(program);

	elf_end(program->elf);

	close(program->fd);

	return program;
}

/**
 * Closes and frees an already allocated vm_program.
 */
void vm_close_program(struct vm_program *program)
{
	LIST_FREE(program->symbols);

	vm_free_sections(program);
	LIST_FREE(program->sections);

	free(program);
}

/**
 * Checks if the vm_program is of the right type, e.g. ELF and ARM 32-bit.
 */
static int vm_check_elf32bitarm(struct vm_program *program)
{
	char *e_ident;
	Elf32_Ehdr *header;

	if (elf_kind(program->elf) != ELF_K_ELF)
		FATAL_ERROR("not an ELF object");

	e_ident = elf_getident(program->elf, NULL);
	if (e_ident[EI_CLASS] != ELFCLASS32)
		FATAL_ERROR("not 32-bit architecture");

	header = elf32_getehdr(program->elf);
	if (header->e_type != ET_EXEC)
		FATAL_ERROR("not an executable file");
	if (header->e_machine != EM_ARM)
		FATAL_ERROR("not ARM architecture");

	program->entrypoint = header->e_entry;

	return 0;
}

/**
 * Reads the sections table, and loads the executable sections into memory.
 * Also, if it finds a section with symbols (like the symbols table),
 * it retrieves names and stores them for later.
 */
static int vm_load_sections_elf32bitarm(struct vm_program *program)
{
	Elf_Scn *scn;
	GElf_Shdr shdr;
	Elf_Data *edata;
	GElf_Sym sym;
	int symbols_num, i;

	// Read ELF, section by section
	program->elf = elf_begin(program->fd, ELF_C_READ, NULL);
	scn = NULL;
	while ((scn = elf_nextscn(program->elf, scn)) != NULL) {
		gelf_getshdr(scn, &shdr);

		// If this section should be present in memory during program,
		// execution, load it
		if ((shdr.sh_flags & SHF_ALLOC) && (shdr.sh_type == SHT_PROGBITS)) {
			//printf("i'm going to load section!\n");
			vm_load_section(program, &shdr, scn);
		}

		// If this section contains symbols, retrieve them
		if (shdr.sh_type == SHT_SYMTAB) {
			edata = elf_getdata(scn, NULL);
			symbols_num = shdr.sh_size / shdr.sh_entsize;
			for (i = 0; i < symbols_num; i++) {
				if (gelf_getsym(edata, i, &sym) == 0)
					FATAL_ERROR("gelf_getsym");
				if (sym.st_name != 0)
					vm_set_symbol_name(program, sym.st_value,
						elf_strptr(program->elf, shdr.sh_link, sym.st_name));
					//printf("%08x\t%d\t%s\n", (unsigned int) sym.st_value, (int) sym.st_size, elf_strptr(program->elf, shdr.sh_link, sym.st_name));
			}
		}
	}

	//vm_dump_symbols(program);

	return 0;
}

/**
 * Loads a given section from the source program by allocating memory and
 * copying data.
 */
static int vm_load_section(struct vm_program *program, GElf_Shdr *shdr,
	Elf_Scn *scn)
{
	struct vm_elf_section section;
	Elf_Data *edata;
	vmptr_t offset;

	// Add new element to sections list
	section.offset = shdr->sh_offset;
	section.vaddr = shdr->sh_addr;
	section.size = shdr->sh_size;
	section.map_addr = malloc(shdr->sh_size);
	if (section.map_addr == NULL)
		FATAL_ERROR("malloc");
	LIST_APPEND(program->sections, section);

	// Fill with data
	edata = NULL;
	offset = 0;
	while ((edata = elf_getdata(scn, edata)) != NULL) {
		//printf("elf_getdata()\n");
		memcpy(section.map_addr + offset, edata->d_buf, edata->d_size);
		offset += edata->d_size;
	}

	return 0;
}

/**
 * Frees sections allocated by vm_load_section.
 */
static int vm_free_sections(struct vm_program *program)
{
	int i;

	LIST_ITERATOR(program->sections, i) {
		if (program->sections[i].map_addr == NULL)
			printf("== ERROR: program->sections[%d].map_addr == NULL\n", i);
		else
			free(program->sections[i].map_addr);
	}

	return 0;
}

/**
 * Reads an instruction (or any other data) at a given address in the studied
 * program.
 */
uint32_t vm_read_instruction(struct vm_program *program, vmptr_t vaddr)
{
	int i;
	void *addr;

	LIST_ITERATOR(program->sections, i) {
		if (vaddr >= program->sections[i].vaddr &&
			vaddr < program->sections[i].vaddr + program->sections[i].size) {
			//printf("in section %d (%p - %p)\n", length,
			//	program->sections[length].vaddr,
			//	program->sections[length].vaddr +
			//	program->sections[length].size);
			addr = program->sections[i].map_addr + vaddr
				- program->sections[i].vaddr;
			return *((uint32_t *) addr);
		}
	}

	FATAL_ERROR("read at invalid address 0x%08x", (int) vaddr);
	return 0;
}

/**
 * Used to manage functions names.
 * Adds a new entry in the list of names, or replace the entry if there is
 * already one with the same address.
 */
static void vm_set_symbol_name(struct vm_program *program, vmptr_t addr,
	const char *name)
{
	int i;
	struct vm_symbol new_symbol;

	// Count list elements
	LIST_ITERATOR(program->symbols, i) {
		// If entry already exist, just replace the name
		if (program->symbols[i].addr == addr) {
			strncpy(program->symbols[i].name, name, NAMES_LENGTH - 1);
			return;
		}
	}

	// Entry does not exist yet, create a new one
	new_symbol.addr = addr;
	strncpy(new_symbol.name, name, NAMES_LENGTH - 1);
	new_symbol.name[NAMES_LENGTH - 1] = 0;
	LIST_APPEND(program->symbols, new_symbol);
}

/**
 * Used to manage functions names.
 * Retrieves the symbol name associated with a given address, if it exists.
 */
int vm_get_symbol_name(struct vm_program *program, vmptr_t addr, char *name)
{
	int i;

	LIST_ITERATOR(program->symbols, i) {
		if (program->symbols[i].addr == addr) {
			strncpy(name, program->symbols[i].name, NAMES_LENGTH - 1);
			return 0;
		}
	}

	return 1;
}

/**
 * Used to manage functions names.
 * Retrieves the symbol address associated with a given name, if it exists.
 */
int vm_get_symbol_addr(struct vm_program *program, const char *name,
	vmptr_t *addr)
{
	int i;

	LIST_ITERATOR(program->symbols, i) {
		if (strcmp(program->symbols[i].name, name) == 0) {
			*addr = program->symbols[i].addr;
			return 0;
		}
	}

	return 1;
}

/**
 * Displays the whole list of symbols names.
 */
void vm_dump_symbols(struct vm_program *program)
{
	int i;

	LIST_ITERATOR(program->symbols, i)
		printf("symbol:\t0x%x\t%s\n", (int) program->symbols[i].addr,
			program->symbols[i].name);
}
