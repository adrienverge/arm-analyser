/**
 * @file    groups.h
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

#if !defined(GROUPS_H)
#define GROUPS_H

#include <unistd.h>

#include "common.h"

struct interval {
	vmptr_t start;
	vmptr_t end;
};

struct group {
	struct interval *intervals;
};

struct group *group_init();
void group_free(struct group *group);
void group_dump(struct group *group);

void group_add_interval(struct group *group, vmptr_t start, vmptr_t end);

int group_is_in_group(struct group *group, vmptr_t item);

#endif
