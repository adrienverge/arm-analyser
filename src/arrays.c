/**
 * @file    arrays.c
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

#include <stdlib.h>
#include <string.h>

#include "rebuilt_program.h"

/**
 * Merge-sort algorithm.
 *
 * cf. http://en.wikipedia.org/wiki/Merge_sort
 */
static void merge(void *array, size_t element_size,
	int (*cmp_fn)(const void *, const void *), // the function used to compare elements
	const int start1, const int end1, const int end2)
{
	void *temp_array;
	int start2 = end1 + 1;
	int pos1 = start1;
	int pos2 = start2;
	int i;

	void *new, *candidate1, *candidate2;

	// Let's copy all elements in the beggining
	// of the array to a temporary location
	temp_array = malloc((end1 - start1 + 1) * element_size);
	memcpy(temp_array, array + start1*element_size,
		(end1 - start1 + 1)*element_size);

	for (i = start1; i <= end2; i++) {
		// We are going to copy either candidate 1 or 2, into new element
		new = array + i*element_size;
		candidate1 = temp_array + (pos1 - start1)*element_size;
		candidate2 = array + pos2*element_size;
		// If all elements from the first array have been used already
		if (pos1 == start2) {
			break;
		// If all elements from the second array have been used already,
		// or candidate 2 is greater than candidate 1
		} else if (pos2 == end2 + 1 || cmp_fn(candidate1, candidate2) < 0) {
			// Let's the new element be candidate 1
			memcpy(new, candidate1, element_size);
			pos1++;
		// If candidate 1 is greater than candidate 2
		} else {
			// Let's the new element be candidate 2
			memcpy(new, candidate2, element_size);
			pos2++;
		}
	}

	free(temp_array);
}

static void merge_sort_bis(void *array, size_t element_size,
	int (*cmp_fn)(const void *, const void *), const int start, const int end)
{
	int middle;

	if (start != end) {
		middle = (end + start) / 2;
		merge_sort_bis(array, element_size, cmp_fn, start, middle);
		merge_sort_bis(array, element_size, cmp_fn, middle + 1, end);
		merge(array, element_size, cmp_fn, start, middle, end);
	}
}

void merge_sort(void *array, size_t element_size, int length,
	int (*cmp_fn)(const void *, const void *))
{
	if (length > 0)
		merge_sort_bis(array, element_size, cmp_fn, 0, length - 1);
}

/*int main()
{
	struct statement tableau[] = {
		{80, 1080, JUMP, DYNAMIC, CONDITIONAL},
		{10, 1010, UNKNOWN, STATIC, UNCONDITIONAL},
		{102, 1102, CALL, DYNAMIC, CONDITIONAL},
		{106, 1106, RETURN, DYNAMIC, CONDITIONAL},
		{50, 1050, JUMP, STATIC, UNCONDITIONAL},
		{100, 1100, CALL, STATIC, CONDITIONAL},
		{60, 1060, JUMP, STATIC, CONDITIONAL},
		{40, 1040, UNKNOWN, DYNAMIC, CONDITIONAL},
		{104, 1104, RETURN, STATIC, CONDITIONAL},
		{101, 1101, CALL, DYNAMIC, UNCONDITIONAL},
		{70, 1070, JUMP, DYNAMIC, UNCONDITIONAL},
		{90, 1090, CALL, STATIC, UNCONDITIONAL},
		{30, 1030, UNKNOWN, DYNAMIC, UNCONDITIONAL},
		{103, 1103, RETURN, STATIC, UNCONDITIONAL},
		{20, 1020, UNKNOWN, STATIC, CONDITIONAL},
		{105, 1105, RETURN, DYNAMIC, UNCONDITIONAL},
	};

	int length = sizeof(tableau) / sizeof(struct statement);
	int i;

	merge_sort(tableau, length);

	for (i = 0; i < length; i++) {
		printf("%4d  %4d  %d  %d  %d\n", tableau[i].addr, tableau[i].to,
			tableau[i].type, tableau[i].staticity, tableau[i].cond);
	}

	return 0;
}*/
