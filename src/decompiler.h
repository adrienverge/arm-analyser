/**
 * @file    decompiler.h
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

#if !defined(DECOMPILER_H)
#define DECOMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "common.h"
#include "rebuilt_program.h"
#include "vm.h"

enum { STDLIB_SHOW, STDLIB_HIDE };

int decompile(struct vm_program *program, struct rebuilt_program *rp);

#endif
