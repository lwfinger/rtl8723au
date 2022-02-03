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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

//============================================================
// include files
//============================================================

#include "odm_precomp.h"



const u16 dB_Invert_Table[8][12] = {
	{	1,		1,		1,		2,		2,		2,		2,		3,		3,		3,		4,		4},
	{	4,		5,		6,		6,		7,		8,		9,		10,		11,		13,		14,		16},
	{	18,		20,		22,		25,		28,		32,		35,		40,		45,		50,		56,		63},
	{	71,		79,		89,		100,	112,	126,	141,	158,	178,	200,	224,	251},
	{	282,	316,	355,	398,	447,	501,	562,	631,	708,	794,	891,	1000},
	{	1122,	1259,	1413,	1585,	1778,	1995,	2239,	2512,	2818,	3162,	3548,	3981},
	{	4467,	5012,	5623,	6310,	7079,	7943,	8913,	10000,	11220,	12589,	14125,	15849},
	{	17783,	19953,	22387,	25119,	28184,	31623,	35481,	39811,	44668,	50119,	56234,	65535}};

// 20100515 Joseph: Add global variable to keep temporary scan list for antenna switching test.
//u8			tmpNumBssDesc;
//RT_WLAN_BSS	tmpbssDesc[MAX_BSS_DESC];


//avoid to warn in FreeBSD ==> To DO modify
u32 EDCAParam[HT_IOT_PEER_MAX][3] =
{          // UL			DL
	{0x5ea42b, 0x5ea42b, 0x5ea42b}, //0:unknown AP
	{0xa44f, 0x5ea44f, 0x5e431c}, // 1:realtek AP
	{0x5ea42b, 0x5ea42b, 0x5ea42b}, // 2:unknown AP => realtek_92SE
	{0x5ea32b, 0x5ea42b, 0x5e4322}, // 3:broadcom AP
	{0x5ea422, 0x00a44f, 0x00a44f}, // 4:ralink AP
	{0x5ea322, 0x00a630, 0x00a44f}, // 5:atheros AP
	//{0x5ea42b, 0x5ea42b, 0x5ea42b},// 6:cisco AP
	{0x5e4322, 0x5e4322, 0x5e4322},// 6:cisco AP
	//{0x3ea430, 0x00a630, 0x3ea44f}, // 7:cisco AP
	{0x5ea44f, 0x00a44f, 0x5ea42b}, // 8:marvell AP
	//{0x5ea44f, 0x5ea44f, 0x5ea44f}, // 9realtek AP
	{0x5ea42b, 0x5ea42b, 0x5ea42b}, // 10:unknown AP=> 92U AP
	{0x5ea42b, 0xa630, 0x5e431c}, // 11:airgocap AP
//	{0x5e4322, 0x00a44f, 0x5ea44f}, // 12:unknown AP
};
//============================================================
// EDCA Paramter for AP/ADSL   by Mingzhi 2011-11-22
//============================================================

//============================================================
// Global var
//============================================================
u32 OFDMSwingTable[OFDM_TABLE_SIZE_92D] = {
	0x7f8001fe, // 0, +6.0dB
	0x788001e2, // 1, +5.5dB
	0x71c001c7, // 2, +5.0dB
	0x6b8001ae, // 3, +4.5dB
	0x65400195, // 4, +4.0dB
	0x5fc0017f, // 5, +3.5dB
	0x5a400169, // 6, +3.0dB
	0x55400155, // 7, +2.5dB
	0x50800142, // 8, +2.0dB
	0x4c000130, // 9, +1.5dB
	0x47c0011f, // 10, +1.0dB
	0x43c0010f, // 11, +0.5dB
	0x40000100, // 12, +0dB
	0x3c8000f2, // 13, -0.5dB
	0x390000e4, // 14, -1.0dB
	0x35c000d7, // 15, -1.5dB
	0x32c000cb, // 16, -2.0dB
	0x300000c0, // 17, -2.5dB
	0x2d4000b5, // 18, -3.0dB
	0x2ac000ab, // 19, -3.5dB
	0x288000a2, // 20, -4.0dB
	0x26000098, // 21, -4.5dB
	0x24000090, // 22, -5.0dB
	0x22000088, // 23, -5.5dB
	0x20000080, // 24, -6.0dB
	0x1e400079, // 25, -6.5dB
	0x1c800072, // 26, -7.0dB
	0x1b00006c, // 27. -7.5dB
	0x19800066, // 28, -8.0dB
	0x18000060, // 29, -8.5dB
	0x16c0005b, // 30, -9.0dB
	0x15800056, // 31, -9.5dB
	0x14400051, // 32, -10.0dB
	0x1300004c, // 33, -10.5dB
	0x12000048, // 34, -11.0dB
	0x11000044, // 35, -11.5dB
	0x10000040, // 36, -12.0dB
	0x0f00003c,// 37, -12.5dB
	0x0e400039,// 38, -13.0dB
	0x0d800036,// 39, -13.5dB
	0x0cc00033,// 40, -14.0dB
	0x0c000030,// 41, -14.5dB
	0x0b40002d,// 42, -15.0dB
};


u8 CCKSwingTable_Ch1_Ch13[CCK_TABLE_SIZE][8] = {
	{0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04},	// 0, +0dB
	{0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},	// 1, -0.5dB
	{0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},	// 2, -1.0dB
	{0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},	// 3, -1.5dB
	{0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},	// 4, -2.0dB
	{0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},	// 5, -2.5dB
	{0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},	// 6, -3.0dB
	{0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},	// 7, -3.5dB
	{0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},	// 8, -4.0dB
	{0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},	// 9, -4.5dB
	{0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},	// 10, -5.0dB
	{0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},	// 11, -5.5dB
	{0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},	// 12, -6.0dB
	{0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},	// 13, -6.5dB
	{0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},	// 14, -7.0dB
	{0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},	// 15, -7.5dB
	{0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},	// 16, -8.0dB
	{0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},	// 17, -8.5dB
	{0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},	// 18, -9.0dB
	{0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 19, -9.5dB
	{0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 20, -10.0dB
	{0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 21, -10.5dB
	{0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 22, -11.0dB
	{0x0e, 0x0e, 0x0c, 0x0a, 0x08, 0x05, 0x02, 0x01},	// 23, -11.5dB
	{0x0d, 0x0d, 0x0c, 0x0a, 0x07, 0x05, 0x02, 0x01},	// 24, -12.0dB
	{0x0d, 0x0c, 0x0b, 0x09, 0x07, 0x04, 0x02, 0x01},	// 25, -12.5dB
	{0x0c, 0x0c, 0x0a, 0x09, 0x06, 0x04, 0x02, 0x01},	// 26, -13.0dB
	{0x0b, 0x0b, 0x0a, 0x08, 0x06, 0x04, 0x02, 0x01},	// 27, -13.5dB
	{0x0b, 0x0a, 0x09, 0x08, 0x06, 0x04, 0x02, 0x01},	// 28, -14.0dB
	{0x0a, 0x0a, 0x09, 0x07, 0x05, 0x03, 0x02, 0x01},	// 29, -14.5dB
	{0x0a, 0x09, 0x08, 0x07, 0x05, 0x03, 0x02, 0x01},	// 30, -15.0dB
	{0x09, 0x09, 0x08, 0x06, 0x05, 0x03, 0x01, 0x01},	// 31, -15.5dB
	{0x09, 0x08, 0x07, 0x06, 0x04, 0x03, 0x01, 0x01}	// 32, -16.0dB
};


u8 CCKSwingTable_Ch14 [CCK_TABLE_SIZE][8]= {
	{0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00},	// 0, +0dB
	{0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},	// 1, -0.5dB
	{0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},	// 2, -1.0dB
	{0x2d, 0x2d, 0x17, 0x17, 0x00, 0x00, 0x00, 0x00},	// 3, -1.5dB
	{0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},	// 4, -2.0dB
	{0x28, 0x28, 0x24, 0x14, 0x00, 0x00, 0x00, 0x00},	// 5, -2.5dB
	{0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},	// 6, -3.0dB
	{0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},	// 7, -3.5dB
	{0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},	// 8, -4.0dB
	{0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},	// 9, -4.5dB
	{0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},	// 10, -5.0dB
	{0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 11, -5.5dB
	{0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 12, -6.0dB
	{0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},	// 13, -6.5dB
	{0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},	// 14, -7.0dB
	{0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 15, -7.5dB
	{0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 16, -8.0dB
	{0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 17, -8.5dB
	{0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 18, -9.0dB
	{0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 19, -9.5dB
	{0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 20, -10.0dB
	{0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},	// 21, -10.5dB
	{0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00},	// 22, -11.0dB
	{0x0e, 0x0e, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	// 23, -11.5dB
	{0x0d, 0x0d, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	// 24, -12.0dB
	{0x0d, 0x0c, 0x0b, 0x06, 0x00, 0x00, 0x00, 0x00},	// 25, -12.5dB
	{0x0c, 0x0c, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	// 26, -13.0dB
	{0x0b, 0x0b, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	// 27, -13.5dB
	{0x0b, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	// 28, -14.0dB
	{0x0a, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	// 29, -14.5dB
	{0x0a, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	// 30, -15.0dB
	{0x09, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	// 31, -15.5dB
	{0x09, 0x08, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00}	// 32, -16.0dB
};


#ifdef AP_BUILD_WORKAROUND

unsigned int TxPwrTrk_OFDM_SwingTbl[TxPwrTrk_OFDM_SwingTbl_Len] = {
	/*  +6.0dB */ 0x7f8001fe,
	/*  +5.5dB */ 0x788001e2,
	/*  +5.0dB */ 0x71c001c7,
	/*  +4.5dB */ 0x6b8001ae,
	/*  +4.0dB */ 0x65400195,
	/*  +3.5dB */ 0x5fc0017f,
	/*  +3.0dB */ 0x5a400169,
	/*  +2.5dB */ 0x55400155,
	/*  +2.0dB */ 0x50800142,
	/*  +1.5dB */ 0x4c000130,
	/*  +1.0dB */ 0x47c0011f,
	/*  +0.5dB */ 0x43c0010f,
	/*   0.0dB */ 0x40000100,
	/*  -0.5dB */ 0x3c8000f2,
	/*  -1.0dB */ 0x390000e4,
	/*  -1.5dB */ 0x35c000d7,
	/*  -2.0dB */ 0x32c000cb,
	/*  -2.5dB */ 0x300000c0,
	/*  -3.0dB */ 0x2d4000b5,
	/*  -3.5dB */ 0x2ac000ab,
	/*  -4.0dB */ 0x288000a2,
	/*  -4.5dB */ 0x26000098,
	/*  -5.0dB */ 0x24000090,
	/*  -5.5dB */ 0x22000088,
	/*  -6.0dB */ 0x20000080,
	/*  -6.5dB */ 0x1a00006c,
	/*  -7.0dB */ 0x1c800072,
	/*  -7.5dB */ 0x18000060,
	/*  -8.0dB */ 0x19800066,
	/*  -8.5dB */ 0x15800056,
	/*  -9.0dB */ 0x26c0005b,
	/*  -9.5dB */ 0x14400051,
	/* -10.0dB */ 0x24400051,
	/* -10.5dB */ 0x1300004c,
	/* -11.0dB */ 0x12000048,
	/* -11.5dB */ 0x11000044,
	/* -12.0dB */ 0x10000040
};
#endif

//============================================================
// Local Function predefine.
//============================================================

//START------------COMMON INFO RELATED---------------//
void
odm_CommonInfoSelfInit(
		PDM_ODM_T		pDM_Odm
	);

void
odm_CommonInfoSelfUpdate(
		PDM_ODM_T		pDM_Odm
	);

void
odm_CmnInfoInit_Debug(
		PDM_ODM_T		pDM_Odm
	);

void
odm_CmnInfoHook_Debug(
		PDM_ODM_T		pDM_Odm
	);

void
odm_CmnInfoUpdate_Debug(
		PDM_ODM_T		pDM_Odm
	);
/*
void
odm_FindMinimumRSSI(
		PDM_ODM_T		pDM_Odm
	);

void
odm_IsLinked(
		PDM_ODM_T		pDM_Odm
	);
*/
//END------------COMMON INFO RELATED---------------//

//START---------------DIG---------------------------//
void
odm_FalseAlarmCounterStatistics(
		PDM_ODM_T		pDM_Odm
	);

void
odm_DIGInit(
		PDM_ODM_T		pDM_Odm
	);

void
odm_DIG(
		PDM_ODM_T		pDM_Odm
	);

void
odm_CCKPacketDetectionThresh(
		PDM_ODM_T		pDM_Odm
	);
//END---------------DIG---------------------------//

//START-------BB POWER SAVE-----------------------//
void
odm_DynamicBBPowerSavingInit(
		PDM_ODM_T		pDM_Odm
	);

void
odm_DynamicBBPowerSaving(
		PDM_ODM_T		pDM_Odm
	);

void
odm_1R_CCA(
		PDM_ODM_T		pDM_Odm
	);
//END---------BB POWER SAVE-----------------------//


void
odm_RefreshRateAdaptiveMaskMP(
		PDM_ODM_T		pDM_Odm
	);

void
odm_RefreshRateAdaptiveMaskCE(
		PDM_ODM_T		pDM_Odm
	);

void
odm_RefreshRateAdaptiveMaskAPADSL(
		PDM_ODM_T		pDM_Odm
	);

void
odm_DynamicTxPowerInit(
		PDM_ODM_T		pDM_Odm
	);

void
odm_DynamicTxPowerRestorePowerIndex(
	PDM_ODM_T	pDM_Odm
	);

void
odm_DynamicTxPowerNIC(
	PDM_ODM_T	pDM_Odm
	);

void
odm_DynamicTxPowerSavePowerIndex(
		PDM_ODM_T		pDM_Odm
	);

void
odm_DynamicTxPowerWritePowerIndex(
	PDM_ODM_T	pDM_Odm,
	u8		Value);

void
odm_DynamicTxPower_92C(
	PDM_ODM_T	pDM_Odm
	);

void
odm_DynamicTxPower_92D(
	PDM_ODM_T	pDM_Odm
	);

void
odm_RSSIMonitorInit(
	PDM_ODM_T	pDM_Odm
	);

void
odm_RSSIMonitorCheckMP(
	PDM_ODM_T	pDM_Odm
	);

void
odm_RSSIMonitorCheckCE(
		PDM_ODM_T		pDM_Odm
	);
void
odm_RSSIMonitorCheckAP(
		PDM_ODM_T		pDM_Odm
	);



void
odm_RSSIMonitorCheck(
		PDM_ODM_T		pDM_Odm
	);
void
odm_DynamicTxPower(
		PDM_ODM_T		pDM_Odm
	);

void
odm_DynamicTxPowerAP(
		PDM_ODM_T		pDM_Odm
	);


void
odm_SwAntDivInit(
		PDM_ODM_T		pDM_Odm
	);

void
odm_SwAntDivInit_NIC(
		PDM_ODM_T		pDM_Odm
	);

void
odm_SwAntDivChkAntSwitch(
		PDM_ODM_T		pDM_Odm,
		u8			Step
	);

void
odm_SwAntDivChkAntSwitchNIC(
		PDM_ODM_T		pDM_Odm,
		u8		Step
	);


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void odm_SwAntDivChkAntSwitchCallback(void *FunctionContext);
#else
void odm_SwAntDivChkAntSwitchCallback(struct timer_list *t);
#endif

void
odm_GlobalAdapterCheck(
		void
	);

void
odm_RefreshRateAdaptiveMask(
		PDM_ODM_T		pDM_Odm
	);

void
ODM_TXPowerTrackingCheck(
		PDM_ODM_T		pDM_Odm
	);

void
odm_TXPowerTrackingCheckAP(
		PDM_ODM_T		pDM_Odm
	);







void
odm_RateAdaptiveMaskInit(
	PDM_ODM_T	pDM_Odm
	);

void
odm_TXPowerTrackingThermalMeterInit(
	PDM_ODM_T	pDM_Odm
	);


void
odm_TXPowerTrackingInit(
	PDM_ODM_T	pDM_Odm
	);

void
odm_TXPowerTrackingCheckMP(
	PDM_ODM_T	pDM_Odm
	);


void
odm_TXPowerTrackingCheckCE(
	PDM_ODM_T	pDM_Odm
	);

void
odm_EdcaTurboCheck(
		PDM_ODM_T		pDM_Odm
	);
void
ODM_EdcaTurboInit(
	PDM_ODM_T		pDM_Odm
);

void
odm_EdcaTurboCheckCE(
		PDM_ODM_T		pDM_Odm
	);

#define		RxDefaultAnt1		0x65a9
#define	RxDefaultAnt2		0x569a

void
odm_InitHybridAntDiv(
 PDM_ODM_T	pDM_Odm
	);

bool
odm_StaDefAntSel(
 PDM_ODM_T	pDM_Odm,
 u32		OFDM_Ant1_Cnt,
 u32		OFDM_Ant2_Cnt,
 u32		CCK_Ant1_Cnt,
 u32		CCK_Ant2_Cnt,
 u8		*pDefAnt
	);

void
odm_SetRxIdleAnt(
	PDM_ODM_T	pDM_Odm,
	u8	Ant,
   bool   bDualPath
);



void
odm_HwAntDiv(
	PDM_ODM_T	pDM_Odm
);


//============================================================
//3 Export Interface
//============================================================

//
// 2011/09/21 MH Add to describe different team necessary resource allocate??
//
void
ODM_DMInit(
		PDM_ODM_T		pDM_Odm
	)
{

#if (FPGA_TWO_MAC_VERIFICATION == 1)
	odm_RateAdaptiveMaskInit(pDM_Odm);
	return;
#endif

	//2012.05.03 Luke: For all IC series
	odm_CommonInfoSelfInit(pDM_Odm);
	odm_CmnInfoInit_Debug(pDM_Odm);
	odm_DIGInit(pDM_Odm);
	odm_RateAdaptiveMaskInit(pDM_Odm);

	if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
	{

	}
	else if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
	{
		#if (RTL8188E_SUPPORT == 1)
		odm_PrimaryCCA_Init(pDM_Odm);    // Gary
		#endif
		odm_DynamicBBPowerSavingInit(pDM_Odm);
		odm_DynamicTxPowerInit(pDM_Odm);
		odm_TXPowerTrackingInit(pDM_Odm);
		ODM_EdcaTurboInit(pDM_Odm);
		#if (RTL8188E_SUPPORT == 1)
		ODM_RAInfo_Init_all(pDM_Odm);
		#endif
		if(( pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV )	||
			( pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV )	||
			( pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV ))
		{
			odm_InitHybridAntDiv(pDM_Odm);
		}
		else if( pDM_Odm->AntDivType == CGCS_RX_SW_ANTDIV)
		{
			odm_SwAntDivInit(pDM_Odm);
		}
	}
}

//
// 2011/09/20 MH This is the entry pointer for all team to execute HW out source DM.
// You can not add any dummy function here, be care, you can only use DM structure
// to perform any new ODM_DM.
//
void
ODM_DMWatchdog(
		PDM_ODM_T		pDM_Odm
	)
{
	//2012.05.03 Luke: For all IC series
	odm_GlobalAdapterCheck();
	odm_CmnInfoHook_Debug(pDM_Odm);
	odm_CmnInfoUpdate_Debug(pDM_Odm);
	odm_CommonInfoSelfUpdate(pDM_Odm);
	odm_FalseAlarmCounterStatistics(pDM_Odm);
	odm_RSSIMonitorCheck(pDM_Odm);

	//For CE Platform(SPRD or Tablet)
	//8723A or 8189ES platform
	//NeilChen--2012--08--24--
	//Fix Leave LPS issue
	if((pDM_Odm->Adapter->pwrctrlpriv.pwr_mode != PS_MODE_ACTIVE) &&// in LPS mode
		(
			(pDM_Odm->SupportICType & (ODM_RTL8723A ) )||
			(pDM_Odm->SupportICType & (ODM_RTL8188E) &&((pDM_Odm->SupportInterface  == ODM_ITRF_SDIO)) )

		//&&((pDM_Odm->SupportInterface  == ODM_ITRF_SDIO))
		)
	)
	{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("----Step1: odm_DIG is in LPS mode\n"));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("---Step2: 8723AS is in LPS mode\n"));
			odm_DIGbyRSSI_LPS(pDM_Odm);
	}
	else
	{
		odm_DIG(pDM_Odm);
	}


	odm_CCKPacketDetectionThresh(pDM_Odm);

	if(*(pDM_Odm->pbPowerSaving)==TRUE)
		return;

	odm_RefreshRateAdaptiveMask(pDM_Odm);

	#if (RTL8192D_SUPPORT == 1)
	ODM_DynamicEarlyMode(pDM_Odm);
	#endif
	odm_DynamicBBPowerSaving(pDM_Odm);
	#if (RTL8188E_SUPPORT == 1)
	odm_DynamicPrimaryCCA(pDM_Odm);
	#endif
	if(( pDM_Odm->AntDivType ==  CG_TRX_HW_ANTDIV )	||
		( pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV )	||
		( pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV ))
	{
		odm_HwAntDiv(pDM_Odm);
	}
	else if( pDM_Odm->AntDivType == CGCS_RX_SW_ANTDIV)
	{
		odm_SwAntDivChkAntSwitch(pDM_Odm, SWAW_STEP_PEAK);
	}

	if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
	{

	}
	else if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
	{
		ODM_TXPowerTrackingCheck(pDM_Odm);
	      odm_EdcaTurboCheck(pDM_Odm);
		odm_DynamicTxPower(pDM_Odm);
	}

	odm_dtc(pDM_Odm);
}


//
// Init /.. Fixed HW value. Only init time.
//
void
ODM_CmnInfoInit(
		PDM_ODM_T		pDM_Odm,
		ODM_CMNINFO_E	CmnInfo,
		u32			Value
	)
{
	//ODM_RT_TRACE(pDM_Odm,);

	//
	// This section is used for init value
	//
	switch	(CmnInfo)
	{
		//
		// Fixed ODM value.
		//
		case	ODM_CMNINFO_ABILITY:
			pDM_Odm->SupportAbility = (u32)Value;
			break;
		case	ODM_CMNINFO_PLATFORM:
			pDM_Odm->SupportPlatform = (u8)Value;
			break;

		case	ODM_CMNINFO_INTERFACE:
			pDM_Odm->SupportInterface = (u8)Value;
			break;

		case	ODM_CMNINFO_MP_TEST_CHIP:
			pDM_Odm->bIsMPChip= (u8)Value;
			break;

		case	ODM_CMNINFO_IC_TYPE:
			pDM_Odm->SupportICType = Value;
			break;

		case	ODM_CMNINFO_CUT_VER:
			pDM_Odm->CutVersion = (u8)Value;
			break;

		case	ODM_CMNINFO_FAB_VER:
			pDM_Odm->FabVersion = (u8)Value;
			break;

		case	ODM_CMNINFO_RF_TYPE:
			pDM_Odm->RFType = (u8)Value;
			break;

		case    ODM_CMNINFO_RF_ANTENNA_TYPE:
			pDM_Odm->AntDivType= (u8)Value;
			break;

		case	ODM_CMNINFO_BOARD_TYPE:
			pDM_Odm->BoardType = (u8)Value;
			break;

		case	ODM_CMNINFO_EXT_LNA:
			pDM_Odm->ExtLNA = (u8)Value;
			break;

		case	ODM_CMNINFO_EXT_PA:
			pDM_Odm->ExtPA = (u8)Value;
			break;

		case	ODM_CMNINFO_EXT_TRSW:
			pDM_Odm->ExtTRSW = (u8)Value;
			break;
		case	ODM_CMNINFO_PATCH_ID:
			pDM_Odm->PatchID = (u8)Value;
			break;
		case	ODM_CMNINFO_BINHCT_TEST:
			pDM_Odm->bInHctTest = (bool)Value;
			break;
		case	ODM_CMNINFO_BWIFI_TEST:
			pDM_Odm->bWIFITest = (bool)Value;
			break;

		case	ODM_CMNINFO_SMART_CONCURRENT:
			pDM_Odm->bDualMacSmartConcurrent = (bool )Value;
			break;

		//To remove the compiler warning, must add an empty default statement to handle the other values.
		default:
			//do nothing
			break;

	}

	//
	// Tx power tracking BB swing table.
	// The base index = 12. +((12-n)/2)dB 13~?? = decrease tx pwr by -((n-12)/2)dB
	//
	pDM_Odm->BbSwingIdxOfdm			= 12; // Set defalut value as index 12.
	pDM_Odm->BbSwingIdxOfdmCurrent	= 12;
	pDM_Odm->BbSwingFlagOfdm		= FALSE;

}


void
ODM_CmnInfoHook(
		PDM_ODM_T		pDM_Odm,
		ODM_CMNINFO_E	CmnInfo,
		void *			pValue
	)
{
	//
	// Hook call by reference pointer.
	//
	switch	(CmnInfo)
	{
		//
		// Dynamic call by reference pointer.
		//
		case	ODM_CMNINFO_MAC_PHY_MODE:
			pDM_Odm->pMacPhyMode = (u8 *)pValue;
			break;

		case	ODM_CMNINFO_TX_UNI:
			pDM_Odm->pNumTxBytesUnicast = (u64 *)pValue;
			break;

		case	ODM_CMNINFO_RX_UNI:
			pDM_Odm->pNumRxBytesUnicast = (u64 *)pValue;
			break;

		case	ODM_CMNINFO_WM_MODE:
			pDM_Odm->pWirelessMode = (u8 *)pValue;
			break;

		case	ODM_CMNINFO_BAND:
			pDM_Odm->pBandType = (u8 *)pValue;
			break;

		case	ODM_CMNINFO_SEC_CHNL_OFFSET:
			pDM_Odm->pSecChOffset = (u8 *)pValue;
			break;

		case	ODM_CMNINFO_SEC_MODE:
			pDM_Odm->pSecurity = (u8 *)pValue;
			break;

		case	ODM_CMNINFO_BW:
			pDM_Odm->pBandWidth = (u8 *)pValue;
			break;

		case	ODM_CMNINFO_CHNL:
			pDM_Odm->pChannel = (u8 *)pValue;
			break;

		case	ODM_CMNINFO_DMSP_GET_VALUE:
			pDM_Odm->pbGetValueFromOtherMac = (bool *)pValue;
			break;

		case	ODM_CMNINFO_BUDDY_ADAPTOR:
			pDM_Odm->pBuddyAdapter = (struct rtw_adapter **)pValue;
			break;

		case	ODM_CMNINFO_DMSP_IS_MASTER:
			pDM_Odm->pbMasterOfDMSP = (bool *)pValue;
			break;

		case	ODM_CMNINFO_SCAN:
			pDM_Odm->pbScanInProcess = (bool *)pValue;
			break;

		case	ODM_CMNINFO_POWER_SAVING:
			pDM_Odm->pbPowerSaving = (bool *)pValue;
			break;

		case	ODM_CMNINFO_ONE_PATH_CCA:
			pDM_Odm->pOnePathCCA = (u8 *)pValue;
			break;

		case	ODM_CMNINFO_DRV_STOP:
			pDM_Odm->pbDriverStopped =  (bool *)pValue;
			break;

		case	ODM_CMNINFO_PNP_IN:
			pDM_Odm->pbDriverIsGoingToPnpSetPowerSleep =  (bool *)pValue;
			break;

		case	ODM_CMNINFO_INIT_ON:
			pDM_Odm->pinit_adpt_in_progress =  (bool *)pValue;
			break;

		case	ODM_CMNINFO_ANT_TEST:
			pDM_Odm->pAntennaTest =  (u8 *)pValue;
			break;

		case	ODM_CMNINFO_NET_CLOSED:
			pDM_Odm->pbNet_closed = (bool *)pValue;
			break;

		//case	ODM_CMNINFO_BT_COEXIST:
		//	pDM_Odm->BTCoexist = (bool *)pValue;

		//case	ODM_CMNINFO_STA_STATUS:
			//pDM_Odm->pODM_StaInfo[] = (PSTA_INFO_T)pValue;
			//break;

		//case	ODM_CMNINFO_PHY_STATUS:
		//	pDM_Odm->pPhyInfo = (ODM_PHY_INFO *)pValue;
		//	break;

		//case	ODM_CMNINFO_MAC_STATUS:
		//	pDM_Odm->pMacInfo = (ODM_MAC_INFO *)pValue;
		//	break;
		//To remove the compiler warning, must add an empty default statement to handle the other values.
		default:
			//do nothing
			break;

	}

}


void
ODM_CmnInfoPtrArrayHook(
		PDM_ODM_T		pDM_Odm,
		ODM_CMNINFO_E	CmnInfo,
		u16			Index,
		void *			pValue
	)
{
	//
	// Hook call by reference pointer.
	//
	switch	(CmnInfo)
	{
		//
		// Dynamic call by reference pointer.
		//
		case	ODM_CMNINFO_STA_STATUS:
			pDM_Odm->pODM_StaInfo[Index] = (PSTA_INFO_T)pValue;
			break;
		//To remove the compiler warning, must add an empty default statement to handle the other values.
		default:
			//do nothing
			break;
	}

}


//
// Update Band/CHannel/.. The values are dynamic but non-per-packet.
//
void
ODM_CmnInfoUpdate(
		PDM_ODM_T		pDM_Odm,
		u32			CmnInfo,
		u64			Value
	)
{
	//
	// This init variable may be changed in run time.
	//
	switch	(CmnInfo)
	{
		case	ODM_CMNINFO_ABILITY:
			pDM_Odm->SupportAbility = (u32)Value;
			break;

		case	ODM_CMNINFO_RF_TYPE:
			pDM_Odm->RFType = (u8)Value;
			break;

		case	ODM_CMNINFO_WIFI_DIRECT:
			pDM_Odm->bWIFI_Direct = (bool)Value;
			break;

		case	ODM_CMNINFO_WIFI_DISPLAY:
			pDM_Odm->bWIFI_Display = (bool)Value;
			break;

		case	ODM_CMNINFO_LINK:
			pDM_Odm->bLinked = (bool)Value;
			break;

		case	ODM_CMNINFO_RSSI_MIN:
			pDM_Odm->RSSI_Min= (u8)Value;
			break;

		case	ODM_CMNINFO_DBG_COMP:
			pDM_Odm->DebugComponents = Value;
			break;

		case	ODM_CMNINFO_DBG_LEVEL:
			pDM_Odm->DebugLevel = (u32)Value;
			break;
		case	ODM_CMNINFO_RA_THRESHOLD_HIGH:
			pDM_Odm->RateAdaptive.HighRSSIThresh = (u8)Value;
			break;

		case	ODM_CMNINFO_RA_THRESHOLD_LOW:
			pDM_Odm->RateAdaptive.LowRSSIThresh = (u8)Value;
			break;
#if(BT_30_SUPPORT == 1)
		// The following is for BT HS mode and BT coexist mechanism.
		case ODM_CMNINFO_BT_DISABLED:
			pDM_Odm->bBtDisabled = (bool)Value;
			break;

		case	ODM_CMNINFO_BT_OPERATION:
			pDM_Odm->bBtHsOperation = (bool)Value;
			break;

		case ODM_CMNINFO_BT_DIG:
			pDM_Odm->btHsDigVal = (u8)Value;
			break;

		case	ODM_CMNINFO_BT_BUSY:
			pDM_Odm->bBtBusy = (bool)Value;
			break;

		case	ODM_CMNINFO_BT_DISABLE_EDCA:
			pDM_Odm->bBtDisableEdcaTurbo = (bool)Value;
			break;
#endif

	}


}

void
odm_CommonInfoSelfInit(
		PDM_ODM_T		pDM_Odm
	)
{
	pDM_Odm->bCckHighPower = (bool) ODM_GetBBReg(pDM_Odm, 0x824, BIT9);
	pDM_Odm->RFPathRxEnable = (u8) ODM_GetBBReg(pDM_Odm, 0xc04, 0x0F);
	if(pDM_Odm->SupportICType & (ODM_RTL8192C|ODM_RTL8192D))
	{
#if(defined(CONFIG_HW_ANTENNA_DIVERSITY))
		pDM_Odm->AntDivType = CG_TRX_HW_ANTDIV;
#elif (defined(CONFIG_SW_ANTENNA_DIVERSITY))
		pDM_Odm->AntDivType = CGCS_RX_SW_ANTDIV;
#endif
	}
	if(pDM_Odm->SupportICType & (ODM_RTL8723A))
		pDM_Odm->AntDivType = CGCS_RX_SW_ANTDIV;

	ODM_InitDebugSetting(pDM_Odm);
}

void
odm_CommonInfoSelfUpdate(
		PDM_ODM_T		pDM_Odm
	)
{
	u8	EntryCnt=0;
	u8	i;
	PSTA_INFO_T	pEntry;

	if(*(pDM_Odm->pBandWidth) == ODM_BW40M)
	{
		if(*(pDM_Odm->pSecChOffset) == 1)
			pDM_Odm->ControlChannel = *(pDM_Odm->pChannel) -2;
		else if(*(pDM_Odm->pSecChOffset) == 2)
			pDM_Odm->ControlChannel = *(pDM_Odm->pChannel) +2;
	}
	else
		pDM_Odm->ControlChannel = *(pDM_Odm->pChannel);

	for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
	{
		pEntry = pDM_Odm->pODM_StaInfo[i];
		if(IS_STA_VALID(pEntry))
			EntryCnt++;
	}
	if(EntryCnt == 1)
		pDM_Odm->bOneEntryOnly = TRUE;
	else
		pDM_Odm->bOneEntryOnly = FALSE;
}

void
odm_CmnInfoInit_Debug(
		PDM_ODM_T		pDM_Odm
	)
{
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_CmnInfoInit_Debug==>\n"));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportPlatform=%d\n",pDM_Odm->SupportPlatform) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportAbility=0x%x\n",pDM_Odm->SupportAbility) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportInterface=%d\n",pDM_Odm->SupportInterface) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportICType=0x%x\n",pDM_Odm->SupportICType) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("CutVersion=%d\n",pDM_Odm->CutVersion) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("FabVersion=%d\n",pDM_Odm->FabVersion) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("RFType=%d\n",pDM_Odm->RFType) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("BoardType=%d\n",pDM_Odm->BoardType) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("ExtLNA=%d\n",pDM_Odm->ExtLNA) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("ExtPA=%d\n",pDM_Odm->ExtPA) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("ExtTRSW=%d\n",pDM_Odm->ExtTRSW) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("PatchID=%d\n",pDM_Odm->PatchID) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bInHctTest=%d\n",pDM_Odm->bInHctTest) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bWIFITest=%d\n",pDM_Odm->bWIFITest) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bDualMacSmartConcurrent=%d\n",pDM_Odm->bDualMacSmartConcurrent) );

}

void
odm_CmnInfoHook_Debug(
		PDM_ODM_T		pDM_Odm
	)
{
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_CmnInfoHook_Debug==>\n"));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pNumTxBytesUnicast=%llu\n",*(pDM_Odm->pNumTxBytesUnicast)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pNumRxBytesUnicast=%llu\n",*(pDM_Odm->pNumRxBytesUnicast)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pWirelessMode=0x%x\n",*(pDM_Odm->pWirelessMode)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pSecChOffset=%d\n",*(pDM_Odm->pSecChOffset)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pSecurity=%d\n",*(pDM_Odm->pSecurity)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pBandWidth=%d\n",*(pDM_Odm->pBandWidth)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pChannel=%d\n",*(pDM_Odm->pChannel)) );

#if (RTL8192D_SUPPORT==1)
	if(pDM_Odm->pBandType)
	    ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pBandType=%d\n",*(pDM_Odm->pBandType)) );
	if(pDM_Odm->pMacPhyMode)
	    ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pMacPhyMode=%d\n",*(pDM_Odm->pMacPhyMode)) );
	if(pDM_Odm->pBuddyAdapter)
	    ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pbGetValueFromOtherMac=%d\n",*(pDM_Odm->pbGetValueFromOtherMac)) );
	if(pDM_Odm->pBuddyAdapter)
	    ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pBuddyAdapter=%p\n",*(pDM_Odm->pBuddyAdapter)) );
	if(pDM_Odm->pbMasterOfDMSP)
	    ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pbMasterOfDMSP=%d\n",*(pDM_Odm->pbMasterOfDMSP)) );
#endif
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pbScanInProcess=%d\n",*(pDM_Odm->pbScanInProcess)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pbPowerSaving=%d\n",*(pDM_Odm->pbPowerSaving)) );

	if(pDM_Odm->SupportPlatform & (ODM_AP|ODM_ADSL))
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pOnePathCCA=%d\n",*(pDM_Odm->pOnePathCCA)) );
}

void
odm_CmnInfoUpdate_Debug(
		PDM_ODM_T		pDM_Odm
	)
{
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_CmnInfoUpdate_Debug==>\n"));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bWIFI_Direct=%d\n",pDM_Odm->bWIFI_Direct) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bWIFI_Display=%d\n",pDM_Odm->bWIFI_Display) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bLinked=%d\n",pDM_Odm->bLinked) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("RSSI_Min=%d\n",pDM_Odm->RSSI_Min) );
}

//3============================================================
//3 DIG
//3============================================================
/*-----------------------------------------------------------------------------
 * Function:	odm_DIGInit()
 *
 * Overview:	Set DIG scheme init value.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *
 *---------------------------------------------------------------------------*/
void
ODM_ChangeDynamicInitGainThresh(
	PDM_ODM_T	pDM_Odm,
	u32		DM_Type,
	u32		DM_Value
	)
{
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;

	if (DM_Type == DIG_TYPE_THRESH_HIGH)
	{
		pDM_DigTable->RssiHighThresh = DM_Value;
	}
	else if (DM_Type == DIG_TYPE_THRESH_LOW)
	{
		pDM_DigTable->RssiLowThresh = DM_Value;
	}
	else if (DM_Type == DIG_TYPE_ENABLE)
	{
		pDM_DigTable->Dig_Enable_Flag	= TRUE;
	}
	else if (DM_Type == DIG_TYPE_DISABLE)
	{
		pDM_DigTable->Dig_Enable_Flag = FALSE;
	}
	else if (DM_Type == DIG_TYPE_BACKOFF)
	{
		if(DM_Value > 30)
			DM_Value = 30;
		pDM_DigTable->BackoffVal = (u8)DM_Value;
	}
	else if(DM_Type == DIG_TYPE_RX_GAIN_MIN)
	{
		if(DM_Value == 0)
			DM_Value = 0x1;
		pDM_DigTable->rx_gain_range_min = (u8)DM_Value;
	}
	else if(DM_Type == DIG_TYPE_RX_GAIN_MAX)
	{
		if(DM_Value > 0x50)
			DM_Value = 0x50;
		pDM_DigTable->rx_gain_range_max = (u8)DM_Value;
	}
}	/* DM_ChangeDynamicInitGainThresh */

int getIGIForDiff(int value_IGI)
{
	#define ONERCCA_LOW_TH		0x30
	#define ONERCCA_LOW_DIFF	8

	if (value_IGI < ONERCCA_LOW_TH) {
		if ((ONERCCA_LOW_TH - value_IGI) < ONERCCA_LOW_DIFF)
			return ONERCCA_LOW_TH;
		else
			return value_IGI + ONERCCA_LOW_DIFF;
	} else {
		return value_IGI;
	}
}


void
ODM_Write_DIG(
	PDM_ODM_T		pDM_Odm,
	u8			CurrentIGI
	)
{
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("ODM_REG(IGI_A,pDM_Odm)=0x%x, ODM_BIT(IGI,pDM_Odm)=0x%x \n",
		ODM_REG(IGI_A,pDM_Odm),ODM_BIT(IGI,pDM_Odm)));

	if(pDM_DigTable->CurIGValue != CurrentIGI)//if(pDM_DigTable->PreIGValue != CurrentIGI)
	{
		if(pDM_Odm->SupportPlatform & (ODM_CE|ODM_MP))
		{
			ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
				if(pDM_Odm->SupportICType != ODM_RTL8188E)
				ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
		}
		else if(pDM_Odm->SupportPlatform & (ODM_AP|ODM_ADSL))
		{
			switch(*(pDM_Odm->pOnePathCCA))
			{
			case ODM_CCA_2R:
				ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
					if(pDM_Odm->SupportICType != ODM_RTL8188E)
					ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
				break;
			case ODM_CCA_1R_A:
				ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
					if(pDM_Odm->SupportICType != ODM_RTL8188E)
					ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B,pDM_Odm), ODM_BIT(IGI,pDM_Odm), getIGIForDiff(CurrentIGI));
				break;
			case ODM_CCA_1R_B:
				ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm), getIGIForDiff(CurrentIGI));
					if(pDM_Odm->SupportICType != ODM_RTL8188E)
					ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
					break;
				}
		}
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("CurrentIGI(0x%02x). \n",CurrentIGI));
		//pDM_DigTable->PreIGValue = pDM_DigTable->CurIGValue;
		pDM_DigTable->CurIGValue = CurrentIGI;
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("ODM_Write_DIG():CurrentIGI=0x%x \n",CurrentIGI));
}


//Need LPS mode for CE platform --2012--08--24---
//8723AS/8189ES
void
odm_DIGbyRSSI_LPS(
		PDM_ODM_T		pDM_Odm
	)
{
	struct rtw_adapter *					pAdapter =pDM_Odm->Adapter;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = &pDM_Odm->FalseAlmCnt;
	u8	RSSI_Lower=DM_DIG_MIN_NIC;   //0x1E or 0x1C
	u8	bFwCurrentInPSMode = FALSE;
	u8	CurrentIGI=pDM_Odm->RSSI_Min;

	if(! (pDM_Odm->SupportICType & (ODM_RTL8723A |ODM_RTL8188E)))
		return;

	//if((pDM_Odm->SupportInterface==ODM_ITRF_PCIE)||(pDM_Odm->SupportInterface ==ODM_ITRF_USB))
	//	return;

	CurrentIGI=CurrentIGI+RSSI_OFFSET_DIG;
#ifdef CONFIG_LPS
	bFwCurrentInPSMode = pAdapter->pwrctrlpriv.bFwCurrentInPSMode;
#endif

	//ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG_LPS, ODM_DBG_LOUD, ("odm_DIG()==>\n"));

	// Using FW PS mode to make IGI
	if(bFwCurrentInPSMode)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("---Neil---odm_DIG is in LPS mode\n"));
		//Adjust by  FA in LPS MODE
		if(pFalseAlmCnt->Cnt_all> DM_DIG_FA_TH2_LPS)
			CurrentIGI = CurrentIGI+2;
		else if (pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH1_LPS)
			CurrentIGI = CurrentIGI+1;
		else if(pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH0_LPS)
			CurrentIGI = CurrentIGI-1;
	}
	else
	{
		CurrentIGI = RSSI_Lower;
	}

	//Lower bound checking

	//RSSI Lower bound check
	if((pDM_Odm->RSSI_Min-10) > DM_DIG_MIN_NIC)
		RSSI_Lower =(pDM_Odm->RSSI_Min-10);
	else
		RSSI_Lower =DM_DIG_MIN_NIC;

	//Upper and Lower Bound checking
	 if(CurrentIGI > DM_DIG_MAX_NIC)
		CurrentIGI=DM_DIG_MAX_NIC;
	 else if(CurrentIGI < RSSI_Lower)
		CurrentIGI =RSSI_Lower;

	ODM_Write_DIG(pDM_Odm, CurrentIGI);//ODM_Write_DIG(pDM_Odm, pDM_DigTable->CurIGValue);

}


void
odm_DIGInit(
		PDM_ODM_T		pDM_Odm
	)
{
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;

	//pDM_DigTable->Dig_Enable_Flag = TRUE;
	//pDM_DigTable->Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_MAX;
	pDM_DigTable->CurIGValue = (u8) ODM_GetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm));
	//pDM_DigTable->PreIGValue = 0x0;
	//pDM_DigTable->CurSTAConnectState = pDM_DigTable->PreSTAConnectState = DIG_STA_DISCONNECT;
	//pDM_DigTable->CurMultiSTAConnectState = DIG_MultiSTA_DISCONNECT;
	pDM_DigTable->RssiLowThresh	= DM_DIG_THRESH_LOW;
	pDM_DigTable->RssiHighThresh	= DM_DIG_THRESH_HIGH;
	pDM_DigTable->FALowThresh	= DM_FALSEALARM_THRESH_LOW;
	pDM_DigTable->FAHighThresh	= DM_FALSEALARM_THRESH_HIGH;
	if(pDM_Odm->BoardType == ODM_BOARD_HIGHPWR)
	{
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_NIC;
		pDM_DigTable->rx_gain_range_min = DM_DIG_MIN_NIC;
	}
	else
	{
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_NIC;
		pDM_DigTable->rx_gain_range_min = DM_DIG_MIN_NIC;
	}
	pDM_DigTable->BackoffVal = DM_DIG_BACKOFF_DEFAULT;
	pDM_DigTable->BackoffVal_range_max = DM_DIG_BACKOFF_MAX;
	pDM_DigTable->BackoffVal_range_min = DM_DIG_BACKOFF_MIN;
	pDM_DigTable->PreCCK_CCAThres = 0xFF;
	pDM_DigTable->CurCCK_CCAThres = 0x83;
	pDM_DigTable->ForbiddenIGI = DM_DIG_MIN_NIC;
	pDM_DigTable->LargeFAHit = 0;
	pDM_DigTable->Recover_cnt = 0;
	pDM_DigTable->DIG_Dynamic_MIN_0 =DM_DIG_MIN_NIC;
	pDM_DigTable->DIG_Dynamic_MIN_1 = DM_DIG_MIN_NIC;
	pDM_DigTable->bMediaConnect_0 = FALSE;
	pDM_DigTable->bMediaConnect_1 = FALSE;

	//To Initialize pDM_Odm->bDMInitialGainEnable == FALSE to avoid DIG error
	pDM_Odm->bDMInitialGainEnable = TRUE;

}


void
odm_DIG(
		PDM_ODM_T		pDM_Odm
	)
{

	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = &pDM_Odm->FalseAlmCnt;
	pRXHP_T						pRX_HP_Table  = &pDM_Odm->DM_RXHP_Table;
	u8						DIG_Dynamic_MIN;
	u8						DIG_MaxOfMin;
	bool						FirstConnect, FirstDisConnect;
	u8						dm_dig_max, dm_dig_min;
	u8						CurrentIGI = pDM_DigTable->CurIGValue;


	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG()==>\n"));
	//if(!(pDM_Odm->SupportAbility & (ODM_BB_DIG|ODM_BB_FA_CNT)))
	if((!(pDM_Odm->SupportAbility&ODM_BB_DIG)) ||(!(pDM_Odm->SupportAbility&ODM_BB_FA_CNT)))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() Return: SupportAbility ODM_BB_DIG or ODM_BB_FA_CNT is disabled\n"));
		return;
	}

	if(*(pDM_Odm->pbScanInProcess))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() Return: In Scan Progress \n"));
		return;
	}

	//add by Neil Chen to avoid PSD is processing
	if(pDM_Odm->bDMInitialGainEnable == FALSE)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() Return: PSD is Processing \n"));
		return;
	}

	if(pDM_Odm->SupportICType == ODM_RTL8192D)
	{
		if(*(pDM_Odm->pMacPhyMode) == ODM_DMSP)
		{
			if(*(pDM_Odm->pbMasterOfDMSP))
			{
				DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_0;
				FirstConnect = (pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == FALSE);
				FirstDisConnect = (!pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == TRUE);
			}
			else
			{
				DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_1;
				FirstConnect = (pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_1 == FALSE);
				FirstDisConnect = (!pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_1 == TRUE);
			}
		}
		else
		{
			if(*(pDM_Odm->pBandType) == ODM_BAND_5G)
			{
				DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_0;
				FirstConnect = (pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == FALSE);
				FirstDisConnect = (!pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == TRUE);
			}
			else
			{
				DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_1;
				FirstConnect = (pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_1 == FALSE);
				FirstDisConnect = (!pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_1 == TRUE);
			}
		}
	}
	else
	{
		DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_0;
		FirstConnect = (pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == FALSE);
		FirstDisConnect = (!pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == TRUE);
	}

	//1 Boundary Decision
	if((pDM_Odm->SupportICType & (ODM_RTL8192C|ODM_RTL8723A)) &&
		((pDM_Odm->BoardType == ODM_BOARD_HIGHPWR) || pDM_Odm->ExtLNA))
	{
		if(pDM_Odm->SupportPlatform & (ODM_AP|ODM_ADSL))
		{

			dm_dig_max = DM_DIG_MAX_AP_HP;
			dm_dig_min = DM_DIG_MIN_AP_HP;
		}
		else
		{
			dm_dig_max = DM_DIG_MAX_NIC_HP;
			dm_dig_min = DM_DIG_MIN_NIC_HP;
		}
		DIG_MaxOfMin = DM_DIG_MAX_AP_HP;
	}
	else
	{
		if(pDM_Odm->SupportPlatform & (ODM_AP|ODM_ADSL))
		{
			dm_dig_max = DM_DIG_MAX_AP;
			dm_dig_min = DM_DIG_MIN_AP;
			DIG_MaxOfMin = dm_dig_max;
		}
		else
		{
			dm_dig_max = DM_DIG_MAX_NIC;
			dm_dig_min = DM_DIG_MIN_NIC;
			DIG_MaxOfMin = DM_DIG_MAX_AP;
		}
	}


	if(pDM_Odm->bLinked)
	{
	      //2 8723A Series, offset need to be 10 //neil
		if(pDM_Odm->SupportICType==(ODM_RTL8723A))
		{
			//2 Upper Bound
			if(( pDM_Odm->RSSI_Min + 10) > DM_DIG_MAX_NIC )
				pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_NIC;
			else if(( pDM_Odm->RSSI_Min + 10) < DM_DIG_MIN_NIC )
				pDM_DigTable->rx_gain_range_max = DM_DIG_MIN_NIC;
			else
				pDM_DigTable->rx_gain_range_max = pDM_Odm->RSSI_Min + 10;

			//2 If BT is Concurrent, need to set Lower Bound

#if(BT_30_SUPPORT == 1)
			if(pDM_Odm->bBtBusy)
			{
				if(pDM_Odm->RSSI_Min>10)
				{
				if((pDM_Odm->RSSI_Min - 10) > DM_DIG_MAX_NIC)
						DIG_Dynamic_MIN = DM_DIG_MAX_NIC;
				else if((pDM_Odm->RSSI_Min - 10) < DM_DIG_MIN_NIC)
						DIG_Dynamic_MIN = DM_DIG_MIN_NIC;
					else
						DIG_Dynamic_MIN = pDM_Odm->RSSI_Min - 10;
				}
				else
					DIG_Dynamic_MIN=DM_DIG_MIN_NIC;
			}
			else
#endif
			{
				DIG_Dynamic_MIN=DM_DIG_MIN_NIC;
			}
		}
		else
		{
		//2 Modify DIG upper bound
			if((pDM_Odm->RSSI_Min + 20) > dm_dig_max )
				pDM_DigTable->rx_gain_range_max = dm_dig_max;
			else if((pDM_Odm->RSSI_Min + 20) < dm_dig_min )
				pDM_DigTable->rx_gain_range_max = dm_dig_min;
			else
				pDM_DigTable->rx_gain_range_max = pDM_Odm->RSSI_Min + 20;


		//2 Modify DIG lower bound
	/*
		if((pFalseAlmCnt->Cnt_all > 500)&&(DIG_Dynamic_MIN < 0x25))
			DIG_Dynamic_MIN++;
		else if(((pFalseAlmCnt->Cnt_all < 500)||(pDM_Odm->RSSI_Min < 8))&&(DIG_Dynamic_MIN > dm_dig_min))
			DIG_Dynamic_MIN--;
	*/
			if(pDM_Odm->bOneEntryOnly)
			{
				if(pDM_Odm->RSSI_Min < dm_dig_min)
					DIG_Dynamic_MIN = dm_dig_min;
				else if (pDM_Odm->RSSI_Min > DIG_MaxOfMin)
					DIG_Dynamic_MIN = DIG_MaxOfMin;
				else
					DIG_Dynamic_MIN = pDM_Odm->RSSI_Min;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() : bOneEntryOnly=TRUE,  DIG_Dynamic_MIN=0x%x\n",DIG_Dynamic_MIN));
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() : pDM_Odm->RSSI_Min=%d\n",pDM_Odm->RSSI_Min));
			}
			//1 Lower Bound for 88E AntDiv
#if (RTL8188E_SUPPORT == 1)
			else if((pDM_Odm->SupportICType == ODM_RTL8188E)&&(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
			{
				if(pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
				{
					DIG_Dynamic_MIN = (u8) pDM_DigTable->AntDiv_RSSI_max;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_DIG(): pDM_DigTable->AntDiv_RSSI_max=%d \n",pDM_DigTable->AntDiv_RSSI_max));
				}
			}
#endif
			else
			{
				DIG_Dynamic_MIN=dm_dig_min;
			}
		}
	}
	else
	{
		pDM_DigTable->rx_gain_range_max = dm_dig_max;
		DIG_Dynamic_MIN = dm_dig_min;
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() : No Link\n"));
	}

	//1 Modify DIG lower bound, deal with abnormally large false alarm
	if(pFalseAlmCnt->Cnt_all > 10000)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("dm_DIG(): Abnornally false alarm case. \n"));

		if(pDM_DigTable->LargeFAHit != 3)
		pDM_DigTable->LargeFAHit++;
		if(pDM_DigTable->ForbiddenIGI < CurrentIGI)//if(pDM_DigTable->ForbiddenIGI < pDM_DigTable->CurIGValue)
		{
			pDM_DigTable->ForbiddenIGI = CurrentIGI;//pDM_DigTable->ForbiddenIGI = pDM_DigTable->CurIGValue;
			pDM_DigTable->LargeFAHit = 1;
		}

		if(pDM_DigTable->LargeFAHit >= 3)
		{
			if((pDM_DigTable->ForbiddenIGI+1) >pDM_DigTable->rx_gain_range_max)
				pDM_DigTable->rx_gain_range_min = pDM_DigTable->rx_gain_range_max;
			else
				pDM_DigTable->rx_gain_range_min = (pDM_DigTable->ForbiddenIGI + 1);
			pDM_DigTable->Recover_cnt = 3600; //3600=2hr
		}

	}
	else
	{
		//Recovery mechanism for IGI lower bound
		if(pDM_DigTable->Recover_cnt != 0)
			pDM_DigTable->Recover_cnt --;
		else
		{
			if(pDM_DigTable->LargeFAHit < 3)
			{
				if((pDM_DigTable->ForbiddenIGI -1) < DIG_Dynamic_MIN) //DM_DIG_MIN)
				{
					pDM_DigTable->ForbiddenIGI = DIG_Dynamic_MIN; //DM_DIG_MIN;
					pDM_DigTable->rx_gain_range_min = DIG_Dynamic_MIN; //DM_DIG_MIN;
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: At Lower Bound\n"));
				}
				else
				{
					pDM_DigTable->ForbiddenIGI --;
					pDM_DigTable->rx_gain_range_min = (pDM_DigTable->ForbiddenIGI + 1);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: Approach Lower Bound\n"));
				}
			}
			else
			{
				pDM_DigTable->LargeFAHit = 0;
			}
		}
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): pDM_DigTable->LargeFAHit=%d\n",pDM_DigTable->LargeFAHit));

	//1 Adjust initial gain by false alarm
	if(pDM_Odm->bLinked)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): DIG AfterLink\n"));
		if(FirstConnect)
		{
			CurrentIGI = pDM_Odm->RSSI_Min;
			ODM_RT_TRACE(pDM_Odm,	ODM_COMP_DIG, ODM_DBG_LOUD, ("DIG: First Connect\n"));
		}
		else
		{
			if(pDM_Odm->SupportICType == ODM_RTL8192D)
			{
				if(pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH2_92D)
					CurrentIGI = CurrentIGI + 2;//pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+2;
				else if (pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH1_92D)
					CurrentIGI = CurrentIGI + 1; //pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+1;
				else if(pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH0_92D)
					CurrentIGI = CurrentIGI - 1;//pDM_DigTable->CurIGValue =pDM_DigTable->PreIGValue-1;
			}
			else
			{
#if(BT_30_SUPPORT == 1)
				if(pDM_Odm->bBtBusy)
				{
					if(pFalseAlmCnt->Cnt_all > 0x300)
						CurrentIGI = CurrentIGI + 2;
					else if (pFalseAlmCnt->Cnt_all > 0x250)
						CurrentIGI = CurrentIGI + 1;
					else if(pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH0)
						CurrentIGI = CurrentIGI -1;
				}
				else
#endif
				{
				if(pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH2)
						CurrentIGI = CurrentIGI + 4;//pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+2;
				else if (pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH1)
						CurrentIGI = CurrentIGI + 2;//pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+1;
				else if(pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH0)
						CurrentIGI = CurrentIGI - 2;//pDM_DigTable->CurIGValue =pDM_DigTable->PreIGValue-1;


			}
		}
	}
	}
	else
	{
		//CurrentIGI = pDM_DigTable->rx_gain_range_min;//pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_min
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): DIG BeforeLink\n"));
		if(FirstDisConnect)
		{
			CurrentIGI = pDM_DigTable->rx_gain_range_min;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): First DisConnect \n"));
		} else {
			//2012.03.30 LukeLee: enable DIG before link but with very high thresholds
			if(pFalseAlmCnt->Cnt_all > 10000)
				CurrentIGI = CurrentIGI + 2;//pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+2;
			else if (pFalseAlmCnt->Cnt_all > 8000)
				CurrentIGI = CurrentIGI + 1;//pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+1;
			else if(pFalseAlmCnt->Cnt_all < 500)
				CurrentIGI = CurrentIGI - 1;//pDM_DigTable->CurIGValue =pDM_DigTable->PreIGValue-1;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): England DIG \n"));
		}
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): DIG End Adjust IGI\n"));
	//1 Check initial gain by upper/lower bound
/*
	if(pDM_DigTable->CurIGValue > pDM_DigTable->rx_gain_range_max)
		pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_max;
	if(pDM_DigTable->CurIGValue < pDM_DigTable->rx_gain_range_min)
		pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_min;
*/
	if(CurrentIGI > pDM_DigTable->rx_gain_range_max)
		CurrentIGI = pDM_DigTable->rx_gain_range_max;
	if(CurrentIGI < pDM_DigTable->rx_gain_range_min)
		CurrentIGI = pDM_DigTable->rx_gain_range_min;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): rx_gain_range_max=0x%x, rx_gain_range_min=0x%x\n",
		pDM_DigTable->rx_gain_range_max, pDM_DigTable->rx_gain_range_min));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): TotalFA=%d\n", pFalseAlmCnt->Cnt_all));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): CurIGValue=0x%x\n", CurrentIGI));

	//2 High power RSSI threshold

#if (RTL8192D_SUPPORT==1)
	if(pDM_Odm->SupportICType == ODM_RTL8192D)
	{
		//sherry  delete DualMacSmartConncurrent 20110517
		if(*(pDM_Odm->pMacPhyMode) == ODM_DMSP)
		{
			ODM_Write_DIG_DMSP(pDM_Odm, CurrentIGI);//ODM_Write_DIG_DMSP(pDM_Odm, pDM_DigTable->CurIGValue);
			if(*(pDM_Odm->pbMasterOfDMSP))
			{
				pDM_DigTable->bMediaConnect_0 = pDM_Odm->bLinked;
				pDM_DigTable->DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
			}
			else
			{
				pDM_DigTable->bMediaConnect_1 = pDM_Odm->bLinked;
				pDM_DigTable->DIG_Dynamic_MIN_1 = DIG_Dynamic_MIN;
			}
		}
		else
		{
			ODM_Write_DIG(pDM_Odm, CurrentIGI);//ODM_Write_DIG(pDM_Odm, pDM_DigTable->CurIGValue);
			if(*(pDM_Odm->pBandType) == ODM_BAND_5G)
			{
				pDM_DigTable->bMediaConnect_0 = pDM_Odm->bLinked;
				pDM_DigTable->DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
			}
			else
			{
				pDM_DigTable->bMediaConnect_1 = pDM_Odm->bLinked;
				pDM_DigTable->DIG_Dynamic_MIN_1 = DIG_Dynamic_MIN;
			}
		}
	}
	else
#endif
	{
		ODM_Write_DIG(pDM_Odm, CurrentIGI);//ODM_Write_DIG(pDM_Odm, pDM_DigTable->CurIGValue);
		pDM_DigTable->bMediaConnect_0 = pDM_Odm->bLinked;
		pDM_DigTable->DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
	}

}

//3============================================================
//3 FASLE ALARM CHECK
//3============================================================

void
odm_FalseAlarmCounterStatistics(
		PDM_ODM_T		pDM_Odm
	)
{
	u32 ret_value;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(pDM_Odm->FalseAlmCnt);

	if(!(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))
		return;

//	if(pDM_Odm->SupportICType != ODM_RTL8812)
	if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
	{

	//hold ofdm counter
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_HOLDC_11N, BIT31, 1); //hold page C counter
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT31, 1); //hold page D counter

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE1_11N, bMaskDWord);
	FalseAlmCnt->Cnt_Fast_Fsync = (ret_value&0xffff);
	FalseAlmCnt->Cnt_SB_Search_fail = ((ret_value&0xffff0000)>>16);
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE2_11N, bMaskDWord);
	FalseAlmCnt->Cnt_OFDM_CCA = (ret_value&0xffff);
	FalseAlmCnt->Cnt_Parity_Fail = ((ret_value&0xffff0000)>>16);
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE3_11N, bMaskDWord);
	FalseAlmCnt->Cnt_Rate_Illegal = (ret_value&0xffff);
	FalseAlmCnt->Cnt_Crc8_fail = ((ret_value&0xffff0000)>>16);
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE4_11N, bMaskDWord);
	FalseAlmCnt->Cnt_Mcs_fail = (ret_value&0xffff);

	FalseAlmCnt->Cnt_Ofdm_fail =	FalseAlmCnt->Cnt_Parity_Fail + FalseAlmCnt->Cnt_Rate_Illegal +
								FalseAlmCnt->Cnt_Crc8_fail + FalseAlmCnt->Cnt_Mcs_fail +
								FalseAlmCnt->Cnt_Fast_Fsync + FalseAlmCnt->Cnt_SB_Search_fail;

#if (RTL8188E_SUPPORT==1)
	if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_SC_CNT_11N, bMaskDWord);
		FalseAlmCnt->Cnt_BW_LSC = (ret_value&0xffff);
		FalseAlmCnt->Cnt_BW_USC = ((ret_value&0xffff0000)>>16);
	}
#endif

#if (RTL8192D_SUPPORT==1)
	if(pDM_Odm->SupportICType == ODM_RTL8192D)
	{
		odm_GetCCKFalseAlarm_92D(pDM_Odm);
	}
	else
#endif
	{
		//hold cck counter
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT12, 1);
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT14, 1);

			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_FA_LSB_11N, bMaskByte0);
		FalseAlmCnt->Cnt_Cck_fail = ret_value;
			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_FA_MSB_11N, bMaskByte3);
		FalseAlmCnt->Cnt_Cck_fail +=  (ret_value& 0xff)<<8;

			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_CCA_CNT_11N, bMaskDWord);
		FalseAlmCnt->Cnt_CCK_CCA = ((ret_value&0xFF)<<8) |((ret_value&0xFF00)>>8);
	}

	FalseAlmCnt->Cnt_all = (FalseAlmCnt->Cnt_Fast_Fsync +
						FalseAlmCnt->Cnt_SB_Search_fail +
						FalseAlmCnt->Cnt_Parity_Fail +
						FalseAlmCnt->Cnt_Rate_Illegal +
						FalseAlmCnt->Cnt_Crc8_fail +
						FalseAlmCnt->Cnt_Mcs_fail +
						FalseAlmCnt->Cnt_Cck_fail);

	FalseAlmCnt->Cnt_CCA_all = FalseAlmCnt->Cnt_OFDM_CCA + FalseAlmCnt->Cnt_CCK_CCA;

#if (RTL8192C_SUPPORT==1)
	if(pDM_Odm->SupportICType == ODM_RTL8192C)
		odm_ResetFACounter_92C(pDM_Odm);
#endif

#if (RTL8192D_SUPPORT==1)
	if(pDM_Odm->SupportICType == ODM_RTL8192D)
		odm_ResetFACounter_92D(pDM_Odm);
#endif

	if(pDM_Odm->SupportICType >=ODM_RTL8723A)
	{
		//reset false alarm counter registers
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTC_11N, BIT31, 1);
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTC_11N, BIT31, 0);
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT27, 1);
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT27, 0);
		//update ofdm counter
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_HOLDC_11N, BIT31, 0); //update page C counter
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT31, 0); //update page D counter

		//reset CCK CCA counter
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT13|BIT12, 0);
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT13|BIT12, 2);
		//reset CCK FA counter
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT15|BIT14, 0);
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT15|BIT14, 2);
	}

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Enter odm_FalseAlarmCounterStatistics\n"));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Cnt_Fast_Fsync=%d, Cnt_SB_Search_fail=%d\n",
		FalseAlmCnt->Cnt_Fast_Fsync, FalseAlmCnt->Cnt_SB_Search_fail));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Cnt_Parity_Fail=%d, Cnt_Rate_Illegal=%d\n",
		FalseAlmCnt->Cnt_Parity_Fail, FalseAlmCnt->Cnt_Rate_Illegal));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Cnt_Crc8_fail=%d, Cnt_Mcs_fail=%d\n",
		FalseAlmCnt->Cnt_Crc8_fail, FalseAlmCnt->Cnt_Mcs_fail));
	}
	else //FOR ODM_IC_11AC_SERIES
	{
		//read OFDM FA counter
		FalseAlmCnt->Cnt_Ofdm_fail = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_11AC, bMaskLWord);
		FalseAlmCnt->Cnt_Cck_fail = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_FA_11AC, bMaskLWord);
		FalseAlmCnt->Cnt_all = FalseAlmCnt->Cnt_Ofdm_fail + FalseAlmCnt->Cnt_Cck_fail;

		// reset OFDM FA coutner
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RST_11AC, BIT17, 1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RST_11AC, BIT17, 0);
		// reset CCK FA counter
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11AC, BIT15, 0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11AC, BIT15, 1);
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Cnt_Cck_fail=%d\n",	FalseAlmCnt->Cnt_Cck_fail));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Cnt_Ofdm_fail=%d\n",	FalseAlmCnt->Cnt_Ofdm_fail));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Total False Alarm=%d\n",	FalseAlmCnt->Cnt_all));
}

//3============================================================
//3 CCK Packet Detect Threshold
//3============================================================

void
odm_CCKPacketDetectionThresh(
		PDM_ODM_T		pDM_Odm
	)
{

	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;
	u8	CurCCK_CCAThres;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(pDM_Odm->FalseAlmCnt);

	if(!(pDM_Odm->SupportAbility & (ODM_BB_CCK_PD|ODM_BB_FA_CNT)))
		return;

	if(pDM_Odm->ExtLNA)
		return;

	if(pDM_Odm->bLinked)
	{
		if(pDM_Odm->RSSI_Min > 25)
			CurCCK_CCAThres = 0xcd;
		else if((pDM_Odm->RSSI_Min <= 25) && (pDM_Odm->RSSI_Min > 10))
			CurCCK_CCAThres = 0x83;
		else
		{
			if(FalseAlmCnt->Cnt_Cck_fail > 1000)
				CurCCK_CCAThres = 0x83;
			else
				CurCCK_CCAThres = 0x40;
		}
	}
	else
	{
		if(FalseAlmCnt->Cnt_Cck_fail > 1000)
			CurCCK_CCAThres = 0x83;
		else
			CurCCK_CCAThres = 0x40;
	}

#if (RTL8192D_SUPPORT==1)
	if(pDM_Odm->SupportICType == ODM_RTL8192D)
		ODM_Write_CCK_CCA_Thres_92D(pDM_Odm, CurCCK_CCAThres);
	else
#endif
		ODM_Write_CCK_CCA_Thres(pDM_Odm, CurCCK_CCAThres);
}

void
ODM_Write_CCK_CCA_Thres(
	PDM_ODM_T		pDM_Odm,
	u8			CurCCK_CCAThres
	)
{
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;

	if(pDM_DigTable->CurCCK_CCAThres!=CurCCK_CCAThres)		//modify by Guo.Mingzhi 2012-01-03
	{
		ODM_Write1Byte(pDM_Odm, ODM_REG(CCK_CCA,pDM_Odm), CurCCK_CCAThres);
	}
	pDM_DigTable->PreCCK_CCAThres = pDM_DigTable->CurCCK_CCAThres;
	pDM_DigTable->CurCCK_CCAThres = CurCCK_CCAThres;

}

//3============================================================
//3 BB Power Save
//3============================================================
void
odm_DynamicBBPowerSavingInit(
		PDM_ODM_T		pDM_Odm
	)
{
	pPS_T	pDM_PSTable = &pDM_Odm->DM_PSTable;

	pDM_PSTable->PreCCAState = CCA_MAX;
	pDM_PSTable->CurCCAState = CCA_MAX;
	pDM_PSTable->PreRFState = RF_MAX;
	pDM_PSTable->CurRFState = RF_MAX;
	pDM_PSTable->Rssi_val_min = 0;
	pDM_PSTable->initialize = 0;
}


void
odm_DynamicBBPowerSaving(
		PDM_ODM_T		pDM_Odm
	)
{
	if ((pDM_Odm->SupportICType != ODM_RTL8192C) && (pDM_Odm->SupportICType != ODM_RTL8723A))
		return;
	if(!(pDM_Odm->SupportAbility & ODM_BB_PWR_SAVE))
		return;
	if(!(pDM_Odm->SupportPlatform & (ODM_MP|ODM_CE)))
		return;

	//1 2.Power Saving for 92C
	if((pDM_Odm->SupportICType == ODM_RTL8192C) &&(pDM_Odm->RFType == ODM_2T2R))
	{
		odm_1R_CCA(pDM_Odm);
	}

	// 20100628 Joseph: Turn off BB power save for 88CE because it makesthroughput unstable.
	// 20100831 Joseph: Turn ON BB power save again after modifying AGC delay from 900ns ot 600ns.
	//1 3.Power Saving for 88C
	else
	{
		ODM_RF_Saving(pDM_Odm, FALSE);
	}
}

void
odm_1R_CCA(
	PDM_ODM_T	pDM_Odm
	)
{
	pPS_T	pDM_PSTable = &pDM_Odm->DM_PSTable;

	if(pDM_Odm->RSSI_Min!= 0xFF)
	{

		if(pDM_PSTable->PreCCAState == CCA_2R)
		{
			if(pDM_Odm->RSSI_Min >= 35)
				pDM_PSTable->CurCCAState = CCA_1R;
			else
				pDM_PSTable->CurCCAState = CCA_2R;

		}
		else{
			if(pDM_Odm->RSSI_Min <= 30)
				pDM_PSTable->CurCCAState = CCA_2R;
			else
				pDM_PSTable->CurCCAState = CCA_1R;
		}
	}
	else{
		pDM_PSTable->CurCCAState=CCA_MAX;
	}

	if(pDM_PSTable->PreCCAState != pDM_PSTable->CurCCAState)
	{
		if(pDM_PSTable->CurCCAState == CCA_1R)
		{
			if(  pDM_Odm->RFType ==ODM_2T2R )
			{
				ODM_SetBBReg(pDM_Odm, 0xc04  , bMaskByte0, 0x13);
				//PHY_SetBBReg(pAdapter, 0xe70, bMaskByte3, 0x20);
			}
			else
			{
				ODM_SetBBReg(pDM_Odm, 0xc04  , bMaskByte0, 0x23);
				//PHY_SetBBReg(pAdapter, 0xe70, 0x7fc00000, 0x10c); // Set RegE70[30:22] = 9b'100001100
			}
		}
		else
		{
			ODM_SetBBReg(pDM_Odm, 0xc04  , bMaskByte0, 0x33);
			//PHY_SetBBReg(pAdapter,0xe70, bMaskByte3, 0x63);
		}
		pDM_PSTable->PreCCAState = pDM_PSTable->CurCCAState;
	}
	//ODM_RT_TRACE(pDM_Odm,	COMP_BB_POWERSAVING, DBG_LOUD, ("CCAStage = %s\n",(pDM_PSTable->CurCCAState==0)?"1RCCA":"2RCCA"));
}

void
ODM_RF_Saving(
	PDM_ODM_T	pDM_Odm,
	u8		bForceInNormal
	)
{
	pPS_T	pDM_PSTable = &pDM_Odm->DM_PSTable;
	u8	Rssi_Up_bound = 30 ;
	u8	Rssi_Low_bound = 25;
	if(pDM_Odm->PatchID == 40 ) //RT_CID_819x_FUNAI_TV
	{
		Rssi_Up_bound = 50 ;
		Rssi_Low_bound = 45;
	}
	if(pDM_PSTable->initialize == 0){

		pDM_PSTable->Reg874 = (ODM_GetBBReg(pDM_Odm, 0x874, bMaskDWord)&0x1CC000)>>14;
		pDM_PSTable->RegC70 = (ODM_GetBBReg(pDM_Odm, 0xc70, bMaskDWord)&BIT3)>>3;
		pDM_PSTable->Reg85C = (ODM_GetBBReg(pDM_Odm, 0x85c, bMaskDWord)&0xFF000000)>>24;
		pDM_PSTable->RegA74 = (ODM_GetBBReg(pDM_Odm, 0xa74, bMaskDWord)&0xF000)>>12;
		//Reg818 = PHY_QueryBBReg(pAdapter, 0x818, bMaskDWord);
		pDM_PSTable->initialize = 1;
	}

	if(!bForceInNormal)
	{
		if(pDM_Odm->RSSI_Min != 0xFF)
		{
			if(pDM_PSTable->PreRFState == RF_Normal)
			{
				if(pDM_Odm->RSSI_Min >= Rssi_Up_bound)
					pDM_PSTable->CurRFState = RF_Save;
				else
					pDM_PSTable->CurRFState = RF_Normal;
			}
			else{
				if(pDM_Odm->RSSI_Min <= Rssi_Low_bound)
					pDM_PSTable->CurRFState = RF_Normal;
				else
					pDM_PSTable->CurRFState = RF_Save;
			}
		}
		else
			pDM_PSTable->CurRFState=RF_MAX;
	}
	else
	{
		pDM_PSTable->CurRFState = RF_Normal;
	}

	if(pDM_PSTable->PreRFState != pDM_PSTable->CurRFState)
	{
		if(pDM_PSTable->CurRFState == RF_Save)
		{
			// <tynli_note> 8723 RSSI report will be wrong. Set 0x874[5]=1 when enter BB power saving mode.
			// Suggested by SD3 Yu-Nan. 2011.01.20.
			if(pDM_Odm->SupportICType == ODM_RTL8723A)
			{
				ODM_SetBBReg(pDM_Odm, 0x874  , BIT5, 0x1); //Reg874[5]=1b'1
			}
			ODM_SetBBReg(pDM_Odm, 0x874  , 0x1C0000, 0x2); //Reg874[20:18]=3'b010
			ODM_SetBBReg(pDM_Odm, 0xc70, BIT3, 0); //RegC70[3]=1'b0
			ODM_SetBBReg(pDM_Odm, 0x85c, 0xFF000000, 0x63); //Reg85C[31:24]=0x63
			ODM_SetBBReg(pDM_Odm, 0x874, 0xC000, 0x2); //Reg874[15:14]=2'b10
			ODM_SetBBReg(pDM_Odm, 0xa74, 0xF000, 0x3); //RegA75[7:4]=0x3
			ODM_SetBBReg(pDM_Odm, 0x818, BIT28, 0x0); //Reg818[28]=1'b0
			ODM_SetBBReg(pDM_Odm, 0x818, BIT28, 0x1); //Reg818[28]=1'b1
			//ODM_RT_TRACE(pDM_Odm,	COMP_BB_POWERSAVING, DBG_LOUD, (" RF_Save"));
		}
		else
		{
			ODM_SetBBReg(pDM_Odm, 0x874  , 0x1CC000, pDM_PSTable->Reg874);
			ODM_SetBBReg(pDM_Odm, 0xc70, BIT3, pDM_PSTable->RegC70);
			ODM_SetBBReg(pDM_Odm, 0x85c, 0xFF000000, pDM_PSTable->Reg85C);
			ODM_SetBBReg(pDM_Odm, 0xa74, 0xF000, pDM_PSTable->RegA74);
			ODM_SetBBReg(pDM_Odm,0x818, BIT28, 0x0);

			if(pDM_Odm->SupportICType == ODM_RTL8723A)
			{
				ODM_SetBBReg(pDM_Odm,0x874  , BIT5, 0x0); //Reg874[5]=1b'0
			}
			//ODM_RT_TRACE(pDM_Odm,	COMP_BB_POWERSAVING, DBG_LOUD, (" RF_Normal"));
		}
		pDM_PSTable->PreRFState =pDM_PSTable->CurRFState;
	}
}


//3============================================================
//3 RATR MASK
//3============================================================
//3============================================================
//3 Rate Adaptive
//3============================================================

void
odm_RateAdaptiveMaskInit(
	PDM_ODM_T	pDM_Odm
	)
{
	PODM_RATE_ADAPTIVE	pOdmRA = &pDM_Odm->RateAdaptive;

	pOdmRA->Type = DM_Type_ByDriver;
	if (pOdmRA->Type == DM_Type_ByDriver)
		pDM_Odm->bUseRAMask = _TRUE;
	else
		pDM_Odm->bUseRAMask = _FALSE;

	pOdmRA->RATRState = DM_RATR_STA_INIT;
	pOdmRA->HighRSSIThresh = 50;
	pOdmRA->LowRSSIThresh = 20;
}

u32 ODM_Get_Rate_Bitmap(
	PDM_ODM_T	pDM_Odm,
	u32		macid,
	u32		ra_mask,
	u8		rssi_level)
{
	PSTA_INFO_T	pEntry;
	u32	rate_bitmap = 0x0fffffff;
	u8	WirelessMode;
	//u8	WirelessMode =*(pDM_Odm->pWirelessMode);


	pEntry = pDM_Odm->pODM_StaInfo[macid];
	if(!IS_STA_VALID(pEntry))
		return ra_mask;

	WirelessMode = pEntry->wireless_mode;

	switch(WirelessMode)
	{
		case ODM_WM_B:
			if(ra_mask & 0x0000000c)		//11M or 5.5M enable
				rate_bitmap = 0x0000000d;
			else
				rate_bitmap = 0x0000000f;
			break;

		case (ODM_WM_A|ODM_WM_G):
			if(rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x00000f00;
			else
				rate_bitmap = 0x00000ff0;
			break;

		case (ODM_WM_B|ODM_WM_G):
			if(rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x00000f00;
			else if(rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x00000ff0;
			else
				rate_bitmap = 0x00000ff5;
			break;

		case (ODM_WM_B|ODM_WM_G|ODM_WM_N24G)	:
		case (ODM_WM_A|ODM_WM_B|ODM_WM_G|ODM_WM_N24G)	:
			{
				if (pDM_Odm->RFType == ODM_1T2R ||pDM_Odm->RFType == ODM_1T1R)
				{
					if(rssi_level == DM_RATR_STA_HIGH)
					{
						rate_bitmap = 0x000f0000;
					}
					else if(rssi_level == DM_RATR_STA_MIDDLE)
					{
						rate_bitmap = 0x000ff000;
					}
					else{
						if (*(pDM_Odm->pBandWidth) == ODM_BW40M)
							rate_bitmap = 0x000ff015;
						else
							rate_bitmap = 0x000ff005;
					}
				}
				else
				{
					if(rssi_level == DM_RATR_STA_HIGH)
					{
						rate_bitmap = 0x0f8f0000;
					}
					else if(rssi_level == DM_RATR_STA_MIDDLE)
					{
						rate_bitmap = 0x0f8ff000;
					}
					else
					{
						if (*(pDM_Odm->pBandWidth) == ODM_BW40M)
							rate_bitmap = 0x0f8ff015;
						else
							rate_bitmap = 0x0f8ff005;
					}
				}
			}
			break;
		default:
		//case WIRELESS_11_24N:
		//case WIRELESS_11_5N:
			if(pDM_Odm->RFType == RF_1T2R)
				rate_bitmap = 0x000fffff;
			else
				rate_bitmap = 0x0fffffff;
			break;

	}

	//printk("%s ==> rssi_level:0x%02x, WirelessMode:0x%02x, rate_bitmap:0x%08x \n",__FUNCTION__,rssi_level,WirelessMode,rate_bitmap);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, (" ==> rssi_level:0x%02x, WirelessMode:0x%02x, rate_bitmap:0x%08x \n",rssi_level,WirelessMode,rate_bitmap));

	return rate_bitmap;

}

/*-----------------------------------------------------------------------------
 * Function:	odm_RefreshRateAdaptiveMask()
 *
 * Overview:	Update rate table mask according to rssi
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	05/27/2009	hpfan	Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void
odm_RefreshRateAdaptiveMask(
		PDM_ODM_T		pDM_Odm
	)
{
	if (!(pDM_Odm->SupportAbility & ODM_BB_RA_MASK))
		return;
	//
	// 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	// at the same time. In the stage2/3, we need to prive universal interface and merge all
	// HW dynamic mechanism.
	//
	switch	(pDM_Odm->SupportPlatform)
	{
		case	ODM_MP:
			odm_RefreshRateAdaptiveMaskMP(pDM_Odm);
			break;

		case	ODM_CE:
			odm_RefreshRateAdaptiveMaskCE(pDM_Odm);
			break;

		case	ODM_AP:
		case	ODM_ADSL:
			odm_RefreshRateAdaptiveMaskAPADSL(pDM_Odm);
			break;
	}

}

void
odm_RefreshRateAdaptiveMaskMP(
		PDM_ODM_T		pDM_Odm
	)
{
}


void
odm_RefreshRateAdaptiveMaskCE(
		PDM_ODM_T		pDM_Odm
	)
{
	u8	i;
	struct rtw_adapter *	pAdapter	 =  pDM_Odm->Adapter;

	if(pAdapter->bDriverStopped)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("<---- odm_RefreshRateAdaptiveMask(): driver is going to unload\n"));
		return;
	}

	if(!pDM_Odm->bUseRAMask)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("<---- odm_RefreshRateAdaptiveMask(): driver does not control rate adaptive mask\n"));
		return;
	}

	//printk("==> %s \n",__FUNCTION__);

	for(i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++){
		PSTA_INFO_T pstat = pDM_Odm->pODM_StaInfo[i];
		if(IS_STA_VALID(pstat) ) {
			if( TRUE == ODM_RAStateCheck(pDM_Odm, pstat->rssi_stat.UndecoratedSmoothedPWDB, FALSE , &pstat->rssi_level) )
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI:%d, RSSI_LEVEL:%d\n", pstat->rssi_stat.UndecoratedSmoothedPWDB, pstat->rssi_level));
				//printk("RSSI:%d, RSSI_LEVEL:%d\n", pstat->rssi_stat.UndecoratedSmoothedPWDB, pstat->rssi_level);
				rtw_hal_update_ra_mask(pstat, pstat->rssi_level);
			}

		}
	}

}

void
odm_RefreshRateAdaptiveMaskAPADSL(
		PDM_ODM_T		pDM_Odm
	)
{
}

// Return Value: bool
// - TRUE: RATRState is changed.
bool
ODM_RAStateCheck(
		PDM_ODM_T		pDM_Odm,
		s32			RSSI,
		bool			bForceUpdate,
		u8 *			pRATRState
	)
{
	PODM_RATE_ADAPTIVE pRA = &pDM_Odm->RateAdaptive;
	const u8 GoUpGap = 5;
	u8 HighRSSIThreshForRA = pRA->HighRSSIThresh;
	u8 LowRSSIThreshForRA = pRA->LowRSSIThresh;
	u8 RATRState;

	// Threshold Adjustment:
	// when RSSI state trends to go up one or two levels, make sure RSSI is high enough.
	// Here GoUpGap is added to solve the boundary's level alternation issue.
	switch (*pRATRState)
	{
		case DM_RATR_STA_INIT:
		case DM_RATR_STA_HIGH:
			break;

		case DM_RATR_STA_MIDDLE:
			HighRSSIThreshForRA += GoUpGap;
			break;

		case DM_RATR_STA_LOW:
			HighRSSIThreshForRA += GoUpGap;
			LowRSSIThreshForRA += GoUpGap;
			break;

		default:
			ODM_RT_ASSERT(pDM_Odm, FALSE, ("wrong rssi level setting %d !", *pRATRState) );
			break;
	}

	// Decide RATRState by RSSI.
	if(RSSI > HighRSSIThreshForRA)
		RATRState = DM_RATR_STA_HIGH;
	else if(RSSI > LowRSSIThreshForRA)
		RATRState = DM_RATR_STA_MIDDLE;
	else
		RATRState = DM_RATR_STA_LOW;
	//printk("==>%s,RATRState:0x%02x ,RSSI:%d \n",__FUNCTION__,RATRState,RSSI);

	if( *pRATRState!=RATRState || bForceUpdate)
	{
		ODM_RT_TRACE( pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI Level %d -> %d\n", *pRATRState, RATRState) );
		*pRATRState = RATRState;
		return TRUE;
	}

	return FALSE;
}


//============================================================

//3============================================================
//3 Dynamic Tx Power
//3============================================================

void
odm_DynamicTxPowerInit(
		PDM_ODM_T		pDM_Odm
	)
{
	struct rtw_adapter *	Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	pdmpriv->bDynamicTxPowerEnable = _FALSE;

	#if (RTL8192C_SUPPORT==1)

	if(pHalData->BoardType == BOARD_USB_High_PA)
	{
		//odm_SavePowerIndex(Adapter);
		odm_DynamicTxPowerSavePowerIndex(pDM_Odm);
		pdmpriv->bDynamicTxPowerEnable = _TRUE;
	}
	else
	#endif

	pdmpriv->LastDTPLvl = TxHighPwrLevel_Normal;
	pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
}

void
odm_DynamicTxPowerSavePowerIndex(
		PDM_ODM_T		pDM_Odm
	)
{
	u8		index;
	u32		Power_Index_REG[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};

	struct rtw_adapter *	Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	for(index = 0; index< 6; index++)
		pdmpriv->PowerIndex_backup[index] = rtw_read8(Adapter, Power_Index_REG[index]);
}

void
odm_DynamicTxPowerRestorePowerIndex(
		PDM_ODM_T		pDM_Odm
	)
{
	u8			index;
	struct rtw_adapter *		Adapter = pDM_Odm->Adapter;

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u32			Power_Index_REG[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	for(index = 0; index< 6; index++)
		rtw_write8(Adapter, Power_Index_REG[index], pdmpriv->PowerIndex_backup[index]);
}

void
odm_DynamicTxPowerWritePowerIndex(
	PDM_ODM_T	pDM_Odm,
	u8		Value)
{

	u8			index;
	u32			Power_Index_REG[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};

	for(index = 0; index< 6; index++)
		//PlatformEFIOWrite1Byte(Adapter, Power_Index_REG[index], Value);
		ODM_Write1Byte(pDM_Odm, Power_Index_REG[index], Value);

}


void
odm_DynamicTxPower(
		PDM_ODM_T		pDM_Odm
	)
{
	//
	// For AP/ADSL use prtl8192cd_priv
	// For CE/NIC use struct rtw_adapter *
	//
	//struct rtw_adapter *		pAdapter = pDM_Odm->Adapter;
//	prtl8192cd_priv	priv		= pDM_Odm->priv;

	if (!(pDM_Odm->SupportAbility & ODM_BB_DYNAMIC_TXPWR))
		return;

	// 2012/01/12 MH According to Luke's suggestion, only high power will support the feature.
	if (pDM_Odm->ExtPA == FALSE)
		return;


	//
	// 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	// at the same time. In the stage2/3, we need to prive universal interface and merge all
	// HW dynamic mechanism.
	//
	switch	(pDM_Odm->SupportPlatform)
	{
		case	ODM_MP:
		case	ODM_CE:
			odm_DynamicTxPowerNIC(pDM_Odm);
			break;
		case	ODM_AP:
			odm_DynamicTxPowerAP(pDM_Odm);
			break;

		case	ODM_ADSL:
			//odm_DIGAP(pDM_Odm);
			break;
	}


}


void
odm_DynamicTxPowerNIC(
		PDM_ODM_T		pDM_Odm
	)
{
	if (!(pDM_Odm->SupportAbility & ODM_BB_DYNAMIC_TXPWR))
		return;

	if(pDM_Odm->SupportICType == ODM_RTL8192C)
	{
		odm_DynamicTxPower_92C(pDM_Odm);
	}
	else if(pDM_Odm->SupportICType == ODM_RTL8192D)
	{
		odm_DynamicTxPower_92D(pDM_Odm);
	}
	else if (pDM_Odm->SupportICType & ODM_RTL8188E)
	{
		// Add Later.
	}
	else if (pDM_Odm->SupportICType == ODM_RTL8188E)
	{
		// ???
		// This part need to be redefined.
	}
}

void
odm_DynamicTxPowerAP(
		PDM_ODM_T		pDM_Odm

	)
{
}


void
odm_DynamicTxPower_92C(
	PDM_ODM_T	pDM_Odm
	)
{
#if (RTL8192C_SUPPORT==1)
	struct rtw_adapter *Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct mlme_priv	*pmlmepriv = &(Adapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &Adapter->mlmeextpriv;
	int	UndecoratedSmoothedPWDB;

	if (!pdmpriv->bDynamicTxPowerEnable)
		return;

	// STA not connected and AP not connected
	if ((check_fwstate(pmlmepriv, _FW_LINKED) != _TRUE) &&
	    (pdmpriv->EntryMinUndecoratedSmoothedPWDB == 0)) {
		//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("Not connected to any \n"));
		pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;

		//the LastDTPlvl should reset when disconnect,
		//otherwise the tx power level wouldn't change when disconnect and connect again.
		// Maddest 20091220.
		pdmpriv->LastDTPLvl=TxHighPwrLevel_Normal;
		return;
	}

	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {	// Default port
		UndecoratedSmoothedPWDB = pdmpriv->EntryMinUndecoratedSmoothedPWDB;
	} else { // associated entry pwdb
		UndecoratedSmoothedPWDB = pdmpriv->EntryMinUndecoratedSmoothedPWDB;
		//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("AP Ext Port PWDB = 0x%x \n", UndecoratedSmoothedPWDB));
	}

	if (UndecoratedSmoothedPWDB >= TX_POWER_NEAR_FIELD_THRESH_LVL2) {
		pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Level2;
		//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_Level1 (TxPwr=0x0)\n"));
	} else if ((UndecoratedSmoothedPWDB < (TX_POWER_NEAR_FIELD_THRESH_LVL2-3)) &&
		   (UndecoratedSmoothedPWDB >= TX_POWER_NEAR_FIELD_THRESH_LVL1) ) {
		pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Level1;
		//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_Level1 (TxPwr=0x10)\n"));
	} else if (UndecoratedSmoothedPWDB < (TX_POWER_NEAR_FIELD_THRESH_LVL1-5)) {
		pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
		//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_Normal\n"));
	}
	if ((pdmpriv->DynamicTxHighPowerLvl != pdmpriv->LastDTPLvl)) {
		PHY_SetTxPowerLevel8192C(Adapter, pHalData->CurrentChannel);
		if(pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_Normal) // HP1 -> Normal  or HP2 -> Normal
			odm_DynamicTxPowerRestorePowerIndex(pDM_Odm);
		else if(pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_Level1)
			odm_DynamicTxPowerWritePowerIndex(pDM_Odm, 0x14);
		else if(pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_Level2)
			odm_DynamicTxPowerWritePowerIndex(pDM_Odm, 0x10);
	}
	pdmpriv->LastDTPLvl = pdmpriv->DynamicTxHighPowerLvl;
#endif
}


void
odm_DynamicTxPower_92D(
	PDM_ODM_T	pDM_Odm
	)
{
#if (RTL8192D_SUPPORT==1)
	struct rtw_adapter *Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct mlme_priv	*pmlmepriv = &(Adapter->mlmepriv);

	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	DM_ODM_T		*podmpriv = &pHalData->odmpriv;
	int	UndecoratedSmoothedPWDB;
	#if (RTL8192D_EASY_SMART_CONCURRENT == 1)
	struct rtw_adapter *	BuddyAdapter = Adapter->BuddyAdapter;
	bool		bGetValueFromBuddyAdapter = DualMacGetParameterFromBuddyAdapter(Adapter);
	u8		HighPowerLvlBackForMac0 = TxHighPwrLevel_Level1;
	#endif

	// If dynamic high power is disabled.
	if( (pdmpriv->bDynamicTxPowerEnable != _TRUE) ||
		(!(podmpriv->SupportAbility& ODM_BB_DYNAMIC_TXPWR)) )
	{
		pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
		return;
	}

	// STA not connected and AP not connected
	if((check_fwstate(pmlmepriv, _FW_LINKED) != _TRUE) &&
		(pdmpriv->EntryMinUndecoratedSmoothedPWDB == 0))
	{
		//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("Not connected to any \n"));
		pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
		//the LastDTPlvl should reset when disconnect,
		//otherwise the tx power level wouldn't change when disconnect and connect again.
		// Maddest 20091220.
		pdmpriv->LastDTPLvl=TxHighPwrLevel_Normal;
		return;
	}

	if(check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE)	// Default port
	{
		UndecoratedSmoothedPWDB = pdmpriv->EntryMinUndecoratedSmoothedPWDB;
	}
	else // associated entry pwdb
	{
		UndecoratedSmoothedPWDB = pdmpriv->EntryMinUndecoratedSmoothedPWDB;
		//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("AP Ext Port PWDB = 0x%x \n", UndecoratedSmoothedPWDB));
	}
#if TX_POWER_FOR_5G_BAND == 1
	if(pHalData->CurrentBandType92D == BAND_ON_5G){
		if(UndecoratedSmoothedPWDB >= 0x33)
		{
			pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Level2;
			//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("5G:TxHighPwrLevel_Level2 (TxPwr=0x0)\n"));
		}
		else if((UndecoratedSmoothedPWDB <0x33) &&
			(UndecoratedSmoothedPWDB >= 0x2b) )
		{
			pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Level1;
			//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("5G:TxHighPwrLevel_Level1 (TxPwr=0x10)\n"));
		}
		else if(UndecoratedSmoothedPWDB < 0x2b)
		{
			pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
			//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("5G:TxHighPwrLevel_Normal\n"));
		}
	}
	else
#endif
	{
		if(UndecoratedSmoothedPWDB >= TX_POWER_NEAR_FIELD_THRESH_LVL2)
		{
			pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Level2;
			//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_Level1 (TxPwr=0x0)\n"));
		}
		else if((UndecoratedSmoothedPWDB < (TX_POWER_NEAR_FIELD_THRESH_LVL2-3)) &&
			(UndecoratedSmoothedPWDB >= TX_POWER_NEAR_FIELD_THRESH_LVL1) )
		{
			pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Level1;
			//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_Level1 (TxPwr=0x10)\n"));
		}
		else if(UndecoratedSmoothedPWDB < (TX_POWER_NEAR_FIELD_THRESH_LVL1-5))
		{
			pdmpriv->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
			//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_Normal\n"));
		}
	}
#if (RTL8192D_EASY_SMART_CONCURRENT == 1)
	if(bGetValueFromBuddyAdapter)
	{
		//ODM_RT_TRACE(pDM_Odm,COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() mac 0 for mac 1 \n"));
		if(Adapter->DualMacDMSPControl.bChangeTxHighPowerLvlForAnotherMacOfDMSP)
		{
			//ODM_RT_TRACE(pDM_Odm,COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() change value \n"));
			HighPowerLvlBackForMac0 = pHalData->DynamicTxHighPowerLvl;
			pHalData->DynamicTxHighPowerLvl = Adapter->DualMacDMSPControl.CurTxHighLvlForAnotherMacOfDMSP;
			PHY_SetTxPowerLevel8192D(Adapter, pHalData->CurrentChannel);
			pHalData->DynamicTxHighPowerLvl = HighPowerLvlBackForMac0;
			Adapter->DualMacDMSPControl.bChangeTxHighPowerLvlForAnotherMacOfDMSP = _FALSE;
		}
	}
#endif

	if( (pdmpriv->DynamicTxHighPowerLvl != pdmpriv->LastDTPLvl) )
	{
		//ODM_RT_TRACE(pDM_Odm,COMP_HIPWR, DBG_LOUD, ("PHY_SetTxPowerLevel8192S() Channel = %d \n" , pHalData->CurrentChannel));
#if (RTL8192D_EASY_SMART_CONCURRENT == 1)
		if(BuddyAdapter == NULL)
		{
			//ODM_RT_TRACE(pDM_Odm,COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() BuddyAdapter == NULL case \n"));
			if(!Adapter->bSlaveOfDMSP)
			{
				PHY_SetTxPowerLevel8192D(Adapter, pHalData->CurrentChannel);
			}
		}
		else
		{
			if(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY)
			{
				//ODM_RT_TRACE(pDM_Odm,COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() BuddyAdapter DMSP \n"));
				if(Adapter->bSlaveOfDMSP)
				{
					//ODM_RT_TRACE(pDM_Odm,COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() bslave case  \n"));
					BuddyAdapter->DualMacDMSPControl.bChangeTxHighPowerLvlForAnotherMacOfDMSP = _TRUE;
					BuddyAdapter->DualMacDMSPControl.CurTxHighLvlForAnotherMacOfDMSP = pHalData->DynamicTxHighPowerLvl;
				}
				else
				{
					//ODM_RT_TRACE(pDM_Odm,COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() master case  \n"));
					if(!bGetValueFromBuddyAdapter)
					{
						//ODM_RT_TRACE(pDM_Odm,COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() mac 0 for mac 0 \n"));
						PHY_SetTxPowerLevel8192D(Adapter, pHalData->CurrentChannel);
					}
				}
			}
			else
			{
				//ODM_RT_TRACE(pDM_Odm,COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() BuddyAdapter DMDP\n"));
				PHY_SetTxPowerLevel8192D(Adapter, pHalData->CurrentChannel);
			}
		}
#else
		PHY_SetTxPowerLevel8192D(Adapter, pHalData->CurrentChannel);
#endif
	}
	pdmpriv->LastDTPLvl = pdmpriv->DynamicTxHighPowerLvl;
#endif
}


//3============================================================
//3 RSSI Monitor
//3============================================================

void
odm_RSSIMonitorInit(
	PDM_ODM_T	pDM_Odm
	)
{
}

void
odm_RSSIMonitorCheck(
		PDM_ODM_T		pDM_Odm
	)
{
	//
	// For AP/ADSL use prtl8192cd_priv
	// For CE/NIC use struct rtw_adapter *
	//
	struct rtw_adapter *		pAdapter = pDM_Odm->Adapter;
	prtl8192cd_priv	priv		= pDM_Odm->priv;

	if (!(pDM_Odm->SupportAbility & ODM_BB_RSSI_MONITOR))
		return;

	//
	// 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	// at the same time. In the stage2/3, we need to prive universal interface and merge all
	// HW dynamic mechanism.
	//
	switch	(pDM_Odm->SupportPlatform)
	{
		case	ODM_MP:
			odm_RSSIMonitorCheckMP(pDM_Odm);
			break;

		case	ODM_CE:
			odm_RSSIMonitorCheckCE(pDM_Odm);
			break;

		case	ODM_AP:
			odm_RSSIMonitorCheckAP(pDM_Odm);
			break;

		case	ODM_ADSL:
			//odm_DIGAP(pDM_Odm);
			break;
	}

}	// odm_RSSIMonitorCheck


void
odm_RSSIMonitorCheckMP(
	PDM_ODM_T	pDM_Odm
	)
{
}

//
//sherry move from DUSC to here 20110517
//
static void
FindMinimumRSSI_Dmsp(
	struct rtw_adapter *	pAdapter
)
{
}

static void
FindMinimumRSSI(
	struct rtw_adapter *	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	PDM_ODM_T		pDM_Odm = &(pHalData->odmpriv);

	//1 1.Determine the minimum RSSI

	if((pDM_Odm->bLinked != _TRUE) &&
		(pdmpriv->EntryMinUndecoratedSmoothedPWDB == 0))
	{
		pdmpriv->MinUndecoratedPWDBForDM = 0;
		//ODM_RT_TRACE(pDM_Odm,COMP_BB_POWERSAVING, DBG_LOUD, ("Not connected to any \n"));
	}
	else
	{
		pdmpriv->MinUndecoratedPWDBForDM = pdmpriv->EntryMinUndecoratedSmoothedPWDB;
	}

	//DBG_8723A("%s=>MinUndecoratedPWDBForDM(%d)\n",__FUNCTION__,pdmpriv->MinUndecoratedPWDBForDM);
	//ODM_RT_TRACE(pDM_Odm,COMP_DIG, DBG_LOUD, ("MinUndecoratedPWDBForDM =%d\n",pHalData->MinUndecoratedPWDBForDM));
}

void
odm_RSSIMonitorCheckCE(
		PDM_ODM_T		pDM_Odm
	)
{
	struct rtw_adapter *	Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	int	i;
	int	tmpEntryMaxPWDB=0, tmpEntryMinPWDB=0xff;
	u8	sta_cnt=0;
	u32 PWDB_rssi[NUM_STA]={0};//[0~15]:MACID, [16~31]:PWDB_rssi

	if(pDM_Odm->bLinked != _TRUE)
		return;

	//if(check_fwstate(&Adapter->mlmepriv, WIFI_AP_STATE|WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE) == _TRUE)
	{
		#if 1
		struct sta_info *psta;

		for(i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++) {
			if (IS_STA_VALID(psta = pDM_Odm->pODM_StaInfo[i]))
                        {
					if(psta->rssi_stat.UndecoratedSmoothedPWDB < tmpEntryMinPWDB)
						tmpEntryMinPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

					if(psta->rssi_stat.UndecoratedSmoothedPWDB > tmpEntryMaxPWDB)
						tmpEntryMaxPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

					if(psta->rssi_stat.UndecoratedSmoothedPWDB != (-1)) {
						#if(RTL8192D_SUPPORT==1)
						PWDB_rssi[sta_cnt++] = (psta->mac_id | (psta->rssi_stat.UndecoratedSmoothedPWDB<<16) | ((Adapter->stapriv.asoc_sta_count+1) << 8));
						#else
						PWDB_rssi[sta_cnt++] = (psta->mac_id | (psta->rssi_stat.UndecoratedSmoothedPWDB<<16) );
						#endif
					}
			}
		}
		#endif

		for(i=0; i< sta_cnt; i++)
		{
			if(PWDB_rssi[i] != (0)){
				if(pHalData->fw_ractrl == _TRUE)// Report every sta's RSSI to FW
				{
					#if(RTL8192D_SUPPORT==1)
					FillH2CCmd92D(Adapter, H2C_RSSI_REPORT, 3, (u8 *)(&PWDB_rssi[i]));
					#elif((RTL8192C_SUPPORT==1)||(RTL8723A_SUPPORT==1))
					rtl8192c_set_rssi_cmd(Adapter, (u8*)&PWDB_rssi[i]);
					#endif
				}
				else{
					#if((RTL8188E_SUPPORT==1)&&(RATE_ADAPTIVE_SUPPORT == 1))
					ODM_RA_SetRSSI_8188E(
					&(pHalData->odmpriv), (PWDB_rssi[i]&0xFF), (u8)((PWDB_rssi[i]>>16) & 0xFF));
					#endif
				}
			}
		}
	}

	if(tmpEntryMaxPWDB != 0)	// If associated entry is found
	{
		pdmpriv->EntryMaxUndecoratedSmoothedPWDB = tmpEntryMaxPWDB;
	}
	else
	{
		pdmpriv->EntryMaxUndecoratedSmoothedPWDB = 0;
	}

	if(tmpEntryMinPWDB != 0xff) // If associated entry is found
	{
		pdmpriv->EntryMinUndecoratedSmoothedPWDB = tmpEntryMinPWDB;
	}
	else
	{
		pdmpriv->EntryMinUndecoratedSmoothedPWDB = 0;
	}

	FindMinimumRSSI(Adapter);//get pdmpriv->MinUndecoratedPWDBForDM

	#if(RTL8192D_SUPPORT==1)
	FindMinimumRSSI_Dmsp(Adapter);
	#endif

	ODM_CmnInfoUpdate(&pHalData->odmpriv ,ODM_CMNINFO_RSSI_MIN, pdmpriv->MinUndecoratedPWDBForDM);
}
void
odm_RSSIMonitorCheckAP(
		PDM_ODM_T		pDM_Odm
	)
{
}


void
ODM_InitAllTimers(
 PDM_ODM_T	pDM_Odm
	)
{
	ODM_InitializeTimer(pDM_Odm,&pDM_Odm->DM_SWAT_Table.SwAntennaSwitchTimer,
		(RT_TIMER_CALL_BACK)odm_SwAntDivChkAntSwitchCallback, NULL, "SwAntennaSwitchTimer");
}

void
ODM_CancelAllTimers(
 PDM_ODM_T	pDM_Odm
	)
{
	ODM_CancelTimer(pDM_Odm,&pDM_Odm->DM_SWAT_Table.SwAntennaSwitchTimer);
}


void
ODM_ReleaseAllTimers(
 PDM_ODM_T	pDM_Odm
	)
{
	ODM_ReleaseTimer(pDM_Odm,&pDM_Odm->DM_SWAT_Table.SwAntennaSwitchTimer);

#if (RTL8188E_SUPPORT == 1)
	ODM_ReleaseTimer(pDM_Odm,&pDM_Odm->FastAntTrainingTimer);
#endif

}



//#endif
//3============================================================
//3 Tx Power Tracking
//3============================================================

void
odm_TXPowerTrackingInit(
	PDM_ODM_T	pDM_Odm
	)
{
	odm_TXPowerTrackingThermalMeterInit(pDM_Odm);
}


void
odm_TXPowerTrackingThermalMeterInit(
	PDM_ODM_T	pDM_Odm
	)
{
	#ifdef CONFIG_RTL8188E
	{
		pDM_Odm->RFCalibrateInfo.bTXPowerTracking = _TRUE;
		pDM_Odm->RFCalibrateInfo.TXPowercount = 0;
		pDM_Odm->RFCalibrateInfo.bTXPowerTrackingInit = _FALSE;
		pDM_Odm->RFCalibrateInfo.TxPowerTrackControl = _TRUE;
		MSG_8723A("pDM_Odm TxPowerTrackControl = %d\n", pDM_Odm->RFCalibrateInfo.TxPowerTrackControl);
	}
	#else
	{
		struct rtw_adapter *		Adapter = pDM_Odm->Adapter;
		HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
		struct dm_priv	*pdmpriv = &pHalData->dmpriv;

		pdmpriv->bTXPowerTracking = _TRUE;
		pdmpriv->TXPowercount = 0;
		pdmpriv->bTXPowerTrackingInit = _FALSE;
		pdmpriv->TxPowerTrackControl = _TRUE;
		MSG_8723A("pdmpriv->TxPowerTrackControl = %d\n", pdmpriv->TxPowerTrackControl);

	}
	#endif//endif (CONFIG_RTL8188E==1)

    pDM_Odm->RFCalibrateInfo.TxPowerTrackControl = TRUE;
}


void
ODM_TXPowerTrackingCheck(
		PDM_ODM_T		pDM_Odm
	)
{
	//
	// For AP/ADSL use prtl8192cd_priv
	// For CE/NIC use struct rtw_adapter *
	//
	struct rtw_adapter *		pAdapter = pDM_Odm->Adapter;
	prtl8192cd_priv	priv		= pDM_Odm->priv;

	//if (!(pDM_Odm->SupportAbility & ODM_RF_TX_PWR_TRACK))
		//return;

	//
	// 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	// at the same time. In the stage2/3, we need to prive universal interface and merge all
	// HW dynamic mechanism.
	//
	switch	(pDM_Odm->SupportPlatform)
	{
		case	ODM_MP:
			odm_TXPowerTrackingCheckMP(pDM_Odm);
			break;

		case	ODM_CE:
			odm_TXPowerTrackingCheckCE(pDM_Odm);
			break;

		case	ODM_AP:
			odm_TXPowerTrackingCheckAP(pDM_Odm);
			break;

		case	ODM_ADSL:
			//odm_DIGAP(pDM_Odm);
			break;
	}

}

void
odm_TXPowerTrackingCheckCE(
		PDM_ODM_T		pDM_Odm
	)
{
	struct rtw_adapter *	Adapter = pDM_Odm->Adapter;
	#if( (RTL8192C_SUPPORT==1) ||  (RTL8723A_SUPPORT==1) )
	rtl8192c_odm_CheckTXPowerTracking(Adapter);
	#endif

	#if (RTL8192D_SUPPORT==1)
	#if (RTL8192D_EASY_SMART_CONCURRENT == 1)
	if(!Adapter->bSlaveOfDMSP)
	#endif
		rtl8192d_odm_CheckTXPowerTracking(Adapter);
	#endif
	#if(RTL8188E_SUPPORT==1)

	//if(!pMgntInfo->bTXPowerTracking /*|| (!pdmpriv->TxPowerTrackControl && pdmpriv->bAPKdone)*/)
	if(!(pDM_Odm->SupportAbility & ODM_RF_TX_PWR_TRACK))
	{
		return;
	}

	if(!pDM_Odm->RFCalibrateInfo.TM_Trigger)		//at least delay 1 sec
	{
		//pHalData->TxPowerCheckCnt++;	//cosa add for debug
		//ODM_SetRFReg(pDM_Odm, RF_PATH_A, RF_T_METER, bRFRegOffsetMask, 0x60);
		PHY_SetRFReg(Adapter, RF_PATH_A, RF_T_METER_88E, BIT17 | BIT16, 0x03);
		//DBG_8723A("Trigger 92C Thermal Meter!!\n");

		pDM_Odm->RFCalibrateInfo.TM_Trigger = 1;
		return;

	}
	else
	{
		//DBG_8723A("Schedule TxPowerTracking direct call!!\n");
		odm_TXPowerTrackingCallback_ThermalMeter_8188E(Adapter);
		pDM_Odm->RFCalibrateInfo.TM_Trigger = 0;
	}
	#endif
}

void
odm_TXPowerTrackingCheckMP(
		PDM_ODM_T		pDM_Odm
	)
{
}

void
odm_TXPowerTrackingCheckAP(
		PDM_ODM_T		pDM_Odm
	)
{
}



//antenna mapping info
// 1: right-side antenna
// 2/0: left-side antenna
//PDM_SWAT_Table->CCK_Ant1_Cnt /OFDM_Ant1_Cnt:  for right-side antenna:   Ant:1    RxDefaultAnt1
//PDM_SWAT_Table->CCK_Ant2_Cnt /OFDM_Ant2_Cnt:  for left-side antenna:     Ant:0    RxDefaultAnt2
// We select left antenna as default antenna in initial process, modify it as needed
//


//3============================================================
//3 SW Antenna Diversity
//3============================================================
#if(defined(CONFIG_SW_ANTENNA_DIVERSITY))
void
odm_SwAntDivInit(
		PDM_ODM_T		pDM_Odm
	)
{
	odm_SwAntDivInit_NIC(pDM_Odm);
}
#if (RTL8723A_SUPPORT==1)
// Only for 8723A SW ANT DIV INIT--2012--07--17
void
odm_SwAntDivInit_NIC_8723A(
	PDM_ODM_T		pDM_Odm)
{
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	struct rtw_adapter *		Adapter = pDM_Odm->Adapter;
	u8			btAntNum=BT_GetPGAntNum(Adapter);


	pDM_SWAT_Table->ANTA_ON =TRUE;

	// Set default antenna B status by PG
	if(btAntNum == Ant_x2)
		pDM_SWAT_Table->ANTB_ON = TRUE;
	else if(btAntNum ==Ant_x1)
		pDM_SWAT_Table->ANTB_ON = FALSE;
	else
		pDM_SWAT_Table->ANTB_ON = TRUE;
}
#endif
void
odm_SwAntDivInit_NIC(
		PDM_ODM_T		pDM_Odm
	)
{
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
// Init SW ANT DIV mechanism for 8723AE/AU/AS// Neil Chen--2012--07--17---
// CE/AP/ADSL no using SW ANT DIV for 8723A Series IC
//#if (DM_ODM_SUPPORT_TYPE==ODM_MP)
#if (RTL8723A_SUPPORT==1)
	if(pDM_Odm->SupportICType == ODM_RTL8723A)
	{
		odm_SwAntDivInit_NIC_8723A(pDM_Odm);
	}
#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS:Init SW Antenna Switch\n"));
	pDM_SWAT_Table->RSSI_sum_A = 0;
	pDM_SWAT_Table->RSSI_cnt_A = 0;
	pDM_SWAT_Table->RSSI_sum_B = 0;
	pDM_SWAT_Table->RSSI_cnt_B = 0;
	pDM_SWAT_Table->CurAntenna = Antenna_A;
	pDM_SWAT_Table->PreAntenna = Antenna_A;
	pDM_SWAT_Table->try_flag = 0xff;
	pDM_SWAT_Table->PreRSSI = 0;
	pDM_SWAT_Table->SWAS_NoLink_State = 0;
	pDM_SWAT_Table->bTriggerAntennaSwitch = 0;
	pDM_SWAT_Table->SelectAntennaMap=0xAA;
	pDM_SWAT_Table->lastTxOkCnt = 0;
	pDM_SWAT_Table->lastRxOkCnt = 0;
	pDM_SWAT_Table->TXByteCnt_A = 0;
	pDM_SWAT_Table->TXByteCnt_B = 0;
	pDM_SWAT_Table->RXByteCnt_A = 0;
	pDM_SWAT_Table->RXByteCnt_B = 0;
	pDM_SWAT_Table->TrafficLoad = TRAFFIC_LOW;
	pDM_SWAT_Table->SWAS_NoLink_BK_Reg860 = ODM_Read4Byte(pDM_Odm, 0x860);
}

//
// 20100514 Joseph:
// Add new function to reset the state of antenna diversity before link.
//
void
ODM_SwAntDivResetBeforeLink(
		PDM_ODM_T		pDM_Odm
	)
{

	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;

	pDM_SWAT_Table->SWAS_NoLink_State = 0;

}

//
// 20100514 Luke/Joseph:
// Add new function to reset antenna diversity state after link.
//
void
ODM_SwAntDivRestAfterLink(
 PDM_ODM_T	pDM_Odm
	)
{
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;

	pDM_SWAT_Table->RSSI_cnt_A = 0;
	pDM_SWAT_Table->RSSI_cnt_B = 0;
	pDM_Odm->RSSI_test = FALSE;
	pDM_SWAT_Table->try_flag = 0xff;
	pDM_SWAT_Table->RSSI_Trying = 0;
	pDM_SWAT_Table->SelectAntennaMap=0xAA;
}

void
ODM_SwAntDivChkPerPktRssi(
 PDM_ODM_T	pDM_Odm,
 u8		StationID,
 PODM_PHY_INFO_T pPhyInfo
	)
{
	SWAT_T		*pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;

	if(!(pDM_Odm->SupportAbility & (ODM_BB_ANT_DIV)))
		return;

	if(StationID == pDM_SWAT_Table->RSSI_target)
	{
		//1 RSSI for SW Antenna Switch
		if(pDM_SWAT_Table->CurAntenna == Antenna_A)
		{
			pDM_SWAT_Table->RSSI_sum_A += pPhyInfo->RxPWDBAll;
			pDM_SWAT_Table->RSSI_cnt_A++;
		}
		else
		{
			pDM_SWAT_Table->RSSI_sum_B += pPhyInfo->RxPWDBAll;
			pDM_SWAT_Table->RSSI_cnt_B++;

		}
	}

}

//
void
odm_SwAntDivChkAntSwitch(
		PDM_ODM_T		pDM_Odm,
		u8			Step
	)
{
	//
	// For AP/ADSL use prtl8192cd_priv
	// For CE/NIC use struct rtw_adapter *
	//
	struct rtw_adapter *		pAdapter = pDM_Odm->Adapter;
	prtl8192cd_priv	priv		= pDM_Odm->priv;

	//
	// 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	// at the same time. In the stage2/3, we need to prive universal interface and merge all
	// HW dynamic mechanism.
	//
	switch	(pDM_Odm->SupportPlatform)
	{
		case	ODM_MP:
		case	ODM_CE:
			odm_SwAntDivChkAntSwitchNIC(pDM_Odm, Step);
			break;

		case	ODM_AP:
		case	ODM_ADSL:
			break;
	}

}

//
// 20100514 Luke/Joseph:
// Add new function for antenna diversity after link.
// This is the main function of antenna diversity after link.
// This function is called in HalDmWatchDog() and ODM_SwAntDivChkAntSwitchCallback().
// HalDmWatchDog() calls this function with SWAW_STEP_PEAK to initialize the antenna test.
// In SWAW_STEP_PEAK, another antenna and a 500ms timer will be set for testing.
// After 500ms, ODM_SwAntDivChkAntSwitchCallback() calls this function to compare the signal just
// listened on the air with the RSSI of original antenna.
// It chooses the antenna with better RSSI.
// There is also a aged policy for error trying. Each error trying will cost more 5 seconds waiting
// penalty to get next try.


void
ODM_SetAntenna(
	PDM_ODM_T	pDM_Odm,
	u8		Antenna)
{
	ODM_SetBBReg(pDM_Odm, 0x860, BIT8|BIT9, Antenna);
}
//--------------------------------2012--09--06--
//Note: Antenna_Main--> Antenna_A
//        Antenna_Aux---> Antenna_B
//----------------------------------
void
odm_SwAntDivChkAntSwitchNIC(
		PDM_ODM_T		pDM_Odm,
		u8		Step
	)
{
	//PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	//HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	s32			curRSSI=100, RSSI_A, RSSI_B;
	u8			nextAntenna=Antenna_B;
	//static u64		lastTxOkCnt=0, lastRxOkCnt=0;
	u64			curTxOkCnt, curRxOkCnt;
	//static u64		TXByteCnt_A=0, TXByteCnt_B=0, RXByteCnt_A=0, RXByteCnt_B=0;
	u64			CurByteCnt=0, PreByteCnt=0;
	//static u8		TrafficLoad = TRAFFIC_LOW;
	u8			Score_A=0, Score_B=0;
	u8			i;

	if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
		return;

	if (pDM_Odm->SupportICType & (ODM_RTL8192D|ODM_RTL8188E))
		return;

	if((pDM_Odm->SupportICType == ODM_RTL8192C) &&(pDM_Odm->RFType == ODM_2T2R))
		return;

	if(pDM_Odm->SupportPlatform & ODM_MP)
	{
		if(*(pDM_Odm->pAntennaTest))
			return;
	}

	if((pDM_SWAT_Table->ANTA_ON == FALSE) ||(pDM_SWAT_Table->ANTB_ON == FALSE))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,
				("odm_SwAntDivChkAntSwitch(): No AntDiv Mechanism, Antenna A or B is off\n"));
		return;
	}

	// Radio off: Status reset to default and return.
	if(*(pDM_Odm->pbPowerSaving)==TRUE) //pHalData->eRFPowerState==eRfOff
	{
		ODM_SwAntDivRestAfterLink(pDM_Odm);
		return;
	}


	// Handling step mismatch condition.
	// Peak step is not finished at last time. Recover the variable and check again.
	if(	Step != pDM_SWAT_Table->try_flag	)
	{
		ODM_SwAntDivRestAfterLink(pDM_Odm);
	}

	if(pDM_SWAT_Table->try_flag == 0xff)
	{
		pDM_SWAT_Table->RSSI_target = 0xff;

		{
			u8			index = 0;
			PSTA_INFO_T		pEntry = NULL;


			for(index=0; index<ODM_ASSOCIATE_ENTRY_NUM; index++)
			{
				pEntry =  pDM_Odm->pODM_StaInfo[index];
				if(IS_STA_VALID(pEntry) ) {
					break;
				}
			}
			if(pEntry == NULL)
			{
				ODM_SwAntDivRestAfterLink(pDM_Odm);
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_SwAntDivChkAntSwitch(): No Link.\n"));
				return;
			}
			else
			{
				pDM_SWAT_Table->RSSI_target = index;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_SwAntDivChkAntSwitch(): RSSI_target is PEER STA\n"));
			}
                }

		pDM_SWAT_Table->RSSI_cnt_A = 0;
		pDM_SWAT_Table->RSSI_cnt_B = 0;
		pDM_SWAT_Table->try_flag = 0;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("odm_SwAntDivChkAntSwitch(): Set try_flag to 0 prepare for peak!\n"));
		return;
	}
	else
	{
		curTxOkCnt = *(pDM_Odm->pNumTxBytesUnicast) - pDM_SWAT_Table->lastTxOkCnt;
		curRxOkCnt = *(pDM_Odm->pNumRxBytesUnicast) - pDM_SWAT_Table->lastRxOkCnt;
		pDM_SWAT_Table->lastTxOkCnt = *(pDM_Odm->pNumTxBytesUnicast);
		pDM_SWAT_Table->lastRxOkCnt = *(pDM_Odm->pNumRxBytesUnicast);
             ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("curTxOkCnt = %lld\n",curTxOkCnt));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("curRxOkCnt = %lld\n",curRxOkCnt));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("lastTxOkCnt = %lld\n",pDM_SWAT_Table->lastTxOkCnt));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("lastRxOkCnt = %lld\n",pDM_SWAT_Table->lastRxOkCnt));

		if(pDM_SWAT_Table->try_flag == 1)
		{
			if(pDM_SWAT_Table->CurAntenna == Antenna_A)
			{
				pDM_SWAT_Table->TXByteCnt_A += curTxOkCnt;
				pDM_SWAT_Table->RXByteCnt_A += curRxOkCnt;
			}
			else
			{
				pDM_SWAT_Table->TXByteCnt_B += curTxOkCnt;
				pDM_SWAT_Table->RXByteCnt_B += curRxOkCnt;
			}

			nextAntenna = (pDM_SWAT_Table->CurAntenna == Antenna_A)? Antenna_B : Antenna_A;
			pDM_SWAT_Table->RSSI_Trying--;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("RSSI_Trying = %d\n",pDM_SWAT_Table->RSSI_Trying));
			if(pDM_SWAT_Table->RSSI_Trying == 0)
			{
				CurByteCnt = (pDM_SWAT_Table->CurAntenna == Antenna_A)? (pDM_SWAT_Table->TXByteCnt_A+pDM_SWAT_Table->RXByteCnt_A) : (pDM_SWAT_Table->TXByteCnt_B+pDM_SWAT_Table->RXByteCnt_B);
				PreByteCnt = (pDM_SWAT_Table->CurAntenna == Antenna_A)? (pDM_SWAT_Table->TXByteCnt_B+pDM_SWAT_Table->RXByteCnt_B) : (pDM_SWAT_Table->TXByteCnt_A+pDM_SWAT_Table->RXByteCnt_A);

				if(pDM_SWAT_Table->TrafficLoad == TRAFFIC_HIGH)
					//CurByteCnt = PlatformDivision64(CurByteCnt, 9);
					PreByteCnt = PreByteCnt*9;
				else if(pDM_SWAT_Table->TrafficLoad == TRAFFIC_LOW)
					//CurByteCnt = PlatformDivision64(CurByteCnt, 2);
					PreByteCnt = PreByteCnt*2;

				if(pDM_SWAT_Table->RSSI_cnt_A > 0)
					RSSI_A = pDM_SWAT_Table->RSSI_sum_A/pDM_SWAT_Table->RSSI_cnt_A;
				else
					RSSI_A = 0;
				if(pDM_SWAT_Table->RSSI_cnt_B > 0)
					RSSI_B = pDM_SWAT_Table->RSSI_sum_B/pDM_SWAT_Table->RSSI_cnt_B;
				else
					RSSI_B = 0;
				curRSSI = (pDM_SWAT_Table->CurAntenna == Antenna_A)? RSSI_A : RSSI_B;
				pDM_SWAT_Table->PreRSSI =  (pDM_SWAT_Table->CurAntenna == Antenna_A)? RSSI_B : RSSI_A;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Luke:PreRSSI = %d, CurRSSI = %d\n",pDM_SWAT_Table->PreRSSI, curRSSI));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS: preAntenna= %s, curAntenna= %s \n",
				(pDM_SWAT_Table->PreAntenna == Antenna_A?"A":"B"), (pDM_SWAT_Table->CurAntenna == Antenna_A?"A":"B")));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Luke:RSSI_A= %d, RSSI_cnt_A = %d, RSSI_B= %d, RSSI_cnt_B = %d\n",
					RSSI_A, pDM_SWAT_Table->RSSI_cnt_A, RSSI_B, pDM_SWAT_Table->RSSI_cnt_B));
			}

		}
		else
		{

			if(pDM_SWAT_Table->RSSI_cnt_A > 0)
				RSSI_A = pDM_SWAT_Table->RSSI_sum_A/pDM_SWAT_Table->RSSI_cnt_A;
			else
				RSSI_A = 0;
			if(pDM_SWAT_Table->RSSI_cnt_B > 0)
				RSSI_B = pDM_SWAT_Table->RSSI_sum_B/pDM_SWAT_Table->RSSI_cnt_B;
			else
				RSSI_B = 0;
			curRSSI = (pDM_SWAT_Table->CurAntenna == Antenna_A)? RSSI_A : RSSI_B;
			pDM_SWAT_Table->PreRSSI =  (pDM_SWAT_Table->PreAntenna == Antenna_A)? RSSI_A : RSSI_B;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Ekul:PreRSSI = %d, CurRSSI = %d\n", pDM_SWAT_Table->PreRSSI, curRSSI));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS: preAntenna= %s, curAntenna= %s \n",
			(pDM_SWAT_Table->PreAntenna == Antenna_A?"A":"B"), (pDM_SWAT_Table->CurAntenna == Antenna_A?"A":"B")));

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Ekul:RSSI_A= %d, RSSI_cnt_A = %d, RSSI_B= %d, RSSI_cnt_B = %d\n",
				RSSI_A, pDM_SWAT_Table->RSSI_cnt_A, RSSI_B, pDM_SWAT_Table->RSSI_cnt_B));
			//RT_TRACE(COMP_SWAS, DBG_LOUD, ("Ekul:curTxOkCnt = %d\n", curTxOkCnt));
			//RT_TRACE(COMP_SWAS, DBG_LOUD, ("Ekul:curRxOkCnt = %d\n", curRxOkCnt));
		}

		//1 Trying State
		if((pDM_SWAT_Table->try_flag == 1)&&(pDM_SWAT_Table->RSSI_Trying == 0))
		{

			if(pDM_SWAT_Table->TestMode == TP_MODE)
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS: TestMode = TP_MODE"));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("TRY:CurByteCnt = %lld,", CurByteCnt));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("TRY:PreByteCnt = %lld\n",PreByteCnt));
				if(CurByteCnt < PreByteCnt)
				{
					if(pDM_SWAT_Table->CurAntenna == Antenna_A)
						pDM_SWAT_Table->SelectAntennaMap=pDM_SWAT_Table->SelectAntennaMap<<1;
					else
						pDM_SWAT_Table->SelectAntennaMap=(pDM_SWAT_Table->SelectAntennaMap<<1)+1;
				}
				else
				{
					if(pDM_SWAT_Table->CurAntenna == Antenna_A)
						pDM_SWAT_Table->SelectAntennaMap=(pDM_SWAT_Table->SelectAntennaMap<<1)+1;
					else
						pDM_SWAT_Table->SelectAntennaMap=pDM_SWAT_Table->SelectAntennaMap<<1;
				}
				for (i= 0; i<8; i++)
				{
					if(((pDM_SWAT_Table->SelectAntennaMap>>i)&BIT0) == 1)
						Score_A++;
					else
						Score_B++;
				}
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SelectAntennaMap=%x\n ",pDM_SWAT_Table->SelectAntennaMap));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Score_A=%d, Score_B=%d\n", Score_A, Score_B));

				if(pDM_SWAT_Table->CurAntenna == Antenna_A)
				{
					nextAntenna = (Score_A > Score_B)?Antenna_A:Antenna_B;
				}
				else
				{
					nextAntenna = (Score_B > Score_A)?Antenna_B:Antenna_A;
				}
				//RT_TRACE(COMP_SWAS, DBG_LOUD, ("nextAntenna=%s\n",(nextAntenna==Antenna_A)?"A":"B"));
				//RT_TRACE(COMP_SWAS, DBG_LOUD, ("preAntenna= %s, curAntenna= %s \n",
				//(DM_SWAT_Table.PreAntenna == Antenna_A?"A":"B"), (DM_SWAT_Table.CurAntenna == Antenna_A?"A":"B")));

				if(nextAntenna != pDM_SWAT_Table->CurAntenna)
				{
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS: Switch back to another antenna"));
				}
				else
				{
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS: current anntena is good\n"));
				}
			}

			if(pDM_SWAT_Table->TestMode == RSSI_MODE)
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS: TestMode = RSSI_MODE"));
				pDM_SWAT_Table->SelectAntennaMap=0xAA;
				if(curRSSI < pDM_SWAT_Table->PreRSSI) //Current antenna is worse than previous antenna
				{
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS: Switch back to another antenna"));
					nextAntenna = (pDM_SWAT_Table->CurAntenna == Antenna_A)? Antenna_B : Antenna_A;
				}
				else // current anntena is good
				{
					nextAntenna =pDM_SWAT_Table->CurAntenna;
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS: current anntena is good\n"));
				}
			}
			pDM_SWAT_Table->try_flag = 0;
			pDM_Odm->RSSI_test = FALSE;
			pDM_SWAT_Table->RSSI_sum_A = 0;
			pDM_SWAT_Table->RSSI_cnt_A = 0;
			pDM_SWAT_Table->RSSI_sum_B = 0;
			pDM_SWAT_Table->RSSI_cnt_B = 0;
			pDM_SWAT_Table->TXByteCnt_A = 0;
			pDM_SWAT_Table->TXByteCnt_B = 0;
			pDM_SWAT_Table->RXByteCnt_A = 0;
			pDM_SWAT_Table->RXByteCnt_B = 0;

		}

		//1 Normal State
		else if(pDM_SWAT_Table->try_flag == 0)
		{
			if(pDM_SWAT_Table->TrafficLoad == TRAFFIC_HIGH)
			{
				if ((curTxOkCnt+curRxOkCnt) > 3750000)//if(PlatformDivision64(curTxOkCnt+curRxOkCnt, 2) > 1875000)
					pDM_SWAT_Table->TrafficLoad = TRAFFIC_HIGH;
				else
					pDM_SWAT_Table->TrafficLoad = TRAFFIC_LOW;
			}
			else if(pDM_SWAT_Table->TrafficLoad == TRAFFIC_LOW)
				{
				if ((curTxOkCnt+curRxOkCnt) > 3750000) //if(PlatformDivision64(curTxOkCnt+curRxOkCnt, 2) > 1875000)
					pDM_SWAT_Table->TrafficLoad = TRAFFIC_HIGH;
				else
					pDM_SWAT_Table->TrafficLoad = TRAFFIC_LOW;
			}
			if(pDM_SWAT_Table->TrafficLoad == TRAFFIC_HIGH)
				pDM_SWAT_Table->bTriggerAntennaSwitch = 0;
			//RT_TRACE(COMP_SWAS, DBG_LOUD, ("Normal:TrafficLoad = %llu\n", curTxOkCnt+curRxOkCnt));

			//Prepare To Try Antenna
					nextAntenna = (pDM_SWAT_Table->CurAntenna == Antenna_A)? Antenna_B : Antenna_A;
					pDM_SWAT_Table->try_flag = 1;
					pDM_Odm->RSSI_test = TRUE;
			if((curRxOkCnt+curTxOkCnt) > 1000)
			{
				pDM_SWAT_Table->RSSI_Trying = 4;
				pDM_SWAT_Table->TestMode = TP_MODE;
				}
				else
				{
				pDM_SWAT_Table->RSSI_Trying = 2;
				pDM_SWAT_Table->TestMode = RSSI_MODE;

			}
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS: Normal State -> Begin Trying!\n"));


			pDM_SWAT_Table->RSSI_sum_A = 0;
			pDM_SWAT_Table->RSSI_cnt_A = 0;
			pDM_SWAT_Table->RSSI_sum_B = 0;
			pDM_SWAT_Table->RSSI_cnt_B = 0;
		}
	}

	//1 4.Change TRX antenna
	if(nextAntenna != pDM_SWAT_Table->CurAntenna)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SWAS: Change TX Antenna!\n "));
		//PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, nextAntenna);
		{
			bool bEnqueue;
			bEnqueue = (pDM_Odm->SupportInterface ==  ODM_ITRF_PCIE)?FALSE :TRUE;
			rtw_antenna_select_cmd(pDM_Odm->Adapter, nextAntenna, bEnqueue);
		}
	}

	//1 5.Reset Statistics
	pDM_SWAT_Table->PreAntenna = pDM_SWAT_Table->CurAntenna;
	pDM_SWAT_Table->CurAntenna = nextAntenna;
	pDM_SWAT_Table->PreRSSI = curRSSI;

	//1 6.Set next timer
	{
		struct rtw_adapter *		pAdapter = pDM_Odm->Adapter;
		HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);


	if(pDM_SWAT_Table->RSSI_Trying == 0)
		return;

	if(pDM_SWAT_Table->RSSI_Trying%2 == 0)
	{
		if(pDM_SWAT_Table->TestMode == TP_MODE)
		{
			if(pDM_SWAT_Table->TrafficLoad == TRAFFIC_HIGH)
			{
				//PlatformSetTimer( pAdapter, &pHalData->SwAntennaSwitchTimer, 10 ); //ms
				ODM_SetTimer(pDM_Odm,&pDM_SWAT_Table->SwAntennaSwitchTimer, 10 ); //ms

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("dm_SW_AntennaSwitch(): Test another antenna for 10 ms\n"));
			}
			else if(pDM_SWAT_Table->TrafficLoad == TRAFFIC_LOW)
			{
				//PlatformSetTimer( pAdapter, &pHalData->SwAntennaSwitchTimer, 50 ); //ms
				ODM_SetTimer(pDM_Odm,&pDM_SWAT_Table->SwAntennaSwitchTimer, 50 ); //ms
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("dm_SW_AntennaSwitch(): Test another antenna for 50 ms\n"));
			}
		}
		else
		{
			//PlatformSetTimer( pAdapter, &pHalData->SwAntennaSwitchTimer, 500 ); //ms
			ODM_SetTimer(pDM_Odm,&pDM_SWAT_Table->SwAntennaSwitchTimer, 500 ); //ms
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("dm_SW_AntennaSwitch(): Test another antenna for 500 ms\n"));
		}
	}
	else
	{
		if(pDM_SWAT_Table->TestMode == TP_MODE)
		{
			if(pDM_SWAT_Table->TrafficLoad == TRAFFIC_HIGH)
				//PlatformSetTimer( pAdapter, &pHalData->SwAntennaSwitchTimer, 90 ); //ms
				ODM_SetTimer(pDM_Odm,&pDM_SWAT_Table->SwAntennaSwitchTimer, 90 ); //ms
			else if(pDM_SWAT_Table->TrafficLoad == TRAFFIC_LOW)
				//PlatformSetTimer( pAdapter, &pHalData->SwAntennaSwitchTimer, 100 ); //ms
				ODM_SetTimer(pDM_Odm,&pDM_SWAT_Table->SwAntennaSwitchTimer, 100 ); //ms
		}
		else
			//PlatformSetTimer( pAdapter, &pHalData->SwAntennaSwitchTimer, 500 ); //ms
			ODM_SetTimer(pDM_Odm,&pDM_SWAT_Table->SwAntennaSwitchTimer, 500 ); //ms
	}
	}
}


//
// 20100514 Luke/Joseph:
// Callback function for 500ms antenna test trying.
//
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void odm_SwAntDivChkAntSwitchCallback(void *FunctionContext)
#else
void odm_SwAntDivChkAntSwitchCallback(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	PDM_ODM_T	pDM_Odm= (PDM_ODM_T)FunctionContext;
#else
	PDM_ODM_T pDM_Odm = from_timer(pDM_Odm, t, DM_SWAT_Table.SwAntennaSwitchTimer);
#endif
	struct rtw_adapter *	padapter = pDM_Odm->Adapter;
	if(padapter->net_closed == _TRUE)
	    return;
	odm_SwAntDivChkAntSwitch(pDM_Odm, SWAW_STEP_DETERMINE);
}

#else //#if(defined(CONFIG_SW_ANTENNA_DIVERSITY))

void odm_SwAntDivInit(		PDM_ODM_T		pDM_Odm	) {}
void ODM_SwAntDivChkPerPktRssi(
 PDM_ODM_T	pDM_Odm,
 u8		StationID,
 PODM_PHY_INFO_T pPhyInfo
	) {}
void odm_SwAntDivChkAntSwitch(
		PDM_ODM_T		pDM_Odm,
		u8			Step
	) {}
void ODM_SwAntDivResetBeforeLink(		PDM_ODM_T		pDM_Odm	){}
void ODM_SwAntDivRestAfterLink(		PDM_ODM_T		pDM_Odm	){}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void odm_SwAntDivChkAntSwitchCallback(void *FunctionContext) {};
#else
void odm_SwAntDivChkAntSwitchCallback(struct timer_list *t) {};
#endif

#endif //#if(defined(CONFIG_SW_ANTENNA_DIVERSITY))

//3============================================================
//3 SW Antenna Diversity
//3============================================================

#if(defined(CONFIG_HW_ANTENNA_DIVERSITY))
void
odm_InitHybridAntDiv_88C_92D(
 PDM_ODM_T	pDM_Odm
	)
{

	SWAT_T			*pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	u8                  bTxPathSel=0;	        //0:Path-A   1:Path-B
	u8			i;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("odm_InitHybridAntDiv==============>\n"));

	if((pDM_Odm->SupportICType != ODM_RTL8192C) && (pDM_Odm->SupportICType != ODM_RTL8192D))
		return;


	bTxPathSel=(pDM_Odm->RFType==ODM_1T1R)?FALSE:TRUE;

	ODM_SetBBReg(pDM_Odm,ODM_REG_BB_PWR_SAV1_11N, BIT23, 0); //No update ANTSEL during GNT_BT=1
	ODM_SetBBReg(pDM_Odm,ODM_REG_TX_ANT_CTRL_11N, BIT21, 1); //TX atenna selection from tx_info
	ODM_SetBBReg(pDM_Odm,ODM_REG_ANTSEL_PIN_11N, BIT23, 1);	//enable LED[1:0] pin as ANTSEL
	ODM_SetBBReg(pDM_Odm,ODM_REG_ANTSEL_CTRL_11N, BIT8|BIT9, 0x01);	// 0x01: left antenna, 0x02: right antenna
	// check HW setting: ANTSEL pin connection

	// only AP support different path selection temperarly
	if(!bTxPathSel){                 //PATH-A
		ODM_SetBBReg(pDM_Odm,ODM_REG_PIN_CTRL_11N, BIT8|BIT9, 0 ); // ANTSEL as HW control
		ODM_SetBBReg(pDM_Odm,ODM_REG_ANTSEL_PATH_11N, BIT13, 1);	 //select TX ANTESEL from path A
	}
	else	{
		ODM_SetBBReg(pDM_Odm,ODM_REG_PIN_CTRL_11N, BIT24|BIT25, 0 ); // ANTSEL as HW control
		ODM_SetBBReg(pDM_Odm,ODM_REG_ANTSEL_PATH_11N, BIT13, 0);		 //select ANTESEL from path B
	}

	//Set OFDM HW RX Antenna Diversity
	ODM_SetBBReg(pDM_Odm,ODM_REG_ANTDIV_PARA1_11N, 0x7FF, 0x0c0); //Pwdb threshold=8dB
	ODM_SetBBReg(pDM_Odm,ODM_REG_ANTDIV_PARA1_11N, BIT11, 0); //Switch to another antenna by checking pwdb threshold
	ODM_SetBBReg(pDM_Odm,ODM_REG_ANTDIV_PARA3_11N, BIT23, 1);	// Decide final antenna by comparing 2 antennas' pwdb

	//Set CCK HW RX Antenna Diversity
	ODM_SetBBReg(pDM_Odm,ODM_REG_CCK_ANTDIV_PARA2_11N, BIT4, 0); //Antenna diversity decision period = 32 sample
	ODM_SetBBReg(pDM_Odm,ODM_REG_CCK_ANTDIV_PARA2_11N, 0xf, 0xf); //Threshold for antenna diversity. Check another antenna power if input power < ANT_lim*4
	ODM_SetBBReg(pDM_Odm,ODM_REG_CCK_ANTDIV_PARA3_11N, BIT13, 1); //polarity ana_A=1 and ana_B=0
	ODM_SetBBReg(pDM_Odm,ODM_REG_CCK_ANTDIV_PARA4_11N, 0x1f, 0x8); //default antenna power = inpwr*(0.5 + r_ant_step/16)


	//Enable HW Antenna Diversity
	if(!bTxPathSel)                 //PATH-A
		ODM_SetBBReg(pDM_Odm,ODM_REG_IGI_A_11N, BIT7,1);	// Enable Hardware antenna switch
	else
		ODM_SetBBReg(pDM_Odm,ODM_REG_IGI_B_11N, BIT7,1);	// Enable Hardware antenna switch
	ODM_SetBBReg(pDM_Odm,ODM_REG_CCK_ANTDIV_PARA1_11N, BIT15, 1);//Enable antenna diversity

	pDM_SWAT_Table->CurAntenna=0;			//choose left antenna as default antenna
	pDM_SWAT_Table->PreAntenna=0;
	for(i=0; i<ASSOCIATE_ENTRY_NUM ; i++)
	{
		pDM_SWAT_Table->CCK_Ant1_Cnt[i] = 0;
		pDM_SWAT_Table->CCK_Ant2_Cnt[i] = 0;
		pDM_SWAT_Table->OFDM_Ant1_Cnt[i] = 0;
		pDM_SWAT_Table->OFDM_Ant2_Cnt[i] = 0;
		pDM_SWAT_Table->RSSI_Ant1_Sum[i] = 0;
		pDM_SWAT_Table->RSSI_Ant2_Sum[i] = 0;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("<==============odm_InitHybridAntDiv\n"));
}


void
odm_InitHybridAntDiv(
 PDM_ODM_T	pDM_Odm
	)
{
	if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("Return: Not Support HW AntDiv\n"));
		return;
	}

	if(pDM_Odm->SupportICType & (ODM_RTL8192C | ODM_RTL8192D))
	{
#if ((RTL8192C_SUPPORT == 1)||(RTL8192D_SUPPORT == 1))
		odm_InitHybridAntDiv_88C_92D(pDM_Odm);
#endif
	}
	else if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
#if (RTL8188E_SUPPORT == 1)
		ODM_AntennaDiversityInit_88E(pDM_Odm);
#endif
	}

}


bool
odm_StaDefAntSel(
 PDM_ODM_T	pDM_Odm,
 u32		OFDM_Ant1_Cnt,
 u32		OFDM_Ant2_Cnt,
 u32		CCK_Ant1_Cnt,
 u32		CCK_Ant2_Cnt,
 u8		*pDefAnt

	)
{
#if 1
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("odm_StaDefAntSelect==============>\n"));

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("OFDM_Ant1_Cnt:%d, OFDM_Ant2_Cnt:%d\n",OFDM_Ant1_Cnt,OFDM_Ant2_Cnt));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("CCK_Ant1_Cnt:%d, CCK_Ant2_Cnt:%d\n",CCK_Ant1_Cnt,CCK_Ant2_Cnt));


	if(((OFDM_Ant1_Cnt+OFDM_Ant2_Cnt)==0)&&((CCK_Ant1_Cnt + CCK_Ant2_Cnt) <10)){
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("odm_StaDefAntSelect Fail: No enough packet info!\n"));
		return	FALSE;
	}

	if(OFDM_Ant1_Cnt || OFDM_Ant2_Cnt )	{
		//if RX OFDM packet number larger than 0
		if(OFDM_Ant1_Cnt > OFDM_Ant2_Cnt)
			(*pDefAnt)=1;
		else
			(*pDefAnt)=0;
	}
	// else if RX CCK packet number larger than 10
	else if((CCK_Ant1_Cnt + CCK_Ant2_Cnt) >=10 )
	{
		if(CCK_Ant1_Cnt > (5*CCK_Ant2_Cnt))
			(*pDefAnt)=1;
		else if(CCK_Ant2_Cnt > (5*CCK_Ant1_Cnt))
			(*pDefAnt)=0;
		else if(CCK_Ant1_Cnt > CCK_Ant2_Cnt)
			(*pDefAnt)=0;
		else
			(*pDefAnt)=1;

	}

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("TxAnt = %s\n",((*pDefAnt)==1)?"Ant1":"Ant2"));

#endif
	//u32 antsel = ODM_GetBBReg(pDM_Odm, 0xc88, bMaskByte0);
	//(*pDefAnt)= (u8) antsel;




	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("<==============odm_StaDefAntSelect\n"));

	return TRUE;


}


void
odm_SetRxIdleAnt(
	PDM_ODM_T	pDM_Odm,
	u8	Ant,
   bool   bDualPath
)
{
	SWAT_T			*pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;

	//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("odm_SetRxIdleAnt==============>\n"));

	if(Ant != pDM_SWAT_Table->RxIdleAnt)
	{
	//for path-A
	if(Ant==1)
			ODM_SetBBReg(pDM_Odm,ODM_REG_RX_DEFUALT_A_11N, 0xFFFF, 0x65a9);   //right-side antenna
	else
			ODM_SetBBReg(pDM_Odm,ODM_REG_RX_DEFUALT_A_11N, 0xFFFF, 0x569a);   //left-side antenna

	//for path-B
	if(bDualPath){
			if(Ant==0)
				ODM_SetBBReg(pDM_Odm,ODM_REG_RX_DEFUALT_A_11N, 0xFFFF0000, 0x65a9);   //right-side antenna
		else
				ODM_SetBBReg(pDM_Odm,ODM_REG_RX_DEFUALT_A_11N, 0xFFFF0000, 0x569a);  //left-side antenna
		}
	}
		pDM_SWAT_Table->RxIdleAnt = Ant;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("RxIdleAnt: %s  Reg858=0x%x\n",(Ant==1)?"Ant1":"Ant2",(Ant==1)?0x65a9:0x569a));

	//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("<==============odm_SetRxIdleAnt\n"));

	}

void
ODM_AntselStatistics_88C(
		PDM_ODM_T		pDM_Odm,
		u8			MacId,
		u32			PWDBAll,
		bool			isCCKrate
)
{
	SWAT_T			*pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;

	if(pDM_SWAT_Table->antsel == 1)
	{
		if(isCCKrate)
			pDM_SWAT_Table->CCK_Ant1_Cnt[MacId]++;
		else
		{
			pDM_SWAT_Table->OFDM_Ant1_Cnt[MacId]++;
			pDM_SWAT_Table->RSSI_Ant1_Sum[MacId] += PWDBAll;
		}
	}
	else
	{
		if(isCCKrate)
			pDM_SWAT_Table->CCK_Ant2_Cnt[MacId]++;
		else
		{
			pDM_SWAT_Table->OFDM_Ant2_Cnt[MacId]++;
			pDM_SWAT_Table->RSSI_Ant2_Sum[MacId] += PWDBAll;
		}
	}

}


void
ODM_SetTxAntByTxInfo_88C_92D(
		PDM_ODM_T		pDM_Odm
)
{

}

void
odm_HwAntDiv_92C_92D(
	PDM_ODM_T	pDM_Odm
)
{
	SWAT_T			*pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	u32			RSSI_Min=0xFF, RSSI, RSSI_Ant1, RSSI_Ant2;
	u8			RxIdleAnt, i;
	bool		bRet=FALSE;
	PSTA_INFO_T	pEntry;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("odm_HwAntDiv==============>\n"));

	if(!(pDM_Odm->SupportAbility&ODM_BB_ANT_DIV))                                    //if don't support antenna diveristy
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("odm_HwAntDiv: Not supported!\n"));
		return;
	}

	if((pDM_Odm->SupportICType != ODM_RTL8192C) && (pDM_Odm->SupportICType != ODM_RTL8192D))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("Return: IC Type is not 92C or 92D\n"));
		return;
	}

	if(!pDM_Odm->bLinked)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("Return: bLinked is FALSE\n"));
		return;
	}

	for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
	{
		pEntry = pDM_Odm->pODM_StaInfo[i];
		if(IS_STA_VALID(pEntry))
		{

			RSSI_Ant1 = (pDM_SWAT_Table->OFDM_Ant1_Cnt[i] == 0)?0:(pDM_SWAT_Table->RSSI_Ant1_Sum[i]/pDM_SWAT_Table->OFDM_Ant1_Cnt[i]);
			RSSI_Ant2 = (pDM_SWAT_Table->OFDM_Ant2_Cnt[i] == 0)?0:(pDM_SWAT_Table->RSSI_Ant2_Sum[i]/pDM_SWAT_Table->OFDM_Ant2_Cnt[i]);

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("RSSI_Ant1=%d,  RSSI_Ant2=%d\n", RSSI_Ant1, RSSI_Ant2));

			if(RSSI_Ant1 ||RSSI_Ant2)
			{
				RSSI = (RSSI_Ant1 < RSSI_Ant2) ? RSSI_Ant1 : RSSI_Ant2;
				if((!RSSI) || ( RSSI < RSSI_Min) ) {
					pDM_SWAT_Table->TargetSTA = i;
					RSSI_Min = RSSI;
				}
			}
			///STA: found out default antenna
			bRet=odm_StaDefAntSel(pDM_Odm,
						 pDM_SWAT_Table->OFDM_Ant1_Cnt[i],
						 pDM_SWAT_Table->OFDM_Ant2_Cnt[i],
						 pDM_SWAT_Table->CCK_Ant1_Cnt[i],
						 pDM_SWAT_Table->CCK_Ant2_Cnt[i],
						 &pDM_SWAT_Table->TxAnt[i]);

			//if Tx antenna selection: successful
			if(bRet){
				pDM_SWAT_Table->RSSI_Ant1_Sum[i] = 0;
				pDM_SWAT_Table->RSSI_Ant2_Sum[i] = 0;
				pDM_SWAT_Table->OFDM_Ant1_Cnt[i] = 0;
				pDM_SWAT_Table->OFDM_Ant2_Cnt[i] = 0;
				pDM_SWAT_Table->CCK_Ant1_Cnt[i] = 0;
				pDM_SWAT_Table->CCK_Ant2_Cnt[i] = 0;
			}
		}
	}

	//set RX Idle Ant
	RxIdleAnt = pDM_SWAT_Table->TxAnt[pDM_SWAT_Table->TargetSTA];
	odm_SetRxIdleAnt(pDM_Odm, RxIdleAnt, FALSE);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("<==============odm_HwAntDiv\n"));
}

void
odm_HwAntDiv(
	PDM_ODM_T	pDM_Odm
)
{
	if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("Return: Not Support HW AntDiv\n"));
		return;
	}

	if(pDM_Odm->SupportICType & (ODM_RTL8192C | ODM_RTL8192D))
	{
#if ((RTL8192C_SUPPORT == 1)||(RTL8192D_SUPPORT == 1))
		odm_HwAntDiv_92C_92D(pDM_Odm);
#endif
	}
	else if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
#if (RTL8188E_SUPPORT == 1)
		ODM_AntennaDiversity_88E(pDM_Odm);
#endif
	}

}



#else //#if(defined(CONFIG_HW_ANTENNA_DIVERSITY))

void odm_InitHybridAntDiv( PDM_ODM_T	pDM_Odm		){}
void odm_HwAntDiv(	PDM_ODM_T	pDM_Odm){}
void ODM_SetTxAntByTxInfo_88C_92D(		PDM_ODM_T		pDM_Odm){ }

#endif //#if(defined(CONFIG_HW_ANTENNA_DIVERSITY))



//============================================================
//EDCA Turbo
//============================================================
void
ODM_EdcaTurboInit(
    PDM_ODM_T		pDM_Odm)
{

	struct rtw_adapter *	Adapter = pDM_Odm->Adapter;
	pDM_Odm->DM_EDCA_Table.bCurrentTurboEDCA = FALSE;
	pDM_Odm->DM_EDCA_Table.bIsCurRDLState = FALSE;
	Adapter->recvpriv.bIsAnyNonBEPkts =FALSE;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EDCA_TURBO,ODM_DBG_LOUD,("Orginial VO PARAM: 0x%x\n",ODM_Read4Byte(pDM_Odm,ODM_EDCA_VO_PARAM)));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EDCA_TURBO,ODM_DBG_LOUD,("Orginial VI PARAM: 0x%x\n",ODM_Read4Byte(pDM_Odm,ODM_EDCA_VI_PARAM)));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EDCA_TURBO,ODM_DBG_LOUD,("Orginial BE PARAM: 0x%x\n",ODM_Read4Byte(pDM_Odm,ODM_EDCA_BE_PARAM)));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EDCA_TURBO,ODM_DBG_LOUD,("Orginial BK PARAM: 0x%x\n",ODM_Read4Byte(pDM_Odm,ODM_EDCA_BK_PARAM)));


}	// ODM_InitEdcaTurbo

void
odm_EdcaTurboCheck(
		PDM_ODM_T		pDM_Odm
	)
{
	//
	// For AP/ADSL use prtl8192cd_priv
	// For CE/NIC use struct rtw_adapter *
	//
	struct rtw_adapter *		pAdapter = pDM_Odm->Adapter;
	prtl8192cd_priv	priv		= pDM_Odm->priv;

	//
	// 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	// at the same time. In the stage2/3, we need to prive universal interface and merge all
	// HW dynamic mechanism.
	//
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EDCA_TURBO,ODM_DBG_LOUD,("odm_EdcaTurboCheck========================>\n"));

	if(!(pDM_Odm->SupportAbility& ODM_MAC_EDCA_TURBO ))
		return;

	switch	(pDM_Odm->SupportPlatform)
	{
		case	ODM_MP:
			break;

		case	ODM_CE:
			odm_EdcaTurboCheckCE(pDM_Odm);
			break;

		case	ODM_AP:
		case	ODM_ADSL:
			break;
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EDCA_TURBO,ODM_DBG_LOUD,("<========================odm_EdcaTurboCheck\n"));

}	// odm_CheckEdcaTurbo

void
odm_EdcaTurboCheckCE(
		PDM_ODM_T		pDM_Odm
	)
{
	struct rtw_adapter *Adapter = pDM_Odm->Adapter;

	u32	trafficIndex;
	u32	edca_param;
	u64	cur_tx_bytes = 0;
	u64	cur_rx_bytes = 0;
	u8	bbtchange = _FALSE;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	struct xmit_priv		*pxmitpriv = &(Adapter->xmitpriv);
	struct recv_priv		*precvpriv = &(Adapter->recvpriv);
	struct registry_priv	*pregpriv = &Adapter->registrypriv;
	struct mlme_ext_priv	*pmlmeext = &(Adapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);


	if ((pregpriv->wifi_spec == 1) )//|| (pmlmeinfo->HT_enable == 0))
	{
		goto dm_CheckEdcaTurbo_EXIT;
	}

	if (pmlmeinfo->assoc_AP_vendor >=  HT_IOT_PEER_MAX)
	{
		goto dm_CheckEdcaTurbo_EXIT;
	}

#ifdef CONFIG_BT_COEXIST
	if (BT_DisableEDCATurbo(Adapter))
	{
		goto dm_CheckEdcaTurbo_EXIT;
	}
#endif

	// Check if the status needs to be changed.
	if((bbtchange) || (!precvpriv->bIsAnyNonBEPkts) )
	{
		cur_tx_bytes = pxmitpriv->tx_bytes - pxmitpriv->last_tx_bytes;
		cur_rx_bytes = precvpriv->rx_bytes - precvpriv->last_rx_bytes;

		//traffic, TX or RX
		if((pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_RALINK)||(pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_ATHEROS))
		{
			if (cur_tx_bytes > (cur_rx_bytes << 2))
			{ // Uplink TP is present.
				trafficIndex = UP_LINK;
			}
			else
			{ // Balance TP is present.
				trafficIndex = DOWN_LINK;
			}
		}
		else
		{
			if (cur_rx_bytes > (cur_tx_bytes << 2))
			{ // Downlink TP is present.
				trafficIndex = DOWN_LINK;
			}
			else
			{ // Balance TP is present.
				trafficIndex = UP_LINK;
			}
		}

		if ((pDM_Odm->DM_EDCA_Table.prv_traffic_idx != trafficIndex) || (!pDM_Odm->DM_EDCA_Table.bCurrentTurboEDCA))
		{
			if((pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_CISCO) && (pmlmeext->cur_wireless_mode & WIRELESS_11_24N))
			{
				edca_param = EDCAParam[pmlmeinfo->assoc_AP_vendor][trafficIndex];
			}
			else
			{
				edca_param = EDCAParam[HT_IOT_PEER_UNKNOWN][trafficIndex];
			}
			rtw_write32(Adapter, REG_EDCA_BE_PARAM, edca_param);

			pDM_Odm->DM_EDCA_Table.prv_traffic_idx = trafficIndex;
		}

		pDM_Odm->DM_EDCA_Table.bCurrentTurboEDCA = _TRUE;
	}
	else
	{
		//
		// Turn Off EDCA turbo here.
		// Restore original EDCA according to the declaration of AP.
		//
		 if(pDM_Odm->DM_EDCA_Table.bCurrentTurboEDCA)
		{
			rtw_write32(Adapter, REG_EDCA_BE_PARAM, pHalData->AcParam_BE);
			pDM_Odm->DM_EDCA_Table.bCurrentTurboEDCA = _FALSE;
		}
	}

dm_CheckEdcaTurbo_EXIT:
	// Set variables for next time.
	precvpriv->bIsAnyNonBEPkts = _FALSE;
	pxmitpriv->last_tx_bytes = pxmitpriv->tx_bytes;
	precvpriv->last_rx_bytes = precvpriv->rx_bytes;
}


// need to ODM CE Platform
//move to here for ANT detection mechanism using

u32
GetPSDData(
 PDM_ODM_T	pDM_Odm,
	unsigned int	point,
	u8 initial_gain_psd)
{
	//unsigned int	val, rfval;
	//int	psd_report;
	u32	psd_report;

	//HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	//Debug Message
	//val = PHY_QueryBBReg(Adapter,0x908, bMaskDWord);
	//DbgPrint("Reg908 = 0x%x\n",val);
	//val = PHY_QueryBBReg(Adapter,0xDF4, bMaskDWord);
	//rfval = PHY_QueryRFReg(Adapter, RF_PATH_A, 0x00, bRFRegOffsetMask);
	//DbgPrint("RegDF4 = 0x%x, RFReg00 = 0x%x\n",val, rfval);
	//DbgPrint("PHYTXON = %x, OFDMCCA_PP = %x, CCKCCA_PP = %x, RFReg00 = %x\n",
		//(val&BIT25)>>25, (val&BIT14)>>14, (val&BIT15)>>15, rfval);

	//Set DCO frequency index, offset=(40MHz/SamplePts)*point
	ODM_SetBBReg(pDM_Odm, 0x808, 0x3FF, point);

	//Start PSD calculation, Reg808[22]=0->1
	ODM_SetBBReg(pDM_Odm, 0x808, BIT22, 1);
	//Need to wait for HW PSD report
	ODM_StallExecution(30);
	ODM_SetBBReg(pDM_Odm, 0x808, BIT22, 0);
	//Read PSD report, Reg8B4[15:0]
	psd_report = ODM_GetBBReg(pDM_Odm,0x8B4, bMaskDWord) & 0x0000FFFF;

#if 1//(DEV_BUS_TYPE == RT_PCI_INTERFACE) && ( (RT_PLATFORM == PLATFORM_LINUX) || (RT_PLATFORM == PLATFORM_MACOSX))
	psd_report = (u32) (ConvertTo_dB(psd_report))+(u32)(initial_gain_psd-0x1c);
#else
	psd_report = (int) (20*log10((double)psd_report))+(int)(initial_gain_psd-0x1c);
#endif

	return psd_report;

}

u32
ConvertTo_dB(
	u32	Value)
{
	u8 i;
	u8 j;
	u32 dB;

	Value = Value & 0xFFFF;

	for (i=0;i<8;i++)
	{
		if (Value <= dB_Invert_Table[i][11])
		{
			break;
		}
	}

	if (i >= 8)
	{
		return (96);	// maximum 96 dB
	}

	for (j=0;j<12;j++)
	{
		if (Value <= dB_Invert_Table[i][j])
		{
			break;
		}
	}

	dB = i*12 + j + 1;

	return (dB);
}

//
// 2011/09/22 MH Add for 92D global spin lock utilization.
//
void
odm_GlobalAdapterCheck(
		void
	)
{
}	// odm_GlobalAdapterCheck


//
// Description:
//	Set Single/Dual Antenna default setting for products that do not do detection in advance.
//
// Added by Joseph, 2012.03.22
//
void
ODM_SingleDualAntennaDefaultSetting(
		PDM_ODM_T		pDM_Odm
	)
{
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pDM_SWAT_Table->ANTA_ON=TRUE;
	pDM_SWAT_Table->ANTB_ON=TRUE;
}


//2 8723A ANT DETECT


void
odm_PHY_SaveAFERegisters(
	PDM_ODM_T	pDM_Odm,
	u32 *		AFEReg,
	u32 *		AFEBackup,
	u32		RegisterNum
	)
{
	u32	i;

	//RTPRINT(FINIT, INIT_IQK, ("Save ADDA parameters.\n"));
	for( i = 0 ; i < RegisterNum ; i++){
		AFEBackup[i] = ODM_GetBBReg(pDM_Odm, AFEReg[i], bMaskDWord);
	}
}

void
odm_PHY_ReloadAFERegisters(
	PDM_ODM_T	pDM_Odm,
	u32 *		AFEReg,
	u32 *		AFEBackup,
	u32		RegiesterNum
	)
{
	u32	i;

	//RTPRINT(FINIT, INIT_IQK, ("Reload ADDA power saving parameters !\n"));
	for(i = 0 ; i < RegiesterNum; i++)
	{

		ODM_SetBBReg(pDM_Odm, AFEReg[i], bMaskDWord, AFEBackup[i]);
	}
}

//2 8723A ANT DETECT
//
// Description:
//	Implement IQK single tone for RF DPK loopback and BB PSD scanning.
//	This function is cooperated with BB team Neil.
//
// Added by Roger, 2011.12.15
//
bool
ODM_SingleDualAntennaDetection(
		PDM_ODM_T		pDM_Odm,
		u8			mode
	)
{

	//HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	//PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	u32		CurrentChannel,RfLoopReg;
	u8		n;
	u32		Reg88c, Regc08, Reg874, Regc50;
	u8		initial_gain = 0x5a;
	u32		PSD_report_tmp;
	u32		AntA_report = 0x0, AntB_report = 0x0,AntO_report=0x0;
	bool		bResult = TRUE;
	u32		AFE_Backup[16];
	u32		AFE_REG_8723A[16] = {
					rRx_Wait_CCA,	rTx_CCK_RFON,
					rTx_CCK_BBON,	rTx_OFDM_RFON,
					rTx_OFDM_BBON,	rTx_To_Rx,
					rTx_To_Tx,		rRx_CCK,
					rRx_OFDM,		rRx_Wait_RIFS,
					rRx_TO_Rx,		rStandby,
					rSleep,			rPMPD_ANAEN,
					rFPGA0_XCD_SwitchControl, rBlue_Tooth};

	if(!(pDM_Odm->SupportICType & (ODM_RTL8723A|ODM_RTL8192C)))
		return bResult;

	if(!(pDM_Odm->SupportAbility&ODM_BB_ANT_DIV))
		return bResult;

	if(pDM_Odm->SupportICType == ODM_RTL8192C)
	{
		//Which path in ADC/DAC is turnned on for PSD: both I/Q
		ODM_SetBBReg(pDM_Odm, 0x808, BIT10|BIT11, 0x3);
		//Ageraged number: 8
		ODM_SetBBReg(pDM_Odm, 0x808, BIT12|BIT13, 0x1);
		//pts = 128;
		ODM_SetBBReg(pDM_Odm, 0x808, BIT14|BIT15, 0x0);
	}

	//1 Backup Current RF/BB Settings

	CurrentChannel = ODM_GetRFReg(pDM_Odm, RF_PATH_A, ODM_CHANNEL, bRFRegOffsetMask);
	RfLoopReg = ODM_GetRFReg(pDM_Odm, RF_PATH_A, 0x00, bRFRegOffsetMask);
	ODM_SetBBReg(pDM_Odm, rFPGA0_XA_RFInterfaceOE, ODM_DPDT, Antenna_A);  // change to Antenna A
	// Step 1: USE IQK to transmitter single tone

	ODM_StallExecution(10);

	//Store A Path Register 88c, c08, 874, c50
	Reg88c = ODM_GetBBReg(pDM_Odm, rFPGA0_AnalogParameter4, bMaskDWord);
	Regc08 = ODM_GetBBReg(pDM_Odm, rOFDM0_TRMuxPar, bMaskDWord);
	Reg874 = ODM_GetBBReg(pDM_Odm, rFPGA0_XCD_RFInterfaceSW, bMaskDWord);
	Regc50 = ODM_GetBBReg(pDM_Odm, rOFDM0_XAAGCCore1, bMaskDWord);

	// Store AFE Registers
	odm_PHY_SaveAFERegisters(pDM_Odm, AFE_REG_8723A, AFE_Backup, 16);

	//Set PSD 128 pts
	ODM_SetBBReg(pDM_Odm, rFPGA0_PSDFunction, BIT14|BIT15, 0x0);  //128 pts

	// To SET CH1 to do
	ODM_SetRFReg(pDM_Odm, RF_PATH_A, ODM_CHANNEL, bRFRegOffsetMask, 0x01);     //Channel 1

	// AFE all on step
	ODM_SetBBReg(pDM_Odm, rRx_Wait_CCA, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rTx_CCK_RFON, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rTx_CCK_BBON, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rTx_OFDM_RFON, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rTx_OFDM_BBON, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rTx_To_Rx, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rTx_To_Tx, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rRx_CCK, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rRx_OFDM, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rRx_Wait_RIFS, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rRx_TO_Rx, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rStandby, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rSleep, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rPMPD_ANAEN, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rFPGA0_XCD_SwitchControl, bMaskDWord, 0x6FDB25A4);
	ODM_SetBBReg(pDM_Odm, rBlue_Tooth, bMaskDWord, 0x6FDB25A4);

	// 3 wire Disable
	ODM_SetBBReg(pDM_Odm, rFPGA0_AnalogParameter4, bMaskDWord, 0xCCF000C0);

	//BB IQK Setting
	ODM_SetBBReg(pDM_Odm, rOFDM0_TRMuxPar, bMaskDWord, 0x000800E4);
	ODM_SetBBReg(pDM_Odm, rFPGA0_XCD_RFInterfaceSW, bMaskDWord, 0x22208000);

	//IQK setting tone@ 4.34Mhz
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x10008C1C);
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, 0x01007c00);


	//Page B init
	ODM_SetBBReg(pDM_Odm, rConfig_AntA, bMaskDWord, 0x00080000);
	ODM_SetBBReg(pDM_Odm, rConfig_AntA, bMaskDWord, 0x0f600000);
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x10008c1f);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x82150008);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_A, bMaskDWord, 0x28150008);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x001028d0);

	//RF loop Setting
	ODM_SetRFReg(pDM_Odm, RF_PATH_A, 0x0, 0xFFFFF, 0x50008);

	//IQK Single tone start
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskDWord, 0x80800000);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8000000);
	ODM_StallExecution(1000);
	PSD_report_tmp=0x0;

	for (n=0;n<2;n++)
	{
		PSD_report_tmp =  GetPSDData(pDM_Odm, 14, initial_gain);
		if(PSD_report_tmp >AntA_report)
			AntA_report=PSD_report_tmp;
	}

	PSD_report_tmp=0x0;

	ODM_SetBBReg(pDM_Odm, rFPGA0_XA_RFInterfaceOE, 0x300, Antenna_B);  // change to Antenna B
	ODM_StallExecution(10);


	for (n=0;n<2;n++)
	{
		PSD_report_tmp =  GetPSDData(pDM_Odm, 14, initial_gain);
		if(PSD_report_tmp > AntB_report)
			AntB_report=PSD_report_tmp;
	}

	// change to open case
	ODM_SetBBReg(pDM_Odm, rFPGA0_XA_RFInterfaceOE, 0x300, 0);  // change to Ant A and B all open case
	ODM_StallExecution(10);

	for (n=0;n<2;n++)
	{
		PSD_report_tmp =  GetPSDData(pDM_Odm, 14, initial_gain);
		if(PSD_report_tmp > AntO_report)
			AntO_report=PSD_report_tmp;
	}

	//Close IQK Single Tone function
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskDWord, 0x00000000);
	PSD_report_tmp = 0x0;

	//1 Return to antanna A
	ODM_SetBBReg(pDM_Odm, rFPGA0_XA_RFInterfaceOE, 0x300, Antenna_A);
	ODM_SetBBReg(pDM_Odm, rFPGA0_AnalogParameter4, bMaskDWord, Reg88c);
	ODM_SetBBReg(pDM_Odm, rOFDM0_TRMuxPar, bMaskDWord, Regc08);
	ODM_SetBBReg(pDM_Odm, rFPGA0_XCD_RFInterfaceSW, bMaskDWord, Reg874);
	ODM_SetBBReg(pDM_Odm, rOFDM0_XAAGCCore1, 0x7F, 0x40);
	ODM_SetBBReg(pDM_Odm, rOFDM0_XAAGCCore1, bMaskDWord, Regc50);
	ODM_SetRFReg(pDM_Odm, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask,CurrentChannel);
	ODM_SetRFReg(pDM_Odm, RF_PATH_A, 0x00, bRFRegOffsetMask,RfLoopReg);

	//Reload AFE Registers
	odm_PHY_ReloadAFERegisters(pDM_Odm, AFE_REG_8723A, AFE_Backup, 16);

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("psd_report_A[%d]= %d \n", 2416, AntA_report));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("psd_report_B[%d]= %d \n", 2416, AntB_report));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("psd_report_O[%d]= %d \n", 2416, AntO_report));


	if(pDM_Odm->SupportICType == ODM_RTL8723A)
	{
	//2 Test Ant B based on Ant A is ON
	if(mode==ANTTESTB)
	{
	if(AntA_report >=	100)
	{
		if(AntB_report > (AntA_report+1))
		{
			pDM_SWAT_Table->ANTB_ON=FALSE;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Single Antenna A\n"));
		}
		else
		{
			pDM_SWAT_Table->ANTB_ON=TRUE;
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Dual Antenna is A and B\n"));
		}
	}
	else
	{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Need to check again\n"));
		pDM_SWAT_Table->ANTB_ON=FALSE; // Set Antenna B off as default
		bResult = FALSE;
	}
	}
	//2 Test Ant A and B based on DPDT Open
	else if(mode==ANTTESTALL)
	{
		if((AntO_report >=100)&(AntO_report <118))
		{
			if(AntA_report > (AntO_report+1))
			{
				pDM_SWAT_Table->ANTA_ON=FALSE;
				//RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("ODM_AntennaDetection(): Antenna A is OFF\n"));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Ant A is OFF"));
			}
			else
			{
				pDM_SWAT_Table->ANTA_ON=TRUE;
				//RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("ODM_AntennaDetection(): Antenna A is ON\n"));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Ant A is ON"));
			}

				if(AntB_report > (AntO_report+2))
			{
				pDM_SWAT_Table->ANTB_ON=FALSE;
				//RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("ODM_AntennaDetection(): Antenna B is OFF\n"));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Ant B is OFF"));
			}
			else
			{
				pDM_SWAT_Table->ANTB_ON=TRUE;
				//RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("ODM_AntennaDetection(): Antenna B is ON\n"));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Ant B is ON"));
			}
		}
	}
	}
	else if(pDM_Odm->SupportICType == ODM_RTL8192C)
	{
		if(AntA_report >=	100)
		{
			if(AntB_report > (AntA_report+2))
			{
				pDM_SWAT_Table->ANTA_ON=FALSE;
				pDM_SWAT_Table->ANTB_ON=TRUE;
				ODM_SetBBReg(pDM_Odm,  rFPGA0_XA_RFInterfaceOE, 0x300, Antenna_B);
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Single Antenna B\n"));
			}
			else if(AntA_report > (AntB_report+2))
			{
				pDM_SWAT_Table->ANTA_ON=TRUE;
				pDM_SWAT_Table->ANTB_ON=FALSE;
				ODM_SetBBReg(pDM_Odm,  rFPGA0_XA_RFInterfaceOE, 0x300, Antenna_A);
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Single Antenna A\n"));
			}
			else
			{
				pDM_SWAT_Table->ANTA_ON=TRUE;
				pDM_SWAT_Table->ANTB_ON=TRUE;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Dual Antenna \n"));
			}
		}
		else
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Need to check again\n"));
			pDM_SWAT_Table->ANTA_ON=TRUE; // Set Antenna A on as default
			pDM_SWAT_Table->ANTB_ON=FALSE; // Set Antenna B off as default
			bResult = FALSE;
		}
	}
	return bResult;

}

/* Justin: According to the current RRSI to adjust Response Frame TX power, 2012/11/05 */
void odm_dtc(PDM_ODM_T pDM_Odm)
{
#ifdef CONFIG_DM_RESP_TXAGC
	#define DTC_BASE            35	/* RSSI higher than this value, start to decade TX power */
	#define DTC_DWN_BASE       (DTC_BASE-5)	/* RSSI lower than this value, start to increase TX power */

	/* RSSI vs TX power step mapping: decade TX power */
	static const u8 dtc_table_down[]={
		DTC_BASE,
		(DTC_BASE+5),
		(DTC_BASE+10),
		(DTC_BASE+15),
		(DTC_BASE+20),
		(DTC_BASE+25)
	};

	/* RSSI vs TX power step mapping: increase TX power */
	static const u8 dtc_table_up[]={
		DTC_DWN_BASE,
		(DTC_DWN_BASE-5),
		(DTC_DWN_BASE-10),
		(DTC_DWN_BASE-15),
		(DTC_DWN_BASE-15),
		(DTC_DWN_BASE-20),
		(DTC_DWN_BASE-20),
		(DTC_DWN_BASE-25),
		(DTC_DWN_BASE-25),
		(DTC_DWN_BASE-30),
		(DTC_DWN_BASE-35)
	};

	u8 i;
	u8 dtc_steps=0;
	u8 sign;
	u8 resp_txagc=0;

	if (DTC_BASE < pDM_Odm->RSSI_Min) {
		/* need to decade the CTS TX power */
		sign = 1;
		for (i=0;i<ARRAY_SIZE(dtc_table_down);i++)
		{
			if ((dtc_table_down[i] >= pDM_Odm->RSSI_Min) || (dtc_steps >= 6))
				break;
			else
				dtc_steps++;
		}
	}
	else
	{
		sign = 0;
		dtc_steps = 0;
	}

	resp_txagc = dtc_steps | (sign << 4);
	resp_txagc = resp_txagc | (resp_txagc << 5);
	ODM_Write1Byte(pDM_Odm, 0x06d9, resp_txagc);

	DBG_8723A("%s RSSI_Min:%u, set RESP_TXAGC to %s %u\n",
		__func__, pDM_Odm->RSSI_Min, sign?"minus":"plus", dtc_steps);
#endif /* CONFIG_RESP_TXAGC_ADJUST */
}
