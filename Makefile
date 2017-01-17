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
# mt7620,
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

#Support Net interface block while Tx-Sw queue full
HAS_BLOCK_NET_IF=n

# Support ED CCA monitor
HAS_ED_MONITOR_SUPPORT=n

# Support for Multiple Cards
HAS_MC_SUPPORT=n

#Support for PCI-MSI
HAS_MSI_SUPPORT=n

#Support features of 802.11n Draft3
HAS_DOT11N_DRAFT3_SUPPORT=y

#Support features of 802.11n
HAS_DOT11_N_SUPPORT=y

#Support for 802.11ac VHT
HAS_DOT11_VHT_SUPPORT=y



#Support for 2860/2880 co-exist
HAS_RT2880_RT2860_COEXIST=n

HAS_KTHREAD_SUPPORT=n







#Support statistics count
HAS_STATS_COUNT=y

#Support TSSI Antenna Variation
HAS_TSSI_ANTENNA_VARIATION=n

#Support USB_BULK_BUF_ALIGMENT
HAS_USB_BULK_BUF_ALIGMENT=n

#Support for USB_SUPPORT_SELECTIVE_SUSPEND
HAS_USB_SUPPORT_SELECTIVE_SUSPEND=n

#Support USB load firmware by multibyte
HAS_USB_FIRMWARE_MULTIBYTE_WRITE=n


#HAS_IFUP_IN_PROBE_SUPPORT
HAS_IFUP_IN_PROBE_SUPPORT=n

#Support for dot11w Protected Management Frame
HAS_DOT11W_PMF_SUPPORT=n

#Support for Bridge Fast Path & Bridge Fast Path function open to other module
HAS_BGFP_SUPPORT=n
HAS_BGFP_OPEN_SUPPORT=n

# Support HOSTAPD function
HAS_HOSTAPD_SUPPORT=n

#Support GreenAP function
HAS_GREENAP_SUPPORT=n

#Support cfg80211 function with Linux Only.
#Please make sure insmod the cfg80211.ko before our driver,
#our driver references to its symbol.
HAS_CFG80211_SUPPORT=y
#smooth the scan signal for cfg80211 based driver
HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT=y
#Cfg80211-based P2P Support
HAS_CFG80211_P2P_SUPPORT=n
#Cfg80211-based P2P Mode Selection (must one be chosen)
HAS_CFG80211_P2P_CONCURRENT_DEVICE=n
HAS_CFG80211_P2P_SINGLE_DEVICE=n
HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE=n

HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT=n
# Support NEW TDLS using supplicant
HAS_CFG80211_TDLS_SUPPORT=n

#For android wifi priv-lib (cfg80211_based wpa_supplicant cmd expansion)
HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT=n

#Support RFKILL hardware block/unblock LINUX-only function
HAS_RFKILL_HW_SUPPORT=n



HAS_MT76XX_BT_COEXISTENCE_SUPPORT=n

HAS_TEMPERATURE_TX_ALC=n

HAS_RTMP_FLASH_SUPPORT=n

#Support WIDI feature
#Must enable HAS_WSC at the same time.

HAS_TXBF_SUPPORT=y
HAS_VHT_TXBF_SUPPORT=y

HAS_STREAM_MODE_SUPPORT=n

HAS_NEW_RATE_ADAPT_SUPPORT=n

#MT7601
HAS_RX_CSO_SUPPORT=y


HAS_WOW_SUPPORT=n
HAS_WOW_IFDOWN_SUPPORT=n
HAS_NEW_WOW_SUPPORT=n

HAS_ANDES_FIRMWARE_SUPPORT=n

HAS_HDR_TRANS_SUPPORT=n


HAS_MICROWAVE_OVEN_SUPPORT=n


HAS_SWITCH_CHANNEL_OFFLOAD=n

HAS_RESOURCE_PRE_ALLOC=n
HAS_RESOURCE_BOOT_ALLOC=n





HAS_CAL_FREE_IC_SUPPORT=y

#################################################

WFLAGS := -g -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX -Wall -Wstrict-prototypes -Wno-trigraphs -DENHANCED_STAT_DISPLAY
WFLAGS += -DSYSTEM_LOG_SUPPORT -DRT28xx_MODE=$(RT28xx_MODE) -DCHIPSET=$(MODULE) -DRESOURCE_PRE_ALLOC -DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS
#APsoc Specific
WFLAGS += -DCONFIG_RA_NAT_NONE
#end of /* APsoc Specific */

WFLAGS += -I$(RT28xx_DIR)/include





ifeq ($(HAS_KTHREAD_SUPPORT),y)
WFLAGS += -DKTHREAD_SUPPORT
endif

ifeq ($(HAS_RTMP_FLASH_SUPPORT),y)
WFLAGS += -DRTMP_FLASH_SUPPORT
endif

ifeq ($(HAS_STREAM_MODE_SUPPORT),y)
WFLAGS += -DSTREAM_MODE_SUPPORT
endif

ifeq ($(HAS_DOT11_VHT_SUPPORT),y)
WFLAGS += -DDOT11_VHT_AC
endif

ifeq ($(HAS_ANDES_FIRMWARE_SUPPORT),y)
WFLAGS += -DANDES_FIRMWARE_SUPPORT
endif

ifeq ($(HAS_HDR_TRANS_SUPPORT),y)
WFLAGS += -DHDR_TRANS_SUPPORT
endif

ifeq ($(HAS_CAL_FREE_IC_SUPPORT),y)
WFLAGS += -DCAL_FREE_IC_SUPPORT
endif

ifeq ($(HAS_TEMPERATURE_TX_ALC),y)
WFLAGS += -DRTMP_TEMPERATURE_TX_ALC
endif

###############################################################################
#
# config for AP mode
#
###############################################################################


ifeq ($(RT28xx_MODE),AP)
WFLAGS += -DCONFIG_AP_SUPPORT  -DUAPSD_SUPPORT -DMBSS_SUPPORT -DDBG -DDOT1X_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT

ifeq ($(HAS_HOSTAPD_SUPPORT),y)
WFLAGS += -DHOSTAPD_SUPPORT
endif

ifeq ($(HAS_ED_MONITOR_SUPPORT),y)
WFLAGS += -DED_MONITOR
endif

ifeq ($(HAS_DOT11_N_SUPPORT),y)
WFLAGS += -DDOT11_N_SUPPORT

ifeq ($(HAS_DOT11N_DRAFT3_SUPPORT),y)
WFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(HAS_TXBF_SUPPORT),y)
WFLAGS += -DTXBF_SUPPORT
endif

ifeq ($(HAS_VHT_TXBF_SUPPORT),y)
WFLAGS += -DVHT_TXBF_SUPPORT
endif

ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif

ifeq ($(HAS_GREENAP_SUPPORT),y)
WFLAGS += -DGREENAP_SUPPORT
endif

endif


ifeq ($(HAS_STATS_COUNT),y)
WFLAGS += -DSTATS_COUNT_SUPPORT
endif

ifeq ($(HAS_TSSI_ANTENNA_VARIATION),y)
WFLAGS += -DTSSI_ANTENNA_VARIATION
endif

ifeq ($(HAS_USB_BULK_BUF_ALIGMENT),y)
WFLAGS += -DUSB_BULK_BUF_ALIGMENT
endif

ifeq ($(HAS_CFG80211_SUPPORT),y)
WFLAGS += -DRT_CFG80211_SUPPORT -DWPA_SUPPLICANT_SUPPORT
ifeq ($(HAS_RFKILL_HW_SUPPORT),y)
WFLAGS += -DRFKILL_HW_SUPPORT
endif
ifeq ($(HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT),y)
WFLAGS += -DCFG80211_SCAN_SIGNAL_AVG
endif
ifeq ($(HAS_CFG80211_P2P_SUPPORT),y)
WFLAGS += -DRT_CFG80211_P2P_SUPPORT
WFLAGS += -DCONFIG_AP_SUPPORT -DUAPSD_SUPPORT -DMBSS_SUPPORT -DAP_SCAN_SUPPORT
WFLAGS += -DAPCLI_SUPPORT
ifeq ($(HAS_CFG80211_P2P_SINGLE_DEVICE),y)
WFLAGS += -DRT_CFG80211_P2P_SINGLE_DEVICE
else
ifeq ($(HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE),y)
WFLAGS += -DRT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
else
WFLAGS += -DRT_CFG80211_P2P_CONCURRENT_DEVICE
ifeq ($(HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT),y)
WFLAGS += -DRT_CFG80211_P2P_MULTI_CHAN_SUPPORT
endif
endif
endif
endif
ifeq ($(HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT),y)
WFLAGS += -DRT_RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT
endif
ifeq ($(HAS_CFG80211_TDLS_SUPPORT),y)
WFLAGS += -DCFG_TDLS_SUPPORT -DUAPSD_SUPPORT
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

ifeq ($(HAS_MT76XX_BT_COEXISTENCE_SUPPORT),y)
WFLAGS += -DMT76XX_BTCOEX_SUPPORT
endif



ifeq ($(HAS_WPA_SUPPLICANT),y)
WFLAGS += -DWPA_SUPPLICANT_SUPPORT
ifeq ($(HAS_NATIVE_WPA_SUPPLICANT_SUPPORT),y)
WFLAGS += -DNATIVE_WPA_SUPPLICANT_SUPPORT
endif
endif

ifeq ($(HAS_DOT11_N_SUPPORT),y)
WFLAGS += -DDOT11_N_SUPPORT

ifeq ($(HAS_DOT11N_DRAFT3_SUPPORT),y)
WFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(HAS_TXBF_SUPPORT),y)
WFLAGS += -DTXBF_SUPPORT
endif

ifeq ($(HAS_VHT_TXBF_SUPPORT),y)
WFLAGS += -DVHT_TXBF_SUPPORT
endif

ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif

endif

ifeq ($(HAS_STATS_COUNT),y)
WFLAGS += -DSTATS_COUNT_SUPPORT
endif

ifeq ($(HAS_TSSI_ANTENNA_VARIATION),y)
WFLAGS += -DTSSI_ANTENNA_VARIATION
endif


ifeq ($(HAS_IFUP_IN_PROBE_SUPPORT),y)
WFLAGS += -DIFUP_IN_PROBE
endif

ifeq ($(HAS_USB_SUPPORT_SELECTIVE_SUSPEND),y)
WFLAGS += -DUSB_SUPPORT_SELECTIVE_SUSPEND
endif

ifeq ($(HAS_USB_FIRMWARE_MULTIBYTE_WRITE),y)
WFLAGS += -DUSB_FIRMWARE_MULTIBYTE_WRITE -DMULTIWRITE_BYTES=4
endif

ifeq ($(HAS_CFG80211_SUPPORT),y)
WFLAGS += -DRT_CFG80211_SUPPORT -DWPA_SUPPLICANT_SUPPORT
ifeq ($(HAS_RFKILL_HW_SUPPORT),y)
WFLAGS += -DRFKILL_HW_SUPPORT
endif
ifeq ($(HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT),y)
WFLAGS += -DCFG80211_SCAN_SIGNAL_AVG
endif
ifeq ($(HAS_CFG80211_P2P_SUPPORT),y)
WFLAGS += -DRT_CFG80211_P2P_SUPPORT -DUAPSD_SUPPORT -DUAPSD_SP_ACCURATE -DMBSS_SUPPORT -DAP_SCAN_SUPPORT
WFLAGS += -DCONFIG_AP_SUPPORT -DAPCLI_SUPPORT
ifeq ($(HAS_CFG80211_P2P_SINGLE_DEVICE),y)
WFLAGS += -DRT_CFG80211_P2P_SINGLE_DEVICE
else
ifeq ($(HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE),y)
WFLAGS += -DRT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
else
WFLAGS += -DRT_CFG80211_P2P_CONCURRENT_DEVICE
ifeq ($(HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT),y)
WFLAGS += -DRT_CFG80211_P2P_MULTI_CHAN_SUPPORT
endif #HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT
endif #HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
endif #HAS_CFG80211_P2P_SINGLE_DEVICE
endif #HAS_CFG80211_P2P_SUPPORT
ifeq ($(HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT),y)
WFLAGS += -DRT_RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT
endif #HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT
ifeq ($(HAS_CFG80211_TDLS_SUPPORT),y)
WFLAGS += -DCFG_TDLS_SUPPORT -DUAPSD_SUPPORT
endif
endif

ifeq ($(HAS_WIDI_SUPPORT),y)
WFLAGS += -DWIDI_SUPPORT

ifeq ($(HAS_INTEL_L2SD_TOGGLE_SCAN_SUPPORT),y)
WFLAGS += -DINTEL_L2SD_TOGGLE_SCAN_SUPPORT
endif

ifeq ($(HAS_P2P_SUPPORT),y)
ifeq ($(HAS_INTEL_WFD_SUPPORT),y)
WFLAGS += -DINTEL_WFD_SUPPORT
endif

ifeq ($(HAS_WFA_WFD_SUPPORT),y)
WFLAGS += -DWFA_WFD_SUPPORT
endif
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
WFLAGS += -DCONFIG_AP_SUPPORT -DCONFIG_STA_SUPPORT -DCONFIG_APSTA_MIXED_SUPPORT -DUAPSD_SUPPORT -DMBSS_SUPPORT -DDOT1X_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DDBG

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


ifeq ($(HAS_DOT11_N_SUPPORT),y)
WFLAGS += -DDOT11_N_SUPPORT
endif

ifeq ($(HAS_DOT11N_DRAFT3_SUPPORT),y)
WFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(HAS_TXBF_SUPPORT),y)
WFLAGS += -DTXBF_SUPPORT
endif

ifeq ($(HAS_VHT_TXBF_SUPPORT),y)
WFLAGS += -DVHT_TXBF_SUPPORT
endif

ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif

ifeq ($(HAS_STATS_COUNT),y)
WFLAGS += -DSTATS_COUNT_SUPPORT
endif

ifeq ($(HAS_TSSI_ANTENNA_VARIATION),y)
WFLAGS += -DTSSI_ANTENNA_VARIATION
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

ifeq ($(HAS_P2P_SUPPORT),y)
WFLAGS += -DP2P_SUPPORT
WFLAGS += -DAPCLI_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DP2P_APCLI_SUPPORT
endif

endif
# endif of ifeq ($(RT28xx_MODE),APSTA)


##########################################################
#
# Common compiler flag
#
##########################################################

ifeq ($(HAS_BGFP_SUPPORT),y)
WFLAGS += -DBG_FT_SUPPORT
endif

ifeq ($(HAS_BGFP_OPEN_SUPPORT),y)
WFLAGS += -DBG_FT_OPEN_SUPPORT
endif

ifeq ($(HAS_DOT11W_PMF_SUPPORT),y)
WFLAGS += -DDOT11W_PMF_SUPPORT -DSOFT_ENCRYPT
endif




ifeq ($(HAS_MICROWAVE_OVEN_SUPPORT),y)
WFLAGS += -DMICROWAVE_OVEN_SUPPORT
endif

#################################################
# ChipSet specific definitions.
#
#################################################

#MT7620
ifneq ($(findstring mt7620,$(CHIPSET)),)
WFLAGS += -DRTMP_RBUS_SUPPORT -DRTMP_MAC_PCI -DRT6352 -DRTMP_RF_RW_SUPPORT -DRF_BANK -DRTMP_FLASH_SUPPORT -DCONFIG_SWMCU_SUPPORT -DSPECIFIC_BCN_BUF_SUPPORT -DVCORECAL_SUPPORT -DRESOURCE_PRE_ALLOC -DENHANCED_STAT_DISPLAY -DLOFT_IQ_CAL_SUPPORT -DRTMP_TEMPERATURE_CALIBRATION -DDYNAMIC_VGA_SUPPORT -DMCS_LUT_SUPPORT -DPEER_DELBA_TX_ADAPT -DENHANCE_ULTP
endif
## End of MT7620 ##

# MT7650U
ifneq ($(or $(findstring mt7650u,$(CHIPSET)),$(findstring mt7630u,$(CHIPSET)),$(findstring mt7610u,$(CHIPSET))),)
HAS_RLT_BBP=y
HAS_RLT_MAC=y
HAS_RLT_RF=y

WFLAGS += -DMT76x0 -DRT65xx -DRLT_MAC -DRLT_BBP -DRLT_RF -DRTMP_MAC_USB -DRTMP_USB_SUPPORT -DRTMP_TIMER_TASK_SUPPORT
WFLAGS += -DA_BAND_SUPPORT -DRX_DMA_SCATTER -DRTMP_EFUSE_SUPPORT -DNEW_MBSSID_MODE -DCONFIG_ANDES_SUPPORT

ifneq ($(findstring mt7650u,$(CHIPSET)),)
WFLAGS += -DMT7650
endif

ifneq ($(findstring mt7630u,$(CHIPSET)),)
WFLAGS += -DMT7630
endif

ifneq ($(findstring mt7610u,$(CHIPSET)),)
WFLAGS += -DMT7610
endif

ifneq ($(findstring $(RT28xx_MODE),AP),)
#WFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
endif

ifeq ($(HAS_CSO_SUPPORT), y)
WFLAGS += -DCONFIG_CSO_SUPPORT
ifeq ($(HAS_TSO_SUPPORT), y)
WFLAGS += -DCONFIG_TSO_SUPPORT -DTX_PKT_SG
endif
endif

endif
## End of MT7650U ##

# MT7662U
ifneq ($(or $(findstring mt7662u,$(CHIPSET)),$(findstring mt7662u,$(CHIPSET))),)
WFLAGS += -DMT76x2 -DRT65xx -DRLT_MAC -DRLT_BBP -DMT_RF -DRTMP_MAC_USB -DRTMP_USB_SUPPORT -DRTMP_TIMER_TASK_SUPPORT -DA_BAND_SUPPORT -DRTMP_EFUSE_SUPPORT -DNEW_MBSSID_MODE -DCONFIG_ANDES_SUPPORT -DRTMP_RF_RW_SUPPORT -DDYNAMIC_VGA_SUPPORT
HAS_NEW_RATE_ADAPT_SUPPORT=y
ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif
WFLAGS += -DFIFO_EXT_SUPPORT
HAS_RLT_BBP=y
HAS_RLT_MAC=y

ifneq ($(findstring mt7662u,$(CHIPSET)),)
WFLAGS += -DMT7662
endif

ifneq ($(findstring mt7612u,$(CHIPSET)),)
WFLAGS += -DMT7612
endif

ifeq ($(HAS_CSO_SUPPORT), y)
WFLAGS += -DCONFIG_CSO_SUPPORT -DCONFIG_TSO_SUPPORT
endif

ifneq ($(findstring $(RT28xx_MODE),STA APSTA),)
WFLAGS += -DRTMP_FREQ_CALIBRATION_SUPPORT
endif

CHIPSET_DAT = 2870
endif
## End of 7662U ##

#################################################
# Platform Related definitions
#
#################################################

ifeq ($(HAS_BLOCK_NET_IF),y)
WFLAGS += -DBLOCK_NET_IF
endif

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
	common/eeprom.o\
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
	mgmt/mgmt_hw.o\
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

ifeq ($(HAS_RLT_RF),y)
obj_phy += phy/rlt_rf.o
endif

ifeq ($(HAS_MT76XX_BT_COEXISTENCE_SUPPORT),y)
obj_cmm += mcu/bt_coex.o
endif

ifeq ($(HAS_RLT_MAC),y)
obj_mac += mac/ral_nmac.o
endif

obj_cmm += $(obj_phy) $(obj_mac)

ifeq ($(HAS_BLOCK_NET_IF),y)
obj_cmm += common/netif_block.o
endif

ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
obj_cmm += rate_ctrl/alg_grp.o
endif

#ifdef ANDES_FIRMWARE_SUPPORT
ifeq ($(HAS_ANDES_FIRMWARE_SUPPORT),y)
obj_cmm += mcu/rtmp_and.o
endif
#endif /* ANDES_FIRMWARE_SUPPORT */




#ifdef DOT11W_PMF_SUPPORT
ifeq ($(HAS_DOT11W_PMF_SUPPORT),y)
obj_cmm += common/pmf.o
endif
#endif // DOT11W_PMF_SUPPORT //


#ifdef DOT11_N_SUPPORT
ifeq ($(HAS_DOT11_N_SUPPORT),y)
obj_cmm += \
        common/ba_action.o\
        mgmt/mgmt_ht.o

#ifdef TXBF_SUPPORT
ifeq ($(HAS_TXBF_SUPPORT),y)
obj_cmm += \
        common/cmm_txbf.o\
        common/cmm_txbf_cal.o
endif
#endif // TXBF_SUPPORT //
endif
#endif // DOT11_N_SUPPORT //

#ifdef DOT11_VHT_SUPPORT
ifeq ($(HAS_DOT11_VHT_SUPPORT),y)
obj_vht += mgmt/mgmt_vht.o\
	common/vht.o
endif
#endif // DOT11_VHT_SUPPORT //








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

ifeq ($(HAS_WSC),y)
obj_ap += common/wsc_ufd.o
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

#ifdef BG_FT_SUPPORT
ifeq ($(HAS_BGFP_SUPPORT),y)
$(MOD_NAME)-objs += \
	os/linux/br_ftph.o
endif
#endif // BG_FT_SUPPORT //

$(MOD_NAME)-objs += \
	common/rt_os_util.o\
	os/linux/rt_linux.o\
	os/linux/rt_main_dev.o

ifeq ($(HAS_SNIFFER_SUPPORT),y)
$(MOD_NAME)-objs += \
	sniffer/sniffer_prism.o	\
	sniffer/sniffer_radiotap.o
endif

#endif // CRDA_SUPPORT //

ifeq ($(HAS_RT2880_RT2860_COEXIST),y)
RT28XX_AP_OBJ += \
	os/linux/rt_pci_rbus.o\
	os/linux/rt_rbus_pci_util.o\
	os/linux/pci_main_dev.o\
	common/dfs.o
endif

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

ifeq ($(HAS_SNIFFER_SUPPORT),y)
$(MOD_NAME)-objs += \
	sniffer/sniffer_prism.o\
	sniffer/sniffer_radiotap.o
endif

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

ifeq ($(HAS_SNIFFER_SUPPORT),y)
$(MOD_NAME)-objs += \
	sniffer/sniffer_prism.o\
	sniffer/sniffer_radiotap.o
endif

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

ifneq ($(or $(findstring mt7662u,$(CHIPSET)),$(findstring mt7612u,$(CHIPSET))),)
$(MOD_NAME)-objs += \
	common/cmm_mac_usb.o\
	common/cmm_data_usb.o\
	common/rtusb_io.o\
	common/rtusb_data.o\
	common/rtusb_bulk.o\
	os/linux/rt_usb.o\
	common/ee_prom.o\
	common/ee_efuse.o\
	chips/rt65xx.o\
	chips/mt76x2.o\
	mac/ral_nmac.o\
	mcu/mcu.o\
	mcu/mcu_and.o\
	phy/rt_rf.o\
	phy/mt_rf.o

ifeq ($(HAS_RTMP_FLASH_SUPPORT),y)
$(MOD_NAME)-objs += \
	common/ee_flash.o
endif

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

endif
#endif // MT76x2 //

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



