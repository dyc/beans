i'm developing this on an early 2015 mbp. it has a 2.7ghz i5, 8gb memory, and is running macos 10.14.6.

# cross compiler

my system gcc version was 4.2.1. i updated to the [latest version](https://wiki.osdev.org/Building_GCC) which was 10.2.0:

```
# paths relative to the directory containing gcc source
mkdir gccbuild
cd gccbuild
../gcc-10.2.0/configure \
--prefix=/usr/local/gcc-10.2.0 \
--program-suffix=-10.2 \
--enable-checking=release \
--with-sysroot=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

# this took over 12? hours on my poor lil laptop :'-)
make -j8
sudo make install-strip
```

next i built the cross compiler using new gcc:

```
export CC=/usr/local/gcc-10.2.0/bin/gcc-10.2
export CXX=/usr/local/gcc-10.2.0/bin/g++-10.2
export CPP=/usr/local/gcc-10.2.0/bin/cpp-10.2
export LD=/usr/local/gcc-10.2.0/bin/gcc-10.2
export TARGET=i686-elf
export PREFIX=/usr/local/gcc-10.2.0-cross
export PATH="$PREFIX/bin:$PATH"

mkdir gcccrossbuild
cd gcccrossbuild
../gcc-10.2.0/configure \
--prefix="$PREFIX" \
--program-suffix=-10.2-cross \
--target=$TARGET \
--disable-nls \
--enable-languages=c,c++ \
--without-headers \
--with-sysroot=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

make all-gcc
make all-target-libgcc
sudo make install-gcc
sudo make install-target-libgcc
```

and cross binutils, version 2.35:

```
export PREFIX=/usr/local/binutils-2.35-cross

mkdir binutilscross
cd binutilscross
../binutils-2.35/configure \
--target=$TARGET \
--prefix="$PREFIX" \
--with-sysroot \
--disable-nls \
--disable-werror

make -j8
sudo make install
```
