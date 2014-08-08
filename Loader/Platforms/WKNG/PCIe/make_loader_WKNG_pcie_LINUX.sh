#!/bin/bash

CUR=$(pwd)
cd ../../../Build
make -f BootLoader_linux.mak PLATFORM=WKNG SPI=1 DMA=1
cd $CUR

cp ../release/JASPER/WKNG_LINUX_ARM_3_3_1.bin ./WKNG_Loader_spi_ARM_3_3_1.bin
cp ../release/JASPER/WKNG_LINUX_ARM_3_3_1.map ./WKNG_Loader_spi_ARM_3_3_1.map

