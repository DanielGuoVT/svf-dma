#!/bin/bash

# Download and extract a tarball to specified destination
function download() {
    URL=$1
    DST=$2
    FILENAME=$(basename $URL)
    wget $URL
    mkdir -p $DST
    tar --strip 1 -xf $FILENAME -C $DST
    rm $FILENAME
}


# Set up make jobs 
MAKE_JOBS=1
CNT_CPU=$(grep -c ^processor /proc/cpuinfo)
if [ $CNT_CPU -gt 2 ]; then
	MAKE_JOBS=$((CNT_CPU / 2))
fi

# Install required packages
#apt-get update -y --fix-missing
#apt install -y wget subversion cmake libboost-dev libboost-system-dev git python subversion build-essential curl libcap-dev libncurses5-dev python-minimal python-pip unzip libtcmalloc-minimal4 libgoogle-perftools-dev libgmp-dev zlib1g-dev 


# Download & build llvm 6.0 and clang 6.0 
llvm_version=$(llvm-config --version)
if [ "$llvm_version" != "6.0.0" ]; then
    # URLs
    URL_LLVM=http://releases.llvm.org/6.0.0/llvm-6.0.0.src.tar.xz
    URL_CLANG=http://releases.llvm.org/6.0.0/cfe-6.0.0.src.tar.xz

    download $URL_LLVM llvm 
    download $URL_CLANG llvm/tools/clang

    mkdir -p llvm/build
    cd llvm/build
    cmake -DLLVM_ENABLE_RTTI=ON -DLLVM_BUILD_RUNTIME=ON -DLLVM_ENABLE_ZLIB=ON -DCMAKE_INSTALL_PREFIX=/usr ..
    make -j$MAKE_JOBS

		cd ../..
		export LLVM_SRC=./llvm
		export LLVM_DIR=./llvm/build
		export PATH=$LLVM_DIR/bin:$PATH
fi

# Build SVF with our dma library
DIRECTORY=Release-build
if [ ! -d "$DIRECTORY" ]; then
	mkdir $DIRECTORY
fi

cd $DIRECTORY
cmake ../
make -j

