ifeq ($(WIFI_MODE),)
RT28xx_MODE = APSTA
else
RT28xx_MODE = $(WIFI_MODE)
endif

# CHIPSET
# rt2860, rt2870, rt2880, rt2070, rt3070, rt3090, rt3572, rt3062, rt3562, rt3593, rt3573
# rt3562(for rt3592), rt3050, rt3350, rt3352, rt5350, rt5370, rt5390, rt5572, rt5592,
# rt8592(for rt85592),
# mt7601e, mt7601u,
# mt7650e, mt7630e, mt7610e, mt7650u, mt7630u, mt7610u
# mt7662e, mt7632e, mt7612e, mt7662u, mt7632u, mt7612u

ifeq ($(CHIPSET),)
CHIPSET = mt7612u mt7662u mt7632u
endif

MODULE = $(word 1, $(CHIPSET))

# Support ATE function
HAS_ATE=y

# Support Wpa_Supplicant
# i.e. wpa_supplicant -Dralink
HAS_WPA_SUPPLICANT=y


# Support Native WpaSupplicant for Network Maganger
# i.e. wpa_supplicant -Dwext
HAS_NATIVE_WPA_SUPPLICANT_SUPPORT=n

# Support for Multiple Cards
HAS_MC_SUPPORT=n

#Support for PCI-MSI
HAS_MSI_SUPPORT=n

HAS_KTHREAD_SUPPORT=n







#Support statistics count
HAS_STATS_COUNT=y

#Support for dot11w Protected Management Frame
HAS_DOT11W_PMF_SUPPORT=n

# Support HOSTAPD function
HAS_HOSTAPD_SUPPORT=n

#Support cfg80211 function with Linux Only.
#Please make sure insmod the cfg80211.ko before our driver,
#our driver references to its symbol.
HAS_CFG80211_SUPPORT=y
#smooth the scan signal for cfg80211 based driver
HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT=y
#Cfg80211-based P2P Support
#Cfg80211-based P2P Mode Selection (must one be chosen)
HAS_CFG80211_P2P_CONCURRENT_DEVICE=n
HAS_CFG80211_P2P_SINGLE_DEVICE=n
HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE=n

HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT=n

#Support RFKILL hardware block/unblock LINUX-only function
HAS_RFKILL_HW_SUPPORT=n

HAS_TEMPERATURE_TX_ALC=n

HAS_NEW_RATE_ADAPT_SUPPORT=n

#MT7601
HAS_RX_CSO_SUPPORT=y


HAS_WOW_SUPPORT=n
HAS_WOW_IFDOWN_SUPPORT=n
HAS_NEW_WOW_SUPPORT=n

HAS_SWITCH_CHANNEL_OFFLOAD=n

HAS_RESOURCE_BOOT_ALLOC=n





HAS_CAL_FREE_IC_SUPPORT=y

#################################################

WFLAGS := -g -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX -Wall -Wstrict-prototypes -Wno-trigraphs
WFLAGS += -DSYSTEM_LOG_SUPPORT -DRT28xx_MODE=$(RT28xx_MODE) -DCHIPSET=$(MODULE) -DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS
#APsoc Specific
WFLAGS += -DCONFIG_RA_NAT_NONE
#end of /* APsoc Specific */

WFLAGS += -I$(RT28xx_DIR)/include





ifeq ($(HAS_KTHREAD_SUPPORT),y)
WFLAGS += -DKTHREAD_SUPPORT
endif

ifeq ($(HAS_CAL_FREE_IC_SUPPORT),y)
WFLAGS += -DCAL_FREE_IC_SUPPORT
endif

###############################################################################
#
# config for AP mode
#
###############################################################################


ifeq ($(RT28xx_MODE),AP)
WFLAGS += -DCONFIG_AP_SUPPORT -DMBSS_SUPPORT -DDBG -DDOT1X_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT

ifeq ($(HAS_HOSTAPD_SUPPORT),y)
WFLAGS += -DHOSTAPD_SUPPORT
endif

ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif

ifeq ($(HAS_STATS_COUNT),y)
WFLAGS += -DSTATS_COUNT_SUPPORT
endif

ifeq ($(HAS_CFG80211_SUPPORT),y)
WFLAGS += -DRT_CFG80211_SUPPORT -DWPA_SUPPLICANT_SUPPORT
ifeq ($(HAS_RFKILL_HW_SUPPORT),y)
WFLAGS += -DRFKILL_HW_SUPPORT
endif
ifeq ($(HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT),y)
WFLAGS += -DCFG80211_SCAN_SIGNAL_AVG
endif
endif

endif #// endif of RT2860_MODE == AP //

########################################################
#
# config for STA mode
#
########################################################


ifeq ($(RT28xx_MODE),STA)
WFLAGS += -DCONFIG_STA_SUPPORT -DSCAN_SUPPORT -DDBG

ifeq ($(HAS_WPA_SUPPLICANT),y)
WFLAGS += -DWPA_SUPPLICANT_SUPPORT
ifeq ($(HAS_NATIVE_WPA_SUPPLICANT_SUPPORT),y)
WFLAGS += -DNATIVE_WPA_SUPPLICANT_SUPPORT
endif
endif

ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif

ifeq ($(HAS_STATS_COUNT),y)
WFLAGS += -DSTATS_COUNT_SUPPORT
endif

ifeq ($(HAS_CFG80211_SUPPORT),y)
WFLAGS += -DRT_CFG80211_SUPPORT -DWPA_SUPPLICANT_SUPPORT
ifeq ($(HAS_RFKILL_HW_SUPPORT),y)
WFLAGS += -DRFKILL_HW_SUPPORT
endif
ifeq ($(HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT),y)
WFLAGS += -DCFG80211_SCAN_SIGNAL_AVG
endif
endif

ifeq ($(HAS_WIDI_SUPPORT),y)
WFLAGS += -DWIDI_SUPPORT

ifeq ($(HAS_INTEL_L2SD_TOGGLE_SCAN_SUPPORT),y)
WFLAGS += -DINTEL_L2SD_TOGGLE_SCAN_SUPPORT
endif

endif

ifeq ($(HAS_WOW_SUPPORT),y)
WFLAGS += -DWOW_SUPPORT
endif

ifeq ($(HAS_WOW_IFDOWN_SUPPORT),y)
WFLAGS += -DWOW_IFDOWN_SUPPORT
endif

ifeq ($(HAS_NEW_WOW_SUPPORT),y)
WFLAGS += -DNEW_WOW_SUPPORT
endif

endif
# endif of ifeq ($(RT28xx_MODE),STA)

###########################################################
#
# config for APSTA
#
###########################################################


ifeq ($(RT28xx_MODE),APSTA)
WFLAGS += -DCONFIG_AP_SUPPORT -DCONFIG_STA_SUPPORT -DCONFIG_APSTA_MIXED_SUPPORT -DMBSS_SUPPORT -DDOT1X_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DDBG

ifeq ($(HAS_IGMP_SNOOP_SUPPORT),y)
WFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(HAS_QOS_DLS_SUPPORT),y)
WFLAGS += -DQOS_DLS_SUPPORT
endif

ifeq ($(HAS_WPA_SUPPLICANT),y)
WFLAGS += -DWPA_SUPPLICANT_SUPPORT
ifeq ($(HAS_NATIVE_WPA_SUPPLICANT_SUPPORT),y)
WFLAGS += -DNATIVE_WPA_SUPPLICANT_SUPPORT
endif
endif


ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif

ifeq ($(HAS_STATS_COUNT),y)
WFLAGS += -DSTATS_COUNT_SUPPORT
endif

ifeq ($(HAS_CFG80211_SUPPORT),y)
WFLAGS += -DRT_CFG80211_SUPPORT -DWPA_SUPPLICANT_SUPPORT
ifeq ($(HAS_RFKILL_HW_SUPPORT),y)
WFLAGS += -DRFKILL_HW_SUPPORT
endif
ifeq ($(HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT),y)
WFLAGS += -DCFG80211_SCAN_SIGNAL_AVG
endif
endif

endif
# endif of ifeq ($(RT28xx_MODE),APSTA)


##########################################################
#
# Common compiler flag
#
##########################################################

ifeq ($(HAS_DOT11W_PMF_SUPPORT),y)
WFLAGS += -DDOT11W_PMF_SUPPORT -DSOFT_ENCRYPT
endif

#################################################
# ChipSet specific definitions.
#
#################################################

WFLAGS += -DMT76x2 -DRLT_MAC -DRLT_BBP -DMT_RF -DRTMP_MAC_USB -DRTMP_USB_SUPPORT -DRTMP_TIMER_TASK_SUPPORT -DRTMP_EFUSE_SUPPORT -DNEW_MBSSID_MODE -DCONFIG_ANDES_SUPPORT -DRTMP_RF_RW_SUPPORT -DDYNAMIC_VGA_SUPPORT
HAS_NEW_RATE_ADAPT_SUPPORT=y
ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif
WFLAGS += -DFIFO_EXT_SUPPORT
HAS_RLT_BBP=y
HAS_RLT_MAC=y

ifeq ($(HAS_CSO_SUPPORT), y)
WFLAGS += -DCONFIG_CSO_SUPPORT -DCONFIG_TSO_SUPPORT
endif

ifneq ($(findstring $(RT28xx_MODE),STA APSTA),)
WFLAGS += -DRTMP_FREQ_CALIBRATION_SUPPORT
endif

#################################################
# Platform Related definitions
#
#################################################

EXTRA_CFLAGS := $(WFLAGS)

#RT28xx_DIR = home directory of RT28xx source code
RT28xx_DIR = $(shell pwd)

PLATFORM = PC

#APSOC

#RELEASE Package
RELEASE = DPOA

obj_ap :=
obj_sta :=
obj_p2p :=
obj_wsc :=
obj_vht :=
obj_cmm := \
	common/crypt_md5.o\
	common/crypt_sha2.o\
	common/crypt_hmac.o\
	common/crypt_aes.o\
	common/crypt_arc4.o\
	common/mlme.o\
	common/cmm_wep.o\
	common/action.o\
	common/cmm_data.o\
	common/rtmp_init.o\
	common/rtmp_init_inf.o\
	common/cmm_tkip.o\
	common/cmm_aes.o\
	common/cmm_sync.o\
	common/cmm_sanity.o\
	common/cmm_info.o\
	common/cmm_cfg.o\
	common/cmm_wpa.o\
	common/cmm_radar.o\
	common/spectrum.o\
	common/rtmp_timer.o\
	common/rt_channel.o\
	common/cmm_profile.o\
	common/cmm_asic.o\
	common/scan.o\
	common/cmm_cmd.o\
	common/uapsd.o\
	common/ps.o\
	common/sys_log.o\
	common/txpower.o\
	rate_ctrl/ra_ctrl.o\
	rate_ctrl/alg_legacy.o\
	chips/rtmp_chip.o\
	mgmt/mgmt_entrytb.o\
	tx_rx/wdev_tx.o \
	os/linux/rt_profile.o


obj_phy := phy/phy.o	\
	   phy/rf.o

obj_mac := mac/rtmp_mac.o

ifeq ($(HAS_RTMP_BBP),y)
obj_phy += phy/rtmp_phy.o
endif

ifeq ($(HAS_RLT_BBP),y)
obj_phy += phy/rlt_phy.o
endif

ifeq ($(HAS_RLT_MAC),y)
obj_mac += mac/ral_nmac.o
endif

obj_cmm += $(obj_phy) $(obj_mac)

ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
obj_cmm += rate_ctrl/alg_grp.o
endif

#ifdef DOT11W_PMF_SUPPORT
ifeq ($(HAS_DOT11W_PMF_SUPPORT),y)
obj_cmm += common/pmf.o
endif
#endif // DOT11W_PMF_SUPPORT //


#ifdef DOT11_N_SUPPORT
obj_cmm += \
        common/ba_action.o\
        mgmt/mgmt_ht.o

obj_cmm += 	common/cmm_txbf.o\
		common/cmm_txbf_cal.o

obj_vht += mgmt/mgmt_vht.o\
	common/vht.o








###############################################################################
#
# config for AP mode
#
###############################################################################

obj_ap += \
	ap/ap_mbss.o\
	ap/ap.o\
	ap/ap_assoc.o\
	ap/ap_auth.o\
	ap/ap_connect.o\
	ap/ap_mlme.o\
	ap/ap_sanity.o\
	ap/ap_sync.o\
	ap/ap_wpa.o\
	ap/ap_data.o\
	ap/ap_autoChSel.o\
	ap/ap_qload.o\
	ap/ap_cfg.o

ifeq ($(HAS_QOS_DLS_SUPPORT),y)
obj_ap += ap/ap_dls.o
endif

ifeq ($(HAS_IDS_SUPPORT),y)
obj_ap += ap/ap_ids.o
endif

obj_ap += \
	ap/ap_mbss_inf.o\
	os/linux/ap_ioctl.o

ifeq ($(HAS_IGMP_SNOOP_SUPPORT),y)
obj_ap += common/igmp_snoop.o
endif

MOD_NAME = $(MODULE)

###############################################################################
#
# config for STA mode
#
###############################################################################

obj_sta += \
	sta/assoc.o\
	sta/auth.o\
	sta/auth_rsp.o\
	sta/sync.o\
	sta/sanity.o\
	sta/rtmp_data.o\
	sta/connect.o\
	sta/wpa.o\
	sta/sta_cfg.o\
	sta/sta.o


obj_sta += os/linux/sta_ioctl.o

MOD_NAME = $(MODULE)


###############################################################################
#
# config for AP/STA mixed mode
#
###############################################################################

MOD_NAME = $(MODULE)

###############################################################################
#
# Module Base
#
###############################################################################

obj-m := $(MOD_NAME).o

#ifdef CONFIG_AP_SUPPORT
ifeq ($(RT28xx_MODE),AP)

$(MOD_NAME)-objs := \
	$(obj_ap)\
	$(obj_vht)\
	$(obj_cmm)\
	$(obj_wsc)\
	$(obj_phy)

$(MOD_NAME)-objs += \
	common/rt_os_util.o\
	os/linux/rt_linux.o\
	os/linux/rt_main_dev.o

#endif // CRDA_SUPPORT //

endif
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
ifeq ($(RT28xx_MODE), STA)

$(MOD_NAME)-objs := \
	$(obj_sta)\
	$(obj_p2p)\
	$(obj_vht)\
	$(obj_cmm)\
	$(obj_wsc)\
	$(obj_phy)

$(MOD_NAME)-objs += \
	common/rt_os_util.o\
	os/linux/sta_ioctl.o\
	os/linux/rt_linux.o\
	os/linux/rt_main_dev.o

#ifdef ETH_CONVERT
ifeq ($(HAS_ETH_CONVERT_SUPPORT), y)
$(MOD_NAME)-objs += \
	common/cmm_mat.o \
	common/cmm_mat_iparp.o \
	common/cmm_mat_pppoe.o \
	common/cmm_mat_ipv6.o
endif
#endif // ETH_CONVERT //




ifeq ($(HAS_QOS_DLS_SUPPORT),y)
$(MOD_NAME)-objs += sta/dls.o
endif

endif
#endif // CONFIG_STA_SUPPORT //


#ifdef CRDA_SUPPORT
ifeq ($(HAS_CFG80211_SUPPORT),y)
$(MOD_NAME)-objs += \
	os/linux/cfg80211/cfg80211.o\
	os/linux/cfg80211/cfg80211_util.o\
	os/linux/cfg80211/cfg80211_scan.o\
	os/linux/cfg80211/cfg80211_rx.o\
	os/linux/cfg80211/cfg80211_tx.o\
	os/linux/cfg80211/cfg80211_inf.o\
	os/linux/cfg80211/cfg80211_p2p.o\
	os/linux/cfg80211/cfg80211_ap.o\
	os/linux/cfg80211/cfg80211drv.o
endif

#ifdef CONFIG_APSTA_SUPPORT
ifeq ($(RT28xx_MODE), APSTA)
$(MOD_NAME)-objs := \
	$(obj_ap)\
	$(obj_sta)\
	$(obj_p2p)\
	$(obj_vht)\
	$(obj_cmm)\
	$(obj_wsc)\
	$(obj_phy)

$(MOD_NAME)-objs += \
	common/rt_os_util.o\
	os/linux/sta_ioctl.o\
	os/linux/rt_linux.o\
	os/linux/rt_main_dev.o

#ifdef ETH_CONVERT
ifeq ($(HAS_ETH_CONVERT_SUPPORT), y)
$(MOD_NAME)-objs += \
	common/cmm_mat.o \
	common/cmm_mat_iparp.o \
	common/cmm_mat_pppoe.o \
	common/cmm_mat_ipv6.o
endif
#endif // ETH_CONVERT //




ifeq ($(HAS_QOS_DLS_SUPPORT),y)
$(MOD_NAME)-objs += sta/dls.o
endif

#ifdef CRDA_SUPPORT
ifeq ($(HAS_CFG80211_SUPPORT),y)
$(MOD_NAME)-objs += \
	os/linux/cfg80211/cfg80211.o\
	os/linux/cfg80211/cfg80211_util.o\
	os/linux/cfg80211/cfg80211_scan.o\
	os/linux/cfg80211/cfg80211_rx.o\
	os/linux/cfg80211/cfg80211_tx.o\
	os/linux/cfg80211/cfg80211_inf.o\
	os/linux/cfg80211/cfg80211_p2p.o\
	os/linux/cfg80211/cfg80211_ap.o\
	os/linux/cfg80211/cfg80211drv.o
endif

endif
#endif // CONFIG_APSTA_SUPPORT //


#chip releated

$(MOD_NAME)-objs += \
	common/cmm_mac_usb.o\
	common/cmm_data_usb.o\
	common/rtusb_io.o\
	common/rtusb_data.o\
	common/rtusb_bulk.o\
	os/linux/rt_usb.o\
	chips/rt65xx.o\
	chips/mt76x2.o\
	mac/ral_nmac.o\
	mcu/mcu.o\
	mcu/mcu_and.o\
	phy/rt_rf.o\
	phy/mt_rf.o

ifeq ($(HAS_TSO_SUPPORT),y)
$(MOD_NAME)-objs += \
	naf/net_acc.o\
	naf/cso.o
endif

ifeq ($(HAS_CSO_SUPPORT), y)
$(MOD_NAME)-objs += \
	naf/net_acc.o\
	naf/cso.o
endif

$(MOD_NAME)-objs += \
	os/linux/rt_usb.o\
	os/linux/rt_usb_util.o\
	os/linux/usb_main_dev.o\
	common/rtusb_dev_id.o

ifneq ($(findstring $(RT28xx_MODE),STA APSTA),)
$(MOD_NAME)-objs += \
        common/frq_cal.o
endif


# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)



MAKE = make

ifeq ($(PLATFORM),PC)
# Linux 2.6
KSRC = /lib/modules/$(shell uname -r)/build
CROSS_COMPILE =
EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN
EXTRA_CFLAGS += -Wno-unused
SUBARCH := $(shell uname -m | sed -e s/i.86/i386/)
ARCH ?= $(SUBARCH)
endif

export RT28xx_DIR RT28xx_MODE KSRC CROSS_COMPILE CROSS_COMPILE_INCLUDE PLATFORM RELEASE CHIPSET MODULE  KSRC HAS_WOW_SUPPORT

all: modules

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(PWD) modules

clean:
	rm -f */*.o
	rm -f */.*.{cmd,flags,d}
	rm -f *.{o,ko,mod.{o,c}}
	rm -f */*/*.{o,ko,mod.{o,c}}
	rm -f */*/.*.{cmd,flags,d}
	rm -f */*/*/*.{o,ko,mod.{o,c}}
	rm -f */*/*/.*.{cmd,flags,d}
	rm -fr .tmp_versions
	rm -f Module.symvers
	rm -f Modules.symvers
	rm -f Module.markers
	rm -f modules.order

installfw:
	cp -n firmware/* /lib/firmware

help:
	@echo "options :"
	@echo "modules		build this module"
	@echo "installfw	install firmware file"
	@echo "clean		clean"
	@echo "help		this help text"

# Declare the contents of the .PHONY variable as phony.  We keep that information in a variable
.PHONY: $(PHONY)



