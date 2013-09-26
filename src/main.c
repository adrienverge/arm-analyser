/**
 * @file    main.c
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
 * This file contain the entry point of the project. It parses the arguments
 * and interpretes what to do.
 */

#include <ctype.h>
#include <errno.h>

#include "decompiler.h"
#include "common.h"
#include "rebuilt_program.h"
#include "vm.h"

#define USAGE	\
	"Usage: %s action [options] program\n"\
	"actions:\n"\
	"  help      display this help\n"\
	"  fn        dump functions\n"\
	"  cg        generate callgraph\n"\
	"  cfg       generate CFG (option -f needed)\n"\
	"options:\n"\
	"  -s        show standard C library\n"\
	"  -f FN     limit action to function FN (name or address)\n"\
	"options for action fn:\n"\
	"  -c        compact dump (names, addresses and childs)\n"\
	"  -cc       very compact dump (only addresses)\n"
#define usage()	\
	printf(USAGE, argv[0]);

enum {
	ACTION_HELP,
	ACTION_DUMP_FUNCTIONS,
	ACTION_MAKE_CALLGRAPH,
	ACTION_MAKE_CFG
};

/**
 * This is the entry point of the program.
 */
int main(int argc, char **argv)
{
	int ret = 0;

	int c;
	int action = ACTION_HELP;
	int hide_stdlib = STDLIB_HIDE;
	char *function = NULL;
	vmptr_t function_addr = 0;
	int compacity = 0;
	char *binary;

	struct vm_program *program;
	struct rebuilt_program *rp;

	// Get the options
	while ((c = getopt(argc, argv, "sf:c")) != -1) {
		switch (c) {
		case 's':
			hide_stdlib = STDLIB_SHOW;
			break;
		case 'f':
			function = optarg;
			break;
		case 'c':
			compacity++;
			break;
		case '?':
			if (optopt == 'f')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		}
	}

	// Make sure we were given an action
	if (optind > argc - 1) {
		usage();
		return 1;
	}
	// Get the action
	if (strcmp(argv[optind], "help") == 0) {
		//action = ACTION_HELP;
		usage();
		return 0;
	} else if (strcmp(argv[optind], "fn") == 0) {
		action = ACTION_DUMP_FUNCTIONS;
	} else if (strcmp(argv[optind], "cg") == 0) {
		action = ACTION_MAKE_CALLGRAPH;
	} else if (strcmp(argv[optind], "cfg") == 0 && function != NULL) {
		action = ACTION_MAKE_CFG;
	} else {
		usage();
		return 1;
	}

	// Make sure we were given a file name
	optind++;
	if (optind != argc - 1) {
		usage();
		return 1;
	}
	// Get the binary program name
	binary = argv[optind];

	// Start the virtual machine
	program = vm_open_program(binary);

	if (function != NULL) {
		// If a function was given, decode it...
		//     Is it an address?
		if (function[0] == '0' && function[1] == 'x') {
			errno = 0;
			function_addr = strtol(function, NULL, 16);
			if (errno != 0) // error decoding the string
				function_addr = 0;
		}
		//     It's not an address, so let's find it in the symbols
		if (function_addr == 0) {
			if (vm_get_symbol_addr(program, function, &function_addr) != 0) {
				printf("error: function not found: \"%s\"\n", function);
				ret = 1;
				goto end_vm;
			}
		}
	}

	// Create a new rebuilt program and launch decompilation!
	rp = rp_new();
	decompile(program, rp);

	// Finally, display what the user wants
	if (action == ACTION_DUMP_FUNCTIONS) {
		if (function != NULL)
			rp_dump_function_by_addr(rp, function_addr, compacity);
		else
			rp_dump_functions(rp, hide_stdlib, compacity);
	} else if (action == ACTION_MAKE_CALLGRAPH) {
		rp_dump_callgraph(rp, hide_stdlib);
	} else if (action == ACTION_MAKE_CFG) {
		rp_dump_cfg_for_function(rp, function_addr);
	}

	rp_free(rp);

end_vm:
	vm_close_program(program);

	return ret;
}
