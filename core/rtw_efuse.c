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
#define _RTW_EFUSE_C_

#include <osdep_service.h>
#include <drv_types.h>

#include <rtw_efuse.h>

/*------------------------Define local variable------------------------------*/

/*  */
#define REG_EFUSE_CTRL		0x0030
#define EFUSE_CTRL			REG_EFUSE_CTRL		/*  E-Fuse Control. */
/*  */

/*-----------------------------------------------------------------------------
 * Function:	Efuse_PowerSwitch
 *
 * Overview:	When we want to enable write operation, we should change to
 *				pwr on state. When we stop write, we should switch to 500k mode
 *				and disable LDO 2.5V.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/17/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void
Efuse_PowerSwitch(
	struct rtw_adapter *	pAdapter,
	u8		bWrite,
	u8		PwrState)
{
	pAdapter->HalFunc.EfusePowerSwitch(pAdapter, bWrite, PwrState);
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_GetCurrentSize
 *
 * Overview:	Get current efuse size!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
u16
Efuse_GetCurrentSize(struct rtw_adapter *pAdapter, u8 efuseType)
{
	u16 ret = 0;

	ret = pAdapter->HalFunc.EfuseGetCurrentSize(pAdapter, efuseType);

	return ret;
}

/*  11/16/2008 MH Add description. Get current efuse area enabled word!!. */
u8
Efuse_CalculateWordCnts(u8 word_en)
{
	u8 word_cnts = 0;
	if(!(word_en & BIT(0)))	word_cnts++; /*  0 : write enable */
	if(!(word_en & BIT(1)))	word_cnts++;
	if(!(word_en & BIT(2)))	word_cnts++;
	if(!(word_en & BIT(3)))	word_cnts++;
	return word_cnts;
}

/*  */
/*	Description: */
/*		Execute E-Fuse read byte operation. */
/*		Refered from SD1 Richard. */
/*  */
/*	Assumption: */
/*		1. Boot from E-Fuse and successfully auto-load. */
/*		2. PASSIVE_LEVEL (USB interface) */
/*  */
/*	Created by Roger, 2008.10.21. */
/*  */
void
ReadEFuseByte(struct rtw_adapter *Adapter, u16 _offset, u8 *pbuf)
{
	u32	value32;
	u8	readbyte;
	u16	retry;
	/* u32 start=rtw_get_current_time(); */

	/* Write Address */
	rtw_write8(Adapter, EFUSE_CTRL+1, (_offset & 0xff));
	readbyte = rtw_read8(Adapter, EFUSE_CTRL+2);
	rtw_write8(Adapter, EFUSE_CTRL+2, ((_offset >> 8) & 0x03) | (readbyte & 0xfc));

	/* Write bit 32 0 */
	readbyte = rtw_read8(Adapter, EFUSE_CTRL+3);
	rtw_write8(Adapter, EFUSE_CTRL+3, (readbyte & 0x7f));

	/* Check bit 32 read-ready */
	retry = 0;
	value32 = rtw_read32(Adapter, EFUSE_CTRL);
	/* while(!(((value32 >> 24) & 0xff) & 0x80)  && (retry<10)) */
	while(!(((value32 >> 24) & 0xff) & 0x80)  && (retry<10000))
	{
		value32 = rtw_read32(Adapter, EFUSE_CTRL);
		retry++;
	}

	/*  20100205 Joseph: Add delay suggested by SD1 Victor. */
	/*  This fix the problem that Efuse read error in high temperature condition. */
	/*  Designer says that there shall be some delay after ready bit is set, or the */
	/*  result will always stay on last data we read. */
	udelay(50);
	value32 = rtw_read32(Adapter, EFUSE_CTRL);

	*pbuf = (u8)(value32 & 0xff);
	/* DBG_8723A("ReadEFuseByte _offset:%08u, in %d ms\n",_offset ,rtw_get_passing_time_ms(start)); */
}

/*  */
/*	Description: */
/*		1. Execute E-Fuse read byte operation according as map offset and */
/*		    save to E-Fuse table. */
/*		2. Refered from SD1 Richard. */
/*  */
/*	Assumption: */
/*		1. Boot from E-Fuse and successfully auto-load. */
/*		2. PASSIVE_LEVEL (USB interface) */
/*  */
/*	Created by Roger, 2008.10.21. */
/*  */
/*	2008/12/12 MH	1. Reorganize code flow and reserve bytes. and add description. */
/*					2. Add efuse utilization collect. */
/*	2008/12/22 MH	Read Efuse must check if we write section 1 data again!!! Sec1 */
/*					write addr must be after sec5. */
/*  */

void
efuse_ReadEFuse(struct rtw_adapter *Adapter, u8 efuseType,
		u16 _offset, u16 _size_byte, u8 *pbuf);
void
efuse_ReadEFuse(struct rtw_adapter *Adapter, u8 efuseType,
		u16 _offset, u16 _size_byte, u8 *pbuf)
{
	Adapter->HalFunc.ReadEFuse(Adapter, efuseType, _offset,
				   _size_byte, pbuf);
}

void
EFUSE_GetEfuseDefinition(struct rtw_adapter *pAdapter, u8 efuseType,
			 u8 type, void *pOut)
{
	pAdapter->HalFunc.EFUSEGetEfuseDefinition(pAdapter, efuseType,
						  type, pOut);
}

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_Read1Byte
 *
 * Overview:	Copy from WMAC fot EFUSE read 1 byte.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 09/23/2008	MHC		Copy from WMAC.
 *
 *---------------------------------------------------------------------------*/
u8
EFUSE_Read1Byte(struct rtw_adapter *Adapter, u16 Address)
{
	u8	data;
	u8	Bytetemp = {0x00};
	u8	temp = {0x00};
	u32	k=0;
	u16	contentLen=0;

	EFUSE_GetEfuseDefinition(Adapter, EFUSE_WIFI,
				 TYPE_EFUSE_REAL_CONTENT_LEN,
				 (void *)&contentLen);

	if (Address < contentLen)	/* E-fuse 512Byte */
	{
		/* Write E-fuse Register address bit0~7 */
		temp = Address & 0xFF;
		rtw_write8(Adapter, EFUSE_CTRL+1, temp);
		Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+2);
		/* Write E-fuse Register address bit8~9 */
		temp = ((Address >> 8) & 0x03) | (Bytetemp & 0xFC);
		rtw_write8(Adapter, EFUSE_CTRL+2, temp);

		/* Write 0x30[31]=0 */
		Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+3);
		temp = Bytetemp & 0x7F;
		rtw_write8(Adapter, EFUSE_CTRL+3, temp);

		/* Wait Write-ready (0x30[31]=1) */
		Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+3);
		while(!(Bytetemp & 0x80))
		{
			Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+3);
			k++;
			if(k==1000)
			{
				k=0;
				break;
			}
		}
		data=rtw_read8(Adapter, EFUSE_CTRL);
		return data;
	}
	else
		return 0xFF;
}/* EFUSE_Read1Byte */

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_Write1Byte
 *
 * Overview:	Copy from WMAC fot EFUSE write 1 byte.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 09/23/2008	MHC		Copy from WMAC.
 *
 *---------------------------------------------------------------------------*/

void
EFUSE_Write1Byte(
	struct rtw_adapter *	Adapter,
	u16		Address,
	u8		Value);
void
EFUSE_Write1Byte(
	struct rtw_adapter *	Adapter,
	u16		Address,
	u8		Value)
{
	u8	Bytetemp = {0x00};
	u8	temp = {0x00};
	u32	k=0;
	u16	contentLen=0;

	/* RT_TRACE(COMP_EFUSE, DBG_LOUD, ("Addr=%x Data =%x\n", Address, Value)); */
	EFUSE_GetEfuseDefinition(Adapter, EFUSE_WIFI,
				 TYPE_EFUSE_REAL_CONTENT_LEN,
				 (void *)&contentLen);

	if( Address < contentLen)	/* E-fuse 512Byte */
	{
		rtw_write8(Adapter, EFUSE_CTRL, Value);

		/* Write E-fuse Register address bit0~7 */
		temp = Address & 0xFF;
		rtw_write8(Adapter, EFUSE_CTRL+1, temp);
		Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+2);

		/* Write E-fuse Register address bit8~9 */
		temp = ((Address >> 8) & 0x03) | (Bytetemp & 0xFC);
		rtw_write8(Adapter, EFUSE_CTRL+2, temp);

		/* Write 0x30[31]=1 */
		Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+3);
		temp = Bytetemp | 0x80;
		rtw_write8(Adapter, EFUSE_CTRL+3, temp);

		/* Wait Write-ready (0x30[31]=0) */
		Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+3);
		while(Bytetemp & 0x80)
		{
			Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+3);
			k++;
			if(k==100)
			{
				k=0;
				break;
			}
		}
	}
}/* EFUSE_Write1Byte */

/*  11/16/2008 MH Read one byte from real Efuse. */
u8
efuse_OneByteRead(struct rtw_adapter *pAdapter, u16 addr, u8 *data)
{
	u8	tmpidx = 0;
	u8	bResult;

	/*  -----------------e-fuse reg ctrl --------------------------------- */
	/* address */
	rtw_write8(pAdapter, EFUSE_CTRL+1, (u8)(addr&0xff));
	rtw_write8(pAdapter, EFUSE_CTRL+2, ((u8)((addr>>8) &0x03) ) |
	(rtw_read8(pAdapter, EFUSE_CTRL+2)&0xFC ));

	rtw_write8(pAdapter, EFUSE_CTRL+3,  0x72);/* read cmd */

	while(!(0x80 &rtw_read8(pAdapter, EFUSE_CTRL+3))&&(tmpidx<100))
	{
		tmpidx++;
	}
	if(tmpidx<100)
	{
		*data=rtw_read8(pAdapter, EFUSE_CTRL);
		bResult = true;
	}
	else
	{
		*data = 0xff;
		bResult = false;
	}
	return bResult;
}

/*  11/16/2008 MH Write one byte to reald Efuse. */
u8
efuse_OneByteWrite(struct rtw_adapter *pAdapter, u16 addr, u8 data)
{
	u8	tmpidx = 0;
	u8	bResult;

	/* RT_TRACE(COMP_EFUSE, DBG_LOUD, ("Addr = %x Data=%x\n", addr, data)); */

	/* return	0; */

	/*  -----------------e-fuse reg ctrl --------------------------------- */
	/* address */
	rtw_write8(pAdapter, EFUSE_CTRL+1, (u8)(addr&0xff));
	rtw_write8(pAdapter, EFUSE_CTRL+2,
	(rtw_read8(pAdapter, EFUSE_CTRL+2)&0xFC )|(u8)((addr>>8)&0x03) );
	rtw_write8(pAdapter, EFUSE_CTRL, data);/* data */

	rtw_write8(pAdapter, EFUSE_CTRL+3, 0xF2);/* write cmd */

	while((0x80 &  rtw_read8(pAdapter, EFUSE_CTRL+3)) && (tmpidx<100) ){
		tmpidx++;
	}

	if(tmpidx<100)
	{
		bResult = true;
	}
	else
	{
		bResult = false;
	}

	return bResult;
}

int
Efuse_PgPacketRead(struct rtw_adapter *pAdapter, u8 offset, u8 *data)
{
	int	ret=0;

	ret =  pAdapter->HalFunc.Efuse_PgPacketRead(pAdapter, offset, data);

	return ret;
}

int
Efuse_PgPacketWrite(struct rtw_adapter *pAdapter, u8 offset,
		    u8 word_en, u8 *data)
{
	int ret;

	ret =  pAdapter->HalFunc.Efuse_PgPacketWrite(pAdapter, offset,
						     word_en, data);

	return ret;
}

static int
Efuse_PgPacketWrite_BT(struct rtw_adapter *pAdapter, u8	offset,
		       u8 word_en, u8 *data)
{
	int ret;

	ret =  pAdapter->HalFunc.Efuse_PgPacketWrite_BT(pAdapter, offset,
							word_en, data);

	return ret;
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataRead
 *
 * Overview:	Read allowed word in current efuse section data.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008	MHC		Create Version 0.
 * 11/21/2008	MHC		Fix Write bug when we only enable late word.
 *
 *---------------------------------------------------------------------------*/
void
efuse_WordEnableDataRead(u8	word_en,
			 u8	*sourdata,
			 u8	*targetdata)
{
	if (!(word_en&BIT(0)))
	{
		targetdata[0] = sourdata[0];
		targetdata[1] = sourdata[1];
	}
	if (!(word_en&BIT(1)))
	{
		targetdata[2] = sourdata[2];
		targetdata[3] = sourdata[3];
	}
	if (!(word_en&BIT(2)))
	{
		targetdata[4] = sourdata[4];
		targetdata[5] = sourdata[5];
	}
	if (!(word_en&BIT(3)))
	{
		targetdata[6] = sourdata[6];
		targetdata[7] = sourdata[7];
	}
}

u8
Efuse_WordEnableDataWrite(struct rtw_adapter *pAdapter, u16 efuse_addr,
			  u8 word_en, u8 *data)
{
	u8 ret = 0;

	ret = pAdapter->HalFunc.Efuse_WordEnableDataWrite(pAdapter, efuse_addr,
							  word_en, data);

	return ret;
}

static u8 efuse_read8(struct rtw_adapter *padapter, u16 address, u8 *value)
{
	return efuse_OneByteRead(padapter, address, value);
}

static u8 efuse_write8(struct rtw_adapter *padapter, u16 address, u8 *value)
{
	return efuse_OneByteWrite(padapter, address, *value);
}

/*
 * read/wirte raw efuse data
 */
u8 rtw_efuse_access(struct rtw_adapter *padapter, u8 bWrite, u16 start_addr,
		    u16 cnts, u8 *data)
{
	int i = 0;
	u16	real_content_len = 0, max_available_size = 0;
	u8 res = _FAIL ;
	u8 (*rw8)(struct rtw_adapter *, u16, u8*);

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI,
				 TYPE_EFUSE_REAL_CONTENT_LEN,
				 (void *)&real_content_len);
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI,
				 TYPE_AVAILABLE_EFUSE_BYTES_TOTAL,
				 (void *)&max_available_size);

	if (start_addr > real_content_len)
		return _FAIL;

	if (true == bWrite) {
		if ((start_addr + cnts) > max_available_size)
			return _FAIL;
		rw8 = &efuse_write8;
	} else
		rw8 = &efuse_read8;

	Efuse_PowerSwitch(padapter, bWrite, true);

	/*  e-fuse one byte read / write */
	for (i = 0; i < cnts; i++) {
		if (start_addr >= real_content_len) {
			res = _FAIL;
			break;
		}

		res = rw8(padapter, start_addr++, data++);
		if (_FAIL == res) break;
	}

	Efuse_PowerSwitch(padapter, bWrite, false);

	return res;
}
/*  */
u16 efuse_GetMaxSize(struct rtw_adapter *padapter)
{
	u16	max_size;
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI,
				 TYPE_AVAILABLE_EFUSE_BYTES_TOTAL,
				 (void *)&max_size);
	return max_size;
}
/*  */
u8 efuse_GetCurrentSize(struct rtw_adapter *padapter, u16 *size)
{
	Efuse_PowerSwitch(padapter, false, true);
	*size = Efuse_GetCurrentSize(padapter, EFUSE_WIFI);
	Efuse_PowerSwitch(padapter, false, false);

	return _SUCCESS;
}
/*  */
u8 rtw_efuse_map_read(struct rtw_adapter *padapter, u16 addr, u16 cnts, u8 *data)
{
	u16	mapLen=0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI,
				 TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	Efuse_PowerSwitch(padapter, false, true);

	efuse_ReadEFuse(padapter, EFUSE_WIFI, addr, cnts, data);

	Efuse_PowerSwitch(padapter, false, false);

	return _SUCCESS;
}

u8 rtw_BT_efuse_map_read(struct rtw_adapter *padapter, u16 addr, u16 cnts, u8 *data)
{
	u16	mapLen=0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT,
				 TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	Efuse_PowerSwitch(padapter, false, true);

	efuse_ReadEFuse(padapter, EFUSE_BT, addr, cnts, data);

	Efuse_PowerSwitch(padapter, false, false);

	return _SUCCESS;
}

/*-----------------------------------------------------------------------------
 * Function:	Efuse_ReadAllMap
 *
 * Overview:	Read All Efuse content
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/11/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void
Efuse_ReadAllMap(struct rtw_adapter *pAdapter, u8 efuseType, u8 *Efuse);
void
Efuse_ReadAllMap(struct rtw_adapter *pAdapter, u8 efuseType, u8 *Efuse)
{
	u16	mapLen=0;

	Efuse_PowerSwitch(pAdapter,false, true);

	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_MAP_LEN,
				 (void *)&mapLen);

	efuse_ReadEFuse(pAdapter, efuseType, 0, mapLen, Efuse);

	Efuse_PowerSwitch(pAdapter,false, false);
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_ShadowRead1Byte
 *			efuse_ShadowRead2Byte
 *			efuse_ShadowRead4Byte
 *
 * Overview:	Read from efuse init map by one/two/four bytes !!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void
efuse_ShadowRead1Byte(
	struct rtw_adapter *	pAdapter,
	u16		Offset,
	u8		*Value)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(pAdapter);

	*Value = pEEPROM->efuse_eeprom_data[Offset];
}	/*  EFUSE_ShadowRead1Byte */

/* Read Two Bytes */
static void
efuse_ShadowRead2Byte(
	struct rtw_adapter *	pAdapter,
	u16		Offset,
	u16		*Value)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(pAdapter);

	*Value = pEEPROM->efuse_eeprom_data[Offset];
	*Value |= pEEPROM->efuse_eeprom_data[Offset+1]<<8;
}	/*  EFUSE_ShadowRead2Byte */

/* Read Four Bytes */
static void
efuse_ShadowRead4Byte(
	struct rtw_adapter *	pAdapter,
	u16		Offset,
	u32		*Value)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(pAdapter);

	*Value = pEEPROM->efuse_eeprom_data[Offset];
	*Value |= pEEPROM->efuse_eeprom_data[Offset+1]<<8;
	*Value |= pEEPROM->efuse_eeprom_data[Offset+2]<<16;
	*Value |= pEEPROM->efuse_eeprom_data[Offset+3]<<24;
}	/*  efuse_ShadowRead4Byte */

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowMapUpdate
 *
 * Overview:	Transfer current EFUSE content to shadow init and modify map.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/13/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void EFUSE_ShadowMapUpdate(struct rtw_adapter *pAdapter, u8 efuseType)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(pAdapter);
	u16	mapLen=0;

	EFUSE_GetEfuseDefinition(pAdapter, efuseType,
				 TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if (pEEPROM->bautoload_fail_flag == true)
		memset(pEEPROM->efuse_eeprom_data, 0xFF, mapLen);
	else
		Efuse_ReadAllMap(pAdapter, efuseType,
				 pEEPROM->efuse_eeprom_data);

}/*  EFUSE_ShadowMapUpdate */

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowRead
 *
 * Overview:	Read from efuse init map !!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void
EFUSE_ShadowRead(
	struct rtw_adapter *	pAdapter,
	u8		Type,
	u16		Offset,
	u32		*Value	)
{
	if (Type == 1)
		efuse_ShadowRead1Byte(pAdapter, Offset, (u8 *)Value);
	else if (Type == 2)
		efuse_ShadowRead2Byte(pAdapter, Offset, (u16 *)Value);
	else if (Type == 4)
		efuse_ShadowRead4Byte(pAdapter, Offset, (u32 *)Value);
}	/*  EFUSE_ShadowRead */
