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

PLATFORM = PC

#APSOC

#RELEASE Package
RELEASE = DPOA

MAKE = make

ifeq ($(PLATFORM),PC)
# Linux 2.6
KSRC = /lib/modules/$(shell uname -r)/build
CROSS_COMPILE =
EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN
SUBARCH := $(shell uname -m | sed -e s/i.86/i386/)
ARCH ?= $(SUBARCH)
endif

export OSABL RT28xx_DIR RT28xx_MODE KSRC CROSS_COMPILE CROSS_COMPILE_INCLUDE PLATFORM RELEASE CHIPSET MODULE  KSRC TARGET HAS_WOW_SUPPORT

all: build_tools modules

build_tools:
	$(MAKE) -C tools
	$(RT28xx_DIR)/tools/bin2h

test:
	$(MAKE) -C tools test

modules:
	cp -f os/linux/Makefile.6 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules

clean:
	rm -f common/*.o
	rm -f common/.*.{cmd,flags,d}
	rm -f os/linux/*.{o,ko,mod.{o,c}}
	rm -f os/linux/.*.{cmd,flags,d}
	rm -fr os/linux/.tmp_versions
#Must clean Module.symvers; or you will suffer symbol version not match
#when OS_ABL = YES.
	rm -f os/linux/Module.symvers
	rm -f os/linux/Modules.symvers
	rm -f os/linux/Module.markers
	rm -f os/linux/modules.order
	rm -f chips/*.o
	rm -f chips/.*.{cmd,flags,d}
	rm -f ap/*.o
	rm -f ap/.*.{cmd,flags,d}
	rm -f sta/*.o
	rm -f sta/.*.{cmd,flags,d}

# Declare the contents of the .PHONY variable as phony.  We keep that information in a variable
.PHONY: $(PHONY)



