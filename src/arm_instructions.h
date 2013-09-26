/**
 * @file    arm_instructions.h
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
 * This file contains functions that help recognize ARM instructions, and
 * compute addresses when it's possible.
 */

#if !defined(ARM_INSTRUCTIONS_H)
#define ARM_INSTRUCTIONS_H

#include <stdint.h>

#include "vm.h"

int arm_instr_is_unconditional(uint32_t instr);

int arm_instr_is_branch(vmptr_t pc, uint32_t instr,
	struct vm_program *program, vmptr_t *branch_to);

int arm_instr_branch_is_static(uint32_t instr);
int arm_instr_branch_is_bl(uint32_t instr);
int arm_instr_branch_is_return(uint32_t instr);
uint32_t arm_instr_branch_static_get_addr(vmptr_t vaddr, uint32_t instr);
int arm_instr_is_nop(uint32_t instr);

int arm_instr_is_software_interrupt(uint32_t instr);
uint32_t arm_instr_mov_r7_immediate_get_value(uint32_t instr);

int arm_instr_is_load_store_static(uint32_t instr);
uint32_t arm_instr_load_store_static_get_addr(uint32_t instr, vmptr_t pc);

#endif
