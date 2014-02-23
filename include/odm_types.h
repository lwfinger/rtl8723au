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
#ifndef __ODM_TYPES_H__
#define __ODM_TYPES_H__

/*  */
/*  Define Different SW team support */
/*  */

typedef enum _HAL_STATUS{
	HAL_STATUS_SUCCESS,
	HAL_STATUS_FAILURE,
}HAL_STATUS,*PHAL_STATUS;

typedef enum _RT_SPINLOCK_TYPE{
	RT_TEMP =1,
}RT_SPINLOCK_TYPE;


	#include <basic_types.h>

	#define	STA_INFO_T			struct sta_info
	#define	PSTA_INFO_T		struct sta_info *

	#define SET_TX_DESC_ANTSEL_A_88E(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+8, 24, 1, __Value)
	#define SET_TX_DESC_ANTSEL_B_88E(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+8, 25, 1, __Value)
	#define SET_TX_DESC_ANTSEL_C_88E(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+28, 29, 1, __Value)

	/* define useless flag to avoid compile warning */
	#define	USE_WORKITEM			0
	#define		FOR_BRAZIL_PRETEST	0
	#define	BT_30_SUPPORT			0
	#define   FPGA_TWO_MAC_VERIFICATION	0

#endif /*  __ODM_TYPES_H__ */
