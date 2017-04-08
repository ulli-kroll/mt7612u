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
	mcu_and.c

	Abstract:
	on-chip CPU related codes

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include	"rt_config.h"


#ifdef RTMP_MAC_USB

/* Known USB Vendor Commands */
#define MT7612U_VENDOR_RESET		0x01	/* need better name */
#define MT7612U_VENDOR_SINGLE_WRITE	0x02
#define MT7612U_VENDOR_WRITE_MAC	0x06
#define MT7612U_VENDOR_READ_MAC		0x07
#define MT7612U_VENDOR_WRITE_EEPROM	0x08	/* Not used */
#define MT7612U_VENDOR_READ_EEPROM	0x09

#define MT7612U_VENDOR_USB_CFG_READ	0x47
#define MT7612U_VENDOR_USB_CFG_WRITE	0x46

static void mt7612u_vendor_reset(struct rtmp_adapter *pAd)
{
	RTUSB_VendorRequest(pAd, DEVICE_VENDOR_REQUEST_OUT,
			    MT7612U_VENDOR_RESET, 0x1, 0,
			    NULL, 0);

}

int mt7612u_mcu_usb_enable_patch(struct rtmp_adapter *ad)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct rtmp_chip_cap *cap = &ad->chipCap;

	/* enable patch command */
	u8 cmd[11];
	cmd[0] = 0x6F;
	cmd[1] = 0xFC;
	cmd[2] = 0x08;
	cmd[3] = 0x01;
	cmd[4] = 0x20;
	cmd[5] = 0x04;
	cmd[6] = 0x00;
	cmd[7] = (cap->rom_patch_offset & 0xFF);
	cmd[8] = (cap->rom_patch_offset & 0xFF00) >> 8;
	cmd[9] = (cap->rom_patch_offset & 0xFF0000) >> 16;
	cmd[10] = (cap->rom_patch_offset & 0xFF000000) >> 24;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	ret = RTUSB_VendorRequest(ad,
			  DEVICE_CLASS_REQUEST_OUT,
			  0x01,
			  0x12,
			  0x00,
			  cmd,
			  11);

	return ret;
}

int mt7612u_mcu_usb_reset_wmt(struct rtmp_adapter *ad)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct rtmp_chip_cap *cap = &ad->chipCap;

	/* reset command */
	u8 cmd[8] = {0x6F, 0xFC, 0x05, 0x01, 0x07, 0x01, 0x00, 0x04};

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTUSB_VendorRequest(ad,
			DEVICE_CLASS_REQUEST_OUT,
			0x01,
			0x12,
			0x00,
			cmd,
			8);

	return ret;
}

u16 checksume16(u8 *pData, int len)
{
	int sum = 0;

	while (len > 1) {
		sum += *((u16*)pData);

		pData = pData + 2;

		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);

		len -= 2;
	}

	if (len)
		sum += *((u8*)pData);

	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	return ~sum;
}

int mt7612u_mcu_usb_chk_crc(struct rtmp_adapter *ad, u32 checksum_len)
{
	int ret = 0;
	u8 cmd[8];
	struct rtmp_chip_cap *cap = &ad->chipCap;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	memmove(cmd, &cap->rom_patch_offset, 4);
	memmove(&cmd[4], &checksum_len, 4);

	ret = RTUSB_VendorRequest(ad,
			  DEVICE_VENDOR_REQUEST_OUT,
			  0x01,
			  0x20,
			  0x00,
			  cmd,
			  8);

	return ret;

}

u16 mt7612u_mcu_usb_get_crc(struct rtmp_adapter *ad)
{
	int ret = 0;
	u16 crc, count = 0;

	while (1) {
		ret = RTUSB_VendorRequest(ad,
				 DEVICE_VENDOR_REQUEST_IN,
				 0x01,
				 0x21,
				 0x00,
				 &crc,
				 2);

		if (crc != 0xFFFF)
			break;

		mdelay(100);

		if (count++ > 100) {
			DBGPRINT(RT_DEBUG_ERROR, ("Query CRC over %d times\n", count));
			break;
		}
	}

	return crc;
}

static void usb_upload_rom_patch_complete(struct urb *urb)
{
	struct completion *load_rom_patch_done = urb->context;

	complete(load_rom_patch_done);
}

int mt7612u_mcu_usb_load_rom_patch(struct rtmp_adapter *ad)
{
	PURB urb;
	struct usb_device *udev = ad->OS_Cookie->pUsb_Dev;
	ra_dma_addr_t rom_patch_dma;
	u8 *rom_patch_data;
	TXINFO_NMAC_CMD *tx_info;
	s32 sent_len;
	u32 cur_len = 0;
	u32 mac_value, loop = 0;
	u16 value;
	int ret = 0, total_checksum = 0;
	struct rtmp_chip_cap *cap = &ad->chipCap;
	USB_DMA_CFG_STRUC cfg;
	u32 patch_len = 0;
	struct completion load_rom_patch_done;
	u8 *fw_patch_image;
	const struct firmware *fw;

	dev_info(&udev->dev, "loading firmware patch %s\n", cap->fw_patch_name);

	ret = request_firmware(&fw, cap->fw_patch_name, &udev->dev);
	if (ret) {
		dev_info(&udev->dev, "loading failed patch %s\n", cap->fw_patch_name);
		return ret;
	}

	dev_info(&udev->dev, "firmware %s patch loaded\n", cap->fw_name);

	if (cap->rom_code_protect) {
load_patch_protect:
		mac_value = mt7612u_read32(ad, SEMAPHORE_03);
		loop++;

		if (((mac_value & 0x01) == 0x00) && (loop < GET_SEMAPHORE_RETRY_MAX)) {
			mdelay(1);
			goto load_patch_protect;
		}

		if (loop >= GET_SEMAPHORE_RETRY_MAX) {
			DBGPRINT(RT_DEBUG_ERROR,
				 ("%s: can not get the hw semaphore\n", __FUNCTION__));
			return NDIS_STATUS_FAILURE;
		}
	}

	/* Check rom patch if ready */
	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
		mac_value = mt7612u_read32(ad, CLOCK_CTL);

		if (((mac_value & 0x01) == 0x01) && (cap->rom_code_protect)) {
			goto error0;
		}
	} else {
		mac_value = mt7612u_read32(ad, COM_REG0);

		if (((mac_value & 0x02) == 0x02) && (cap->rom_code_protect)) {
			goto error0;
		}
	}

	/* Enable USB_DMA_CFG */
	cfg.word = mt7612u_usb_cfg_read_v3(ad);
	cfg.word |= 0x00c00020;
	mt7612u_usb_cfg_write_v3(ad, cfg.word);

	fw_patch_image = (u8 *) fw->data;

	mt7612u_vendor_reset(ad);
	mdelay(5);

	/* get rom patch information */
	DBGPRINT(RT_DEBUG_OFF, ("build time = \n"));

	for (loop = 0; loop < 16; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(fw_patch_image + loop)));

	if (IS_MT76x2(ad)) {
		if (((strncmp(fw_patch_image, "20130809", 8) >= 0)) && (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))) {
			DBGPRINT(RT_DEBUG_OFF, ("rom patch for E3 IC\n"));

		} else if (((strncmp(fw_patch_image, "20130809", 8) < 0)) && (MT_REV_LT(ad, MT76x2, REV_MT76x2E3))){

			DBGPRINT(RT_DEBUG_OFF, ("rom patch for E2 IC\n"));
		} else {
			DBGPRINT(RT_DEBUG_OFF, ("rom patch do not match IC version\n"));
			mac_value = mt7612u_read32(ad, 0x0);
			DBGPRINT(RT_DEBUG_OFF, ("IC version(%x)\n", mac_value));
			ret = NDIS_STATUS_FAILURE;
			goto error0;
		}
	}

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("platform = \n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(fw_patch_image + 16 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("hw/sw version = \n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(fw_patch_image + 20 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("patch version = \n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(fw_patch_image + 24 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	/* Enable FCE */
	mt7612u_write32(ad, FCE_PSE_CTRL, 0x01);

	/* FCE tx_fs_base_ptr */
	mt7612u_write32(ad, TX_CPU_PORT_FROM_FCE_BASE_PTR, 0x400230);

	/* FCE tx_fs_max_cnt */
	mt7612u_write32(ad, TX_CPU_PORT_FROM_FCE_MAX_COUNT, 0x01);

	/* FCE pdma enable */
	mt7612u_write32(ad, FCE_PDMA_GLOBAL_CONF, 0x44);

	/* FCE skip_fs_en */
	mt7612u_write32(ad, FCE_SKIP_FS, 0x03);

	/* Allocate URB */
	urb = usb_alloc_urb(0, GFP_ATOMIC);

	if (!urb) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate URB\n"));
		ret = NDIS_STATUS_RESOURCES;
		goto error0;
	}

	/* Allocate TransferBuffer */
	rom_patch_data = usb_alloc_coherent(udev, UPLOAD_PATCH_UNIT, GFP_ATOMIC, &rom_patch_dma);

	if (!rom_patch_data) {
		ret = NDIS_STATUS_RESOURCES;
		goto error1;
	}

	DBGPRINT(RT_DEBUG_OFF, ("loading rom patch"));

	init_completion(&load_rom_patch_done);

	cur_len = 0x00;
	patch_len = fw->size - PATCH_INFO_SIZE;

	/* loading rom patch */
	while (1) {
		s32 sent_len_max = UPLOAD_PATCH_UNIT - sizeof(*tx_info) - USB_END_PADDING;
		sent_len = (patch_len - cur_len) >=  sent_len_max ? sent_len_max : (patch_len - cur_len);

		DBGPRINT(RT_DEBUG_OFF, ("patch_len = %d\n", patch_len));
		DBGPRINT(RT_DEBUG_OFF, ("cur_len = %d\n", cur_len));
		DBGPRINT(RT_DEBUG_OFF, ("sent_len = %d\n", sent_len));

		if (sent_len > 0) {
			tx_info = (TXINFO_NMAC_CMD *)rom_patch_data;
			tx_info->info_type = CMD_PACKET;
			tx_info->pkt_len = sent_len;
			tx_info->d_port = CPU_TX_PORT;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((u8 *)tx_info, TYPE_TXINFO);
#endif
			memmove(rom_patch_data + sizeof(*tx_info), fw_patch_image + PATCH_INFO_SIZE + cur_len, sent_len);

			/* four zero bytes for end padding */
			memset(rom_patch_data + sizeof(*tx_info) + sent_len, 0, 4);

			value = (cur_len + cap->rom_patch_offset) & 0xFFFF;

			DBGPRINT(RT_DEBUG_OFF, ("rom_patch_offset = %x\n", cap->rom_patch_offset));

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad,
					 DEVICE_VENDOR_REQUEST_OUT,
					 0x42,
					 value,
					 0x230,
					 NULL,
					 0);


			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma descriptor fail\n"));
				goto error2;
			}

			value = (((cur_len + cap->rom_patch_offset) & 0xFFFF0000) >> 16);

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad,
					 DEVICE_VENDOR_REQUEST_OUT,
					 0x42,
					 value,
					 0x232,
					 NULL,
					 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma descriptor fail\n"));
				goto error2;
			}

			cur_len += sent_len;

			while ((sent_len % 4) != 0)
				sent_len++;

			value = ((sent_len << 16) & 0xFFFF);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad,
					 DEVICE_VENDOR_REQUEST_OUT,
					 0x42,
					 value,
					 0x234,
					 NULL,
					 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma length fail\n"));
				goto error2;
			}

			value = (((sent_len << 16) & 0xFFFF0000) >> 16);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad,
					 DEVICE_VENDOR_REQUEST_OUT,
					 0x42,
					 value,
					 0x236,
					 NULL,
					 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma length fail\n"));
				goto error2;
			}

			/* Initialize URB descriptor */
			RTUSB_FILL_HTTX_BULK_URB(urb,
					 udev,
					 cap->CommandBulkOutAddr,
					 rom_patch_data,
					 sent_len + sizeof(*tx_info) + 4,
					 usb_upload_rom_patch_complete,
					 &load_rom_patch_done,
					 rom_patch_dma);

			ret = usb_submit_urb(urb, GFP_ATOMIC);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("submit urb fail\n"));
				goto error2;
			}

			DBGPRINT(RT_DEBUG_INFO, ("%s: submit urb, sent_len = %d, patch_ilm = %d, cur_len = %d\n", __FUNCTION__, sent_len, patch_len, cur_len));

			if (!wait_for_completion_timeout(&load_rom_patch_done, RTMPMsecsToJiffies(1000))) {
				usb_kill_urb(urb);
				ret = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR, ("upload fw timeout\n"));
				goto error2;
			}
			DBGPRINT(RT_DEBUG_OFF, ("."));

			mac_value = mt7612u_read32(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX);
			mac_value++;
			mt7612u_write32(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, mac_value);

			mdelay(5);
		} else {
			break;
		}

	}

	total_checksum = checksume16(fw_patch_image + PATCH_INFO_SIZE, patch_len);

	mdelay(5);
	DBGPRINT(RT_DEBUG_OFF, ("Send checksum req..\n"));
	mt7612u_mcu_usb_chk_crc(ad, patch_len);
	mdelay(20);

	if (total_checksum != mt7612u_mcu_usb_get_crc(ad)) {
		DBGPRINT(RT_DEBUG_OFF, ("checksum fail!, local(0x%x) <> fw(0x%x)\n", total_checksum, mt7612u_mcu_usb_get_crc(ad)));

		ret = NDIS_STATUS_FAILURE;
		goto error2;
	}

	ret = mt7612u_mcu_usb_enable_patch(ad);

	if (ret) {
		ret = NDIS_STATUS_FAILURE;
		goto error2;
	}

	ret = mt7612u_mcu_usb_reset_wmt(ad);

	mdelay(20);

	/* Check ROM_PATCH if ready */
	loop = 0;

	do {
		if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
			mac_value = mt7612u_read32(ad, CLOCK_CTL);
			if ((mac_value & 0x01) == 0x1)
				break;
		} else {
			mac_value = mt7612u_read32(ad, COM_REG0);
			if ((mac_value & 0x02) == 0x2)
				break;
		}

		mdelay(10);
		loop++;
	} while (loop <= 100);

	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
		DBGPRINT(RT_DEBUG_TRACE, ("%s: CLOCK_CTL(0x%x) = 0x%x\n", __FUNCTION__, CLOCK_CTL, mac_value));

		if ((mac_value & 0x01) != 0x1)
			ret = NDIS_STATUS_FAILURE;
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("%s: CLOCK_CTL(0x%x) = 0x%x\n", __FUNCTION__, COM_REG0, mac_value));

		if ((mac_value & 0x02) != 0x2)
			ret = NDIS_STATUS_FAILURE;
	}

error2:
	/* Free TransferBuffer */
	usb_free_coherent(udev, UPLOAD_PATCH_UNIT, rom_patch_data, rom_patch_dma);

error1:
	/* Free URB */
	usb_free_urb(urb);

error0:
	if (cap->rom_code_protect)
		mt7612u_write32(ad, SEMAPHORE_03, 0x1);

	return ret;
}

static void usb_uploadfw_complete(struct urb *urb)
{
	struct completion *load_fw_done = urb->context;

	complete(load_fw_done);
}

static int usb_load_ivb(struct rtmp_adapter *ad, u8 *fw_image)
{
	int Status = NDIS_STATUS_SUCCESS;
	struct rtmp_chip_cap *cap = &ad->chipCap;


	if (cap->load_iv) {
		Status = RTUSB_VendorRequest(ad,
				 DEVICE_VENDOR_REQUEST_OUT,
				 0x01,
				 0x12,
				 0x00,
				 fw_image + 32,
				 64);
	} else {
		Status = RTUSB_VendorRequest(ad,
				 DEVICE_VENDOR_REQUEST_OUT,
				 0x01,
				 0x12,
				 0x00,
				 NULL,
				 0x00);

	}

	if (Status)
	{
			DBGPRINT(RT_DEBUG_ERROR, ("Upload IVB Fail\n"));
			return Status;
	}

	return Status;
}

int mt7612u_mcu_usb_loadfw(struct rtmp_adapter *ad)
{
	PURB urb;
	struct usb_device *udev = ad->OS_Cookie->pUsb_Dev;
	ra_dma_addr_t fw_dma;
	u8 *fw_data;
	TXINFO_NMAC_CMD *tx_info;
	s32 sent_len;
	u32 cur_len = 0;
	u32 mac_value, loop = 0;
	u16 value;
	int ret = 0;
	struct rtmp_chip_cap *cap = &ad->chipCap;
	USB_DMA_CFG_STRUC cfg;
	u32 ilm_len = 0, dlm_len = 0;
	u16 fw_ver, build_ver;
	struct completion load_fw_done;
	const struct firmware *fw;
	u8 *fw_image;

	dev_info(&udev->dev, "loading firmware %s\n", cap->fw_name);

	ret = request_firmware(&fw, cap->fw_name, &udev->dev);
	if (ret) {
		dev_info(&udev->dev, "loading failed %s\n", cap->fw_name);
		return ret;
	}

	fw_image = (u8 *) fw->data;

	dev_info(&udev->dev, "firmware %s loaded\n", cap->fw_name);

	if (cap->ram_code_protect) {
loadfw_protect:
		mac_value = mt7612u_read32(ad, SEMAPHORE_00);
		loop++;

		if (((mac_value & 0x01) == 0x00) && (loop < GET_SEMAPHORE_RETRY_MAX)) {
			mdelay(1);
			goto loadfw_protect;
		}

		if (loop >= GET_SEMAPHORE_RETRY_MAX) {
			DBGPRINT(RT_DEBUG_ERROR,
				 ("%s: can not get the hw semaphore\n", __FUNCTION__));
			return NDIS_STATUS_FAILURE;
		}
	}

	/* Check MCU if ready */
	mac_value = mt7612u_read32(ad, COM_REG0);

	if (((mac_value & 0x01) == 0x01) && (cap->ram_code_protect)) {
		goto error0;
	}

	mt7612u_vendor_reset(ad);
	mdelay(5);

	/* Enable USB_DMA_CFG */
	cfg.word = mt7612u_usb_cfg_read_v3(ad);
	cfg.word |= 0x00c00020;
	mt7612u_usb_cfg_write_v3(ad, cfg.word);

	/* Get FW information */
	ilm_len = (*(fw_image + 3) << 24) | (*(fw_image + 2) << 16) |
		  (*(fw_image + 1) << 8) | (*fw_image);

	dlm_len = (*(fw_image + 7) << 24) | (*(fw_image + 6) << 16) |
		  (*(fw_image + 5) << 8) | (*(fw_image + 4));

	fw_ver = (*(fw_image + 11) << 8) | (*(fw_image + 10));

	build_ver = (*(fw_image + 9) << 8) | (*(fw_image + 8));

	DBGPRINT(RT_DEBUG_OFF, ("fw version:%d.%d.%02d ", (fw_ver & 0xf000) >> 8,
						(fw_ver & 0x0f00) >> 8, fw_ver & 0x00ff));
	DBGPRINT(RT_DEBUG_OFF, ("build:%x\n", build_ver));
	DBGPRINT(RT_DEBUG_OFF, ("build time:"));

	for (loop = 0; loop < 16; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(fw_image + 16 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	if (IS_MT76x2(ad)) {
		if (((strncmp(fw_image + 16, "20130811", 8) >= 0)) && (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))) {
			DBGPRINT(RT_DEBUG_OFF, ("fw for E3 IC\n"));

		} else if (((strncmp(fw_image + 16, "20130811", 8) < 0)) && (MT_REV_LT(ad, MT76x2, REV_MT76x2E3))){

			DBGPRINT(RT_DEBUG_OFF, ("fw for E2 IC\n"));
		} else {
			DBGPRINT(RT_DEBUG_OFF, ("fw do not match IC version\n"));
			mac_value = mt7612u_read32(ad, 0x0);
			DBGPRINT(RT_DEBUG_OFF, ("IC version(%x)\n", mac_value));
			ret = NDIS_STATUS_FAILURE;
			goto error0;
		}
	}

	DBGPRINT(RT_DEBUG_OFF, ("ilm length = %d(bytes)\n", ilm_len));
	DBGPRINT(RT_DEBUG_OFF, ("dlm length = %d(bytes)\n", dlm_len));

	/* Enable FCE to send in-band cmd */
	mt7612u_write32(ad, FCE_PSE_CTRL, 0x01);

	/* FCE tx_fs_base_ptr */
	mt7612u_write32(ad, TX_CPU_PORT_FROM_FCE_BASE_PTR, 0x400230);

	/* FCE tx_fs_max_cnt */
	mt7612u_write32(ad, TX_CPU_PORT_FROM_FCE_MAX_COUNT, 0x01);

	/* FCE pdma enable */
	mt7612u_write32(ad, FCE_PDMA_GLOBAL_CONF, 0x44);

	/* FCE skip_fs_en */
	mt7612u_write32(ad, FCE_SKIP_FS, 0x03);


	/* Allocate URB */
	urb = usb_alloc_urb(0, GFP_ATOMIC);

	if (!urb) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate URB\n"));
		ret = NDIS_STATUS_RESOURCES;
		goto error0;
	}

	/* Allocate TransferBuffer */
	fw_data = usb_alloc_coherent(udev, UPLOAD_FW_UNIT, GFP_ATOMIC, &fw_dma);

	if (!fw_data) {
		ret = NDIS_STATUS_RESOURCES;
		goto error1;
	}

	DBGPRINT(RT_DEBUG_OFF, ("loading fw"));

	init_completion(&load_fw_done);

	if (cap->load_iv)
		cur_len = 0x40;
	else
		cur_len = 0x00;

	/* Loading ILM */
	while (1) {
		s32 sent_len_max = UPLOAD_FW_UNIT - sizeof(*tx_info) - USB_END_PADDING;
		sent_len = (ilm_len - cur_len) >=  sent_len_max ? sent_len_max : (ilm_len - cur_len);

		if (sent_len > 0) {
			tx_info = (TXINFO_NMAC_CMD *)fw_data;
			tx_info->info_type = CMD_PACKET;
			tx_info->pkt_len = sent_len;
			tx_info->d_port = CPU_TX_PORT;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((u8 *)tx_info, TYPE_TXINFO);
#endif
			memmove(fw_data + sizeof(*tx_info), fw_image + FW_INFO_SIZE + cur_len, sent_len);

			/* four zero bytes for end padding */
			memset(fw_data + sizeof(*tx_info) + sent_len, 0, USB_END_PADDING);

			value = (cur_len + cap->ilm_offset) & 0xFFFF;

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad,
					 DEVICE_VENDOR_REQUEST_OUT,
					 0x42,
					 value,
					 0x230,
					 NULL,
					 0);


			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma descriptor fail\n"));
				goto error2;
			}

			value = (((cur_len + cap->ilm_offset) & 0xFFFF0000) >> 16);

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad,
					 DEVICE_VENDOR_REQUEST_OUT,
					 0x42,
					 value,
					 0x232,
					 NULL,
					 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma descriptor fail\n"));
				goto error2;
			}



			cur_len += sent_len;

			while ((sent_len % 4) != 0)
				sent_len++;

			value = ((sent_len << 16) & 0xFFFF);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad,
					 DEVICE_VENDOR_REQUEST_OUT,
					 0x42,
					 value,
					 0x234,
					 NULL,
					 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma length fail\n"));
				goto error2;
			}

			value = (((sent_len << 16) & 0xFFFF0000) >> 16);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad,
					 DEVICE_VENDOR_REQUEST_OUT,
					 0x42,
					 value,
					 0x236,
					 NULL,
					 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma length fail\n"));
				goto error2;
			}

			/* Initialize URB descriptor */
			RTUSB_FILL_HTTX_BULK_URB(urb,
					 udev,
					 cap->CommandBulkOutAddr,
					 fw_data,
					 sent_len + sizeof(*tx_info) + USB_END_PADDING,
					 usb_uploadfw_complete,
					 &load_fw_done,
					 fw_dma);

			ret = usb_submit_urb(urb, GFP_ATOMIC);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("submit urb fail\n"));
				goto error2;
			}

			DBGPRINT(RT_DEBUG_INFO, ("%s: submit urb, sent_len = %d, ilm_ilm = %d, cur_len = %d\n", __FUNCTION__, sent_len, ilm_len, cur_len));

			if (!wait_for_completion_timeout(&load_fw_done, RTMPMsecsToJiffies(UPLOAD_FW_TIMEOUT))) {
				usb_kill_urb(urb);
				ret = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR, ("upload fw timeout(%dms)\n", UPLOAD_FW_TIMEOUT));
				DBGPRINT(RT_DEBUG_ERROR, ("%s: submit urb, sent_len = %d, ilm_ilm = %d, cur_len = %d\n", __FUNCTION__, sent_len, ilm_len, cur_len));

				goto error2;
			}
			DBGPRINT(RT_DEBUG_OFF, ("."));

			mac_value = mt7612u_read32(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX);
			mac_value++;
			mt7612u_write32(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, mac_value);

			mdelay(5);
		} else {
			break;
		}

	}

	/* Re-Initialize completion */
	init_completion(&load_fw_done);

	cur_len = 0x00;

	/* Loading DLM */
	while (1) {
		s32 sent_len_max = UPLOAD_FW_UNIT - sizeof(*tx_info) - USB_END_PADDING;
		sent_len = (dlm_len - cur_len) >= sent_len_max ? sent_len_max : (dlm_len - cur_len);

		if (sent_len > 0) {
			tx_info = (TXINFO_NMAC_CMD *)fw_data;
			tx_info->info_type = CMD_PACKET;
			tx_info->pkt_len = sent_len;
			tx_info->d_port = CPU_TX_PORT;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((u8 *)tx_info, TYPE_TXINFO);
#endif
			memmove(fw_data + sizeof(*tx_info), fw_image + FW_INFO_SIZE + ilm_len + cur_len, sent_len);

			memset(fw_data + sizeof(*tx_info) + sent_len, 0, USB_END_PADDING);

			if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))
				value = ((cur_len + (cap->dlm_offset + 0x800)) & 0xFFFF);
			else
				value = ((cur_len + (cap->dlm_offset)) & 0xFFFF);

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad,
					 DEVICE_VENDOR_REQUEST_OUT,
					 0x42,
					 value,
					 0x230,
					 NULL,
					 0);


			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma descriptor fail\n"));
				goto error2;
			}

			if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))
				value = (((cur_len + (cap->dlm_offset + 0x800)) & 0xFFFF0000) >> 16);
			else
				value = (((cur_len + (cap->dlm_offset)) & 0xFFFF0000) >> 16);

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad,
					  DEVICE_VENDOR_REQUEST_OUT,
					  0x42,
					  value,
					  0x232,
					  NULL,
					  0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma descriptor fail\n"));
				goto error2;
			}

			cur_len += sent_len;

			while ((sent_len % 4) != 0)
				sent_len++;

			value = ((sent_len << 16) & 0xFFFF);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad,
					  DEVICE_VENDOR_REQUEST_OUT,
					  0x42,
					  value,
					  0x234,
					  NULL,
					  0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma length fail\n"));
				goto error2;
			}

			value = (((sent_len << 16) & 0xFFFF0000) >> 16);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad,
					  DEVICE_VENDOR_REQUEST_OUT,
					  0x42,
					  value,
					  0x236,
					  NULL,
					  0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma length fail\n"));
				goto error2;
			}

			/* Initialize URB descriptor */
			RTUSB_FILL_HTTX_BULK_URB(urb,
					 udev,
					 cap->CommandBulkOutAddr,
					 fw_data,
					 sent_len + sizeof(*tx_info) + USB_END_PADDING,
					 usb_uploadfw_complete,
					 &load_fw_done,
					 fw_dma);

			ret = usb_submit_urb(urb, GFP_ATOMIC);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("submit urb fail\n"));
				goto error2;
			}

			DBGPRINT(RT_DEBUG_INFO, ("%s: submit urb, sent_len = %d, dlm_len = %d, cur_len = %d\n", __FUNCTION__, sent_len, dlm_len, cur_len));

			if (!wait_for_completion_timeout(&load_fw_done, RTMPMsecsToJiffies(UPLOAD_FW_TIMEOUT))) {
				usb_kill_urb(urb);
				ret = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR, ("upload fw timeout(%dms)\n", UPLOAD_FW_TIMEOUT));
				DBGPRINT(RT_DEBUG_INFO, ("%s: submit urb, sent_len = %d, dlm_len = %d, cur_len = %d\n", __FUNCTION__, sent_len, dlm_len, cur_len));

				goto error2;
			}
			DBGPRINT(RT_DEBUG_OFF, ("."));

			mac_value = mt7612u_read32(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX);
			mac_value++;
			mt7612u_write32(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, mac_value);
			mdelay(5);
		} else {
			break;
		}

	}

	/* Upload new 64 bytes interrupt vector or reset andes */
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
	usb_load_ivb(ad, fw_image);

	/* Check MCU if ready */
	loop = 0;
	do {
		mac_value = mt7612u_read32(ad, COM_REG0);
		if ((mac_value & 0x01) == 0x01)
			break;
		mdelay(10);
		loop++;
	} while (loop <= 100);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: COM_REG0(0x%x) = 0x%x\n", __FUNCTION__, COM_REG0, mac_value));

	mac_value = mt7612u_read32(ad, COM_REG0);
	mac_value |= (1 << 1);
	mt7612u_write32(ad, COM_REG0, mac_value);

	if ((mac_value & 0x01) != 0x01)
		ret = NDIS_STATUS_FAILURE;

error2:
	/* Free TransferBuffer */
	usb_free_coherent(udev, UPLOAD_FW_UNIT, fw_data, fw_dma);

error1:
	/* Free URB */
	usb_free_urb(urb);

error0:
	if (cap->ram_code_protect)
		mt7612u_write32(ad, SEMAPHORE_00, 0x1);

	/* Enable FCE to send in-band cmd */
	mt7612u_write32(ad, FCE_PSE_CTRL, 0x01);

	return ret;
}
#endif

struct cmd_msg *mt7612u_mcu_alloc_cmd_msg(struct rtmp_adapter *ad, unsigned int length)
{
	struct cmd_msg *msg = NULL;
	struct rtmp_chip_cap *cap = &ad->chipCap;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct sk_buff *net_pkt = NULL;
#ifdef RTMP_USB_SUPPORT
	PURB urb = NULL;
#endif

	net_pkt = dev_alloc_skb(cap->cmd_header_len + length + cap->cmd_padding_len);

	if (!net_pkt) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate net_pkt\n"));
		goto error0;
	}

	skb_reserve(net_pkt, cap->cmd_header_len);

	msg = kmalloc(sizeof(*msg), GFP_ATOMIC);

	if (!msg) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate cmd msg\n"));
		goto error1;
	}

	CMD_MSG_CB(net_pkt)->msg = msg;

	memset(msg, 0x00, sizeof(*msg));

#ifdef RTMP_USB_SUPPORT
	urb = usb_alloc_urb(0, GFP_ATOMIC);

	if (!urb) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate urb\n"));
		goto error2;
	}

	msg->urb = urb;
#endif

	msg->priv = (void *)ad;
	msg->net_pkt = net_pkt;

	return msg;

#ifdef RTMP_USB_SUPPORT
error2:
#endif
	kfree(msg);
error1:
	dev_kfree_skb_any(net_pkt);
error0:
	return NULL;
}

static void mt7612u_mcu_init_cmd_msg(struct cmd_msg *msg, enum mcu_cmd_type type,
				     bool need_wait, u16 timeout,
				     bool need_retransmit, bool need_rsp)
{
	u16 rsp_payload_len = 0;
	char *rsp_payload = NULL;
	MSG_RSP_HANDLER rsp_handler = NULL;

	msg->type = type;
#ifdef RTMP_USB_SUPPORT
	msg->need_wait= need_wait;
#else
	msg->need_wait= false;
#endif
	msg->timeout = timeout;

	if (need_wait) {

#ifdef RTMP_USB_SUPPORT
		init_completion(&msg->ack_done);
#endif
	}

#ifdef RTMP_USB_SUPPORT
	msg->need_retransmit = need_retransmit;
#else
	msg->need_retransmit = 0;
#endif

#ifdef RTMP_USB_SUPPORT
	if (need_retransmit)
		msg->retransmit_times = CMD_MSG_RETRANSMIT_TIMES;
#else
		msg->retransmit_times = 0;
#endif

#ifdef RTMP_USB_SUPPORT
	msg->need_rsp = need_rsp;
#else
	msg->need_rsp = false;
#endif
	msg->rsp_payload_len = rsp_payload_len;
	msg->rsp_payload = rsp_payload;
	msg->rsp_handler = rsp_handler;
}

void mt7612u_mcu_append_cmd_msg(struct cmd_msg *msg, char *data, unsigned int len)
{
	struct sk_buff *net_pkt = msg->net_pkt;

	if (data)
		memcpy(skb_put(net_pkt, len), data, len);
}

void mt7612u_mcu_free_cmd_msg(struct cmd_msg *msg)
{
	struct sk_buff *net_pkt = msg->net_pkt;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)(msg->priv);
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

#ifdef RTMP_USB_SUPPORT
	usb_free_urb(msg->urb);
#endif

	kfree(msg);

	dev_kfree_skb_any(net_pkt);
}

static inline void mt7612u_mcu_inc_error_count(struct MCU_CTRL *ctl, enum cmd_msg_error_type type)
{
	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		switch (type) {
		case error_tx_kickout_fail:
			ctl->tx_kickout_fail_count++;
		break;
		case error_tx_timeout_fail:
			ctl->tx_timeout_fail_count++;
		break;
		case error_rx_receive_fail:
			ctl->rx_receive_fail_count++;
		break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s:unknown cmd_msg_error_type(%d)\n", __FUNCTION__, type));
		}
	}
}

static spinlock_t *mt7612u_mcu_get_spin_lock(struct MCU_CTRL *ctl, DL_LIST *list)
{
	spinlock_t *lock = NULL;

	if (list == &ctl->txq)
		lock = &ctl->txq_lock;
	else if (list == &ctl->rxq)
		lock = &ctl->rxq_lock;
	else if (list == &ctl->ackq)
		lock = &ctl->ackq_lock;
	else if (list == &ctl->kickq)
		lock = &ctl->kickq_lock;
	else if (list == &ctl->tx_doneq)
		lock = &ctl->tx_doneq_lock;
	else if (list == &ctl->rx_doneq)
		lock = &ctl->rx_doneq_lock;
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal list\n", __FUNCTION__));

	return lock;
}

static inline u8 mt7612u_mcu_get_cmd_msg_seq(struct rtmp_adapter *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg;
	unsigned long flags;

	spin_lock_irqsave(&ctl->ackq_lock, flags);
get_seq:
	ctl->cmd_seq >= 0xf ? ctl->cmd_seq = 1 : ctl->cmd_seq++;
	DlListForEach(msg, &ctl->ackq, struct cmd_msg, list) {
		if (msg->seq == ctl->cmd_seq) {
			DBGPRINT(RT_DEBUG_ERROR, ("command(seq: %d) is still running\n", ctl->cmd_seq));
			goto get_seq;
		}
	}
	spin_unlock_irqrestore(&ctl->ackq_lock, flags);

	return ctl->cmd_seq;
}

static void _mt7612u_mcu_queue_tail_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
										enum cmd_msg_state state)
{
	msg->state = state;
	DlListAddTail(list, &msg->list);
}

static void mt7612u_mcu_queue_tail_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
										enum cmd_msg_state state)
{
	unsigned long flags;
	spinlock_t *lock;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	_mt7612u_mcu_queue_tail_cmd_msg(list, msg, state);
	spin_unlock_irqrestore(lock, flags);
}

static void _mt7612u_mcu_queue_head_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
										enum cmd_msg_state state)
{
	msg->state = state;
	DlListAdd(list, &msg->list);
}

static void mt7612u_mcu_queue_head_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
										enum cmd_msg_state state)
{
	unsigned long flags;
	spinlock_t *lock;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	_mt7612u_mcu_queue_head_cmd_msg(list, msg, state);
	spin_unlock_irqrestore(lock, flags);
}

static u32 mt7612u_mcu_queue_len(struct MCU_CTRL *ctl, DL_LIST *list)
{
	u32 qlen;
	unsigned long flags;
	spinlock_t *lock;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	qlen = DlListLen(list);
	spin_unlock_irqrestore(lock, flags);

	return qlen;
}

static int mt7612u_mcu_queue_empty(struct MCU_CTRL *ctl, DL_LIST *list)
{
	unsigned long flags;
	int is_empty;
	spinlock_t *lock;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	is_empty = DlListEmpty(list);
	spin_unlock_irqrestore(lock, flags);

	return is_empty;
}

static void mt7612u_mcu_queue_init(struct MCU_CTRL *ctl, DL_LIST *list)
{

	unsigned long flags;
	spinlock_t *lock;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	DlListInit(list);
	spin_unlock_irqrestore(lock, flags);
}

static void _mt7612u_mcu_unlink_cmd_msg(struct cmd_msg *msg, DL_LIST *list)
{
	if (!msg)
		return;

	DlListDel(&msg->list);
}

static void mt7612u_mcu_unlink_cmd_msg(struct cmd_msg *msg, DL_LIST *list)
{
	unsigned long flags;
	spinlock_t *lock;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	_mt7612u_mcu_unlink_cmd_msg(msg, list);
	spin_unlock_irqrestore(lock, flags);
}

static struct cmd_msg *_mt7612u_mcu_dequeue_cmd_msg(DL_LIST *list)
{
	struct cmd_msg *msg;

	msg = DlListFirst(list, struct cmd_msg, list);

	_mt7612u_mcu_unlink_cmd_msg(msg, list);

	return msg;
}

static struct cmd_msg *mt7612u_mcu_dequeue_cmd_msg(struct MCU_CTRL *ctl, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg;
	spinlock_t *lock;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	msg = _mt7612u_mcu_dequeue_cmd_msg(list);
	spin_unlock_irqrestore(lock, flags);

	return msg;
}

void mt7612u_mcu_rx_process_cmd_msg(struct rtmp_adapter *ad, struct cmd_msg *rx_msg)
{
	struct sk_buff *net_pkt = rx_msg->net_pkt;
	struct cmd_msg *msg, *msg_tmp;
	RXFCE_INFO_CMD *rx_info = (RXFCE_INFO_CMD *)net_pkt->data;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	unsigned long flags;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((u8 *)rx_info, TYPE_RXINFO);
#endif

	DBGPRINT(RT_DEBUG_INFO, ("(andex_rx_cmd)info_type=%d,evt_type=%d,d_port=%d,"
                                "qsel=%d,pcie_intr=%d,cmd_seq=%d,"
                                "self_gen=%d,pkt_len=%d\n",
                                rx_info->info_type, rx_info->evt_type,rx_info->d_port,
                                rx_info->qsel, rx_info->pcie_intr, rx_info->cmd_seq,
                                rx_info->self_gen, rx_info->pkt_len));

	if ((rx_info->info_type != CMD_PACKET)) {
		DBGPRINT(RT_DEBUG_ERROR, ("packet is not command response/self event\n"));
		return;
	}

	if (rx_info->self_gen) {
		/* if have callback function */
		RTEnqueueInternalCmd(ad, CMDTHREAD_RESPONSE_EVENT_CALLBACK,
				     net_pkt->data + sizeof(*rx_info), rx_info->pkt_len);
	} else {
#ifdef RTMP_USB_SUPPORT
		spin_lock_irq(&ctl->ackq_lock);
#endif

		DlListForEachSafe(msg, msg_tmp, &ctl->ackq, struct cmd_msg, list) {
			if (msg->seq == rx_info->cmd_seq)
			{
				_mt7612u_mcu_unlink_cmd_msg(msg, &ctl->ackq);
#ifdef RTMP_USB_SUPPORT
				spin_unlock_irq(&ctl->ackq_lock);
#endif


				if ((msg->rsp_payload_len == rx_info->pkt_len) && (msg->rsp_payload_len != 0))
				{
					msg->rsp_handler(msg, net_pkt->data + sizeof(*rx_info), rx_info->pkt_len);
				}
				else if ((msg->rsp_payload_len == 0) && (rx_info->pkt_len == 8))
				{
					DBGPRINT(RT_DEBUG_INFO, ("command response(ack) success\n"));
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("expect response len(%d), command response len(%d) invalid\n", msg->rsp_payload_len, rx_info->pkt_len));
					msg->rsp_payload_len = rx_info->pkt_len;
				}

				if (msg->need_wait) {

#ifdef RTMP_USB_SUPPORT
					complete(&msg->ack_done);
#endif
				} else {
					mt7612u_mcu_free_cmd_msg(msg);
				}
#ifdef RTMP_USB_SUPPORT
				spin_lock_irq(&ctl->ackq_lock);
#endif

				break;
			}
		}

#ifdef RTMP_USB_SUPPORT
		spin_unlock_irq(&ctl->ackq_lock);
#endif

	}
}


#ifdef RTMP_USB_SUPPORT
static void usb_rx_cmd_msg_complete(PURB urb)
{
	struct sk_buff *net_pkt = urb->context;
	struct cmd_msg *msg = CMD_MSG_CB(net_pkt)->msg;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)msg->priv;
	struct os_cookie *pObj = ad->OS_Cookie;
	struct rtmp_chip_cap *pChipCap = &ad->chipCap;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	enum cmd_msg_state state;
	unsigned long flags;
	int ret = 0;

	mt7612u_mcu_unlink_cmd_msg(msg, &ctl->rxq);

	skb_put(net_pkt, urb->actual_length);

	if (urb->status == 0) {
		state = rx_done;
	} else {
		state = rx_receive_fail;
		mt7612u_mcu_inc_error_count(ctl, error_rx_receive_fail);
		DBGPRINT(RT_DEBUG_ERROR, ("receive cmd msg fail(%d)\n", urb->status));
	}

	spin_lock_irqsave(&ctl->rx_doneq_lock, flags);
	_mt7612u_mcu_queue_tail_cmd_msg(&ctl->rx_doneq, msg, state);
	spin_unlock_irqrestore(&ctl->rx_doneq_lock, flags);

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		msg = mt7612u_mcu_alloc_cmd_msg(ad, 512);

		if (!msg) {
			return;
		}

		net_pkt = msg->net_pkt;

		usb_fill_bulk_urb(msg->urb, pObj->pUsb_Dev,
				  usb_rcvbulkpipe(pObj->pUsb_Dev, pChipCap->CommandRspBulkInAddr),
				  net_pkt->data, 512, usb_rx_cmd_msg_complete, net_pkt);

		mt7612u_mcu_queue_tail_cmd_msg(&ctl->rxq, msg, rx_start);

		ret = usb_submit_urb(msg->urb, GFP_ATOMIC);

		if (ret) {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->rxq);
			mt7612u_mcu_inc_error_count(ctl, error_rx_receive_fail);
			DBGPRINT(RT_DEBUG_ERROR, ("%s:submit urb fail(%d)\n", __FUNCTION__, ret));
			mt7612u_mcu_queue_tail_cmd_msg(&ctl->rx_doneq, msg, rx_receive_fail);
		}

	}

	mt7612u_mcu_bh_schedule(ad);
}

int usb_rx_cmd_msg_submit(struct rtmp_adapter *ad)
{
	struct rtmp_chip_cap *pChipCap = &ad->chipCap;
	struct os_cookie *pObj = ad->OS_Cookie;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg = NULL;
	struct sk_buff *net_pkt = NULL;
	int ret = 0;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return ret;

	msg =  mt7612u_mcu_alloc_cmd_msg(ad, 512);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		return ret;
	}

	net_pkt = msg->net_pkt;

	usb_fill_bulk_urb(msg->urb, pObj->pUsb_Dev,
			  usb_rcvbulkpipe(pObj->pUsb_Dev, pChipCap->CommandRspBulkInAddr),
			  net_pkt->data, 512, usb_rx_cmd_msg_complete, net_pkt);

	mt7612u_mcu_queue_tail_cmd_msg(&ctl->rxq, msg, rx_start);

	ret = usb_submit_urb(msg->urb, GFP_ATOMIC);

	if (ret) {
		mt7612u_mcu_unlink_cmd_msg(msg, &ctl->rxq);
		mt7612u_mcu_inc_error_count(ctl, error_rx_receive_fail);
		DBGPRINT(RT_DEBUG_ERROR, ("%s:submit urb fail(%d)\n", __FUNCTION__, ret));
		mt7612u_mcu_queue_tail_cmd_msg(&ctl->rx_doneq, msg, rx_receive_fail);
	}

	return ret;
}

int usb_rx_cmd_msgs_receive(struct rtmp_adapter *ad)
{
	int ret = 0;
	int i;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	for (i = 0; (i < 1) && (mt7612u_mcu_queue_len(ctl, &ctl->rxq) < 1); i++) {
		ret = usb_rx_cmd_msg_submit(ad);
		if (ret)
			break;
	}

	return ret;
}
#endif

void mt7612u_mcu_cmd_msg_bh(unsigned long param)
{
	struct rtmp_adapter *ad = (struct rtmp_adapter *)param;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg = NULL;

	while ((msg = mt7612u_mcu_dequeue_cmd_msg(ctl, &ctl->rx_doneq))) {
		switch (msg->state) {
		case rx_done:
			mt7612u_mcu_rx_process_cmd_msg(ad, msg);
			mt7612u_mcu_free_cmd_msg(msg);
			continue;
		case rx_receive_fail:
			mt7612u_mcu_free_cmd_msg(msg);
			continue;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("unknow msg state(%d)\n", msg->state));
		}
	}

	while ((msg = mt7612u_mcu_dequeue_cmd_msg(ctl, &ctl->tx_doneq))) {
		switch (msg->state) {
		case tx_done:
		case tx_kickout_fail:
		case tx_timeout_fail:
			mt7612u_mcu_free_cmd_msg(msg);
			continue;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("unknow msg state(%d)\n", msg->state));
		}
	}

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		mt7612u_mcu_bh_schedule(ad);
#ifdef RTMP_USB_SUPPORT
		usb_rx_cmd_msgs_receive(ad);
#endif
	}
}

void mt7612u_mcu_bh_schedule(struct rtmp_adapter *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	if (((mt7612u_mcu_queue_len(ctl, &ctl->rx_doneq) > 0)
							|| (mt7612u_mcu_queue_len(ctl, &ctl->tx_doneq) > 0))
							&& OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
#ifndef WORKQUEUE_BH
		RTMP_NET_TASK_DATA_ASSIGN(&ctl->cmd_msg_task, (unsigned long)(ad));
		RTMP_OS_TASKLET_SCHE(&ctl->cmd_msg_task);
#else
		tasklet_hi_schedule(&ctl->cmd_msg_task);
#endif
	}
}


#ifdef RTMP_USB_SUPPORT
#endif

#ifdef RTMP_USB_SUPPORT
static void usb_kick_out_cmd_msg_complete(PURB urb)
{
	struct sk_buff *net_pkt = urb->context;
	struct cmd_msg *msg = CMD_MSG_CB(net_pkt)->msg;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	if (urb->status == 0) {
		if (!msg->need_rsp) {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->kickq);
			mt7612u_mcu_queue_tail_cmd_msg(&ctl->tx_doneq, msg, tx_done);
		} else {
			msg->state = wait_ack;
		}
	} else {
		if (!msg->need_rsp) {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->kickq);
			mt7612u_mcu_queue_tail_cmd_msg(&ctl->tx_doneq, msg, tx_kickout_fail);
			mt7612u_mcu_inc_error_count(ctl, error_tx_kickout_fail);
		} else {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->ackq);
			msg->state = tx_kickout_fail;
			mt7612u_mcu_inc_error_count(ctl, error_tx_kickout_fail);
			complete(&msg->ack_done);
		}

		DBGPRINT(RT_DEBUG_ERROR, ("kick out cmd msg fail(%d)\n", urb->status));
	}

	mt7612u_mcu_bh_schedule(ad);
}

int usb_kick_out_cmd_msg(struct rtmp_adapter *ad, struct cmd_msg *msg)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct os_cookie *pObj = ad->OS_Cookie;
	int ret = 0;
	struct sk_buff *net_pkt = msg->net_pkt;
	struct rtmp_chip_cap *pChipCap = &ad->chipCap;

	if (msg->state != tx_retransmit) {
		/* append four zero bytes padding when usb aggregate enable */
		memset(skb_put(net_pkt, USB_END_PADDING), 0x00, USB_END_PADDING);
	}

	usb_fill_bulk_urb(msg->urb, pObj->pUsb_Dev,
			  usb_sndbulkpipe(pObj->pUsb_Dev, pChipCap->CommandBulkOutAddr),
			  net_pkt->data, net_pkt->len, usb_kick_out_cmd_msg_complete, net_pkt);

	if (msg->need_rsp)
		mt7612u_mcu_queue_tail_cmd_msg(&ctl->ackq, msg, wait_cmd_out_and_ack);
	else
		mt7612u_mcu_queue_tail_cmd_msg(&ctl->kickq, msg, wait_cmd_out);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return -1;

	ret = usb_submit_urb(msg->urb, GFP_ATOMIC);

	if (ret) {
		if (!msg->need_rsp) {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->kickq);
			mt7612u_mcu_queue_tail_cmd_msg(&ctl->tx_doneq, msg, tx_kickout_fail);
			mt7612u_mcu_inc_error_count(ctl, error_tx_kickout_fail);
		} else {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->ackq);
			msg->state = tx_kickout_fail;
			mt7612u_mcu_inc_error_count(ctl, error_tx_kickout_fail);
			complete(&msg->ack_done);
		}

		DBGPRINT(RT_DEBUG_ERROR, ("%s:submit urb fail(%d)\n", __FUNCTION__, ret));
	}

	return ret;
}

void mt7612u_mcu_usb_unlink_urb(struct rtmp_adapter *ad, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg, *msg_tmp;
	spinlock_t *lock;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	DlListForEachSafe(msg, msg_tmp, list, struct cmd_msg, list) {
		spin_unlock_irqrestore(lock, flags);
		if ((msg->state == wait_cmd_out_and_ack) || (msg->state == wait_cmd_out) ||
						(msg->state == tx_start) || (msg->state == rx_start) ||
						(msg->state == tx_retransmit))
			usb_kill_urb(msg->urb);
		spin_lock_irqsave(lock, flags);
	}
	spin_unlock_irqrestore(lock, flags);
}

#endif

void mt7612u_mcu_cleanup_cmd_msg(struct rtmp_adapter *ad, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg, *msg_tmp;
	spinlock_t *lock;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	DlListForEachSafe(msg, msg_tmp, list, struct cmd_msg, list) {
		_mt7612u_mcu_unlink_cmd_msg(msg, list);
		mt7612u_mcu_free_cmd_msg(msg);
	}
	DlListInit(list);
	spin_unlock_irqrestore(lock, flags);
}


#ifdef RTMP_USB_SUPPORT
static void mt7612u_mcu_ctrl_usb_init(struct rtmp_adapter *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = 0;

	ret = down_interruptible(&(ad->mcu_atomic));
	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	ctl->cmd_seq = 0;
	RTMP_OS_TASKLET_INIT(ad, &ctl->cmd_msg_task, mt7612u_mcu_cmd_msg_bh, (unsigned long)ad);
	spin_lock_init(&ctl->txq_lock);
	mt7612u_mcu_queue_init(ctl, &ctl->txq);
	spin_lock_init(&ctl->rxq_lock);
	mt7612u_mcu_queue_init(ctl, &ctl->rxq);
	spin_lock_init(&ctl->ackq_lock);
	mt7612u_mcu_queue_init(ctl, &ctl->ackq);
	spin_lock_init(&ctl->kickq_lock);
	mt7612u_mcu_queue_init(ctl, &ctl->kickq);
	spin_lock_init(&ctl->tx_doneq_lock);
	mt7612u_mcu_queue_init(ctl, &ctl->tx_doneq);
	spin_lock_init(&ctl->rx_doneq_lock);
	mt7612u_mcu_queue_init(ctl, &ctl->rx_doneq);
	ctl->tx_kickout_fail_count = 0;
	ctl->tx_timeout_fail_count = 0;
	ctl->rx_receive_fail_count = 0;
	ctl->ad = ad;
	OS_SET_BIT(MCU_INIT, &ctl->flags);
	usb_rx_cmd_msgs_receive(ad);
	up(&(ad->mcu_atomic));
}
#endif

void mt7612u_mcu_ctrl_init(struct rtmp_adapter *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {

#ifdef RTMP_USB_SUPPORT
		mt7612u_mcu_ctrl_usb_init(ad);
#endif
	}

	ctl->power_on = false;
	ctl->dpd_on = false;
}

#ifdef RTMP_USB_SUPPORT
static void mt7612u_mcu_ctrl_usb_exit(struct rtmp_adapter *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = 0;

	ret = down_interruptible(&(ad->mcu_atomic));
	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	OS_CLEAR_BIT(MCU_INIT, &ctl->flags);
	mt7612u_mcu_usb_unlink_urb(ad, &ctl->txq);
	mt7612u_mcu_usb_unlink_urb(ad, &ctl->kickq);
	mt7612u_mcu_usb_unlink_urb(ad, &ctl->ackq);
	mt7612u_mcu_usb_unlink_urb(ad, &ctl->rxq);
	RTMP_OS_TASKLET_KILL(&ctl->cmd_msg_task);
	mt7612u_mcu_cleanup_cmd_msg(ad, &ctl->txq);
	mt7612u_mcu_cleanup_cmd_msg(ad, &ctl->ackq);
	mt7612u_mcu_cleanup_cmd_msg(ad, &ctl->rxq);
	mt7612u_mcu_cleanup_cmd_msg(ad, &ctl->kickq);
	mt7612u_mcu_cleanup_cmd_msg(ad, &ctl->tx_doneq);
	mt7612u_mcu_cleanup_cmd_msg(ad, &ctl->rx_doneq);
	DBGPRINT(RT_DEBUG_OFF, ("tx_kickout_fail_count = %ld\n", ctl->tx_kickout_fail_count));
	DBGPRINT(RT_DEBUG_OFF, ("tx_timeout_fail_count = %ld\n", ctl->tx_timeout_fail_count));
	DBGPRINT(RT_DEBUG_OFF, ("rx_receive_fail_count = %ld\n", ctl->rx_receive_fail_count));
	up(&(ad->mcu_atomic));
}
#endif


void mt7612u_mcu_ctrl_exit(struct rtmp_adapter *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {

#ifdef RTMP_USB_SUPPORT
		mt7612u_mcu_ctrl_usb_exit(ad);
#endif
	}

	ctl->power_on = false;
	ctl->dpd_on = false;
}

static int mt7612u_mcu_dequeue_and_kick_out_cmd_msgs(struct rtmp_adapter *ad)
{
	struct cmd_msg *msg = NULL;
	struct sk_buff *net_pkt = NULL;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = NDIS_STATUS_SUCCESS;
	TXINFO_NMAC_CMD *tx_info;

	while ((msg = mt7612u_mcu_dequeue_cmd_msg(ctl, &ctl->txq)) != NULL) {
		if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD)
				|| RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)
				|| RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
			if (!msg->need_rsp)
				mt7612u_mcu_free_cmd_msg(msg);
			continue;
		}

		if (mt7612u_mcu_queue_len(ctl, &ctl->ackq) > 0) {
			mt7612u_mcu_queue_head_cmd_msg(&ctl->txq, msg, msg->state);
			ret = NDIS_STATUS_FAILURE;
			continue;
		}

		net_pkt = msg->net_pkt;

		if (msg->state != tx_retransmit) {
			if (msg->need_rsp)
				msg->seq = mt7612u_mcu_get_cmd_msg_seq(ad);
			else
				msg->seq = 0;

			tx_info = (TXINFO_NMAC_CMD *)skb_push(net_pkt, sizeof(*tx_info));
			tx_info->info_type = CMD_PACKET;
			tx_info->d_port = CPU_TX_PORT;
			tx_info->cmd_type = msg->type;
			tx_info->cmd_seq = msg->seq;
			tx_info->pkt_len = net_pkt->len - sizeof(*tx_info);

#ifdef RT_BIG_ENDIAN
			*(uint32_t *)tx_info = le2cpu32(*(uint32_t *)tx_info);
			//RTMPDescriptorEndianChange((u8 *)tx_info, TYPE_TXINFO);
#endif
		}


#ifdef RTMP_USB_SUPPORT
		ret = usb_kick_out_cmd_msg(ad, msg);
#endif


		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, ("kick out msg fail\n"));
			break;
		}
	}

	mt7612u_mcu_bh_schedule(ad);

	return ret;
}

static int mt7612u_mcu_wait_for_complete_timeout(struct cmd_msg *msg, long timeout)
{
	int ret = 0;
#ifdef RTMP_USB_SUPPORT
	long expire;
	expire = timeout ? RTMPMsecsToJiffies(timeout) : RTMPMsecsToJiffies(CMD_MSG_TIMEOUT);
#endif


#ifdef RTMP_USB_SUPPORT
	ret = wait_for_completion_timeout(&msg->ack_done, expire);
#endif

	return ret;
}

int mt7612u_mcu_send_cmd_msg(struct rtmp_adapter *ad, struct cmd_msg *msg)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = 0;
	bool need_wait = msg->need_wait;


#ifdef RTMP_USB_SUPPORT
	ret = down_interruptible(&(ad->mcu_atomic));
#endif

	if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD)
				|| RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)
				|| RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
		mt7612u_mcu_free_cmd_msg(msg);

#ifdef RTMP_USB_SUPPORT
		up(&(ad->mcu_atomic));
#endif
		return NDIS_STATUS_FAILURE;
	}

	mt7612u_mcu_queue_tail_cmd_msg(&ctl->txq, msg, tx_start);

retransmit:
	mt7612u_mcu_dequeue_and_kick_out_cmd_msgs(ad);

	/* Wait for response */
	if (need_wait) {
		enum cmd_msg_state state;
		if (!mt7612u_mcu_wait_for_complete_timeout(msg, msg->timeout)) {
			ret = NDIS_STATUS_FAILURE;
			DBGPRINT(RT_DEBUG_ERROR, ("command (%d) timeout(%dms)\n", msg->type, CMD_MSG_TIMEOUT));
			DBGPRINT(RT_DEBUG_ERROR, ("txq qlen = %d\n", mt7612u_mcu_queue_len(ctl, &ctl->txq)));
			DBGPRINT(RT_DEBUG_ERROR, ("rxq qlen = %d\n", mt7612u_mcu_queue_len(ctl, &ctl->rxq)));
			DBGPRINT(RT_DEBUG_ERROR, ("kickq qlen = %d\n", mt7612u_mcu_queue_len(ctl, &ctl->kickq)));
			DBGPRINT(RT_DEBUG_ERROR, ("ackq qlen = %d\n", mt7612u_mcu_queue_len(ctl, &ctl->ackq)));
			DBGPRINT(RT_DEBUG_ERROR, ("tx_doneq.qlen = %d\n", mt7612u_mcu_queue_len(ctl, &ctl->tx_doneq)));
			DBGPRINT(RT_DEBUG_ERROR, ("rx_done qlen = %d\n", mt7612u_mcu_queue_len(ctl, &ctl->rx_doneq)));
			if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
				if (msg->state == wait_cmd_out_and_ack) {
#ifdef RTMP_USB_SUPPORT
					usb_kill_urb(msg->urb);
#endif
				} else if (msg->state == wait_ack) {
					mt7612u_mcu_unlink_cmd_msg(msg, &ctl->ackq);
				}
			}

			mt7612u_mcu_inc_error_count(ctl, error_tx_timeout_fail);
			state = tx_timeout_fail;
			if (msg->retransmit_times > 0)
				msg->retransmit_times--;
			DBGPRINT(RT_DEBUG_ERROR, ("msg->retransmit_times = %d\n", msg->retransmit_times));
		} else {
			if (msg->state == tx_kickout_fail) {
				state = tx_kickout_fail;
				msg->retransmit_times--;
			} else {
				state = tx_done;
				msg->retransmit_times = 0;
			}
		}

		if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
			if (msg->need_retransmit && (msg->retransmit_times > 0)) {

#ifdef RTMP_USB_SUPPORT
				init_completion(&msg->ack_done);
#endif
				state = tx_retransmit;
				mt7612u_mcu_queue_head_cmd_msg(&ctl->txq, msg, state);
				goto retransmit;
			} else {
				mt7612u_mcu_queue_tail_cmd_msg(&ctl->tx_doneq, msg, state);
			}
		} else {
			mt7612u_mcu_free_cmd_msg(msg);
		}
	}

#ifdef RTMP_USB_SUPPORT
	up(&(ad->mcu_atomic));
#endif


	return ret;
}

static void mt7612u_mcu_pwr_event_handler(struct rtmp_adapter *ad, char *payload, u16 payload_len)
{


}


static void mt7612u_mcu_wow_event_handler(struct rtmp_adapter *ad, char *payload, u16 payload_len)
{


}

static void mt7612u_mcu_carrier_detect_event_handler(struct rtmp_adapter *ad, char *payload, u16 payload_len)
{



}

static void mt7612u_mcu_dfs_detect_event_handler(struct rtmp_adapter *ad, char *payload, u16 payload_len)
{



}

MSG_EVENT_HANDLER msg_event_handler_tb[] =
{
	mt7612u_mcu_pwr_event_handler,
	mt7612u_mcu_wow_event_handler,
	mt7612u_mcu_carrier_detect_event_handler,
	mt7612u_mcu_dfs_detect_event_handler,
};

int mt7612u_mcu_random_write(struct rtmp_adapter *ad, struct rtmp_reg_pair *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	struct rtmp_chip_cap *cap = &ad->chipCap;
	int ret = 0;
	bool last_packet = false;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len)
	{
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen
									? cap->InbandPacketMaxLen : (var_len - cur_len);

		if ((sent_len < cap->InbandPacketMaxLen) || (cur_len + cap->InbandPacketMaxLen) == var_len)
			last_packet = true;

		msg = mt7612u_mcu_alloc_cmd_msg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		if (last_packet)
			mt7612u_mcu_init_cmd_msg(msg, CMD_RANDOM_WRITE, true, 0, true, true);
		else
			mt7612u_mcu_init_cmd_msg(msg, CMD_RANDOM_WRITE, false, 0, false, false);

		for (i = 0; i < (sent_len / 8); i++)
		{
			/* Address */
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

			/* UpdateData */
			value = cpu2le32(reg_pair[i + cur_index].Value);
			mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);
		};

		ret = mt7612u_mcu_send_cmd_msg(ad, msg);


		cur_index += (sent_len / 8);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

int mt7612u_mcu_pwr_saving(struct rtmp_adapter *ad, u32 op, u32 level)
{
	struct cmd_msg *msg;
	unsigned int var_len;
	u32 value;
	int ret = 0;

	/* Power operation and Power Level */
	var_len = 8;

	msg = mt7612u_mcu_alloc_cmd_msg(ad, var_len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_POWER_SAVING_OP, false, 0, false, false);

	/* Power operation */
	value = cpu2le32(op);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	/* Power Level */
	value = cpu2le32(level);

	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	ret = mt7612u_mcu_send_cmd_msg(ad, msg);

error:
	return ret;
}

int mt7612u_mcu_fun_set(struct rtmp_adapter *ad, u32 fun_id, u32 param)
{
	struct cmd_msg *msg;
	u32 value;
	int ret = 0;

	/* Function ID and Parameter */
	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	if (fun_id != Q_SELECT)
		mt7612u_mcu_init_cmd_msg(msg, CMD_FUN_SET_OP, true, 0, true, true);
	else
		mt7612u_mcu_init_cmd_msg(msg, CMD_FUN_SET_OP, false, 0, false, false);

	/* Function ID */
	value = cpu2le32(fun_id);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	/* Parameter */
	value = cpu2le32(param);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	ret = mt7612u_mcu_send_cmd_msg(ad, msg);

error:
	return ret;
}

void mt7612u_mcu_calibration(struct rtmp_adapter *ad, u32 cal_id, u32 param)
{
	struct cmd_msg *msg;
	u32 value;

	DBGPRINT(RT_DEBUG_INFO, ("%s:cal_id(%d)\n ", __FUNCTION__, cal_id));


	/* Calibration ID and Parameter */

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		return;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_CALIBRATION_OP, true, 0, true, true);

	/* Calibration ID */
	value = cpu2le32(cal_id);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	value = cpu2le32(param);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	mt7612u_mcu_send_cmd_msg(ad, msg);

}

void mt7612u_mcu_tssi_comp(struct rtmp_adapter *ad, struct mt7612u_tssi_comp *param)
{
	struct cmd_msg *msg;
	u32 value;

	DBGPRINT(RT_DEBUG_INFO, ("%s:cal_id(%d)\n ", __FUNCTION__, TSSI_COMPENSATION_7662));


	/* Calibration ID and Parameter */
	msg = mt7612u_mcu_alloc_cmd_msg(ad, 12);

	if (!msg) {
		return;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_CALIBRATION_OP, true, 0, true, true);

	/* Calibration ID */
	value = cpu2le32(TSSI_COMPENSATION_7662);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	/* Parameter */
	value = cpu2le32(param->pa_mode);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	value = cpu2le32(param->tssi_slope_offset);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	mt7612u_mcu_send_cmd_msg(ad, msg);

}


int mt7612u_mcu_load_cr(struct rtmp_adapter *ad, u32 cr_type, UINT8 temp_level, UINT8 channel)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s:cr_type(%d) temp_level(%d) channel(%d)\n", __FUNCTION__, cr_type, temp_level, channel));

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_LOAD_CR, true, 0, true, true);

	/* CR type */
	value &= ~LOAD_CR_MODE_MASK;
	value |= LOAD_CR_MODE(cr_type);

	if (cr_type == HL_TEMP_CR_UPDATE) {
		value &= ~LOAD_CR_TEMP_LEVEL_MASK;
		value |= LOAD_CR_TEMP_LEVEL(temp_level);

		value &= ~LOAD_CR_CHL_MASK;
		value |= LOAD_CR_CHL(channel);
	}

	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	value = 0x80000000;
	value |= ((ad->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] >> 8) & 0xFF);
	value |= ((ad->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] & 0xFF) << 8 );
	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	ret = mt7612u_mcu_send_cmd_msg(ad, msg);

error:
	return ret;
}

int mt7612u_mcu_switch_channel(struct rtmp_adapter *ad, u8 channel, bool scan, unsigned int bw, unsigned int tx_rx_setting, u8 bbp_ch_idx)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret;

	DBGPRINT(RT_DEBUG_INFO, ("%s:channel(%d),scan(%d),bw(%d),trx(0x%x)\n", __FUNCTION__, channel, scan, bw, tx_rx_setting));

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_SWITCH_CHANNEL_OP, true, 0, true, true);

	/*
     * switch channel related param
     * channel, scan, bw, tx_rx_setting
     */
	value &= ~SC_PARAM1_CHL_MASK;
	value |= SC_PARAM1_CHL(channel);
	value &= ~SC_PARAM1_SCAN_MASK;
	value |= SC_PARAM1_SCAN(scan);
	value &= ~SC_PARAM1_BW_MASK;
	value |= SC_PARAM1_BW(bw);
	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	value = 0;
	value |= SC_PARAM2_TR_SETTING(tx_rx_setting);
	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	ret = mt7612u_mcu_send_cmd_msg(ad, msg);

	mdelay(5);

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_SWITCH_CHANNEL_OP, true, 0, true, true);

	/*
     * switch channel related param
     * channel, scan, bw, tx_rx_setting, extension channel
     */
	value &= ~SC_PARAM1_CHL_MASK;
	value |= SC_PARAM1_CHL(channel);
	value &= ~SC_PARAM1_SCAN_MASK;
	value |= SC_PARAM1_SCAN(scan);
	value &= ~SC_PARAM1_BW_MASK;
	value |= SC_PARAM1_BW(bw);
	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	value = 0;
	value |= SC_PARAM2_TR_SETTING(tx_rx_setting);
	value &= ~SC_PARAM2_EXTENSION_CHL_MASK;

	if (bbp_ch_idx == 0)
		value |= SC_PARAM2_EXTENSION_CHL(0xe0);
	else if (bbp_ch_idx == 1)
		value |= SC_PARAM2_EXTENSION_CHL(0xe1);
	else if (bbp_ch_idx == 2)
		value |= SC_PARAM2_EXTENSION_CHL(0xe2);
	else if (bbp_ch_idx == 3)
		value |= SC_PARAM2_EXTENSION_CHL(0xe3);

	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	ret = mt7612u_mcu_send_cmd_msg(ad, msg);

error:
	return ret;
}

int mt7612u_mcu_init_gain(struct rtmp_adapter *ad, UINT8 channel, bool force_mode, unsigned int gain_from_e2p)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s:channel(%d), force mode(%d), init gain parameter(0x%08x)\n",
		__FUNCTION__, channel, force_mode, gain_from_e2p));

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_INIT_GAIN_OP, true, 0, true, true);

	/* init gain parameter#1 */
	if (force_mode == true)
		value = 0x80000000;

	value |= channel;
	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	/* init gain parameter#2 while force mode is enabled */
	value = gain_from_e2p;
	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	ret = mt7612u_mcu_send_cmd_msg(ad, msg);

error:
	return ret;
}

int mt7612u_mcu_dynamic_vga(struct rtmp_adapter *ad, UINT8 channel, bool mode, bool ext, int rssi, unsigned int false_cca)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int rssi_val = 0, ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s:channel(%d), ap/sta mode(%d), extension(%d), rssi(%d), false cca count(%d)\n",
		__FUNCTION__, channel, mode, ext, rssi, false_cca));

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_DYNC_VGA_OP, true, 0, true, true);

	/* dynamic VGA parameter#1: true = AP mode ; false = STA mode */
	if (mode == true)
		value |= 0x80000000;

	if (ext == true)
		value |= 0x40000000;

	value |= channel;
	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	/* dynamic VGA parameter#2: RSSI (signed value) */
	rssi_val = cpu2le32(rssi);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&rssi_val, 4);

	/* dynamic VGA parameter#3: false CCA count */
	value = cpu2le32(false_cca);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	ret = mt7612u_mcu_send_cmd_msg(ad, msg);

error:
	return ret;
}

int mt7612u_mcu_led_op(struct rtmp_adapter *ad, u32 led_idx, u32 link_status)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s:led_idx(%d), link_status(%d)\n ",
		__FUNCTION__, led_idx, link_status));

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_LED_MODE_OP, false, 0, false, false);

	/* Led index */
	value = cpu2le32(led_idx);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	/* Link status */
	value = cpu2le32(link_status);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	ret = mt7612u_mcu_send_cmd_msg(ad, msg);

error:
	return ret;
}



#ifdef RTMP_USB_SUPPORT
void mt7612u_mcu_usb_fw_init(struct rtmp_adapter *ad)
{
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	mt7612u_write32(ad, HEADER_TRANS_CTRL_REG, 0x0);
	mt7612u_write32(ad, TSO_CTRL, 0x0);

	RT28XXDMAEnable(ad);
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	mt7612u_mcu_fun_set(ad, Q_SELECT, ad->chipCap.CmdRspRxRing);
	usb_rx_cmd_msgs_receive(ad);
	mt7612u_mcu_pwr_saving(ad, RADIO_ON, 0);
}
#endif /* RTMP_USB_SUPPORT */

