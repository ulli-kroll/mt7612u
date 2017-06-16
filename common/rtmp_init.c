/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_init.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"

u8 NUM_BIT8[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#ifdef DBG
char *CipherName[] = {"none","wep64","wep128","TKIP","AES","CKIP64","CKIP128","CKIP152","SMS4"};
#endif



/*
	Use the global variable is not a good solution.
	But we can not put it to pAd and use the lock in pAd of RALINK_TIMER_STRUCT;
	Or when the structure is cleared, we maybe get NULL for pAd and can not lock.
	Maybe we can put pAd in RTMPSetTimer/ RTMPModTimer/ RTMPCancelTimer.
*/
spinlock_t TimerSemLock;


/*
	========================================================================

	Routine Description:
		Allocate struct rtmp_adapter data block and do some initialization

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
int RTMPAllocAdapterBlock(struct os_cookie *handle, struct rtmp_adapter **ppAdapter)
{
	struct rtmp_adapter *pAd = NULL;
	int  Status;
	INT index;
	u8 *pBeaconBuf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPAllocAdapterBlock\n"));

	/* init UTIL module */
	RtmpUtilInit();

	*ppAdapter = NULL;

	do
	{
		pBeaconBuf = kmalloc(MAX_BEACON_SIZE, GFP_ATOMIC);
		if (pBeaconBuf == NULL) {
			Status = NDIS_STATUS_FAILURE;
			DBGPRINT_ERR(("Failed to allocate memory - BeaconBuf!\n"));
			break;
		}
		memset(pBeaconBuf, 0, MAX_BEACON_SIZE);

		/* Allocate struct rtmp_adapter memory block*/
		Status = AdapterBlockAllocateMemory(&pAd, sizeof(struct rtmp_adapter));
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT_ERR(("Failed to allocate memory - ADAPTER\n"));
			break;
		}
		else
		{
			/* init resource list (must be after pAd allocation) */
			initList(&pAd->RscTimerMemList);
			initList(&pAd->RscTaskMemList);
			initList(&pAd->RscLockMemList);
			initList(&pAd->RscTaskletMemList);
			initList(&pAd->RscSemMemList);
			initList(&pAd->RscAtomicMemList);

			initList(&pAd->RscTimerCreateList);

			pAd->OS_Cookie = handle;
#ifdef WORKQUEUE_BH
			((struct os_cookie *)(handle))->pAd_va = (uint32_t)pAd;
#endif /* WORKQUEUE_BH */
		}
		pAd->BeaconBuf = pBeaconBuf;
		DBGPRINT(RT_DEBUG_OFF, ("\n\n=== pAd = %p, size = %d ===\n\n", pAd, (int) sizeof(struct rtmp_adapter)));

		if (RtmpOsStatsAlloc(&pAd->stats, &pAd->iw_stats) == false)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		/* Init spin locks*/
		spin_lock_init(&pAd->MgmtRingLock);

		for (index =0 ; index < NUM_OF_TX_RING; index++)
		{
			spin_lock_init(&pAd->TxSwQueueLock[index]);
			spin_lock_init(&pAd->DeQueueLock[index]);
			pAd->DeQueueRunning[index] = false;
		}

		/*
			move this function from rt28xx_init() to here. now this function only allocate memory and
			leave the initialization job to RTMPInitTxRxRingMemory() which called in rt28xx_init().
		*/
		Status = RTMPAllocTxRxRingMemory(pAd);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT_ERR(("Failed to allocate memory - TxRxRing\n"));
			break;
		}

		spin_lock_init(&pAd->irq_lock);


		spin_lock_init(&TimerSemLock);

		*ppAdapter = (VOID *)pAd;
	} while (false);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pBeaconBuf)
			kfree(pBeaconBuf);

		if (pAd != NULL)
		{
			if (pAd->stats != NULL)
				kfree(pAd->stats);

			if (pAd->iw_stats != NULL)
				kfree(pAd->iw_stats);

			kfree(pAd);
		}

		return Status;
	}

	/* Init ProbeRespIE Table */
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++)
	{
		pAd->ProbeRespIE[index].pIe =
			kmalloc(MAX_VIE_LEN, GFP_ATOMIC);

		if (pAd->ProbeRespIE[index].pIe == NULL)
			RTMPZeroMemory(pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN);
		else
			pAd->ProbeRespIE[index].pIe = NULL;
	}

	DBGPRINT_S(Status, ("<-- RTMPAllocAdapterBlock, Status=%x\n", Status));
	return Status;
}


static UINT8 NICGetBandSupported(struct rtmp_adapter *pAd)
{
	if (BOARD_IS_5G_ONLY(pAd))
	{
		return RFIC_5GHZ;
	}
	else if (BOARD_IS_2G_ONLY(pAd))
	{
		return RFIC_24GHZ;
	}
	else if (RFIC_IS_5G_BAND(pAd))
	{
		return RFIC_DUAL_BAND;
	}
	else
		return RFIC_24GHZ;
}


bool RTMPCheckPhyMode(struct rtmp_adapter *pAd, UINT8 band_cap, u8 *pPhyMode)
{
	bool RetVal = true;

	if (band_cap == RFIC_24GHZ)
	{
		if (!WMODE_2G_ONLY(*pPhyMode))
		{
			DBGPRINT(RT_DEBUG_TRACE,
					("%s(): Warning! The board type is 2.4G only!\n",
					__FUNCTION__));
			RetVal =  false;
		}
	}
	else if (band_cap == RFIC_5GHZ)
	{
		if (!WMODE_5G_ONLY(*pPhyMode))
		{
			DBGPRINT(RT_DEBUG_TRACE,
					("%s(): Warning! The board type is 5G only!\n",
					__FUNCTION__));
			RetVal =  false;
		}
	}
	else if (band_cap == RFIC_DUAL_BAND)
	{
		RetVal = true;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE,
				("%s(): Unknown supported band (%u), assume dual band used.\n",
				__FUNCTION__, band_cap));

		RetVal = true;
	}

	if (RetVal == false)
	{
		if (band_cap == RFIC_5GHZ) /*5G ony: change to A/N mode */
			*pPhyMode = PHY_11AN_MIXED;
		else /* 2.4G only or Unknown supported band: change to B/G/N mode */
			*pPhyMode = PHY_11BGN_MIXED;

		DBGPRINT(RT_DEBUG_TRACE,
				("%s(): Changed PhyMode to %u\n",
				__FUNCTION__, *pPhyMode));
	}

	return RetVal;

}


/*
	========================================================================

	Routine Description:
		Read initial parameters from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID NICReadEEPROMParameters(struct rtmp_adapter *pAd)
{
	unsigned short i, value, value2;
	EEPROM_VERSION_STRUC Version;
	EEPROM_ANTENNA_STRUC Antenna;
	EEPROM_NIC_CONFIG0_STRUC NicCfg0;
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;
	unsigned short  Addr01,Addr23,Addr45 ;
	MAC_DW0_STRUC csr2;
	MAC_DW1_STRUC csr3;


	DBGPRINT(RT_DEBUG_TRACE, ("--> NICReadEEPROMParameters\n"));

	/* Read MAC setting from EEPROM and record as permanent MAC address */
	DBGPRINT(RT_DEBUG_TRACE, ("Initialize MAC Address from E2PROM \n"));

	Addr01 = mt7612u_read_eeprom16(pAd, 0x04);
	Addr23 = mt7612u_read_eeprom16(pAd, 0x06);
	Addr45 = mt7612u_read_eeprom16(pAd, 0x08);

	pAd->PermanentAddress[0] = (u8)(Addr01 & 0xff);
	pAd->PermanentAddress[1] = (u8)(Addr01 >> 8);
	pAd->PermanentAddress[2] = (u8)(Addr23 & 0xff);
	pAd->PermanentAddress[3] = (u8)(Addr23 >> 8);
	pAd->PermanentAddress[4] = (u8)(Addr45 & 0xff);
	pAd->PermanentAddress[5] = (u8)(Addr45 >> 8);

	/*more conveninet to test mbssid, so ap's bssid &0xf1*/
	if (pAd->PermanentAddress[0] == 0xff)
		pAd->PermanentAddress[0] = RandomByte(pAd)&0xf8;

	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM MAC: =%02x:%02x:%02x:%02x:%02x:%02x\n",
								PRINT_MAC(pAd->PermanentAddress)));

	COPY_MAC_ADDR(pAd->CurrentAddress, pAd->PermanentAddress);
	DBGPRINT(RT_DEBUG_OFF, ("Use the MAC address what is assigned from EEPROM. \n"));

	/* Set the current MAC to ASIC */
	csr2.field.Byte0 = pAd->CurrentAddress[0];
	csr2.field.Byte1 = pAd->CurrentAddress[1];
	csr2.field.Byte2 = pAd->CurrentAddress[2];
	csr2.field.Byte3 = pAd->CurrentAddress[3];
	mt7612u_write32(pAd, MAC_ADDR_DW0, csr2.word);
	csr3.word = 0;
	csr3.field.Byte4 = pAd->CurrentAddress[4];
	csr3.field.Byte5 = pAd->CurrentAddress[5];
	csr3.field.U2MeMask = 0xff;
	mt7612u_write32(pAd, MAC_ADDR_DW1, csr3.word);


	DBGPRINT_RAW(RT_DEBUG_TRACE,("Current MAC: =%02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(pAd->CurrentAddress)));

	/* if not return early. cause fail at emulation.*/
	/* Init the channel number for TX channel power*/
	mt76x2_read_chl_pwr(pAd);

	/* if E2PROM version mismatch with driver's expectation, then skip*/
	/* all subsequent E2RPOM retieval and set a system error bit to notify GUI*/
	Version.word = mt7612u_read_eeprom16(pAd, EEPROM_VERSION_OFFSET);
	pAd->EepromVersion = Version.field.Version + Version.field.FaeReleaseNumber * 256;
	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: Version = %d, FAE release #%d\n", Version.field.Version, Version.field.FaeReleaseNumber));

	/* Read BBP default value from EEPROM and store to array(EEPROMDefaultValue) in pAd */
	value = mt7612u_read_eeprom16(pAd, EEPROM_NIC1_OFFSET);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] = value;
	NicCfg0.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET];
	DBGPRINT(RT_DEBUG_OFF, ("EEPROM_NIC_CFG1_OFFSET = 0x%04x\n", NicCfg0.word));

	/* EEPROM offset 0x36 - NIC Configuration 1 */
	value = mt7612u_read_eeprom16(pAd, EEPROM_NIC2_OFFSET);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] = value;
	NicConfig2.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET];

	value = mt7612u_read_eeprom16(pAd, EEPROM_NIC3_OFFSET);

	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG3_OFFSET] = value;
	pAd->NicConfig3.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG3_OFFSET];
	DBGPRINT(RT_DEBUG_OFF, ("Bluetooth Support word %04x\n", pAd->NicConfig3.word));

	value = mt7612u_read_eeprom16(pAd, EEPROM_COUNTRY_REGION);	/* Country Region*/

	/*
	 * ULLI : hard fix for invalid country region on some eeproms
	 * ULLI : this occurs with missing REGION_30_A_BAND in A-BAND
	 */

	value = 0xffff;		/* HARD fix */

	pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] = value;
	DBGPRINT(RT_DEBUG_OFF, ("Country Region from e2p = %x\n", value));

	for(i = 0; i < 8; i++) 	{
		value = mt7612u_read_eeprom16(pAd, EEPROM_BBP_BASE_OFFSET + i*2);
		pAd->EEPROMDefaultValue[i+EEPROM_BBP_ARRAY_OFFSET] = value;
	}

	/* We have to parse NIC configuration 0 at here.*/
	/* If TSSI did not have preloaded value, it should reset the TxAutoAgc to false*/
	/* Therefore, we have to read TxAutoAgc control beforehand.*/
	/* Read Tx AGC control bit*/

	/*
	 * ULLI : we read here the antenna control word, but hat no value
	 * ULLI : because we have fix TX/RX streams
	 */

#if 0	/* ULLI : disabled */
	Antenna.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET];
#endif

	/* must be put here, because RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET() will clear *
	 * EPROM 0x34~3 */

	Antenna.word = 0;
	Antenna.field.RfIcType = RFIC_7662;
	Antenna.field.TxPath = NicCfg0.field.TxPath;
	Antenna.field.RxPath = NicCfg0.field.RxPath;

	/* Choose the desired Tx&Rx stream.*/
	/* ULLI : fixed TX/RX streams */
	pAd->CommonCfg.TxStream = Antenna.field.TxPath;
	pAd->CommonCfg.RxStream = Antenna.field.RxPath;

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s(): AfterAdjust, RxPath = %d, TxPath = %d\n",
					__FUNCTION__, Antenna.field.RxPath, Antenna.field.TxPath));

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (NicConfig2.word == 0xffff)
			NicConfig2.word = 0;
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if ((NicConfig2.word & 0x00ff) == 0xff)
			NicConfig2.word &= 0xff00;

		if ((NicConfig2.word >> 8) == 0xff)
			NicConfig2.word &= 0x00ff;
	}
#endif /* CONFIG_STA_SUPPORT */

	/* Save value for future using */
	pAd->NicConfig2.word = NicConfig2.word;

	/* Save the antenna for future use*/
	pAd->Antenna.word = Antenna.word;

	/* Set the RfICType here, then we can initialize RFIC related operation callbacks*/
	pAd->Mlme.RealRxPath = (u8) Antenna.field.RxPath;
	pAd->RfIcType = (u8) Antenna.field.RfIcType;

	if (IS_MT7662(pAd))
		pAd->RfIcType = RFIC_7662;

	if (IS_MT7612(pAd))
		pAd->RfIcType = RFIC_7612;

	pAd->phy_ctrl.rf_band_cap = NICGetBandSupported(pAd);

	/* check if the chip supports 5G band */
	if (WMODE_CAP_5G(pAd->CommonCfg.PhyMode)) {
		if (!RFIC_IS_5G_BAND(pAd)) {
			DBGPRINT_RAW(RT_DEBUG_ERROR,
						("%s():Err! chip not support 5G band %d!\n",
						__FUNCTION__, pAd->RfIcType));
			/* change to bgn mode */
			Set_WirelessMode_Proc(pAd, "9");
			pAd->phy_ctrl.rf_band_cap = RFIC_24GHZ;
		}
		pAd->phy_ctrl.rf_band_cap = RFIC_24GHZ | RFIC_5GHZ;
	} else {
			pAd->phy_ctrl.rf_band_cap = RFIC_24GHZ;
	}

	if (IS_MT76x2(pAd)) {
		mt76x2_read_temp_info_from_eeprom(pAd);
	}

	pAd->BbpRssiToDbmDelta = 0x0;

	/* Read frequency offset setting for RF*/
	value = mt7612u_read_eeprom16(pAd, EEPROM_FREQ_OFFSET);

	if ((value & 0x00FF) != 0x00FF)
		pAd->RfFreqOffset = (ULONG) (value & 0x00FF);
	else
		pAd->RfFreqOffset = 0;


	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: RF FreqOffset=0x%x \n", pAd->RfFreqOffset));

	/*CountryRegion byte offset (38h)*/

	value = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] >> 8;		/* 2.4G band*/
	value2 = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] & 0x00FF;	/* 5G band*/

	if ((value <= REGION_MAXIMUM_BG_BAND) || (value == REGION_31_BG_BAND) || (value == REGION_32_BG_BAND) || (value == REGION_33_BG_BAND) )
		pAd->CommonCfg.CountryRegion = ((u8) value) | EEPROM_IS_PROGRAMMED;

	if (value2 <= REGION_MAXIMUM_A_BAND)
		pAd->CommonCfg.CountryRegionForABand = ((u8) value2) | EEPROM_IS_PROGRAMMED;

	/* Get RSSI Offset on EEPROM 0x9Ah & 0x9Ch.*/
	/* The valid value are (-10 ~ 10) */
	/* */
	if (IS_MT76x2(pAd)) {
		value = mt7612u_read_eeprom16(pAd, EEPROM_RSSI_BG_OFFSET);

		if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
			pAd->BGRssiOffset[0] = 0;
		} else {
			if (value & RSSI0_OFFSET_G_BAND_EN) {
				if (value & RSSI0_OFFSET_G_BAND_SIGN)
					pAd->BGRssiOffset[0] = (value & RSSI0_OFFSET_G_BAND_MASK);
				else
					pAd->BGRssiOffset[0] = -(value & RSSI0_OFFSET_G_BAND_MASK);
			} else {
				pAd->BGRssiOffset[0] = 0;
			}
		}

		if (((value & 0xff00) == 0x0000) || ((value & 0xff00) == 0xff00)) {
			pAd->BGRssiOffset[1] = 0;
		} else {
			if (value & RSSI1_OFFSET_G_BAND_EN) {
				if (value & RSSI1_OFFSET_G_BAND_SIGN)
					pAd->BGRssiOffset[1] = ((value & RSSI1_OFFSET_G_BAND_MASK) >> 8);
				else
					pAd->BGRssiOffset[1] = -((value & RSSI1_OFFSET_G_BAND_MASK) >> 8);
			} else {
				pAd->BGRssiOffset[1] = 0;
			}
		}
	} else 	{
		value = mt7612u_read_eeprom16(pAd, EEPROM_RSSI_BG_OFFSET);
		pAd->BGRssiOffset[0] = value & 0x00ff;
		pAd->BGRssiOffset[1] = (value >> 8);
	}

	value = mt7612u_read_eeprom16(pAd, EEPROM_RSSI_BG_OFFSET+2);
/*			if (IS_RT2860(pAd))  RT2860 supports 3 Rx and the 2.4 GHz RSSI #2 offset is in the EEPROM 0x48*/
	pAd->BGRssiOffset[2] = value & 0x00ff;
	pAd->ALNAGain1 = (value >> 8);

	value = mt7612u_read_eeprom16(pAd, EEPROM_LNA_OFFSET);
	pAd->BLNAGain = value & 0x00ff;
	/* External LNA gain for 5GHz Band(CH36~CH64) */
	pAd->ALNAGain0 = (value >> 8);


	if (IS_MT76x2(pAd)) {
		value = mt7612u_read_eeprom16(pAd, EEPROM_RSSI_A_OFFSET);

		if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
			pAd->ARssiOffset[0] = 0;
		} else {
			if (value & RSSI0_OFFSET_A_BANE_EN) {
				if (value & RSSI0_OFFSET_A_BAND_SIGN)
					pAd->ARssiOffset[0] = (value & RSSI0_OFFSET_A_BAND_MASK);
				else
					pAd->ARssiOffset[0] = -(value & RSSI0_OFFSET_A_BAND_MASK);
			} else {
				pAd->ARssiOffset[0] = 0;
			}
		}

		if (((value & 0xff00) == 0x0000) || ((value & 0xff00) == 0xff00)) {
			pAd->ARssiOffset[1] = 0;
		} else {
			if (value & RSSI1_OFFSET_A_BAND_EN) {
				if (value & RSSI1_OFFSET_A_BAND_SIGN)
					pAd->ARssiOffset[1] = ((value & RSSI1_OFFSET_A_BAND_MASK) >> 8);
				else
					pAd->ARssiOffset[1] = -((value & RSSI1_OFFSET_A_BAND_MASK) >> 8);
			} else {
				pAd->ARssiOffset[1] = 0;
			}
		}
	} else {
		value = mt7612u_read_eeprom16(pAd, EEPROM_RSSI_A_OFFSET);
		pAd->ARssiOffset[0] = value & 0x00ff;
		pAd->ARssiOffset[1] = (value >> 8);
	}

	value = mt7612u_read_eeprom16(pAd, (EEPROM_RSSI_A_OFFSET+2));
	pAd->ARssiOffset[2] = value & 0x00ff;
	pAd->ALNAGain2 = (value >> 8);


	if (((u8)pAd->ALNAGain1 == 0xFF) || (pAd->ALNAGain1 == 0x00))
		pAd->ALNAGain1 = pAd->ALNAGain0;
	if (((u8)pAd->ALNAGain2 == 0xFF) || (pAd->ALNAGain2 == 0x00))
		pAd->ALNAGain2 = pAd->ALNAGain0;

	DBGPRINT(RT_DEBUG_TRACE, ("ALNAGain0 = %d, ALNAGain1 = %d, ALNAGain2 = %d\n",
					pAd->ALNAGain0, pAd->ALNAGain1, pAd->ALNAGain2));

	/* Validate 11a/b/g RSSI 0/1/2 offset.*/
	for (i =0 ; i < 3; i++) {
		if ((pAd->BGRssiOffset[i] < -10) || (pAd->BGRssiOffset[i] > 10))
			pAd->BGRssiOffset[i] = 0;

		if ((pAd->ARssiOffset[i] < -10) || (pAd->ARssiOffset[i] > 10))
			pAd->ARssiOffset[i] = 0;
	}

	/*
		Get TX mixer gain setting
		0xff are invalid value
		Note:
			RT30xx default value is 0x00 and will program to RF_R17
				only when this value is not zero
			RT359x default value is 0x02
	*/

	mt76x2_get_tx_pwr_per_rate(pAd);

	if (IS_MT76x2(pAd))
		mt76x2_get_external_lna_gain(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->Antenna.field.BoardType = %d\n",
		__FUNCTION__,
		pAd->Antenna.field.BoardType));


	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICReadEEPROMParameters\n"));
}


/*
	========================================================================

	Routine Description:
		Set default value from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID NICInitAsicFromEEPROM(struct rtmp_adapter *pAd)
{
#ifdef CONFIG_STA_SUPPORT
	uint32_t data = 0;
#endif /* CONFIG_STA_SUPPORT */
#if defined(RT30xx)
	unsigned short i;
#endif /* defined(RT30xx) */
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitAsicFromEEPROM\n"));

	NicConfig2.word = pAd->NicConfig2.word;

#ifdef RTMP_RF_RW_SUPPORT
	/*Init RFRegisters after read RFIC type from EEPROM*/
	mt76x2_init_rf_cr(pAd);
#endif /* RTMP_RF_RW_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Read Hardware controlled Radio state enable bit*/
		if (NicConfig2.field.HardwareRadioControl == 1)
		{
			bool radioOff = false;
			pAd->StaCfg.bHardwareRadio = true;

			{
				/* Read GPIO pin2 as Hardware controlled radio state*/
				data = mt7612u_read32(pAd, GPIO_CTRL_CFG);
				if ((data & 0x04) == 0)
					radioOff = true;
			}

			if (radioOff)
			{
				pAd->StaCfg.bHwRadio = false;
				pAd->StaCfg.bRadio = false;
				/* mt7612u_write32(pAd, PWR_PIN_CFG, 0x00001818); */
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
			}
		}
		else
			pAd->StaCfg.bHardwareRadio = false;


	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef WIN_NDIS
	/* Turn off patching for cardbus controller */
	/*
	if (NicConfig2.field.CardbusAcceleration == 1)
		pAd->bTest1 = true;
	*/
#endif /* WIN_NDIS */


	mt7612u_bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		INT dac = 0;
		/* Handle the difference when 1T*/
		{
			if(pAd->Antenna.field.TxPath == 1)
				dac = 0;
			else
				dac = 2;
		}
		mt7612u_bbp_set_txdac(pAd, dac);
		DBGPRINT(RT_DEBUG_TRACE, ("Use Hw Radio Control Pin=%d; if used Pin=%d;\n",
					pAd->StaCfg.bHardwareRadio, pAd->StaCfg.bHardwareRadio));
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
		/*
			Only for RT3593, RT5390 (Maybe add other chip in the future)
			Sometimes the frequency will be shift, we need to adjust it.
		*/
		if (pAd->StaCfg.AdaptiveFreq == true) /*Todo: iwpriv and profile support.*/
			pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration = true;

		DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration = %d\n",
					__FUNCTION__, pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration));

#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	DBGPRINT(RT_DEBUG_TRACE, ("TxPath = %d, RxPath = %d, RFIC=%d\n",
				pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, pAd->RfIcType));
	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitAsicFromEEPROM\n"));
}


/*
	========================================================================

	Routine Description:
		Initialize NIC hardware

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
int NICInitializeAdapter(struct rtmp_adapter *pAd, bool bHardReset)
{
	int Status = NDIS_STATUS_SUCCESS;
	UINT rty_cnt = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitializeAdapter\n"));

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
retry:

	if (AsicWaitPDMAIdle(pAd, 100, 1000) != true) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return NDIS_STATUS_FAILURE;
	}



	/* Initialze ASIC for TX & Rx operation*/
	if (NICInitializeAsic(pAd , bHardReset) != NDIS_STATUS_SUCCESS)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return NDIS_STATUS_FAILURE;

		if (rty_cnt++ == 0) {
			NICLoadFirmware(pAd);
			goto retry;
		}
		return NDIS_STATUS_FAILURE;
	}


	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitializeAdapter\n"));
	return Status;
}

/*
	========================================================================

	Routine Description:
		Initialize ASIC

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
int NICInitializeAsic(struct rtmp_adapter *pAd, bool bHardReset)
{
	ULONG Index = 0;
	uint32_t mac_val = 0;
	uint32_t Counter = 0;
	unsigned short KeyIdx;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitializeAsic\n"));

	/*@!Release
		For MT76x0 series,
		PWR_PIN_CFG[2:0]: obsolete, no function
		Don't need to change PWR_PIN_CFG here.
	*/


	/* Make sure MAC gets ready after NICLoadFirmware().*/

	/*To avoid hang-on issue when interface up in kernel 2.4, */
	/*we use a local variable "MacCsr0" instead of using "pAd->MACVersion" directly.*/
	if (WaitForAsicReady(pAd) != true)
		return NDIS_STATUS_FAILURE;

	// TODO: shiang, how about the value setting of pAd->MACVersion?? Original it assigned here

	DBGPRINT(RT_DEBUG_OFF, ("MAC[Ver=0x%08x]\n",
				pAd->MACVersion));
	if (!(IS_MT7601(pAd) || IS_MT76x2(pAd))) {
		/* turn on bit13 (set to zero) after rt2860D. This is to solve high-current issue.*/
		mac_val = mt7612u_read32(pAd, PBF_SYS_CTRL);
		mac_val &= (~0x2000);
		mt7612u_write32(pAd, PBF_SYS_CTRL, mac_val);
	}


	mt7612u_mcu_usb_fw_init(pAd);
	rtmp_mac_init(pAd);

	rtmp_mac_bcn_buf_init(pAd);

	mt7612u_phy_init_bbp(pAd);


	if (IS_RT65XX(pAd)) /* 3*3*/
	{
		uint32_t csr;
		csr = mt7612u_read32(pAd, MAX_LEN_CFG);
#if defined(RT2883) || defined(RT3883) || defined(RT3593)
		if (IS_RT65XX(pAd) || IS_MT7601(pAd))
			csr |= 0x3fff;
		else
#endif /* defined(RT2883) || defined(RT3883) || defined(RT3593) */
		{
			csr &= 0xFFF;
			csr |= 0x2000;
		}
		mt7612u_write32(pAd, MAX_LEN_CFG, csr);
	}

	{
		u8 MAC_Value[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0,0};

		/*Initialize WCID table*/
		for(Index =0 ;Index < 254;Index++)
		{
			RTUSBMultiWrite(pAd, (unsigned short)(MAC_WCID_BASE + Index * 8), MAC_Value, 8);
		}
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Add radio off control*/
		if (pAd->StaCfg.bRadio == false)
		{
			/* mt7612u_write32(pAd, PWR_PIN_CFG, 0x00001818);*/
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
			DBGPRINT(RT_DEBUG_TRACE, ("Set Radio Off\n"));
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	/* Clear raw counters*/
	NicResetRawCounters(pAd);

	/*
		ASIC will keep garbage value after boot
		Clear all shared key table when initial
		This routine can be ignored in radio-ON/OFF operation.
	*/
	if (bHardReset)
	{
		uint32_t wcid_attr_base = 0, wcid_attr_size = 0, share_key_mode_base = 0;

		wcid_attr_base = RLT_MAC_WCID_ATTRIBUTE_BASE;
		wcid_attr_size = RLT_HW_WCID_ATTRI_SIZE;
		share_key_mode_base = RLT_SHARED_KEY_MODE_BASE;

		for (KeyIdx = 0; KeyIdx < 4; KeyIdx++)
		{
			mt7612u_write32(pAd, share_key_mode_base + 4*KeyIdx, 0);
		}

		/* Clear all pairwise key table when initial*/
		for (KeyIdx = 0; KeyIdx < 256; KeyIdx++)
		{
			mt7612u_write32(pAd, wcid_attr_base + (KeyIdx * wcid_attr_size), 1);
		}
	}

	/* It isn't necessary to clear this space when not hard reset. */
	if (bHardReset == true)
	{
		/* clear all on-chip BEACON frame space */
#ifdef CONFIG_AP_SUPPORT
		INT	i, apidx;
		for (apidx = 0; apidx < HW_BEACON_MAX_COUNT(pAd); apidx++)
		{
			if (pAd->BeaconOffset[apidx] > 0) {
				// TODO: shiang-6590, if we didn't define MBSS_SUPPORT, the pAd->BeaconOffset[x] may set as 0 when chipCap.BcnMaxHwNum != HW_BEACON_MAX_COUNT
				for (i = 0; i < HW_BEACON_OFFSET; i+=4)
					mt7612u_write32(pAd, pAd->BeaconOffset[apidx] + i, 0x00);

				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
					if (pAd->CommonCfg.pBeaconSync)
						pAd->CommonCfg.pBeaconSync->BeaconBitMap &= (~(BEACON_BITMAP_MASK & (1 << apidx)));
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	AsicDisableSync(pAd);

	/* Clear raw counters*/
	NicResetRawCounters(pAd);

	/* Default PCI clock cycle per ms is different as default setting, which is based on PCI.*/
	Counter = mt7612u_read32(pAd, USB_CYC_CFG);
	Counter&=0xffffff00;
	Counter|=0x000001e;
	mt7612u_write32(pAd, USB_CYC_CFG, Counter);

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* for rt2860E and after, init TXOP_CTRL_CFG with 0x583f. This is for extension channel overlapping IOT.*/
		if ((pAd->MACVersion & 0xffff) != 0x0101)
			mt7612u_write32(pAd, TXOP_CTRL_CFG, 0x583f);
	}
#endif /* CONFIG_STA_SUPPORT */


	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitializeAsic\n"));
	return NDIS_STATUS_SUCCESS;
}




#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */


VOID NICUpdateFifoStaCounters(struct rtmp_adapter *pAd)
{
#ifdef FIFO_EXT_SUPPORT
	TX_STA_FIFO_EXT_STRUC StaFifoExt;
#endif /* FIFO_EXT_SUPPORT */
	TX_STA_FIFO_STRUC	StaFifo;
	MAC_TABLE_ENTRY		*pEntry = NULL;
	uint32_t 			i = 0;
	u8 			pid = 0, wcid = 0;
	int32_t 			reTry;
	u8 			succMCS, PhyMode;

#ifdef CONFIG_STA_SUPPORT
	if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
		return;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	if (pAd->MacTab.Size <= 8)
	{
		if (IS_RT65XX(pAd))
			return;
	}
#endif /* CONFIG_AP_SUPPORT */


	do
	{
#ifdef FIFO_EXT_SUPPORT
		StaFifoExt.word = mt7612u_read32(pAd, TX_STA_FIFO_EXT);
#endif /* FIFO_EXT_SUPPORT */
		StaFifo.word = mt7612u_read32(pAd, TX_STA_FIFO);

		if (StaFifo.field.bValid == 0)
			break;

		wcid = (u8)StaFifo.field.wcid;

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFIFO) {
			dbQueueEnqueue(0x73, (u8 *)(&StaFifo.word));
		}
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		/* ignore NoACK and MGMT frame use 0xFF as WCID */
		if ((StaFifo.field.TxAckRequired == 0) || (wcid >= MAX_LEN_OF_MAC_TABLE))
		{
			i++;
			continue;
		}

		/* PID store Tx MCS Rate */
		if (IS_MT76x2(pAd))
		{
			PhyMode = StaFifo.field.PhyMode;
			if((PhyMode == 2) || (PhyMode == 3))
			{
 				pid = (u8)StaFifoExt.field.PidType & 0xF;
			}
			else if(PhyMode == 4)
			{
				pid = (u8)StaFifoExt.field.PidType & 0xF;
				pid += (((u8)StaFifoExt.field.PidType & 0x10) ? 10 : 0);
			}
		}
		else
		pid = (u8)StaFifo.field.PidType;

		pEntry = &pAd->MacTab.Content[wcid];


#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

		if (IS_MT76x2(pAd))
		{
			if(pEntry->LowPacket == false)
 			continue;
		}

		pEntry->DebugFIFOCount++;


		/* Update BF statistics*/
		if (pAd->chipCap.FlgHwTxBfCap)
		{
			int succMCS = (StaFifo.field.SuccessRate & 0x7F);
			int origMCS = pid;

			if (succMCS==32)
				origMCS = 32;

			if (succMCS>origMCS)
				origMCS = succMCS+1;

			/* MCS16 falls back to MCS8*/
			if (origMCS>=16 && succMCS<=8)
				succMCS += 8;

			/* MCS8 falls back to 0 */
			if (origMCS >= 8 && succMCS == 0)
				succMCS += 7;

			reTry = origMCS-succMCS;

			if (StaFifo.field.eTxBF) {
				if (StaFifo.field.TxSuccess)
					pEntry->TxBFCounters.ETxSuccessCount++;
				else
					pEntry->TxBFCounters.ETxFailCount++;
					pEntry->TxBFCounters.ETxRetryCount += reTry;
			}
			else if (StaFifo.field.iTxBF) {
				if (StaFifo.field.TxSuccess)
					pEntry->TxBFCounters.ITxSuccessCount++;
				else
					pEntry->TxBFCounters.ITxFailCount++;
				pEntry->TxBFCounters.ITxRetryCount += reTry;
			}
			else {
			if (StaFifo.field.TxSuccess)
				pEntry->TxBFCounters.TxSuccessCount++;
			else
				pEntry->TxBFCounters.TxFailCount++;
			pEntry->TxBFCounters.TxRetryCount += reTry;
		}
	}


#ifdef CONFIG_STA_SUPPORT
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		continue;
#endif /* CONFIG_STA_SUPPORT */

	if (!StaFifo.field.TxSuccess)
	{
		pEntry->FIFOCount++;
		pEntry->OneSecTxFailCount++;
#ifdef CONFIG_AP_SUPPORT
		pEntry->StatTxFailCount += pEntry->OneSecTxFailCount;
		pAd->ApCfg.MBSSID[pEntry->apidx].StatTxFailCount += pEntry->StatTxFailCount;
#endif /* CONFIG_AP_SUPPORT */


		if (pEntry->FIFOCount >= 1)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("#"));
			pEntry->NoBADataCountDown = 64;

			/* Update the continuous transmission counter.*/
			pEntry->ContinueTxFailCnt++;

			if(pEntry->PsMode == PWR_ACTIVE)
			{
				int tid;

#ifdef CONFIG_AP_SUPPORT
#ifdef MULTI_CLIENT_SUPPORT
				if ((pAd->ApCfg.EntryClientCount > 2) &&
					(pEntry->HTPhyMode.field.MODE >= MODE_HTMIX) &&
					(pEntry->lowTrafficCount >= 4 /* 2 sec */))
					pEntry->NoBADataCountDown = 10;
#endif /* MULTI_CLIENT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

				for (tid=0; tid<NUM_OF_TID; tid++)
					BAOriSessionTearDown(pAd, pEntry->wcid,  tid, false, false);

			}
		}
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	}
	else
	{
		if ((pEntry->PsMode != PWR_SAVE) && (pEntry->NoBADataCountDown > 0))
		{
			pEntry->NoBADataCountDown--;
			if (pEntry->NoBADataCountDown==0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("@\n"));
			}
		}
		pEntry->FIFOCount = 0;
		pEntry->OneSecTxNoRetryOkCount++;


		/* update NoDataIdleCount when sucessful send packet to STA.*/
		pEntry->NoDataIdleCount = 0;
		pEntry->ContinueTxFailCnt = 0;

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
	}

	if (IS_MT76x2(pAd))
	{
		PhyMode = StaFifo.field.PhyMode;
		if((PhyMode == 2) || (PhyMode == 3))
		{
  	    		succMCS = StaFifo.field.SuccessRate & 0xF;


			if (StaFifo.field.TxSuccess)
			{
				pEntry->TXMCSExpected[pid]++;
				if (pid == succMCS)
					pEntry->TXMCSSuccessful[pid]++;
				else
					pEntry->TXMCSAutoFallBack[pid][succMCS]++;
			}
			else
			{
				pEntry->TXMCSFailed[pid]++;
			}


			reTry = pid - succMCS;

			if (reTry > 0)
			{
				/* MCS8 falls back to 0 */
				if (pid>=8 && succMCS==0)
					reTry -= 7;
		    		//else if ((pid >= 12) && succMCS <=7)
			    	//	reTry -= 4;

				pEntry->OneSecTxRetryOkCount += reTry;

#ifdef CONFIG_AP_SUPPORT
				pEntry->StatTxRetryOkCount += pEntry->OneSecTxRetryOkCount;
				pAd->ApCfg.MBSSID[pEntry->apidx].StatTxRetryOkCount += pEntry->StatTxRetryOkCount;
#endif /* CONFIG_AP_SUPPORT */

			}
		}
		else if(PhyMode == 4)
		{
  	    		succMCS = StaFifo.field.SuccessRate & 0xF;
			succMCS += ((StaFifo.field.SuccessRate & 0x10) ? 10 : 0);
			//DBGPRINT(0, ("%s()Succ MCS :TxMCS(%d):PHYMode(%d)\n", __FUNCTION__, pid, PhyMode));
		    	if (StaFifo.field.TxSuccess)
		    	{
		    		pEntry->TXMCSExpected[pid]++;

			    	if (pid == succMCS)
			    		pEntry->TXMCSSuccessful[pid]++;
			    	else
			    		pEntry->TXMCSAutoFallBack[pid][succMCS]++;
		    	}
		    	else
			{
				pEntry->TXMCSFailed[pid]++;
			}

			reTry = pid - succMCS;

			if (reTry > 0)
			{
				/* MCS10 falls back to 0 */
				if (pid >= 10 && succMCS == 0)
					reTry -= 9;

				pEntry->OneSecTxRetryOkCount += reTry;

			}

			if(reTry <= 0)
				pEntry->DownTxMCSRate[0]++;
			else if(reTry > (NUM_OF_SWFB-1))
				pEntry->DownTxMCSRate[NUM_OF_SWFB-1]++;
			else
				pEntry->DownTxMCSRate[reTry]++;
			}
		}
		else
		{
			succMCS = StaFifo.field.SuccessRate & 0x7F;


			if (StaFifo.field.TxSuccess)
			{
				pEntry->TXMCSExpected[pid]++;
				if (pid == succMCS)
				{
					pEntry->TXMCSSuccessful[pid]++;
				}
				else
				{
					pEntry->TXMCSAutoFallBack[pid][succMCS]++;
				}
			}
			else
			{
				pEntry->TXMCSFailed[pid]++;
			}


			reTry = pid - succMCS;

			if (reTry > 0)
			{
				/* MCS8 falls back to 0 */
				if (pid>=8 && succMCS==0)
					reTry -= 7;
				else if ((pid >= 12) && succMCS <=7)
				{
					reTry -= 4;
				}

				pEntry->OneSecTxRetryOkCount += reTry;
			}
		}

		i++;	/* ASIC store 16 stack*/
	} while ( i < (TX_RING_SIZE<<1) );

}


#ifdef FIFO_EXT_SUPPORT
bool NicGetMacFifoTxCnt(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry)
{
	if (pEntry->wcid >= 1 && pEntry->wcid <= 8)
	{
		WCID_TX_CNT_STRUC wcidTxCnt;
		uint32_t regAddr;

		regAddr = WCID_TX_CNT_0 + (pEntry->wcid - 1) * 4;
		wcidTxCnt.word = mt7612u_read32(pAd, regAddr);

		pEntry->fifoTxSucCnt += wcidTxCnt.field.succCnt;
		pEntry->fifoTxRtyCnt += wcidTxCnt.field.reTryCnt;
	}

	return true;
}


VOID AsicFifoExtSet(IN struct rtmp_adapter *pAd)
{
	if (pAd->chipCap.FlgHwFifoExtCap)
	{
		mt7612u_write32(pAd, WCID_MAPPING_0, 0x04030201);
		mt7612u_write32(pAd, WCID_MAPPING_1, 0x08070605);
	}
}


VOID AsicFifoExtEntryClean(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry)
{
	WCID_TX_CNT_STRUC wcidTxCnt;
	uint32_t regAddr;

	if (pAd->chipCap.FlgHwFifoExtCap)
	{
		/* We clean the fifo info when MCS is 0 and Aid is from 1~8 */
		if (pEntry->wcid >=1  && pEntry->wcid <= 8)
		{
			regAddr = WCID_TX_CNT_0 + (pEntry->wcid - 1) * 4;
			wcidTxCnt.word = mt7612u_read32(pAd, regAddr);
		}
	}
}
#endif /* FIFO_EXT_SUPPORT */


/*
	========================================================================

	Routine Description:
		Read Tx statistic raw counters from hardware registers and record to
		related software variables for later on query

	Arguments:
		pAd					Pointer to our adapter
		pStaTxCnt0			Pointer to record "TX_STA_CNT0" (0x170c)
		pStaTxCnt1			Pointer to record "TX_STA_CNT1" (0x1710)

	Return Value:
		None

	========================================================================
*/
VOID NicGetTxRawCounters(
	IN struct rtmp_adapter *pAd,
	IN TX_STA_CNT0_STRUC *pStaTxCnt0,
	IN TX_STA_CNT1_STRUC *pStaTxCnt1)
{

	pStaTxCnt0->word = mt7612u_read32(pAd, TX_STA_CNT0);
	pStaTxCnt1->word = mt7612u_read32(pAd, TX_STA_CNT1);

	pAd->bUpdateBcnCntDone = true;	/* not appear in Rory's code */
	pAd->RalinkCounters.OneSecBeaconSentCnt += pStaTxCnt0->field.TxBeaconCount;
	pAd->RalinkCounters.OneSecTxRetryOkCount += pStaTxCnt1->field.TxRetransmit;
	pAd->RalinkCounters.OneSecTxNoRetryOkCount += pStaTxCnt1->field.TxSuccess;
	pAd->RalinkCounters.OneSecTxFailCount += pStaTxCnt0->field.TxFailCount;

#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.TransmittedFragmentCount.u.LowPart += pStaTxCnt1->field.TxSuccess;
	pAd->WlanCounters.RetryCount.u.LowPart += pStaTxCnt1->field.TxRetransmit;
	pAd->WlanCounters.FailedCount.u.LowPart += pStaTxCnt0->field.TxFailCount;
#endif /* STATS_COUNT_SUPPORT */


}


/*
	========================================================================

	Routine Description:
		Clean all Tx/Rx statistic raw counters from hardware registers

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	========================================================================
*/
VOID NicResetRawCounters(struct rtmp_adapter *pAd)
{
	uint32_t Counter;

	Counter = mt7612u_read32(pAd, RX_STA_CNT0);
	Counter = mt7612u_read32(pAd, RX_STA_CNT1);
	Counter = mt7612u_read32(pAd, RX_STA_CNT2);
	Counter = mt7612u_read32(pAd, TX_STA_CNT0);
	Counter = mt7612u_read32(pAd, TX_STA_CNT1);
	Counter = mt7612u_read32(pAd, TX_STA_CNT2);
}


/*
	========================================================================

	Routine Description:
		Read statistical counters from hardware registers and record them
		in software variables for later on query

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
VOID NICUpdateRawCounters(struct rtmp_adapter *pAd)
{
	uint32_t OldValue;/*, Value2;*/
	/*ULONG	PageSum, OneSecTransmitCount;*/
	/*ULONG	TxErrorRatio, Retry, Fail;*/
	RX_STA_CNT0_STRUC	 RxStaCnt0;
	RX_STA_CNT1_STRUC   RxStaCnt1;
	RX_STA_CNT2_STRUC   RxStaCnt2;
	TX_STA_CNT0_STRUC 	 TxStaCnt0;
	TX_STA_CNT1_STRUC	 StaTx1;
	TX_STA_CNT2_STRUC	 StaTx2;
#ifdef STATS_COUNT_SUPPORT
	TX_NAG_AGG_CNT_STRUC	TxAggCnt;
	TX_AGG_CNT0_STRUC	TxAggCnt0;
	TX_AGG_CNT1_STRUC	TxAggCnt1;
	TX_AGG_CNT2_STRUC	TxAggCnt2;
	TX_AGG_CNT3_STRUC	TxAggCnt3;
	TX_AGG_CNT4_STRUC	TxAggCnt4;
	TX_AGG_CNT5_STRUC	TxAggCnt5;
	TX_AGG_CNT6_STRUC	TxAggCnt6;
	TX_AGG_CNT7_STRUC	TxAggCnt7;
#endif /* STATS_COUNT_SUPPORT */
	COUNTER_RALINK		*pRalinkCounters;


	pRalinkCounters = &pAd->RalinkCounters;
#ifdef STATS_COUNT_SUPPORT
	if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
		return;
#endif /* STATS_COUNT_SUPPORT */



	RxStaCnt0.word = mt7612u_read32(pAd, RX_STA_CNT0);
	RxStaCnt2.word = mt7612u_read32(pAd, RX_STA_CNT2);

	pAd->RalinkCounters.PhyErrCnt += RxStaCnt0.field.PhyErr;
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	{
		RxStaCnt1.word = mt7612u_read32(pAd, RX_STA_CNT1);
		/* Update RX PLCP error counter*/
		pAd->RalinkCounters.PlcpErrCnt += RxStaCnt1.field.PlcpErr;
		/* Update False CCA counter*/
		pAd->RalinkCounters.OneSecFalseCCACnt += RxStaCnt1.field.FalseCca;
		pAd->RalinkCounters.FalseCCACnt += RxStaCnt1.field.FalseCca;
	}

#ifdef STATS_COUNT_SUPPORT
	/* Update FCS counters*/
	OldValue= pAd->WlanCounters.FCSErrorCount.u.LowPart;
	pAd->WlanCounters.FCSErrorCount.u.LowPart += (RxStaCnt0.field.CrcErr); /* >> 7);*/
	if (pAd->WlanCounters.FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters.FCSErrorCount.u.HighPart++;
#endif /* STATS_COUNT_SUPPORT */

	/* Add FCS error count to private counters*/
	pRalinkCounters->OneSecRxFcsErrCnt += RxStaCnt0.field.CrcErr;
	OldValue = pRalinkCounters->RealFcsErrCount.u.LowPart;
	pRalinkCounters->RealFcsErrCount.u.LowPart += RxStaCnt0.field.CrcErr;
	if (pRalinkCounters->RealFcsErrCount.u.LowPart < OldValue)
		pRalinkCounters->RealFcsErrCount.u.HighPart++;

	/* Update Duplicate Rcv check*/
	pRalinkCounters->DuplicateRcv += RxStaCnt2.field.RxDupliCount;
#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.FrameDuplicateCount.u.LowPart += RxStaCnt2.field.RxDupliCount;
#endif /* STATS_COUNT_SUPPORT */
	/* Update RX Overflow counter*/
	pAd->Counters8023.RxNoBuffer += (RxStaCnt2.field.RxFifoOverflowCount);

	/*pAd->RalinkCounters.RxCount = 0;*/
	if (pRalinkCounters->RxCount != pAd->watchDogRxCnt)
	{
		pAd->watchDogRxCnt = pRalinkCounters->RxCount;
		pAd->watchDogRxOverFlowCnt = 0;
	}
	else
	{
		if (RxStaCnt2.field.RxFifoOverflowCount)
			pAd->watchDogRxOverFlowCnt++;
		else
			pAd->watchDogRxOverFlowCnt = 0;
	}


	/*if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED) || */
	/*	(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED) && (pAd->MacTab.Size != 1)))*/
	if (!pAd->bUpdateBcnCntDone)
	{
		/* Update BEACON sent count*/
		NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);
		StaTx2.word = mt7612u_read32(pAd, TX_STA_CNT2);
	}


	/*if (pAd->bStaFifoTest == true)*/
#ifdef STATS_COUNT_SUPPORT
	{
		TxAggCnt.word = mt7612u_read32(pAd, TX_AGG_CNT);
		TxAggCnt0.word = mt7612u_read32(pAd, TX_AGG_CNT0);
		TxAggCnt1.word = mt7612u_read32(pAd, TX_AGG_CNT1);
		TxAggCnt2.word = mt7612u_read32(pAd, TX_AGG_CNT2);
		TxAggCnt3.word = mt7612u_read32(pAd, TX_AGG_CNT3);
		TxAggCnt4.word = mt7612u_read32(pAd, TX_AGG_CNT4);
		TxAggCnt5.word = mt7612u_read32(pAd, TX_AGG_CNT5);
		TxAggCnt6.word = mt7612u_read32(pAd, TX_AGG_CNT6);
		TxAggCnt7.word = mt7612u_read32(pAd, TX_AGG_CNT7);
		pRalinkCounters->TxAggCount += TxAggCnt.field.AggTxCount;
		pRalinkCounters->TxNonAggCount += TxAggCnt.field.NonAggTxCount;
		pRalinkCounters->TxAgg1MPDUCount += TxAggCnt0.field.AggSize1Count;
		pRalinkCounters->TxAgg2MPDUCount += TxAggCnt0.field.AggSize2Count;

		pRalinkCounters->TxAgg3MPDUCount += TxAggCnt1.field.AggSize3Count;
		pRalinkCounters->TxAgg4MPDUCount += TxAggCnt1.field.AggSize4Count;
		pRalinkCounters->TxAgg5MPDUCount += TxAggCnt2.field.AggSize5Count;
		pRalinkCounters->TxAgg6MPDUCount += TxAggCnt2.field.AggSize6Count;

		pRalinkCounters->TxAgg7MPDUCount += TxAggCnt3.field.AggSize7Count;
		pRalinkCounters->TxAgg8MPDUCount += TxAggCnt3.field.AggSize8Count;
		pRalinkCounters->TxAgg9MPDUCount += TxAggCnt4.field.AggSize9Count;
		pRalinkCounters->TxAgg10MPDUCount += TxAggCnt4.field.AggSize10Count;

		pRalinkCounters->TxAgg11MPDUCount += TxAggCnt5.field.AggSize11Count;
		pRalinkCounters->TxAgg12MPDUCount += TxAggCnt5.field.AggSize12Count;
		pRalinkCounters->TxAgg13MPDUCount += TxAggCnt6.field.AggSize13Count;
		pRalinkCounters->TxAgg14MPDUCount += TxAggCnt6.field.AggSize14Count;

		pRalinkCounters->TxAgg15MPDUCount += TxAggCnt7.field.AggSize15Count;
		pRalinkCounters->TxAgg16MPDUCount += TxAggCnt7.field.AggSize16Count;

		/* Calculate the transmitted A-MPDU count*/
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += TxAggCnt0.field.AggSize1Count;
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt0.field.AggSize2Count >> 1);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt1.field.AggSize3Count / 3);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt1.field.AggSize4Count >> 2);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt2.field.AggSize5Count / 5);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt2.field.AggSize6Count / 6);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt3.field.AggSize7Count / 7);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt3.field.AggSize8Count >> 3);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt4.field.AggSize9Count / 9);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt4.field.AggSize10Count / 10);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt5.field.AggSize11Count / 11);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt5.field.AggSize12Count / 12);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt6.field.AggSize13Count / 13);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt6.field.AggSize14Count / 14);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt7.field.AggSize15Count / 15);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt7.field.AggSize16Count >> 4);
	}
#endif /* STATS_COUNT_SUPPORT */

#ifdef DBG_DIAGNOSE
	{
		RtmpDiagStruct *pDiag;
		u8 ArrayCurIdx, i;
		struct dbg_diag_info *diag_info;

		pDiag = &pAd->DiagStruct;
		ArrayCurIdx = pDiag->ArrayCurIdx;

		if (pDiag->inited == 0)
		{
			memset(pDiag, 0, sizeof(struct _RtmpDiagStrcut_));
			pDiag->ArrayStartIdx = pDiag->ArrayCurIdx = 0;
			pDiag->inited = 1;
		}
		else
		{
			diag_info = &pDiag->diag_info[ArrayCurIdx];

			/* Tx*/
			diag_info->TxFailCnt = TxStaCnt0.field.TxFailCount;
#ifdef DBG_TX_AGG_CNT
			diag_info->TxAggCnt = TxAggCnt.field.AggTxCount;
			diag_info->TxNonAggCnt = TxAggCnt.field.NonAggTxCount;

			diag_info->TxAMPDUCnt[0] = TxAggCnt0.field.AggSize1Count;
			diag_info->TxAMPDUCnt[1] = TxAggCnt0.field.AggSize2Count;
			diag_info->TxAMPDUCnt[2] = TxAggCnt1.field.AggSize3Count;
			diag_info->TxAMPDUCnt[3] = TxAggCnt1.field.AggSize4Count;
			diag_info->TxAMPDUCnt[4] = TxAggCnt2.field.AggSize5Count;
			diag_info->TxAMPDUCnt[5] = TxAggCnt2.field.AggSize6Count;
			diag_info->TxAMPDUCnt[6] = TxAggCnt3.field.AggSize7Count;
			diag_info->TxAMPDUCnt[7] = TxAggCnt3.field.AggSize8Count;
			diag_info->TxAMPDUCnt[8] = TxAggCnt4.field.AggSize9Count;
			diag_info->TxAMPDUCnt[9] = TxAggCnt4.field.AggSize10Count;
			diag_info->TxAMPDUCnt[10] = TxAggCnt5.field.AggSize11Count;
			diag_info->TxAMPDUCnt[11] = TxAggCnt5.field.AggSize12Count;
			diag_info->TxAMPDUCnt[12] = TxAggCnt6.field.AggSize13Count;
			diag_info->TxAMPDUCnt[13] = TxAggCnt6.field.AggSize14Count;
			diag_info->TxAMPDUCnt[14] = TxAggCnt7.field.AggSize15Count;
			diag_info->TxAMPDUCnt[15] = TxAggCnt7.field.AggSize16Count;
#endif /* DBG_TX_AGG_CNT */

			diag_info->RxCrcErrCnt = RxStaCnt0.field.CrcErr;

			INC_RING_INDEX(pDiag->ArrayCurIdx,  DIAGNOSE_TIME);
			ArrayCurIdx = pDiag->ArrayCurIdx;

			memset(&pDiag->diag_info[ArrayCurIdx], 0, sizeof(pDiag->diag_info[ArrayCurIdx]));

			if (pDiag->ArrayCurIdx == pDiag->ArrayStartIdx)
				INC_RING_INDEX(pDiag->ArrayStartIdx,  DIAGNOSE_TIME);
		}
	}
#endif /* DBG_DIAGNOSE */


}


int NICLoadFirmware(struct rtmp_adapter *ad)
{
	int ret	= NDIS_STATUS_SUCCESS;
	ULONG Old, New, Diff;

	RTMP_GetCurrentSystemTick(&Old);
	ret = mt7612u_mcu_usb_loadfw(ad);
	RTMP_GetCurrentSystemTick(&New);
	Diff = (New - Old) * 1000 / OS_HZ;
	DBGPRINT(RT_DEBUG_TRACE, ("load fw spent %ldms\n", Diff));

	return ret;
}


/*
	========================================================================

	Routine Description:
		Compare two memory block

	Arguments:
		pSrc1		Pointer to first memory address
		pSrc2		Pointer to second memory address

	Return Value:
		0:			memory is equal
		1:			pSrc1 memory is larger
		2:			pSrc2 memory is larger

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
ULONG RTMPCompareMemory(VOID *pSrc1, VOID *pSrc2, ULONG Length)
{
	u8 *pMem1;
	u8 *pMem2;
	ULONG	Index = 0;

	pMem1 = (u8 *) pSrc1;
	pMem2 = (u8 *) pSrc2;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] > pMem2[Index])
			return (1);
		else if (pMem1[Index] < pMem2[Index])
			return (2);
	}

	/* Equal*/
	return (0);
}


/*
	========================================================================

	Routine Description:
		Zero out memory block

	Arguments:
		pSrc1		Pointer to memory address
		Length		Size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPZeroMemory(VOID *pSrc, ULONG Length)
{
	u8 *pMem;
	ULONG	Index = 0;

	pMem = (u8 *) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem[Index] = 0x00;
	}
}


/*
	========================================================================

	Routine Description:
		Copy data from memory block 1 to memory block 2

	Arguments:
		pDest		Pointer to destination memory address
		pSrc		Pointer to source memory address
		Length		Copy size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPMoveMemory(VOID *pDest, VOID *pSrc, ULONG Length)
{
	u8 *pMem1;
	u8 *pMem2;
	UINT	Index;

	ASSERT((Length==0) || (pDest && pSrc));

	pMem1 = (u8 *) pDest;
	pMem2 = (u8 *) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem1[Index] = pMem2[Index];
	}
}


VOID UserCfgExit(struct rtmp_adapter *pAd)
{
#ifdef RT_CFG80211_SUPPORT
	/* Reset the CFG80211 Internal Flag */
	RTMP_DRIVER_80211_RESET(pAd);
#endif /* RT_CFG80211_SUPPORT */

	BATableExit(pAd);
}


/*
	========================================================================

	Routine Description:
		Initialize port configuration structure

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID UserCfgInit(struct rtmp_adapter *pAd)
{
	UINT i;
#ifdef CONFIG_AP_SUPPORT
	UINT j;
#endif /* CONFIG_AP_SUPPORT */
	UINT key_index, bss_index;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	DBGPRINT(RT_DEBUG_TRACE, ("--> UserCfgInit\n"));

	pAd->IndicateMediaState = NdisMediaStateDisconnected;

	/* part I. intialize common configuration */
	pAd->CommonCfg.BasicRateBitmap = 0xF;
	pAd->CommonCfg.BasicRateBitmapOld = 0xF;

	pAd->BulkOutReq = 0;

	pAd->BulkOutComplete = 0;
	pAd->BulkOutCompleteOther = 0;
	pAd->BulkOutCompleteCancel = 0;
	pAd->BulkInReq = 0;
	pAd->BulkInComplete = 0;
	pAd->BulkInCompleteFail = 0;

	/*pAd->QuickTimerP = 100;*/
	/*pAd->TurnAggrBulkInCount = 0;*/
	pAd->bUsbTxBulkAggre = 0;


	pAd->CommonCfg.MaxPktOneTxBulk = 2;
	pAd->CommonCfg.TxBulkFactor = 1;
	pAd->CommonCfg.RxBulkFactor =1;

	pAd->CommonCfg.TxPower = 100; /*mW*/

	memset(&pAd->CommonCfg.IOTestParm, 0, sizeof(pAd->CommonCfg.IOTestParm));
#ifdef CONFIG_STA_SUPPORT
	pAd->CountDowntoPsm = 0;
	pAd->StaCfg.Connectinfoflag = false;
#endif /* CONFIG_STA_SUPPORT */


	for(key_index=0; key_index<SHARE_KEY_NUM; key_index++)
	{
		for(bss_index = 0; bss_index < MAX_MBSSID_NUM(pAd) + MAX_P2P_NUM; bss_index++)
		{
			pAd->SharedKey[bss_index][key_index].KeyLen = 0;
			pAd->SharedKey[bss_index][key_index].CipherAlg = CIPHER_NONE;
		}
	}

	pAd->EepromAccess = false;

	pAd->Antenna.word = 0;
	pAd->CommonCfg.BBPCurrentBW = BW_20;

	pAd->RfIcType = RFIC_6352;	/* ULLI : lowest possible */

	/* Init timer for reset complete event*/
	pAd->CommonCfg.CentralChannel = 1;
	pAd->bForcePrintTX = false;
	pAd->bForcePrintRX = false;
	pAd->bStaFifoTest = false;
	pAd->bProtectionTest = false;
	pAd->bHCCATest = false;
	pAd->bGenOneHCCA = false;
	pAd->CommonCfg.Dsifs = 10;      /* in units of usec */
	pAd->CommonCfg.TxPower = 100; /* mW*/
	pAd->CommonCfg.TxPowerPercentage = 0xffffffff; /* AUTO*/
	pAd->CommonCfg.TxPowerDefault = 0xffffffff; /* AUTO*/
	pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto; /* use Long preamble on TX by defaut*/
	pAd->CommonCfg.bUseZeroToDisableFragment = false;
	pAd->CommonCfg.RtsThreshold = 2347;
	pAd->CommonCfg.FragmentThreshold = 2346;
	pAd->CommonCfg.UseBGProtection = 0;    /* 0: AUTO*/
	pAd->CommonCfg.bEnableTxBurst = true; /* 0;    	*/
	pAd->CommonCfg.PhyMode = 0xff;     /* unknown*/
	pAd->CommonCfg.SavedPhyMode = pAd->CommonCfg.PhyMode;

	pAd->wmm_cw_min = 4;
	switch (pAd->OpMode)
	{
		case OPMODE_AP:
			pAd->wmm_cw_max = 6;
			break;
		case OPMODE_STA:
			pAd->wmm_cw_max = 10;
			break;
	}


#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	pAd->ApCfg.ACSCheckTime = 0;
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	pAd->CommonCfg.bNeedSendTriggerFrame = false;
	pAd->CommonCfg.TriggerTimerCount = 0;
	pAd->CommonCfg.bAPSDForcePowerSave = false;
	/*pAd->CommonCfg.bCountryFlag = false;*/
	pAd->CommonCfg.TxStream = 0;
	pAd->CommonCfg.RxStream = 0;

	memset(&pAd->BeaconTxWI, 0, TXWISize);

	if (IS_MT76x2(pAd))
		pAd->CommonCfg.b256QAM_2G = true;
	else
		pAd->CommonCfg.b256QAM_2G = false;

	memset(&pAd->CommonCfg.HtCapability, 0, sizeof(pAd->CommonCfg.HtCapability));
	pAd->HTCEnable = false;
	pAd->bBroadComHT = false;
	pAd->CommonCfg.bRdg = false;

	pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
	pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
	pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
	pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
	pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
	pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (pAd->CommonCfg.Dot11BssWidthTriggerScanInt * pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);

	pAd->CommonCfg.bBssCoexEnable = true; /* by default, we enable this feature, you can disable it via the profile or ioctl command*/
	pAd->CommonCfg.BssCoexApCntThr = 0;
	pAd->CommonCfg.Bss2040NeedFallBack = 0;

	pAd->CommonCfg.bRcvBSSWidthTriggerEvents = false;

	memset(&pAd->CommonCfg.AddHTInfo, 0, sizeof(pAd->CommonCfg.AddHTInfo));
	pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_DISABLE;
	pAd->CommonCfg.BACapability.field.MpduDensity = 0;
	pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
	pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64; /*32;*/
	pAd->CommonCfg.BACapability.field.TxBAWinLimit = 64; /*32;*/
	DBGPRINT(RT_DEBUG_TRACE, ("--> UserCfgInit. BACapability = 0x%x\n", pAd->CommonCfg.BACapability.word));

	pAd->CommonCfg.BACapability.field.AutoBA = false;
	BATableInit(pAd, &pAd->BATable);

	pAd->CommonCfg.bExtChannelSwitchAnnouncement = 1;
	pAd->CommonCfg.bHTProtect = 1;
	pAd->CommonCfg.bMIMOPSEnable = true;
	pAd->CommonCfg.bBADecline = false;
	pAd->CommonCfg.bDisableReordering = false;

	if (pAd->MACVersion == 0x28720200)
		pAd->CommonCfg.TxBASize = 13; /*by Jerry recommend*/
	else
		pAd->CommonCfg.TxBASize = 7;

	pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;

	pAd->CommonCfg.TxRate = RATE_6;

	pAd->CommonCfg.MlmeTransmit.field.MCS = MCS_RATE_6;
	pAd->CommonCfg.MlmeTransmit.field.BW = BW_20;
	pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;

	pAd->CommonCfg.BeaconPeriod = 100;     /* in mSec*/



	pAd->CommonCfg.ETxBfNoncompress = 0;
	pAd->CommonCfg.ETxBfIncapable = 0;

#ifdef NEW_RATE_ADAPT_SUPPORT
	pAd->CommonCfg.lowTrafficThrd = 2;
	pAd->CommonCfg.TrainUpRule = 2; // 1;
	pAd->CommonCfg.TrainUpRuleRSSI = -70; // 0;
	pAd->CommonCfg.TrainUpLowThrd = 90;
	pAd->CommonCfg.TrainUpHighThrd = 110;
#endif /* NEW_RATE_ADAPT_SUPPORT */



#ifdef CFO_TRACK
#endif /* CFO_TRACK */

#ifdef DBG_CTRL_SUPPORT
	pAd->CommonCfg.DebugFlags = 0;
#endif /* DBG_CTRL_SUPPORT */

	/* WFA policy - disallow TH rate in WEP or TKIP cipher */
	pAd->CommonCfg.HT_DisallowTKIP = true;

	/* Frequency for rate adaptation */
	pAd->ra_interval = DEF_RA_TIME_INTRVAL;
	pAd->ra_fast_interval = DEF_QUICK_RA_TIME_INTERVAL;

	/* Tx Sw queue length setting */
	pAd->TxSwQMaxLen = MAX_PACKETS_IN_QUEUE;

	pAd->CommonCfg.bRalinkBurstMode = false;

#ifdef CONFIG_STA_SUPPORT
	/* part II. intialize STA specific configuration*/
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_DIRECT);
		RX_FILTER_CLEAR_FLAG(pAd, fRX_FILTER_ACCEPT_MULTICAST);
		RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_BROADCAST);
		RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_ALL_MULTICAST);

		pAd->StaCfg.Psm = PWR_ACTIVE;

		pAd->StaCfg.PairCipher = Ndis802_11EncryptionDisabled;
		pAd->StaCfg.GroupCipher = Ndis802_11EncryptionDisabled;
		pAd->StaCfg.bMixCipher = false;
		pAd->StaCfg.wdev.DefaultKeyId = 0;

		/* 802.1x port control*/
		pAd->StaCfg.PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pAd->StaCfg.wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		pAd->StaCfg.LastMicErrorTime = 0;
		pAd->StaCfg.MicErrCnt        = 0;
		pAd->StaCfg.bBlockAssoc      = false;
		pAd->StaCfg.WpaState         = SS_NOTUSE;

		pAd->CommonCfg.NdisRadioStateOff = false;		/* New to support microsoft disable radio with OID command*/

		pAd->StaCfg.RssiTrigger = 0;
		memset(&pAd->StaCfg.RssiSample, 0, sizeof(RSSI_SAMPLE));
		pAd->StaCfg.RssiTriggerMode = RSSI_TRIGGERED_UPON_BELOW_THRESHOLD;
		pAd->StaCfg.AtimWin = 0;
		pAd->StaCfg.DefaultListenCount = 3;/*default listen count;*/
		pAd->StaCfg.BssType = BSS_INFRA;  /* BSS_INFRA or BSS_ADHOC or BSS_MONITOR*/
		pAd->StaCfg.bSkipAutoScanConn = false;
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);

		pAd->StaCfg.wdev.bAutoTxRateSwitch = true;
		pAd->StaCfg.wdev.DesiredTransmitSetting.field.MCS = MCS_AUTO;
		pAd->StaCfg.bAutoConnectIfNoSSID = false;
#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
		pAd->StaCfg.AdaptiveFreq = true; /* Todo: iwpriv and profile support. */
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */
	}

#endif /* CONFIG_STA_SUPPORT */

	/* global variables mXXXX used in MAC protocol state machines*/
	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_INFRA_ON);

	/* PHY specification*/
	pAd->CommonCfg.PhyMode = (WMODE_B | WMODE_G);		/* default PHY mode*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);  /* CCK use LONG preamble*/

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* user desired power mode*/
		pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeCAM;
		pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
		pAd->StaCfg.bWindowsACCAMEnable = false;

		pAd->StaCfg.bHwRadio = true; /* Default Hardware Radio status is On*/
		pAd->StaCfg.bSwRadio = true; /* Default Software Radio status is On*/
		pAd->StaCfg.bRadio = true; /* bHwRadio && bSwRadio*/
		pAd->StaCfg.bHardwareRadio = false;		/* Default is OFF*/
		pAd->StaCfg.bShowHiddenSSID = false;		/* Default no show*/

		/* Nitro mode control*/
#if defined(NATIVE_WPA_SUPPLICANT_SUPPORT) || defined(RT_CFG80211_SUPPORT)
		pAd->StaCfg.bAutoReconnect = false;
#else
		pAd->StaCfg.bAutoReconnect = true;
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT || RT_CFG80211_SUPPORT*/

		/* Save the init time as last scan time, the system should do scan after 2 seconds.*/
		/* This patch is for driver wake up from standby mode, system will do scan right away.*/
		NdisGetSystemUpTime(&pAd->StaCfg.LastScanTime);
		if (pAd->StaCfg.LastScanTime > 10 * OS_HZ)
			pAd->StaCfg.LastScanTime -= (10 * OS_HZ);

#ifdef PROFILE_STORE
		pAd->bWriteDat = false;
#endif /* PROFILE_STORE */
#ifdef WPA_SUPPLICANT_SUPPORT
		pAd->StaCfg.wdev.IEEE8021X = false;
		pAd->StaCfg.wpa_supplicant_info.IEEE8021x_required_keys = false;
		pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
		pAd->StaCfg.wpa_supplicant_info.bRSN_IE_FromWpaSupplicant = false;

#if defined(NATIVE_WPA_SUPPLICANT_SUPPORT) || defined(RT_CFG80211_SUPPORT)
		pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;
#ifdef PROFILE_STORE
		pAd->bWriteDat = true;
#endif /* PROFILE_STORE */
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT || RT_CFG80211_SUPPORT */

		pAd->StaCfg.wpa_supplicant_info.bLostAp = false;
		pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe = NULL;
		pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen = 0;
		pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe = NULL;
		pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen = 0;
		pAd->StaCfg.wpa_supplicant_info.WpaSupplicantScanCount = 0;
#endif /* WPA_SUPPLICANT_SUPPORT */

		memset(pAd->StaCfg.ReplayCounter, 0, 8);


		pAd->StaCfg.bAutoConnectByBssid = false;
		pAd->StaCfg.BeaconLostTime = BEACON_LOST_TIME;
		memset(pAd->StaCfg.WpaPassPhrase, 0, 64);
		pAd->StaCfg.WpaPassPhraseLen = 0;
		pAd->StaCfg.bAutoRoaming = false;
		pAd->StaCfg.bForceTxBurst = false;
		pAd->StaCfg.bNotFirstScan = false;
		pAd->StaCfg.bImprovedScan = false;
		pAd->StaCfg.bAdhocN = true;
		pAd->StaCfg.bFastConnect = false;
		pAd->StaCfg.bAdhocCreator = false;
	}
#endif /* CONFIG_STA_SUPPORT */

	/* Default for extra information is not valid*/
	pAd->ExtraInfo = EXTRA_INFO_CLEAR;

	/* Default Config change flag*/
	pAd->bConfigChanged = false;

	/*
		part III. AP configurations
	*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* Set MBSS Default Configurations*/
		pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
		for(j = BSS0; j < pAd->ApCfg.BssidNum; j++)
		{
			MULTISSID_STRUCT *mbss = &pAd->ApCfg.MBSSID[j];
			struct rtmp_wifi_dev *wdev = &pAd->ApCfg.MBSSID[j].wdev;

			mbss->AssocReqFailRssiThreshold = 0;
			mbss->AssocReqNoRspRssiThreshold = 0;
			mbss->AuthFailRssiThreshold = 0;
			mbss->AuthNoRspRssiThreshold = 0;
			mbss->RssiLowForStaKickOut = 0;

			mbss->StatTxRetryOkCount = 0;
			mbss->StatTxFailCount = 0;

			wdev->AuthMode = Ndis802_11AuthModeOpen;
			wdev->WepStatus = Ndis802_11EncryptionDisabled;
			wdev->GroupKeyWepStatus = Ndis802_11EncryptionDisabled;
			wdev->DefaultKeyId = 0;
			wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;
			mbss->RekeyCountDown = 0;	/* it's used for WPA rekey */

			mbss->ProbeRspTimes = 3;
#ifdef SPECIFIC_TX_POWER_SUPPORT
			if (IS_RT6352(pAd) || IS_MT76x2(pAd))
				mbss->TxPwrAdj = -1;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

#ifdef DOT1X_SUPPORT
			mbss->wdev.IEEE8021X = false;
			mbss->PreAuth = false;

			/* PMK cache setting*/
			mbss->PMKCachePeriod = (10 * 60 * OS_HZ); /* unit : tick(default: 10 minute)*/
			memset(&mbss->PMKIDCache, 0, sizeof(NDIS_AP_802_11_PMKID));

			/* dot1x related per BSS */
			mbss->radius_srv_num = 0;
			mbss->NasIdLen = 0;
#endif /* DOT1X_SUPPORT */

			/* VLAN related */
			mbss->wdev.VLAN_VID = 0;

			/* Default MCS as AUTO*/
			wdev->bAutoTxRateSwitch = true;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;

			/* Default is zero. It means no limit.*/
			mbss->MaxStaNum = 0;
			mbss->StaCount = 0;



			for(i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
	        		mbss->TimBitmaps[i] = 0;
		}
		pAd->ApCfg.DtimCount  = 0;
		pAd->ApCfg.DtimPeriod = DEFAULT_DTIM_PERIOD;

		pAd->ApCfg.ErpIeContent = 0;

		pAd->ApCfg.StaIdleTimeout = MAC_TABLE_AGEOUT_TIME;




#ifdef APCLI_SUPPORT
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = false;
		pAd->ApCfg.ApCliNum = MAX_APCLI_NUM;
#ifdef APCLI_CERT_SUPPORT
		pAd->bApCliCertTest = false;
#endif /* APCLI_CERT_SUPPORT */
		for(j = 0; j < MAX_APCLI_NUM; j++)
		{
			APCLI_STRUCT *apcli_entry = &pAd->ApCfg.ApCliTab[j];
			struct rtmp_wifi_dev *wdev = &apcli_entry->wdev;

			wdev->AuthMode = Ndis802_11AuthModeOpen;
			wdev->WepStatus = Ndis802_11WEPDisabled;
			wdev->bAutoTxRateSwitch = true;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			apcli_entry->UapsdInfo.bAPSDCapable = false;
#ifdef APCLI_CONNECTION_TRIAL
			apcli_entry->TrialCh = 0;//if the channel is 0, AP will connect the rootap is in the same channel with ra0.
#endif /* APCLI_CONNECTION_TRIAL */

#ifdef WPA_SUPPLICANT_SUPPORT
			apcli_entry->wdev.IEEE8021X=false;
			apcli_entry->wpa_supplicant_info.IEEE8021x_required_keys=false;
			apcli_entry->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant=false;
			apcli_entry->wpa_supplicant_info.bLostAp=false;
			apcli_entry->bScanReqIsFromWebUI=false;
			apcli_entry->bConfigChanged=false;
			apcli_entry->wpa_supplicant_info.DesireSharedKeyId=0;
			apcli_entry->wpa_supplicant_info.WpaSupplicantUP=WPA_SUPPLICANT_DISABLE;
			apcli_entry->wpa_supplicant_info.WpaSupplicantScanCount=0;
			apcli_entry->wpa_supplicant_info.pWpsProbeReqIe=NULL;
			apcli_entry->wpa_supplicant_info.WpsProbeReqIeLen=0;
			apcli_entry->wpa_supplicant_info.pWpaAssocIe=NULL;
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen=0;
			apcli_entry->SavedPMKNum=0;
			RTMPZeroMemory(apcli_entry->SavedPMK, (PMKID_NO * sizeof(BSSID_INFO)));
#endif/*WPA_SUPPLICANT_SUPPORT*/
			apcli_entry->bBlockAssoc=false;
			apcli_entry->MicErrCnt=0;
		}
#endif /* APCLI_SUPPORT */
		pAd->ApCfg.EntryClientCount = 0;
		pAd->ApCfg.ChangeTxOpClient = 0;
	}

	if (IS_MT76x2(pAd)) {
		pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = false;
		pAd->CommonCfg.lna_vga_ctl.nFalseCCATh = 800;
		pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh = 10;
	}

#endif /* CONFIG_AP_SUPPORT */


#ifdef IP_ASSEMBLY
	if (pAd->OpMode == OPMODE_STA)
	{
		pAd->StaCfg.bFragFlag = true;
	}
#endif /* IP_ASSEMBLY */

	/*
		part IV. others
	*/

	/* dynamic BBP R66:sensibity tuning to overcome background noise*/
	pAd->BbpTuning.bEnable = true;
	pAd->BbpTuning.FalseCcaLowerThreshold = 100;
	pAd->BbpTuning.FalseCcaUpperThreshold = 512;
	pAd->BbpTuning.R66Delta = 4;
	pAd->Mlme.bEnableAutoAntennaCheck = true;

	/* Also initial R66CurrentValue, RTUSBResumeMsduTransmission might use this value.*/
	/* if not initial this value, the default value will be 0.*/
	pAd->BbpTuning.R66CurrentValue = 0x38;

	pAd->BbpForCCK = false;

	/* initialize MAC table and allocate spin lock*/
	memset(&pAd->MacTab, 0, sizeof(MAC_TABLE));
	InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
	spin_lock_init(&pAd->MacTabLock);

	/*RTMPInitTimer(pAd, &pAd->RECBATimer, RECBATimerTimeout, pAd, true);*/
	/*RTMPSetTimer(&pAd->RECBATimer, REORDER_EXEC_INTV);*/


	pAd->CommonCfg.bWiFiTest = false;

#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.EntryLifeCheck = MAC_ENTRY_LIFE_CHECK_CNT;

#endif /* CONFIG_AP_SUPPORT */

	pAd->RxAnt.Pair1PrimaryRxAnt = 0;
	pAd->RxAnt.Pair1SecondaryRxAnt = 1;

		pAd->RxAnt.EvaluatePeriod = 0;
		pAd->RxAnt.RcvPktNumWhenEvaluate = 0;
#ifdef CONFIG_STA_SUPPORT
		pAd->RxAnt.Pair1AvgRssi[0] = pAd->RxAnt.Pair1AvgRssi[1] = 0;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		pAd->RxAnt.Pair1AvgRssiGroup1[0] = pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
		pAd->RxAnt.Pair1AvgRssiGroup2[0] = pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
#endif /* CONFIG_AP_SUPPORT */

#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++)
	{
		BSS_ENTRY *pBssEntry = &pAd->ScanTab.BssEntry[i];

		if (pAd->ProbeRespIE[i].pIe)
			pBssEntry->pVarIeFromProbRsp = pAd->ProbeRespIE[i].pIe;
		else
			pBssEntry->pVarIeFromProbRsp = NULL;
	}
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */

#if defined(WOW_SUPPORT) || defined(NEW_WOW_SUPPORT)
	pAd->WOW_Cfg.bEnable = false;
	pAd->WOW_Cfg.bWOWFirmware = false;	/* load normal firmware */
	pAd->WOW_Cfg.bInBand = true;		/* use in-band signal */
	pAd->WOW_Cfg.nSelectedGPIO = 1;
	pAd->WOW_Cfg.nDelay = 3; /* (3+1)*3 = 12 sec */
	pAd->WOW_Cfg.nHoldTime = 1; /* 1*10 = 10 ms */
	DBGPRINT(RT_DEBUG_OFF, ("WOW Enable %d, WOWFirmware %d\n", pAd->WOW_Cfg.bEnable, pAd->WOW_Cfg.bWOWFirmware));
#endif /* defined(WOW_SUPPORT) || defined(NEW_WOW_SUPPORT) */

	/* 802.11H and DFS related params*/
	pAd->Dot11_H.CSCount = 0;
	pAd->Dot11_H.CSPeriod = 10;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
#endif

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pAd->Dot11_H.RDMode = RD_SILENCE_MODE;
#endif

	pAd->Dot11_H.ChMovingTime = 65;
	pAd->Dot11_H.bDFSIndoor = 1;



#ifdef AP_PARTIAL_SCAN_SUPPORT
	pAd->ApCfg.bPartialScanning = false;
	pAd->ApCfg.PartialScanChannelNum = DEFLAUT_PARTIAL_SCAN_CH_NUM;
	pAd->ApCfg.LastPartialScanChannel = 0;
	pAd->ApCfg.PartialScanBreakTime = 0;
#endif /* AP_PARTIAL_SCAN_SUPPORT */


#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	pAd->ApCfg.ApCliAutoConnectRunning= false;
	pAd->ApCfg.ApCliAutoConnectChannelSwitching = false;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#endif /* APCLI_SUPPORT */


	pAd->CommonCfg.bNonVhtDisallow = false;

	pAd->usb_ctl.usb_aggregation = true;


	DBGPRINT(RT_DEBUG_TRACE, ("<-- UserCfgInit\n"));
}


/* IRQL = PASSIVE_LEVEL*/
u8 BtoH(STRING ch)
{
	if (ch >= '0' && ch <= '9') return (ch - '0');        /* Handle numerals*/
	if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  /* Handle capitol hex digits*/
	if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  /* Handle small hex digits*/
	return(255);
}


/*
	FUNCTION: AtoH(char *, u8 *, int)

	PURPOSE:  Converts ascii string to network order hex

	PARAMETERS:
		src    - pointer to input ascii string
		dest   - pointer to output hex
		destlen - size of dest

	COMMENTS:

		2 ascii bytes make a hex byte so must put 1st ascii byte of pair
		into upper nibble and 2nd ascii byte of pair into lower nibble.

	IRQL = PASSIVE_LEVEL
*/
void AtoH(char *src, u8 *dest, int destlen)
{
	char *srcptr;
	u8 *destTemp;

	srcptr = src;
	destTemp = (u8 *) dest;

	while(destlen--)
	{
		*destTemp = BtoH(*srcptr++) << 4;    /* Put 1st ascii byte in upper nibble.*/
		*destTemp += BtoH(*srcptr++);      /* Add 2nd ascii byte to above.*/
		destTemp++;
	}
}


/*
========================================================================
Routine Description:
	Add a timer to the timer list.

Arguments:
	pAd				- WLAN control block pointer
	pRsc			- the OS resource

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_TimerListAdd(struct rtmp_adapter *pAd, VOID *pRsc)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;


	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	while(1)
	{
		if (pObj == NULL)
			break;
		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc)
			return; /* exists */
		pObj = pObj->pNext;
	}

	/* allocate a timer record entry */
	pObj = kmalloc(sizeof(LIST_RESOURCE_OBJ_ENTRY), GFP_ATOMIC);
	if (pObj == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s: alloc timer obj fail!\n", __FUNCTION__));
		return;
	}
	else
	{
		pObj->pRscObj = pRsc;
		insertTailList(pRscList, (LIST_ENTRY *)pObj);
		DBGPRINT(RT_DEBUG_WARN, ("%s: add timer obj %lx!\n", __FUNCTION__, (ULONG)pRsc));
	}
}


VOID RTMP_TimerListRelease(struct rtmp_adapter *pAd, VOID *pRsc)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	LIST_ENTRY *pListEntry;

	pListEntry = pRscList->pHead;
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;

	while (pObj)
	{
		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc)
		{
			pListEntry = (LIST_ENTRY *)pObj;
			break;
		}

		pListEntry = pListEntry->pNext;
		pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;
	}

	if (pListEntry)
	{
		delEntryList(pRscList, pListEntry);

		/* free a timer record entry */
		DBGPRINT(RT_DEBUG_ERROR, ("%s: release timer obj %lx!\n", __FUNCTION__, (ULONG)pRsc));
		kfree(pObj);
	}
}

/*
========================================================================
Routine Description:
	Cancel all timers in the timer list.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_AllTimerListRelease(struct rtmp_adapter *pAd)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj, *pObjOld;
	bool Cancel;

	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	while(1)
	{
		if (pObj == NULL)
			break;
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Cancel timer obj %lx!\n", __FUNCTION__, (ULONG)(pObj->pRscObj)));
		pObjOld = pObj;
		pObj = pObj->pNext;
		RTMPReleaseTimer(pObjOld->pRscObj, &Cancel);

		//TODO: It will casued kernel panic on reboot
		//kfree(pObjOld);
	}

	/* reset TimerList */
	initList(&pAd->RscTimerCreateList);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pAd			Pointer to our adapter
		pTimer				Timer structure
		pTimerFunc			Function to execute when timer expired
		Repeat				Ture for period timer

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPInitTimer(
	IN	struct rtmp_adapter *pAd,
	IN	RALINK_TIMER_STRUCT *pTimer,
	IN	VOID *pTimerFunc,
	IN	VOID *pData,
	IN	bool	 Repeat)
{
	RTMP_SEM_LOCK(&TimerSemLock);

	RTMP_TimerListAdd(pAd, pTimer);


	/* Set Valid to true for later used.*/
	/* It will crash if we cancel a timer or set a timer */
	/* that we haven't initialize before.*/
	/* */
	pTimer->Valid      = true;

	pTimer->PeriodicType = Repeat;
	pTimer->State      = false;
	pTimer->cookie = (ULONG) pData;
	pTimer->pAd = pAd;

	RTMP_OS_Init_Timer(pAd, &pTimer->TimerObj,	pTimerFunc, (PVOID) pTimer, &pAd->RscTimerMemList);
	DBGPRINT(RT_DEBUG_TRACE,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));

	RTMP_SEM_UNLOCK(&TimerSemLock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPSetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	RTMP_SEM_LOCK(&TimerSemLock);

	if (pTimer->Valid)
	{
		struct rtmp_adapter *pAd;

		pAd = (struct rtmp_adapter *)pTimer->pAd;
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		{
			DBGPRINT_ERR(("RTMPSetTimer failed, Halt in Progress!\n"));
			RTMP_SEM_UNLOCK(&TimerSemLock);
			return;
		}

		pTimer->TimerValue = Value;
		pTimer->State      = false;
		if (pTimer->PeriodicType == true)
		{
			pTimer->Repeat = true;
			RTMP_SetPeriodicTimer(&pTimer->TimerObj, Value);
		}
		else
		{
			pTimer->Repeat = false;
			RTMP_OS_Add_Timer(&pTimer->TimerObj, Value);
		}

		DBGPRINT(RT_DEBUG_INFO,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		DBGPRINT_ERR(("RTMPSetTimer failed, Timer hasn't been initialize!\n"));
	}
	RTMP_SEM_UNLOCK(&TimerSemLock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPModTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	bool	Cancel;


	RTMP_SEM_LOCK(&TimerSemLock);

	if (pTimer->Valid)
	{
		pTimer->TimerValue = Value;
		pTimer->State      = false;
		if (pTimer->PeriodicType == true)
		{
			RTMP_SEM_UNLOCK(&TimerSemLock);
			RTMPCancelTimer(pTimer, &Cancel);
			RTMPSetTimer(pTimer, Value);
		}
		else
		{
			RTMP_OS_Mod_Timer(&pTimer->TimerObj, Value);
			RTMP_SEM_UNLOCK(&TimerSemLock);
		}
		DBGPRINT(RT_DEBUG_TRACE,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		DBGPRINT_ERR(("RTMPModTimer failed, Timer hasn't been initialize!\n"));
		RTMP_SEM_UNLOCK(&TimerSemLock);
	}
}


/*
	========================================================================

	Routine Description:
		Cancel timer objects

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:
		1.) To use this routine, must call RTMPInitTimer before.
		2.) Reset NIC to initial state AS IS system boot up time.

	========================================================================
*/
VOID RTMPCancelTimer(RALINK_TIMER_STRUCT *pTimer, bool *pCancelled)
{
	// TODO: shiang-usw, check the purpose of this SemLock!
	RTMP_SEM_LOCK(&TimerSemLock);

	if (pTimer->Valid)
	{
		if (pTimer->State == false)
			pTimer->Repeat = false;

		RTMP_SEM_UNLOCK(&TimerSemLock);
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		RTMP_SEM_LOCK(&TimerSemLock);

		if (*pCancelled == true)
			pTimer->State = true;

#ifdef RTMP_TIMER_TASK_SUPPORT
		/* We need to go-through the TimerQ to findout this timer handler and remove it if */
		/*		it's still waiting for execution.*/
		RtmpTimerQRemove(pTimer->pAd, pTimer);
#endif /* RTMP_TIMER_TASK_SUPPORT */

		DBGPRINT(RT_DEBUG_INFO,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		DBGPRINT(RT_DEBUG_INFO,("RTMPCancelTimer failed, Timer hasn't been initialize!\n"));
	}

	RTMP_SEM_UNLOCK(&TimerSemLock);
}


VOID RTMPReleaseTimer(RALINK_TIMER_STRUCT *pTimer, bool *pCancelled)
{
	RTMP_SEM_LOCK(&TimerSemLock);

	if (pTimer->Valid)
	{
		if (pTimer->State == false)
			pTimer->Repeat = false;

		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);

		if (*pCancelled == true)
			pTimer->State = true;

#ifdef RTMP_TIMER_TASK_SUPPORT
		/* We need to go-through the TimerQ to findout this timer handler and remove it if */
		/*		it's still waiting for execution.*/
		RtmpTimerQRemove(pTimer->pAd, pTimer);
#endif /* RTMP_TIMER_TASK_SUPPORT */

		/* release timer */
		RTMP_OS_Release_Timer(&pTimer->TimerObj);

		pTimer->Valid = false;

		RTMP_TimerListRelease(pTimer->pAd, pTimer);

		DBGPRINT(RT_DEBUG_INFO,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		DBGPRINT(RT_DEBUG_INFO,("RTMPReleasefailed, Timer hasn't been initialize!\n"));
	}

	RTMP_SEM_UNLOCK(&TimerSemLock);
}


/*
	========================================================================

	Routine Description:
		Enable RX

	Arguments:
		pAd						Pointer to our adapter

	Return Value:
		None

	IRQL <= DISPATCH_LEVEL

	Note:
		Before Enable RX, make sure you have enabled Interrupt.
	========================================================================
*/
VOID RTMPEnableRxTx(struct rtmp_adapter *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPEnableRxTx\n"));

	RT28XXDMAEnable(pAd);

	AsicSetRxFilter(pAd);

	{
		mt7612u_write32(pAd, MAC_SYS_CTRL, 0xc);
//+++Add by shiang for debug for pbf_loopback
//			mt7612u_write32(pAd, MAC_SYS_CTRL, 0x2c);
//---Add by shiang for debug
//+++Add by shiang for debug invalid RxWI->WCID
//---Add by shiang for  debug invalid RxWI->WCID
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPEnableRxTx\n"));
}


void CfgInitHook(struct rtmp_adapter *pAd)
{
	/*pAd->bBroadComHT = true;*/
}


static INT RtmpChipOpsRegister(struct rtmp_adapter *pAd)
{
	int ret = 0;

	memset(&pAd->chipCap, 0, sizeof(struct rtmp_chip_cap));

	ret = RtmpChipOpsHook(pAd);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, ("chipOps hook error\n"));
		return ret;
	}

	get_dev_config_idx(pAd);

	return ret;
}


bool PairEP(struct rtmp_adapter *pAd, UINT8 EP)
{
	struct rtmp_chip_cap *pChipCap = &pAd->chipCap;
	int i;
	int found = 0;

	if (EP == pChipCap->CommandBulkOutAddr) {
		DBGPRINT(RT_DEBUG_OFF, ("Endpoint(%x) is for In-band Command\n", EP));
		found = 1;
	}

	for (i = 0; i < 4; i++) {
		if (EP == pChipCap->WMM0ACBulkOutAddr[i]) {
			DBGPRINT(RT_DEBUG_OFF, ("Endpoint(%x) is for WMM0 AC%d\n", EP, i));
			found = 1;
		}
	}

	if (EP == pChipCap->DataBulkInAddr) {
		DBGPRINT(RT_DEBUG_OFF, ("Endpoint(%x) is for Data-In\n", EP));
		found = 1;
	}

	if (EP == pChipCap->CommandRspBulkInAddr) {
		DBGPRINT(RT_DEBUG_OFF, ("Endpoint(%x) is for Command Rsp\n", EP));
		found = 1;
	}

	if (!found) {
		DBGPRINT(RT_DEBUG_OFF, ("Endpoint(%x) do not pair\n", EP));
		return false;
	} else {
		return true;
	}
}


INT RtmpRaDevCtrlInit(struct rtmp_adapter *pAd)
{
	UINT8 i;
	uint32_t ret;

#ifdef CONFIG_STA_SUPPORT
	pAd->OpMode = OPMODE_STA;
	DBGPRINT(RT_DEBUG_TRACE, ("STA Driver version-%s\n", STA_DRIVER_VERSION));
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	pAd->OpMode = OPMODE_AP;
	DBGPRINT(RT_DEBUG_TRACE, ("AP Driver version-%s\n", AP_DRIVER_VERSION));
#endif /* CONFIG_AP_SUPPORT */

	sema_init(&(pAd->UsbVendorReq_semaphore), 1);
	sema_init(&(pAd->reg_atomic), 1);
	sema_init(&(pAd->hw_atomic), 1);
	sema_init(&(pAd->mcu_atomic), 1);
	sema_init(&(pAd->tssi_lock), 1);

	pAd->UsbVendorReqBuf = kmalloc(MAX_PARAM_BUFFER_SIZE - 1, GFP_ATOMIC);
	if (pAd->UsbVendorReqBuf == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate vendor request temp buffer failed!\n"));
		return false;
	}

	if (RtmpChipOpsRegister(pAd))
		return false;

	for (i = 0; i < 6; i++)
	{
		if (!PairEP(pAd, pAd->BulkOutEpAddr[i]))
			DBGPRINT(RT_DEBUG_ERROR, ("Invalid bulk out ep(%x)\n", pAd->BulkOutEpAddr[i]));
	}

	for (i = 0; i < 2; i++)
	{
		if (!PairEP(pAd, pAd->BulkInEpAddr[i]))
			DBGPRINT(RT_DEBUG_ERROR, ("Invalid bulk in ep(%x)\n", pAd->BulkInEpAddr[i]));
	}

	return 0;
}


bool RtmpRaDevCtrlExit(IN struct rtmp_adapter *pAd)
{
	INT index;

#ifdef RLT_MAC
	if ((IS_MT76x0(pAd) || IS_MT76x2(pAd))&& (pAd->WlanFunCtrl.field.WLAN_EN == 1))
	{
		mt7612u_chip_onoff(pAd, false, false);
	}
#endif /* RLT_MAC */


	if (pAd->UsbVendorReqBuf)
		kfree(pAd->UsbVendorReqBuf);

	/*
		Free ProbeRespIE Table
	*/
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++)
	{
		if (pAd->ProbeRespIE[index].pIe)
			kfree(pAd->ProbeRespIE[index].pIe);
	}

	RTMPFreeTxRxRingMemory(pAd);

	RTMPFreeAdapter(pAd);

	return true;
}


#ifdef CONFIG_AP_SUPPORT
VOID RTMP_11N_D3_TimerInit(struct rtmp_adapter *pAd)
{
	RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, false);
}
#endif /* CONFIG_AP_SUPPORT */
