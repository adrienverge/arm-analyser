# cross-platform gcc: apt-get install gcc-4.4-arm-linux-gnueabi
# cross-platform objdump: apt-get install binutils-multiarch

ARMANALYSER = arm-analyser
SOURCES = main.c common.h decompiler.c decompiler.h vm.c vm.h \
	rebuilt_program.c rebuilt_program.h syscalls.c syscalls.h \
	groups.c groups.h arrays.c arrays.h arm_instructions.c arm_instructions.h

CC ?= gcc
CFLAGS = -std=gnu11 -Wall
LDFLAGS = -lelf

default: $(ARMANALYSER)

$(ARMANALYSER): $(SOURCES)
	$(CC) $(CFLAGS) $^ -o $@ -DDEBUG $(LDFLAGS)

clean:
	rm -f $(ARMANALYSER)

syscalls.c:
	echo "// auto-generated file, use 'make syscalls.c' to re-generate it\n\nchar *arm_syscall_name(const int syscall_no)\n{\n\tswitch (syscall_no) {" > syscalls.c
	grep '^#define __NR_[a-z0-9_]\+\s\+(' /docs/linux/arch/arm/include/uapi/asm/unistd.h | sed -e "s/^#define __NR_\([a-z0-9_]\+\)\s\+(__NR_SYSCALL_BASE+\s*\(.\+\))\(\s.*\)*/\t\tcase \2:\treturn \"\1\";/g" >> syscalls.c
	echo "\t}\n\treturn \"(unknown)\";\n}" >> syscalls.c

