/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"


#ifdef CONFIG_STA_SUPPORT
VOID AsicUpdateAutoFallBackTable(
	IN	struct rtmp_adapter *pAd,
	IN	u8 *		pRateTable)
{
	UCHAR					i;
	HT_FBK_CFG0_STRUC		HtCfg0;
	HT_FBK_CFG1_STRUC		HtCfg1;
	LG_FBK_CFG0_STRUC		LgCfg0;
	LG_FBK_CFG1_STRUC		LgCfg1;
	RTMP_RA_LEGACY_TB *pCurrTxRate, *pNextTxRate;


	/* set to initial value*/
	HtCfg0.word = 0x65432100;
	HtCfg1.word = 0xedcba980;
	LgCfg0.word = 0xedcba988;
	LgCfg1.word = 0x00002100;


	if (IS_MT76x2(pAd))
		LgCfg1.word = 0x87872100;

#ifdef NEW_RATE_ADAPT_SUPPORT
	/* Use standard fallback if using new rate table */
	if (ADAPT_RATE_TABLE(pRateTable))
		goto skipUpdate;
#endif /* NEW_RATE_ADAPT_SUPPORT */

		pNextTxRate = (RTMP_RA_LEGACY_TB *)pRateTable+1;

	for (i = 1; i < *((u8 *) pRateTable); i++)
	{
			pCurrTxRate = (RTMP_RA_LEGACY_TB *)pRateTable+1+i;

		switch (pCurrTxRate->Mode)
		{
			case 0:		/* CCK */
				break;
			case 1:		/* OFDM */
				{
					switch(pCurrTxRate->CurrMCS)
					{
						case 0:
							LgCfg0.field.OFDMMCS0FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 1:
							LgCfg0.field.OFDMMCS1FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 2:
							LgCfg0.field.OFDMMCS2FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 3:
							LgCfg0.field.OFDMMCS3FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 4:
							LgCfg0.field.OFDMMCS4FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 5:
							LgCfg0.field.OFDMMCS5FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 6:
							LgCfg0.field.OFDMMCS6FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 7:
							LgCfg0.field.OFDMMCS7FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
					}
				}
				break;
			case 2:		/* HT-MIX */
			case 3:		/* HT-GF */
				{
					if ((pNextTxRate->Mode >= MODE_HTMIX) && (pCurrTxRate->CurrMCS != pNextTxRate->CurrMCS))
					{
						if (pCurrTxRate->CurrMCS <= 15)
						{
							switch(pCurrTxRate->CurrMCS)
							{
								case 0:
									HtCfg0.field.HTMCS0FBK = pNextTxRate->CurrMCS;
									break;
								case 1:
									HtCfg0.field.HTMCS1FBK = pNextTxRate->CurrMCS;
									break;
								case 2:
									HtCfg0.field.HTMCS2FBK = pNextTxRate->CurrMCS;
									break;
								case 3:
									HtCfg0.field.HTMCS3FBK = pNextTxRate->CurrMCS;
									break;
								case 4:
									HtCfg0.field.HTMCS4FBK = pNextTxRate->CurrMCS;
									break;
								case 5:
									HtCfg0.field.HTMCS5FBK = pNextTxRate->CurrMCS;
									break;
								case 6:
									HtCfg0.field.HTMCS6FBK = pNextTxRate->CurrMCS;
									break;
								case 7:
									HtCfg0.field.HTMCS7FBK = pNextTxRate->CurrMCS;
									break;
								case 8:
									HtCfg1.field.HTMCS8FBK = 0;//pNextTxRate->CurrMCS;
									break;
								case 9:
									HtCfg1.field.HTMCS9FBK = pNextTxRate->CurrMCS;
									break;
								case 10:
									HtCfg1.field.HTMCS10FBK = pNextTxRate->CurrMCS;
									break;
								case 11:
									HtCfg1.field.HTMCS11FBK = pNextTxRate->CurrMCS;
									break;
								case 12:
									HtCfg1.field.HTMCS12FBK = pNextTxRate->CurrMCS;
									break;
								case 13:
									HtCfg1.field.HTMCS13FBK = pNextTxRate->CurrMCS;
									break;
								case 14:
									HtCfg1.field.HTMCS14FBK = pNextTxRate->CurrMCS;
									break;
								case 15:
									HtCfg1.field.HTMCS15FBK = pNextTxRate->CurrMCS;
									break;
							}
						}
						else
							DBGPRINT(RT_DEBUG_ERROR, ("AsicUpdateAutoFallBackTable: not support CurrMCS=%d\n", pCurrTxRate->CurrMCS));
					}
				}
				break;
		}

		pNextTxRate = pCurrTxRate;
	}


#ifdef NEW_RATE_ADAPT_SUPPORT
skipUpdate:
#endif /* NEW_RATE_ADAPT_SUPPORT */

	mt7612u_write32(pAd, HT_FBK_CFG0, HtCfg0.word);
	mt7612u_write32(pAd, HT_FBK_CFG1, HtCfg1.word);
	mt7612u_write32(pAd, LG_FBK_CFG0, LgCfg0.word);
	mt7612u_write32(pAd, LG_FBK_CFG1, LgCfg1.word);

}
#endif /* CONFIG_STA_SUPPORT */


INT AsicSetAutoFallBack(struct rtmp_adapter *pAd, bool enable)
{
	TX_RTY_CFG_STRUC tx_rty_cfg = {.word = 0};

	tx_rty_cfg.word = mt7612u_read32(pAd, TX_RTY_CFG);
	tx_rty_cfg.field.TxautoFBEnable = ((enable == true) ? 1 : 0);
	mt7612u_write32(pAd, TX_RTY_CFG, tx_rty_cfg.word);

	return true;
}


INT AsicAutoFallbackInit(struct rtmp_adapter *pAd)
{
#ifdef RANGE_EXTEND
	mt7612u_write32(pAd, HT_FBK_CFG1, 0xedcba980);
#endif // RANGE_EXTEND //

	return true;
}


/*
	========================================================================

	Routine Description:
		Set MAC register value according operation mode.
		OperationMode AND bNonGFExist are for MM and GF Proteciton.
		If MM or GF mask is not set, those passing argument doesn't not take effect.

		Operation mode meaning:
		= 0 : Pure HT, no preotection.
		= 0x01; there may be non-HT devices in both the control and extension channel, protection is optional in BSS.
		= 0x10: No Transmission in 40M is protected.
		= 0x11: Transmission in both 40M and 20M shall be protected
		if (bNonGFExist)
			we should choose not to use GF. But still set correct ASIC registers.
	========================================================================
*/
typedef enum _PROT_REG_IDX_{
	REG_IDX_CCK = 0,	/* 0x1364 */
	REG_IDX_OFDM = 1,	/* 0x1368 */
	REG_IDX_MM20 = 2,  /* 0x136c */
	REG_IDX_MM40 = 3, /* 0x1370 */
	REG_IDX_GF20 = 4, /* 0x1374 */
	REG_IDX_GF40 = 5, /* 0x1378 */
}PROT_REG_IDX;


VOID AsicUpdateProtect(
	IN struct rtmp_adapter *pAd,
	IN USHORT OperationMode,
	IN UCHAR SetMask,
	IN bool bDisableBGProtect,
	IN bool bNonGFExist)
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	uint32_t Protect[6], PhyMode = 0x4000;
	USHORT offset;
	UCHAR i;
	uint32_t MacReg = 0;
	PROT_CFG_STRUC vht_port_cfg = {.word = 0};
	uint16_t protect_rate = 0;

	if (!(pAd->CommonCfg.bHTProtect) && (OperationMode != 8))
		return;

	if (pAd->BATable.numDoneOriginator)
	{
		/* enable the RTS/CTS to avoid channel collision*/
		SetMask |= ALLN_SETPROTECT;
		OperationMode = 8;
	}

	/* Config ASIC RTS threshold register*/
	MacReg = mt7612u_read32(pAd, TX_RTS_CFG);
	MacReg &= 0xFF0000FF;
	/* If the user want disable RtsThreshold and enbale Amsdu/Ralink-Aggregation, set the RtsThreshold as 4096*/
        if ((
			(pAd->CommonCfg.BACapability.field.AmsduEnable) ||
			(pAd->CommonCfg.bAggregationCapable == true))
            && pAd->CommonCfg.RtsThreshold == MAX_RTS_THRESHOLD)
        {
			MacReg |= (0x1000 << 8);
        }
        else
        {
			MacReg |= (pAd->CommonCfg.RtsThreshold << 8);
        }

	mt7612u_write32(pAd, TX_RTS_CFG, MacReg);

	/* Initial common protection settings*/
	RTMPZeroMemory(Protect, sizeof(Protect));
	ProtCfg4.word = 0;
	ProtCfg.word = 0;
	ProtCfg.field.TxopAllowGF40 = 1;
	ProtCfg.field.TxopAllowGF20 = 1;
	ProtCfg.field.TxopAllowMM40 = 1;
	ProtCfg.field.TxopAllowMM20 = 1;
	ProtCfg.field.TxopAllowOfdm = 1;
	ProtCfg.field.TxopAllowCck = 1;
	ProtCfg.field.RTSThEn = 1;
	ProtCfg.field.ProtectNav = ASIC_SHORTNAV;

	// TODO: shiang, is that a correct way to set 0x2000 here??
	if (IS_RT65XX(pAd))
		PhyMode = 0x2000; /* Bit 15:13, 0:Legacy CCK, 1: Legacy OFDM, 2: HT mix mode, 3: HT green field, 4: VHT mode, 5-7: Reserved */

	/* update PHY mode and rate*/
	if (pAd->OpMode == OPMODE_AP)
	{
		/* update PHY mode and rate*/
		if (pAd->CommonCfg.Channel > 14)
			ProtCfg.field.ProtectRate = PhyMode;
		ProtCfg.field.ProtectRate |= pAd->CommonCfg.RtsRate;
	}
	else if (pAd->OpMode == OPMODE_STA)
	{
		// Decide Protect Rate for Legacy packet
		if (pAd->CommonCfg.Channel > 14)
		{
			ProtCfg.field.ProtectRate = PhyMode; // OFDM 6Mbps
		}
		else
		{
			ProtCfg.field.ProtectRate = 0x0000; // CCK 1Mbps
			if (pAd->CommonCfg.MinTxRate > RATE_11)
				ProtCfg.field.ProtectRate |= PhyMode; // OFDM 6Mbps
		}
	}

	if (IS_RT65XX(pAd))
		protect_rate = ProtCfg.field.ProtectRate;


	/* Handle legacy(B/G) protection*/
	if (bDisableBGProtect)
	{
		/*ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;*/
		ProtCfg.field.ProtectCtrl = 0;
		Protect[REG_IDX_CCK] = ProtCfg.word;
		Protect[REG_IDX_OFDM] = ProtCfg.word;
		pAd->FlgCtsEnabled = 0; /* CTS-self is not used */
	}
	else
	{
		if (pAd->CommonCfg.Channel <= 14) {
			/*ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;*/
			ProtCfg.field.ProtectCtrl = 0;			/* CCK do not need to be protected*/
			Protect[REG_IDX_CCK] = ProtCfg.word;
			ProtCfg.field.ProtectCtrl = ASIC_CTS;	/* OFDM needs using CCK to protect*/
			Protect[REG_IDX_OFDM] = ProtCfg.word;
			pAd->FlgCtsEnabled = 1; /* CTS-self is used */
		} else {
			ProtCfg.field.ProtectCtrl = 0;
			Protect[REG_IDX_CCK] = ProtCfg.word;
			Protect[REG_IDX_OFDM] = ProtCfg.word;
			pAd->FlgCtsEnabled = 0; /* CTS-self is not used */
		}
	}

	/* Decide HT frame protection.*/
	if ((SetMask & ALLN_SETPROTECT) != 0)
	{
		switch(OperationMode)
		{
			case 0x0:
				/* NO PROTECT */
				/* 1.All STAs in the BSS are 20/40 MHz HT*/
				/* 2. in ai 20/40MHz BSS*/
				/* 3. all STAs are 20MHz in a 20MHz BSS*/
				/* Pure HT. no protection.*/

				/* MM20_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 010111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
				/* 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)*/
				Protect[2] = 0x01744004;

				/* MM40_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 111111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None) */
				/* 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)*/
				Protect[3] = 0x03f44084;

				/* CF20_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 010111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
				/* 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)*/
				Protect[4] = 0x01744004;

				/* CF40_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 111111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
				/* 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)*/
				Protect[5] = 0x03f44084;

				if (bNonGFExist)
				{
					/* PROT_NAV(19:18)  -- 01 (Short NAV protectiion)*/
					/* PROT_CTRL(17:16) -- 01 (RTS/CTS)*/
					Protect[REG_IDX_GF20] = 0x01754004;
					Protect[REG_IDX_GF40] = 0x03f54084;
				}
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = false;

				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG6);
					vht_port_cfg.field.ProtectCtrl = 0;
					mt7612u_write32(pAd, TX_PROT_CFG6, vht_port_cfg.word);

					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG7);
					vht_port_cfg.field.ProtectCtrl = 0;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					mt7612u_write32(pAd, TX_PROT_CFG7, vht_port_cfg.word);

					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG8);
					vht_port_cfg.field.ProtectCtrl = 0;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					mt7612u_write32(pAd, TX_PROT_CFG8, vht_port_cfg.word);
				}
				break;

 			case 1:
				/* This is "HT non-member protection mode."*/
				/* If there may be non-HT STAs my BSS*/
				ProtCfg.word = 0x01744004;	/* PROT_CTRL(17:16) : 0 (None)*/
				ProtCfg4.word = 0x03f44084; /* duplicaet legacy 24M. BW set 1.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083; */
				}
				/*Assign Protection method for 20&40 MHz packets*/
				ProtCfg.field.ProtectCtrl = ASIC_RTS;
				ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[REG_IDX_MM20] = ProtCfg.word;
				Protect[REG_IDX_MM40] = ProtCfg4.word;
				Protect[REG_IDX_GF20] = ProtCfg.word;
				Protect[REG_IDX_GF40] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = true;

				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					// Temporary tuen on RTS in VHT, MAC: TX_PROT_CFG6, TX_PROT_CFG7, TX_PROT_CFG8
					PROT_CFG_STRUC vht_port_cfg;

					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG6);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					mt7612u_write32(pAd, TX_PROT_CFG6, vht_port_cfg.word);

					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG7);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					mt7612u_write32(pAd, TX_PROT_CFG7, vht_port_cfg.word);

					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG8);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					mt7612u_write32(pAd, TX_PROT_CFG8, vht_port_cfg.word);
				}

				break;

			case 2:
				/* If only HT STAs are in BSS. at least one is 20MHz. Only protect 40MHz packets*/
				ProtCfg.word = 0x01744004;  /* PROT_CTRL(17:16) : 0 (None)*/
				ProtCfg4.word = 0x03f44084; /* duplicaet legacy 24M. BW set 1.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083; */
				}
				/*Assign Protection method for 40MHz packets*/
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[REG_IDX_MM20] = ProtCfg.word;
				Protect[REG_IDX_MM40] = ProtCfg4.word;
				if (bNonGFExist)
				{
					ProtCfg.field.ProtectCtrl = ASIC_RTS;
					ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				}
				Protect[REG_IDX_GF20] = ProtCfg.word;
				Protect[REG_IDX_GF40] = ProtCfg4.word;

				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = false;

				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG6);
					vht_port_cfg.field.ProtectCtrl = 0;
					mt7612u_write32(pAd, TX_PROT_CFG6, vht_port_cfg.word);

					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG7);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					mt7612u_write32(pAd, TX_PROT_CFG7, vht_port_cfg.word);

					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG8);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					mt7612u_write32(pAd, TX_PROT_CFG8, vht_port_cfg.word);
				}
				break;

			case 3:
				/* HT mixed mode. PROTECT ALL!*/
				/* Assign Rate*/
				ProtCfg.word = 0x01744004;	/*duplicaet legacy 24M. BW set 1.*/
				ProtCfg4.word = 0x03f44084;
				/* both 20MHz and 40MHz are protected. Whether use RTS or CTS-to-self depends on the*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083*/
				}
				/*Assign Protection method for 20&40 MHz packets*/
				ProtCfg.field.ProtectCtrl = ASIC_RTS;
				ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[REG_IDX_MM20] = ProtCfg.word;
				Protect[REG_IDX_MM40] = ProtCfg4.word;
				Protect[REG_IDX_GF20] = ProtCfg.word;
				Protect[REG_IDX_GF40] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = true;

				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					// Temporary turn on RTS in VHT, MAC: TX_PROT_CFG6, TX_PROT_CFG7, TX_PROT_CFG8
					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG6);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					mt7612u_write32(pAd, TX_PROT_CFG6, vht_port_cfg.word);

					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG7);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					mt7612u_write32(pAd, TX_PROT_CFG7, vht_port_cfg.word);

					vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG8);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					mt7612u_write32(pAd, TX_PROT_CFG8, vht_port_cfg.word);
				}
				break;

			case 8:
				/* Special on for Atheros problem n chip.*/
				ProtCfg.word = 0x01754004;	/*duplicaet legacy 24M. BW set 1.*/
				ProtCfg4.word = 0x03f54084;
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01750003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f50003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083*/
				}


                               if (IS_RT65XX(pAd))
                               {
                                       // Temporary tuen on RTS in VHT, MAC: TX_PROT_CFG6, TX_PROT_CFG7, TX_PROT_CFG8
                                       PROT_CFG_STRUC vht_port_cfg;

                                       vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG6);
                                       vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
                                       vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
                                       vht_port_cfg.field.ProtectRate = protect_rate;
                                       mt7612u_write32(pAd, TX_PROT_CFG6, vht_port_cfg.word);

                                       vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG7);
                                       vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
                                       vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
                                       vht_port_cfg.field.ProtectRate = protect_rate;
                                       mt7612u_write32(pAd, TX_PROT_CFG7, vht_port_cfg.word);

                                       vht_port_cfg.word = mt7612u_read32(pAd, TX_PROT_CFG8);
                                       vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
                                       vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
                                       vht_port_cfg.field.ProtectRate = protect_rate;
                                       mt7612u_write32(pAd, TX_PROT_CFG8, vht_port_cfg.word);
                               }

				Protect[REG_IDX_MM20] = ProtCfg.word; 	/*0x01754004;*/
				Protect[REG_IDX_MM40] = ProtCfg4.word; /*0x03f54084;*/
				Protect[REG_IDX_GF20] = ProtCfg.word; 	/*0x01754004;*/
				Protect[REG_IDX_GF40] = ProtCfg4.word; /*0x03f54084;*/
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = true;
				break;
		}
	}

	offset = CCK_PROT_CFG;
	for (i = 0;i < 6;i++)
	{
		if ((SetMask & (1<< i)))
		{
			if (IS_RT65XX(pAd)) {
				if ((Protect[i] & 0x4000) == 0x4000)
					Protect[i] = ((Protect[i] & (~0x4000)) | 0x2000);
			}
		}
		mt7612u_write32(pAd, offset + i*4, Protect[i]);
	}

	if (IS_RT65XX(pAd))
	{
		uint32_t cfg_reg;
		for (cfg_reg = TX_PROT_CFG6; cfg_reg <= TX_PROT_CFG8; cfg_reg += 4)
		{
			MacReg = mt7612u_read32(pAd, cfg_reg);
			MacReg &= (~0x18000000);
			if (pAd->CommonCfg.vht_bw_signal)
			{
				if (pAd->CommonCfg.vht_bw_signal == BW_SIGNALING_STATIC) /* static */
					MacReg |= 0x08000000;
				else if (pAd->CommonCfg.vht_bw_signal == BW_SIGNALING_DYNAMIC)/* dynamic */
					MacReg |= 0x18000000;
			}
			mt7612u_write32(pAd, cfg_reg, MacReg);
		}
	}
}


VOID AsicBBPAdjust(struct rtmp_adapter *pAd)
{
	// TODO: shiang-6590, now this function only used for AP mode, why we need this differentation?
	if (pAd->chipOps.ChipBBPAdjust != NULL)
		pAd->chipOps.ChipBBPAdjust(pAd);
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSwitchChannel(struct rtmp_adapter *pAd, UCHAR Channel, bool bScan)
{
	UCHAR bw;
	uint32_t value32;
#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		return;

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	/* clear all statistics count for QBSS Load */
	QBSS_LoadStatusClear(pAd);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if (pAd->chipOps.ChipSwitchChannel)
		pAd->chipOps.ChipSwitchChannel(pAd, Channel, bScan);
	else
		DBGPRINT(RT_DEBUG_ERROR, ("For this chip, no specified channel switch function!\n"));

	/* R66 should be set according to Channel and use 20MHz when scanning*/
	if (bScan)
		bw = BW_20;
	else {
		bw = pAd->CommonCfg.BBPCurrentBW;
	}

	rtmp_asic_set_bf(pAd); // FW will initialize TxBf HW status. Re-calling this AP could recover previous status

	if (IS_MT76x2(pAd))
	{
		// Disable BF HW to apply profile to packets when nSS == 2.
		// Maybe it can be initialized at chip init but removing the same CR initialization from FW will be better
		value32 = mt7612u_read32(pAd, TXO_R4);
		value32 |= 0x2000000;
		mt7612u_write32(pAd, TXO_R4, value32);

		// Enable SIG-B CRC check
		value32 = mt7612u_read32(pAd, RXO_R13);
		value32 |= 0x100;
		mt7612u_write32(pAd, RXO_R13, value32);
	}
}


/*
	==========================================================================
	Description:
		This function is required for 2421 only, and should not be used during
		site survey. It's only required after NIC decided to stay at a channel
		for a longer period.
		When this function is called, it's always after AsicSwitchChannel().

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicLockChannel(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Channel)
{
}

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */



#ifdef CONFIG_STA_SUPPORT
/*
	==========================================================================
	Description:
		put PHY to sleep here, and set next wakeup timer. PHY doesn't not wakeup
		automatically. Instead, MCU will issue a TwakeUpInterrupt to host after
		the wakeup timer timeout. Driver has to issue a separate command to wake
		PHY up.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSleepThenAutoWakeup(
	IN struct rtmp_adapter *pAd,
	IN USHORT TbttNumToNextWakeUp)
{
	RTMP_STA_SLEEP_THEN_AUTO_WAKEUP(pAd, TbttNumToNextWakeUp);
}

/*
	==========================================================================
	Description:
		AsicForceWakeup() is used whenever manual wakeup is required
		AsicForceSleep() should only be used when not in INFRA BSS. When
		in INFRA BSS, we should use AsicSleepThenAutoWakeup() instead.
	==========================================================================
 */
VOID AsicForceSleep(
	IN struct rtmp_adapter *pAd)
{

}

/*
	==========================================================================
	Description:
		AsicForceWakeup() is used whenever Twakeup timer (set via AsicSleepThenAutoWakeup)
		expired.

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	==========================================================================
 */
VOID AsicForceWakeup(
	IN struct rtmp_adapter *pAd,
	IN bool    bFromTx)
{
    DBGPRINT(RT_DEBUG_INFO, ("--> AsicForceWakeup \n"));
    RTMP_STA_FORCE_WAKEUP(pAd, bFromTx);
}
#endif /* CONFIG_STA_SUPPORT */


/*
	==========================================================================
	Description:
		Set My BSSID

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
 /* CFG_TODO */
VOID AsicSetBssid(struct rtmp_adapter *pAd, UCHAR *pBssid)
{
	uint32_t Addr4;

	DBGPRINT(RT_DEBUG_TRACE, ("===> AsicSetBssid %x:%x:%x:%x:%x:%x\n",
				PRINT_MAC(pBssid)));

	Addr4 = (uint32_t)(pBssid[0]) |
			(uint32_t)(pBssid[1] << 8)  |
			(uint32_t)(pBssid[2] << 16) |
			(uint32_t)(pBssid[3] << 24);
	mt7612u_write32(pAd, MAC_BSSID_DW0, Addr4);


	Addr4 = 0;
	/* always one BSSID in STA mode*/
	Addr4 = (uint32_t)(pBssid[4]) | (ULONG)(pBssid[5] << 8);


	mt7612u_write32(pAd, MAC_BSSID_DW1, Addr4);


}


INT AsicSetDevMac(struct rtmp_adapter *pAd, UCHAR *addr)
{
	MAC_DW0_STRUC csr2;
	MAC_DW1_STRUC csr3;

	csr2.field.Byte0 = addr[0];
	csr2.field.Byte1 = addr[1];
	csr2.field.Byte2 = addr[2];
	csr2.field.Byte3 = addr[3];
	mt7612u_write32(pAd, MAC_ADDR_DW0, csr2.word);

	csr3.word = 0;
	csr3.field.Byte4 = addr[4];
	{
		csr3.field.Byte5 = addr[5];
		csr3.field.U2MeMask = 0xff;
	}
	mt7612u_write32(pAd, MAC_ADDR_DW1, csr3.word);

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("SetDevMAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
			PRINT_MAC(addr)));


	return true;
}

#ifdef CONFIG_AP_SUPPORT
VOID AsicSetMbssMode(struct rtmp_adapter *pAd, UCHAR NumOfBcns)
{
	UCHAR NumOfMacs;
	uint32_t regValue;

	regValue = mt7612u_read32(pAd, MAC_BSSID_DW1);
	regValue &= 0x0000FFFF;

	/*
		Note:
			1.The MAC address of Mesh and AP-Client link are different from Main BSSID.
			2.If the Mesh link is included, its MAC address shall follow the last MBSSID's MAC by increasing 1.
			3.If the AP-Client link is included, its MAC address shall follow the Mesh interface MAC by increasing 1.
	*/
	NumOfMacs = pAd->ApCfg.BssidNum + MAX_MESH_NUM + MAX_APCLI_NUM;


	/* set Multiple BSSID mode */
	if (NumOfMacs <= 1)
	{
		pAd->ApCfg.MacMask = ~(1-1);
		/*regValue |= 0x0; */
	}
	else if (NumOfMacs <= 2)
	{
		if ((pAd->CurrentAddress[5] % 2 != 0)
		)
			DBGPRINT(RT_DEBUG_ERROR, ("The 2-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 2\n"));

		regValue |= (1<<16);
		pAd->ApCfg.MacMask = ~(2-1);
	}
	else if (NumOfMacs <= 4)
	{
		if (pAd->CurrentAddress[5] % 4 != 0)
			DBGPRINT(RT_DEBUG_ERROR, ("The 4-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 4\n"));

		regValue |= (2<<16);
		pAd->ApCfg.MacMask = ~(4-1);
	}
	else if (NumOfMacs <= 8)
	{
		if (pAd->CurrentAddress[5] % 8 != 0)
			DBGPRINT(RT_DEBUG_ERROR, ("The 8-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 8\n"));

		regValue |= (3<<16);
		pAd->ApCfg.MacMask = ~(8-1);
	}
	else if (NumOfMacs <= 16)
	{
		/* Set MULTI_BSSID_MODE_BIT4 in MAC register 0x1014 */
		regValue |= (1<<22);
		pAd->ApCfg.MacMask = ~(16-1);
	}

	/* set Multiple BSSID Beacon number */
	if (NumOfBcns > 1)
	{
		if (NumOfBcns > 8)
			regValue |= (((NumOfBcns - 1) >> 3) << 23);
		regValue |= (((NumOfBcns - 1) & 0x7)  << 18);
	}

	/* 	set as 0/1 bit-21 of MAC_BSSID_DW1(offset: 0x1014)
		to disable/enable the new MAC address assignment.  */
	if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
	{
		regValue |= (1 << 21);
#ifdef ENHANCE_NEW_MBSSID_MODE
		if (pAd->chipCap.MBSSIDMode == MBSSID_MODE2)
			regValue |=  (1 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE3)
			regValue |=  (2 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE4)
			regValue |=  (3 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE5)
			regValue |=  (4 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE6)
			regValue |=  (5 << 24);
#endif /* ENHANCE_NEW_MBSSID_MODE */
	}

	mt7612u_write32(pAd, MAC_BSSID_DW1, regValue);

}
#endif /* CONFIG_AP_SUPPORT */


INT AsicSetRxFilter(struct rtmp_adapter *pAd)
{
	uint32_t rx_filter_flag;

	/* enable RX of MAC block*/
	if ((pAd->OpMode == OPMODE_AP)
	)
	{
		rx_filter_flag = APNORMAL;

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	}
#ifdef CONFIG_STA_SUPPORT
	else
	{
			rx_filter_flag = STANORMAL;     /* Staion not drop control frame will fail WiFi Certification.*/
	}
#endif /* CONFIG_STA_SUPPORT */
	mt7612u_write32(pAd, RX_FILTR_CFG, rx_filter_flag);

	return true;
}


INT AsicSetRDG(struct rtmp_adapter *pAd, bool bEnable)
{
	TX_LINK_CFG_STRUC TxLinkCfg;
	uint32_t Data = 0;

	TxLinkCfg.word = mt7612u_read32(pAd, TX_LINK_CFG);
	TxLinkCfg.field.TxRDGEn =  (bEnable ? 1 : 0);
	mt7612u_write32(pAd, TX_LINK_CFG, TxLinkCfg.word);

	Data = mt7612u_read32(pAd, EDCA_AC0_CFG);
	Data &= 0xFFFFFF00;
	if (bEnable) {
		Data |= 0x80;
	} else {
		/* For CWC test, change txop from 0x30 to 0x20 in TxBurst mode*/
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE)
			&& (pAd->CommonCfg.bEnableTxBurst == true)
			&& (pAd->MacTab.fAnyStationMIMOPSDynamic == false)
		)
			Data |= 0x20;
	}
	mt7612u_write32(pAd, EDCA_AC0_CFG, Data);


	if (bEnable)
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
	else
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);

	return true;
}


/*
    ========================================================================
    Routine Description:
        Set/reset MAC registers according to bPiggyBack parameter

    Arguments:
        pAd         - Adapter pointer
        bPiggyBack  - Enable / Disable Piggy-Back

    Return Value:
        None

    ========================================================================
*/
VOID RTMPSetPiggyBack(struct rtmp_adapter *pAd, bool bPiggyBack)
{
	TX_LINK_CFG_STRUC  TxLinkCfg;

	TxLinkCfg.word = mt7612u_read32(pAd, TX_LINK_CFG);

	TxLinkCfg.field.TxCFAckEn = bPiggyBack;
	mt7612u_write32(pAd, TX_LINK_CFG, TxLinkCfg.word);
}

VOID AsicCtrlBcnMask(struct rtmp_adapter *pAd, INT mask)
{
	BCN_BYPASS_MASK_STRUC bms;

	bms.word = mt7612u_read32(pAd, TX_BCN_BYPASS_MASK);
	bms.field.BeaconDropMask = mask;
	mt7612u_write32(pAd, TX_BCN_BYPASS_MASK, bms.word);
}

static INT AsicSetIntTimerEn(struct rtmp_adapter *pAd, bool enable, uint32_t type, uint32_t timeout)
{
	uint32_t val, mask;
	uint32_t time_mask = 0xffff;	/* ULLI : to fix warning */

	/* ULLI : only two valid types are used */

	if (type == INT_TIMER_EN_PRE_TBTT) {
		mask = 0x1;
		timeout = (timeout & 0xffff);
		time_mask = 0xffff;
	}
	else if (type == INT_TIMER_EN_GP_TIMER) {
		mask = 0x2;
		timeout = (timeout & 0xffff) << 16;
		time_mask = (0xffff << 16);
	}
	else
		mask = 0x3;

	val = mt7612u_read32(pAd, INT_TIMER_EN);
	if (enable == false)
		val &= (~mask);
	else
		val |= mask;
	mt7612u_write32(pAd, INT_TIMER_EN, val);

	if (enable) {
		val = mt7612u_read32(pAd, INT_TIMER_CFG);
		val = (val & (~time_mask)) | timeout;
		mt7612u_write32(pAd, INT_TIMER_CFG, val);
	}

	return true;
}


INT AsicSetPreTbtt(struct rtmp_adapter *pAd, bool enable)
{
	uint32_t timeout = 0;

	if (enable == true)
		timeout = 6 << 4; /* Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable. */

	return AsicSetIntTimerEn(pAd, enable, INT_TIMER_EN_PRE_TBTT, timeout);
}


INT AsicSetGPTimer(struct rtmp_adapter *pAd, bool enable, uint32_t timeout)
{
	return AsicSetIntTimerEn(pAd, enable, INT_TIMER_EN_GP_TIMER, timeout);
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicDisableSync(
	IN struct rtmp_adapter *pAd)
{
	BCN_TIME_CFG_STRUC csr;

	DBGPRINT(RT_DEBUG_TRACE, ("--->Disable TSF synchronization\n"));

	pAd->TbttTickCount = 0;
	csr.word = mt7612u_read32(pAd, BCN_TIME_CFG);
	csr.field.bBeaconGen = 0;
	csr.field.bTBTTEnable = 0;
	csr.field.TsfSyncMode = 0;
	csr.field.bTsfTicking = 0;
	mt7612u_write32(pAd, BCN_TIME_CFG, csr.word);

}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicEnableBssSync(
	IN struct rtmp_adapter *pAd)
{
	BCN_TIME_CFG_STRUC csr;

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableBssSync(INFRA mode)\n"));

	csr.word = mt7612u_read32(pAd, BCN_TIME_CFG);
/*	mt7612u_write32(pAd, BCN_TIME_CFG, 0x00000000);*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
		csr.field.bTsfTicking = 1;
		csr.field.TsfSyncMode = 3; /* sync TSF similar as in ADHOC mode?*/
		csr.field.bBeaconGen  = 1; /* AP should generate BEACON*/
		csr.field.bTBTTEnable = 1;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
		csr.field.bTsfTicking = 1;
		csr.field.TsfSyncMode = 1; /* sync TSF in INFRASTRUCTURE mode*/
		csr.field.bBeaconGen  = 0; /* do NOT generate BEACON*/
		csr.field.bTBTTEnable = 1;
	}
#endif /* CONFIG_STA_SUPPORT */
	mt7612u_write32(pAd, BCN_TIME_CFG, csr.word);
}

/*CFG_TODO*/
VOID AsicEnableApBssSync(
	IN struct rtmp_adapter *pAd)
{
	BCN_TIME_CFG_STRUC csr;

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableBssSync(INFRA mode)\n"));

	csr.word = mt7612u_read32(pAd, BCN_TIME_CFG);

	csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
	csr.field.bTsfTicking = 1;
	csr.field.TsfSyncMode = 3; /* sync TSF similar as in ADHOC mode?*/
	csr.field.bBeaconGen  = 1; /* AP should generate BEACON*/
	csr.field.bTBTTEnable = 1;

	mt7612u_write32(pAd, BCN_TIME_CFG, csr.word);
}


#ifdef CONFIG_STA_SUPPORT
/*
	==========================================================================
	Description:
	Note:
		BEACON frame in shared memory should be built ok before this routine
		can be called. Otherwise, a garbage frame maybe transmitted out every
		Beacon period.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicEnableIbssSync(struct rtmp_adapter *pAd)
{
	BCN_TIME_CFG_STRUC csr9;
	UCHAR *ptr;
	UINT i;
	ULONG beaconBaseLocation = 0;
	USHORT beaconLen = 0;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	beaconLen = pAd->BeaconTxWI.TXWI_N.MPDUtotalByteCnt;

#ifdef RT_BIG_ENDIAN
	{
		TXWI_STRUC localTxWI;

		memmove(&localTxWI, &pAd->BeaconTxWI, TXWISize);
		RTMPWIEndianChange(pAd, (u8 *)&localTxWI, TYPE_TXWI);

		beaconLen = localTxWI.TXWI_N.MPDUtotalByteCnt;

	}
#endif /* RT_BIG_ENDIAN */

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableIbssSync(ADHOC mode, beaconLen=%d)\n",
				beaconLen));

	csr9.word = mt7612u_read32(pAd, BCN_TIME_CFG);
	csr9.field.bBeaconGen = 0;
	csr9.field.bTBTTEnable = 0;
	csr9.field.bTsfTicking = 0;
	mt7612u_write32(pAd, BCN_TIME_CFG, csr9.word);
	beaconBaseLocation = HW_BEACON_BASE0(pAd);


#ifdef RTMP_MAC_USB
	/* move BEACON TXD and frame content to on-chip memory*/
	ptr = (u8 *)&pAd->BeaconTxWI;
	for (i = 0; i < TXWISize; i += 4) {
		u32 dword;

		dword =  *ptr +
			(*(ptr + 1) << 8);
			(*(ptr + 2) << 16);
			(*(ptr + 3) << 24);

		mt7612u_write32(pAd, HW_BEACON_BASE0(pAd) + i, dword);
		ptr += 4;
	}

	/* start right after the 16-byte TXWI field*/
	ptr = pAd->BeaconBuf;
	for (i = 0; i< beaconLen; i += 4) {
		u32 dword;

		dword =  *ptr +
			(*(ptr + 1) << 8);
			(*(ptr + 2) << 16);
			(*(ptr + 3) << 24);

		mt7612u_write32(pAd, HW_BEACON_BASE0(pAd) + TXWISize + i,
				    dword);
		ptr +=4;
	}
#endif /* RTMP_MAC_USB */

	/*
		For Wi-Fi faily generated beacons between participating stations.
		Set TBTT phase adaptive adjustment step to 8us (default 16us)
	*/
	/* don't change settings 2006-5- by Jerry*/
	/*mt7612u_write32(pAd, TBTT_SYNC_CFG, 0x00001010);*/

	/* start sending BEACON*/
	csr9.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
	csr9.field.bTsfTicking = 1;
	/*
		(STA ad-hoc mode) Upon the reception of BEACON frame from associated BSS,
		local TSF is updated with remote TSF only if the remote TSF is greater than local TSF
	*/
	csr9.field.TsfSyncMode = 2; /* sync TSF in IBSS mode*/
	csr9.field.bTBTTEnable = 1;
	csr9.field.bBeaconGen = 1;
	mt7612u_write32(pAd, BCN_TIME_CFG, csr9.word);
}
#endif /* CONFIG_STA_SUPPORT */


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetEdcaParm(struct rtmp_adapter *pAd, PEDCA_PARM pEdcaParm)
{
	EDCA_AC_CFG_STRUC Ac0Cfg, Ac1Cfg, Ac2Cfg, Ac3Cfg;
	AC_TXOP_CSR0_STRUC csr0;
	AC_TXOP_CSR1_STRUC csr1;
	AIFSN_CSR_STRUC AifsnCsr;
	CWMIN_CSR_STRUC CwminCsr;
	CWMAX_CSR_STRUC CwmaxCsr;
	int i;

	Ac0Cfg.word = 0;
	Ac1Cfg.word = 0;
	Ac2Cfg.word = 0;
	Ac3Cfg.word = 0;
	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == false))
	{
		DBGPRINT(RT_DEBUG_TRACE,("AsicSetEdcaParm\n"));
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		for (i=0; i < MAX_LEN_OF_MAC_TABLE; i++)
		{
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) || IS_ENTRY_APCLI(&pAd->MacTab.Content[i]))
				CLIENT_STATUS_CLEAR_FLAG(&pAd->MacTab.Content[i], fCLIENT_STATUS_WMM_CAPABLE);
		}

		/*========================================================*/
		/*      MAC Register has a copy .*/
		/*========================================================*/
		if( pAd->CommonCfg.bEnableTxBurst )
		{
			/* For CWC test, change txop from 0x30 to 0x20 in TxBurst mode*/
			Ac0Cfg.field.AcTxop = 0x20; /* Suggest by John for TxBurst in HT Mode*/
		}
		else
			Ac0Cfg.field.AcTxop = 0;	/* QID_AC_BE*/

		Ac0Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac0Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac0Cfg.field.Aifsn = 2;
		mt7612u_write32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);

		Ac1Cfg.field.AcTxop = 0;	/* QID_AC_BK*/
		Ac1Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac1Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac1Cfg.field.Aifsn = 2;
		mt7612u_write32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);

		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
		{
			Ac2Cfg.field.AcTxop = 192;	/* AC_VI: 192*32us ~= 6ms*/
			Ac3Cfg.field.AcTxop = 96;	/* AC_VO: 96*32us  ~= 3ms*/
		}
		else
		{
			Ac2Cfg.field.AcTxop = 96;	/* AC_VI: 96*32us ~= 3ms*/
			Ac3Cfg.field.AcTxop = 48;	/* AC_VO: 48*32us ~= 1.5ms*/
		}
		Ac2Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac2Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac2Cfg.field.Aifsn = 2;
		mt7612u_write32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		Ac3Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac3Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac3Cfg.field.Aifsn = 2;
		mt7612u_write32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);

		/*========================================================*/
		/*      DMA Register has a copy too.*/
		/*========================================================*/
		csr0.field.Ac0Txop = 0;		/* QID_AC_BE*/
		csr0.field.Ac1Txop = 0;		/* QID_AC_BK*/
		mt7612u_write32(pAd, WMM_TXOP0_CFG, csr0.word);
		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
		{
			csr1.field.Ac2Txop = 192;		/* AC_VI: 192*32us ~= 6ms*/
			csr1.field.Ac3Txop = 96;		/* AC_VO: 96*32us  ~= 3ms*/
		}
		else
		{
			csr1.field.Ac2Txop = 96;		/* AC_VI: 96*32us ~= 3ms*/
			csr1.field.Ac3Txop = 48;		/* AC_VO: 48*32us ~= 1.5ms*/
		}
		mt7612u_write32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = pAd->wmm_cw_min;
		CwminCsr.field.Cwmin1 = pAd->wmm_cw_min;
		CwminCsr.field.Cwmin2 = pAd->wmm_cw_min;
		CwminCsr.field.Cwmin3 = pAd->wmm_cw_min;
		mt7612u_write32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = pAd->wmm_cw_max;
		CwmaxCsr.field.Cwmax1 = pAd->wmm_cw_max;
		CwmaxCsr.field.Cwmax2 = pAd->wmm_cw_max;
		CwmaxCsr.field.Cwmax3 = pAd->wmm_cw_max;
		mt7612u_write32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		mt7612u_write32(pAd, WMM_AIFSN_CFG, 0x00002222);

		memset(&pAd->CommonCfg.APEdcaParm, 0, sizeof(EDCA_PARM));

	}
	else
	{
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		/*========================================================*/
		/*      MAC Register has a copy.*/
		/*========================================================*/

		/* Modify Cwmin/Cwmax/Txop on queue[QID_AC_VI], Recommend by Jerry 2005/07/27*/
		/* To degrade our VIDEO Queue's throughput for WiFi WMM S3T07 Issue.*/

		/*pEdcaParm->Txop[QID_AC_VI] = pEdcaParm->Txop[QID_AC_VI] * 7 / 10;  rt2860c need this		*/

		Ac0Cfg.field.AcTxop =  pEdcaParm->Txop[QID_AC_BE];
		Ac0Cfg.field.Cwmin= pEdcaParm->Cwmin[QID_AC_BE];
		Ac0Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_BE];
		Ac0Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_BE]; /*+1;*/

		Ac1Cfg.field.AcTxop =  pEdcaParm->Txop[QID_AC_BK];
		Ac1Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_BK]; /*+2; */
		Ac1Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_BK];
		Ac1Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_BK]; /*+1;*/


		Ac2Cfg.field.AcTxop = (pEdcaParm->Txop[QID_AC_VI] * 6) / 10;
		{
			Ac2Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VI];
			Ac2Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VI];
		}
		/*sync with window 20110524*/
		Ac2Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VI] + 1; /* 5.2.27 T6 Pass Tx VI+BE, but will impack 5.2.27/28 T7. Tx VI*/

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			/* Tuning for Wi-Fi WMM S06*/
			if (pAd->CommonCfg.bWiFiTest &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
				Ac2Cfg.field.Aifsn -= 1;

			/* Tuning for TGn Wi-Fi 5.2.32*/
			/* STA TestBed changes in this item: conexant legacy sta ==> broadcom 11n sta*/
			if (STA_TGN_WIFI_ON(pAd) &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				Ac0Cfg.field.Aifsn = 3;
				Ac2Cfg.field.AcTxop = 5;
			}

		}
#endif /* CONFIG_STA_SUPPORT */

		Ac3Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VO];
		Ac3Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VO];
		Ac3Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VO];
		Ac3Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VO];

		if (pAd->CommonCfg.bWiFiTest)
		{
			if (Ac3Cfg.field.AcTxop == 102)
			{
			Ac0Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BE] ? pEdcaParm->Txop[QID_AC_BE] : 10;
				Ac0Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BE]-1; /* AIFSN must >= 1 */
			Ac1Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BK];
				Ac1Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BK];
			Ac2Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VI];
			}
		}

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */


		mt7612u_write32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
		mt7612u_write32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);
		mt7612u_write32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		mt7612u_write32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);


		/*========================================================*/
		/*      DMA Register has a copy too.*/
		/*========================================================*/
		csr0.field.Ac0Txop = Ac0Cfg.field.AcTxop;
		csr0.field.Ac1Txop = Ac1Cfg.field.AcTxop;
		mt7612u_write32(pAd, WMM_TXOP0_CFG, csr0.word);

		csr1.field.Ac2Txop = Ac2Cfg.field.AcTxop;
		csr1.field.Ac3Txop = Ac3Cfg.field.AcTxop;
		mt7612u_write32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = pEdcaParm->Cwmin[QID_AC_BE];
		CwminCsr.field.Cwmin1 = pEdcaParm->Cwmin[QID_AC_BK];
		CwminCsr.field.Cwmin2 = pEdcaParm->Cwmin[QID_AC_VI];
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO];
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO] - 1; /*for TGn wifi test*/
#endif /* CONFIG_STA_SUPPORT */
		mt7612u_write32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = pEdcaParm->Cwmax[QID_AC_BE];
		CwmaxCsr.field.Cwmax1 = pEdcaParm->Cwmax[QID_AC_BK];
		CwmaxCsr.field.Cwmax2 = pEdcaParm->Cwmax[QID_AC_VI];
		CwmaxCsr.field.Cwmax3 = pEdcaParm->Cwmax[QID_AC_VO];
		mt7612u_write32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		AifsnCsr.word = 0;
		AifsnCsr.field.Aifsn0 = Ac0Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_BE];*/
		AifsnCsr.field.Aifsn1 = Ac1Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_BK];*/
#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_USB
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if(pAd->Antenna.field.TxPath == 1)
				AifsnCsr.field.Aifsn1 = Ac1Cfg.field.Aifsn + 2; 	/*5.2.27 T7 Pass*/
		}
#endif /* RTMP_MAC_USB */
#endif /* CONFIG_STA_SUPPORT */
		AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VI];*/

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			/* Tuning for Wi-Fi WMM S06*/
			if (pAd->CommonCfg.bWiFiTest &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
				AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn - 4;

			/* Tuning for TGn Wi-Fi 5.2.32*/
			/* STA TestBed changes in this item: connexant legacy sta ==> broadcom 11n sta*/
			if (STA_TGN_WIFI_ON(pAd) &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				AifsnCsr.field.Aifsn0 = 3;
				AifsnCsr.field.Aifsn2 = 7;
			}

			if (INFRA_ON(pAd))
				CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[BSSID_WCID], fCLIENT_STATUS_WMM_CAPABLE);
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VO]*/
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn - 1; /*pEdcaParm->Aifsn[QID_AC_VO]; for TGn wifi test*/

			/* TODO: Is this modification also suitable for RT3052/RT3050 ???*/
			if (0
			)
			{
				AifsnCsr.field.Aifsn2 = 0x2; /*pEdcaParm->Aifsn[QID_AC_VI]; for WiFi WMM S4-T04.*/
			}
		}
#endif /* CONFIG_STA_SUPPORT */
		mt7612u_write32(pAd, WMM_AIFSN_CFG, AifsnCsr.word);

		memmove(&pAd->CommonCfg.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
		if (!ADHOC_ON(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE,("EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM\n", pEdcaParm->EdcaUpdateCount));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_BE      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[0],
									 pEdcaParm->Cwmin[0],
									 pEdcaParm->Cwmax[0],
									 pEdcaParm->Txop[0]<<5,
									 pEdcaParm->bACM[0]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_BK      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[1],
									 pEdcaParm->Cwmin[1],
									 pEdcaParm->Cwmax[1],
									 pEdcaParm->Txop[1]<<5,
									 pEdcaParm->bACM[1]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_VI      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[2],
									 pEdcaParm->Cwmin[2],
									 pEdcaParm->Cwmax[2],
									 pEdcaParm->Txop[2]<<5,
									 pEdcaParm->bACM[2]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_VO      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[3],
									 pEdcaParm->Cwmin[3],
									 pEdcaParm->Cwmax[3],
									 pEdcaParm->Txop[3]<<5,
									 pEdcaParm->bACM[3]));
		}

	}

	pAd->CommonCfg.RestoreBurstMode = Ac0Cfg.word;
}



INT AsicSetRetryLimit(struct rtmp_adapter *pAd, uint32_t type, uint32_t limit)
{
	TX_RTY_CFG_STRUC tx_rty_cfg;

	tx_rty_cfg.word = mt7612u_read32(pAd, TX_RTY_CFG);
	if (type == TX_RTY_CFG_RTY_LIMIT_SHORT)
		tx_rty_cfg.field.ShortRtyLimit = limit;
	else if (type == TX_RTY_CFG_RTY_LIMIT_LONG)
		tx_rty_cfg.field.LongRtyLimit = limit;
	mt7612u_write32(pAd, TX_RTY_CFG, tx_rty_cfg.word);

	return true;
}


uint32_t AsicGetRetryLimit(struct rtmp_adapter *pAd, uint32_t type)
{
	TX_RTY_CFG_STRUC tx_rty_cfg = {.word = 0};

	tx_rty_cfg.word = mt7612u_read32(pAd, TX_RTY_CFG);
	if (type == TX_RTY_CFG_RTY_LIMIT_SHORT)
		return tx_rty_cfg.field.ShortRtyLimit;
	else if (type == TX_RTY_CFG_RTY_LIMIT_LONG)
		return tx_rty_cfg.field.LongRtyLimit;

	return 0;
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetSlotTime(
	IN struct rtmp_adapter *pAd,
	IN bool bUseShortSlotTime)
{
	ULONG	SlotTime;
	uint32_t RegValue = 0;

#ifdef CONFIG_STA_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
		bUseShortSlotTime = true;
#endif /* CONFIG_STA_SUPPORT */

	if (bUseShortSlotTime && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
		return;
	else if ((!bUseShortSlotTime) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED)))
		return;

	if (bUseShortSlotTime)
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
	else
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

	SlotTime = (bUseShortSlotTime)? 9 : 20;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* force using short SLOT time for FAE to demo performance when TxBurst is ON*/
		if (((pAd->StaActive.SupportedPhyInfo.bHtEnable == false) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)))
			|| ((pAd->StaActive.SupportedPhyInfo.bHtEnable == true) && (pAd->CommonCfg.BACapability.field.Policy == BA_NOTUSE))
			)
		{
			/* In this case, we will think it is doing Wi-Fi test*/
			/* And we will not set to short slot when bEnableTxBurst is true.*/
		}
		else if (pAd->CommonCfg.bEnableTxBurst)
		{
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 9;
		}
	}
#endif /* CONFIG_STA_SUPPORT */


	/* For some reasons, always set it to short slot time.*/
	/* ToDo: Should consider capability with 11B*/
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->StaCfg.BssType == BSS_ADHOC)
		{
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 20;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	RegValue = mt7612u_read32(pAd, BKOFF_SLOT_CFG);
	RegValue = RegValue & 0xFFFFFF00;

	RegValue |= SlotTime;

	mt7612u_write32(pAd, BKOFF_SLOT_CFG, RegValue);
}


#ifdef CONFIG_AP_SUPPORT
VOID RTMPGetTxTscFromAsic(struct rtmp_adapter *pAd, UCHAR apidx, UCHAR *pTxTsc)
{
	USHORT Wcid;
	USHORT offset;
	UCHAR IvEiv[8];
	INT i;

	/* Sanity check of apidx */
	if (apidx >= MAX_MBSSID_NUM(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RTMPGetTxTscFromAsic : invalid apidx(%d)\n", apidx));
		return;
	}

	/* Initial value */
	memset(IvEiv, 0, 8);
	memset(pTxTsc, 0, 6);

	/* Get apidx for this BSSID */
	GET_GroupKey_WCID(pAd, Wcid, apidx);

	/* When the group rekey action is triggered, a count-down(3 seconds) is started.
	   During the count-down, use the initial PN as TSC.
	   Otherwise, get the IVEIV from ASIC. */
	if (pAd->ApCfg.MBSSID[apidx].RekeyCountDown > 0)
	{
		/*
		In IEEE 802.11-2007 8.3.3.4.3 described :
		The PN shall be implemented as a 48-bit monotonically incrementing
		non-negative integer, initialized to 1 when the corresponding
		temporal key is initialized or refreshed. */
		IvEiv[0] = 1;
	}
	else
	{
		uint32_t temp1, temp2;
		uint32_t iveiv_tb_base = 0, iveiv_tb_size = 0;

		iveiv_tb_base = RLT_MAC_IVEIV_TABLE_BASE;
		iveiv_tb_size = RLT_HW_IVEIV_ENTRY_SIZE;

		/* Read IVEIV from Asic */
		offset = iveiv_tb_base + (Wcid * iveiv_tb_size);

		/* Use Read32 to avoid endian problem */
		temp1 = mt7612u_read32(pAd, offset);
		temp2 = mt7612u_read32(pAd, offset+4);
		for ( i=0; i<4; i++)
		{
			IvEiv[i] = (UCHAR)(temp1 >> (i*8));
			IvEiv[i+4] = (UCHAR)(temp2 >> (i*8));
		}
	}

	/* Record current TxTsc */
	if (pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus == Ndis802_11AESEnable)
	{	/* AES */
		*pTxTsc 	= IvEiv[0];
		*(pTxTsc+1) = IvEiv[1];
		*(pTxTsc+2) = IvEiv[4];
		*(pTxTsc+3) = IvEiv[5];
		*(pTxTsc+4) = IvEiv[6];
		*(pTxTsc+5) = IvEiv[7];
	}
	else
	{	/* TKIP */
		*pTxTsc 	= IvEiv[2];
		*(pTxTsc+1) = IvEiv[0];
		*(pTxTsc+2) = IvEiv[4];
		*(pTxTsc+3) = IvEiv[5];
		*(pTxTsc+4) = IvEiv[6];
		*(pTxTsc+5) = IvEiv[7];
	}
	DBGPRINT(RT_DEBUG_TRACE, ("RTMPGetTxTscFromAsic : WCID(%d) TxTsc 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x \n",
									Wcid, *pTxTsc, *(pTxTsc+1), *(pTxTsc+2), *(pTxTsc+3), *(pTxTsc+4), *(pTxTsc+5)));


}
#endif /* CONFIG_AP_SUPPORT */


/*
	========================================================================
	Description:
		Add Shared key information into ASIC.
		Update shared key, TxMic and RxMic to Asic Shared key table
		Update its cipherAlg to Asic Shared key Mode.

    Return:
	========================================================================
*/
VOID AsicAddSharedKeyEntry(
	IN struct rtmp_adapter *	pAd,
	IN UCHAR		 	BssIndex,
	IN UCHAR		 	KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
	ULONG offset; /*, csr0;*/
	SHAREDKEY_MODE_STRUC csr1;
	UCHAR org_bssindex;

	u8 *	pKey = pCipherKey->Key;
	u8 *	pTxMic = pCipherKey->TxMic;
	u8 *	pRxMic = pCipherKey->RxMic;
	UCHAR		CipherAlg = pCipherKey->CipherAlg;

	DBGPRINT(RT_DEBUG_TRACE, ("AsicAddSharedKeyEntry BssIndex=%d, KeyIdx=%d\n", BssIndex,KeyIdx));
/*============================================================================================*/

	DBGPRINT(RT_DEBUG_TRACE,("AsicAddSharedKeyEntry: %s key #%d\n", CipherName[CipherAlg], BssIndex*4 + KeyIdx));
	DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}
/*============================================================================================*/

	org_bssindex = BssIndex;
	if (BssIndex >= 8)
		BssIndex -= 8;

	{
		/* fill key material - key + TX MIC + RX MIC*/
		uint32_t share_key_base = 0, share_key_size = 0;
		if (org_bssindex >= 8)
			share_key_base = RLT_SHARED_KEY_TABLE_BASE_EXT;
		else
		share_key_base = RLT_SHARED_KEY_TABLE_BASE;
		share_key_size = RLT_HW_KEY_ENTRY_SIZE;

		offset = share_key_base + (4*BssIndex + KeyIdx)*share_key_size;

#ifdef RTMP_MAC_USB
		{
			RTUSBMultiWrite(pAd, offset, pKey, MAX_LEN_OF_SHARE_KEY);

			offset += MAX_LEN_OF_SHARE_KEY;
			if (pTxMic)
			{
				RTUSBMultiWrite(pAd, offset, pTxMic, 8);
			}

			offset += 8;
			if (pRxMic)
			{
				RTUSBMultiWrite(pAd, offset, pRxMic, 8);
			}
		}
#endif /* RTMP_MAC_USB */
	}

	{
		uint32_t share_key_mode_base = 0;

		if (org_bssindex >= 8)
			share_key_mode_base = RLT_SHARED_KEY_MODE_BASE_EXT;
		else
		share_key_mode_base= RLT_SHARED_KEY_MODE_BASE;

		/* Update cipher algorithm. WSTA always use BSS0*/
		csr1.word = mt7612u_read32(pAd, share_key_mode_base + 4 * (BssIndex/2));
		DBGPRINT(RT_DEBUG_TRACE,("Read: SHARED_KEY_MODE_BASE at this Bss[%d] KeyIdx[%d]= 0x%x \n", BssIndex,KeyIdx, csr1.word));
		if ((BssIndex%2) == 0)
		{
			if (KeyIdx == 0)
				csr1.field.Bss0Key0CipherAlg = CipherAlg;
			else if (KeyIdx == 1)
				csr1.field.Bss0Key1CipherAlg = CipherAlg;
			else if (KeyIdx == 2)
				csr1.field.Bss0Key2CipherAlg = CipherAlg;
			else
				csr1.field.Bss0Key3CipherAlg = CipherAlg;
		}
		else
		{
			if (KeyIdx == 0)
				csr1.field.Bss1Key0CipherAlg = CipherAlg;
			else if (KeyIdx == 1)
				csr1.field.Bss1Key1CipherAlg = CipherAlg;
			else if (KeyIdx == 2)
				csr1.field.Bss1Key2CipherAlg = CipherAlg;
			else
				csr1.field.Bss1Key3CipherAlg = CipherAlg;
		}
		DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
		mt7612u_write32(pAd, share_key_mode_base+ 4 * (BssIndex/2), csr1.word);
	}
}


/*	IRQL = DISPATCH_LEVEL*/
VOID AsicRemoveSharedKeyEntry(
	IN struct rtmp_adapter *pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
	/*ULONG SecCsr0;*/
	SHAREDKEY_MODE_STRUC csr1;

	DBGPRINT(RT_DEBUG_TRACE,("AsicRemoveSharedKeyEntry: #%d \n", BssIndex*4 + KeyIdx));

	{
		uint32_t share_key_mode_base = 0;

		share_key_mode_base= RLT_SHARED_KEY_MODE_BASE;

		csr1.word = mt7612u_read32(pAd, share_key_mode_base+4*(BssIndex/2));
		if ((BssIndex%2) == 0)
		{
			if (KeyIdx == 0)
				csr1.field.Bss0Key0CipherAlg = 0;
			else if (KeyIdx == 1)
				csr1.field.Bss0Key1CipherAlg = 0;
			else if (KeyIdx == 2)
				csr1.field.Bss0Key2CipherAlg = 0;
			else
				csr1.field.Bss0Key3CipherAlg = 0;
		}
		else
		{
			if (KeyIdx == 0)
				csr1.field.Bss1Key0CipherAlg = 0;
			else if (KeyIdx == 1)
				csr1.field.Bss1Key1CipherAlg = 0;
			else if (KeyIdx == 2)
				csr1.field.Bss1Key2CipherAlg = 0;
			else
				csr1.field.Bss1Key3CipherAlg = 0;
		}
		DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
		mt7612u_write32(pAd, share_key_mode_base+4*(BssIndex/2), csr1.word);
	}
	ASSERT(BssIndex < 4);
	ASSERT(KeyIdx < 4);

}


VOID AsicUpdateWCIDIVEIV(
	IN struct rtmp_adapter *pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV)
{
	ULONG	offset;
	uint32_t iveiv_tb_base = 0, iveiv_tb_size = 0;

	iveiv_tb_base = RLT_MAC_IVEIV_TABLE_BASE;
	iveiv_tb_size = RLT_HW_IVEIV_ENTRY_SIZE;

	offset = iveiv_tb_base + (WCID * iveiv_tb_size);

	mt7612u_write32(pAd, offset, uIV);
	mt7612u_write32(pAd, offset + 4, uEIV);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: wcid(%d) 0x%08lx, 0x%08lx \n",
									__FUNCTION__, WCID, uIV, uEIV));
}


VOID AsicUpdateRxWCIDTable(
	IN struct rtmp_adapter *pAd,
	IN USHORT WCID,
	IN u8 *pAddr)
{
	ULONG offset;
	ULONG Addr;

	offset = MAC_WCID_BASE + (WCID * HW_WCID_ENTRY_SIZE);
	Addr = pAddr[0] + (pAddr[1] << 8) +(pAddr[2] << 16) +(pAddr[3] << 24);
	mt7612u_write32(pAd, offset, Addr);
	Addr = pAddr[4] + (pAddr[5] << 8);
	mt7612u_write32(pAd, offset + 4, Addr);
}


/*
	========================================================================
	Description:
		Add Client security information into ASIC WCID table and IVEIV table.
    Return:

    Note :
		The key table selection rule :
    	1.	Wds-links and Mesh-links always use Pair-wise key table.
		2. 	When the CipherAlg is TKIP, AES, SMS4 or the dynamic WEP is enabled,
			it needs to set key into Pair-wise Key Table.
		3.	The pair-wise key security mode is set NONE, it means as no security.
		4.	In STA Adhoc mode, it always use shared key table.
		5.	Otherwise, use shared key table

	========================================================================
*/
VOID AsicUpdateWcidAttributeEntry(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8			Wcid,
	IN	UINT8			KeyTabFlag)
{
	WCID_ATTRIBUTE_STRUC WCIDAttri;
	USHORT offset;
	uint32_t wcid_attr_base = 0, wcid_attr_size = 0;

	wcid_attr_base = RLT_MAC_WCID_ATTRIBUTE_BASE;
	wcid_attr_size = RLT_HW_WCID_ATTRI_SIZE;

	/* Initialize the content of WCID Attribue  */
	WCIDAttri.word = 0;

	/* The limitation of HW WCID table */
	if (Wcid > 254)
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s:Invalid wcid(%d)\n", __FUNCTION__, Wcid));
		return;
	}

	/* Update the pairwise key security mode.
	   Use bit10 and bit3~1 to indicate the pairwise cipher mode */
	WCIDAttri.field.PairKeyModeExt = ((CipherAlg & 0x08) >> 3);
	WCIDAttri.field.PairKeyMode = (CipherAlg & 0x07);

	/* Update the MBSS index.
	   Use bit11 and bit6~4 to indicate the BSS index */
	WCIDAttri.field.BSSIdxExt = ((BssIdx & 0x08) >> 3);
	WCIDAttri.field.BSSIdx = (BssIdx & 0x07);


	/* Assign Key Table selection */
	WCIDAttri.field.KeyTab = KeyTabFlag;

	/* Update related information to ASIC */
	offset = wcid_attr_base + (Wcid * wcid_attr_size);
	mt7612u_write32(pAd, offset, WCIDAttri.word);

	DBGPRINT(RT_DEBUG_TRACE, ("%s:WCID #%d, KeyIdx #%d, WCIDAttri=0x%x, Alg=%s\n",
					__FUNCTION__, Wcid, KeyIdx, WCIDAttri.word, CipherName[CipherAlg]));
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicDelWcidTab(struct rtmp_adapter *pAd, UCHAR wcid_idx)
{
	uint32_t offset;
	UCHAR cnt, cnt_s, cnt_e;
#ifdef MCS_LUT_SUPPORT
	uint32_t mcs_tb_offset = 0;
#endif /* MCS_LUT_SUPPORT */


	DBGPRINT(RT_DEBUG_TRACE, ("AsicDelWcidTab==>wcid_idx = 0x%x\n",wcid_idx));
	if (wcid_idx == WCID_ALL) {
		cnt_s = 0;
		cnt_e = (WCID_ALL - 1);
	} else {
#ifdef MCS_LUT_SUPPORT
		if (RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT))
			mcs_tb_offset = 0x400;
#endif /* MCS_LUT_SUPPORT */
		cnt_s = cnt_e = wcid_idx;
	}

	for (cnt = cnt_s; cnt_s <= cnt_e; cnt_s++)
	{
		offset = MAC_WCID_BASE + cnt * HW_WCID_ENTRY_SIZE;
		mt7612u_write32(pAd, offset, 0x0);
		mt7612u_write32(pAd, offset + 4, 0x0);
#ifdef MCS_LUT_SUPPORT
		if (mcs_tb_offset) {
			offset += mcs_tb_offset;
			mt7612u_write32(pAd, offset, 0x0);
			mt7612u_write32(pAd, offset + 4, 0x0);
		}
#endif /* MCS_LUT_SUPPORT */
	}
}


/*
	========================================================================
	Description:
		Add Pair-wise key material into ASIC.
		Update pairwise key, TxMic and RxMic to Asic Pair-wise key table

    Return:
	========================================================================
*/
VOID AsicAddPairwiseKeyEntry(
	IN struct rtmp_adapter *	pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey)
{
	INT i;
	ULONG offset;
	uint32_t pairwise_key_base = 0, pairwise_key_len = 0;
	UCHAR *pKey = pCipherKey->Key;
	UCHAR *pTxMic = pCipherKey->TxMic;
	UCHAR *pRxMic = pCipherKey->RxMic;
	UCHAR CipherAlg = pCipherKey->CipherAlg;

	pairwise_key_base = RLT_PAIRWISE_KEY_TABLE_BASE;
	pairwise_key_len = RLT_HW_KEY_ENTRY_SIZE;

	/* EKEY */
	offset = pairwise_key_base + (WCID * pairwise_key_len);
#ifdef RTMP_MAC_USB
	RTUSBMultiWrite(pAd, offset, &pCipherKey->Key[0], MAX_LEN_OF_PEER_KEY);
#endif /* RTMP_MAC_USB */
	for (i=0; i<MAX_LEN_OF_PEER_KEY; i+=4)
	{
		uint32_t Value;
		Value = mt7612u_read32(pAd, offset + i);
	}
	offset += MAX_LEN_OF_PEER_KEY;

	/*  MIC KEY */
	if (pTxMic)
	{
#ifdef RTMP_MAC_USB
		RTUSBMultiWrite(pAd, offset, &pCipherKey->TxMic[0], 8);
#endif /* RTMP_MAC_USB */
	}
	offset += 8;

	if (pRxMic)
	{
#ifdef RTMP_MAC_USB
		RTUSBMultiWrite(pAd, offset, &pCipherKey->RxMic[0], 8);
#endif /* RTMP_MAC_USB */
	}
	DBGPRINT(RT_DEBUG_TRACE,("AsicAddPairwiseKeyEntry: WCID #%d Alg=%s\n",WCID, CipherName[CipherAlg]));
	DBGPRINT(RT_DEBUG_TRACE,("	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}
}


/*
	========================================================================
	Description:
		Remove Pair-wise key material from ASIC.

    Return:
	========================================================================
*/
VOID AsicRemovePairwiseKeyEntry(
	IN struct rtmp_adapter *pAd,
	IN UCHAR		 Wcid)
{
	/* Set the specific WCID attribute entry as OPEN-NONE */
	AsicUpdateWcidAttributeEntry(pAd,
							  BSS0,
							  0,
							  CIPHER_NONE,
							  Wcid,
							  PAIRWISEKEYTABLE);
}


bool AsicSendCommandToMcu(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN bool in_atomic)
{
	// TODO: shiang-6590, fix me, currently firmware is not ready yet, so ignore it!
	if (IS_RT65XX(pAd))
		return true;


	if (pAd->chipOps.sendCommandToMcu)
		return pAd->chipOps.sendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);
	else
		return false;
}


bool AsicSendCmdToMcuAndWait(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN bool in_atomic)
{
	bool cmd_done = true;

	AsicSendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);


	return cmd_done;
}


bool AsicSendCommandToMcuBBP(
	IN struct rtmp_adapter *pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN bool		FlgIsNeedLocked)
{
	// TODO: shiang-6590, fix me, currently firmware is not ready yet, so ignore it!
	if (IS_RT65XX(pAd))
		return true;


	if (pAd->chipOps.sendCommandToMcu)
		return pAd->chipOps.sendCommandToMcu(pAd, Command, Token, Arg0, Arg1, FlgIsNeedLocked);
	else
		return false;
}

/*
	========================================================================
	Description:
		For 1x1 chipset : 2070 / 3070 / 3090 / 3370 / 3390 / 5370 / 5390
		Usage :	1. Set Default Antenna as initialize
				2. Antenna Diversity switching used
				3. iwpriv command switch Antenna

    Return:
	========================================================================
 */
VOID AsicSetRxAnt(
	IN struct rtmp_adapter *pAd,
	IN UCHAR			Ant)
{
	if (pAd->chipOps.SetRxAnt)
		pAd->chipOps.SetRxAnt(pAd, Ant);
}











VOID AsicSetTxPreamble(struct rtmp_adapter *pAd, USHORT TxPreamble)
{
	AUTO_RSP_CFG_STRUC csr4;

	csr4.word = mt7612u_read32(pAd, AUTO_RSP_CFG);
	if (TxPreamble == Rt802_11PreambleLong)
		csr4.field.AutoResponderPreamble = 0;
	else
		csr4.field.AutoResponderPreamble = 1;
	mt7612u_write32(pAd, AUTO_RSP_CFG, csr4.word);
}


INT AsicSetRalinkBurstMode(struct rtmp_adapter *pAd, bool enable)
{
	uint32_t 			Data = 0;

	Data = mt7612u_read32(pAd, EDCA_AC0_CFG);
	if (enable)
	{
	pAd->CommonCfg.RestoreBurstMode = Data;
	Data  &= 0xFFF00000;
	Data  |= 0x86380;
	} else {
	Data = pAd->CommonCfg.RestoreBurstMode;
	Data &= 0xFFFFFF00;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE)
		&& (pAd->MacTab.fAnyStationMIMOPSDynamic == false)
	)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
			Data |= 0x80;
		else if (pAd->CommonCfg.bEnableTxBurst)
			Data |= 0x20;
	}
	}
	mt7612u_write32(pAd, EDCA_AC0_CFG, Data);

	if (enable)
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE);
	else
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE);

	return true;
}


#ifdef WOW_SUPPORT
#ifdef RTMP_MAC_USB

/* switch firmware
   a) before into WOW mode, switch firmware to WOW-enable firmware
   b) exit from WOW mode, switch firmware to normal firmware
*/
VOID AsicLoadWOWFirmware(
	IN struct rtmp_adapter *pAd,
	IN bool WOW)
{
	if (WOW)
		pAd->WOW_Cfg.bWOWFirmware = true;
	else
		pAd->WOW_Cfg.bWOWFirmware = false;

	RtmpAsicLoadFirmware(pAd);
}

/* In WOW mode, 8051 mcu will send null frame, and pick data from 0x7780
 * the null frame includes TxWI and 802.11 header 						*/
VOID AsicWOWSendNullFrame(
	IN struct rtmp_adapter *pAd,
	IN UCHAR TxRate,
	IN bool bQosNull)
{

	TXWI_STRUC *TxWI;
	u8 *NullFrame;
	UINT8  packet_len;
	u8 *ptr;
	USHORT offset;
	uint32_t cipher = pAd->StaCfg.GroupCipher;
	uint32_t Value;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	ComposeNullFrame(pAd);
	TxWI = (TXWI_STRUC *)&pAd->NullContext.TransferBuffer->field.WirelessPacket[TXINFO_SIZE];
	NullFrame = (u8 *)&pAd->NullFrame;
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
			packet_len = TxWI->TXWI_N.MPDUtotalByteCnt;
	}
#endif /* RLT_MAC */

	DBGPRINT(RT_DEBUG_OFF, ("TxWI:\n"));
	/* copy TxWI to MCU memory */
	ptr = (u8 *)TxWI;
	for (offset = 0; offset < TXWISize; offset += 4)
	{
		RTMPMoveMemory(&Value, ptr+offset, 4);
		DBGPRINT(RT_DEBUG_OFF, ("offset: %02d %08x\n", offset, Value));
		mt7612u_write32(pAd, HW_NULL2_BASE + offset, Value);
	}

	DBGPRINT(RT_DEBUG_OFF, ("802.11 header:\n"));
	/* copy 802.11 header to memory */
	ptr = (u8 *)NullFrame;
	for (offset = 0; offset < packet_len; offset += 4)
	{
		RTMPMoveMemory(&Value, ptr+offset, 4);
		DBGPRINT(RT_DEBUG_OFF, ("offset: %02d %08x\n", offset, Value));
		mt7612u_write32(pAd, HW_NULL2_BASE + TXWISize + offset, Value);
	}

	DBGPRINT(RT_DEBUG_OFF, ("Write GroupCipher Mode: %d\n", pAd->StaCfg.GroupCipher));

	{
		uint32_t share_key_mode_base = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
			share_key_mode_base= RLT_SHARED_KEY_MODE_BASE;
#endif /* RLT_MAC */

		Value = mt7612u_read32(pAd, share_key_mode_base);
		switch (cipher) /* don't care WEP, because it dosen't have re-key issue */
		{
			case Ndis802_11TKIPEnable: /* TKIP */
				Value |= 0x0330;
				break;
			case Ndis802_11AESEnable: /* AES */
				Value |= 0x0440;
				break;
		}
		mt7612u_write32(pAd, share_key_mode_base, Value);
	}
}

#endif /* RTMP_MAC_USB */
#endif /* WOW_SUPPORT */


INT AsicSetPreTbttInt(struct rtmp_adapter *pAd, bool enable)
{
	uint32_t val;

	val = mt7612u_read32(pAd, INT_TIMER_CFG);
	val &= 0xffff0000;
	val |= 6 << 4; /* Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable. */
	mt7612u_write32(pAd, INT_TIMER_CFG, val);
	/* Enable pre-tbtt interrupt */
	val = mt7612u_read32(pAd, INT_TIMER_EN);
	val |=0x1;
	mt7612u_write32(pAd, INT_TIMER_EN, val);

	return true;
}


bool AsicWaitPDMAIdle(struct rtmp_adapter *pAd, INT round, INT wait_us)
{
	INT i = 0;
	WPDMA_GLO_CFG_STRUC GloCfg;


	do {
		GloCfg.word = mt7612u_read32(pAd, WPDMA_GLO_CFG);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0)) {
			DBGPRINT(RT_DEBUG_TRACE, ("==>  DMAIdle, GloCfg=0x%x\n", GloCfg.word));
			return true;
		}
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return false;
		RtmpusecDelay(wait_us);
	}while ((i++) < round);

	DBGPRINT(RT_DEBUG_TRACE, ("==>  DMABusy, GloCfg=0x%x\n", GloCfg.word));

	return false;
}


INT rtmp_asic_top_init(struct rtmp_adapter *pAd)
{
	uint32_t mac_val;

#ifdef RLT_MAC
	if (IS_MT76x2(pAd)) {
		uint32_t MacValue;
		MacValue = mt7612u_read32(pAd, MAC_CSR0);
		pAd->MACVersion = MacValue;

		if ((pAd->MACVersion == 0xffffffff) || (pAd->MACVersion == 0))
			mt76x2_pwrOn(pAd);
	}

	if (IS_MT76x0(pAd) || IS_MT76x2(pAd) || IS_MT7601(pAd)) {
		if (pAd->WlanFunCtrl.field.WLAN_EN == 0)
			rlt_wlan_chip_onoff(pAd, true, false);
	}
#endif /* RLT_MAC */

	/* Make sure MAC gets ready.*/
	if (WaitForAsicReady(pAd) != true)
		return false;


	return true;
}


INT StopDmaRx(struct rtmp_adapter *pAd, UCHAR Level)
{
	struct sk_buff *pRxPacket;
	RX_BLK RxBlk, *pRxBlk;
	uint32_t RxPending = 0, MacReg = 0, MTxCycle = 0;
	bool bReschedule = false;
	bool bCmdRspPacket = false;
#ifdef RTMP_MAC_USB
	UINT8 IdleNums = 0;
#endif /* RTMP_MAC_USB */

	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));

	/*
		process whole rx ring
	*/
	while (1)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return 0;
		pRxBlk = &RxBlk;
		pRxPacket = GetPacketFromRxRing(pAd, pRxBlk, &bReschedule, &RxPending, 0);
		bCmdRspPacket = false;
		if ((RxPending == 0) && (bReschedule == false))
			break;
		else
			dev_kfree_skb_any(pRxPacket);
	}

	/*
		Check DMA Rx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{

#ifdef RTMP_MAC_USB
		MacReg = mt7612u_usb_cfg_read_v3(pAd);
		if ((MacReg & 0x40000000) && (IdleNums < 10))
		{
			IdleNums++;
			RtmpusecDelay(50);
		}
		else
		{
			break;
		}
#endif

		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return 0;
		}
	}

	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:RX DMA busy!! DMA_CFG = 0x%08x\n", __FUNCTION__, MacReg));
	}

	if (Level == RTMP_HALT)
	{
		/* Disable DMA RX */
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<==== %s\n", __FUNCTION__));

	return 0;
}


INT StopDmaTx(struct rtmp_adapter *pAd, UCHAR Level)
{
	uint32_t MacReg = 0, MTxCycle = 0;
#ifdef RTMP_MAC_USB
	UINT8 IdleNums = 0;
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));

	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{

#ifdef RTMP_MAC_USB
		MacReg = mt7612u_usb_cfg_read_v3(pAd);
		if (((MacReg & 0x80000000) == 0) && IdleNums > 10)
		{
			break;
		}
		else
		{
			IdleNums++;
			RtmpusecDelay(50);
		}
#endif

		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return 0;
		}
	}

	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("TX DMA busy!! DMA_CFG(%x)\n", MacReg));
	}

	if (Level == RTMP_HALT)
	{

	}

	DBGPRINT(RT_DEBUG_TRACE, ("<==== %s\n", __FUNCTION__));

	return 0;
}


#define MAX_AGG_CNT	48

INT AsicReadAggCnt(struct rtmp_adapter *pAd, ULONG *aggCnt, int cnt_len)
{
	uint32_t reg_addr;
	TX_AGG_CNT_STRUC reg_val;
	int i, cnt, seg;
	static USHORT aggReg[] = {
						TX_AGG_CNT, TX_AGG_CNT3,
#if MAX_AGG_CNT > 8
						TX_AGG_CNT4, TX_AGG_CNT7,
#endif
#if MAX_AGG_CNT > 16
						TX_AGG_CNT8, TX_AGG_CNT15,
#endif
#if MAX_AGG_CNT > 32
						TX_AGG_CNT16, TX_AGG_CNT23,
#endif
	};


	memset(aggCnt, 0, cnt_len * sizeof(ULONG));
	seg = (sizeof(aggReg) /sizeof(USHORT));

	cnt = 0;
	for (i = 0; i < seg; i += 2)
	{
		for (reg_addr = aggReg[i] ; reg_addr <= aggReg[i+1] ; reg_addr += 4)
		{
			reg_val.word = mt7612u_read32(pAd, reg_addr);
			if (cnt < (cnt_len -1)) {
				aggCnt[cnt] = reg_val.field.AggCnt_x;
				aggCnt[cnt+1] = reg_val.field.AggCnt_y;
				DBGPRINT(RT_DEBUG_TRACE, ("%s():Get AggSize at Reg(0x%x) with val(0x%08x) [AGG_%d=>%ld, AGG_%d=>%ld]\n",
						__FUNCTION__, reg_addr, reg_val.word, cnt, aggCnt[cnt], cnt+1, aggCnt[cnt+1]));
				cnt += 2;
			} else {
				DBGPRINT(RT_DEBUG_TRACE, ("%s():Get AggSize at Reg(0x%x) failed, no enough buffer(cnt_len=%d, cnt=%d)\n",
							__FUNCTION__, reg_addr, cnt_len, cnt));
			}
		}
	}

	return true;
}



INT AsicSetChannel(struct rtmp_adapter *pAd, UCHAR ch, UINT8 bw, UINT8 ext_ch, bool bScan)
{
	mt7612u_bbp_set_bw(pAd, bw);

	/*  Tx/RX : control channel setting */
	mt7612u_bbp_set_ctrlch(pAd, ext_ch);
	mt7612u_mac_set_ctrlch(pAd, ext_ch);

	/* Let BBP register at 20MHz to do scan */
	AsicSwitchChannel(pAd, ch, bScan);
	AsicLockChannel(pAd, ch);

#ifdef RT28xx
	RT28xx_ch_tunning(pAd, bw);
#endif /* RT28xx */

	return 0;
}


#ifdef MAC_APCLI_SUPPORT
/*
	==========================================================================
	Description:
		Set BSSID of Root AP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetApCliBssid(
	IN struct rtmp_adapter *pAd,
	IN u8 *pBssid,
	IN UCHAR index)
{
	uint32_t 	  Addr4 = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s():%x:%x:%x:%x:%x:%x\n",
				__FUNCTION__, PRINT_MAC(pBssid)));

	Addr4 = (uint32_t)(pBssid[0]) |
			(uint32_t)(pBssid[1] << 8)  |
			(uint32_t)(pBssid[2] << 16) |
			(uint32_t)(pBssid[3] << 24);
	mt7612u_write32(pAd, MAC_APCLI_BSSID_DW0, Addr4);

	Addr4 = 0;
	Addr4 = (ULONG)(pBssid[4]) | (ULONG)(pBssid[5] << 8);
	/* Enable APCLI mode */
	Addr4 |= 0x10000;

	mt7612u_write32(pAd, MAC_APCLI_BSSID_DW1, Addr4);
}
#endif /* MAC_APCLI_SUPPORT */

#ifdef NEW_WOW_SUPPORT
VOID RT28xxAndesWOWEnable(
	IN struct rtmp_adapter *pAd)
{
	NEW_WOW_MASK_CFG_STRUCT mask_cfg;
	NEW_WOW_SEC_CFG_STRUCT sec_cfg;
	NEW_WOW_INFRA_CFG_STRUCT infra_cfg;
	NEW_WOW_P2P_CFG_STRUCT p2p_cfg;
	NEW_WOW_PARAM_STRUCT wow_param;
	struct CMD_UNIT CmdUnit;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	int32_t Ret;
	MAC_TABLE_ENTRY *pEntry = NULL;


	memset(&CmdUnit, sizeof(CmdUnit));

	/* WOW enable */
	memset(&wow_param, sizeof(wow_param));

	wow_param.Parameter = WOW_ENABLE; /* WOW enable */
	wow_param.Value = true;

	CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* feature enable */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (u8 *)&wow_param;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, wow_param.Parameter);
		return;
	}

	mdelay(1);
	/* mask configuration */
	memset(&mask_cfg, sizeof(mask_cfg));

	mask_cfg.Config_Type = WOW_MASK_CFG; 	/* detect mask config */
	mask_cfg.Function_Enable = true;
	mask_cfg.Detect_Mask = 1UL << WOW_MAGIC_PKT;	/* magic packet */
	mask_cfg.Event_Mask = 0;

	CmdUnit.u.ANDES.Type = CMD_WOW_CONFIG; /* WOW config */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_MASK_CFG_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (u8 *)&mask_cfg;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed!!(%d/%d)\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, mask_cfg.Config_Type);
		return;
	}

	mdelay(1);

	/* security configuration */
	if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPAPSK)
	{
		memset(&sec_cfg, sizeof(sec_cfg));

		sec_cfg.Config_Type = WOW_SEC_CFG; 	/* security config */

		if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK)
			sec_cfg.WPA_Ver = 0;
		else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
			sec_cfg.WPA_Ver = 1;

		pEntry = &pAd->MacTab.Content[BSSID_WCID];

		memcpy(sec_cfg.PTK, pEntry->PTK, 64);
		memcpy(sec_cfg.R_COUNTER, pEntry->R_Counter, LEN_KEY_DESC_REPLAY);

		sec_cfg.Key_Id = pAd->StaCfg.DefaultKeyId;
		sec_cfg.Cipher_Alg = pEntry->WepStatus;
		printk("\x1b[31m%s: wep status %d\x1b[m\n", __FUNCTION__, pEntry->WepStatus);
		sec_cfg.Group_Cipher = pAd->StaCfg.GroupCipher;
		printk("\x1b[31m%s: group status %d\x1b[m\n", __FUNCTION__, sec_cfg.Group_Cipher);
		printk("\x1b[31m%s: aid %d\x1b[m\n", __FUNCTION__, pEntry->Aid);
		sec_cfg.WCID = BSSID_WCID;

		CmdUnit.u.ANDES.Type = CMD_WOW_CONFIG; /* WOW config */
		CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_SEC_CFG_STRUCT);
		CmdUnit.u.ANDES.CmdPayload = (u8 *)&sec_cfg;

		Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

		if (Ret != NDIS_STATUS_SUCCESS)
		{
			printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, sec_cfg.Config_Type);
			return;
		}

		mdelay(1);
	}

	/* Infra configuration */

	memset(&infra_cfg, sizeof(infra_cfg));

	infra_cfg.Config_Type = WOW_INFRA_CFG; 	/* infra config */

	COPY_MAC_ADDR(infra_cfg.STA_MAC, pAd->CurrentAddress);
	COPY_MAC_ADDR(infra_cfg.AP_MAC, pAd->CommonCfg.Bssid);

	CmdUnit.u.ANDES.Type = CMD_WOW_CONFIG; /* WOW config */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_INFRA_CFG_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (u8 *)&infra_cfg;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
		infra_cfg.AP_Status = true;
	else
		infra_cfg.AP_Status = false;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, infra_cfg.Config_Type);
		return;
	}

	mdelay(1);


	/* P2P configuration */

	/* Wakeup Option */
	memset(&wow_param, sizeof(wow_param));

	wow_param.Parameter = WOW_WAKEUP; /* Wakeup Option */
	if (pAd->WOW_Cfg.bInBand)
	{
		wow_param.Value = WOW_WAKEUP_BY_USB;
	}
	else
	{
		int32_t Value;

		wow_param.Value = WOW_WAKEUP_BY_GPIO;

		Value = mt7612u_read32(pAd, WLAN_FUN_CTRL);
		printk("\x1b[31m%s: 0x80 = %x\x1b[m\n", __FUNCTION__, Value);
		Value &= ~0x01010000; /* GPIO0(ouput) --> 0(data) */
		mt7612u_write32(pAd, WLAN_FUN_CTRL, Value);
	}

	CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* feature enable */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (u8 *)&wow_param;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, wow_param.Parameter);
		return;
	}

	mdelay(1);


	/* traffic to Andes */
	memset(&wow_param, sizeof(wow_param));
	wow_param.Parameter = WOW_TRAFFIC; /* Traffic switch */
	wow_param.Value = WOW_PKT_TO_ANDES;	/* incoming packet to FW */

	CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* feature enable */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (u8 *)&wow_param.Parameter;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, wow_param.Parameter);
		return;
	}

	mdelay(1);

    RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
}

VOID RT28xxAndesWOWDisable(
    IN struct rtmp_adapter *pAd)
{
    NEW_WOW_PARAM_STRUCT param;
    struct CMD_UNIT CmdUnit;
    RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
    int32_t Ret;
    uint32_t Value;
    MAC_TABLE_ENTRY *pEntry = NULL;

    printk("\x1b[31m%s: ...\x1b[m", __FUNCTION__);

    /* clean BulkIn Reset flag */
    //pAd->Flags &= ~0x80000;
    RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

    memset(&CmdUnit, sizeof(CmdUnit));

    /* WOW disable */
    memset(&param, sizeof(param));
    param.Parameter = WOW_ENABLE;
    param.Value = false;

    CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* WOW enable */
    CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
    CmdUnit.u.ANDES.CmdPayload = (u8 *)&param;

    Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

    if (Ret != NDIS_STATUS_SUCCESS)
    {
        printk("\x1b[31m%s: send WOW config command failed!!\x1b[m\n", __FUNCTION__);
        return;
    }

    mdelay(1);


    /* traffic to Host */
    memset(&param, 0, sizeof(param));
    param.Parameter = WOW_TRAFFIC;
    param.Value = WOW_PKT_TO_HOST;

    CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE;
    CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
    CmdUnit.u.ANDES.CmdPayload = (u8 *)&param;

    Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

    if (Ret != NDIS_STATUS_SUCCESS)
    {
        printk("\x1b[31m%s: send WOW config command failed!!\x1b[m\n", __FUNCTION__);
        return;
    }

    mdelay(1);


    /* Restore MAC TX/RX */
    Value = mt7612u_read32(pAd, MAC_SYS_CTRL);
    Value |= 0xC;
    mt7612u_write32(pAd, MAC_SYS_CTRL, Value);


    RTUSBBulkReceive(pAd);
    RTUSBBulkCmdRspEventReceive(pAd);

    //printk("\x1b[31m%s: pendingRx %d\x1b[m\n", __FUNCTION__, pAd->PendingRx);
    //printk("\x1b[31m%s: BulkInReq %d\x1b[m\n", __FUNCTION__, pAd->BulkInReq);

    /* restore hardware remote wakeup flag */
    Value = mt7612u_read32(pAd, WLAN_FUN_CTRL);
    printk("\x1b[31m%s: 0x80 %08x\x1b[m\n", __FUNCTION__, Value);
    Value &= ~0x80;
    mt7612u_write32(pAd, WLAN_FUN_CTRL, Value);

	if (pAd->WOW_Cfg.bInBand == false)
	{
		int32_t Value;

		Value = mt7612u_read32(pAd, WLAN_FUN_CTRL);
		printk("\x1b[31m%s: 0x80 = %x\x1b[m\n", __FUNCTION__, Value);
		Value &= ~0x01010000; /* GPIO0(ouput) --> 0(data) */
		mt7612u_write32(pAd, WLAN_FUN_CTRL, Value);
	}
}

#endif /* NEW_WOW_SUPPORT */

#ifdef DROP_MASK_SUPPORT
VOID asic_set_drop_mask(
	struct rtmp_adapter *ad,
	USHORT	wcid,
	bool enable)
{
	uint32_t mac_reg = 0, reg_id, group_index;
	uint32_t drop_mask = (1 << (wcid % 32));

	/* each group has 32 entries */
	group_index = (wcid - (wcid % 32)) >> 5 /* divided by 32 */;
	reg_id = (TX_WCID_DROP_MASK0 + 4*group_index);

	mac_reg = mt7612u_read32(ad, reg_id);

	mac_reg = (enable ? \
				(mac_reg | drop_mask):(mac_reg & ~drop_mask));
	mt7612u_write32(ad, reg_id, mac_reg);
	DBGPRINT(RT_DEBUG_TRACE,
			("%s(%u):, wcid = %u, reg_id = 0x%08x, mac_reg = 0x%08x, group_index = %u, drop_mask = 0x%08x\n",
			__FUNCTION__, enable, wcid, reg_id, mac_reg, group_index, drop_mask));
}


VOID asic_drop_mask_reset(
	struct rtmp_adapter *ad)
{
	uint32_t i, reg_id;

	for ( i = 0; i < 8 /* num of drop mask group */; i++)
	{
		reg_id = (TX_WCID_DROP_MASK0 + i*4);
		mt7612u_write32(ad, reg_id, 0);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s()\n", __FUNCTION__));
}
#endif /* DROP_MASK_SUPPORT */

#ifdef MULTI_CLIENT_SUPPORT
VOID asic_change_tx_retry(
	IN struct rtmp_adapter *pAd,
	IN USHORT num)
{
	uint32_t TxRtyCfg, MacReg = 0;

	if (pAd->CommonCfg.txRetryCfg == 0) {
		/* txRetryCfg is invalid, should not be 0 */
		DBGPRINT(RT_DEBUG_TRACE, ("txRetryCfg=%x\n", pAd->CommonCfg.txRetryCfg));
		return ;
	}

	if (num < 3)
	{
		/* Tx date retry default 15 */
		TxRtyCfg = mt7612u_read32(pAd, TX_RTY_CFG);
		TxRtyCfg = ((TxRtyCfg & 0xffff0000) | (pAd->CommonCfg.txRetryCfg & 0x0000ffff));
		mt7612u_write32(pAd, TX_RTY_CFG, TxRtyCfg);

		/* Tx RTS retry default 32 */
		mt7612u_read32(pAd, TX_RTS_CFG, &MacReg);
		MacReg &= 0xFEFFFF00;
		MacReg |= 0x20;
		mt7612u_write32(pAd, TX_RTS_CFG, MacReg);
	}
	else
	{
		/* Tx date retry 10 */
		TxRtyCfg = 0x4100080A;
		mt7612u_write32(pAd, TX_RTY_CFG, TxRtyCfg);

		/* Tx RTS retry 3 */
		MacReg = mt7612u_read32(pAd, TX_RTS_CFG);
		MacReg &= 0xFEFFFF00;
		MacReg |= 0x01000003;
		mt7612u_write32(pAd, TX_RTS_CFG, MacReg);

		/* enable fallback legacy */
		if (pAd->CommonCfg.Channel > 14)
			mt7612u_write32(pAd, HT_FBK_TO_LEGACY, 0x1818);
		else
			mt7612u_write32(pAd, HT_FBK_TO_LEGACY, 0x1010);
	}
}

VOID pkt_aggr_num_change(
	IN struct rtmp_adapter *pAd,
	IN USHORT num)
{
	if (IS_RT6352(pAd))
	{
		if (num < 5)
		{
			/* use default */
			mt7612u_write32(pAd, AMPDU_MAX_LEN_20M1S, 0x77777777);
			mt7612u_write32(pAd, AMPDU_MAX_LEN_20M2S, 0x77777777);
			mt7612u_write32(pAd, AMPDU_MAX_LEN_40M1S, 0x77777777);
			mt7612u_write32(pAd, AMPDU_MAX_LEN_40M2S, 0x77777777);
		}
		else
		{
			/* modify by MCS */
			mt7612u_write32(pAd, AMPDU_MAX_LEN_20M1S, 0x77754433);
			mt7612u_write32(pAd, AMPDU_MAX_LEN_20M2S, 0x77765543);
			mt7612u_write32(pAd, AMPDU_MAX_LEN_40M1S, 0x77765544);
			mt7612u_write32(pAd, AMPDU_MAX_LEN_40M2S, 0x77765544);
		}
	}

}

VOID asic_tune_be_wmm(
	IN struct rtmp_adapter *pAd,
	IN USHORT num)
{
	UCHAR  bssCwmin = 4, apCwmin = 4, apCwmax = 6;

	if (num <= 4)
	{
		/* use profile cwmin */
		if (pAd->CommonCfg.APCwmin > 0 && pAd->CommonCfg.BSSCwmin > 0 && pAd->CommonCfg.APCwmax > 0)
		{
			apCwmin = pAd->CommonCfg.APCwmin;
			apCwmax = pAd->CommonCfg.APCwmax;
			bssCwmin = pAd->CommonCfg.BSSCwmin;
		}
	}
	else if (num > 4 && num <= 8)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 5;
	}
	else if (num > 8 && num <= 16)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 6;
	}
	else if (num > 16 && num <= 64)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 7;
	}
	else if (num > 64 && num <= 128)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 8;
	}

	pAd->CommonCfg.APEdcaParm.Cwmin[0] = apCwmin;
	pAd->CommonCfg.APEdcaParm.Cwmax[0] = apCwmax;
	pAd->ApCfg.BssEdcaParm.Cwmin[0] = bssCwmin;

	AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);
}
#endif /* MULTI_CLIENT_SUPPORT */

