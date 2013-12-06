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

 #include "odm_precomp.h"

#if(DM_ODM_SUPPORT_TYPE & ODM_MP)
#include "Mp_Precomp.h"

void
phy_PathAStandBy(
	PADAPTER	pAdapter
	)
{
	RTPRINT(FINIT, INIT_IQK, ("Path-A standby mode!\n"));

	PHY_SetBBReg(pAdapter, rFPGA0_IQK, bMaskDWord, 0x0);
	PHY_SetBBReg(pAdapter, 0x840, bMaskDWord, 0x00010000);
	PHY_SetBBReg(pAdapter, rFPGA0_IQK, bMaskDWord, 0x80800000);
}

//1 7.	IQK
//#define MAX_TOLERANCE		5
//#define IQK_DELAY_TIME		1		//ms

u1Byte			//bit0 = 1 => Tx OK, bit1 = 1 => Rx OK
phy_PathA_IQK_8192C(
	PADAPTER	pAdapter,
	bool		configPathB
	)
{

	u4Byte regEAC, regE94, regE9C, regEA4;
	u1Byte result = 0x00;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	RTPRINT(FINIT, INIT_IQK, ("Path A IQK!\n"));

	//path-A IQK setting
	RTPRINT(FINIT, INIT_IQK, ("Path-A IQK setting!\n"));
	if(pAdapter->interfaceIndex == 0)
	{
		PHY_SetBBReg(pAdapter, rTx_IQK_Tone_A, bMaskDWord, 0x10008c1f);
		PHY_SetBBReg(pAdapter, rRx_IQK_Tone_A, bMaskDWord, 0x10008c1f);
	}
	else
	{
		PHY_SetBBReg(pAdapter, rTx_IQK_Tone_A, bMaskDWord, 0x10008c22);
		PHY_SetBBReg(pAdapter, rRx_IQK_Tone_A, bMaskDWord, 0x10008c22);
	}

	PHY_SetBBReg(pAdapter, rTx_IQK_PI_A, bMaskDWord, 0x82140102);

	PHY_SetBBReg(pAdapter, rRx_IQK_PI_A, bMaskDWord, configPathB ? 0x28160202 :
		IS_81xxC_VENDOR_UMC_B_CUT(pHalData->VersionID)?0x28160202:0x28160502);

	//path-B IQK setting
	if(configPathB)
	{
		PHY_SetBBReg(pAdapter, rTx_IQK_Tone_B, bMaskDWord, 0x10008c22);
		PHY_SetBBReg(pAdapter, rRx_IQK_Tone_B, bMaskDWord, 0x10008c22);
		PHY_SetBBReg(pAdapter, rTx_IQK_PI_B, bMaskDWord, 0x82140102);
		PHY_SetBBReg(pAdapter, rRx_IQK_PI_B, bMaskDWord, 0x28160202);
	}

	//LO calibration setting
	RTPRINT(FINIT, INIT_IQK, ("LO calibration setting!\n"));
	PHY_SetBBReg(pAdapter, rIQK_AGC_Rsp, bMaskDWord, 0x001028d1);

	//One shot, path A LOK & IQK
	RTPRINT(FINIT, INIT_IQK, ("One shot, path A LOK & IQK!\n"));
	PHY_SetBBReg(pAdapter, rIQK_AGC_Pts, bMaskDWord, 0xf9000000);
	PHY_SetBBReg(pAdapter, rIQK_AGC_Pts, bMaskDWord, 0xf8000000);

	// delay x ms
	RTPRINT(FINIT, INIT_IQK, ("Delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME));
	PlatformStallExecution(IQK_DELAY_TIME*1000);

	// Check failed
	regEAC = PHY_QueryBBReg(pAdapter, rRx_Power_After_IQK_A_2, bMaskDWord);
	RTPRINT(FINIT, INIT_IQK, ("0xeac = 0x%x\n", regEAC));
	regE94 = PHY_QueryBBReg(pAdapter, rTx_Power_Before_IQK_A, bMaskDWord);
	RTPRINT(FINIT, INIT_IQK, ("0xe94 = 0x%x\n", regE94));
	regE9C= PHY_QueryBBReg(pAdapter, rTx_Power_After_IQK_A, bMaskDWord);
	RTPRINT(FINIT, INIT_IQK, ("0xe9c = 0x%x\n", regE9C));
	regEA4= PHY_QueryBBReg(pAdapter, rRx_Power_Before_IQK_A_2, bMaskDWord);
	RTPRINT(FINIT, INIT_IQK, ("0xea4 = 0x%x\n", regEA4));

	if(!(regEAC & BIT28) &&
		(((regE94 & 0x03FF0000)>>16) != 0x142) &&
		(((regE9C & 0x03FF0000)>>16) != 0x42) )
		result |= 0x01;
	else							//if Tx not OK, ignore Rx
		return result;

	if(!(regEAC & BIT27) &&		//if Tx is OK, check whether Rx is OK
		(((regEA4 & 0x03FF0000)>>16) != 0x132) &&
		(((regEAC & 0x03FF0000)>>16) != 0x36))
		result |= 0x02;
	else
		RTPRINT(FINIT, INIT_IQK, ("Path A Rx IQK fail!!\n"));

	return result;


}

u1Byte				//bit0 = 1 => Tx OK, bit1 = 1 => Rx OK
phy_PathB_IQK_8192C(
	PADAPTER	pAdapter
	)
{
	u4Byte regEAC, regEB4, regEBC, regEC4, regECC;
	u1Byte	result = 0x00;
	RTPRINT(FINIT, INIT_IQK, ("Path B IQK!\n"));

	//One shot, path B LOK & IQK
	RTPRINT(FINIT, INIT_IQK, ("One shot, path A LOK & IQK!\n"));
	PHY_SetBBReg(pAdapter, rIQK_AGC_Cont, bMaskDWord, 0x00000002);
	PHY_SetBBReg(pAdapter, rIQK_AGC_Cont, bMaskDWord, 0x00000000);

	// delay x ms
	RTPRINT(FINIT, INIT_IQK, ("Delay %d ms for One shot, path B LOK & IQK.\n", IQK_DELAY_TIME));
	PlatformStallExecution(IQK_DELAY_TIME*1000);

	// Check failed
	regEAC = PHY_QueryBBReg(pAdapter, rRx_Power_After_IQK_A_2, bMaskDWord);
	RTPRINT(FINIT, INIT_IQK, ("0xeac = 0x%x\n", regEAC));
	regEB4 = PHY_QueryBBReg(pAdapter, rTx_Power_Before_IQK_B, bMaskDWord);
	RTPRINT(FINIT, INIT_IQK, ("0xeb4 = 0x%x\n", regEB4));
	regEBC= PHY_QueryBBReg(pAdapter, rTx_Power_After_IQK_B, bMaskDWord);
	RTPRINT(FINIT, INIT_IQK, ("0xebc = 0x%x\n", regEBC));
	regEC4= PHY_QueryBBReg(pAdapter, rRx_Power_Before_IQK_B_2, bMaskDWord);
	RTPRINT(FINIT, INIT_IQK, ("0xec4 = 0x%x\n", regEC4));
	regECC= PHY_QueryBBReg(pAdapter, rRx_Power_After_IQK_B_2, bMaskDWord);
	RTPRINT(FINIT, INIT_IQK, ("0xecc = 0x%x\n", regECC));

	if(!(regEAC & BIT31) &&
		(((regEB4 & 0x03FF0000)>>16) != 0x142) &&
		(((regEBC & 0x03FF0000)>>16) != 0x42))
		result |= 0x01;
	else
		return result;

	if(!(regEAC & BIT30) &&
		(((regEC4 & 0x03FF0000)>>16) != 0x132) &&
		(((regECC & 0x03FF0000)>>16) != 0x36))
		result |= 0x02;
	else
		RTPRINT(FINIT, INIT_IQK, ("Path B Rx IQK fail!!\n"));


	return result;

}

void
phy_PathAFillIQKMatrix(
	PADAPTER	pAdapter,
  bool	bIQKOK,
	s4Byte		result[][8],
	u1Byte		final_candidate,
  bool		bTxOnly
	)
{
	u4Byte	Oldval_0, X, TX0_A, reg;
	s4Byte	Y, TX0_C;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	RTPRINT(FINIT, INIT_IQK, ("Path A IQ Calibration %s !\n",(bIQKOK)?"Success":"Failed"));

	if(final_candidate == 0xFF)
		return;

	else if(bIQKOK)
	{
		Oldval_0 = (PHY_QueryBBReg(pAdapter, rOFDM0_XATxIQImbalance, bMaskDWord) >> 22) & 0x3FF;

		X = result[final_candidate][0];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX0_A = (X * Oldval_0) >> 8;
		RTPRINT(FINIT, INIT_IQK, ("X = 0x%x, TX0_A = 0x%x, Oldval_0 0x%x\n", X, TX0_A, Oldval_0));
		PHY_SetBBReg(pAdapter, rOFDM0_XATxIQImbalance, 0x3FF, TX0_A);
		PHY_SetBBReg(pAdapter, rOFDM0_ECCAThreshold, BIT(31), ((X* Oldval_0>>7) & 0x1));

		Y = result[final_candidate][1];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;

		//path B IQK result + 3
		if(pAdapter->interfaceIndex == 1 && pHalData->CurrentBandType92D == BAND_ON_5G)
			Y += 3;

		TX0_C = (Y * Oldval_0) >> 8;
		RTPRINT(FINIT, INIT_IQK, ("Y = 0x%x, TX = 0x%x\n", Y, TX0_C));
		PHY_SetBBReg(pAdapter, rOFDM0_XCTxAFE, 0xF0000000, ((TX0_C&0x3C0)>>6));
		PHY_SetBBReg(pAdapter, rOFDM0_XATxIQImbalance, 0x003F0000, (TX0_C&0x3F));
		PHY_SetBBReg(pAdapter, rOFDM0_ECCAThreshold, BIT(29), ((Y* Oldval_0>>7) & 0x1));

		if(bTxOnly)
		{
			RTPRINT(FINIT, INIT_IQK, ("phy_PathAFillIQKMatrix only Tx OK\n"));
			return;
		}

		reg = result[final_candidate][2];
		PHY_SetBBReg(pAdapter, rOFDM0_XARxIQImbalance, 0x3FF, reg);

		reg = result[final_candidate][3] & 0x3F;
		PHY_SetBBReg(pAdapter, rOFDM0_XARxIQImbalance, 0xFC00, reg);

		reg = (result[final_candidate][3] >> 6) & 0xF;
		PHY_SetBBReg(pAdapter, rOFDM0_RxIQExtAnta, 0xF0000000, reg);
	}
}

void
phy_PathBFillIQKMatrix(
	PADAPTER	pAdapter,
  bool	bIQKOK,
	s4Byte		result[][8],
	u1Byte		final_candidate,
	bool		bTxOnly			//do Tx only
	)
{
	u4Byte	Oldval_1, X, TX1_A, reg;
	s4Byte	Y, TX1_C;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	RTPRINT(FINIT, INIT_IQK, ("Path B IQ Calibration %s !\n",(bIQKOK)?"Success":"Failed"));

	if(final_candidate == 0xFF)
		return;

	else if(bIQKOK)
	{
		Oldval_1 = (PHY_QueryBBReg(pAdapter, rOFDM0_XBTxIQImbalance, bMaskDWord) >> 22) & 0x3FF;

		X = result[final_candidate][4];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX1_A = (X * Oldval_1) >> 8;
		RTPRINT(FINIT, INIT_IQK, ("X = 0x%x, TX1_A = 0x%x\n", X, TX1_A));
		PHY_SetBBReg(pAdapter, rOFDM0_XBTxIQImbalance, 0x3FF, TX1_A);
		PHY_SetBBReg(pAdapter, rOFDM0_ECCAThreshold, BIT(27), ((X* Oldval_1>>7) & 0x1));

		Y = result[final_candidate][5];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		if(pHalData->CurrentBandType92D == BAND_ON_5G)
			Y += 3;		//temp modify for preformance
		TX1_C = (Y * Oldval_1) >> 8;
		RTPRINT(FINIT, INIT_IQK, ("Y = 0x%x, TX1_C = 0x%x\n", Y, TX1_C));
		PHY_SetBBReg(pAdapter, rOFDM0_XDTxAFE, 0xF0000000, ((TX1_C&0x3C0)>>6));
		PHY_SetBBReg(pAdapter, rOFDM0_XBTxIQImbalance, 0x003F0000, (TX1_C&0x3F));
		PHY_SetBBReg(pAdapter, rOFDM0_ECCAThreshold, BIT(25), ((Y* Oldval_1>>7) & 0x1));

		if(bTxOnly)
			return;

		reg = result[final_candidate][6];
		PHY_SetBBReg(pAdapter, rOFDM0_XBRxIQImbalance, 0x3FF, reg);

		reg = result[final_candidate][7] & 0x3F;
		PHY_SetBBReg(pAdapter, rOFDM0_XBRxIQImbalance, 0xFC00, reg);

		reg = (result[final_candidate][7] >> 6) & 0xF;
		PHY_SetBBReg(pAdapter, rOFDM0_AGCRSSITable, 0x0000F000, reg);
	}
}


bool
phy_SimularityCompare_92C(
	PADAPTER	pAdapter,
	s4Byte		result[][8],
	u1Byte		 c1,
	u1Byte		 c2
	)
{
	u4Byte		i, j, diff, SimularityBitMap, bound = 0;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	u1Byte		final_candidate[2] = {0xFF, 0xFF};	//for path A and path B
	bool		bResult = TRUE, is2T = IS_92C_SERIAL( pHalData->VersionID);

	if(is2T)
		bound = 8;
	else
		bound = 4;

	SimularityBitMap = 0;

	for( i = 0; i < bound; i++ )
	{
		diff = (result[c1][i] > result[c2][i]) ? (result[c1][i] - result[c2][i]) : (result[c2][i] - result[c1][i]);
		if (diff > MAX_TOLERANCE)
		{
			if((i == 2 || i == 6) && !SimularityBitMap)
			{
				if(result[c1][i]+result[c1][i+1] == 0)
					final_candidate[(i/4)] = c2;
				else if (result[c2][i]+result[c2][i+1] == 0)
					final_candidate[(i/4)] = c1;
				else
					SimularityBitMap = SimularityBitMap|(1<<i);
			}
			else
				SimularityBitMap = SimularityBitMap|(1<<i);
		}
	}

	if ( SimularityBitMap == 0)
	{
		for( i = 0; i < (bound/4); i++ )
		{
			if(final_candidate[i] != 0xFF)
			{
				for( j = i*4; j < (i+1)*4-2; j++)
					result[3][j] = result[final_candidate[i]][j];
				bResult = FALSE;
			}
		}
		return bResult;
	}
	else if (!(SimularityBitMap & 0x0F))			//path A OK
	{
		for(i = 0; i < 4; i++)
			result[3][i] = result[c1][i];
		return FALSE;
	}
	else if (!(SimularityBitMap & 0xF0) && is2T)	//path B OK
	{
		for(i = 4; i < 8; i++)
			result[3][i] = result[c1][i];
		return FALSE;
	}
	else
		return FALSE;

}

/*
return FALSE => do IQK again
*/
bool
phy_SimularityCompare(
	PADAPTER	pAdapter,
	s4Byte		result[][8],
	u1Byte		 c1,
	u1Byte		 c2
	)
{
	return phy_SimularityCompare_92C(pAdapter, result, c1, c2);
}

void
phy_IQCalibrate_8192C(
	PADAPTER	pAdapter,
	s4Byte		result[][8],
	u1Byte		t,
	bool		is2T
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	u4Byte			i;
	u1Byte			PathAOK, PathBOK;
	u4Byte			ADDA_REG[IQK_ADDA_REG_NUM] = {
						rFPGA0_XCD_SwitchControl,	rBlue_Tooth,
						rRx_Wait_CCA,		rTx_CCK_RFON,
						rTx_CCK_BBON,	rTx_OFDM_RFON,
						rTx_OFDM_BBON,	rTx_To_Rx,
						rTx_To_Tx,		rRx_CCK,
						rRx_OFDM,		rRx_Wait_RIFS,
						rRx_TO_Rx,		rStandby,
						rSleep,				rPMPD_ANAEN };
	u4Byte			IQK_MAC_REG[IQK_MAC_REG_NUM] = {
						REG_TXPAUSE,		REG_BCN_CTRL,
						REG_BCN_CTRL_1,	REG_GPIO_MUXCFG};

	//since 92C & 92D have the different define in IQK_BB_REG
	u4Byte	IQK_BB_REG_92C[IQK_BB_REG_NUM] = {
							rOFDM0_TRxPathEnable,		rOFDM0_TRMuxPar,
							rFPGA0_XCD_RFInterfaceSW,	rConfig_AntA,	rConfig_AntB,
							rFPGA0_XAB_RFInterfaceSW,	rFPGA0_XA_RFInterfaceOE,
							rFPGA0_XB_RFInterfaceOE,	rFPGA0_RFMOD
							};

	u4Byte	IQK_BB_REG_92D[IQK_BB_REG_NUM_92D] = {	//for normal
							rFPGA0_XAB_RFInterfaceSW,	rFPGA0_XA_RFInterfaceOE,
							rFPGA0_XB_RFInterfaceOE,	rOFDM0_TRMuxPar,
							rFPGA0_XCD_RFInterfaceSW,	rOFDM0_TRxPathEnable,
							rFPGA0_RFMOD,			rFPGA0_AnalogParameter4,
							rOFDM0_XAAGCCore1,		rOFDM0_XBAGCCore1
						};
	u4Byte	retryCount;
	retryCount = 2;


	//Neil Chen--2011--05--19--
       //3 Path Div
	u1Byte                 rfPathSwitch=0x0;

	// Note: IQ calibration must be performed after loading
	//		PHY_REG.txt , and radio_a, radio_b.txt

	u4Byte bbvalue;

	if(t==0)
	{
		 bbvalue = PHY_QueryBBReg(pAdapter, rFPGA0_RFMOD, bMaskDWord);
			RTPRINT(FINIT, INIT_IQK, ("phy_IQCalibrate_8192C()==>0x%08x\n",bbvalue));

			RTPRINT(FINIT, INIT_IQK, ("IQ Calibration for %s\n", (is2T ? "2T2R" : "1T1R")));

		// Save ADDA parameters, turn Path A ADDA on
		phy_SaveADDARegisters(pAdapter, ADDA_REG, pHalData->ADDA_backup, IQK_ADDA_REG_NUM);
		phy_SaveMACRegisters(pAdapter, IQK_MAC_REG, pHalData->IQK_MAC_backup);
		phy_SaveADDARegisters(pAdapter, IQK_BB_REG_92C, pHalData->IQK_BB_backup, IQK_BB_REG_NUM);
	}

	phy_PathADDAOn(pAdapter, ADDA_REG, TRUE, is2T);

	if(t==0)
		pHalData->bRfPiEnable = (u1Byte)PHY_QueryBBReg(pAdapter, rFPGA0_XA_HSSIParameter1, BIT(8));

	if(!pHalData->bRfPiEnable){
		// Switch BB to PI mode to do IQ Calibration.
		phy_PIModeSwitch(pAdapter, TRUE);
	}

	PHY_SetBBReg(pAdapter, rFPGA0_RFMOD, BIT24, 0x00);
	PHY_SetBBReg(pAdapter, rOFDM0_TRxPathEnable, bMaskDWord, 0x03a05600);
	PHY_SetBBReg(pAdapter, rOFDM0_TRMuxPar, bMaskDWord, 0x000800e4);
	PHY_SetBBReg(pAdapter, rFPGA0_XCD_RFInterfaceSW, bMaskDWord, 0x22204000);
	PHY_SetBBReg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT10, 0x01);
	PHY_SetBBReg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT26, 0x01);
	PHY_SetBBReg(pAdapter, rFPGA0_XA_RFInterfaceOE, BIT10, 0x00);
	PHY_SetBBReg(pAdapter, rFPGA0_XB_RFInterfaceOE, BIT10, 0x00);

	if(is2T) {
		PHY_SetBBReg(pAdapter, rFPGA0_XA_LSSIParameter, bMaskDWord, 0x00010000);
		PHY_SetBBReg(pAdapter, rFPGA0_XB_LSSIParameter, bMaskDWord, 0x00010000);
	}

	//MAC settings
	phy_MACSettingCalibration(pAdapter, IQK_MAC_REG, pHalData->IQK_MAC_backup);

	//Page B init
	PHY_SetBBReg(pAdapter, rConfig_AntA, bMaskDWord, 0x00080000);

	if(is2T)
		PHY_SetBBReg(pAdapter, rConfig_AntB, bMaskDWord, 0x00080000);
	// IQ calibration setting
	RTPRINT(FINIT, INIT_IQK, ("IQK setting!\n"));
	PHY_SetBBReg(pAdapter, rFPGA0_IQK, bMaskDWord, 0x80800000);
	PHY_SetBBReg(pAdapter, rTx_IQK, bMaskDWord, 0x01007c00);
	PHY_SetBBReg(pAdapter, rRx_IQK, bMaskDWord, 0x01004800);

	for(i = 0 ; i < retryCount ; i++){
		PathAOK = phy_PathA_IQK_8192C(pAdapter, is2T);
		if(PathAOK == 0x03){
			RTPRINT(FINIT, INIT_IQK, ("Path A IQK Success!!\n"));
				result[t][0] = (PHY_QueryBBReg(pAdapter, rTx_Power_Before_IQK_A, bMaskDWord)&0x3FF0000)>>16;
				result[t][1] = (PHY_QueryBBReg(pAdapter, rTx_Power_After_IQK_A, bMaskDWord)&0x3FF0000)>>16;
				result[t][2] = (PHY_QueryBBReg(pAdapter, rRx_Power_Before_IQK_A_2, bMaskDWord)&0x3FF0000)>>16;
				result[t][3] = (PHY_QueryBBReg(pAdapter, rRx_Power_After_IQK_A_2, bMaskDWord)&0x3FF0000)>>16;
			break;
		}
		else if (i == (retryCount-1) && PathAOK == 0x01)	//Tx IQK OK
		{
			RTPRINT(FINIT, INIT_IQK, ("Path A IQK Only  Tx Success!!\n"));

			result[t][0] = (PHY_QueryBBReg(pAdapter, rTx_Power_Before_IQK_A, bMaskDWord)&0x3FF0000)>>16;
			result[t][1] = (PHY_QueryBBReg(pAdapter, rTx_Power_After_IQK_A, bMaskDWord)&0x3FF0000)>>16;
		}
	}

	if(0x00 == PathAOK){
		RTPRINT(FINIT, INIT_IQK, ("Path A IQK failed!!\n"));
	}

	if(is2T){
		phy_PathAStandBy(pAdapter);

		// Turn Path B ADDA on
		phy_PathADDAOn(pAdapter, ADDA_REG, FALSE, is2T);

		for(i = 0 ; i < retryCount ; i++){
			PathBOK = phy_PathB_IQK_8192C(pAdapter);
			if(PathBOK == 0x03){
				RTPRINT(FINIT, INIT_IQK, ("Path B IQK Success!!\n"));
				result[t][4] = (PHY_QueryBBReg(pAdapter, rTx_Power_Before_IQK_B, bMaskDWord)&0x3FF0000)>>16;
				result[t][5] = (PHY_QueryBBReg(pAdapter, rTx_Power_After_IQK_B, bMaskDWord)&0x3FF0000)>>16;
				result[t][6] = (PHY_QueryBBReg(pAdapter, rRx_Power_Before_IQK_B_2, bMaskDWord)&0x3FF0000)>>16;
				result[t][7] = (PHY_QueryBBReg(pAdapter, rRx_Power_After_IQK_B_2, bMaskDWord)&0x3FF0000)>>16;
				break;
			}
			else if (i == (retryCount - 1) && PathBOK == 0x01)	//Tx IQK OK
			{
				RTPRINT(FINIT, INIT_IQK, ("Path B Only Tx IQK Success!!\n"));
				result[t][4] = (PHY_QueryBBReg(pAdapter, rTx_Power_Before_IQK_B, bMaskDWord)&0x3FF0000)>>16;
				result[t][5] = (PHY_QueryBBReg(pAdapter, rTx_Power_After_IQK_B, bMaskDWord)&0x3FF0000)>>16;
			}
		}

		if(0x00 == PathBOK){
			RTPRINT(FINIT, INIT_IQK, ("Path B IQK failed!!\n"));
		}
	}

	//Back to BB mode, load original value
	RTPRINT(FINIT, INIT_IQK, ("IQK:Back to BB mode, load original value!\n"));
	PHY_SetBBReg(pAdapter, rFPGA0_IQK, bMaskDWord, 0);

	if(t!=0)
	{
		if(!pHalData->bRfPiEnable){
			// Switch back BB to SI mode after finish IQ Calibration.
			phy_PIModeSwitch(pAdapter, FALSE);
		}

		// Reload ADDA power saving parameters
		phy_ReloadADDARegisters(pAdapter, ADDA_REG, pHalData->ADDA_backup, IQK_ADDA_REG_NUM);

		// Reload MAC parameters
		phy_ReloadMACRegisters(pAdapter, IQK_MAC_REG, pHalData->IQK_MAC_backup);

		// Reload BB parameters
		phy_ReloadADDARegisters(pAdapter, IQK_BB_REG_92C, pHalData->IQK_BB_backup, IQK_BB_REG_NUM);

		// Restore RX initial gain
		PHY_SetBBReg(pAdapter, rFPGA0_XA_LSSIParameter, bMaskDWord, 0x00032ed3);
		if(is2T)
			PHY_SetBBReg(pAdapter, rFPGA0_XB_LSSIParameter, bMaskDWord, 0x00032ed3);
		//load 0xe30 IQC default value
		PHY_SetBBReg(pAdapter, rTx_IQK_Tone_A, bMaskDWord, 0x01008c00);
		PHY_SetBBReg(pAdapter, rRx_IQK_Tone_A, bMaskDWord, 0x01008c00);

	}
	RTPRINT(FINIT, INIT_IQK, ("phy_IQCalibrate_8192C() <==\n"));
}

void
phy_LCCalibrate92C(
	PADAPTER	pAdapter,
	bool		is2T
	)
{
	u1Byte	tmpReg;
	u4Byte	RF_Amode=0, RF_Bmode=0, LC_Cal;
//	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	//Check continuous TX and Packet TX
	tmpReg = PlatformEFIORead1Byte(pAdapter, 0xd03);

	if((tmpReg&0x70) != 0)			//Deal with contisuous TX case
		PlatformEFIOWrite1Byte(pAdapter, 0xd03, tmpReg&0x8F);	//disable all continuous TX
	else							// Deal with Packet TX case
		PlatformEFIOWrite1Byte(pAdapter, REG_TXPAUSE, 0xFF);			// block all queues

	if((tmpReg&0x70) != 0)
	{
		//1. Read original RF mode
		//Path-A
		RF_Amode = PHY_QueryRFReg(pAdapter, RF_PATH_A, RF_AC, bMask12Bits);

		//Path-B
		if(is2T)
			RF_Bmode = PHY_QueryRFReg(pAdapter, RF_PATH_B, RF_AC, bMask12Bits);

		//2. Set RF mode = standby mode
		//Path-A
		PHY_SetRFReg(pAdapter, RF_PATH_A, RF_AC, bMask12Bits, (RF_Amode&0x8FFFF)|0x10000);

		//Path-B
		if(is2T)
			PHY_SetRFReg(pAdapter, RF_PATH_B, RF_AC, bMask12Bits, (RF_Bmode&0x8FFFF)|0x10000);
	}

	//3. Read RF reg18
	LC_Cal = PHY_QueryRFReg(pAdapter, RF_PATH_A, RF_CHNLBW, bMask12Bits);

	//4. Set LC calibration begin	bit15
	PHY_SetRFReg(pAdapter, RF_PATH_A, RF_CHNLBW, bMask12Bits, LC_Cal|0x08000);

	delay_ms(100);


	//Restore original situation
	if((tmpReg&0x70) != 0)	//Deal with contisuous TX case
	{
		//Path-A
		PlatformEFIOWrite1Byte(pAdapter, 0xd03, tmpReg);
		PHY_SetRFReg(pAdapter, RF_PATH_A, RF_AC, bMask12Bits, RF_Amode);

		//Path-B
		if(is2T)
			PHY_SetRFReg(pAdapter, RF_PATH_B, RF_AC, bMask12Bits, RF_Bmode);
	}
	else // Deal with Packet TX case
	{
		PlatformEFIOWrite1Byte(pAdapter, REG_TXPAUSE, 0x00);
	}
}


void
phy_LCCalibrate(
	PADAPTER	pAdapter,
	bool		is2T
	)
{
	phy_LCCalibrate92C(pAdapter, is2T);
}



//Analog Pre-distortion calibration
#define		APK_BB_REG_NUM	8
#define		APK_CURVE_REG_NUM 4
#define		PATH_NUM		2

void
phy_APCalibrate_8192C(
	PADAPTER	pAdapter,
	s1Byte		delta,
	bool		is2T
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	u4Byte			regD[PATH_NUM];
	u4Byte			tmpReg, index, offset, i, apkbound;
	u1Byte			path, pathbound = PATH_NUM;
	u4Byte			BB_backup[APK_BB_REG_NUM];
	u4Byte			BB_REG[APK_BB_REG_NUM] = {
						rFPGA1_TxBlock,		rOFDM0_TRxPathEnable,
						rFPGA0_RFMOD,	rOFDM0_TRMuxPar,
						rFPGA0_XCD_RFInterfaceSW,	rFPGA0_XAB_RFInterfaceSW,
						rFPGA0_XA_RFInterfaceOE,	rFPGA0_XB_RFInterfaceOE	};
	u4Byte			BB_AP_MODE[APK_BB_REG_NUM] = {
						0x00000020, 0x00a05430, 0x02040000,
						0x000800e4, 0x00204000 };
	u4Byte			BB_normal_AP_MODE[APK_BB_REG_NUM] = {
						0x00000020, 0x00a05430, 0x02040000,
						0x000800e4, 0x22204000 };

	u4Byte			AFE_backup[IQK_ADDA_REG_NUM];
	u4Byte			AFE_REG[IQK_ADDA_REG_NUM] = {
						rFPGA0_XCD_SwitchControl,	rBlue_Tooth,
						rRx_Wait_CCA,		rTx_CCK_RFON,
						rTx_CCK_BBON,	rTx_OFDM_RFON,
						rTx_OFDM_BBON,	rTx_To_Rx,
						rTx_To_Tx,		rRx_CCK,
						rRx_OFDM,		rRx_Wait_RIFS,
						rRx_TO_Rx,		rStandby,
						rSleep,				rPMPD_ANAEN };

	u4Byte			MAC_backup[IQK_MAC_REG_NUM];
	u4Byte			MAC_REG[IQK_MAC_REG_NUM] = {
						REG_TXPAUSE,		REG_BCN_CTRL,
						REG_BCN_CTRL_1,	REG_GPIO_MUXCFG};

	u4Byte			APK_RF_init_value[PATH_NUM][APK_BB_REG_NUM] = {
					{0x0852c, 0x1852c, 0x5852c, 0x1852c, 0x5852c},
					{0x2852e, 0x0852e, 0x3852e, 0x0852e, 0x0852e}
					};

	u4Byte			APK_normal_RF_init_value[PATH_NUM][APK_BB_REG_NUM] = {
					{0x0852c, 0x0a52c, 0x3a52c, 0x5a52c, 0x5a52c},	//path settings equal to path b settings
					{0x0852c, 0x0a52c, 0x5a52c, 0x5a52c, 0x5a52c}
					};

	u4Byte			APK_RF_value_0[PATH_NUM][APK_BB_REG_NUM] = {
					{0x52019, 0x52014, 0x52013, 0x5200f, 0x5208d},
					{0x5201a, 0x52019, 0x52016, 0x52033, 0x52050}
					};

	u4Byte			APK_normal_RF_value_0[PATH_NUM][APK_BB_REG_NUM] = {
					{0x52019, 0x52017, 0x52010, 0x5200d, 0x5206a},	//path settings equal to path b settings
					{0x52019, 0x52017, 0x52010, 0x5200d, 0x5206a}
					};
#if 0
	u4Byte			APK_RF_value_A[PATH_NUM][APK_BB_REG_NUM] = {
					{0x1adb0, 0x1adb0, 0x1ada0, 0x1ad90, 0x1ad80},
					{0x00fb0, 0x00fb0, 0x00fa0, 0x00f90, 0x00f80}
					};
#endif
	u4Byte			AFE_on_off[PATH_NUM] = {
					0x04db25a4, 0x0b1b25a4};	//path A on path B off / path A off path B on

	u4Byte			APK_offset[PATH_NUM] = {
					rConfig_AntA, rConfig_AntB};

	u4Byte			APK_normal_offset[PATH_NUM] = {
					rConfig_Pmpd_AntA, rConfig_Pmpd_AntB};

	u4Byte			APK_value[PATH_NUM] = {
					0x92fc0000, 0x12fc0000};

	u4Byte			APK_normal_value[PATH_NUM] = {
					0x92680000, 0x12680000};

	s1Byte			APK_delta_mapping[APK_BB_REG_NUM][13] = {
					{-4, -3, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
					{-4, -3, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
					{-6, -4, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
					{-1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6},
					{-11, -9, -7, -5, -3, -1, 0, 0, 0, 0, 0, 0, 0}
					};

	u4Byte			APK_normal_setting_value_1[13] = {
					0x01017018, 0xf7ed8f84, 0x1b1a1816, 0x2522201e, 0x322e2b28,
					0x433f3a36, 0x5b544e49, 0x7b726a62, 0xa69a8f84, 0xdfcfc0b3,
					0x12680000, 0x00880000, 0x00880000
					};

	u4Byte			APK_normal_setting_value_2[16] = {
					0x01c7021d, 0x01670183, 0x01000123, 0x00bf00e2, 0x008d00a3,
					0x0068007b, 0x004d0059, 0x003a0042, 0x002b0031, 0x001f0025,
					0x0017001b, 0x00110014, 0x000c000f, 0x0009000b, 0x00070008,
					0x00050006
					};

	u4Byte			APK_result[PATH_NUM][APK_BB_REG_NUM];	//val_1_1a, val_1_2a, val_2a, val_3a, val_4a
//	u4Byte			AP_curve[PATH_NUM][APK_CURVE_REG_NUM];

	s4Byte			BB_offset, delta_V, delta_offset;

	RTPRINT(FINIT, INIT_IQK, ("==>phy_APCalibrate_8192C() delta %d\n", delta));
	RTPRINT(FINIT, INIT_IQK, ("AP Calibration for %s\n", (is2T ? "2T2R" : "1T1R")));
	if(!is2T)
		pathbound = 1;

	//2 FOR NORMAL CHIP SETTINGS

// Temporarily do not allow normal driver to do the following settings because these offset
// and value will cause RF internal PA to be unpredictably disabled by HW, such that RF Tx signal
// will disappear after disable/enable card many times on 88CU. RF SD and DD have not find the
// root cause, so we remove these actions temporarily. Added by tynli and SD3 Allen. 2010.05.31.
	return;
}


void
PHY_IQCalibrate_8192C(
	PADAPTER	pAdapter,
	bool		bReCovery
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	s4Byte			result[4][8];	//last is final result
	u1Byte			i, final_candidate, Indexforchannel;
	bool			bPathAOK, bPathBOK;
	s4Byte			RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC, RegTmp = 0;
	bool			is12simular, is13simular, is23simular;
	bool			bStartContTx = FALSE, bSingleTone = FALSE, bCarrierSuppression = FALSE;
	u4Byte			IQK_BB_REG_92C[IQK_BB_REG_NUM] = {
					rOFDM0_XARxIQImbalance,		rOFDM0_XBRxIQImbalance,
					rOFDM0_ECCAThreshold,	rOFDM0_AGCRSSITable,
					rOFDM0_XATxIQImbalance,		rOFDM0_XBTxIQImbalance,
					rOFDM0_XCTxAFE,				rOFDM0_XDTxAFE,
					rOFDM0_RxIQExtAnta};

	if (ODM_CheckPowerStatus(pAdapter) == FALSE)
		return;

	//ignore IQK when continuous Tx
	if(bStartContTx || bSingleTone || bCarrierSuppression)
		return;

#if DISABLE_BB_RF
	return;
#endif
	if(pAdapter->bSlaveOfDMSP)
		return;

	if(bReCovery) {
		phy_ReloadADDARegisters(pAdapter, IQK_BB_REG_92C, pHalData->IQK_BB_backup_recover, 9);
		return;
	}
	RTPRINT(FINIT, INIT_IQK, ("IQK:Start!!!\n"));

	for(i = 0; i < 8; i++)
	{
		result[0][i] = 0;
		result[1][i] = 0;
		result[2][i] = 0;
		result[3][i] = 0;
	}
	final_candidate = 0xff;
	bPathAOK = FALSE;
	bPathBOK = FALSE;
	is12simular = FALSE;
	is23simular = FALSE;
	is13simular = FALSE;


	RTPRINT(FINIT, INIT_IQK, ("IQK !!!interface %d currentband %d\n", pAdapter->interfaceIndex, pHalData->CurrentBandType92D));
	AcquireCCKAndRWPageAControl(pAdapter);
//	RT_TRACE(COMP_INIT,DBG_LOUD,("Acquire Mutex in IQCalibrate \n"));
	for (i=0; i<3; i++)
	{
		if(IS_92C_SERIAL( pHalData->VersionID)) {
			phy_IQCalibrate_8192C(pAdapter, result, i, TRUE);
		} else {
			// For 88C 1T1R
			phy_IQCalibrate_8192C(pAdapter, result, i, FALSE);
		}

		if(i == 1) {
			is12simular = phy_SimularityCompare(pAdapter, result, 0, 1);
			if(is12simular) {
				final_candidate = 0;
				break;
			}
		}

		if(i == 2) {
			is13simular = phy_SimularityCompare(pAdapter, result, 0, 2);
			if(is13simular) {
				final_candidate = 0;
				break;
			}

			is23simular = phy_SimularityCompare(pAdapter, result, 1, 2);
			if(is23simular)
				final_candidate = 1;
			else
			{
				for(i = 0; i < 8; i++)
					RegTmp += result[3][i];

				if(RegTmp != 0)
					final_candidate = 3;
				else
					final_candidate = 0xFF;
			}
		}
	}
	ReleaseCCKAndRWPageAControl(pAdapter);

	for (i=0; i<4; i++)
	{
		RegE94 = result[i][0];
		RegE9C = result[i][1];
		RegEA4 = result[i][2];
		RegEAC = result[i][3];
		RegEB4 = result[i][4];
		RegEBC = result[i][5];
		RegEC4 = result[i][6];
		RegECC = result[i][7];
		RTPRINT(FINIT, INIT_IQK, ("IQK: RegE94=%x RegE9C=%x RegEA4=%x RegEAC=%x RegEB4=%x RegEBC=%x RegEC4=%x RegECC=%x\n ", RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC));
	}

	if(final_candidate != 0xff)
	{
		pHalData->RegE94 = RegE94 = result[final_candidate][0];
		pHalData->RegE9C = RegE9C = result[final_candidate][1];
		RegEA4 = result[final_candidate][2];
		RegEAC = result[final_candidate][3];
		pHalData->RegEB4 = RegEB4 = result[final_candidate][4];
		pHalData->RegEBC = RegEBC = result[final_candidate][5];
		RegEC4 = result[final_candidate][6];
		RegECC = result[final_candidate][7];
		RTPRINT(FINIT, INIT_IQK, ("IQK: final_candidate is %x\n",final_candidate));
		RTPRINT(FINIT, INIT_IQK, ("IQK: RegE94=%x RegE9C=%x RegEA4=%x RegEAC=%x RegEB4=%x RegEBC=%x RegEC4=%x RegECC=%x\n ", RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC));
		bPathAOK = bPathBOK = TRUE;
	}
	else
	{
		RegE94 = RegEB4 = pHalData->RegE94 = pHalData->RegEB4 = 0x100;	//X default value
		RegE9C = RegEBC = pHalData->RegE9C = pHalData->RegEBC = 0x0;		//Y default value
	}

	if((RegE94 != 0)/*&&(RegEA4 != 0)*/)
	{
		if(pHalData->CurrentBandType92D == BAND_ON_5G)
			phy_PathAFillIQKMatrix_5G_Normal(pAdapter, bPathAOK, result, final_candidate, (RegEA4 == 0));
		else
			phy_PathAFillIQKMatrix(pAdapter, bPathAOK, result, final_candidate, (RegEA4 == 0));

	}

	if (IS_92C_SERIAL(pHalData->VersionID) || IS_92D_SINGLEPHY(pHalData->VersionID))
	{
		if((RegEB4 != 0)/*&&(RegEC4 != 0)*/)
		{
			if(pHalData->CurrentBandType92D == BAND_ON_5G)
				phy_PathBFillIQKMatrix_5G_Normal(pAdapter, bPathBOK, result, final_candidate, (RegEC4 == 0));
			else
				phy_PathBFillIQKMatrix(pAdapter, bPathBOK, result, final_candidate, (RegEC4 == 0));
		}
	}

	phy_SaveADDARegisters(pAdapter, IQK_BB_REG_92C, pHalData->IQK_BB_backup_recover, 9);

}


void
PHY_LCCalibrate_8192C(
	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	bool			bStartContTx = FALSE, bSingleTone = FALSE, bCarrierSuppression = FALSE;
	PMGNT_INFO		pMgntInfo=&pAdapter->MgntInfo;
	PMGNT_INFO		pMgntInfoBuddyAdapter;
	u4Byte			timeout = 2000, timecount = 0;
	PADAPTER	BuddyAdapter = pAdapter->BuddyAdapter;

#if DISABLE_BB_RF
	return;
#endif

	//ignore LCK when continuous Tx
	if(bStartContTx || bSingleTone || bCarrierSuppression)
		return;

	if(BuddyAdapter != NULL &&
		((pAdapter->interfaceIndex == 0 && pHalData->CurrentBandType92D == BAND_ON_2_4G) ||
		(pAdapter->interfaceIndex == 1 && pHalData->CurrentBandType92D == BAND_ON_5G)))
	{
		pMgntInfoBuddyAdapter=&BuddyAdapter->MgntInfo;
		while(pMgntInfoBuddyAdapter->bScanInProgress && timecount < timeout)
		{
			delay_ms(50);
			timecount += 50;
		}
	}

	while(pMgntInfo->bScanInProgress && timecount < timeout)
	{
		delay_ms(50);
		timecount += 50;
	}

	pHalData->bLCKInProgress = TRUE;

	RTPRINT(FINIT, INIT_IQK, ("LCK:Start!!!interface %d currentband %x delay %d ms\n", pAdapter->interfaceIndex, pHalData->CurrentBandType92D, timecount));

	//if(IS_92C_SERIAL(pHalData->VersionID) || IS_92D_SINGLEPHY(pHalData->VersionID))
	if(IS_2T2R(pHalData->VersionID))
	{
		phy_LCCalibrate(pAdapter, TRUE);
	}
	else{
		// For 88C 1T1R
		phy_LCCalibrate(pAdapter, FALSE);
	}

	pHalData->bLCKInProgress = FALSE;

	RTPRINT(FINIT, INIT_IQK, ("LCK:Finish!!!interface %d\n", pAdapter->interfaceIndex));


}

void
PHY_APCalibrate_8192C(
	PADAPTER	pAdapter,
	s1Byte		delta
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	//default disable APK, because Tx NG issue, suggest by Jenyu, 2011.11.25
	return;
}


#endif


//3============================================================
//3 IQ Calibration
//3============================================================

void
ODM_ResetIQKResult(
	PDM_ODM_T	pDM_Odm
)
{
	u1Byte		i;
#if (DM_ODM_SUPPORT_TYPE == ODM_MP || DM_ODM_SUPPORT_TYPE == ODM_CE)
	PADAPTER	Adapter = pDM_Odm->Adapter;

	return;
#endif
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD,("PHY_ResetIQKResult:: settings regs %d default regs %d\n", (u32)(sizeof(pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting)/sizeof(IQK_MATRIX_REGS_SETTING)), IQK_Matrix_Settings_NUM));
	//0xe94, 0xe9c, 0xea4, 0xeac, 0xeb4, 0xebc, 0xec4, 0xecc

	for(i = 0; i < IQK_Matrix_Settings_NUM; i++)
	{
		{
			pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[i].Value[0][0] =
				pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[i].Value[0][2] =
				pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[i].Value[0][4] =
				pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[i].Value[0][6] = 0x100;

			pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[i].Value[0][1] =
				pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[i].Value[0][3] =
				pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[i].Value[0][5] =
				pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[i].Value[0][7] = 0x0;

			pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[i].bIQKDone = FALSE;

		}
	}

}
#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
u1Byte ODM_GetRightChnlPlaceforIQK(u1Byte chnl)
{
	u1Byte	channel_all[ODM_TARGET_CHNL_NUM_2G_5G] =
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,149,151,153,155,157,159,161,163,165};
	u1Byte	place = chnl;


	if(chnl > 14)
	{
		for(place = 14; place<sizeof(channel_all); place++)
		{
			if(channel_all[place] == chnl)
			{
				return place-13;
			}
		}
	}
	return 0;

}
#endif
