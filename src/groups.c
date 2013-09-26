/**
 * @file    groups.c
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
 * This file gives structures and function to manipulate intervals and groups
 * of intervals. You can add intervals, like this:
 *   [0-10] [20-30] [100-999]
 * When you add an interval (for instance [20-30], it is automatically placed
 * at the right place, by increasing order. It is also automatically merged
 * with other intervals if needed. For example, if you add [9-20] to the
 * previous example, you get the following group of intervals:
 *   [0-30] [100-999]
 * When a group of intervals is set up, you can test if a value is within one
 * of these intervals by calling group_in_group().
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "groups.h"

struct group *group_init()
{
	struct group *group;

	group = malloc(sizeof(struct group));
	if (group == NULL)
		FATAL_ERROR("malloc");
	memset(group, 0, sizeof(struct group));

	LIST_INIT(group->intervals);

	return group;
}

void group_free(struct group *group)
{
	LIST_FREE(group->intervals);

	free(group);
}

void group_add_interval(struct group* group, vmptr_t start, vmptr_t end)
{
	int i, j;
	struct interval new_interval;

	int nobody = 1;
	int first_one = 0;
	int last_one = 1;

	if (start >= end)
		FATAL_ERROR("start >= end");

	//printf("group_add_interval(0x%x, 0x%x)\n", (int) start, (int) end);

	// Loop until we are at left from the correct interval
	LIST_ITERATOR(group->intervals, i) {
		nobody = 0;
		if (start <= group->intervals[i].end) {
			last_one = 0;
			break;
		}
	}

	if (nobody) { // First interval, there is no else
		new_interval.start = start;
		new_interval.end = end;
		LIST_APPEND(group->intervals, new_interval);
		return;
	}

	if (end < group->intervals[0].start)
		first_one = 1;

	//LIST_ITERATOR(group->intervals, j) {
	//	if (end < group->intervals[j].start) {
	//		j--;
	//		break;
	//	}
	LIST_ITERATOR_REVERSE(group->intervals, j) {
		if (end >= group->intervals[j].start)
			break;
	}
	//printf("i = %d  j = %d  first_one = %d  last_one = %d\n", i, j, first_one, last_one);
	/*  ... [i-1]   [_i_]   [i+1] ... [j-1]   [_j_]   [j+1] ...
	             ¦      ¦                     ¦______¦
                  ^^^^^^ 
                   [_____________new____________]               */

	// If we have zero intersection, create a new one
	if (first_one || last_one || i == j + 1) {
		new_interval.start = start;
		new_interval.end = end;
		LIST_ADD(group->intervals, new_interval, i);
		return;
	}

	// We can merge i and j, if they are different
	// and, by the way, delete ones between i and j
	if (i < j) {
		group->intervals[i].end = group->intervals[j].end;
		while (i < j) {
			LIST_REMOVE(group->intervals, i + 1);
			j--;
		}
	}
	/*  ... [i-1]    [___i___]    [_i+1_] ...
	             ¦   ________¦___¦
                  ^^^^^^^^
                   [____new____]               */

	// In this last case, all we have to do is to enlarge i
	if (start < group->intervals[i].start)
		group->intervals[i].start = start;
	if (end > group->intervals[i].end)
		group->intervals[i].end = end;
}

int group_is_in_group(struct group* group, vmptr_t item)
{
	int i;

	LIST_ITERATOR(group->intervals, i) {
		if (item >= group->intervals[i].start && item < group->intervals[i].end)
			return 1;
		else if (item < group->intervals[i].start)
			return 0;
	}
	return 0;
}

void group_dump(struct group* group)
{
	int i;
	struct interval *interval;

	LIST_ITERATOR(group->intervals, i) {
		interval = &(group->intervals[i]);
		printf("[0x%x-0x%x] ", (int) interval->start, (int) interval->end);
	}
	printf("\n");
}
