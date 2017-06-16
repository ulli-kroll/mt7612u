#ifndef __MT76X2_H__
#define __MT76X2_H__

#include "../mcu/mcu_and.h"
#include "../phy/mt_rf.h"

struct rtmp_adapter;

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

void mt76x2_init(struct rtmp_adapter *ad);
void mt76x2_adjust_per_rate_pwr_delta(struct rtmp_adapter *ad, u8 channel, char delta_pwr);
void mt76x2_get_tx_pwr_per_rate(struct rtmp_adapter *ad);
int mt76x2_read_chl_pwr(struct rtmp_adapter *ad);
void mt7612u_power_on(struct rtmp_adapter *ad);
void mt76x2_calibration(struct rtmp_adapter *ad, u8 channel);
void mt76x2_external_pa_rf_dac_control(struct rtmp_adapter *ad, u8 channel);
void mt76x2_tssi_calibration(struct rtmp_adapter *ad, u8 channel);
void mt76x2_tssi_compensation(struct rtmp_adapter *ad, u8 channel);
int mt76x2_set_ed_cca(struct rtmp_adapter *ad, u8 enable);
int mt76x2_reinit_agc_gain(struct rtmp_adapter *ad, u8 channel);
int mt76x2_reinit_hi_lna_gain(struct rtmp_adapter *ad, u8 channel);
void mt76x2_get_external_lna_gain(struct rtmp_adapter *ad);
void mt76x2_get_agc_gain(struct rtmp_adapter *ad, bool init_phase);
void percentage_delta_pwr(struct rtmp_adapter *ad);

void mt76x2_get_current_temp(struct rtmp_adapter *ad);
void mt76x2_read_temp_info_from_eeprom(struct rtmp_adapter *ad);
void mt76x2_switch_channel(struct rtmp_adapter *ad, u8 channel, bool scan);
void mt76x2_bbp_adjust(struct rtmp_adapter *pAd);

struct mt76x2_frequency_item {
	u8 channel;
	u32 fcal_target;
	u32 sdm_integer;
	u32 sdm_fraction;
};

typedef struct _MT76x2_RATE_PWR_ITEM {
	CHAR mcs_pwr;
} MT76x2_RATE_PWR_ITEM, *PMT76x2_RATE_PWR_ITEM;

typedef struct _MT76x2_RATE_PWR_TABLE {
	MT76x2_RATE_PWR_ITEM CCK[4];
	MT76x2_RATE_PWR_ITEM OFDM[8];
	MT76x2_RATE_PWR_ITEM HT[16];
	MT76x2_RATE_PWR_ITEM VHT1SS[10];
	MT76x2_RATE_PWR_ITEM VHT2SS[10];
	MT76x2_RATE_PWR_ITEM STBC[10];
	MT76x2_RATE_PWR_ITEM MCS32;
} MT76x2_RATE_PWR_Table, *PMT76x2_RATE_PWR_Table;


#endif
