/**
 * @file    arrays.h
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
 * This file provides a function to sort an array, using its 'addr' attribute as
 * the key. We use the merge-sort algorithm, which is a stable sort with
 * performance in O(n*log(n)).
 */

#if !defined(ARRAYS_H)
#define ARRAYS_H

#include "rebuilt_program.h"

void merge_sort(void *array, size_t element_size, int length,
	int (*cmp_fn)(const void *, const void *));

#endif
