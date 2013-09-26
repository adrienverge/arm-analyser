/**
 * @file    rebuilt_program.c
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
 * This file contains structures to describe a newly reconstructed program and
 * its functions. It also gives methods to access these structures.
 */

#include "arrays.h"
#include "common.h"
#include "decompiler.h"
#include "groups.h"
#include "rebuilt_program.h"
#include "syscalls.h"

/**
 * Simply dumps a statement and prints its characteristics.
 */
void statement_dump(struct statement* s)
{
	printf("statement:  0x%08x  %6s -> 0x%08x (%4s)    %6s  %s  %s\n",
		(int) s->addr, STATEMENT_TYPE(s->type),
		(int) s->to_addr, s->to_function==-1?"":"(fX)",
		s->type==BRANCH?STATEMENT_BR_TYPE(s->br_type):"",
		STATEMENT_COND(s->cond), STATEMENT_STATICITY(s->staticity));
}

/**
 * Creates and initializes a new rebuilt_program structure.
 */
struct rebuilt_program *rp_new()
{
	struct rebuilt_program *rp;

	rp = malloc(sizeof(struct rebuilt_program));
	if (rp == NULL)
		FATAL_ERROR("malloc");
	memset(rp, 0, sizeof(struct rebuilt_program));

	LIST_INIT(rp->statements);
	LIST_INIT(rp->functions);
	rp->explored = group_init();

	return rp;
}

/**
 * Frees a rebuilt_program structure allocated by rp_new().
 */
void rp_free(struct rebuilt_program *rp)
{
	int i;

	LIST_ITERATOR(rp->functions, i)
		LIST_FREE(rp->functions[i].statements);

	group_free(rp->explored);
	LIST_FREE(rp->functions);
	LIST_FREE(rp->statements);
}

/**
 * Adds a functions to the rebuilt_program's list of functions.
 */
int rp_add_function(struct rebuilt_program *rp)
{
	struct rebuilt_function function;

	memset(&function, 0, sizeof(function));
	function.id = LIST_LENGTH(rp->functions);
	function.from_stdlib = 0;
	LIST_APPEND(rp->functions, function);

	LIST_INIT(rp->functions[function.id].statements);

	return LIST_LENGTH(rp->functions) - 1;
}

/**
 * Returns the function in the list of functions that matches the given
 * virtual address, if that function exists.
 */
int rp_get_function_by_vaddr(struct rebuilt_program *rp, vmptr_t vaddr)
{
	int i;

	LIST_ITERATOR(rp->functions, i)
		if (rp->functions[i].vaddr_start == vaddr)
			return i;

	return -1;
}

void rp_function_set_name(struct rebuilt_function *f, const char *name)
{
	strncpy(f->name, name, NAMES_LENGTH - 1);
}

void rp_function_add_statement(struct rebuilt_function *f, const struct statement *s)
{
	LIST_APPEND(f->statements, *s);
}

/**
 * Checks if there are some overlapping functions in the list, e.g. if two
 * functions f and g are such as f.start <= g.end and f.end > g.start.
 */
int rp_check_overlapping_functions(struct rebuilt_program *rp)
{
	int ret = 0;
	int i, j;
	struct rebuilt_function *f, *g;
	
	printf(" == checking overlapping functions ==\n");

	for (i = 0; i < LIST_LENGTH(rp->functions); i++) {
		f = &(rp->functions[i]);
		for (j = i + 1; j < LIST_LENGTH(rp->functions); j++) {
			g = &(rp->functions[j]);
			if (f->vaddr_end > g->vaddr_start
				&& f->vaddr_start < g->vaddr_end) {
				printf("overlapping functions: %s and %s\n", f->name, g->name);
				printf("\t0x%08x -> 0x%08x\tand\t0x%08x -> 0x%08x\n",
					(int) f->vaddr_start, (int) f->vaddr_end,
					(int) g->vaddr_start, (int) g->vaddr_end);
			}
		}
	}

	return ret;
}

void rp_fix_overlapping_functions(struct rebuilt_program *rp)
{
	int i, j;
	struct rebuilt_function *f, *g;

	for (i = 0; i < LIST_LENGTH(rp->functions); i++) {
		f = &(rp->functions[i]);
		for (j = i + 1; j < LIST_LENGTH(rp->functions); j++) {
			g = &(rp->functions[j]);
			if (f->vaddr_end > g->vaddr_start
				&& f->vaddr_start < g->vaddr_end) {
				// One of these functions needs to be fixed
				/*if (f->vaddr_end != g->vaddr_end) {
					FATAL_ERROR("cannot fix overlapping functions %s and %s",
						f->name, g->name);
				} else*/ if (f->vaddr_start < g->vaddr_start) {
					f->vaddr_end = g->vaddr_start;
				} else {
					g->vaddr_end = f->vaddr_start;
				}
			}
		}
	}
}

/**
 * Displays one function, very compactly: start and end addresses.
 */
void rp_dump_function_very_compact(struct rebuilt_program *rp,
	struct rebuilt_function *f)
{
	printf("0x%08x\t0x%08x\n", (int) f->vaddr_start, (int) f->vaddr_end);
}

/**
 * Displays one function, compactly: addresses and childs, on one line.
 */
void rp_dump_function_compact(struct rebuilt_program *rp,
	struct rebuilt_function *f)
{
	int j;
	struct statement *s;
	int *already_done_f;
	int first_child = 1;

	printf("%s\t0x%08x\t0x%08x\t", f->name, (int) f->vaddr_start,
		(int) f->vaddr_end);

	LIST_INIT(already_done_f);
	LIST_ITERATOR(f->statements, j) {
		s = &(f->statements[j]);

		// Dump child functions
		if (s->type == BRANCH && s->to_function != -1) {
			LIST_IFNOT_CONTAINS(already_done_f, s->to_function) {
				if (!first_child)
					printf(",");
				printf("%s", rp->functions[s->to_function].name);
				LIST_APPEND(already_done_f, s->to_function);
				first_child = 0;
			}
		}
	}
	LIST_FREE(already_done_f);

	printf("\n");
}

/**
 * Displays one function, with all info: addresses and all inner statements.
 */
void rp_dump_function_debug(struct rebuilt_program *rp,
	struct rebuilt_function *f)
{
	int j;
	struct statement *s;

	printf("%s%s\n", f->name, f->from_stdlib?" (stdlib)":"");
	printf("\t%05x {\n", (int) f->vaddr_start);
	// Dump statements
	LIST_ITERATOR(f->statements, j) {
		s = &(f->statements[j]);
		if (s->type == BRANCH) {
			printf("\t%05x   BRANCH (%s)  %s  %s", (int) s->addr,
				STATEMENT_BR_TYPE(s->br_type),
				STATEMENT_COND(s->cond), STATEMENT_STATICITY(s->staticity));
			if (s->to_addr != 0)
				printf("  -> %05x", s->to_addr);
			if (s->to_function != -1)
				printf(" (%s)", rp->functions[s->to_function].name);
			printf("\n");
		} else if (s->type == WORD) {
			printf("\t%05x   WORD     %08x\n", (int) s->addr, s->value);
		} else if (s->type == SYSCALL) {
			printf("\t%05x   SYSCALL  #%d (%s)\n", (int) s->addr,
				s->value, arm_syscall_name(s->value));
		}
	}
	printf("\t%05x }\n", (int) f->vaddr_end);
}

/**
 * Displays all functions in the rebuilt_program's list.
 */
void rp_dump_functions(struct rebuilt_program *rp, int hide_stdlib,
	int compacity)
{
	int i;
	struct rebuilt_function *f;

	/*printf(" == program entry point ==\n"
		"function %s @ %05x\n",
		(char *) rp->functions[rp->entry_function].name,
		(int) rp->functions[rp->entry_function].vaddr_start);
	printf(" == dumping functions ==\n"
		"%d elements%s\n", LIST_LENGTH(rp->functions),
		hide_stdlib==STDLIB_HIDE?" (stdlib hidden)":"");*/

	LIST_ITERATOR(rp->functions, i) {
		f = &(rp->functions[i]);
		if (hide_stdlib == STDLIB_HIDE && f->from_stdlib)
			continue;

		if (compacity >= 2)
			rp_dump_function_very_compact(rp, f);
		else if (compacity == 1)
			rp_dump_function_compact(rp, f);
		else
			rp_dump_function_debug(rp, f);
	}
}

/**
 * Displays one particuliar function from its start address.
 */
void rp_dump_function_by_addr(struct rebuilt_program *rp, vmptr_t addr,
	int compacity)
{
	int i;
	struct rebuilt_function *f;

	LIST_ITERATOR(rp->functions, i) {
		f = &(rp->functions[i]);

		if (f->vaddr_start == addr) {
			if (compacity >= 2)
				rp_dump_function_very_compact(rp, f);
			else if (compacity == 1)
				rp_dump_function_compact(rp, f);
			else
				rp_dump_function_debug(rp, f);
			return;
		}
	}
	printf("error: function at address 0x%x not found\n", (int) addr);
}

/**
 * Displays all functions in the rebuilt_program's list, in a format readable by
 * GraphViz. This is a callgraph, so nodes are functions, and oriented edges
 * represent calls from one function to another.
 */
void rp_dump_callgraph(struct rebuilt_program *rp, int hide_stdlib)
{
	int i, j;
	struct rebuilt_function *f;
	struct statement *s;

	int *already_done_f;
	uint32_t *already_done_s;

	printf("digraph G {\n");

	// Dump functions names
	LIST_ITERATOR(rp->functions, i) {
		f = &(rp->functions[i]);
		if (hide_stdlib == STDLIB_HIDE && f->from_stdlib)
			continue;

		printf("\tF%d [label=\"%s\"];\n", i, f->name);

		LIST_INIT(already_done_f);
		LIST_INIT(already_done_s);
		LIST_ITERATOR(f->statements, j) {
			s = &(f->statements[j]);

			// Dump child functions
			if (s->type == BRANCH) {
				if (s->to_function != -1) {
					LIST_IFNOT_CONTAINS(already_done_f, s->to_function) {
						printf("\tF%d -> F%d;\n", i, s->to_function);
						LIST_APPEND(already_done_f, s->to_function);
					}
				}
			// Dump syscalls
			} else if (s->type == SYSCALL) {
				LIST_IFNOT_CONTAINS(already_done_s, s->value) {
					printf("\tS%d_%d [label=\"syscall #%d\\n%s\", shape=box, "\
						"style=filled, fillcolor=gray50];\n" , i, j,
						s->value, arm_syscall_name(s->value));
					printf("\tF%d -> S%d_%d;\n", i, i, j);
					LIST_APPEND(already_done_s, s->value);
				}
			}
		}
		LIST_FREE(already_done_f);
		LIST_FREE(already_done_s);
	}

	printf("}\n");
}

/**
 * Function to compare two statements by their address.
 * This is used by the sorting algorithm.
 */
static int cmp_cfg_addr(const void *a, const void *b)
{
	struct cfg_node *A = (struct cfg_node *) a,
	                *B = (struct cfg_node *) b;
	if (A->addr == B->addr)
		return A->type - B->type;
	return A->addr - B->addr;
}

/**
 * Displays CFG (control flow graph) one particular function, in a format
 * readable by GraphViz.
 * More info on CFGs on: http://en.wikipedia.org/wiki/Control_flow_graph
 */
void rp_dump_cfg_for_function(struct rebuilt_program *rp, vmptr_t addr)
{
	int i, j;
	struct rebuilt_function *f = NULL;

	struct statement *s;

	struct cfg_node *nodes, *n;
	struct cfg_node node;

	// First, find the function
	LIST_ITERATOR(rp->functions, i) {
		if (rp->functions[i].vaddr_start == addr) {
			f = &(rp->functions[i]);
			break;
		}
	}
	if (f == NULL) {
		printf("error: function at address 0x%x not found\n", (int) addr);
		return;
	}

	LIST_INIT(nodes);

	// Step 1: Determine all nodes
	node.stm = NULL;
	node.show = YES;
	// Add entry node
	node.addr = f->vaddr_start;
	node.type = NODE;
	LIST_APPEND(nodes, node);
	LIST_ITERATOR(f->statements, i) {
		s = &(f->statements[i]);
		if (s->type == BRANCH && s->br_type == JUMP) {
			// Add the statement itself
			node.addr = s->addr;
			node.type = NODE;
			LIST_APPEND(nodes, node);
			// If it's a branch to outside the function, create a special node
			if (s->to_addr == 0 || s->to_addr < f->vaddr_start
				|| s->to_addr >= f->vaddr_end) {
				node.addr = s->addr;
				node.type = FUNCTION;
				LIST_APPEND(nodes, node);
			// If not, create the destination node (if it does not exist yet)
			} else {
				node.addr = s->to_addr;
				node.type = NODE;
				LIST_APPEND(nodes, node);
			}
			// If it's conditional, add a node at addr + 4
			if (s->cond == CONDITIONAL) {
				node.addr = s->addr + 4;
				node.type = NODE;
				LIST_APPEND(nodes, node);
			}
		// If it's a function call or a syscall, add the function node
		// + the next instruction for the return
		} else if ((s->type == BRANCH && s->br_type == CALL)
			|| s->type == SYSCALL) {
			// Add the statement itself
			node.addr = s->addr;
			node.type = NODE;
			LIST_APPEND(nodes, node);
			// Add the special node
			node.addr = s->addr;
			node.type = (s->type == SYSCALL) ? SYSFUNCTION : FUNCTION;
			LIST_APPEND(nodes, node);
			// Add a node at addr + 4 for return
			node.addr = s->addr + 4;
			node.type = NODE;
			LIST_APPEND(nodes, node);
		// If it's a return
		} else if (s->type == BRANCH && s->br_type == RETURN) {
			// Add the statement itself
			node.addr = s->addr;
			node.type = NODE;
			LIST_APPEND(nodes, node);
		}
	}
	// Add exit node
	node.addr = f->vaddr_end;
	node.type = NODE;
	LIST_APPEND(nodes, node);

	// Step 2: sort them by address, and remove doubles
	merge_sort(nodes, sizeof(*nodes), LIST_LENGTH(nodes), cmp_cfg_addr);
	LIST_ITERATOR(nodes, i) {
		if (i == LIST_LENGTH(nodes) - 1)
			break;
		if (nodes[i].addr == nodes[i + 1].addr
			&& nodes[i].type == nodes[i + 1].type) {
			LIST_REMOVE(nodes, i + 1);
			i--;
		}
	}

	// Step 3: match each cfg_node with its statement, if it exists
	LIST_ITERATOR(f->statements, i) {
		s = &(f->statements[i]);
		j = 0;
		while (j < LIST_LENGTH(nodes) && nodes[j].addr < s->addr)
			j++;
		while (j < LIST_LENGTH(nodes) && nodes[j].addr == s->addr) {
			nodes[j].stm = s;
			j++;
		}
	}

	// Step 4: Make edges
	LIST_ITERATOR(nodes, i) {
		n = &nodes[i];
		s = n->stm;

		n->child1 = n->child2 = -1;

		// Make standard edges for branches
		if (n->type == NODE && n->addr == f->vaddr_end) {
			continue;
		} else if (s != NULL) {
			if ((s->cond == CONDITIONAL || n->type == SYSFUNCTION
				|| n->type == FUNCTION) && !(n->type == FUNCTION &&
				s->type == BRANCH && s->br_type == JUMP)) {
				// Find the node at address + 4 and make an edge to it
				// ... except if it's a jump to a function (no return)
				for (j = i + 1; j < LIST_LENGTH(nodes)
					&& nodes[j].addr < n->addr + 4; j++) ;
				n->child1 = j;
			}
			if (n->type == NODE) {
				// If it's a return, link it to the terminal node
				if (s->type == BRANCH && s->br_type == RETURN) {
					LIST_ITERATOR(nodes, j)
						if (nodes[j].addr == f->vaddr_end
							&& nodes[j].type == NODE) {
							n->child2 = j;
							break;
						}
				// If this is a jump inside the function
				} else if (s->type == BRANCH && s->br_type == JUMP
					&& s->to_addr != 0 && s->to_addr >= f->vaddr_start
					&& s->to_addr < f->vaddr_end) {
					LIST_ITERATOR(nodes, j)
						if (nodes[j].addr == s->to_addr
							&& nodes[j].type == NODE) {
							n->child2 = j;
							break;
						}
				// If this is a call to a special edge
				} else if (s->type == SYSCALL
					|| (s->type == BRANCH
					&& (s->br_type == JUMP || s->br_type == CALL))) {
					for (j = i + 1; j < LIST_LENGTH(nodes); j++)
						if (nodes[j].addr == n->addr
							&& (nodes[j].type == FUNCTION
							|| nodes[j].type == SYSFUNCTION)) {
							n->child2 = j;
							break;
						}
				}
			}
		// Invent edges for nodes that dont have childs
		} else {
			for (j = i + 1; j < LIST_LENGTH(nodes)
				&& nodes[j].addr < n->addr + 4; j++) ;
			n->child1 = j;
		}
	}

	// Step 5: Remove useless nodes
	// Do multiple passes until all is correct
	int changed = 1;
	while (changed) {
		changed = 0;
		LIST_ITERATOR(nodes, i) {
			n = &nodes[i];
			int parents = 0, childs;
			int single_parent, single_child;
			LIST_ITERATOR(nodes, j)
				if (nodes[j].child1 == i || nodes[j].child2 == i) {
					parents++;
					single_parent = j;
				}
			childs = (n->child1 >= 0 ? 1 : 0) + (n->child2 >= 0 ? 1 : 0);
			single_child = (childs != 1 ? -1 :
				n->child1 >= 0 ? n->child1 : n->child2);
			// If it's a node, not entry nor exit, with only one parent and
			// one or zero child, and either the parent or child is a node
			if (n->type == NODE && parents == 1 && childs <= 1
				&& n->addr != f->vaddr_start && n->addr != f->vaddr_end
				&& (childs == 0 || nodes[single_parent].type == NODE
				|| nodes[single_child].type == NODE)) {
				// Let's connect its parent to its child
				if (nodes[single_parent].child1 == i)
					nodes[single_parent].child1 = single_child;
				else
					nodes[single_parent].child2 = single_child;
				// And remove this node
				n->show = NO;
				n->child1 = n->child2 = -1;
				changed = 1;
			}
		}
	}
	// Remove exit node if the function never returns
	int exit_node = -1;
	LIST_ITERATOR(nodes, i)
		if (nodes[i].addr == f->vaddr_end) {
			exit_node = i;
			break;
		}
	nodes[exit_node].show = NO;
	LIST_ITERATOR(nodes, i) {
		n = &nodes[i];
		if (n->show == YES &&
			(n->child1 == exit_node || n->child2 == exit_node)) {
			nodes[exit_node].show = YES;
			break;
		}
	}

	// Step 6: Output graph
	printf("digraph G {\n");

	LIST_ITERATOR(nodes, i) {
		n = &nodes[i];

		if (n->show == NO)
			continue;

		// Display this node
		if (n->type == NODE) {
			printf("\tN_%d_%x ", n->type, n->addr);
			if (n->addr == f->vaddr_start)
				printf("[label=\"ENTRY\\n0x%x\"];\n", n->addr);
			else if (n->addr == f->vaddr_end)
				printf("[label=\"EXIT\\n0x%x\"];\n", n->addr);
			else
				printf("[label=\"0x%x\"];\n", n->addr);
		} else if (n->type == FUNCTION) {
			if (n->stm != NULL)
				printf("\tN_%d_%x [label=\"%s\", shape=box, style=filled, "\
					"fillcolor=gray75];\n", n->type, n->addr,
					n->stm->to_function >= 0 ?
					rp->functions[n->stm->to_function].name : "?");
			else
				printf("\tN_%d_%x [label=\"%s\", shape=box, style=filled, "\
					"fillcolor=gray75];\n", n->type, n->addr, "?");
		} else if (n->type == SYSFUNCTION) {
			printf("\tN_%d_%x [label=\"syscall #%d\\n%s\", shape=box, "\
				"style=filled, fillcolor=gray50];\n", n->type, n->addr,
				n->stm->value, arm_syscall_name(n->stm->value));
		}

		// Display edges from this node
		if (n->child1 >= 0)
			printf("\tN_%d_%x -> N_%d_%x;\n", n->type, n->addr,
				nodes[n->child1].type, nodes[n->child1].addr);
		if (n->child2 >= 0)
			printf("\tN_%d_%x -> N_%d_%x;\n", n->type, n->addr,
				nodes[n->child2].type, nodes[n->child2].addr);
	}

	printf("}\n");

	LIST_FREE(nodes);
}

