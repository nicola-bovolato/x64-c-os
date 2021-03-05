#!/bin/bash

TARGET=x86_64-elf
PREFIX="/usr/local/$TARGET-gcc"
PATH="$PREFIX/bin:$PATH"

TMP_FOLDER=/tmp/toolchain-src

if [ "$EUID" -ne 0 ]
  then echo "Must run as root, exiting..."
  exit
fi

mkdir $TMP_FOLDER

# Install dependencies

sudo apt update
sudo apt-get install -y \
    build-essential make curl qemu-system-x86 \
    nasm gcc g++ bison flex libgmp3-dev libmpfr-dev libmpc-dev libexpat-dev texinfo \
    grub-common xorriso grub-pc-bin

# Install binutils

cd $TMP_FOLDER
curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.35.tar.xz
tar xf binutils-2.35.tar.xz
mkdir binutils-build
cd binutils-build
../binutils-2.35/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
make
make all install

# Install gcc

cd $TMP_FOLDER
curl -O https://ftp.gnu.org/gnu/gcc/gcc-10.2.0/gcc-10.2.0.tar.xz
tar xf gcc-10.2.0.tar.xz
mkdir gcc-build
cd gcc-build
../gcc-10.2.0/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

# Install gdb

cd $TMP_FOLDER
curl -O https://ftp.gnu.org/gnu/gdb/gdb-9.2.tar.xz
tar xf gdb-9.2.tar.xz
mkdir gdb-build
cd gdb-build
../gdb-9.2/configure --target=$TARGET --prefix=$PREFIX --program-prefix=$TARGET- --with-expat
make
make install

rm -rf $TMP_FOLDER
