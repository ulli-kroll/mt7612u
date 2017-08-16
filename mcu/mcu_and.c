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
#include "bitfield.h"

#define MT_DMA_HDR_LEN			4
#define MT_TXD_INFO_LEN			GENMASK(15, 0)
#define MT_TXD_CMD_INFO_SEQ            	GENMASK(19, 16)
#define MT_TXD_CMD_INFO_TYPE            GENMASK(26, 20)
#define MT_TXD_INFO_D_PORT		GENMASK(29, 27)
#define MT_TXD_INFO_TYPE		GENMASK(31, 30)

struct mt7612u_dma_buf {
	struct urb *urb;
	void *buf;
	dma_addr_t dma;
	size_t len;
};

bool mt7612u_usb_alloc_buf(struct rtmp_adapter *ad, size_t len,
			   struct mt7612u_dma_buf *buf)
{
	struct usb_device *usb_dev = mt7612u_to_usb_dev(ad);

	buf->len = len;
	buf->urb = usb_alloc_urb(0, GFP_KERNEL);
	buf->buf = usb_alloc_coherent(usb_dev, buf->len, GFP_KERNEL, &buf->dma);

	return !buf->urb || !buf->buf;
}

void mt7612u_usb_free_buf(struct rtmp_adapter *ad, struct mt7612u_dma_buf *buf)
{
	struct usb_device *usb_dev = mt7612u_to_usb_dev(ad);

	usb_free_coherent(usb_dev, buf->len, buf->buf, buf->dma);
	usb_free_urb(buf->urb);
}

inline int mt7612u_dma_skb_wrap(struct sk_buff *skb,
				       enum D_PORT d_port,
				       enum INFO_TYPE type, u32 flags)
{
	u32 info;

	/* Buffer layout:
	 *	|   4B   | xfer len |      pad       |  4B  |
	 *	| TXINFO | pkt/cmd  | zero pad to 4B | zero |
	 *
	 * length field of TXINFO should be set to 'xfer len'.
	 */

	info = flags |
		FIELD_PREP(MT_TXD_INFO_LEN, round_up(skb->len, 4)) |
		FIELD_PREP(MT_TXD_INFO_D_PORT, d_port) |
		FIELD_PREP(MT_TXD_INFO_TYPE, type);

	put_unaligned_le32(info, skb_push(skb, sizeof(info)));
	return skb_put_padto(skb, round_up(skb->len, 4) + 4);
}

static inline void mt7612u_dma_skb_wrap_cmd(struct sk_buff *skb,
					    u8 seq, enum mcu_cmd_type cmd)
{
	WARN_ON(mt7612u_dma_skb_wrap(skb, CPU_TX_PORT, CMD_PACKET,
				     FIELD_PREP(MT_TXD_CMD_INFO_SEQ, seq) |
				     FIELD_PREP(MT_TXD_CMD_INFO_TYPE, cmd)));
}

/* Known USB Vendor Commands */
#define MT7612U_VENDOR_DEVICE_MODE	0x01
#define MT7612U_VENDOR_SINGLE_WRITE	0x02
#define MT7612U_VENDOR_WRITE_MAC	0x06
#define MT7612U_VENDOR_READ_MAC		0x07
#define MT7612U_VENDOR_WRITE_EEPROM	0x08	/* Not used */
#define MT7612U_VENDOR_READ_EEPROM	0x09

#define MT7612U_VENDOR_WRITE_FCE	0x42
#define MT7612U_VENDOR_USB_CFG_READ	0x47
#define MT7612U_VENDOR_USB_CFG_WRITE	0x46

static void mt7612u_mcu_bh_schedule(struct rtmp_adapter *ad);
static int mt7612u_mcu_send_cmd_msg(struct rtmp_adapter *ad, struct cmd_msg *msg);

static void mt7612u_vendor_reset(struct rtmp_adapter *pAd)
{
	mt7612u_vendor_request(pAd, DEVICE_VENDOR_REQUEST_OUT,
			    MT7612U_VENDOR_DEVICE_MODE, 0x1, 0,
			    NULL, 0);

}

static int mt7612u_mcu_usb_enable_patch(struct rtmp_adapter *ad)
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

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __func__));

	ret = mt7612u_vendor_request(ad,
			  DEVICE_CLASS_REQUEST_OUT,
			  MT7612U_VENDOR_DEVICE_MODE,
			  0x12,
			  0x00,
			  cmd,
			  11);

	return ret;
}

static int mt7612u_mcu_usb_reset_wmt(struct rtmp_adapter *ad)
{
	int ret = NDIS_STATUS_SUCCESS;

	/* reset command */
	u8 cmd[8] = {0x6F, 0xFC, 0x05, 0x01, 0x07, 0x01, 0x00, 0x04};

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __func__));

	mt7612u_vendor_request(ad,
			DEVICE_CLASS_REQUEST_OUT,
			MT7612U_VENDOR_DEVICE_MODE,
			0x12,
			0x00,
			cmd,
			8);

	return ret;
}

static u16 checksume16(u8 *pData, int len)
{
	int sum = 0;

	while (len > 1) {
		sum += *((u16 *)pData);

		pData = pData + 2;

		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);

		len -= 2;
	}

	if (len)
		sum += *((u8 *)pData);

	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return ~sum;
}

static int mt7612u_mcu_usb_chk_crc(struct rtmp_adapter *ad, u32 checksum_len)
{
	int ret = 0;
	u8 cmd[8];
	struct rtmp_chip_cap *cap = &ad->chipCap;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __func__));

	memmove(cmd, &cap->rom_patch_offset, 4);
	memmove(&cmd[4], &checksum_len, 4);

	ret = mt7612u_vendor_request(ad,
			  DEVICE_VENDOR_REQUEST_OUT,
			  MT7612U_VENDOR_DEVICE_MODE,
			  0x20,
			  0x00,
			  cmd,
			  8);

	return ret;

}

static u16 mt7612u_mcu_usb_get_crc(struct rtmp_adapter *ad)
{
	int ret = 0;
	u16 crc, count = 0;

	while (1) {
		ret = mt7612u_vendor_request(ad,
				 DEVICE_VENDOR_REQUEST_IN,
				 MT7612U_VENDOR_DEVICE_MODE,
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

void mt7612u_complete_urb(struct urb *urb)
{
	struct completion *cmpl = urb->context;

	complete(cmpl);
}

static int __mt7612u_dma_fw(struct rtmp_adapter *ad,
			    const struct mt7612u_dma_buf *dma_buf,
			    const void *data, u32 len, u32 dst_addr)
{
	DECLARE_COMPLETION_ONSTACK(cmpl);
	struct mt7612u_dma_buf buf = *dma_buf; /* we need to fake length */
	struct usb_device *udev = mt7612u_to_usb_dev(ad);
	u16 value;
	u32 mac_value;
	int ret = 0;
	__le32 reg;

	reg = cpu_to_le32(FIELD_PREP(MT_TXD_INFO_TYPE, CMD_PACKET) |
			  FIELD_PREP(MT_TXD_INFO_D_PORT, CPU_TX_PORT) |
			  FIELD_PREP(MT_TXD_INFO_LEN, len));

	memmove(buf.buf, &reg, sizeof(reg));
	memmove(buf.buf + sizeof(reg), data, len);
	/* four zero bytes for end padding */
	memset(buf.buf + sizeof(reg) + len, 0, 4);

	value = dst_addr & 0xFFFF;

	/* Set FCE DMA descriptor */
	ret = mt7612u_vendor_request(ad,
			 DEVICE_VENDOR_REQUEST_OUT,
			 MT7612U_VENDOR_WRITE_FCE,
			 value,
			 0x230,
			 NULL,
			 0);


	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, ("set fce dma descriptor fail\n"));
		return ret;
	}

	value = ((dst_addr & 0xFFFF0000) >> 16);

	/* Set FCE DMA descriptor */
	ret = mt7612u_vendor_request(ad,
			 DEVICE_VENDOR_REQUEST_OUT,
			 MT7612U_VENDOR_WRITE_FCE,
			 value,
			 0x232,
			 NULL,
			 0);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, ("set fce dma descriptor fail\n"));
		return ret;
	}

	len = roundup(len, 4);

	value = ((len << 16) & 0xFFFF);

	/* Set FCE DMA length */
	ret = mt7612u_vendor_request(ad,
			 DEVICE_VENDOR_REQUEST_OUT,
			 MT7612U_VENDOR_WRITE_FCE,
			 value,
			 0x234,
			 NULL,
			 0);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, ("set fce dma length fail\n"));
		return ret;
	}

	value = (((len << 16) & 0xFFFF0000) >> 16);

	/* Set FCE DMA length */
	ret = mt7612u_vendor_request(ad,
			 DEVICE_VENDOR_REQUEST_OUT,
			 MT7612U_VENDOR_WRITE_FCE,
			 value,
			 0x236,
			 NULL,
			 0);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, ("set fce dma length fail\n"));
		return ret;
	}

	/* Initialize URB descriptor */
	RTUSB_FILL_HTTX_BULK_URB(buf.urb,
			 udev,
			 MT_COMMAND_BULK_OUT_ADDR,
			 buf.buf,
			 len + sizeof(reg) + 4,
			 mt7612u_complete_urb,
			 &cmpl,
			 buf.dma);

	ret = usb_submit_urb(buf.urb, GFP_ATOMIC);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, ("submit urb fail\n"));
		return ret;
	}

	if (!wait_for_completion_timeout(&cmpl, msecs_to_jiffies(1000))) {
		usb_kill_urb(buf.urb);
		ret = NDIS_STATUS_FAILURE;
		DBGPRINT(RT_DEBUG_ERROR, ("upload fw timeout\n"));
		return ret;
	}
	DBGPRINT(RT_DEBUG_OFF, ("."));

	mac_value = mt7612u_read32(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX);
	mac_value++;
	mt7612u_write32(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, mac_value);

	mdelay(5);

	return 0;
}

static int mt7612u_dma_fw(struct rtmp_adapter *ad,
			  const struct mt7612u_dma_buf *dma_buf,
			  const void *data, int len, u32 dst_addr)
{
	int pos = 0;
	int ret = 0;
	int sent_len_max = UPLOAD_PATCH_UNIT - MT_DMA_HDR_LEN - USB_END_PADDING;

	while (len > 0) {
		int sent_len = min(len, sent_len_max);

		DBGPRINT(RT_DEBUG_OFF, ("pos = %d\n", pos));
		DBGPRINT(RT_DEBUG_OFF, ("sent_len = %d\n", sent_len));

		__mt7612u_dma_fw(ad, dma_buf,
				data + pos, sent_len,
				dst_addr + pos);
		if (ret)
			return ret;


		pos += sent_len;
		len -= sent_len;

	}

	return ret;
}

int mt7612u_mcu_usb_load_rom_patch(struct rtmp_adapter *ad)
{
	struct usb_device *udev = mt7612u_to_usb_dev(ad);
	struct mt7612u_dma_buf dma_buf;
	int pos, patch_len = 0;
	u32 mac_value, loop = 0;
	int ret = 0, total_checksum = 0;
	struct rtmp_chip_cap *cap = &ad->chipCap;
	USB_DMA_CFG_STRUC cfg;
	u8 *fw_patch_image;
	const struct firmware *fw;
	int fw_chunk_len;

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
				 ("%s: can not get the hw semaphore\n", __func__));
			return NDIS_STATUS_FAILURE;
		}
	}

	/* Check rom patch if ready */
	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
		mac_value = mt7612u_read32(ad, CLOCK_CTL);

		if (((mac_value & 0x01) == 0x01) &&
		    (cap->rom_code_protect))
			goto error0;
	} else {
		mac_value = mt7612u_read32(ad, COM_REG0);

		if (((mac_value & 0x02) == 0x02) &&
		    (cap->rom_code_protect))
			goto error0;
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
		if (((strncmp(fw_patch_image, "20130809", 8) >= 0)) &&
		    (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))) {
			DBGPRINT(RT_DEBUG_OFF, ("rom patch for E3 IC\n"));

		} else if (((strncmp(fw_patch_image, "20130809", 8) < 0)) &&
			   (MT_REV_LT(ad, MT76x2, REV_MT76x2E3))) {
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

	DBGPRINT(RT_DEBUG_OFF, ("platform =\n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(fw_patch_image + 16 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("hw/sw version =\n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(fw_patch_image + 20 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("patch version =\n"));

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

	/* Allocate TransferBuffer */
	if (mt7612u_usb_alloc_buf(ad, UPLOAD_PATCH_UNIT, &dma_buf)) {
		ret = -ENOMEM;
		goto error0;
	}

	DBGPRINT(RT_DEBUG_OFF, ("loading rom patch"));

	pos = 0x00;
	patch_len = fw->size - PATCH_INFO_SIZE;
	fw_chunk_len = patch_len - pos;

	ret = mt7612u_dma_fw(ad, &dma_buf,
			     fw_patch_image + PATCH_INFO_SIZE + pos, fw_chunk_len,
			     pos + cap->rom_patch_offset);
	if (ret < 0)
		goto  error2;

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
		DBGPRINT(RT_DEBUG_TRACE, ("%s: CLOCK_CTL(0x%x) = 0x%x\n", __func__, CLOCK_CTL, mac_value));

		if ((mac_value & 0x01) != 0x1)
			ret = NDIS_STATUS_FAILURE;
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("%s: CLOCK_CTL(0x%x) = 0x%x\n", __func__, COM_REG0, mac_value));

		if ((mac_value & 0x02) != 0x2)
			ret = NDIS_STATUS_FAILURE;
	}

error2:
	/* Free TransferBuffer */
	mt7612u_usb_free_buf(ad, &dma_buf);

error0:
	if (cap->rom_code_protect)
		mt7612u_write32(ad, SEMAPHORE_03, 0x1);

	return ret;
}

static int usb_load_ivb(struct rtmp_adapter *ad, u8 *fw_image)
{
	int Status = NDIS_STATUS_SUCCESS;
	struct rtmp_chip_cap *cap = &ad->chipCap;


	if (cap->load_iv) {
		Status = mt7612u_vendor_request(ad,
				 DEVICE_VENDOR_REQUEST_OUT,
				 MT7612U_VENDOR_DEVICE_MODE,
				 0x12,
				 0x00,
				 fw_image + 32,
				 64);
	} else {
		Status = mt7612u_vendor_request(ad,
				 DEVICE_VENDOR_REQUEST_OUT,
				 MT7612U_VENDOR_DEVICE_MODE,
				 0x12,
				 0x00,
				 NULL,
				 0x00);

	}

	if (Status) {
		DBGPRINT(RT_DEBUG_ERROR, ("Upload IVB Fail\n"));
		return Status;
	}

	return Status;
}

int mt7612u_mcu_usb_loadfw(struct rtmp_adapter *ad)
{
	struct usb_device *udev = mt7612u_to_usb_dev(ad);
	struct mt7612u_dma_buf dma_buf;
	int pos = 0, ilm_len = 0, dlm_len = 0;
	u32 mac_value, loop = 0, addr;
	int ret = 0;
	struct rtmp_chip_cap *cap = &ad->chipCap;
	USB_DMA_CFG_STRUC cfg;
	u16 fw_ver, build_ver;
	const struct firmware *fw;
	u8 *fw_image;
	int fw_chunk_len;

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
				 ("%s: can not get the hw semaphore\n", __func__));
			return NDIS_STATUS_FAILURE;
		}
	}

	/* Check MCU if ready */
	mac_value = mt7612u_read32(ad, COM_REG0);

	if (((mac_value & 0x01) == 0x01) && (cap->ram_code_protect))
		goto error0;

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

		} else if (((strncmp(fw_image + 16, "20130811", 8) < 0)) &&
			   (MT_REV_LT(ad, MT76x2, REV_MT76x2E3))) {
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

	/* Allocate TransferBuffer */
	if (mt7612u_usb_alloc_buf(ad, UPLOAD_FW_UNIT, &dma_buf)) {
		ret = -ENOMEM;
		goto error0;
	}

	DBGPRINT(RT_DEBUG_OFF, ("loading fw"));

	pos = (cap->load_iv) ? 0x40 : 0x00;
	fw_chunk_len = ilm_len - pos;

	/* Loading ILM */
	ret = mt7612u_dma_fw(ad, &dma_buf,
			     fw_image + FW_INFO_SIZE + pos, fw_chunk_len,
			     pos + cap->ilm_offset);

	if (ret < 0)
		goto  error2;

	pos = 0x00;
	fw_chunk_len = dlm_len - pos;
	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))
		addr = pos + cap->dlm_offset + 0x800;
	else
		addr = pos + cap->dlm_offset;

	/* Loading DLM */
	ret = mt7612u_dma_fw(ad, &dma_buf,
			     fw_image + FW_INFO_SIZE + ilm_len + pos, fw_chunk_len,
			     addr);

	if (ret < 0)
		goto  error2;

	if (ret < 0)
		goto  error2;
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

	DBGPRINT(RT_DEBUG_TRACE, ("%s: COM_REG0(0x%x) = 0x%x\n", __func__, COM_REG0, mac_value));

	mac_value = mt7612u_read32(ad, COM_REG0);
	mac_value |= (1 << 1);
	mt7612u_write32(ad, COM_REG0, mac_value);

	if ((mac_value & 0x01) != 0x01)
		ret = NDIS_STATUS_FAILURE;

error2:
	/* Free TransferBuffer */
	mt7612u_usb_free_buf(ad, &dma_buf);

error0:
	if (cap->ram_code_protect)
		mt7612u_write32(ad, SEMAPHORE_00, 0x1);

	/* Enable FCE to send in-band cmd */
	mt7612u_write32(ad, FCE_PSE_CTRL, 0x01);

	return ret;
}

static struct cmd_msg *mt7612u_mcu_alloc_cmd_msg(struct rtmp_adapter *ad, unsigned int length)
{
	struct cmd_msg *msg = NULL;
	struct sk_buff *skb = NULL;
	struct urb *urb = NULL;

	/* ULLI :
	 * orignal drver used 4 bytes padding, we need 8 bytes due
	 * skb_panic in skb_put() mt7612u_dma_skb_wrap_cmd()
	 * wll check ths later on */
	skb = dev_alloc_skb(MT_DMA_HDR_LEN + length + 8);

	if (!skb) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate skb\n"));
		goto error0;
	}

	skb_reserve(skb, MT_DMA_HDR_LEN);

	msg = kmalloc(sizeof(*msg), GFP_ATOMIC);

	if (!msg) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate cmd msg\n"));
		goto error1;
	}

	CMD_MSG_CB(skb)->msg = msg;

	memset(msg, 0x00, sizeof(*msg));

	urb = usb_alloc_urb(0, GFP_ATOMIC);

	if (!urb) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate urb\n"));
		goto error2;
	}

	msg->urb = urb;

	msg->priv = (void *)ad;
	msg->skb = skb;

	return msg;

error2:
	kfree(msg);
error1:
	dev_kfree_skb_any(skb);
error0:
	return NULL;
}

static void mt7612u_mcu_init_cmd_msg(struct cmd_msg *msg, enum mcu_cmd_type type,
				     bool need_wait,
				     bool need_retransmit, bool need_rsp)
{
	u16 rsp_payload_len = 0;
	char *rsp_payload = NULL;

	msg->type = type;
	msg->need_wait = need_wait;

	if (need_wait)
		init_completion(&msg->ack_done);

	msg->need_retransmit = need_retransmit;

	if (need_retransmit)
		msg->retransmit_times = CMD_MSG_RETRANSMIT_TIMES;

	msg->need_rsp = need_rsp;
	msg->rsp_payload_len = rsp_payload_len;
	msg->rsp_payload = rsp_payload;
}

static void mt7612u_mcu_append_cmd_msg(struct cmd_msg *msg, char *data, unsigned int len)
{
	struct sk_buff *skb = msg->skb;

	if (data)
		memcpy(skb_put(skb, len), data, len);
}

static void mt7612u_mcu_free_cmd_msg(struct cmd_msg *msg)
{
	struct sk_buff *skb = msg->skb;

	usb_free_urb(msg->urb);

	kfree(msg);

	dev_kfree_skb_any(skb);
}

static spinlock_t *mt7612u_mcu_get_spin_lock(struct mt7612u_mcu_ctrl  *ctl, DL_LIST *list)
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
	else if (list == &ctl->rx_doneq)
		lock = &ctl->rx_doneq_lock;
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal list\n", __func__));

	return lock;
}

static inline u8 mt7612u_mcu_get_cmd_msg_seq(struct rtmp_adapter *ad)
{
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg;
	unsigned long flags;

	spin_lock_irqsave(&ctl->ackq_lock, flags);
get_seq:
	ctl->cmd_seq >= 0xf ? ctl->cmd_seq = 1 : ctl->cmd_seq++;


       for (msg = DlListEntry((&ctl->ackq)->Next, struct cmd_msg, list);
               &msg->list != &ctl->ackq;
               msg = DlListEntry(msg->list.Next, struct cmd_msg, list)) {

		if (msg->seq == ctl->cmd_seq) {
			DBGPRINT(RT_DEBUG_ERROR, ("command(seq: %d) is still running\n", ctl->cmd_seq));
			goto get_seq;
		}
	}
	spin_unlock_irqrestore(&ctl->ackq_lock, flags);

	return ctl->cmd_seq;
}

static void mt7612u_mcu_queue_tail_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
					   enum cmd_msg_state state)
{
	unsigned long flags;
	spinlock_t *lock;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)msg->priv;
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	msg->state = state;
	DlListAddTail(list, &msg->list);
	spin_unlock_irqrestore(lock, flags);
}

static void mt7612u_mcu_queue_head_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
					   enum cmd_msg_state state)
{
	unsigned long flags;
	spinlock_t *lock;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)msg->priv;
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);
	msg->state = state;
	DlListAdd(list, &msg->list);
	spin_unlock_irqrestore(lock, flags);
}

static int mt7612u_mcu_queue_empty(struct mt7612u_mcu_ctrl  *ctl, DL_LIST *list)
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
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;

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

static struct cmd_msg *mt7612u_mcu_dequeue_cmd_msg(struct mt7612u_mcu_ctrl  *ctl, DL_LIST *list)
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

static void mt7612u_mcu_rx_process_cmd_msg(struct rtmp_adapter *ad, struct cmd_msg *rx_msg)
{
	struct sk_buff *skb = rx_msg->skb;
	struct cmd_msg *msg, *msg_tmp;
	RXFCE_INFO_CMD *rx_info = (RXFCE_INFO_CMD *)skb->data;
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((u8 *)rx_info, TYPE_RXINFO);
#endif

	DBGPRINT(RT_DEBUG_INFO, ("(andex_rx_cmd)info_type=%d,evt_type=%d,d_port=%d,"
				 "qsel=%d,pcie_intr=%d,cmd_seq=%d,"
				 "self_gen=%d,pkt_len=%d\n",
				 rx_info->info_type, rx_info->evt_type, rx_info->d_port,
				 rx_info->qsel, rx_info->pcie_intr, rx_info->cmd_seq,
				 rx_info->self_gen, rx_info->pkt_len));

	if (rx_info->info_type != CMD_PACKET) {
		DBGPRINT(RT_DEBUG_ERROR, ("packet is not command response/self event\n"));
		return;
	}

	if (rx_info->self_gen) {
		/* if have callback function */
		RTEnqueueInternalCmd(ad, CMDTHREAD_RESPONSE_EVENT_CALLBACK,
				     skb->data + sizeof(*rx_info), rx_info->pkt_len);
	} else {
		spin_lock_irq(&ctl->ackq_lock);

		for (msg = DlListEntry((&ctl->ackq)->Next, struct cmd_msg, list), msg_tmp = DlListEntry(msg->list.Next, struct cmd_msg, list);
			&msg->list != &ctl->ackq;
			msg = msg_tmp, msg_tmp = DlListEntry(msg_tmp->list.Next, struct cmd_msg, list)) {

			if (msg->seq == rx_info->cmd_seq) {
				_mt7612u_mcu_unlink_cmd_msg(msg, &ctl->ackq);
				spin_unlock_irq(&ctl->ackq_lock);

				if ((msg->rsp_payload_len == rx_info->pkt_len) &&
				    (msg->rsp_payload_len != 0)) {
					    ;
				} else if ((msg->rsp_payload_len == 0) && (rx_info->pkt_len == 8)) {
					DBGPRINT(RT_DEBUG_INFO, ("command response(ack) success\n"));
				} else {
					DBGPRINT(RT_DEBUG_ERROR, ("expect response len(%d), command response len(%d) invalid\n", msg->rsp_payload_len, rx_info->pkt_len));
					msg->rsp_payload_len = rx_info->pkt_len;
				}

				if (msg->need_wait)
					complete(&msg->ack_done);
				else
					mt7612u_mcu_free_cmd_msg(msg);

				spin_lock_irq(&ctl->ackq_lock);

				break;
			}
		}

		spin_unlock_irq(&ctl->ackq_lock);

	}
}


static void usb_rx_cmd_msg_complete(struct urb *urb)
{
	struct sk_buff *skb = urb->context;
	struct cmd_msg *msg = CMD_MSG_CB(skb)->msg;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)msg->priv;
	struct usb_device *udev = mt7612u_to_usb_dev(ad);
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;
	enum cmd_msg_state state;
	int ret = 0;

	mt7612u_mcu_unlink_cmd_msg(msg, &ctl->rxq);

	skb_put(skb, urb->actual_length);

	if (urb->status == 0) {
		state = RX_DONE;
	} else {
		state = RX_RECEIVE_FAIL;
		if (OS_TEST_BIT(MCU_INIT, &ctl->flags))
			ctl->rx_receive_fail_count++;

		DBGPRINT(RT_DEBUG_ERROR, ("receive cmd msg fail(%d)\n", urb->status));
	}

	mt7612u_mcu_queue_tail_cmd_msg(&ctl->rx_doneq, msg, state);

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		msg = mt7612u_mcu_alloc_cmd_msg(ad, 512);

		if (!msg)
			return;

		skb = msg->skb;

		usb_fill_bulk_urb(msg->urb, udev,
				  usb_rcvbulkpipe(udev, MT_COMMAND_RSP_BULK_IN_ADDR),
				  skb->data, 512, usb_rx_cmd_msg_complete, skb);

		mt7612u_mcu_queue_tail_cmd_msg(&ctl->rxq, msg, RX_START);

		ret = usb_submit_urb(msg->urb, GFP_ATOMIC);

		if (ret) {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->rxq);
			if (OS_TEST_BIT(MCU_INIT, &ctl->flags))
				ctl->rx_receive_fail_count++;

			DBGPRINT(RT_DEBUG_ERROR, ("%s:submit urb fail(%d)\n", __func__, ret));
			mt7612u_mcu_queue_tail_cmd_msg(&ctl->rx_doneq, msg, RX_RECEIVE_FAIL);
		}

	}

	mt7612u_mcu_bh_schedule(ad);
}

static int usb_rx_cmd_msg_submit(struct rtmp_adapter *ad)
{
	struct usb_device *udev = mt7612u_to_usb_dev(ad);
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg = NULL;
	struct sk_buff *skb = NULL;
	int ret = 0;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return ret;

	msg =  mt7612u_mcu_alloc_cmd_msg(ad, 512);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		return ret;
	}

	skb = msg->skb;

	usb_fill_bulk_urb(msg->urb, udev,
			  usb_rcvbulkpipe(udev, MT_COMMAND_RSP_BULK_IN_ADDR),
			  skb->data, 512, usb_rx_cmd_msg_complete, skb);

	mt7612u_mcu_queue_tail_cmd_msg(&ctl->rxq, msg, RX_START);

	ret = usb_submit_urb(msg->urb, GFP_ATOMIC);

	if (ret) {
		mt7612u_mcu_unlink_cmd_msg(msg, &ctl->rxq);
		if (OS_TEST_BIT(MCU_INIT, &ctl->flags))
			ctl->rx_receive_fail_count++;

		DBGPRINT(RT_DEBUG_ERROR, ("%s:submit urb fail(%d)\n", __func__, ret));
		mt7612u_mcu_queue_tail_cmd_msg(&ctl->rx_doneq, msg, RX_RECEIVE_FAIL);
	}

	return ret;
}

int usb_rx_cmd_msgs_receive(struct rtmp_adapter *ad)
{
	bool tmp;
	int ret = 0;
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;

	tmp = mt7612u_mcu_queue_empty(ctl, &ctl->rx_doneq);
	if (!tmp)
		return ret;

	ret = usb_rx_cmd_msg_submit(ad);

	return ret;
}

static void mt7612u_mcu_cmd_msg_bh(unsigned long param)
{
	struct rtmp_adapter *ad = (struct rtmp_adapter *)param;
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg = NULL;

	while (1) {
		msg = mt7612u_mcu_dequeue_cmd_msg(ctl, &ctl->rx_doneq);
		if (!msg)
			break;

		switch (msg->state) {
		case RX_DONE:
			mt7612u_mcu_rx_process_cmd_msg(ad, msg);
			mt7612u_mcu_free_cmd_msg(msg);
			continue;
		case RX_RECEIVE_FAIL:
			mt7612u_mcu_free_cmd_msg(msg);
			continue;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("unknow msg state(%d)\n", msg->state));
		}
	}

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		mt7612u_mcu_bh_schedule(ad);
		usb_rx_cmd_msgs_receive(ad);
	}
}

static void mt7612u_mcu_bh_schedule(struct rtmp_adapter *ad)
{
	bool tmp;
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	tmp = mt7612u_mcu_queue_empty(ctl, &ctl->rx_doneq);
	if (!tmp) {
		RTMP_NET_TASK_DATA_ASSIGN(&ctl->cmd_msg_task, (unsigned long)(ad));
		RTMP_OS_TASKLET_SCHE(&ctl->cmd_msg_task);
	}
}


static void usb_kick_out_cmd_msg_complete(struct urb *urb)
{
	struct sk_buff *skb = urb->context;
	struct cmd_msg *msg = CMD_MSG_CB(skb)->msg;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)msg->priv;
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	if (urb->status == 0) {
		if (!msg->need_rsp) {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->kickq);
			mt7612u_mcu_free_cmd_msg(msg);
		} else {
			msg->state = WAIT_ACK;
		}
	} else {
		if (!msg->need_rsp) {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->kickq);
			mt7612u_mcu_free_cmd_msg(msg);
			if (OS_TEST_BIT(MCU_INIT, &ctl->flags))
				ctl->tx_kickout_fail_count++;

		} else {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->ackq);
			msg->state = TX_KICKOUT_FAIL;
			if (OS_TEST_BIT(MCU_INIT, &ctl->flags))
				ctl->tx_kickout_fail_count++;

			complete(&msg->ack_done);
		}

		DBGPRINT(RT_DEBUG_ERROR, ("kick out cmd msg fail(%d)\n", urb->status));
	}

	mt7612u_mcu_bh_schedule(ad);
}

static int usb_kick_out_cmd_msg(struct rtmp_adapter *ad, struct cmd_msg *msg)
{
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;
	struct usb_device *udev = mt7612u_to_usb_dev(ad);
	int ret = 0;
	struct sk_buff *skb = msg->skb;

	if (msg->state != TX_RETRANSMIT) {
		/* append four zero bytes padding when usb aggregate enable */
		memset(skb_put(skb, USB_END_PADDING), 0x00, USB_END_PADDING);
	}

	usb_fill_bulk_urb(msg->urb, udev,
			  usb_sndbulkpipe(udev, MT_COMMAND_BULK_OUT_ADDR),
			  skb->data, skb->len, usb_kick_out_cmd_msg_complete, skb);

	if (msg->need_rsp)
		mt7612u_mcu_queue_tail_cmd_msg(&ctl->ackq, msg, WAIT_CMD_OUT_AND_ACK);
	else
		mt7612u_mcu_queue_tail_cmd_msg(&ctl->kickq, msg, WAIT_CMD_OUT);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return -1;

	ret = usb_submit_urb(msg->urb, GFP_ATOMIC);

	if (ret) {
		if (!msg->need_rsp) {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->kickq);
			mt7612u_mcu_free_cmd_msg(msg);
			if (OS_TEST_BIT(MCU_INIT, &ctl->flags))
				ctl->tx_kickout_fail_count++;
		} else {
			mt7612u_mcu_unlink_cmd_msg(msg, &ctl->ackq);
			msg->state = TX_KICKOUT_FAIL;
			if (OS_TEST_BIT(MCU_INIT, &ctl->flags))
				ctl->tx_kickout_fail_count++;

			complete(&msg->ack_done);
		}

		DBGPRINT(RT_DEBUG_ERROR, ("%s:submit urb fail(%d)\n", __func__, ret));
	}

	return ret;
}

static void mt7612u_mcu_usb_unlink_urb(struct rtmp_adapter *ad, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg, *msg_tmp;
	spinlock_t *lock;
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);

	for (msg = DlListEntry(list->Next, struct cmd_msg, list), msg_tmp = DlListEntry(msg->list.Next, struct cmd_msg, list);
		&msg->list != &ctl->ackq;
		msg = msg_tmp, msg_tmp = DlListEntry(msg_tmp->list.Next, struct cmd_msg, list)) {

		spin_unlock_irqrestore(lock, flags);
		if ((msg->state == WAIT_CMD_OUT_AND_ACK) ||
		    (msg->state == WAIT_CMD_OUT) ||
		    (msg->state == TX_START) ||
		    (msg->state == RX_START) ||
		    (msg->state == TX_RETRANSMIT))
			usb_kill_urb(msg->urb);
		spin_lock_irqsave(lock, flags);
	}
	spin_unlock_irqrestore(lock, flags);
}

static void mt7612u_mcu_cleanup_cmd_msg(struct rtmp_adapter *ad, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg, *msg_tmp;
	spinlock_t *lock;
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;

	lock = mt7612u_mcu_get_spin_lock(ctl, list);

	spin_lock_irqsave(lock, flags);

	for (msg = DlListEntry(list->Next, struct cmd_msg, list), msg_tmp = DlListEntry(msg->list.Next, struct cmd_msg, list);
		&msg->list != &ctl->ackq;
		msg = msg_tmp, msg_tmp = DlListEntry(msg_tmp->list.Next, struct cmd_msg, list)) {

		_mt7612u_mcu_unlink_cmd_msg(msg, list);
		mt7612u_mcu_free_cmd_msg(msg);
	}
	DlListInit(list);
	spin_unlock_irqrestore(lock, flags);
}


static void mt7612u_mcu_ctrl_usb_init(struct rtmp_adapter *ad)
{
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;
	int ret = 0;

	ret = down_interruptible(&(ad->mcu_atomic));
	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	ctl->cmd_seq = 0;
	RTMP_OS_TASKLET_INIT(ad, &ctl->cmd_msg_task, mt7612u_mcu_cmd_msg_bh, (unsigned long)ad);

	DlListInit(&ctl->txq);
	DlListInit(&ctl->rxq);
	DlListInit(&ctl->ackq);
	DlListInit(&ctl->kickq);
	DlListInit(&ctl->rx_doneq);

	spin_lock_init(&ctl->txq_lock);
	spin_lock_init(&ctl->rxq_lock);
	spin_lock_init(&ctl->ackq_lock);
	spin_lock_init(&ctl->kickq_lock);
	spin_lock_init(&ctl->rx_doneq_lock);

	ctl->tx_kickout_fail_count = 0;
	ctl->tx_timeout_fail_count = 0;
	ctl->rx_receive_fail_count = 0;
	OS_SET_BIT(MCU_INIT, &ctl->flags);
	usb_rx_cmd_msgs_receive(ad);
	up(&(ad->mcu_atomic));
}

void mt7612u_mcu_ctrl_init(struct rtmp_adapter *ad)
{
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		mt7612u_mcu_ctrl_usb_init(ad);

	ctl->power_on = false;
	ctl->dpd_on = false;
}

static void mt7612u_mcu_ctrl_usb_exit(struct rtmp_adapter *ad)
{
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;
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
	mt7612u_mcu_cleanup_cmd_msg(ad, &ctl->rx_doneq);
	DBGPRINT(RT_DEBUG_OFF, ("tx_kickout_fail_count = %ld\n", ctl->tx_kickout_fail_count));
	DBGPRINT(RT_DEBUG_OFF, ("tx_timeout_fail_count = %ld\n", ctl->tx_timeout_fail_count));
	DBGPRINT(RT_DEBUG_OFF, ("rx_receive_fail_count = %ld\n", ctl->rx_receive_fail_count));
	up(&(ad->mcu_atomic));
}


void mt7612u_mcu_ctrl_exit(struct rtmp_adapter *ad)
{
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags))
		mt7612u_mcu_ctrl_usb_exit(ad);

	ctl->power_on = false;
	ctl->dpd_on = false;
}

static int mt7612u_mcu_dequeue_and_kick_out_cmd_msgs(struct rtmp_adapter *ad)
{
	struct cmd_msg *msg = NULL;
	struct sk_buff *skb = NULL;
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;
	int ret = NDIS_STATUS_SUCCESS;

	while (1) {
		bool tmp;

		msg = mt7612u_mcu_dequeue_cmd_msg(ctl, &ctl->txq);
		if (!msg)
			break;
		if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD) ||
		    RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
		    RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
			if (!msg->need_rsp)
				mt7612u_mcu_free_cmd_msg(msg);
			continue;
		}

		tmp = mt7612u_mcu_queue_empty(ctl, &ctl->ackq);
		if (!tmp) {
			mt7612u_mcu_queue_head_cmd_msg(&ctl->txq, msg, msg->state);
			ret = NDIS_STATUS_FAILURE;
			continue;
		}

		skb = msg->skb;

		if (msg->state != TX_RETRANSMIT) {
			if (msg->need_rsp)
				msg->seq = mt7612u_mcu_get_cmd_msg_seq(ad);
			else
				msg->seq = 0;

			mt7612u_dma_skb_wrap_cmd(skb, msg->seq, msg->type);
		}


		ret = usb_kick_out_cmd_msg(ad, msg);

		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, ("kick out msg fail\n"));
			break;
		}
	}

	mt7612u_mcu_bh_schedule(ad);

	return ret;
}

static int mt7612u_mcu_send_cmd_msg(struct rtmp_adapter *ad, struct cmd_msg *msg)
{
	struct mt7612u_mcu_ctrl  *ctl = &ad->MCUCtrl;
	int ret = 0;
	bool need_wait = msg->need_wait;

	ret = down_interruptible(&(ad->mcu_atomic));

	if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD) ||
	    RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
	    RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
		mt7612u_mcu_free_cmd_msg(msg);

		up(&(ad->mcu_atomic));
		return NDIS_STATUS_FAILURE;
	}

	mt7612u_mcu_queue_tail_cmd_msg(&ctl->txq, msg, TX_START);

retransmit:
	mt7612u_mcu_dequeue_and_kick_out_cmd_msgs(ad);

	/* Wait for response */
	if (need_wait) {
		enum cmd_msg_state state;
		long expire;

		expire =  msecs_to_jiffies(CMD_MSG_TIMEOUT);

		if (!wait_for_completion_timeout(&msg->ack_done, expire)) {
			ret = NDIS_STATUS_FAILURE;
			DBGPRINT(RT_DEBUG_ERROR, ("command (%d) timeout(%dms)\n", msg->type, CMD_MSG_TIMEOUT));
			if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
				if (msg->state == WAIT_CMD_OUT_AND_ACK)
					usb_kill_urb(msg->urb);
				else if (msg->state == WAIT_ACK)
					mt7612u_mcu_unlink_cmd_msg(msg, &ctl->ackq);
			}

			if (OS_TEST_BIT(MCU_INIT, &ctl->flags))
				ctl->tx_timeout_fail_count++;

			state = TX_TIMEOUT_FAIL;
			if (msg->retransmit_times > 0)
				msg->retransmit_times--;
			DBGPRINT(RT_DEBUG_ERROR, ("msg->retransmit_times = %d\n", msg->retransmit_times));
		} else {
			if (msg->state == TX_KICKOUT_FAIL) {
				state = TX_KICKOUT_FAIL;
				msg->retransmit_times--;
			} else {
				state = TX_DONE;
				msg->retransmit_times = 0;
			}
		}

		if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
			if (msg->need_retransmit && (msg->retransmit_times > 0)) {
				init_completion(&msg->ack_done);
				state = TX_RETRANSMIT;
				mt7612u_mcu_queue_head_cmd_msg(&ctl->txq, msg, state);
				goto retransmit;
			} else {
				mt7612u_mcu_free_cmd_msg(msg);
			}
		} else {
			mt7612u_mcu_free_cmd_msg(msg);
		}
	}

	up(&(ad->mcu_atomic));

	return ret;
}

int mt7612u_mcu_random_write(struct rtmp_adapter *ad, struct rtmp_reg_pair *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, pos = 0, sent_len;
	u32 value, i, cur_index = 0;
	struct rtmp_chip_cap *cap = &ad->chipCap;
	int ret = 0;
	bool last_packet = false;

	if (!reg_pair)
		return -1;

	while (pos < var_len) {
		sent_len = ((var_len - pos) > MT_INBAND_PACKET_MAX_LEN) ?
				MT_INBAND_PACKET_MAX_LEN : (var_len - pos);

		if ((sent_len < MT_INBAND_PACKET_MAX_LEN) ||
		    (pos + MT_INBAND_PACKET_MAX_LEN) == var_len)
			last_packet = true;

		msg = mt7612u_mcu_alloc_cmd_msg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		if (last_packet)
			mt7612u_mcu_init_cmd_msg(msg, CMD_RANDOM_WRITE, true, true, true);
		else
			mt7612u_mcu_init_cmd_msg(msg, CMD_RANDOM_WRITE, false, false, false);

		for (i = 0; i < (sent_len / 8); i++) {
			/* Address */
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

			/* UpdateData */
			value = cpu2le32(reg_pair[i + cur_index].Value);
			mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);
		};

		ret = mt7612u_mcu_send_cmd_msg(ad, msg);

		cur_index += (sent_len / 8);
		pos += MT_INBAND_PACKET_MAX_LEN;
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

	mt7612u_mcu_init_cmd_msg(msg, CMD_POWER_SAVING_OP, false,false, false);

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
		mt7612u_mcu_init_cmd_msg(msg, CMD_FUN_SET_OP, true, true, true);
	else
		mt7612u_mcu_init_cmd_msg(msg, CMD_FUN_SET_OP, false, false, false);

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

void mt7612u_mcu_calibration(struct rtmp_adapter *ad, enum mt7612u_mcu_calibration cal_id, u32 param)
{
	struct cmd_msg *msg;
	u32 value;

	DBGPRINT(RT_DEBUG_INFO, ("%s:cal_id(%d)\n ", __func__, cal_id));


	/* Calibration ID and Parameter */

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg)
		return;

	mt7612u_mcu_init_cmd_msg(msg, CMD_CALIBRATION_OP, true, true, true);

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

	DBGPRINT(RT_DEBUG_INFO, ("%s:cal_id(%d)\n ", __func__, TSSI_COMPENSATION_7662));


	/* Calibration ID and Parameter */
	msg = mt7612u_mcu_alloc_cmd_msg(ad, 12);

	if (!msg)
		return;

	mt7612u_mcu_init_cmd_msg(msg, CMD_CALIBRATION_OP, true, true, true);

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

	DBGPRINT(RT_DEBUG_OFF, ("%s:cr_type(%d) temp_level(%d) channel(%d)\n", __func__, cr_type, temp_level, channel));

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_LOAD_CR, true, true, true);

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
	mt7612u_mcu_append_cmd_msg(msg, (char *) &value, 4);

	value = 0x80000000;
	value |= ((ad->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] >> 8) & 0xFF);
	value |= ((ad->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] & 0xFF) << 8);
	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *) &value, 4);

	ret = mt7612u_mcu_send_cmd_msg(ad, msg);

error:
	return ret;
}

int mt7612u_mcu_switch_channel(struct rtmp_adapter *ad, u8 channel,
			       bool scan, unsigned int bw,
			       unsigned int tx_rx_setting, u8 bbp_ch_idx)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret;

	DBGPRINT(RT_DEBUG_INFO, ("%s:channel(%d),scan(%d),bw(%d),trx(0x%x)\n", __func__, channel, scan, bw, tx_rx_setting));

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_SWITCH_CHANNEL_OP, true, true, true);

	/*
	 * switch channel related param
	 * channel, scan, bw, tx_rx_setting
	 */

	value = 0;
	value |= SC_PARAM1_CHL(channel);
	value |= SC_PARAM1_SCAN(scan);
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

	mt7612u_mcu_init_cmd_msg(msg, CMD_SWITCH_CHANNEL_OP, true, true, true);

	/*
	 * switch channel related param
	 * channel, scan, bw, tx_rx_setting, extension channel
	 */

	value = 0;
	value |= SC_PARAM1_CHL(channel);
	value |= SC_PARAM1_SCAN(scan);
	value |= SC_PARAM1_BW(bw);
	value = cpu2le32(value);
	mt7612u_mcu_append_cmd_msg(msg, (char *)&value, 4);

	value = 0;
	value |= SC_PARAM2_TR_SETTING(tx_rx_setting);

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
		__func__, channel, force_mode, gain_from_e2p));

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_INIT_GAIN_OP, true, true, true);

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
		__func__, channel, mode, ext, rssi, false_cca));

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_DYNC_VGA_OP, true, true, true);

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
		__func__, led_idx, link_status));

	msg = mt7612u_mcu_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	mt7612u_mcu_init_cmd_msg(msg, CMD_LED_MODE_OP, false, false, false);

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

void mt7612u_mcu_usb_fw_init(struct rtmp_adapter *ad)
{
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __func__));

	mt7612u_write32(ad, HEADER_TRANS_CTRL_REG, 0x0);
	mt7612u_write32(ad, TSO_CTRL, 0x0);

	RT28XXDMAEnable(ad);
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	mt7612u_mcu_fun_set(ad, Q_SELECT, ad->chipCap.CmdRspRxRing);
	usb_rx_cmd_msgs_receive(ad);
	mt7612u_mcu_pwr_saving(ad, RADIO_ON, 0);
}

