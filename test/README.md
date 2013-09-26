Cross-compile programs for ARM
==============================

For instance, [GNU Coreutils] [1].

Download the source from [1].

Then cross-compile it with static library linking and no optimisation:
```
$ ./configure --host=arm-linux-gnueabi --target=arm-linux-gnueabi CFLAGS=-O0 LDFLAGS=-static
$ make -j8
```

[1]: http://www.gnu.org/software/coreutils/  "GNU Coreutils"
