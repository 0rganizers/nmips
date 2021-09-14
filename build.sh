CUR=$PWD
BUILD_DIR=$PWD/build
echo "[*] Building nmips plugin at $BUILD_DIR!"
rm -rf $BUILD_DIR || echo "No existing build dir, sadge"
mkdir $BUILD_DIR

echo "[*] Decrypting idasdk..."
# echo $IDA_PASS | age -d -o $BUILD_DIR/idasdk.zip idasdk76.zip
openssl aes-256-cbc -d -md sha256 -in idasdk.zip -out $BUILD_DIR/idasdk.zip -pass env:IDA_PASS
echo "[*] Setting up idasdk..."
cd $BUILD_DIR
unzip idasdk
cd $CUR
export IDA_SDK=$BUILD_DIR/idasdk76
echo "[*] IDA SDK at $IDA_SDK"

# echo "[*] Setting up and building binutils..."
# BINUTILS_URL="https://github.com/MediaTek-Labs/nanomips-gnu-toolchain/releases/download/nanoMIPS-2021.02-01/binutils-2021.02-01.src.tar.gz"
# wget $BINUTILS_URL -O $BUILD_DIR/binutils.tar.gz
# cd $BUILD_DIR
# echo "[*] Extracting binutils"
# tar -xzf binutils.tar.gz
# export BINUTILS_DIR=$BUILD_DIR/binutils-2021.02-01
# cd $BINUTILS_DIR
# echo "[*] configuring binutils"
# ./configure --prefix $CUR/libs --enable-shared --disable-werror --target=nanomips-gnu-elf
# echo "[*] making binutils"
# make -j `nproc`
# cd $CUR

# if [[ "$OSTYPE" == "darwin"* ]]; then
#     echo "[*] We are on macOS, force clang!"
#     export CC=clang
#     export CXX=clang++
# fi

echo "[*] Setting up and building plugin"
GOOD_PATH=$(echo "$PATH" | sed -e 's/:\/mingw64\/bin\(:\|$\)//')
GOOD_PATH=$(echo "$GOOD_PATH" | sed -e 's/:\/usr\/bin\(:\|$\)//')
echo "[*] Fixed Path: $GOOD_PATH"
cd $CUR/plugin
PATH=$GOOD_PATH which cl || true
PATH=$GOOD_PATH cl -help || true
PATH=$GOOD_PATH cl -std:c++20 -std=c++20 || true
PATH=$GOOD_PATH meson setup $BUILD_DIR/builddir -Didasdk=$IDA_SDK
echo "[*] Building plugin..."
PATH=$GOOD_PATH meson compile -C $BUILD_DIR/builddir
