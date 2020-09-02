we're running macos 10.14.6, with gcc 4.2.1. before setting up our cross
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

next, set a home for our cross builds, so that they're isolated from system
tools, and what we're targeting:

```sh
export PREFIX=/usr/local/cross
export PATH="$PREFIX/bin:$PATH"
export TARGET=i686-elf
```

now we build binutils for our cross compiler. do this before building gcc,
which presumably links target libraries--we ran into issues when we tried using
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

finally, we'll be building our boot images with grub:

```sh
# grub needs xorriso and objconv
brew install xorriso

mkdir objconvbuild
/usr/local/gcc-10.2.0/bin/g++-10.2 -o objconv -O2 src/*.cpp --prefix="$PREFIX"
cp objconv "$PREFIX/bin"
cd ..

cd grub-2.04
./autogen.sh
# we had to make this 2-line patch to get 2.04 to build:
# https://www.mail-archive.com/grub-devel@gnu.org/msg29007.html
cd ..
mkdir grubbuild
cd grubbuild
../grub-2.04/configure --prefix="$PREFIX" --target=$TARGET --disable-werror
```

_macos debuggers_
we weren't able to get lldb to load the kernel--although
we made only a pitiful effort to do so--and saw mention of 32 bit support being dropped
in catalina so we went back to gdb. the version (8.0.1) of gdb we had `brew install`'ed
a long while ago didn't support 32 bit ELFs so we reinstalled it. we think there
used to be a `--with-all-targets` option for `brew install gdb` but it looks like
that is default enabled so at the time of this writing `brew install gdb` is all
that's needed to get a version (e.g. 9.2 for me) that can load our kernel.

for some reason, we have to `symbol-file` before `file` (possible gdb [bug](https://stackoverflow.com/questions/57239664/gdb-reading-symbols-with-symbol-file-command-on-a-core-file)):

```sh
make gdb # will output symbols in build/bin/beans.sym
(gdb) symbol-file ./build/bin/beans.sym
(gdb) file ./build/bin/beans.bin
(gdb) target remote :1234
(gdb) break src/kernel/main.c:<somewhere useful>
(gdb) print *loopy
(gdb) x/4bx <whatever is in loopy->mod_start> # gdb can't examine this memory
(gdb) stepi # until in the module
(gdb) display/i $pc # should see our loopy jmps
...
```

_debugger debugging_
- check supported architectures:
  - (gdb) set arch debug 1
    (gdb) set architecture <tab> # should show lots of stuff (200 for me)
  - (gdb) set gnutarget <tab> # should show lots of stuff (200 for me)
  - (gdb) show configuration
  - remember, we want to load target: `file ./build/bin/beans.bin`
    - want not stripped
