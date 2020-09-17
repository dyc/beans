i'm running macos 10.14.6, with gcc 4.2.1. before setting up our cross
toolchain, let's grab latest gcc (10.2.0):

```sh
mkdir gccbuild
cd gccbuild
../gcc-10.2.0/configure \
--prefix=/usr/local/gcc-10.2.0 \
--program-suffix=-10.2 \
--enable-checking=release \
--with-sysroot=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

# this took 16? hours on my poor lil laptop :'-)
make -j8
sudo make install-strip
```

and use it:

```sh
export CC=/usr/local/gcc-10.2.0/bin/gcc-10.2
export CXX=/usr/local/gcc-10.2.0/bin/g++-10.2
export CPP=/usr/local/gcc-10.2.0/bin/cpp-10.2
export LD=/usr/local/gcc-10.2.0/bin/gcc-10.2
```

next, set a home for our cross builds so that they're isolated from host
tools, and what we're targeting:

```sh
export PREFIX=/usr/local/cross
export PATH="$PREFIX/bin:$PATH"
export TARGET=i686-elf
```

now build binutils for our cross compiler. do this before building gcc,
which presumably links target libraries--i ran into issues when i tried using
gcc built without cross binutils:

```sh
mkdir crossbin
cd crossbin
../binutils-2.35/configure \
--target=$TARGET \
--prefix="$PREFIX" \
--with-sysroot \
--disable-nls \
--disable-werror

make -j8
sudo make install
```

and the cross compiler:

```sh
cd gcc-10.2.0
./contrib/download_prequisites
cd ..
mkdir crossgcc
cd crossgcc
../gcc-10.2.0/configure \
--prefix="$PREFIX" \
--target=$TARGET \
--disable-nls \
--enable-languages=c,c++ \
--without-headers \
--with-sysroot=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

make -j8 all-gcc
make -j8 all-target-libgcc
sudo make install-gcc
sudo make install-target-libgcc
```

currently, the only supported boot medium is a disk image. `host/scripts/mkimg`
(invoked as part of `make`) will create an 80mb single-partition fat32 image
for this purpose. unfortunately, this will require an interactive `fdisk`
session (see bolded for what to input):

<pre>
+ fdisk -e beans/build/bin/temp.img
fdisk: could not open MBR file /usr/standalone/i386/boot0: No such file or directory
The signature for this MBR is invalid.
Would you like to initialize the partition table? [y] <b>y</b>
Enter 'help' for information
fdisk:*1> <b>disk</b>
Disk: beans/build/bin/temp.img       geometry: 650/4/63 [163840 sectors]
Change disk geometry? [n] <b>n</b>
fdisk:*1> <b>edit 1</b>
         Starting       Ending
 #: id  cyl  hd sec -  cyl  hd sec [     start -       size]
------------------------------------------------------------------------
 1: 00    0   0   0 -    0   0   0 [         0 -          0] unused
Partition id ('0' to disable)  [0 - FF]: [0] (? for help) <b>0B</b>
Do you wish to edit in CHS mode? [n] <b>n</b>
Partition offset [0 - 163840]: [63] <b>\n</b>
Partition size [1 - 163777]: [163777] <b>\n</b>
fdisk:*1> <b>write</b>
Writing MBR at offset 0.
fdisk: 1> <b>quit</b>
</pre>

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
