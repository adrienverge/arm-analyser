# cross-platform gcc: apt-get install gcc-4.4-arm-linux-gnueabi
# cross-platform objdump: apt-get install binutils-multiarch

ARMANALYSER = arm-analyser
SAMPLEPROGRAM = test/helloworld
TARGETS = $(SAMPLEPROGRAM) $(ARMANALYSER)

ARMCC ?= arm-linux-gnueabi-gcc
ARMCFLAGS = -Wall -O0 -static -march=armv5
#ARMCFLAGS = -Wall -O0 -static -march=armv5 -nostdlib

.PHONY:
default:
	make -C src
	[ -f $(ARMANALYSER) ] || ln -s src/$(ARMANALYSER) $(ARMANALYSER)

.PHONY:
test: $(SAMPLEPROGRAM)

$(SAMPLEPROGRAM): $(SAMPLEPROGRAM).c
	$(ARMCC) $(ARMCFLAGS) $< -o $@

clean:
	rm -f $(ARMANALYSER)
	make -C src clean
	rm -f $(SAMPLEPROGRAM)
