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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
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

#define CONFIG_XMIT_ACK
#ifdef CONFIG_XMIT_ACK
	#define CONFIG_ACTIVE_KEEP_ALIVE_CHECK
#endif
#define CONFIG_80211N_HT
#define CONFIG_RECV_REORDERING_CTRL

#define CONFIG_8723AU_AP_MODE
#ifdef CONFIG_8723AU_AP_MODE
	#define CONFIG_NATIVEAP_MLME
	#ifndef CONFIG_NATIVEAP_MLME
		#define CONFIG_HOSTAPD_MLME
	#endif
#endif

#define CONFIG_8723AU_P2P
#ifdef CONFIG_8723AU_P2P
	//The CONFIG_WFD is for supporting the Wi-Fi display
	#define CONFIG_WFD

	#define CONFIG_8723AU_P2P_PS
#endif

#define CONFIG_SKB_COPY	//for amsdu

#define USB_INTERFERENCE_ISSUE // this should be checked in all usb interface
#define CONFIG_LONG_DELAY_ISSUE
#define RTW_NOTCH_FILTER 0 /* 0:Disable, 1:Enable,  */

#ifdef CONFIG_ANTENNA_DIVERSITY
#define CONFIG_SW_ANTENNA_DIVERSITY
#endif


/*
 * Auto Config Section
 */

#undef CONFIG_MP_IWPRIV_SUPPORT
#define CONFIG_LPS		1



#ifdef CONFIG_LED
	#define CONFIG_SW_LED
#endif // CONFIG_LED

#define CONFIG_IPS

#ifdef CONFIG_BR_EXT
#define CONFIG_BR_EXT_BRNAME	"br0"
#endif	// CONFIG_BR_EXT

#define CONFIG_USB_INTERRUPT_IN_PIPE

/*
 * HAL  Related Config
 */

#define RTL8192C_RX_PACKET_INCLUDE_CRC	0

#define SUPPORTED_BLOCK_IO

#define RTL8192CU_FW_DOWNLOAD_ENABLE	1

#define CONFIG_OUT_EP_WIFI_MODE	0

#define ENABLE_USB_DROP_INCORRECT_OUT	0

#define RTL8192CU_ASIC_VERIFICATION	0	// For ASIC verification.

#define RTL8192CU_ADHOC_WORKAROUND_SETTING

#define DISABLE_BB_RF	0

#define RTL8191C_FPGA_NETWORKTYPE_ADHOC 0

#define MP_DRIVER 0

/*
 * Outsource  Related Config
 */

#define		RTL8192CE_SUPPORT				0
#define		RTL8192CU_SUPPORT			0
#define		RTL8192C_SUPPORT				(RTL8192CE_SUPPORT|RTL8192CU_SUPPORT)

#define		RTL8192DE_SUPPORT				0
#define		RTL8192DU_SUPPORT			0
#define		RTL8192D_SUPPORT				(RTL8192DE_SUPPORT|RTL8192DU_SUPPORT)

#define		RTL8723AU_SUPPORT				1
#define		RTL8723AS_SUPPORT				0
#define		RTL8723AE_SUPPORT				0
#define		RTL8723A_SUPPORT				(RTL8723AU_SUPPORT|RTL8723AS_SUPPORT|RTL8723AE_SUPPORT)

#define		RTL8723_FPGA_VERIFICATION		0

#define RTL8188EE_SUPPORT				0
#define RTL8188EU_SUPPORT				0
#define RTL8188ES_SUPPORT				0
#define RTL8188E_SUPPORT				(RTL8188EE_SUPPORT|RTL8188EU_SUPPORT|RTL8188ES_SUPPORT)
#define RTL8188E_FOR_TEST_CHIP			0
#define RATE_ADAPTIVE_SUPPORT			0
#define POWER_TRAINING_ACTIVE			0

#define CONFIG_80211D

/*
 * Debug Related Config
 */
#define DBG	1

#define CONFIG_DEBUG /* DBG_8723A, etc... */

#define CONFIG_PROC_DEBUG

#define DBG_CONFIG_ERROR_DETECT
#define DBG_CONFIG_ERROR_RESET

