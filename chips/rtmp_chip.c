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
	rt_chip.c

	Abstract:
	Ralink Wireless driver CHIP related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include "rt_config.h"


/*
========================================================================
Routine Description:
	Initialize specific beacon frame architecture.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipBcnSpecInit(struct rtmp_adapter *pAd)
{
}

#ifdef RLT_MAC
/*
========================================================================
Routine Description:
	Initialize specific beacon frame architecture.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID rlt_bcn_buf_init(struct rtmp_adapter *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	pChipCap->FlgIsSupSpecBcnBuf = FALSE;

/*
 * ULLI : strange in mt7610u this is
*	pChipCap->BcnMaxHwNum = 16;
*	pChipCap->WcidHwRsvNum = 255;
*/

	pChipCap->BcnMaxHwNum = 8;
	pChipCap->WcidHwRsvNum = 127;

/*
	In 16-MBSS support mode, if AP-Client is enabled,
	the last 8-MBSS would be occupied for AP-Client using.
*/
#ifdef APCLI_SUPPORT
	pChipCap->BcnMaxNum = (8 - MAX_MESH_NUM);
#else
	pChipCap->BcnMaxNum = (pChipCap->BcnMaxHwNum - MAX_MESH_NUM);
#endif /* APCLI_SUPPORT */

	pChipCap->BcnMaxHwSize = 0x2000;

	pChipCap->BcnBase[0] = 0xc000;
	pChipCap->BcnBase[1] = 0xc200;
	pChipCap->BcnBase[2] = 0xc400;
	pChipCap->BcnBase[3] = 0xc600;
	pChipCap->BcnBase[4] = 0xc800;
	pChipCap->BcnBase[5] = 0xca00;
	pChipCap->BcnBase[6] = 0xcc00;
	pChipCap->BcnBase[7] = 0xce00;
	pChipCap->BcnBase[8] = 0xd000;
	pChipCap->BcnBase[9] = 0xd200;
	pChipCap->BcnBase[10] = 0xd400;
	pChipCap->BcnBase[11] = 0xd600;
	pChipCap->BcnBase[12] = 0xd800;
	pChipCap->BcnBase[13] = 0xda00;
	pChipCap->BcnBase[14] = 0xdc00;
	pChipCap->BcnBase[15] = 0xde00;
}
#endif /* RLT_MAC */

INT WaitForAsicReady(struct rtmp_adapter *pAd)
{
	uint32_t mac_val = 0, reg = MAC_CSR0;
	int idx = 0;

	do
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return FALSE;

		mac_val = mt7612u_read32(pAd, reg);
		if ((mac_val != 0x00) && (mac_val != 0xFFFFFFFF))
			return TRUE;

		RtmpOsMsDelay(5);
	} while (idx++ < 500);

	DBGPRINT(RT_DEBUG_ERROR,
				("%s(0x%x):AsicNotReady!\n",
				__FUNCTION__, mac_val));

	return FALSE;
}




/*
========================================================================
Routine Description:
	Initialize chip related information.

Arguments:
	pCB				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
int RtmpChipOpsHook(VOID *pCB)
{
	struct rtmp_adapter *pAd = (struct rtmp_adapter *)pCB;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	uint32_t MacValue;
	int ret = 0;
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

	/* sanity check */
	if (WaitForAsicReady(pAd) == FALSE)
		return -1;

	MacValue = mt7612u_read32(pAd, MAC_CSR0);
	pAd->MACVersion = MacValue;

	if (pAd->MACVersion == 0xffffffff)
		return -1;

	MacValue = mt7612u_read32(pAd, ASIC_VERSION);
	pAd->ChipID = MacValue;

	if (pAd->ChipID == 0xffffffff)
		return -1;

	if (IS_MT76x2(pAd)) {
		mt76x2_init(pAd);
		goto done;
	}

	/* We depends on RfICType and MACVersion to assign the corresponding operation callbacks. */



done:
	DBGPRINT(RT_DEBUG_TRACE, ("Chip VCO calibration mode = %d!\n", pChipCap->FlgIsVcoReCalMode));
#ifdef DOT11W_PMF_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("[PMF] Encryption mode = %d\n", pChipCap->FlgPMFEncrtptMode));
#endif /* DOT11W_PMF_SUPPORT */

	return ret;
}

BOOLEAN isExternalPAMode(struct rtmp_adapter *ad, INT channel)
{
	BOOLEAN pa_mode = FALSE;

        if (channel > 14) {
	        if (ad->chipCap.PAType == EXT_PA_2G_5G)
                	pa_mode = TRUE;
                else if (ad->chipCap.PAType == EXT_PA_5G_ONLY)
                        pa_mode = TRUE;
                else
        	        pa_mode = FALSE;
        } else {
                if (ad->chipCap.PAType == EXT_PA_2G_5G)
                        pa_mode = TRUE;
                else if ((ad->chipCap.PAType == EXT_PA_5G_ONLY) ||
                         (ad->chipCap.PAType == INT_PA_2G_5G))
                        pa_mode = FALSE;
                else if (ad->chipCap.PAType == EXT_PA_2G_ONLY)
                        pa_mode = TRUE;
        }

	return pa_mode;
}

BOOLEAN is_external_lna_mode(struct rtmp_adapter *ad, INT channel)
{
	BOOLEAN lna_mode = FALSE;

	/* b'00: 2.4G+5G external LNA, b'01: 5G external LNA, b'10: 2.4G external LNA, b'11: Internal LNA */
	if (channel > 14) {
		if ((ad->chipCap.LNA_type == 0x0) || (ad->chipCap.LNA_type == 0x1))
	            	lna_mode = TRUE;
	    	else
	        	lna_mode = FALSE;
	} else {
	    	if ((ad->chipCap.LNA_type == 0x0) || (ad->chipCap.LNA_type == 0x10))
	            	lna_mode = TRUE;
	    	else
	            	lna_mode = FALSE;
	}

	return lna_mode;
}

