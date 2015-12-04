ARM Analyser
============

ARM binaries analyser

This programs analyses binaries compiled for the ARM architecture. It reconstructs the program structure by finding functions, even if they are hidden (not in the symbol table).
This is mainly done by registering every branching instruction (jumps, calls, returns; conditional or not; static or dynamic) and running a custom algorithm to determine functions boundaries and who calls who.
The program can output various representations, including DOT files to display graphs (callgraph, CFG) with GraphViz.

For more information, this software is fully described in rapport.pdf (French).

Copyright 2013 Adrien Vergé <adrien.verge@polymtl.ca>

Released under the GPLv2 license.

Installation
------------

First, you need to install the dependency: the libelf headers.

On Fedora:
```
# dnf install elfutils-libelf-devel
```

On Debian:
```
# apt-get install libelf-dev
```

Then, simply run:
```
$ make
```

To print pretty graphs (such as the analysed program CFG), you will need [GraphViz] [1].
To make your own test binaries, you will need a C cross-compiler such as [GNU EABI gcc] [2].

[1]: http://www.graphviz.org/  "GraphViz"
[2]: http://gcc.gnu.org/install/specific.html  "GNU EABI gcc"

Usage
-----

```
$ ./arm-analyser help
Usage: ./arm-analyser action [options] program
actions:
  help      display this help
  fn        dump functions
  cg        generate callgraph
  cfg       generate CFG (option -f needed)
options:
  -s        show standard C library
  -f FN     limit action to function FN (name or address)
options for action fn:
  -c        compact dump (names, addresses and childs)
  -cc       very compact dump (only addresses)
```

You can test this program on sample test binaries, that are given in the `test` directory.

Example: find and display functions
```
$ ./arm-analyser fn -c test/helloworld
main        0x00008388	0x000083d0	function1,function2
function1   0x000082e0	0x0000833c
function2   0x0000833c	0x00008388	function3
function3   0x0000824c	0x00008274
```

Example: show branching inside the `main` function
```
$ ./arm-analyser fn -f main test/helloworld
main
  08388 {
  083ac   BRANCH ( CALL )   static addr -> 082e0 (function1)
  083bc   BRANCH ( CALL )   static addr -> 0833c (function2)
  083cc   BRANCH (RETURN)   dynam. addr
  083d0 }
```

Example: generate callgraph and CFG (requires GraphViz):
```
$ ./arm-analyser cg test/helloworld | dot -Teps -o callgraph.eps
$ ./arm-analyser cfg -f test test/helloworld | dot -Teps -o cfg-test.eps
```
