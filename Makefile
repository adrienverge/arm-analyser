# cross-platform gcc: apt-get install gcc-4.4-arm-linux-gnueabi
# cross-platform objdump: apt-get install binutils-multiarch

ARMANALYSER=arm-analyser
SAMPLEPROGRAM=test/helloworld
TARGETS=$(SAMPLEPROGRAM) $(ARMANALYSER)

ARMCC=arm-linux-gnueabi-gcc
ARMCCFLAGS=-Wall -O0 -static -march=armv5
#ARMCCFLAGS=-Wall -O0 -static -march=armv5 -nostdlib

default:
	make -C src
	[ -f $(ARMANALYSER) ] || ln -s src/$(ARMANALYSER) $(ARMANALYSER)
	make $(SAMPLEPROGRAM)

$(SAMPLEPROGRAM): $(SAMPLEPROGRAM).c
	$(ARMCC) $(ARMCCFLAGS) -o $@ $<

clean:
	rm $(ARMANALYSER)
	make -C src clean
	rm $(SAMPLEPROGRAM)

livrable: livrable.tar.bz2

livrable.tar.bz2: Makefile README $(ARMANALYSER) src/* \
	test/helloworld* test/coreutils/*
	tar -cjf $@ $^
