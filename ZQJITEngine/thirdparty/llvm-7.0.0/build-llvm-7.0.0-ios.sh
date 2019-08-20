#! /bin/bash

#curl http://releases.llvm.org/7.0.0/llvm-7.0.0.src.tar.xz -O 
#tar xvzf llvm-7.0.0.src.tar.xz
#rm llvm-7.0.0.src.tar.xz
mv llvm-7.0.0.src llvm-7.0.0
cd llvm-7.0.0

# # get clang
pushd tools
# #curl http://releases.llvm.org/7.0.0/cfe-7.0.0.src.tar.xz -O
tar xvzf cfe-7.0.0.src.tar.xz
# #rm cfe-7.0.0.src.tar.xz
mv cfe-7.0.0.src clang
popd

# # Get libcxx and libcxxabi
pushd projects
#curl http://releases.llvm.org/7.0.0/libcxx-7.0.0.src.tar.xz -O
tar xvzf libcxx-7.0.0.src.tar.xz
#rm libcxx-7.0.0.src.tar.xz
mv libcxx-7.0.0.src libcxx
#curl http://releases.llvm.org/7.0.0/libcxxabi-7.0.0.src.tar.xz -O
tar xvzf libcxxabi-7.0.0.src.tar.xz
#rm libcxxabi-7.0.0.src.tar.xz
mv libcxxabi-7.0.0.src libcxxabi
popd

# compile for OSX (about 2h, 8GB of disk space with BUILD_SHARED_LIBS=ON)
mkdir  build_osx
pushd build_osx
cmake -DBUILD_SHARED_LIBS=ON  ..
cmake --build .
popd

# get libcxx and libcxxabi out of the way:
mkdir  dontBuild
mv projects/libcxx dontBuild
mv projects/libcxxabi dontBuild
# TODO: some combination of build variables might allow us to build these too. 
# Right now, they fail. Maybe CFLAGS with: -D__need_size_t -D_LIBCPP_STRING_H_HAS_CONST_OVERLOADS 

# Now, compile for iOS using the previous build:
# About 24h, 6 GB of disk space
# Flags you could use: LLVM_LINK_LLVM_DYLIB and BUILD_SHARED_LIBS, to make everything use dynamic libraries
# (I did not test these)
LLVM_SRC=${PWD}
echo "------------"
echo ${LLVM_SRC}
mkdir  build_ios
pushd build_ios
cmake -DBUILD_STATIC_LIBS=ON \
-DCMAKE_BUILD_TYPE=Release \
-DLLVM_TARGET_ARCH=AArch64 \
-DLLVM_TARGETS_TO_BUILD=AArch64 \
-DLLVM_DEFAULT_TARGET_TRIPLE=arm64-apple-darwin18.0.0 \
-DLLVM_ENABLE_THREADS=OFF \
-DLLVM_TABLEGEN=${LLVM_SRC}/build_osx/bin/llvm-tblgen \
-DCLANG_TABLEGEN=${LLVM_SRC}/build_osx/bin/clang-tblgen \
-DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/ \
-DCMAKE_C_COMPILER=${LLVM_SRC}/build_osx/bin/clang \
-DCMAKE_LIBRARY_PATH=${LLVM_SRC}/build_osx/lib/ \
-DCMAKE_INCLUDE_PATH=${LLVM_SRC}/build_osx/include/ \
-DCMAKE_C_FLAGS="-arch arm64 -target arm64-apple-darwin18.0.0 -I${LLVM_SRC}/build_osx/include/ -miphoneos-version-min=9" \
-DCMAKE_CXX_FLAGS="-arch arm64 -target arm64-apple-darwin18.0.0 -I${LLVM_SRC}/build_osx/include/c++/v1/ -miphoneos-version-min=9" \
..
cmake --build .
popd