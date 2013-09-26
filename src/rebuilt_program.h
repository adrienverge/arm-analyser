/**
 * @file    rebuilt_program.h
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
 * This file contains structures to describe a newly reconstructed program and
 * its functions. It also gives methods to access these structures.
 */

#if !defined(REBUILT_PROGRAM_H)
#define REBUILT_PROGRAM_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "groups.h"
#include "vm.h"

#include "rebuilt_program.h"

struct statement {
	vmptr_t addr;
	enum { BRANCH, NOP, WORD, SYSCALL, OTHER } type;
	enum { UNCONDITIONAL, CONDITIONAL } cond;
	// Specific to branches
	vmptr_t to_addr;
	int to_function;
	enum { JUMP, CALL, RETURN } br_type;
	enum { STATIC, DYNAMIC, FALSEDYNAMIC } staticity;
	// Specific to words and syscalls
	uint32_t value;
};

#define STATEMENT_TYPE(val)	\
	(val==BRANCH?"BRANCH":val==NOP?" NOP  ":val==WORD?" WORD ":val==SYSCALL?\
	"SYSCALL":"OTHER ")
#define STATEMENT_COND(val)	\
	(val==CONDITIONAL?"cond.":"     ")
#define STATEMENT_BR_TYPE(val)	\
	(val==JUMP?" JUMP ":val==CALL?" CALL ":"RETURN")
#define STATEMENT_STATICITY(val)	\
	(val==STATIC?"static addr":val==DYNAMIC?"dynam. addr":"false dyn.")

void statement_dump(struct statement* s);

struct rebuilt_function {
	int id;
	vmptr_t vaddr_start;
	vmptr_t vaddr_end;
	char name[NAMES_LENGTH];
	struct statement *statements;
	int from_stdlib;
};

struct rebuilt_program {
	struct statement *statements;
	struct group *explored;
	struct rebuilt_function *functions;
	int entry_function;
};

struct rebuilt_program *rp_new();
void rp_free(struct rebuilt_program *rp);

int rp_add_function(struct rebuilt_program *rp);
int rp_get_function_by_vaddr(struct rebuilt_program *rp, vmptr_t vaddr);

void rp_function_add_statement(struct rebuilt_function *f,
	const struct statement *s);
void rp_function_set_name(struct rebuilt_function *f, const char *name);

int rp_check_overlapping_functions(struct rebuilt_program *rp);
void rp_fix_overlapping_functions(struct rebuilt_program *rp);

void rp_dump_functions(struct rebuilt_program *rp, int hide_stdlib,
	int compacity);
void rp_dump_function_by_addr(struct rebuilt_program *rp, vmptr_t addr,
	int compacity);

struct cfg_node {
	vmptr_t addr;
	enum { NODE, FUNCTION, SYSFUNCTION } type;
	struct statement *stm;
	int child1, child2;
	enum { NO, YES } show;
};

void rp_dump_callgraph(struct rebuilt_program *rp, int hide_stdlib);
void rp_dump_cfg_for_function(struct rebuilt_program *rp, vmptr_t addr);

#endif
