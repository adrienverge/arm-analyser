/**
 * @file    decompiler.c
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
 * This file describes the main part of the project. It contains the functions
 * to call a new virtual machine, to decode the instructions, look for
 * functions and set up a new rebuilt program.
 */

#include "arm_instructions.h"
#include "arrays.h"
#include "common.h"
#include "decompiler.h"
#include "groups.h"
#include "rebuilt_program.h"

/**
 * Sets the name of a rebuilt_program's function, by looking into the symbol
 * table.
 */
static void decompile_set_function_name(struct vm_program *program,
	struct rebuilt_program *rp, vmptr_t function_id, vmptr_t vaddr)
{
	char function_name[NAMES_LENGTH];

	if (vm_get_symbol_name(program, vaddr, function_name) != 0)
		snprintf(function_name, NAMES_LENGTH - 1, "f%d", (int) function_id);

	rp_function_set_name(&(rp->functions[function_id]), function_name);
}

/**
 * This is the main function of this file: it reads the instructions one by
 * one, looks for "function calls", "returns" and other branches, and add
 * entries in the rebuilt_program's list of branches. Theses entries will later
 * be used to determine addresses of functions.
 */
static void decompile_search_branches(struct vm_program *program,
	struct rebuilt_program *rp, vmptr_t entry_addr)
{
	struct statement statement;
	int i;

	vmptr_t *to_explore;

	uint32_t instr, instr_prev;
	vmptr_t pc;

	LIST_INIT(to_explore);

	LIST_APPEND(to_explore, entry_addr);

	LIST_ITERATOR(to_explore, i) {
		//if (group_is_in_group(rp->explored, to_explore[i]))
		//	continue;
		//group_dump(rp->explored);
		//current = &(to_explore[i]);
		//LIST_APPEND(rp->explored, current->addr);
		//if (current->type == DONTJUMP)
		//	continue;
		//printf("exploring from 0x%08x\n", (int) to_explore[i]);
		instr = 0;
		for (pc = to_explore[i]; ; pc += 4, instr_prev = instr) {
			// Check if this part of the program has already been visited,
			// if not, mark it as visited.
			if (group_is_in_group(rp->explored, pc))
				break;
			group_add_interval(rp->explored, pc, pc + 4);

			statement.type = OTHER;
			statement.br_type = 0;
			statement.to_addr = 0;
			statement.cond = 0;
			statement.staticity = 0;
			statement.value = 0;

			instr = vm_read_instruction(program, pc);
			//printf("0x%08x\t0x%08x\n", (int) pc, instr);
			// Return or nop
			/*if (arm_instr_is_return(instr) || arm_instr_is_nop(instr)) {
				group_add_interval(rp->explored, to_explore[i], pc + 4);
			//	group_dump(rp->explored);
				break;
			// Branch
			} else*/
			//if (arm_instr_is_branch(instr) || arm_instr_is_return(instr)) {
			//if (arm_instr_is_branch(instr)) {
			if (arm_instr_is_branch(pc, instr, program, &(statement.to_addr))) {
				//printf("0x%08x\t0x%08x\n", (int) pc, instr);
				statement.type = BRANCH;
				statement.addr = pc;
				// If the branch address was computed successfully, statement.to
				// is set to the address. Else, it is set to 0.

				// Determine the type of branch: jump, call, return...
				if (arm_instr_branch_is_return(instr)) // || arm_instr_is_nop(instr)) {
					statement.br_type = RETURN; // Return
				else if (arm_instr_branch_is_bl(instr)
					|| instr_prev == 0xe1a0e00f) // mov	lr, pc: this IS like a bl
					statement.br_type = CALL; // Branch with return
				else
					statement.br_type = JUMP; // Definitive branch

				// Determine whether the branch is conditional or not
				if (arm_instr_is_unconditional(instr))
					statement.cond = UNCONDITIONAL;
				else
					statement.cond = CONDITIONAL;

				// Determine if the branch address is computed statically...
				/*if (arm_instr_branch_is_static(instr)) {
					statement.staticity = STATIC;
					statement.to = arm_instr_branch_static_get_addr(pc, instr);
				// ... dynamically but easily...
				} else if (arm_instr_branch_dynamic_try_get_addr(pc, instr,
					program, &(statement.to))) {
					statement.staticity = FALSEDYNAMIC;
				// ... or dynamically
				} else {
					statement.staticity = DYNAMIC;
				}*/
				if (statement.to_addr != 0) {
					//statement.staticity = FALSESTATIC;
					statement.staticity = STATIC;
					LIST_APPEND(to_explore, statement.to_addr);
				} else {
					statement.staticity = DYNAMIC;
				}

				LIST_APPEND(rp->statements, statement);

				//if (pc == 0x138e8)
				//	statement_dump(&statement);
				//if (statement.staticity == STATIC) {
				//LIST_IFNOT_CONTAINS(to_explore, statement.to)
				//	LIST_APPEND(to_explore, statement.to);
				//}
				// Definitive and unconditional branch
				if (statement.br_type == RETURN
					|| (statement.br_type == JUMP && statement.cond == UNCONDITIONAL)) {
					// Reached end of function, get out
					//group_add_interval(rp->explored, to_explore[i], pc + 4);
					break;
				}
			/*} else if (arm_instr_is_nop(instr)) {
				statement.addr = pc;
				statement.type = NOP;
				LIST_APPEND(rp->statements, statement);*/
				/*// Reached end of function, get out
				group_add_interval(rp->explored, to_explore[i], pc);
				break;*/
			//} else if (((instr >> 20) & 0xff) == 0x59 && ((instr >> 16) & 0xf) == 15) { // load / store at immediate offset from PC
			} else if (arm_instr_is_load_store_static(instr)) {
				// TODO: what if negative immediate?
				statement.type = WORD;
				statement.addr = arm_instr_load_store_static_get_addr(instr, pc);
				statement.value = vm_read_instruction(program, statement.addr);
				LIST_IFNOT_CONTAINS(rp->statements, statement)
					LIST_APPEND(rp->statements, statement);
				group_add_interval(rp->explored, statement.addr, statement.addr + 4);
				// This helps merging groups, and so, keeping less groups and running faster
				// Ah bon? Pas pour l'instant, à vérifier plus tard
				//group_add_interval(rp->explored, statement.addr, statement.addr + 4);
			}
		}
	}

	LIST_FREE(to_explore);
}

/**
 * Function to compare two statements by their address.
 * This is used by the sorting algorithm.
 */
static int cmp_statements_addr(const void *a, const void *b)
{
	return ((struct statement *) a)->addr - ((struct statement *) b)->addr;
}

static void decompile_search_functions(struct vm_program *program,
	struct rebuilt_program *rp)
{
	struct statement *s;
	int i, j;

	int f_id, f2_id;
	vmptr_t f_end;

	if (LIST_LENGTH(rp->statements) == 0)
		return;

	// Step 1: sort statements by address
	merge_sort(rp->statements, sizeof(*rp->statements),
		LIST_LENGTH(rp->statements), cmp_statements_addr);

#if defined(DEBUG)
	LIST_ITERATOR(rp->statements, i)
		if (i > 0 && rp->statements[i - 1].addr == rp->statements[i].addr)
			printf("DEBUG: statement at 0x%x exists more than once\n",
				(int) rp->statements[i].addr);
#endif

	// Step 2: add the first function
	s = &(rp->statements[0]);
	f_id = rp_add_function(rp);
	s->to_function = f_id;
	rp->functions[f_id].vaddr_start = s->to_addr;
	decompile_set_function_name(program, rp, f_id, s->to_addr);
	//printf("adding f%d starting at 0x%08x\n", f_id, (int) rp->functions[f_id].vaddr_start);

	// Step 3: read statements for each function
	LIST_ITERATOR(rp->functions, f_id) {
		// Find the first branch of the function: j
		for (j = 0; rp->statements[j].addr < rp->functions[f_id].vaddr_start
			&& j < LIST_LENGTH(rp->statements); j++) ;
		// 1st pass: find the end of the function
		f_end = 0;
		for (i = j; i < LIST_LENGTH(rp->statements); i++) {
			s = &(rp->statements[i]);
			s->to_function = -1;
			/*if (s->addr >= 0x13a10 && s->addr < 0x13a80)
				printf("0x%08x     fend = %x\n", s->addr, f_end);*/
			//if (s->addr == 0x8bb0 || s->addr == 0x8c30 || s->to == 0x8c30 || s->addr == 0x8dcc || s->to == 0x8dcc || s->addr == 0x8e30 || s->to == 0x8e30) {
			/*if (b->addr >= 0x12d00 && s->addr < 0x12fb0) {
				printf("f_end = %x\t", f_end);
				branch_dump(s);
			}//*/
			if (s->type == NOP || s->type == WORD) {
				if (f_end <= s->addr + 4) {
					rp->functions[f_id].vaddr_end = s->addr;
					break;
				}
				continue;
			}
			if (s->br_type == RETURN) {
				rp_function_add_statement(&(rp->functions[f_id]), s);
				if (f_end <= s->addr + 4) {
					// We've found the return point
					//printf("in f%d:\treturn point found at 0x%08x\n", f_id, s->addr);
					rp->functions[f_id].vaddr_end = s->addr + 4;
					break;
				}
			} else if (s->br_type == JUMP && s->cond == UNCONDITIONAL) {
				if (f_end <= s->addr + 4) {
					// We've found the return point
					//printf("in f%d:\treturn point found at 0x%08x\n", f_id, s->addr);
					rp->functions[f_id].vaddr_end = s->addr + 4;
					// And add the called function to the list
					//if (s->staticity == STATIC
					// We have an address, may it be static or dynamic...
					if (s->to_addr != 0
						// ... and make sure we don't loop into the function
						&& (s->to_addr < rp->functions[f_id].vaddr_start
							|| s->to_addr >= s->addr + 4)) {
						f2_id = rp_get_function_by_vaddr(rp, s->to_addr);
						if (f2_id == -1) {
							f2_id = rp_add_function(rp);
							rp->functions[f2_id].vaddr_start = s->to_addr;
							decompile_set_function_name(program, rp, f2_id, s->to_addr);
							//printf("in f%d:\tadding f%d starting at 0x%08x\n", f_id, f2_id, (int) rp->functions[f2_id].vaddr_start);
						}
						s->to_function = f2_id;
					}
					rp_function_add_statement(&(rp->functions[f_id]), s);
					break;
				}
				rp_function_add_statement(&(rp->functions[f_id]), s);
			//} else if (s->br_type == JUMP && s->staticity == STATIC) {
			} else if (s->br_type == JUMP && s->to_addr != 0) {
				// End of the function is AT LEAST beyond this point
				f_end = (f_end > s->to_addr + 4 ? f_end : s->to_addr + 4);
				rp_function_add_statement(&(rp->functions[f_id]), s);
			//} else if (s->br_type == CALL && s->staticity == STATIC) {
			} else if (s->br_type == CALL && s->to_addr != 0) {
				// This is a call to a child function
				f2_id = rp_get_function_by_vaddr(rp, s->to_addr);
				if (f2_id == -1) {
					f2_id = rp_add_function(rp);
					rp->functions[f2_id].vaddr_start = s->to_addr;
					decompile_set_function_name(program, rp, f2_id, s->to_addr);
					//printf("in f%d:\tadding f%d starting at 0x%08x\n", f_id, f2_id, (int) rp->functions[f2_id].vaddr_start);
				}
				s->to_function = f2_id;
				rp_function_add_statement(&(rp->functions[f_id]), s);
			} else {
				rp_function_add_statement(&(rp->functions[f_id]), s);
			}
		}
	}
}

/**
 * Reads each rebuilt function instruction, in order to find ones that are
 * system calls.
 */
static void decompile_search_syscalls(struct vm_program *program,
	struct rebuilt_program *rp)
{
	int f_id;

	uint32_t instr, instr2;
	vmptr_t pc;

	struct statement s;

	s.type = SYSCALL;

	LIST_ITERATOR(rp->functions, f_id) {
		// For each function, read the code and search for system calls
		for (pc = rp->functions[f_id].vaddr_start;
			pc < rp->functions[f_id].vaddr_end; pc += 4) {
			instr = vm_read_instruction(program, pc);
			if (arm_instr_is_software_interrupt(instr)) {
				s.addr = pc;
				// Read the previous instruction to know what it really is
				instr2 = vm_read_instruction(program, pc - 4);
				if ((instr2 & 0xfffff000) != 0xe3a07000)
					instr2 = vm_read_instruction(program, pc - 8);
				if ((instr2 & 0xfffff000) == 0xe3a07000) // mov r7, #val
					s.value = arm_instr_mov_r7_immediate_get_value(instr2);
				else
					s.value = -1;
				rp_function_add_statement(&(rp->functions[f_id]), &s);
			}
		}
		// There is a need to re-sort the statements...
		merge_sort(rp->functions[f_id].statements,
			sizeof(*rp->functions[f_id].statements),
			LIST_LENGTH(rp->functions[f_id].statements), cmp_statements_addr);
	}
}

/**
 * Starts the decompilation of the source binary.
 */
int decompile(struct vm_program *program, struct rebuilt_program *rp)
{
	struct statement s;
	int i, j;

	int contains_stdlib;
	int call_to_main;
	vmptr_t libc_start_main;
	vmptr_t main_function;
	vmptr_t *stdlib_addrs;

	// Decompile from the entry point of the program
	s.addr = 0;
	s.type = BRANCH;
	s.to_addr = program->entrypoint;
	s.br_type = JUMP;
	LIST_APPEND(rp->statements, s);
	decompile_search_branches(program, rp, s.to_addr);

	// If the binary was compiled with standard library, main() is not called
	// directly. We need to the find its address, which is stored in the 2nd
	// word at the end of _start() function.
	// First, detect if the program was compiled with the standard library.
	contains_stdlib = 0;
	j = 0;
	LIST_ITERATOR(rp->statements, i) {
		if (rp->statements[i].type == BRANCH) {
			// Detect the typical glibc first function, _start
			if (j == 1 && rp->statements[i].br_type == CALL
				&& rp->statements[i].cond == UNCONDITIONAL
				&& rp->statements[i].addr == rp->statements[0].to_addr + 0x28) {
				libc_start_main = rp->statements[i].to_addr;
				main_function = vm_read_instruction(program, 0x8184);
				contains_stdlib = 1;
			//} else if (j > 1) {
			//	break;
			} else if (rp->statements[i].br_type == CALL
				&& rp->statements[i].cond == UNCONDITIONAL
				&& rp->statements[i].addr == libc_start_main + 0x1a8) {
					call_to_main = i;
					break;
			}
			j++;
		}
	}

	// Step 1/2 of marking stdlib functions as "stdlib" functions
	if (contains_stdlib) {
		LIST_INIT(stdlib_addrs);

		// Add all current functions to the stdlib functions list
		LIST_ITERATOR(rp->statements, i)
			if (rp->statements[i].to_addr != 0)
				LIST_APPEND(stdlib_addrs, rp->statements[i].to_addr);
		
		// And start exploring from main()
		rp->statements[call_to_main].to_addr = main_function;
		decompile_search_branches(program, rp, main_function);
	}

	// Find functions addresses and stop points using all the branches we have
	decompile_search_functions(program, rp);

	// Step 2/2 of marking stdlib functions as "stdlib" functions
	if (contains_stdlib) {
		LIST_ITERATOR(rp->functions, i) {
			LIST_IF_CONTAINS(stdlib_addrs, rp->functions[i].vaddr_start)
				rp->functions[i].from_stdlib = 1;
		}

		LIST_FREE(stdlib_addrs);
	}

	decompile_search_syscalls(program, rp);

	//rp_check_overlapping_functions(rp);
	rp_fix_overlapping_functions(rp);

	return 0;
}
