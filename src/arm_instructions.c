/**
 * @file    arm_instructions.c
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

#include "arm_instructions.h"
#include "common.h"

/**
 * Tests if an ARM instruction is unconditional or not.
 *
 * conditional bits : 28-31
 * always jump: bits == 1110
 */
int arm_instr_is_unconditional(uint32_t instr)
{
	return ((instr >> 28) & 15) >= 0xe;
}

/**
 * TODO: to be continued...
 */
int arm_instr_is_direct_affectation(uint32_t instr,
	struct vm_program *program, char reg, uint32_t *value)
{
	char threebits = (instr >> 25) & 7;
	char opcode = (instr >> 21) & 0xf;
	//char rn = (instr >> 16) & 0xf;
	char rd = (instr >> 12) & 0xf;
	uint32_t operand = instr & 0xfff;

	// TODO: other types of affectation
	if (0) {

	// Data processing immediate
	} else if (threebits == 1) {
		if (rd == reg) {
			// e3e03a0f        mvn     r3, #61440      ; 0xf000
			// threebits == 1
			// opcode == 15
			// rn == 0, rd == 3
			// operand == a0f == 1010 0000 1111
			switch (opcode) {
				case 0: break; // 
				case 1: break; // 
				case 2: break; // Substract
				case 3: break; // 
				case 4: break; // 
				case 5: break; // 
				case 6: break; // 
				case 7: break; // 
				case 12: break; // 
				case 13: break; // 
				case 14: break; // 
				case 15: *value = operand; break; // 
			}
			return 1;
		}
	}

	return 0;
}

/**
 * Search for instructions that affect PC (register 15), which are branching
 * instructions.
 * c.f. ARM Architecture Reference Manual, figure A3-1.
 *
 * When the instruction is a branch, returns 1.
 * Also, tries to compute the address the branch jumps to, and put the result
 * in branch_to. If it fails, branch_to is set to 0.
 */
int arm_instr_is_branch(vmptr_t pc, uint32_t instr,
	struct vm_program *program, vmptr_t *branch_to)
{
	char threebits = (instr >> 25) & 7;
	char opcode = (instr >> 21) & 0xf;
	char rn = (instr >> 16) & 0xf;
	char rd = (instr >> 12) & 0xf;
	char l = (instr >> 20) & 1;

	// Data processing immediate/register shift
	if (threebits == 0) {
		// This really affects PC
		if ((opcode >> 2) != 2 && rd == 15) {
			*branch_to = 0;
			if (rn == 3) { // temporaire, juste pour débuter
				/*vmptr_t pc2;
				uint32_t instr2;
				for (pc2 = pc - 1; ; pc--) {
					instr2 = vm_read_instruction(program, pc2);
					//if ()
				}*/
			}
			// Try to compute the address
			switch (opcode) {
				case 0: break; // 
				case 1: break; // 
				case 2: break; // Substract
				case 3: break; // 
				case 4: break; // 
				case 5: break; // 
				case 6: break; // 
				case 7: break; // 
				case 12: break; // 
				case 13: break; // 
				case 14: break; // 
				case 15: break; // 
			}
			return 1;
		} else if (opcode == 9 && ((instr >> 6) & 3) == 0 && ((instr >> 4) & 3) > 0) {
			// Dynamic branch
			*branch_to = 0;
			return 1;
		}

		/*// BX, BXJ, BLX(2): Rm
		} else if (((instr >> 20) & 0xff) == 0x12
			&& ((instr >> 6) & 3) == 0 && ((instr >> 4) & 3) > 0) {
			FATAL_ERROR("dynamic branch instruction at 0x%08x", (int) vaddr);
		}*/


	// Data processing immediate
	} else if (threebits == 1) {
		// This really affects PC
		if ((opcode >> 2) != 2 && rd == 15) {
			*branch_to = 0;
			return 1;
		}

	// Load/store immediate offset
	} else if (threebits == 2) {
		if (l /* read */ && rd == 15) {
			*branch_to = 0;
			return 1;
		}

	// Load/store register offset
	} else if (threebits == 3) {
		if (l /* read */ && rd == 15) {
			*branch_to = 0;
			return 1;
		}

	// Load/store multiple
	} else if (threebits == 4) {
		if (l /* read */ && ((instr >> 15) & 1)) {
			*branch_to = 0;
			return 1;
		}

	/* Branch and branch with link
	 * Can be either B, BL, or BLX(1)
	 * B:         ----101- -------- -------- -------- (static)
	 * BL:        ----101- -------- -------- -------- (static)   saves return
	 * BLX(1):    1111101- -------- -------- -------- (static)   saves return */
	} else if (threebits == 5) {
		// BLX(1) is a BL to a Thumb instruction,
		// which does not exist in ARMv5.
		if (((instr >> 28) & 0xf) == 0xf)
			FATAL_ERROR("BLX(1) instruction");

		uint32_t immediate; // 24bit immediate
		// Sign-extending from 24 to 30 bit
		if (instr & 0x800000) // negative
			immediate = 0xfe000000 | ((instr & 0x7fffff) << 2);
		else // positive
			immediate = (instr & 0xffffff) << 2;
		*branch_to = (uint32_t) pc + 8 + immediate;
		return 1;
	}

	// This was not a branch instruction
	return 0;
}

/**
 * Knowing that the given ARM instruction is a branch, tests if the jump
 * address is static or dynamic.
 */
int arm_instr_branch_is_static(uint32_t instr)
{
	return ((instr >> 25) & 7) == 5;
}

/**
 * Knowing that the given ARM instruction is a branch, tests if it is a
 * "branch and link" instruction, e.g. if it stores a return address.
 */
int arm_instr_branch_is_bl(uint32_t instr)
{
	if (((instr >> 25) & 7) == 5) {
		return (instr >> 24) & 1;
	} else if (((instr >> 20) & 0xff) == 0x12
		&& ((instr >> 6) & 3) == 0 && ((instr >> 4) & 3) > 0) {
		return ((instr >> 4) & 3) == 3;
	}
	//FATAL_ERROR("not a branch instruction");
	return 0;
}

/**
 * Tests if a given ARM instruction is a return or not.
 */
int arm_instr_branch_is_return(uint32_t instr)
{
	if (instr == 0xe12fff1e || instr == 0xe8bd8800)
		return 1;
	return 0;
}

/**
 * Tests if a given ARM instruction is a NOP or not.
 */
int arm_instr_is_nop(uint32_t instr)
{
	return instr == 0xe1a00000;
}

/**
 * Tests if a given ARM instruction is a software interrupt.
 */
int arm_instr_is_software_interrupt(uint32_t instr)
{
	return ((instr >> 24) & 0xf) == 0xf;
}

/**
 * For an insruction of type:      mov r7, #imm
 * returns the immediate value (#imm)
 */
uint32_t arm_instr_mov_r7_immediate_get_value(uint32_t instr)
{
	uint8_t rot, val;
	if ((instr & 0xfffff000) != 0xe3a07000) // mov r7, #val
		return -1;
	rot = ((instr & 0xf00) >> 8) * 2;
	val = instr & 0xff;
	return (val << (32 - rot)) | (val >> rot);
}

/**
 * Tests if a given ARM instruction is a load or a store at immediate offset
 * from PC, which means it is computable.
 */
int arm_instr_is_load_store_static(uint32_t instr)
{
	return ((instr >> 20) & 0xff) == 0x59 // Load or store
		&& ((instr >> 16) & 0xf) == 15; // At an address computed from PC
}

/**
 * When the instruction is a load or a store at immediate offset from PC,
 * computes the destination address.
 */
vmptr_t arm_instr_load_store_static_get_addr(uint32_t instr, vmptr_t pc)
{
	// TODO: What if negative offset?
	return pc + (instr & 0xfff) + 8;
}
