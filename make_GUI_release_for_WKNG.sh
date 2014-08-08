#!/bin/bash

REL_DIR="./Bootloader_3.3.1_WKNG_Linux"
rm -f -R $REL_DIR
mkdir $REL_DIR

# cp Release_note.txt $REL_DIR

WKNG_DIR="/WKNG"
mkdir -p $REL_DIR$WKNG_DIR/MIFI

cd ./Loader/Platforms/WKNG/MIFI

./make_loader_MIFI_LINUX.sh
cp *.blf ../../../../$REL_DIR$WKNG_DIR/MIFI
cp *.bin ../../../../$REL_DIR$WKNG_DIR/MIFI

cd ../../../../
