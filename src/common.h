/**
 * @file    common.h
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
 * This file contains general-purpose macros, defined to be used in any other
 * file. It mostly contains macros to operate on lists.
 */

#if !defined(COMMON_H)
#define COMMON_H

#include <stdint.h>

#define NAMES_LENGTH	64
#define vmptr_t	uint32_t

#define _CONCAT(a, b) a ## b
#define CONCAT(a, b) _CONCAT(a, b)
#define UNIQUENAME(prefix) CONCAT(prefix, __LINE__)

#define FATAL_ERROR(msg, ...)	\
	do {\
		printf("error at %s:%d: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
		exit(1);\
	} while (0)

/**
 * How lists are handled:
 * For a list L of 3 elements, here is the memory allocation:
 *
 *        malloc returns this address
 *       /      LIST_INIT returns this address
 *      /______/_______________________
 *     [       ¦       ¦       ¦       ]
 *     [   3   ¦  val  ¦  val  ¦  val  ]
 *     [_______¦_______¦_______¦_______]
 *      \       \       \
 *       \       \       pointer to L[1]
 *        \       pointer to *L and L[0]
 *         contains L's length
 */
#define LIST_INIT(_list)	\
	do {\
		_list = malloc(sizeof(*(_list)));\
		if (_list == NULL)\
			FATAL_ERROR("malloc");\
		*((int *) (_list)) = 0;\
		_list += 1;\
	} while (0)

#define LIST_FREE(_list)	\
	do {\
		free((_list) - 1);\
		_list = NULL;\
	} while (0)

#define LIST_LENGTH(_list)	\
	(*((int *) ((_list) - 1)))

#define LIST_APPEND(_list, _element)	\
	do {\
		_list = (typeof(_list)) realloc((_list) - 1, (LIST_LENGTH(_list) + 2) * sizeof(*(_list))) + 1;\
		if (_list == NULL)\
			FATAL_ERROR("realloc");\
		(_list)[LIST_LENGTH(_list)] = _element;\
		LIST_LENGTH(_list)++;\
	} while (0)

#define LIST_APPEND_SORTED(_list, _element)	\
	do {\
		int _i;\
		LIST_ITERATOR(_list, _i)\
			if (((_list)[_i]) >= (_element)) {\
				break;\
			}\
		if (((_list)[_i]) != (_element))\
			LIST_ADD(_list, _element, _i);	\
	} while (0)

#define LIST_ADD(_list, _element, _offset)	\
	do {\
		_list = (typeof(_list)) realloc((_list) - 1, (LIST_LENGTH(_list) + 2) * sizeof(*(_list))) + 1;\
		if (_list == NULL)\
			FATAL_ERROR("realloc");\
		memmove(&((_list)[(_offset) + 1]), &((_list)[(_offset)]), (LIST_LENGTH(_list) - (_offset)) * sizeof(*(_list)));\
		(_list)[_offset] = _element;\
		LIST_LENGTH(_list)++;\
	} while (0)

#define LIST_REMOVE(_list, _offset)	\
	do {\
		memmove(&((_list)[(_offset)]), &((_list)[(_offset) + 1]), (LIST_LENGTH(_list) - 1 - (_offset)) * sizeof(*(_list)));\
		_list = (typeof(_list)) realloc((_list) - 1, LIST_LENGTH(_list) * sizeof(*(_list))) + 1;\
		if (_list == NULL)\
			FATAL_ERROR("realloc");\
		LIST_LENGTH(_list)--;\
	} while (0)

#define LIST_ITERATOR(_list, _i)	\
	for (_i = 0; _i < LIST_LENGTH(_list); _i++)

#define LIST_ITERATOR_REVERSE(_list, _i)	\
	for (_i = LIST_LENGTH(_list) - 1; _i >= 0; _i--)

#define LIST_CONTAINS(_list, _element, _ret)	\
	do {\
		_ret = 0;\
		int _i;\
		LIST_ITERATOR(_list, _i)\
			if (memcmp(&((_list)[_i]), &(_element), sizeof(*(_list))) == 0) {\
				_ret = 1;\
				break;\
			}\
	} while (0)

#define LIST_IF_CONTAINS(_list, _element)	\
	int UNIQUENAME(_ret);\
	LIST_CONTAINS(_list, _element, UNIQUENAME(_ret));\
	if (UNIQUENAME(_ret))

#define LIST_IFNOT_CONTAINS(_list, _element)	\
	int UNIQUENAME(_ret);\
	LIST_CONTAINS(_list, _element, UNIQUENAME(_ret));\
	if (!UNIQUENAME(_ret))

#define LIST_DUMP(_list)	\
	do {\
		int _i;\
		printf("[");\
		LIST_ITERATOR(_list, _i)\
			printf("0x%x, ", (_list)[_i]);\
		printf("]\n");\
	} while (0)
#endif
