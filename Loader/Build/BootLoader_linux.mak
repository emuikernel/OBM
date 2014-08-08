# (C) Copyright 2007 Marvell International Ltd. ?
#?All Rights Reserved
#
##############################################################
#
#   Top level make file for the bootloader
#
##############################################################

.EXPORT_ALL_VARIABLES:

# environment variablesplat
# the following values are frequently overridden on the command line.
# here we'll set some reasonable defaults so a build will work with no parameters.
# the defaults will do a:
#   nontrusted release ttc build for linux that supports:
#   usbci download, uart download,
#   nand, onenand and nor flash

LOADERVERSION = 3_3_1

# directory information about this source tree...
TOPDIR :=$(shell pwd)/../..

################# FLAGS START ################################
RELEASE = 1
ToolChain = LINUX
# ARM_MODE -> 1=ARM, 0=Thumb mode
ARM_MODE = 1
TRUSTED = 0
FBF = 0
DOWNLOAD_USED = 1
MIFI3 = 0
# Set UART_UPLOAD to 1 if USBCI is 0 AND you need upload through UART
UART_UPLOAD = 0
USE_SERIAL_DEBUG = 1
UPDATE_USE_KEY = 0
UPDATE_USE_ATCMD = 0
UPDATE_USE_GPIO = 0
UPDATE_USE_DETECT_USB = 0
FBF_NEW = 1
DECOMPRESS_SUPPORT = 0
ICASE = 0
MIFI_V2R0 = 0
MIFI_V2R1 = 0
MIFI_V3R0 = 0
MIFI_V3R1 = 0
MIFI_V3R2 = 0
MIFI_V3R3 = 0
SANREMO = 0
USTICA = 0
FAN540X = 0
FAN_54013 = 0
RT9455 = 0
RT8973 = 0
PRODUCTION_MODE_SUPPORT = 0
TR069_SUPPORT = 1
LWG_LTG_SUPPORT = 0
MRD_CHECK = 0
NEZHA_MIFI_V3R1 = 0
NEZHA_MIFI_V4R1 = 0
NEZHA_MIFI_V4R1R1 = 0
QPRESS = 0
SBOOT = 0
AZW_OLED = 0
AUTO_BOOTUP = 0
ONLY_DISABLE_WD = 0
SILENT_RESET = 0
BACKUP_IMAGE = 0
ZIMI_LED_SUPPORT = 0
ZIMI_LED_MODE = 0
ZIMI_PB05=0
ZIMI_LAST_LED_MODE=0
################# DEBUG FLAGS ################################
VERBOSE_DEBUG = 0
VERBOSE_MODE = 1
COPYIMAGESTOFLASH = 1
BOARD_DEBUG = 0
################# FLAGS END ##################################


####################################################
# Global Tools
#   These values are defaults and can be platform.mak
#   will override these values if desired
##############################################################
CROSS_COMPILE =  arm-marvell-eabi-
AS = $(CROSS_COMPILE)as $(AFLAGS)
LD = $(CROSS_COMPILE)ld $(LDFLAGS)
CC = $(CROSS_COMPILE)gcc $(CFLAGS)
STRIP = $(CROSS_COMPILE)strip -R .note -R .comment -S
OBJCOPY	= $(CROSS_COMPILE)objcopy -O binary -R .note -R .comment -S
OBJDUMP	= $(CROSS_COMPILE)objdump
MAKE = make
RM = rm -f
DIR = mkdir


##############################################################
# Default compile flags
#   These values are defaults and can be platform.mak
#   will override these values if desired
##############################################################
#generic between Windows and Linux (easy to update both)
CFLAGS = -DI2C=$(I2C) \
		-DNAND_CODE=$(NAND) -DNOR_CODE=$(NOR) -DDMA=$(DMA)\
        -D$(PLATFORM)=1 \
		-DSPI_CODE=$(SPI)\
		-DMMC_CODE=$(MMC) -DMMC_DEBUG=$(MMC_DEBUG) -DMMC_SDMA_MODE=$(MMC_SDMA_MODE) \
        -DHSI=$(HSI) \
		-DBOOTROM=0  -DTRUSTED=$(TRUSTED) -DBOARD_DEBUG=$(BOARD_DEBUG) \
		-DJTAG_PROTOCOL_OVER_JTAG_SUPPORTED=$(JTAG_PROTOCOL_OVER_JTAG_SUPPORTED) -DPROTOCOL_JTAG_USED=$(PROTOCOL_JTAG_USED) \
		-DVERBOSE_MODE=$(VERBOSE_MODE) -DVERBOSE_DEBUG=$(VERBOSE_DEBUG) -DDOWNLOAD_USED=$(DOWNLOAD_USED) \
		-DCOPYIMAGESTOFLASH=$(COPYIMAGESTOFLASH) \
		-DUSBCI=$(USBCI) -DCI2_USB_DDR_BUF=$(CI2_USB_DDR_BUF) \
		-DFBF=$(FBF) \
        -DUART_UPLOAD=$(UART_UPLOAD) \
        -DUSE_SERIAL_DEBUG=$(USE_SERIAL_DEBUG) -DUPDATE_USE_KEY=$(UPDATE_USE_KEY) \
        -DCOPYIMAGESTOFLASH=$(COPYIMAGESTOFLASH) \
        -DFBF_NEW=$(FBF_NEW) -DUPDATE_USE_DETECT_USB=$(UPDATE_USE_DETECT_USB)\
        -DMIFI3=$(MIFI3) -DUPDATE_USE_ATCMD=$(UPDATE_USE_ATCMD) -DUPDATE_USE_GPIO=$(UPDATE_USE_GPIO)\
        -DDECOMPRESS_SUPPORT=$(DECOMPRESS_SUPPORT) -DICASE=$(ICASE) \
        -DOLED_SUPPORT=$(OLED_SUPPORT) -DLED_DISPLAY=$(LED_DISPLAY) \
        -DSSD1306=$(SSD1306) -DSH1106=$(SH1106) \
        -DPM8607=$(PM8607) -DPM8609=$(PM8609) -DMIFI_V2R0=$(MIFI_V2R0) -DMIFI_V2R1=$(MIFI_V2R1) \
        -DSANREMO=$(SANREMO) -DUSTICA=$(USTICA) -DFAN_54013=$(FAN_54013) -DMIFI_V3R0=$(MIFI_V3R0) -DMIFI_V3R1=$(MIFI_V3R1) \
        -DFAN540X=$(FAN540X) -DRT9455=$(RT9455) \
        -DPRODUCTION_MODE_SUPPORT=$(PRODUCTION_MODE_SUPPORT) \
        -DRT8973=$(RT8973) \
        -DTR069_SUPPORT=$(TR069_SUPPORT) \
        -DMIFI_V3R2=$(MIFI_V3R2) -DMIFI_V3R3=$(MIFI_V3R3) \
        -DLWG_LTG_SUPPORT=$(LWG_LTG_SUPPORT)\
        -DMRD_CHECK=$(MRD_CHECK) -DQPRESS=$(QPRESS) \
        -DNEZHA_MIFI_V3R1=$(NEZHA_MIFI_V3R1) \
        -DNEZHA_MIFI_V4R1R1=$(NEZHA_MIFI_V4R1R1) \
        -DNEZHA_MIFI_V4R1=$(NEZHA_MIFI_V4R1) -DONLY_DISABLE_WD=$(ONLY_DISABLE_WD) \
        -DSBOOT=$(SBOOT) -DAZW_OLED=$(AZW_OLED) \
        -DAUTO_BOOTUP=$(AUTO_BOOTUP) \
	-DSILENT_RESET=$(SILENT_RESET) -DBACKUP_IMAGE=$(BACKUP_IMAGE) \
        -DZIMI_LED_SUPPORT=$(ZIMI_LED_SUPPORT) \
        -DZIMI_LED_MODE=$(ZIMI_LED_MODE) \
        -DZIMI_PB05=$(ZIMI_PB05) \
        -DZIMI_LAST_LED_MODE=$(ZIMI_LAST_LED_MODE) \

#linux unique

ifeq "$(ARM_MODE)" "1"
CFLAGS += -DLINUX_BUILD=1 -g -Os -fomit-frame-pointer -fasm -nostdinc -fno-builtin -mcpu=marvell-f -mapcs-frame -gstrict-dwarf
else
CFLAGS += -DLINUX_BUILD=1 -g -Os -fomit-frame-pointer -fasm -nostdinc -fno-builtin -mcpu=marvell-f -mthumb -mthumb-interwork
endif

AFLAGS = -Wall -g -mcpu=marvell-f -mapcs-stack-check -defsym BOOTROM=0 -defsym GCC=1 -defsym $(PLATFORM)=1

LFLAGS = -e __main -nostdlib -format elf32-littlearm

#####################################
# include directories
#####################################
INCLUDE_DIRS = \
	-I $(TOPDIR)/Common/Include \
	-I $(TOPDIR)/Common/Platforms/$(PLATFORM) \
	-I $(TOPDIR)/Common/Platforms/$(PLATFORM)/Include \
	-I $(TOPDIR)/Common/Download \
	-I $(TOPDIR)/Common/Flash \
	-I $(TOPDIR)/Common/Flash/DFC \
	-I $(TOPDIR)/Common/Flash/NAND \
	-I $(TOPDIR)/Common/Flash/XIP \
	-I $(TOPDIR)/Common/Flash/SPI \
	-I $(TOPDIR)/Common/Flash/SDMMC \
	-I $(TOPDIR)/Common/Flash/SDMMC/SDHC2 \
	-I $(TOPDIR)/Common/Misc \
	-I $(TOPDIR)/Common/SDRam \
	-I $(TOPDIR)/Common/Tim \
	-I $(TOPDIR)/Loader/Main \
	-I $(TOPDIR)/Loader/StartUp \
	-I $(TOPDIR)/Common/SecureBoot \
	-I $(TOPDIR)/Common/I2C \
	-I $(TOPDIR)/Common/DMA \
	#	-I $(TOPDIR)/Loader/SecureBoot \
	#-I $(TOPDIR)/Loader/SecureBoot/wtm_mbox \
	#-I $(TOPDIR)/Common/SecureBoot \
	#-I $(TOPDIR)/Common/SecureBoot/GEU \
	#-I $(TOPDIR)/Common/SecureBoot/FuseBlock \
	#-I $(TOPDIR)/Common/SecureBoot/IppCp \

##############################################################
# Pull in the platform specific information
#   This includes object definitions and overrides
##############################################################

ifeq "$(RELEASE)" "1"
OUTDIR = $(TOPDIR)/Loader/Platforms/$(PLATFORM)/release
else
OUTDIR = $(TOPDIR)/Loader/Platforms/$(PLATFORM)/debug
endif

include $(TOPDIR)/Loader/Platforms/$(PLATFORM)/bl_platform_linux.mak

ifeq "$(ARM_MODE)" "1"
InstructionType = ARM
else
InstructionType = THUMB
endif

AFLAGS += $(PLATFORMASMFLAGS)
CFLAGS += $(PLATFORMCFLAGS) $(BOARDDEF)
LFLAGS += $(LIBS)
LFLAGS += $(LDCMD)

#################################
# Build Process
#################################
# ensure the output directories exist
junk :=$(shell mkdir -p -v $(OUTDIR))
junk :=$(shell mkdir -p -v "$(OUTDIR)/$(BOARD)")

## Call perl script to make sure assembly files are in GNU format ##
junk :=$(shell perl makegnuassembly.pl ../)
junk :=$(shell perl makegnuassembly.pl ../../Common/Platforms/$(PLATFORM))
junk :=$(shell perl makegnuassembly.pl ../../Common/Include)
## Create OBJECTS list (changes foo.o to ../Platforms/XXX/Release/foo.o)
OBJECTS = $(patsubst %.o, $(OUTDIR)/%.o, $(OBJS))

LOADER = $(OUTDIR)/$(BOARD)/$(PLATFORM)_$(ToolChain)_$(InstructionType)_$(LOADERVERSION)
##################################################
# Implicit Rules
##################################################
.SUFFIXES: .c .S
%.o : $(TOPDIR)/Common/DMA/%.c ;                       @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Download/%.c ;                  @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Flash/%.c ;                     @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Flash/DFC/%.c ;                 @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Flash/NAND/%.c ;                @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Flash/SDMMC/%.c ;               @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Flash/SDMMC/SDHC1/%.c ;         @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Flash/SDMMC/SDHC2/%.c ;         @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Flash/SPI/%.c ;                 @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Flash/XIP/%.c ;                 @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/I2C/%.c ;                       @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/DMA/%.c ;                       @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Loader/Main/%.c ;                      @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Misc/%.c ;                      @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Platforms/$(PLATFORM)/%.c ;     @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Loader/Platforms/$(PLATFORM)/%.c ;     @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Loader/Platforms/$(PLATFORM)/%.S ;     @$(AS) $(INCLUDE_DIRS) $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/SDRam/%.c ;                     @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Loader/StartUp/%.S ;                   @$(AS) $(INCLUDE_DIRS) $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt
%.o : $(TOPDIR)/Common/Tim/%.c ;                       @$(CC) $(INCLUDE_DIRS) -c $< -o "$(OUTDIR)/$@" >log.txt 2>err.txt


#######################################################
#  Build Process... FINALLY!
#######################################################
"$(LOADER).bin" : $(OBJS)
	@echo assembly/compile done. linking.
	@$(LD) -o "$(LOADER)-elf32" $(OBJECTS) $(LFLAGS) -Map "$(LOADER).map" >log.txt 2>err.txt
	@echo linking done. strip.
	@$(STRIP) "$(LOADER)-elf32" -o "$(LOADER)-elf32-stripped"
	@echo strip done. objcopy.
	@$(OBJCOPY) "$(LOADER)-elf32-stripped" "$(LOADER).bin"
	@echo Build Successful!

clean:
	@-rm -f $(OUTDIR)/*.o "$(LOADER).bin" "$(LOADER)-elf32" "$(LOADER)-elf32-stripped" "$(LOADER).map"

