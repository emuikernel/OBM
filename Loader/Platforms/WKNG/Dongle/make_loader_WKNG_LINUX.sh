#!/bin/bash

CUR=$(pwd)
cd ../../../Build
make -f BootLoader_linux.mak PLATFORM=WKNG NAND=1
cd $CUR

cp ../release/JASPER/WKNG_LINUX_ARM_3_3_1.bin ./WKNG_Loader_Dongle_NAND_ARM_3_3_1.bin
cp ../release/JASPER/WKNG_LINUX_ARM_3_3_1.map ./WKNG_Loader_Dongle_NAND_ARM_3_3_1.map

cd ../../../Build
make -f BootLoader_linux.mak PLATFORM=WKNG SPI=1
cd $CUR

cp ../release/JASPER/WKNG_LINUX_ARM_3_3_1.bin ./WKNG_Loader_Dongle_SPI_ARM_3_3_1.bin
cp ../release/JASPER/WKNG_LINUX_ARM_3_3_1.map ./WKNG_Loader_Dongle_SPI_ARM_3_3_1.map

cd ../../../Build
make -f BootLoader_linux.mak PLATFORM=WKNG SPI=1 DECOMPRESS_SUPPORT=1
cd $CUR

cp ../release/JASPER/WKNG_LINUX_ARM_3_3_1.bin ./WKNG_Loader_Dongle_SPI_ARM_DC_3_3_1_.bin
cp ../release/JASPER/WKNG_LINUX_ARM_3_3_1.map ./WKNG_Loader_Dongle_SPI_ARM_DC_3_3_1_.map

cd ../../../Build
make -f BootLoader_linux.mak PLATFORM=WKNG SPI=1 DECOMPRESS_SUPPORT=1 ICASE=1
cd $CUR

cp ../release/JASPER/WKNG_LINUX_ARM_3_3_1.bin ./WKNG_Loader_Dongle_SPI_ICASE_ARM_3_3_1.bin
cp ../release/JASPER/WKNG_LINUX_ARM_3_3_1.map ./WKNG_Loader_Dongle_SPI_ICASE_ARM_3_3_1.map
