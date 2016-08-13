ifeq ($(WIFI_MODE),)
RT28xx_MODE = STA
else
RT28xx_MODE = $(WIFI_MODE)
endif

ifeq ($(TARGET),)
TARGET = LINUX
endif

# CHIPSET
# rt2860, rt2870, rt2880, rt2070, rt3070, rt3090, rt3572, rt3062, rt3562, rt3593, rt3573
# rt3562(for rt3592), rt3050, rt3350, rt3352, rt5350, rt5370, rt5390, rt5572, rt5592,
# rt8592(for rt85592),
# mt7601e, mt7601u,
# mt7620,
# mt7650e, mt7630e, mt7610e, mt7650u, mt7630u, mt7610u
# mt7662e, mt7632e, mt7612e, mt7662u, mt7632u, mt7612u

ifeq ($(CHIPSET),)
CHIPSET = mt7662u mt7632u mt7612u
endif

MODULE = $(word 1, $(CHIPSET))

#OS ABL - YES or NO
OSABL = NO

#RT28xx_DIR = home directory of RT28xx source code
RT28xx_DIR = $(shell pwd)

include $(RT28xx_DIR)/os/linux/config.mk

RTMP_SRC_DIR = $(RT28xx_DIR)/RT$(MODULE)

#PLATFORM: Target platform
PLATFORM = PC
#PLATFORM = 5VT
#PLATFORM = IKANOS_V160
#PLATFORM = IKANOS_V180
#PLATFORM = SIGMA
#PLATFORM = SIGMA_8622
#PLATFORM = INIC
#PLATFORM = STAR
#PLATFORM = IXP
#PLATFORM = INF_TWINPASS
#PLATFORM = INF_DANUBE
#PLATFORM = INF_AR9
#PLATFORM = INF_VR9
#PLATFORM = BRCM_6358
#PLATFORM = INF_AMAZON_SE
#PLATFORM = CAVM_OCTEON
#PLATFORM = CMPC
#PLATFORM = RALINK_2880
#PLATFORM = RALINK_3052
#PLATFORM = SMDK
#PLATFORM = RMI
#PLATFORM = RMI_64
#PLATFORM = KODAK_DC
#PLATFORM = DM6446
#PLATFORM = FREESCALE8377
#PLATFORM = BL2348
#PLATFORM = BL23570
#PLATFORM = BLUBB
#PLATFORM = BLPMP
#PLATFORM = MT85XX
#PLATFORM = NXP_TV550
#PLATFORM = MVL5
#PLATFORM = RALINK_3352
#PLATFORM = UBICOM_IPX8
#PLATFORM = INTELP6

#APSOC
ifeq ($(MODULE),3050)
PLATFORM = RALINK_3050
endif
ifeq ($(MODULE),3052)
PLATFORM = RALINK_3052
endif
ifeq ($(MODULE),3350)
PLATFORM = RALINK_3050
endif
ifeq ($(MODULE),3352)
PLATFORM = RALINK_3352
endif


#RELEASE Package
RELEASE = DPOA


ifeq ($(TARGET),LINUX)
MAKE = make
endif

ifeq ($(PLATFORM),PC)
# Linux 2.6
LINUX_SRC = /lib/modules/$(shell uname -r)/build
# Linux 2.4 Change to your local setting
#LINUX_SRC = /usr/src/linux-2.4
LINUX_SRC_MODULE = /lib/modules/$(shell uname -r)/kernel/drivers/net/wireless/
CROSS_COMPILE =
endif

export OSABL RT28xx_DIR RT28xx_MODE LINUX_SRC CROSS_COMPILE CROSS_COMPILE_INCLUDE PLATFORM RELEASE CHIPSET MODULE RTMP_SRC_DIR LINUX_SRC_MODULE TARGET HAS_WOW_SUPPORT

# The targets that may be used.
PHONY += all build_tools test UCOS THREADX LINUX release prerelease clean uninstall install libwapi osabl

ifeq ($(TARGET),LINUX)
all: build_tools $(TARGET)
else
all: $(TARGET)
endif



build_tools:
	$(MAKE) -C tools
	$(RT28xx_DIR)/tools/bin2h

test:
	$(MAKE) -C tools test

UCOS:
	$(MAKE) -C os/ucos/ MODE=$(RT28xx_MODE)
	echo $(RT28xx_MODE)

ECOS:
	$(MAKE) -C os/ecos/ MODE=$(RT28xx_MODE)
	cp -f os/ecos/$(MODULE) $(MODULE)

THREADX:
	$(MAKE) -C $(RT28xx_DIR)/os/Threadx -f $(RT28xx_DIR)/os/ThreadX/Makefile

LINUX:
ifeq ($(OSABL),YES)
	cp -f os/linux/Makefile.6.util $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
endif

	cp -f os/linux/Makefile.6 $(RT28xx_DIR)/os/linux/Makefile
ifeq ($(PLATFORM),DM6446)
	$(MAKE)  ARCH=arm CROSS_COMPILE=arm_v5t_le- -C  $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
else
ifeq ($(PLATFORM),FREESCALE8377)
	$(MAKE) ARCH=powerpc CROSS_COMPILE=$(CROSS_COMPILE) -C  $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
else
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
endif
endif

ifeq ($(OSABL),YES)
	cp -f os/linux/Makefile.6.netif $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
endif

clean:
ifeq ($(TARGET), LINUX)
	cp -f os/linux/Makefile.clean os/linux/Makefile
	$(MAKE) -C os/linux clean
	rm -rf os/linux/Makefile
endif

uninstall:
ifeq ($(TARGET), LINUX)
	$(MAKE) -C $(RT28xx_DIR)/os/linux -f Makefile.6 uninstall
endif

install:
ifeq ($(TARGET), LINUX)
	$(MAKE) -C $(RT28xx_DIR)/os/linux -f Makefile.6 install
endif

libwapi:
	cp -f os/linux/Makefile.libwapi.6 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C  $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules

osnet:
ifeq ($(OSABL),YES)
	cp -f os/linux/Makefile.6.netif $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
endif

osdrv:
	cp -f os/linux/Makefile.6 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules

# Declare the contents of the .PHONY variable as phony.  We keep that information in a variable
.PHONY: $(PHONY)



