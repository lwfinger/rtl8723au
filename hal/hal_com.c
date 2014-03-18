/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 ******************************************************************************/
#include <osdep_service.h>
#include <drv_types.h>

#include <hal_intf.h>
#include <hal_com.h>

#include <rtl8723a_hal.h>

#define _HAL_INIT_C_

void dump_chip_info(struct hal_version ChipVersion)
{
	int cnt = 0;
	u8 buf[128];

	cnt += sprintf((buf + cnt), "Chip Version Info: CHIP_8723A_");

	cnt += sprintf((buf + cnt), "%s_", IS_NORMAL_CHIP(ChipVersion) ?
		       "Normal_Chip" : "Test_Chip");
	cnt += sprintf((buf + cnt), "%s_",
		       IS_CHIP_VENDOR_TSMC(ChipVersion) ? "TSMC" : "UMC");
	if (IS_A_CUT(ChipVersion))
		cnt += sprintf((buf + cnt), "A_CUT_");
	else if (IS_B_CUT(ChipVersion))
		cnt += sprintf((buf + cnt), "B_CUT_");
	else if (IS_C_CUT(ChipVersion))
		cnt += sprintf((buf + cnt), "C_CUT_");
	else if (IS_D_CUT(ChipVersion))
		cnt += sprintf((buf + cnt), "D_CUT_");
	else if (IS_E_CUT(ChipVersion))
		cnt += sprintf((buf + cnt), "E_CUT_");
	else
		cnt += sprintf((buf + cnt), "UNKNOWN_CUT(%d)_",
			       ChipVersion.CUTVersion);

	if (IS_1T1R(ChipVersion))
		cnt += sprintf((buf + cnt), "1T1R_");
	else if (IS_1T2R(ChipVersion))
		cnt += sprintf((buf + cnt), "1T2R_");
	else if (IS_2T2R(ChipVersion))
		cnt += sprintf((buf + cnt), "2T2R_");
	else
		cnt += sprintf((buf + cnt), "UNKNOWN_RFTYPE(%d)_",
			       ChipVersion.RFType);

	cnt += sprintf((buf + cnt), "RomVer(%d)\n", ChipVersion.ROMVer);

	DBG_8723A("%s", buf);
}

#define	EEPROM_CHANNEL_PLAN_BY_HW_MASK	0x80

/* return the final channel plan decision */
/* hw_channel_plan:  channel plan from HW (efuse/eeprom) */
/* sw_channel_plan:  channel plan from SW (registry/module param) */
/* def_channel_plan: channel plan used when the former two is invalid */
u8 hal_com_get_channel_plan(struct rtw_adapter *padapter, u8 hw_channel_plan,
			    u8 sw_channel_plan, u8 def_channel_plan,
			    bool AutoLoadFail)
{
	u8 swConfig;
	u8 chnlPlan;

	swConfig = true;
	if (!AutoLoadFail) {
		if (!rtw_is_channel_plan_valid(sw_channel_plan))
			swConfig = false;
		if (hw_channel_plan & EEPROM_CHANNEL_PLAN_BY_HW_MASK)
			swConfig = false;
	}

	if (swConfig == true)
		chnlPlan = sw_channel_plan;
	else
		chnlPlan = hw_channel_plan & (~EEPROM_CHANNEL_PLAN_BY_HW_MASK);

	if (!rtw_is_channel_plan_valid(chnlPlan))
		chnlPlan = def_channel_plan;

	return chnlPlan;
}

u8 MRateToHwRate(u8 rate)
{
	u8 ret = DESC_RATE1M;

	switch (rate) {
		/*  CCK and OFDM non-HT rates */
	case IEEE80211_CCK_RATE_1MB:
		ret = DESC_RATE1M;
		break;
	case IEEE80211_CCK_RATE_2MB:
		ret = DESC_RATE2M;
		break;
	case IEEE80211_CCK_RATE_5MB:
		ret = DESC_RATE5_5M;
		break;
	case IEEE80211_CCK_RATE_11MB:
		ret = DESC_RATE11M;
		break;
	case IEEE80211_OFDM_RATE_6MB:
		ret = DESC_RATE6M;
		break;
	case IEEE80211_OFDM_RATE_9MB:
		ret = DESC_RATE9M;
		break;
	case IEEE80211_OFDM_RATE_12MB:
		ret = DESC_RATE12M;
		break;
	case IEEE80211_OFDM_RATE_18MB:
		ret = DESC_RATE18M;
		break;
	case IEEE80211_OFDM_RATE_24MB:
		ret = DESC_RATE24M;
		break;
	case IEEE80211_OFDM_RATE_36MB:
		ret = DESC_RATE36M;
		break;
	case IEEE80211_OFDM_RATE_48MB:
		ret = DESC_RATE48M;
		break;
	case IEEE80211_OFDM_RATE_54MB:
		ret = DESC_RATE54M;
		break;

		/*  HT rates since here */
		/* case MGN_MCS0:	ret = DESC_RATEMCS0;    break; */
		/* case MGN_MCS1:	ret = DESC_RATEMCS1;    break; */
		/* case MGN_MCS2:	ret = DESC_RATEMCS2;    break; */
		/* case MGN_MCS3:	ret = DESC_RATEMCS3;    break; */
		/* case MGN_MCS4:	ret = DESC_RATEMCS4;    break; */
		/* case MGN_MCS5:	ret = DESC_RATEMCS5;    break; */
		/* case MGN_MCS6:	ret = DESC_RATEMCS6;    break; */
		/* case MGN_MCS7:	ret = DESC_RATEMCS7;    break; */

	default:
		break;
	}

	return ret;
}

void HalSetBrateCfg(struct rtw_adapter *padapter, u8 *mBratesOS)
{
	struct hal_data_8723a *pHalData = GET_HAL_DATA(padapter);
	u8 i, is_brate, brate;
	u16 brate_cfg = 0;
	u8 rate_index;

	for (i = 0; i < NDIS_802_11_LENGTH_RATES_EX; i++) {
		is_brate = mBratesOS[i] & IEEE80211_BASIC_RATE_MASK;
		brate = mBratesOS[i] & 0x7f;

		if (is_brate) {
			switch (brate) {
			case IEEE80211_CCK_RATE_1MB:
				brate_cfg |= RATE_1M;
				break;
			case IEEE80211_CCK_RATE_2MB:
				brate_cfg |= RATE_2M;
				break;
			case IEEE80211_CCK_RATE_5MB:
				brate_cfg |= RATE_5_5M;
				break;
			case IEEE80211_CCK_RATE_11MB:
				brate_cfg |= RATE_11M;
				break;
			case IEEE80211_OFDM_RATE_6MB:
				brate_cfg |= RATE_6M;
				break;
			case IEEE80211_OFDM_RATE_9MB:
				brate_cfg |= RATE_9M;
				break;
			case IEEE80211_OFDM_RATE_12MB:
				brate_cfg |= RATE_12M;
				break;
			case IEEE80211_OFDM_RATE_18MB:
				brate_cfg |= RATE_18M;
				break;
			case IEEE80211_OFDM_RATE_24MB:
				brate_cfg |= RATE_24M;
				break;
			case IEEE80211_OFDM_RATE_36MB:
				brate_cfg |= RATE_36M;
				break;
			case IEEE80211_OFDM_RATE_48MB:
				brate_cfg |= RATE_48M;
				break;
			case IEEE80211_OFDM_RATE_54MB:
				brate_cfg |= RATE_54M;
				break;
			}
		}
	}

	/*  2007.01.16, by Emily */
	/*  Select RRSR (in Legacy-OFDM and CCK) */
	/*  For 8190, we select only 24M, 12M, 6M, 11M, 5.5M, 2M,
	    and 1M from the Basic rate. */
	/*  We do not use other rates. */
	/* 2011.03.30 add by Luke Lee */
	/* CCK 2M ACK should be disabled for some BCM and Atheros AP IOT */
	/* because CCK 2M has poor TXEVM */
	/* CCK 5.5M & 11M ACK should be enabled for better
	   performance */

	brate_cfg = (brate_cfg | 0xd) & 0x15d;
	pHalData->BasicRateSet = brate_cfg;
	brate_cfg |= 0x01;	/*  default enable 1M ACK rate */
	DBG_8723A("HW_VAR_BASIC_RATE: BrateCfg(%#x)\n", brate_cfg);

	/*  Set RRSR rate table. */
	rtw_write8(padapter, REG_RRSR, brate_cfg & 0xff);
	rtw_write8(padapter, REG_RRSR + 1, (brate_cfg >> 8) & 0xff);
	rtw_write8(padapter, REG_RRSR + 2,
		   rtw_read8(padapter, REG_RRSR + 2) & 0xf0);

	rate_index = 0;
	/*  Set RTS initial rate */
	while (brate_cfg > 0x1) {
		brate_cfg = (brate_cfg >> 1);
		rate_index++;
	}
		/*  Ziv - Check */
	rtw_write8(padapter, REG_INIRTS_RATE_SEL, rate_index);

	return;
}

static void _OneOutPipeMapping(struct rtw_adapter *pAdapter)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(pAdapter);

	pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0];	/* VO */
	pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[0];	/* VI */
	pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[0];	/* BE */
	pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[0];	/* BK */

	pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];	/* BCN */
	pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];	/* MGT */
	pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0];	/* HIGH */
	pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];	/* TXCMD */
}

static void _TwoOutPipeMapping(struct rtw_adapter *pAdapter, bool bWIFICfg)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(pAdapter);

	if (bWIFICfg) {		/* WMM */
		/*    BK,   BE,   VI,   VO,   BCN,  CMD,  MGT, HIGH, HCCA */
		/*     0,    1,    0,    1,     0,    0,    0,    0,    0 }; */
		/* 0:H, 1:L */
		pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[1]; /* VO */
		pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[0]; /* VI */
		pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[1]; /* BE */
		pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[0]; /* BK */

		pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0]; /* BCN */
		pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0]; /* MGT */
		pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0]; /* HIGH */
		pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0]; /* TXCMD*/
	} else {		/* typical setting */
		/*    BK,   BE,   VI,   VO,   BCN,  CMD,  MGT, HIGH, HCCA */
		/*     1,    1,    0,    0,     0,    0,    0,    0,    0 }; */
		/* 0:H, 1:L */
		pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0]; /* VO */
		pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[0]; /* VI */
		pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[1]; /* BE */
		pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[1]; /* BK */

		pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0]; /* BCN */
		pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0]; /* MGT */
		pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0]; /* HIGH */
		pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0]; /* TXCMD*/
	}
}

static void _ThreeOutPipeMapping(struct rtw_adapter *pAdapter, bool bWIFICfg)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(pAdapter);

	if (bWIFICfg) {		/* for WMM */
		/*    BK,   BE,   VI,   VO,   BCN,  CMD,  MGT, HIGH, HCCA */
		/*     1,    2,    1,    0,     0,    0,    0,    0,    0 }; */
		/* 0:H, 1:N, 2:L */
		pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0]; /* VO */
		pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[1]; /* VI */
		pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[2]; /* BE */
		pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[1]; /* BK */

		pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0]; /* BCN */
		pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0]; /* MGT */
		pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0]; /* HIGH */
		pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0]; /* TXCMD*/
	} else {		/* typical setting */
		/*    BK,   BE,   VI,   VO,   BCN,  CMD,  MGT, HIGH, HCCA */
		/*     2,    2,    1,    0,     0,    0,    0,    0,    0 }; */
		/* 0:H, 1:N, 2:L */
		pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0]; /* VO */
		pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[1]; /* VI */
		pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[2]; /* BE */
		pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[2]; /* BK */

		pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0]; /* BCN */
		pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0]; /* MGT */
		pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0]; /* HIGH */
		pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0]; /* TXCMD*/
	}
}

bool Hal_MappingOutPipe(struct rtw_adapter *pAdapter, u8 NumOutPipe)
{
	struct registry_priv *pregistrypriv = &pAdapter->registrypriv;

	bool bWIFICfg = (pregistrypriv->wifi_spec) ? true : false;

	bool result = true;

	switch (NumOutPipe) {
	case 2:
		_TwoOutPipeMapping(pAdapter, bWIFICfg);
		break;
	case 3:
		_ThreeOutPipeMapping(pAdapter, bWIFICfg);
		break;
	case 1:
		_OneOutPipeMapping(pAdapter);
		break;
	default:
		result = false;
		break;
	}

	return result;
}

void hal_init_macaddr(struct rtw_adapter *adapter)
{
	rtw_hal_set_hwreg(adapter, HW_VAR_MAC_ADDR,
			  adapter->eeprompriv.mac_addr);
}

/*
* C2H event format:
* Field	 TRIGGER		CONTENT	   CMD_SEQ	CMD_LEN		 CMD_ID
* BITS	 [127:120]	[119:16]      [15:8]		  [7:4]		   [3:0]
*/

void c2h_evt_clear(struct rtw_adapter *adapter)
{
	rtw_write8(adapter, REG_C2HEVT_CLEAR, C2H_EVT_HOST_CLOSE);
}

s32 c2h_evt_read(struct rtw_adapter *adapter, u8 * buf)
{
	s32 ret = _FAIL;
	struct c2h_evt_hdr *c2h_evt;
	int i;
	u8 trigger;

	if (buf == NULL)
		goto exit;

	trigger = rtw_read8(adapter, REG_C2HEVT_CLEAR);

	if (trigger == C2H_EVT_HOST_CLOSE) {
		goto exit;	/* Not ready */
	} else if (trigger != C2H_EVT_FW_CLOSE) {
		goto clear_evt;	/* Not a valid value */
	}

	c2h_evt = (struct c2h_evt_hdr *)buf;

	memset(c2h_evt, 0, 16);

	*buf = rtw_read8(adapter, REG_C2HEVT_MSG_NORMAL);
	*(buf + 1) = rtw_read8(adapter, REG_C2HEVT_MSG_NORMAL + 1);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_info_, "c2h_evt_read(): ",
		      &c2h_evt, sizeof(c2h_evt));

	if (0) {
		DBG_8723A("%s id:%u, len:%u, seq:%u, trigger:0x%02x\n",
			  __func__, c2h_evt->id, c2h_evt->plen, c2h_evt->seq,
			  trigger);
	}

	/* Read the content */
	for (i = 0; i < c2h_evt->plen; i++)
		c2h_evt->payload[i] = rtw_read8(adapter,
						REG_C2HEVT_MSG_NORMAL +
						sizeof(*c2h_evt) + i);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_info_,
		      "c2h_evt_read(): Command Content:\n", c2h_evt->payload,
		      c2h_evt->plen);

	ret = _SUCCESS;

clear_evt:
	/*
	 * Clear event to notify FW we have read the command.
	 * If this field isn't clear, the FW won't update the
	 * next command message.
	 */
	c2h_evt_clear(adapter);
exit:
	return ret;
}

void
rtl8723a_set_ampdu_min_space(struct rtw_adapter *padapter, u8 MinSpacingToSet)
{
	u8 SecMinSpace;

	if (MinSpacingToSet <= 7) {
		switch (padapter->securitypriv.dot11PrivacyAlgrthm) {
		case _NO_PRIVACY_:
		case _AES_:
			SecMinSpace = 0;
			break;

		case _WEP40_:
		case _WEP104_:
		case _TKIP_:
		case _TKIP_WTMIC_:
			SecMinSpace = 6;
			break;
		default:
			SecMinSpace = 7;
			break;
		}

		if (MinSpacingToSet < SecMinSpace)
			MinSpacingToSet = SecMinSpace;

		/* RT_TRACE(COMP_MLME, DBG_LOUD,
		   ("Set HW_VAR_AMPDU_MIN_SPACE: %#x\n",
		   padapter->MgntInfo.MinSpaceCfg)); */
		MinSpacingToSet |=
			rtw_read8(padapter, REG_AMPDU_MIN_SPACE) & 0xf8;
		rtw_write8(padapter, REG_AMPDU_MIN_SPACE,
			   MinSpacingToSet);
	}
}
