Cross-compile programs for ARM
==============================

For instance, let's dot it with [GNU Coreutils] [1].
Download the source from [the official website] [1].

Then cross-compile it with static library linking and no optimisation:
```
$ ./configure --host=arm-linux-gnueabi --target=arm-linux-gnueabi CFLAGS=-O0 LDFLAGS=-static
$ make -j8
```

[1]: http://www.gnu.org/software/coreutils/  "GNU Coreutils"
