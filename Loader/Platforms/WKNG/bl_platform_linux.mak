# (C) Copyright 2007 Marvell International Ltd. ?
#?All Rights Reserved
#
##############################################################
#
#  Platform Specific macros and options	- TTC
#
###########################################################

#############################################################
#
# Use these line to override global definition
#
#############################################################

################### FLAGS START #############################

################### Platform Specific #######################

#Flash Types
NOR = 0
NAND = 0
SPI = 0
MMC = 0
#HSI = 1 HSI_HOST, = 2 HSI_CLIENT
#HSI = 2
HSI = 0
DMA = 0

#USB types
I2C = 0
PM8607 = 0
PM8609 = 0
OLED_SUPPORT = 0
SSD1306 = 0
SH1106 = 0
LED_DISPLAY = 0
USBCI = 1
CI2_USB_DDR_BUF = 1

# MMC FLAGS
#MMC_SDMA_MODE = 1
MMC_SDMA_MODE = 0
MMC_DEBUG = 0

# Crypto and fuse programming module selection (WKNG has none!)
PROTOCOL_JTAG_USED = 0

JTAG_PROTOCOL_OVER_JTAG_SUPPORTED = 0

# Hardware related flags
BOARD = JASPER

################### FLAGS END ################################

CFLAGS += -DHASH_VERIFICATION=$(HASH_VERIFICATION)

PLATFORMASMFLAGS = -defsym DDRBASE=1 -defsym ISRAMBASE=0


##############################################################
#
#  Library Selection
#  	- selected by RVCT_BUILD define, and ARM_MODE possibly
#
##############################################################
ifeq "$(USBCI)" "1"
LIBS +=	"$(TOPDIR)/Common/Download/USB2CI/USB2CI_LINUX_ARM.a"
endif


LDCMD = -T $(TOPDIR)/Loader/Platforms/$(PLATFORM)/startup.lds

##############################################################
#
#  Configure Board Name Macro
#
##############################################################
ifeq "$(BOARD)" "Jasper"
BOARDDEF = -DJASPER=1
endif
ifeq "$(BOARD)" "EVBII"
BOARDDEF = -DEVBII=1
endif
ifeq "$(BOARD)" "WAYLAND"
BOARDDEF = -DWAYLAND=1
endif

#############################################################
#
# Platform Object Definitions
#
#############################################################
#MMC
SDHCOBJS = 	sdhc2_controller.o \
			sdhc2.o
SDMMCOBJS = sdmmc_api.o

NANDOBJS = 	xllp_dfc.o \
           	xllp_dfc_support.o \
			nand.o

HSIOBJS = HSI.o

DMAOBJS = 	dma.o

FLASHAPIOBJS = 	Flash.o FM.o

XIPOBJS = xip.o

SPIOBJS = spi.o macronix.o giga.o

DOWNLOADOBJS = ProtocolManager.o \
      	usb_descriptors.o \
        CI2Download.o \
        CI2Driver.o
#      	UartDownload.o \
#      	uart.o \

MAINOBJS = BootLoader.o \
		   BootMode.o \
		   DownloadMode.o \
		   TIMDownload.o \
		   serial.o \
		   FreqChange.o \
		   wdt.o \
		   qpress.o \
		   quicklz.o

DECOMPRESSOBJS = LzmaDecode.o\
				 mpu.o

TIMOBJS =  tim.o

MISCOBJS = misc.o \
		timer.o \
        RegInstructions.o \
		keypad.o

I2COBJS = I2C.o \
		charger.o

SDRAMOBJS = DDR_Cfg.o 
#          sdram_support.o


PLATFORMOBJS = PlatformConfig.o	\
           platform_interrupts.o 
#            sdram_config.o
#           freq_config.o \

ASSEMBLYOBJS = bl_StartUp_ttc_linux.o \
	           platform_arch_linux.o \
		   platform_StartUp_linux.o

ifeq "$(I2C)" "1"
ASSEMBLYOBJS += bbu_PI2C_linux.o
endif

ifeq "$(ZIMI_PB05)" "1"
ASSEMBLYOBJS += bbu_CI2C_linux.o
endif

ifeq "$(OLED_SUPPORT)" "1"
ASSEMBLYOBJS += bbu_CI2C_linux.o
I2COBJS += oled.o oled_lib.o
endif

ifeq "$(LED_DISPLAY)" "1"
I2COBJS += led.o
endif

#####################################################
################ OBJECT LIST ####################
#
# Note: The list below is where to capture what files
#		are to be built into a particular build
#
#####################################################
OBJS = $(ASSEMBLYOBJS)\
	$(MISCOBJS) \
	$(FLASHAPIOBJS) \
	$(DMAOBJS) \
	$(MAINOBJS) \
	$(PLATFORMOBJS) \
	$(DOWNLOADOBJS) \
	$(SDRAMOBJS) \
	$(TIMOBJS)

#OBJS += $(SECUREOBJS)

ifeq "$(DECOMPRESS_SUPPORT)" "1"
MAINOBJS += $(DECOMPRESSOBJS)
ASSEMBLYOBJS += mpu_linux.o
endif

ifeq "$(ZIMI_PB05)" "1"
OBJS += zimi_bq24295.o
endif

ifeq "$(NOR)" "1"
OBJS += $(XIPOBJS)
endif

ifeq "$(NAND)" "1"
OBJS += $(NANDOBJS)
endif
ifeq "$(I2C)" "1"
OBJS += $(I2COBJS)
endif

ifeq "$(SPI)" "1"
OBJS += $(SPIOBJS)
endif

ifeq "$(MMC)" "1"
OBJS += $(SDMMCOBJS) $(SDHCOBJS)
endif

ifneq "$(HSI)" "0"
OBJS += $(HSIOBJS)
endif

ifeq "$(SANREMO)" "1"
OBJS += sanremo.o
endif

ifeq "$(USTICA)" "1"
OBJS += ustica.o
endif

ifeq "$(FAN540X)" "1"
OBJS += fan540x.o
endif

ifeq "$(RT9455)" "1"
OBJS += rt9455.o
endif

ifeq "$(RT8973)" "1"
ASSEMBLYOBJS += bbu_CI2C_linux.o
OBJS += rt8973.o
endif

ifeq "$(TR069_SUPPORT)" "1"
MISCOBJS += tr069.o
endif

ifeq "$(SSD1306)" "1"
OBJS += ssd1306.o
endif

ifeq "$(SH1106)" "1"
OBJS += sh1106.o
endif

