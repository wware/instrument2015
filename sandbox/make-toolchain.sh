#!/bin/sh
# http://preshing.com/20141119/how-to-build-a-gcc-cross-compiler/
# Also see http://www.theairportwiki.com/index.php/Building_a_cross_compile_of_GCC_for_MIPS_on_OS_X


PREFIX=/opt/gnu-arm

# BINUTILS=binutils-2.24
BINUTILS=binutils-2.25

if [ ! -f $BINUTILS.tar.gz ]; then
wget http://ftp.gnu.org/gnu/binutils/$BINUTILS.tar.gz || exit 1
fi

# GCC=gcc-4.9.2
GCC=gcc-4.8.5

if [ ! -f $GCC.tar.gz ]; then
wget http://ftpmirror.gnu.org/gcc/$GCC/$GCC.tar.gz || exit 1
fi

# wget http://ftpmirror.gnu.org/glibc/glibc-2.20.tar.xz

if [ ! -f mpfr-3.1.2.tar.xz ]; then
wget http://ftpmirror.gnu.org/mpfr/mpfr-3.1.2.tar.xz || exit 1
fi

if [ ! -f gmp-6.0.0a.tar.xz ]; then
wget http://ftpmirror.gnu.org/gmp/gmp-6.0.0a.tar.xz || exit 1
fi

if [ ! -f mpc-1.0.2.tar.gz ]; then
wget http://ftpmirror.gnu.org/mpc/mpc-1.0.2.tar.gz || exit 1
fi

for f in *.tar*; do tar xf $f; done

(cd $GCC
ln -s ../mpfr-3.1.2 mpfr
ln -s ../gmp-6.0.0 gm
ln -s ../mpc-1.0.2 mpc)

mkdir -p $PREFIX

export PATH=$PREFIX/bin:$PATH

mkdir build-binutils
(cd build-binutils
CC=$GCC CFLAGS=-Wno-error=deprecated-declarations ../$BINUTILS/configure \
    --target=arm-none-eabi --enable-interwork --disable-multilib
make -j4
make install)

mkdir -p build-gcc
(cd build-gcc
../$GCC/configure --prefix=$PREFIX --target=arm-none-eabi --enable-languages=c,c++ --disable-multilib
make -j4 all-gcc
make install-gcc)

