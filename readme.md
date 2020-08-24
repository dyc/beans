_up next: serial i/o_

* [setup](#setup)
    * [cross toolchain](#cross-toolchain)
* [doin stuff](#doin-stuff)

# setup

we're using an early 2015 mbp running macos 10.14.6.

## cross toolchain

our system gcc version is 4.2.1. we update to [latest](https://wiki.osdev.org/Building_GCC) (10.2.0):

```sh
# paths relative to the directory containing gcc source
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

next we tell `make` to use gcc-10.2:

```sh
export CC=/usr/local/gcc-10.2.0/bin/gcc-10.2
export CXX=/usr/local/gcc-10.2.0/bin/g++-10.2
export CPP=/usr/local/gcc-10.2.0/bin/cpp-10.2
export LD=/usr/local/gcc-10.2.0/bin/gcc-10.2
```

and set a home for our cross builds:

```sh
# isolate cross environment from system tools
export PREFIX=/usr/local/cross
export PATH="$PREFIX/bin:$PATH"
```

as well as the cross target we're building for:

```sh
export TARGET=i686-elf
```

now we build `binutils` for our cross compiler.
do this before building gcc, which i'm guessing
links target libraries (i ran into issues
when i tried using a `gcc` built without cross
utils):

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

if you'd like to build your boot images with grub:

```sh
# we'll need xorriso and objconv
brew install xorriso

mkdir objconvbuild
/usr/local/gcc-10.2.0/bin/g++-10.2 -o objconv -O2 src/*.cpp --prefix="$PREFIX"
cp objconv "$PREFIX/bin"
cd ..

cd grub-2.04
./autogen.sh
# i had to make this 2-line patch to get 2.04 to build:
# # https://www.mail-archive.com/grub-devel@gnu.org/msg29007.html
cd ..
mkdir grubbuild
cd grubbuild
../grub-2.04/configure --prefix="$PREFIX" --target=$TARGET --disable-werror
```

# doin stuff

`make run`

boot bbos in qemu
