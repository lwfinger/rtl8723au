/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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

/*
 * Public  General Config
 */

#define AUTOCONF_INCLUDED

#define RTL871X_MODULE_NAME "8723AS-VAU"
#define DRV_NAME "rtl8723as-vau"

#define PLATFORM_LINUX

/*
 * Functions Config
 */

#define CONFIG_8723AU_AP_MODE

#define CONFIG_8723AU_P2P

#define USB_INTERFERENCE_ISSUE /*  this should be checked in all usb interface */
#define RTW_NOTCH_FILTER 0 /* 0:Disable, 1:Enable,  */

/*
 * Auto Config Section
 */

#undef CONFIG_MP_IWPRIV_SUPPORT


#ifdef CONFIG_LED
	#define CONFIG_SW_LED
#endif /*  CONFIG_LED */

/*
 * HAL  Related Config
 */

#define SUPPORTED_BLOCK_IO

#define CONFIG_OUT_EP_WIFI_MODE	0

#define ENABLE_USB_DROP_INCORRECT_OUT	0

#define MP_DRIVER 0

/*
 * Outsource  Related Config
 */

#define		RTL8723AU_SUPPORT				1
#define		RTL8723AS_SUPPORT				0
#define		RTL8723AE_SUPPORT				0
#define		RTL8723A_SUPPORT				(RTL8723AU_SUPPORT|RTL8723AS_SUPPORT|RTL8723AE_SUPPORT)

#define		RTL8723_FPGA_VERIFICATION		0

#define RATE_ADAPTIVE_SUPPORT			0

/*
 * Debug Related Config
 */
#define DBG	1

#define CONFIG_DEBUG /* DBG_8723A, etc... */

#define CONFIG_PROC_DEBUG
