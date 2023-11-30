a from-the-bootloader x86 os to try out some ideas in systems theory but mostly
to tide me over this running injury. already did breads n plants n wanted to get
in early on the next big quarantine hobby :'-)

----

i'm on an m1 running macos 13.13.1. we'll need gcc--i built and installed 13.2.0.
as stated in gcc's docs, build _outside_ the source directory. there are
also some build and runtime dependencies to be wary of.

```sh
mkdir gcc-13.2.0.host.build
cd gcc-13.2.0.host.build
../gcc-13.2.0/configure \
--prefix=/usr/local/gcc-13.2.0 \
--program-suffix=-13.2 \
--enable-checking=release \
--with-sysroot=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk \
--with-gmp-include=... \
--with-gmp-lib=... \
--with-mpc=... \
--with-mpfr-include=... \
--with-mpfr-lib=...

make -j8
sudo make install-strip
```

next, set up the environment:

```sh
# use gcc we just installed
export CC=/usr/local/gcc-13.2.0/bin/gcc-13.2
export CXX=/usr/local/gcc-13.2.0/bin/g++-13.2
export CPP=/usr/local/gcc-13.2.0/bin/cpp-13.2
export LD=/usr/local/gcc-13.2.0/bin/gcc-13.2

# home for our cross toolchain so that its isolated from the hosts'
export PREFIX=/usr/local/cross
export PATH="$PREFIX/bin:$PATH"

export TARGET=i686-elf
```

build cross binutils. do this before building cross gcc, which will link
target libraries. i had to install texinfo first.

```sh
mkdir binutils-2.41.cross.build
cd binutils-2.41.cross.build
../binutils-2.41/configure \
--target=$TARGET \
--prefix="$PREFIX" \
--with-sysroot \
--disable-nls \
--disable-werror \
--with-gmp-include=... \
--with-gmp-lib=... \
--with-mpfr-include=... \
--with-mpfr-lib=...

make -j8
sudo make install
```

finally, our cross compiler::

```sh
mkdir gcc-13.2.0.cross.build
cd gcc-13.2.0.cross.build
../gcc-13.2.0/configure \
--prefix="$PREFIX" \
--target=$TARGET \
--disable-nls \
--enable-languages=c,c++ \
--without-headers \
--with-sysroot=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk \
--with-gmp-include=... \
--with-gmp-lib=... \
--with-mpc=... \
--with-mpfr-include=... \
--with-mpfr-lib=...

make -j8 all-gcc
make -j8 all-target-libgcc
sudo make install-gcc
sudo make install-target-libgcc
```

currently, the only supported boot medium is a disk image. `host/scripts/mkimg`
(invoked as part of `make`) will create an 80mb single-partition fat32 image
for this purpose.

_debugger debugging_

i wasn't able to get lldb to load the kernel--although i made only a pitiful
effort to do so--and saw mention of 32 bit support being dropped in catalina so
i went back to gdb. the version (8.0.1) of gdb i had `brew install`'ed a long
while ago didn't support 32 bit ELFs so i reinstalled it. i think there used to
be a `--with-all-targets` install option but it looks like that is default
enabled now, so `brew install gdb` is all that's needed to get a version (e.g.
9.2) that can load our kernel. to check supported architectures, try:

```sh
(gdb) show configuration
(gdb) set debug arch 1
(gdb) set architecture <tab> # should show lots of stuff (200 for us)
(gdb) set gnutarget <tab> # should show lots of stuff (200 for us)
```

also, it looks like i8086 [doesn't support real mode assembly](https://sourceware.org/bugzilla/show_bug.cgi?id=22869),
e.g. `set architecture i8086; break *0x7c00; layout asm` still shows 32 bit code :-(,
but our good friends at so have come up with a [fix](https://stackoverflow.com/questions/32955887/how-to-disassemble-16-bit-x86-boot-sector-code-in-gdb-with-x-i-pc-it-gets-tr).

some other useful tidbits:

for some reason, we have to `symbol-file` before `file` (possible gdb [bug](https://stackoverflow.com/questions/57239664/gdb-reading-symbols-with-symbol-file-command-on-a-core-file)):

```sh
make gdb # will output symbols in build/bin/beans.sym
(gdb) symbol-file ./build/bin/beans.sym
(gdb) file ./build/bin/beans
(gdb) target remote :1234
(gdb) break src/kernel/main.c:<somewhere useful>
(gdb) print *loopy
(gdb) x/4bx <whatever is in loopy->mod_start> # gdb can't examine this memory
(gdb) stepi # until in the module
(gdb) x/i $pc # should see our loopy jmps
...
```

there are some convenience scripts in `host/scripts/gdb`, e.g.

```sh
gdb -command host/scripts/gdb/boot
# breaks at start of partition probing, in relocated mbr (offset from 0x600)
gdb -command host/scripts/gdb/mbr
```
