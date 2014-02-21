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
#define _IOCTL_LINUX_C_

#include <osdep_service.h>
#include <drv_types.h>
#include <wlan_bssdef.h>
#include <rtw_debug.h>
#include <linux/ieee80211.h>
#include <wifi.h>
#include <rtw_mlme.h>
#include <rtw_mlme_ext.h>
#include <rtw_ioctl.h>
#include <rtw_ioctl_set.h>

#include <usb_ops.h>
#include <rtw_version.h>

#include <rtl8723a_pg.h>
#include <rtl8723a_hal.h>

#define RTL_IOCTL_WPA_SUPPLICANT	SIOCIWFIRSTPRIV+30

#define SCAN_ITEM_SIZE 768
#define MAX_CUSTOM_LEN 64
#define RATE_COUNT 4

extern u8 key_2char2num(u8 hch, u8 lch);
extern u8 str_2char2num(u8 hch, u8 lch);

static int wpa_set_auth_algs(struct net_device *dev, u32 value)
{
	struct rtw_adapter *padapter = netdev_priv(dev);
	int ret = 0;

	if ((value & AUTH_ALG_SHARED_KEY)&&(value & AUTH_ALG_OPEN_SYSTEM))
	{
		DBG_8723A("wpa_set_auth_algs, AUTH_ALG_SHARED_KEY and  AUTH_ALG_OPEN_SYSTEM [value:0x%x]\n",value);
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
	}
	else if (value & AUTH_ALG_SHARED_KEY)
	{
		DBG_8723A("wpa_set_auth_algs, AUTH_ALG_SHARED_KEY  [value:0x%x]\n",value);
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;

		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeShared;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Shared;
	}
	else if(value & AUTH_ALG_OPEN_SYSTEM)
	{
		DBG_8723A("wpa_set_auth_algs, AUTH_ALG_OPEN_SYSTEM\n");
		//padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
		if(padapter->securitypriv.ndisauthtype < Ndis802_11AuthModeWPAPSK)
		{
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeOpen;
			padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Open;
		}

	}
	else if(value & AUTH_ALG_LEAP)
	{
		DBG_8723A("wpa_set_auth_algs, AUTH_ALG_LEAP\n");
	}
	else
	{
		DBG_8723A("wpa_set_auth_algs, error!\n");
		ret = -EINVAL;
	}

	return ret;

}

static int wpa_set_encryption(struct net_device *dev, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len,wep_total_len;
	NDIS_802_11_WEP	 *pwep = NULL;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
#ifdef CONFIG_8723AU_P2P
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif //CONFIG_8723AU_P2P

_func_enter_;

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	if (param_len < (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len)
	{
		ret =  -EINVAL;
		goto exit;
	}

	if (is_broadcast_ether_addr(param->sta_addr)) {
		if (param->u.crypt.idx >= WEP_KEYS)
		{
			ret = -EINVAL;
			goto exit;
		}
	} else {
			{
		ret = -EINVAL;
		goto exit;
	}
	}

	if (strcmp(param->u.crypt.alg, "WEP") == 0)
	{
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("wpa_set_encryption, crypt.alg = WEP\n"));
		DBG_8723A("wpa_set_encryption, crypt.alg = WEP\n");

		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
		padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
		padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;

		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("(1)wep_key_idx=%d\n", wep_key_idx));
		DBG_8723A("(1)wep_key_idx=%d\n", wep_key_idx);

		if (wep_key_idx > WEP_KEYS)
			return -EINVAL;

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("(2)wep_key_idx=%d\n", wep_key_idx));

		if (wep_key_len > 0)
		{
			wep_key_len = wep_key_len <= 5 ? 5 : 13;
			wep_total_len = wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
			pwep =(NDIS_802_11_WEP	 *) kzalloc(wep_total_len, GFP_KERNEL);
			if (pwep == NULL){
				RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,(" wpa_set_encryption: pwep allocate fail !!!\n"));
				goto exit;
			}

			pwep->KeyLength = wep_key_len;
			pwep->Length = wep_total_len;

			if(wep_key_len==13)
			{
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
			}
		}
		else {
			ret = -EINVAL;
			goto exit;
		}

		pwep->KeyIndex = wep_key_idx;
		pwep->KeyIndex |= 0x80000000;

		memcpy(pwep->KeyMaterial,  param->u.crypt.key, pwep->KeyLength);

		if(param->u.crypt.set_tx)
		{
			DBG_8723A("wep, set_tx=1\n");

			if(rtw_set_802_11_add_wep(padapter, pwep) == (u8)_FAIL)
			{
				ret = -EOPNOTSUPP ;
			}
		}
		else
		{
			DBG_8723A("wep, set_tx=0\n");

			//don't update "psecuritypriv->dot11PrivacyAlgrthm" and
			//"psecuritypriv->dot11PrivacyKeyIndex=keyid", but can rtw_set_key to fw/cam

			if (wep_key_idx >= WEP_KEYS) {
				ret = -EOPNOTSUPP ;
				goto exit;
			}

		      memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);
			psecuritypriv->dot11DefKeylen[wep_key_idx]=pwep->KeyLength;
			rtw_set_key(padapter, psecuritypriv, wep_key_idx, 0);
		}

		goto exit;
	}

	if(padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) // 802_1x
	{
		struct sta_info * psta,*pbcmc_sta;
		struct sta_priv * pstapriv = &padapter->stapriv;

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_MP_STATE) == true) //sta mode
		{
			psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
			if (psta == NULL) {
				//DEBUG_ERR( ("Set wpa_set_encryption: Obtain Sta_info fail \n"));
			}
			else
			{
				//Jeff: don't disable ieee8021x_blocked while clearing key
				if (strcmp(param->u.crypt.alg, "none") != 0)
					psta->ieee8021x_blocked = false;

				if((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
						(padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled))
				{
					psta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}

				if(param->u.crypt.set_tx ==1)//pairwise key
				{
					memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					if(strcmp(param->u.crypt.alg, "TKIP") == 0)//set mic key
					{
						//DEBUG_ERR(("\nset key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len));
						memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
						memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

						padapter->securitypriv.busetkipkey=false;
					}

					//DEBUG_ERR((" param->u.crypt.key_len=%d\n",param->u.crypt.key_len));
					DBG_8723A(" ~~~~set sta key:unicastkey\n");

					rtw_setstakey_cmd(padapter, (unsigned char *)psta, true);
				}
				else//group key
				{
					memcpy(padapter->securitypriv.dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key,(param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
					memcpy(padapter->securitypriv.dot118021XGrptxmickey[param->u.crypt.idx].skey,&(param->u.crypt.key[16]),8);
					memcpy(padapter->securitypriv.dot118021XGrprxmickey[param->u.crypt.idx].skey,&(param->u.crypt.key[24]),8);
                                        padapter->securitypriv.binstallGrpkey = true;
					//DEBUG_ERR((" param->u.crypt.key_len=%d\n", param->u.crypt.key_len));
					DBG_8723A(" ~~~~set sta key:groupkey\n");

					padapter->securitypriv.dot118021XGrpKeyid = param->u.crypt.idx;

					rtw_set_key(padapter,&padapter->securitypriv,param->u.crypt.idx, 1);
#ifdef CONFIG_8723AU_P2P
					if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_PROVISIONING_ING))
					{
						rtw_p2p_set_state(pwdinfo, P2P_STATE_PROVISIONING_DONE);
					}
#endif //CONFIG_8723AU_P2P

				}
			}

			pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
			if(pbcmc_sta==NULL)
			{
				//DEBUG_ERR( ("Set OID_802_11_ADD_KEY: bcmc stainfo is null \n"));
			}
			else
			{
				//Jeff: don't disable ieee8021x_blocked while clearing key
				if (strcmp(param->u.crypt.alg, "none") != 0)
					pbcmc_sta->ieee8021x_blocked = false;

				if((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
						(padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled))
				{
					pbcmc_sta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}
			}
		}
		else if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE)) //adhoc mode
		{
		}
	}

exit:

	kfree(pwep);

_func_exit_;

	return ret;
}

static int rtw_set_wpa_ie(struct rtw_adapter *padapter, char *pie, unsigned short ielen)
{
	u8 *buf=NULL, *pos=NULL;
	u32 left;
	int group_cipher = 0, pairwise_cipher = 0;
	int ret = 0;
	u8 null_addr[]= {0,0,0,0,0,0};
#ifdef CONFIG_8723AU_P2P
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif //CONFIG_8723AU_P2P

	if((ielen > MAX_WPA_IE_LEN) || (pie == NULL)){
		_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
		if(pie == NULL)
			return ret;
		else
			return -EINVAL;
	}

	if(ielen)
	{
		buf = kmalloc(ielen, GFP_KERNEL);
		if (buf == NULL){
			ret =  -ENOMEM;
			goto exit;
		}

		memcpy(buf, pie , ielen);

		//dump
		{
			int i;
			DBG_8723A("\n wpa_ie(length:%d):\n", ielen);
			for(i=0;i<ielen;i=i+8)
				DBG_8723A("0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x \n",buf[i],buf[i+1],buf[i+2],buf[i+3],buf[i+4],buf[i+5],buf[i+6],buf[i+7]);
		}

		pos = buf;
		if(ielen < RSN_HEADER_LEN){
			RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("Ie len too short %d\n", ielen));
			ret  = -1;
			goto exit;
		}

		if(rtw_parse_wpa_ie(buf, ielen, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS)
		{
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPAPSK;
			memcpy(padapter->securitypriv.supplicant_ie, &buf[0], ielen);
		}

		if(rtw_parse_wpa2_ie(buf, ielen, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS)
		{
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPA2PSK;
			memcpy(padapter->securitypriv.supplicant_ie, &buf[0], ielen);
		}

		if (group_cipher == 0)
		{
			group_cipher = WPA_CIPHER_NONE;
		}
		if (pairwise_cipher == 0)
		{
			pairwise_cipher = WPA_CIPHER_NONE;
		}

		switch(group_cipher)
		{
			case WPA_CIPHER_NONE:
				padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
				padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
				break;
			case WPA_CIPHER_WEP40:
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
			case WPA_CIPHER_TKIP:
				padapter->securitypriv.dot118021XGrpPrivacy=_TKIP_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case WPA_CIPHER_CCMP:
				padapter->securitypriv.dot118021XGrpPrivacy=_AES_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
			case WPA_CIPHER_WEP104:
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
		}

		switch(pairwise_cipher)
		{
			case WPA_CIPHER_NONE:
				padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
				padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
				break;
			case WPA_CIPHER_WEP40:
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
			case WPA_CIPHER_TKIP:
				padapter->securitypriv.dot11PrivacyAlgrthm=_TKIP_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case WPA_CIPHER_CCMP:
				padapter->securitypriv.dot11PrivacyAlgrthm=_AES_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
			case WPA_CIPHER_WEP104:
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
		}

		_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
		{//set wps_ie
			u16 cnt = 0;
			u8 eid, wps_oui[4]={0x0,0x50,0xf2,0x04};

			while( cnt < ielen )
			{
				eid = buf[cnt];

				if ((eid ==_VENDOR_SPECIFIC_IE_) &&
				    (!memcmp(&buf[cnt+2], wps_oui, 4))) {
					DBG_8723A("SET WPS_IE\n");

					padapter->securitypriv.wps_ie_len = ( (buf[cnt+1]+2) < (MAX_WPA_IE_LEN<<2)) ? (buf[cnt+1]+2):(MAX_WPA_IE_LEN<<2);

					memcpy(padapter->securitypriv.wps_ie, &buf[cnt], padapter->securitypriv.wps_ie_len);

					set_fwstate(&padapter->mlmepriv, WIFI_UNDER_WPS);

#ifdef CONFIG_8723AU_P2P
					if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_GONEGO_OK))
					{
						rtw_p2p_set_state(pwdinfo, P2P_STATE_PROVISIONING_ING);
					}
#endif //CONFIG_8723AU_P2P
					cnt += buf[cnt+1]+2;

					break;
				} else {
					cnt += buf[cnt+1]+2; //goto next
				}
			}
		}
	}

	//TKIP and AES disallow multicast packets until installing group key
        if(padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_
                || padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_WTMIC_
                || padapter->securitypriv.dot11PrivacyAlgrthm == _AES_)
                //WPS open need to enable multicast
                //|| check_fwstate(&padapter->mlmepriv, WIFI_UNDER_WPS) == true)
                rtw_hal_set_hwreg(padapter, HW_VAR_OFF_RCR_AM, null_addr);

	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
		 ("rtw_set_wpa_ie: pairwise_cipher=0x%08x padapter->securitypriv.ndisencryptstatus=%d padapter->securitypriv.ndisauthtype=%d\n",
		  pairwise_cipher, padapter->securitypriv.ndisencryptstatus, padapter->securitypriv.ndisauthtype));

exit:

	kfree(buf);

	return ret;
}

static int dummy(struct net_device *dev, struct iw_request_info *a,
		 union iwreq_data *wrqu, char *b)
{
	//struct rtw_adapter *padapter = netdev_priv(dev);
	//struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	//DBG_8723A("cmd_code=%x, fwstate=0x%x\n", a->cmd, get_fwstate(pmlmepriv));

	return -1;

}

#ifdef CONFIG_8723AU_P2P
static int rtw_wext_p2p_enable(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	enum P2P_ROLE init_role = P2P_ROLE_DISABLE;

	if(*extra == '0' )
		init_role = P2P_ROLE_DISABLE;
	else if(*extra == '1')
		init_role = P2P_ROLE_DEVICE;
	else if(*extra == '2')
		init_role = P2P_ROLE_CLIENT;
	else if(*extra == '3')
		init_role = P2P_ROLE_GO;

	if(_FAIL == rtw_p2p_enable(padapter, init_role))
	{
		ret = -EFAULT;
		goto exit;
	}

	//set channel/bandwidth
	if(init_role != P2P_ROLE_DISABLE) {
		u8 channel, ch_offset;
		u16 bwmode;

		if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_LISTEN)) {
			//	Stay at the listen state and wait for discovery.
			channel = pwdinfo->listen_channel;
			pwdinfo->operating_channel = pwdinfo->listen_channel;
			ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
			bwmode = HT_CHANNEL_WIDTH_20;
		} else {
			pwdinfo->operating_channel = pmlmeext->cur_channel;

			channel = pwdinfo->operating_channel;
			ch_offset = pmlmeext->cur_ch_offset;
			bwmode = pmlmeext->cur_bwmode;
		}

		set_channel_bwmode(padapter, channel, ch_offset, bwmode);
	}

exit:
	return ret;

}

static int rtw_p2p_set_go_nego_ssid(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	DBG_8723A( "[%s] ssid = %s, len = %zu\n", __func__, extra, strlen( extra ) );
	memcpy(pwdinfo->nego_ssid, extra, strlen( extra ) );
	pwdinfo->nego_ssidlen = strlen( extra );

	return ret;

}


static int rtw_p2p_set_intent(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	u8 intent = pwdinfo->intent;

	switch (wrqu->data.length) {
		case 1:
		{
			intent = extra[ 0 ] - '0';
			break;
		}
		case 2:
		{
			intent = str_2char2num( extra[ 0 ], extra[ 1 ]);
			break;
		}
	}

	if ( intent <= 15 )
	{
		pwdinfo->intent= intent;
	}
	else
	{
		ret = -1;
	}

	DBG_8723A( "[%s] intent = %d\n", __func__, intent);

	return ret;

}

static int rtw_p2p_set_listen_ch(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	u8	listen_ch = pwdinfo->listen_channel;	//	Listen channel number

	switch( wrqu->data.length )
	{
		case 1:
		{
			listen_ch = extra[ 0 ] - '0';
			break;
		}
		case 2:
		{
			listen_ch = str_2char2num( extra[ 0 ], extra[ 1 ]);
			break;
		}
	}

	if ( ( listen_ch == 1 ) || ( listen_ch == 6 ) || ( listen_ch == 11 ) )
	{
		pwdinfo->listen_channel = listen_ch;
		set_channel_bwmode(padapter, pwdinfo->listen_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);
	}
	else
	{
		ret = -1;
	}

	DBG_8723A( "[%s] listen_ch = %d\n", __func__, pwdinfo->listen_channel );

	return ret;

}

static int rtw_p2p_set_op_ch(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
//	Commented by Albert 20110524
//	This function is used to set the operating channel if the driver will become the group owner

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	u8	op_ch = pwdinfo->operating_channel;	//	Operating channel number

	switch( wrqu->data.length )
	{
		case 1:
		{
			op_ch = extra[ 0 ] - '0';
			break;
		}
		case 2:
		{
			op_ch = str_2char2num( extra[ 0 ], extra[ 1 ]);
			break;
		}
	}

	if ( op_ch > 0 )
	{
		pwdinfo->operating_channel = op_ch;
	}
	else
	{
		ret = -1;
	}

	DBG_8723A( "[%s] op_ch = %d\n", __func__, pwdinfo->operating_channel );

	return ret;

}


static int rtw_p2p_profilefound(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	//	Comment by Albert 2010/10/13
	//	Input data format:
	//	Ex:  0
	//	Ex:  1XX:XX:XX:XX:XX:XXYYSSID
	//	0 => Reflush the profile record list.
	//	1 => Add the profile list
	//	XX:XX:XX:XX:XX:XX => peer's MAC Address ( ex: 00:E0:4C:00:00:01 )
	//	YY => SSID Length
	//	SSID => SSID for persistence group

	DBG_8723A( "[%s] In value = %s, len = %d \n", __func__, extra, wrqu->data.length -1);


	//	The upper application should pass the SSID to driver by using this rtw_p2p_profilefound function.
	if(!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		if ( extra[ 0 ] == '0' )
		{
			//	Remove all the profile information of wifidirect_info structure.
			memset( &pwdinfo->profileinfo[ 0 ], 0x00, sizeof( struct profile_info ) * P2P_MAX_PERSISTENT_GROUP_NUM );
			pwdinfo->profileindex = 0;
		}
		else
		{
			if ( pwdinfo->profileindex >= P2P_MAX_PERSISTENT_GROUP_NUM )
		{
				ret = -1;
		}
		else
		{
				int jj, kk;

				//	Add this profile information into pwdinfo->profileinfo
				//	Ex:  1XX:XX:XX:XX:XX:XXYYSSID
				for( jj = 0, kk = 1; jj < ETH_ALEN; jj++, kk += 3 )
				{
					pwdinfo->profileinfo[ pwdinfo->profileindex ].peermac[ jj ] = key_2char2num(extra[ kk ], extra[ kk+ 1 ]);
				}

				pwdinfo->profileinfo[ pwdinfo->profileindex ].ssidlen = ( extra[18] - '0' ) * 10 + ( extra[ 19 ] - '0' );
				memcpy(pwdinfo->profileinfo[ pwdinfo->profileindex ].ssid, &extra[ 20 ], pwdinfo->profileinfo[ pwdinfo->profileindex ].ssidlen );
				pwdinfo->profileindex++;
			}
		}
	}

	return ret;

}

static int rtw_p2p_setDN(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);


	DBG_8723A( "[%s] %s %d\n", __func__, extra, wrqu->data.length -1  );
	memset( pwdinfo->device_name, 0x00, WPS_MAX_DEVICE_NAME_LEN );
	memcpy(pwdinfo->device_name, extra, wrqu->data.length - 1 );
	pwdinfo->device_name_len = wrqu->data.length - 1;

	return ret;

}


static int rtw_p2p_get_status(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	if ( padapter->bShowGetP2PState )
	{
		DBG_8723A( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __func__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
				pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
				pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);
	}

	//	Commented by Albert 2010/10/12
	//	Because of the output size limitation, I had removed the "Role" information.
	//	About the "Role" information, we will use the new private IOCTL to get the "Role" information.
	sprintf( extra, "\n\nStatus=%.2d\n", rtw_p2p_state(pwdinfo) );
	wrqu->data.length = strlen( extra );

	return ret;

}

//	Commented by Albert 20110520
//	This function will return the config method description
//	This config method description will show us which config method the remote P2P device is intented to use
//	by sending the provisioning discovery request frame.

static int rtw_p2p_get_req_cm(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n\nCM=%s\n", pwdinfo->rx_prov_disc_info.strconfig_method_desc_of_prov_disc_req );
	wrqu->data.length = strlen( extra );
	return ret;

}


static int rtw_p2p_get_role(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_8723A( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __func__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
			pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
			pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);

	sprintf( extra, "\n\nRole=%.2d\n", rtw_p2p_role(pwdinfo) );
	wrqu->data.length = strlen( extra );
	return ret;

}


static int rtw_p2p_get_peer_ifaddr(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_8723A( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __func__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
			pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
			pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);

	sprintf( extra, "\nMAC %.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
			pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
			pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_peer_devaddr(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	DBG_8723A( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __func__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 0 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 1 ],
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 2 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 3 ],
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 4 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 5 ]);
	sprintf( extra, "\n%.2X%.2X%.2X%.2X%.2X%.2X",
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 0 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 1 ],
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 2 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 3 ],
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 4 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 5 ]);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_peer_devaddr_by_invitation(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	DBG_8723A( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __func__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
			pwdinfo->p2p_peer_device_addr[ 0 ], pwdinfo->p2p_peer_device_addr[ 1 ],
			pwdinfo->p2p_peer_device_addr[ 2 ], pwdinfo->p2p_peer_device_addr[ 3 ],
			pwdinfo->p2p_peer_device_addr[ 4 ], pwdinfo->p2p_peer_device_addr[ 5 ]);
	sprintf( extra, "\nMAC %.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
			pwdinfo->p2p_peer_device_addr[ 0 ], pwdinfo->p2p_peer_device_addr[ 1 ],
			pwdinfo->p2p_peer_device_addr[ 2 ], pwdinfo->p2p_peer_device_addr[ 3 ],
			pwdinfo->p2p_peer_device_addr[ 4 ], pwdinfo->p2p_peer_device_addr[ 5 ]);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_groupid(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n%.2X:%.2X:%.2X:%.2X:%.2X:%.2X %s",
			pwdinfo->groupid_info.go_device_addr[ 0 ], pwdinfo->groupid_info.go_device_addr[ 1 ],
			pwdinfo->groupid_info.go_device_addr[ 2 ], pwdinfo->groupid_info.go_device_addr[ 3 ],
			pwdinfo->groupid_info.go_device_addr[ 4 ], pwdinfo->groupid_info.go_device_addr[ 5 ],
			pwdinfo->groupid_info.ssid);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_op_ch(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_8723A( "[%s] Op_ch = %02x\n", __func__, pwdinfo->operating_channel);

	sprintf( extra, "\n\nOp_ch=%.2d\n", pwdinfo->operating_channel );
	wrqu->data.length = strlen( extra );
	return ret;

}

inline static void macstr2num(u8 *dst, u8 *src)
{
	int	jj, kk;
	for (jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3)
	{
		dst[jj] = key_2char2num(src[kk], src[kk + 1]);
	}
}

static int rtw_p2p_get_wps_configmethod(struct net_device *dev,
										struct iw_request_info *info,
										union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead, *ptmp;
	_queue *queue = &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork;
	u8 blnMatch = 0;
	u16	attr_content = 0;
	uint attr_contentlen = 0;
	u8	attr_content_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	//	Commented by Albert 20110727
	//	The input data is the MAC address which the application wants to know its WPS config method.
	//	After knowing its WPS config method, the application can decide the config method for provisioning discovery.
	//	Format: iwpriv wlanx p2p_get_wpsCM 00:E0:4C:00:00:05

	DBG_8723A("[%s] data = %s\n", __func__, subcmd);

	macstr2num(peerMAC, subcmd);

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);

	list_for_each_safe(plist, ptmp, phead) {
		pnetwork = container_of(plist, struct wlan_network, list);
		if (!memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN))
		{
			u8 *wpsie;
			uint	wpsie_len = 0;

			//	The mac address is matched.

			if ((wpsie = rtw_get_wps_ie(&pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &wpsie_len)))
			{
				rtw_get_wps_attr_content(wpsie, wpsie_len, WPS_ATTR_CONF_METHOD, (u8 *)&attr_content, &attr_contentlen);
				if (attr_contentlen)
				{
					attr_content = be16_to_cpu(attr_content);
					sprintf(attr_content_str, "\n\nM=%.4d", attr_content);
					blnMatch = 1;
				}
			}

			break;
		}
	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (!blnMatch)
	{
		sprintf(attr_content_str, "\n\nM=0000");
	}

	wrqu->data.length = strlen(attr_content_str);
	memcpy(extra, attr_content_str, wrqu->data.length);

	return ret;

}

#ifdef CONFIG_8723AU_P2P
static int rtw_p2p_get_peer_wfd_port(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	DBG_8723A( "[%s] p2p_state = %d\n", __func__, rtw_p2p_state(pwdinfo) );

	sprintf( extra, "\n\nPort=%d\n", pwdinfo->wfd_info->peer_rtsp_ctrlport );
	DBG_8723A( "[%s] remote port = %d\n", __func__, pwdinfo->wfd_info->peer_rtsp_ctrlport );

	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_peer_wfd_preferred_connection(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n\nwfd_pc=%d\n", pwdinfo->wfd_info->wfd_pc );
	DBG_8723A( "[%s] wfd_pc = %d\n", __func__, pwdinfo->wfd_info->wfd_pc );

	wrqu->data.length = strlen( extra );
	pwdinfo->wfd_info->wfd_pc = false;	//	Reset the WFD preferred connection to P2P
	return ret;

}

static int rtw_p2p_get_peer_wfd_session_available(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n\nwfd_sa=%d\n", pwdinfo->wfd_info->peer_session_avail );
	DBG_8723A( "[%s] wfd_sa = %d\n", __func__, pwdinfo->wfd_info->peer_session_avail );

	wrqu->data.length = strlen( extra );
	pwdinfo->wfd_info->peer_session_avail = true;	//	Reset the WFD session available
	return ret;

}

#endif // CONFIG_8723AU_P2P

static int rtw_p2p_get_go_device_address(struct net_device *dev,
										 struct iw_request_info *info,
										 union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead, *ptmp;
	_queue *queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork;
	u8 blnMatch = 0;
	u8 *p2pie;
	uint p2pielen = 0, attr_contentlen = 0;
	u8 attr_content[100] = { 0x00 };
	u8 go_devadd_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	//	Commented by Albert 20121209
	//	The input data is the GO's interface address which the application wants to know its device address.
	//	Format: iwpriv wlanx p2p_get2 go_devadd=00:E0:4C:00:00:05

	DBG_8723A("[%s] data = %s\n", __func__, subcmd);

	macstr2num(peerMAC, subcmd);

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);

	list_for_each_safe(plist, ptmp, phead) {
		pnetwork = container_of(plist, struct wlan_network, list);
		if (!memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN))
		{
			//	Commented by Albert 2011/05/18
			//	Match the device address located in the P2P IE
			//	This is for the case that the P2P device address is not the same as the P2P interface address.

			if ((p2pie = rtw_get_p2p_ie(&pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &p2pielen)))
			{
				while (p2pie)
				{
					//	The P2P Device ID attribute is included in the Beacon frame.
					//	The P2P Device Info attribute is included in the probe response frame.

					memset(attr_content, 0x00, 100);
					if (rtw_get_p2p_attr_content(p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen))
					{
						//	Handle the P2P Device ID attribute of Beacon first
						blnMatch = 1;
						break;

					} else if (rtw_get_p2p_attr_content(p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen))
					{
						//	Handle the P2P Device Info attribute of probe response
						blnMatch = 1;
						break;
					}

					//Get the next P2P IE
					p2pie = rtw_get_p2p_ie(p2pie + p2pielen, pnetwork->network.IELength - 12 - (p2pie - &pnetwork->network.IEs[12] + p2pielen), NULL, &p2pielen);
				}
			}
		}
	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (!blnMatch)
	{
		sprintf(go_devadd_str, "\n\ndev_add=NULL");
	} else
	{
		sprintf(go_devadd_str, "\n\ndev_add=%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
				attr_content[0], attr_content[1], attr_content[2], attr_content[3], attr_content[4], attr_content[5]);
	}

	wrqu->data.length = strlen(go_devadd_str);
	memcpy(extra, go_devadd_str, wrqu->data.length);

	return ret;

}

static int rtw_p2p_get_device_type(struct net_device *dev,
								   struct iw_request_info *info,
								   union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead, *ptmp;
	_queue *queue = &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork;
	u8 blnMatch = 0;
	u8 dev_type[8] = { 0x00 };
	uint dev_type_len = 0;
	u8 dev_type_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };    // +9 is for the str "dev_type=", we have to clear it at wrqu->data.pointer

	//	Commented by Albert 20121209
	//	The input data is the MAC address which the application wants to know its device type.
	//	Such user interface could know the device type.
	//	Format: iwpriv wlanx p2p_get2 dev_type=00:E0:4C:00:00:05

	DBG_8723A("[%s] data = %s\n", __func__, subcmd);

	macstr2num(peerMAC, subcmd);

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);

	list_for_each_safe(plist, ptmp, phead) {
		pnetwork = container_of(plist, struct wlan_network, list);
		if (!memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN))
		{
			u8 *wpsie;
			uint	wpsie_len = 0;

			//	The mac address is matched.

			if ((wpsie = rtw_get_wps_ie(&pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &wpsie_len)))
			{
				rtw_get_wps_attr_content(wpsie, wpsie_len, WPS_ATTR_PRIMARY_DEV_TYPE, dev_type, &dev_type_len);
				if (dev_type_len)
				{
					u16	type = 0;

					memcpy(&type, dev_type, 2);
					type = be16_to_cpu(type);
					sprintf(dev_type_str, "\n\nN=%.2d", type);
					blnMatch = 1;
				}
			}
			break;
		}
	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (!blnMatch)
	{
		sprintf(dev_type_str, "\n\nN=00");
	}

	wrqu->data.length = strlen(dev_type_str);
	memcpy(extra, dev_type_str, wrqu->data.length);

	return ret;

}

static int rtw_p2p_get_device_name(struct net_device *dev,
								   struct iw_request_info *info,
								   union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead, *ptmp;
	_queue *queue = &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork;
	u8 blnMatch = 0;
	u8 dev_name[WPS_MAX_DEVICE_NAME_LEN] = { 0x00 };
	uint dev_len = 0;
	u8 dev_name_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	//	Commented by Albert 20121225
	//	The input data is the MAC address which the application wants to know its device name.
	//	Such user interface could show peer device's device name instead of ssid.
	//	Format: iwpriv wlanx p2p_get2 devN=00:E0:4C:00:00:05

	DBG_8723A("[%s] data = %s\n", __func__, subcmd);

	macstr2num(peerMAC, subcmd);

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);

	list_for_each_safe(plist, ptmp, phead) {
		pnetwork = container_of(plist, struct wlan_network, list);
		if (!memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN))
		{
			u8 *wpsie;
			uint	wpsie_len = 0;

			//	The mac address is matched.

			if ((wpsie = rtw_get_wps_ie(&pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &wpsie_len)))
			{
				rtw_get_wps_attr_content(wpsie, wpsie_len, WPS_ATTR_DEVICE_NAME, dev_name, &dev_len);
				if (dev_len)
				{
					sprintf(dev_name_str, "\n\nN=%s", dev_name);
					blnMatch = 1;
				}
			}
			break;
		}
	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (!blnMatch)
	{
		sprintf(dev_name_str, "\n\nN=0000");
	}

	wrqu->data.length = strlen(dev_name_str);
	memcpy(extra, dev_name_str, wrqu->data.length);

	return ret;

}

static int rtw_p2p_get_invitation_procedure(struct net_device *dev,
											struct iw_request_info *info,
											union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead, *ptmp;
	_queue *queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork;
	u8 blnMatch = 0;
	u8 *p2pie;
	uint p2pielen = 0, attr_contentlen = 0;
	u8 attr_content[2] = { 0x00 };
	u8 inv_proc_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	//	Commented by Ouden 20121226
	//	The application wants to know P2P initation procedure is support or not.
	//	Format: iwpriv wlanx p2p_get2 InvProc=00:E0:4C:00:00:05

	DBG_8723A("[%s] data = %s\n", __func__, subcmd);

	macstr2num(peerMAC, subcmd);

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);

	list_for_each_safe(plist, ptmp, phead) {
		pnetwork = container_of(plist, struct wlan_network, list);
		if (!memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN)) {
			//	Commented by Albert 20121226
			//	Match the device address located in the P2P IE
			//	This is for the case that the P2P device address is not the same as the P2P interface address.

			if ((p2pie = rtw_get_p2p_ie(&pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &p2pielen))) {
				while (p2pie) {
					//memset( attr_content, 0x00, 2);
					if (rtw_get_p2p_attr_content(p2pie, p2pielen, P2P_ATTR_CAPABILITY, attr_content, &attr_contentlen)) {
						//	Handle the P2P capability attribute
						blnMatch = 1;
						break;
					}

					//Get the next P2P IE
					p2pie = rtw_get_p2p_ie(p2pie + p2pielen, pnetwork->network.IELength - 12 - (p2pie - &pnetwork->network.IEs[12] + p2pielen), NULL, &p2pielen);
				}
			}
		}
	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (!blnMatch) {
		sprintf(inv_proc_str, "\nIP=-1");
	} else {
		if (attr_content[0] & 0x20)
			sprintf(inv_proc_str, "\nIP=1");
		else
			sprintf(inv_proc_str, "\nIP=0");
	}

	wrqu->data.length = strlen(inv_proc_str);
	memcpy(extra, inv_proc_str, wrqu->data.length);

	return ret;
}

static int rtw_p2p_connect(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter				*padapter = netdev_priv(dev);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int					jj,kk;
	u8					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead;
	_queue				*queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork;
	uint					uintPeerChannel = 0;

	//	Commented by Albert 20110304
	//	The input data contains two informations.
	//	1. First information is the MAC address which wants to formate with
	//	2. Second information is the WPS PINCode or "pbc" string for push button method
	//	Format: 00:E0:4C:00:00:05
	//	Format: 00:E0:4C:00:00:05

	DBG_8723A( "[%s] data = %s\n", __func__, extra );

	if ( pwdinfo->p2p_state == P2P_STATE_NONE )
	{
		DBG_8723A( "[%s] WiFi Direct is disable!\n", __func__ );
		return ret;
	}

	if ( pwdinfo->ui_got_wps_info == P2P_NO_WPSINFO )
	{
		return -1;
	}

	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 )
	{
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);

	list_for_each(plist, phead) {
		pnetwork = container_of(plist, struct wlan_network, list);
		if (!memcmp( pnetwork->network.MacAddress, peerMAC, ETH_ALEN))
		{
			uintPeerChannel = pnetwork->network.Configuration.DSConfig;
			break;
		}
	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if ( uintPeerChannel )
	{
		memset( &pwdinfo->nego_req_info, 0x00, sizeof( struct tx_nego_req_info ) );
		memset( &pwdinfo->groupid_info, 0x00, sizeof( struct group_id_info ) );

		pwdinfo->nego_req_info.peer_channel_num[ 0 ] = uintPeerChannel;
		memcpy(pwdinfo->nego_req_info.peerDevAddr, pnetwork->network.MacAddress, ETH_ALEN );
		pwdinfo->nego_req_info.benable = true;

		del_timer_sync(&pwdinfo->restore_p2p_state_timer);
		if ( rtw_p2p_state(pwdinfo) != P2P_STATE_GONEGO_OK )
		{
			//	Restore to the listen state if the current p2p state is not nego OK
			rtw_p2p_set_state(pwdinfo, P2P_STATE_LISTEN );
		}

		rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));
		rtw_p2p_set_state(pwdinfo, P2P_STATE_GONEGO_ING);

		DBG_8723A( "[%s] Start PreTx Procedure!\n", __func__ );
		mod_timer(&pwdinfo->pre_tx_scan_timer,
			  jiffies + msecs_to_jiffies(P2P_TX_PRESCAN_TIMEOUT));
		mod_timer(&pwdinfo->restore_p2p_state_timer,
			  jiffies + msecs_to_jiffies(P2P_GO_NEGO_TIMEOUT));
	} else {
		DBG_8723A( "[%s] Not Found in Scanning Queue~\n", __func__ );
		ret = -1;
	}
exit:
	return ret;
}

static int rtw_p2p_invite_req(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter					*padapter = netdev_priv(dev);
	struct iw_point				*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	int						jj,kk;
	u8						peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv			*pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead, *ptmp;
	_queue					*queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork;
	uint						uintPeerChannel = 0;
	u8						attr_content[50] = { 0x00 }, _status = 0;
	u8						*p2pie;
	uint						p2pielen = 0, attr_contentlen = 0;
	struct tx_invite_req_info*	pinvite_req_info = &pwdinfo->invitereq_info;

#ifdef CONFIG_8723AU_P2P
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;
#endif // CONFIG_8723AU_P2P

	//	Commented by Albert 20120321
	//	The input data contains two informations.
	//	1. First information is the P2P device address which you want to send to.
	//	2. Second information is the group id which combines with GO's mac address, space and GO's ssid.
	//	Command line sample: iwpriv wlan0 p2p_set invite="00:11:22:33:44:55 00:E0:4C:00:00:05 DIRECT-xy"
	//	Format: 00:11:22:33:44:55 00:E0:4C:00:00:05 DIRECT-xy

	DBG_8723A( "[%s] data = %s\n", __func__, extra );

	if ( wrqu->data.length <=  37 )
	{
		DBG_8723A( "[%s] Wrong format!\n", __func__ );
		return ret;
	}

	if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		DBG_8723A( "[%s] WiFi Direct is disable!\n", __func__ );
		return ret;
	}
	else
	{
		//	Reset the content of struct tx_invite_req_info
		pinvite_req_info->benable = false;
		memset( pinvite_req_info->go_bssid, 0x00, ETH_ALEN );
		memset( pinvite_req_info->go_ssid, 0x00, WLAN_SSID_MAXLEN );
		pinvite_req_info->ssidlen = 0x00;
		pinvite_req_info->operating_ch = pwdinfo->operating_channel;
		memset( pinvite_req_info->peer_macaddr, 0x00, ETH_ALEN );
		pinvite_req_info->token = 3;
	}

	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 )
	{
		pinvite_req_info->peer_macaddr[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);

	list_for_each_safe(plist, ptmp, phead) {
		pnetwork = container_of(plist, struct wlan_network, list);

		//	Commented by Albert 2011/05/18
		//	Match the device address located in the P2P IE
		//	This is for the case that the P2P device address is not the same as the P2P interface address.

		if ( (p2pie=rtw_get_p2p_ie( &pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &p2pielen)) )
		{
			//	The P2P Device ID attribute is included in the Beacon frame.
			//	The P2P Device Info attribute is included in the probe response frame.

			if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen) )
			{
				//	Handle the P2P Device ID attribute of Beacon first
				if (!memcmp(attr_content, pinvite_req_info->peer_macaddr, ETH_ALEN))
				{
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			}
			else if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen) )
			{
				//	Handle the P2P Device Info attribute of probe response
				if (!memcmp(attr_content, pinvite_req_info->peer_macaddr, ETH_ALEN))
				{
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			}

		}
	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

#ifdef CONFIG_8723AU_P2P
	if ( uintPeerChannel )
	{
		u8	wfd_ie[ 128 ] = { 0x00 };
		uint	wfd_ielen = 0;

		if ( rtw_get_wfd_ie( &pnetwork->network.IEs[12], pnetwork->network.IELength - 12,  wfd_ie, &wfd_ielen ) )
		{
			u8	wfd_devinfo[ 6 ] = { 0x00 };
			uint	wfd_devlen = 6;

			DBG_8723A( "[%s] Found WFD IE!\n", __func__ );
			if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen ) )
			{
				u16	wfd_devinfo_field = 0;

				//	Commented by Albert 20120319
				//	The first two bytes are the WFD device information field of WFD device information subelement.
				//	In big endian format.
				wfd_devinfo_field = RTW_GET_BE16(wfd_devinfo);
				if ( wfd_devinfo_field & WFD_DEVINFO_SESSION_AVAIL )
				{
					pwfd_info->peer_session_avail = true;
				}
				else
				{
					pwfd_info->peer_session_avail = false;
				}
			}
		}

		if ( false == pwfd_info->peer_session_avail )
		{
			DBG_8723A( "[%s] WFD Session not avaiable!\n", __func__ );
			goto exit;
		}
	}
#endif // CONFIG_8723AU_P2P

	if ( uintPeerChannel ) {
		//	Store the GO's bssid
		for( jj = 0, kk = 18; jj < ETH_ALEN; jj++, kk += 3 )
		{
			pinvite_req_info->go_bssid[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
		}

		//	Store the GO's ssid
		pinvite_req_info->ssidlen = wrqu->data.length - 36;
		memcpy(pinvite_req_info->go_ssid, &extra[ 36 ], (u32) pinvite_req_info->ssidlen );
		pinvite_req_info->benable = true;
		pinvite_req_info->peer_ch = uintPeerChannel;

		rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));
		rtw_p2p_set_state(pwdinfo, P2P_STATE_TX_INVITE_REQ);

		set_channel_bwmode(padapter, uintPeerChannel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);

		mod_timer(&pwdinfo->pre_tx_scan_timer,
			  jiffies + msecs_to_jiffies(P2P_TX_PRESCAN_TIMEOUT));

		mod_timer(&pwdinfo->restore_p2p_state_timer,
			  jiffies + msecs_to_jiffies(P2P_INVITE_TIMEOUT));
	}
	else
	{
		DBG_8723A( "[%s] NOT Found in the Scanning Queue!\n", __func__ );
	}
exit:

	return ret;

}

static int rtw_p2p_set_persistent(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter					*padapter = netdev_priv(dev);
	struct iw_point				*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	int						jj,kk;
	u8						peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv			*pmlmepriv = &padapter->mlmepriv;
	struct list_head						*plist, *phead;
	_queue					*queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork;
	uint						uintPeerChannel = 0;
	u8						attr_content[50] = { 0x00 }, _status = 0;
	u8						*p2pie;
	uint						p2pielen = 0, attr_contentlen = 0;
	struct tx_invite_req_info*	pinvite_req_info = &pwdinfo->invitereq_info;

#ifdef CONFIG_8723AU_P2P
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;
#endif // CONFIG_8723AU_P2P

	//	Commented by Albert 20120328
	//	The input data is 0 or 1
	//	0: disable persistent group functionality
	//	1: enable persistent group founctionality

	DBG_8723A( "[%s] data = %s\n", __func__, extra );

	if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		DBG_8723A( "[%s] WiFi Direct is disable!\n", __func__ );
		return ret;
	}
	else
	{
		if ( extra[ 0 ] == '0' )	//	Disable the persistent group function.
		{
			pwdinfo->persistent_supported = false;
		}
		else if ( extra[ 0 ] == '1' )	//	Enable the persistent group function.
		{
			pwdinfo->persistent_supported = true;
		}
		else
		{
			pwdinfo->persistent_supported = false;
		}
	}
	printk( "[%s] persistent_supported = %d\n", __func__, pwdinfo->persistent_supported );

exit:

	return ret;

}

#ifdef CONFIG_8723AU_P2P
static int rtw_p2p_set_pc(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter				*padapter = netdev_priv(dev);
	struct iw_point			*pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int					jj,kk;
	u8					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead, *ptmp;
	_queue				*queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork;
	u8					attr_content[50] = { 0x00 }, _status = 0;
	u8 *p2pie;
	uint					p2pielen = 0, attr_contentlen = 0;
	uint					uintPeerChannel = 0;
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;

	//	Commented by Albert 20120512
	//	1. Input information is the MAC address which wants to know the Preferred Connection bit (PC bit)
	//	Format: 00:E0:4C:00:00:05

	DBG_8723A( "[%s] data = %s\n", __func__, extra );

	if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		DBG_8723A( "[%s] WiFi Direct is disable!\n", __func__ );
		return ret;
	}

	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 )
	{
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);

	list_for_each_safe(plist, ptmp, phead) {
		pnetwork = container_of(plist, struct wlan_network, list);

		//	Commented by Albert 2011/05/18
		//	Match the device address located in the P2P IE
		//	This is for the case that the P2P device address is not the same as the P2P interface address.

		if ( (p2pie=rtw_get_p2p_ie( &pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &p2pielen)) )
		{
			//	The P2P Device ID attribute is included in the Beacon frame.
			//	The P2P Device Info attribute is included in the probe response frame.
			printk( "[%s] Got P2P IE\n", __func__ );
			if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen) )
			{
				//	Handle the P2P Device ID attribute of Beacon first
				printk( "[%s] P2P_ATTR_DEVICE_ID \n", __func__ );
				if (!memcmp(attr_content, peerMAC, ETH_ALEN))
				{
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			}
			else if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen) )
			{
				//	Handle the P2P Device Info attribute of probe response
				printk( "[%s] P2P_ATTR_DEVICE_INFO \n", __func__ );
				if (!memcmp( attr_content, peerMAC, ETH_ALEN))
				{
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			}

		}
	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));
	printk( "[%s] channel = %d\n", __func__, uintPeerChannel );

	if ( uintPeerChannel )
	{
		u8	wfd_ie[ 128 ] = { 0x00 };
		uint	wfd_ielen = 0;

		if ( rtw_get_wfd_ie( &pnetwork->network.IEs[12], pnetwork->network.IELength - 12,  wfd_ie, &wfd_ielen ) )
		{
			u8	wfd_devinfo[ 6 ] = { 0x00 };
			uint	wfd_devlen = 6;

			DBG_8723A( "[%s] Found WFD IE!\n", __func__ );
			if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen ) )
			{
				u16	wfd_devinfo_field = 0;

				//	Commented by Albert 20120319
				//	The first two bytes are the WFD device information field of WFD device information subelement.
				//	In big endian format.
				wfd_devinfo_field = RTW_GET_BE16(wfd_devinfo);
				if ( wfd_devinfo_field & WFD_DEVINFO_PC_TDLS )
				{
					pwfd_info->wfd_pc = true;
				}
				else
				{
					pwfd_info->wfd_pc = false;
				}
			}
		}
	}
	else
	{
		DBG_8723A( "[%s] NOT Found in the Scanning Queue!\n", __func__ );
	}

exit:

	return ret;

}

static int rtw_p2p_set_wfd_device_type(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter					*padapter = netdev_priv(dev);
	struct iw_point				*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;

	//	Commented by Albert 20120328
	//	The input data is 0 or 1
	//	0: specify to Miracast source device
	//	1 or others: specify to Miracast sink device (display device)

	DBG_8723A( "[%s] data = %s\n", __func__, extra );

	if ( extra[ 0 ] == '0' )	//	Set to Miracast source device.
	{
		pwfd_info->wfd_device_type = WFD_DEVINFO_SOURCE;
	}
	else					//	Set to Miracast sink device.
	{
		pwfd_info->wfd_device_type = WFD_DEVINFO_PSINK;
	}

exit:

	return ret;

}

static int rtw_p2p_set_scan_result_type(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter					*padapter = netdev_priv(dev);
	struct iw_point				*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	struct wifi_display_info		*pwfd_info = pwdinfo->wfd_info;

	//	Commented by Albert 20120328
	//	The input data is 0 , 1 , 2
	//	0: when the P2P is enabled, the scan result will return all the found P2P device.
	//	1: when the P2P is enabled, the scan result will return all the found P2P device and AP.
	//	2: when the P2P is enabled, the scan result will show up the found Miracast devices base on...
	//	It will show up all the Miracast source device if this device is sink.
	//	It will show up all the Miracast sink device if this device is source.

	DBG_8723A( "[%s] data = %s\n", __func__, extra );

	if ( extra[ 0 ] == '0' )
	{
		pwfd_info->scan_result_type = SCAN_RESULT_P2P_ONLY;
	}
	else if ( extra[ 0 ] == '1' )
	{
		pwfd_info->scan_result_type = SCAN_RESULT_ALL;
	}
	else if ( extra[ 0 ] == '2' )
	{
		pwfd_info->scan_result_type = SCAN_RESULT_WFD_TYPE;
	}
	else
	{
		pwfd_info->scan_result_type = SCAN_RESULT_P2P_ONLY;
	}

exit:

	return ret;

}

//	To set the WFD session available to enable or disable
static int rtw_p2p_set_sa(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter					*padapter = netdev_priv(dev);
	struct iw_point				*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	struct wifi_display_info		*pwfd_info = pwdinfo->wfd_info;

	DBG_8723A( "[%s] data = %s\n", __func__, extra );

	if( 0 )
	{
		DBG_8723A( "[%s] WiFi Direct is disable!\n", __func__ );
		return ret;
	}
	else
	{
		if ( extra[ 0 ] == '0' )	//	Disable the session available.
		{
			pwdinfo->session_available = false;
		}
		else if ( extra[ 0 ] == '1' )	//	Enable the session available.
		{
			pwdinfo->session_available = true;
		}
		else
		{
			pwdinfo->session_available = false;
		}
	}
	printk( "[%s] session available = %d\n", __func__, pwdinfo->session_available );

exit:

	return ret;

}
#endif // CONFIG_8723AU_P2P

static int rtw_p2p_prov_disc(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	struct rtw_adapter				*padapter = netdev_priv(dev);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int					jj,kk;
	u8					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead, *ptmp;
	_queue				*queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork;
	uint					uintPeerChannel = 0;
	u8					attr_content[100] = { 0x00 }, _status = 0;
	u8 *p2pie;
	uint					p2pielen = 0, attr_contentlen = 0;
#ifdef CONFIG_8723AU_P2P
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;
#endif // CONFIG_8723AU_P2P

	//	Commented by Albert 20110301
	//	The input data contains two informations.
	//	1. First information is the MAC address which wants to issue the provisioning discovery request frame.
	//	2. Second information is the WPS configuration method which wants to discovery
	//	Format: 00:E0:4C:00:00:05_display
	//	Format: 00:E0:4C:00:00:05_keypad
	//	Format: 00:E0:4C:00:00:05_pbc
	//	Format: 00:E0:4C:00:00:05_label

	DBG_8723A( "[%s] data = %s\n", __func__, extra );

	if ( pwdinfo->p2p_state == P2P_STATE_NONE )
	{
		DBG_8723A( "[%s] WiFi Direct is disable!\n", __func__ );
		return ret;
	}
	else
	{
		//	Reset the content of struct tx_provdisc_req_info excluded the wps_config_method_request.
		memset( pwdinfo->tx_prov_disc_info.peerDevAddr, 0x00, ETH_ALEN );
		memset( pwdinfo->tx_prov_disc_info.peerIFAddr, 0x00, ETH_ALEN );
		memset( &pwdinfo->tx_prov_disc_info.ssid, 0x00, sizeof( NDIS_802_11_SSID ) );
		pwdinfo->tx_prov_disc_info.peer_channel_num[ 0 ] = 0;
		pwdinfo->tx_prov_disc_info.peer_channel_num[ 1 ] = 0;
		pwdinfo->tx_prov_disc_info.benable = false;
	}

	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 )
	{
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	if (!memcmp(&extra[ 18 ], "display", 7))
	{
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_DISPLYA;
	}
	else if (!memcmp( &extra[ 18 ], "keypad", 7))
	{
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_KEYPAD;
	}
	else if (!memcmp( &extra[ 18 ], "pbc", 3))
	{
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_PUSH_BUTTON;
	}
	else if (!memcmp( &extra[ 18 ], "label", 5))
	{
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_LABEL;
	}
	else
	{
		DBG_8723A( "[%s] Unknown WPS config methodn", __func__ );
		return( ret );
	}

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);

	list_for_each_safe(plist, ptmp, phead) {
		if( uintPeerChannel != 0 )
			break;

		pnetwork = container_of(plist, struct wlan_network, list);

		//	Commented by Albert 2011/05/18
		//	Match the device address located in the P2P IE
		//	This is for the case that the P2P device address is not the same as the P2P interface address.

		if ( (p2pie=rtw_get_p2p_ie( &pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &p2pielen)) )
		{
			while ( p2pie )
			{
				//	The P2P Device ID attribute is included in the Beacon frame.
				//	The P2P Device Info attribute is included in the probe response frame.

				if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen) )
				{
					//	Handle the P2P Device ID attribute of Beacon first
					if (!memcmp( attr_content, peerMAC, ETH_ALEN))
					{
						uintPeerChannel = pnetwork->network.Configuration.DSConfig;
						break;
					}
				}
				else if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen) )
				{
					//	Handle the P2P Device Info attribute of probe response
					if (!memcmp( attr_content, peerMAC, ETH_ALEN))
					{
						uintPeerChannel = pnetwork->network.Configuration.DSConfig;
						break;
					}
				}

				//Get the next P2P IE
				p2pie = rtw_get_p2p_ie(p2pie+p2pielen, pnetwork->network.IELength - 12 -(p2pie -&pnetwork->network.IEs[12] + p2pielen), NULL, &p2pielen);
			}

		}
	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

#ifdef CONFIG_8723AU_P2P
	{
		u8	wfd_ie[ 128 ] = { 0x00 };
		uint	wfd_ielen = 0;

		if ( rtw_get_wfd_ie( &pnetwork->network.IEs[12], pnetwork->network.IELength - 12,  wfd_ie, &wfd_ielen ) )
		{
			u8	wfd_devinfo[ 6 ] = { 0x00 };
			uint	wfd_devlen = 6;

			DBG_8723A( "[%s] Found WFD IE!\n", __func__ );
			if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen ) )
			{
				u16	wfd_devinfo_field = 0;

				//	Commented by Albert 20120319
				//	The first two bytes are the WFD device information field of WFD device information subelement.
				//	In big endian format.
				wfd_devinfo_field = RTW_GET_BE16(wfd_devinfo);
				if ( wfd_devinfo_field & WFD_DEVINFO_SESSION_AVAIL )
				{
					pwfd_info->peer_session_avail = true;
				}
				else
				{
					pwfd_info->peer_session_avail = false;
				}
			}
		}

		if ( false == pwfd_info->peer_session_avail )
		{
			DBG_8723A( "[%s] WFD Session not avaiable!\n", __func__ );
			goto exit;
		}
	}
#endif // CONFIG_8723AU_P2P

	if ( uintPeerChannel )
	{

		DBG_8723A( "[%s] peer channel: %d!\n", __func__, uintPeerChannel );
		memcpy(pwdinfo->tx_prov_disc_info.peerIFAddr, pnetwork->network.MacAddress, ETH_ALEN );
		memcpy(pwdinfo->tx_prov_disc_info.peerDevAddr, peerMAC, ETH_ALEN );
		pwdinfo->tx_prov_disc_info.peer_channel_num[0] = ( u16 ) uintPeerChannel;
		pwdinfo->tx_prov_disc_info.benable = true;
		rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));
		rtw_p2p_set_state(pwdinfo, P2P_STATE_TX_PROVISION_DIS_REQ);

		if(rtw_p2p_chk_role(pwdinfo, P2P_ROLE_CLIENT))
		{
			memcpy(&pwdinfo->tx_prov_disc_info.ssid, &pnetwork->network.Ssid, sizeof( NDIS_802_11_SSID ) );
		}
		else if(rtw_p2p_chk_role(pwdinfo, P2P_ROLE_DEVICE) || rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO))
		{
			memcpy(pwdinfo->tx_prov_disc_info.ssid.Ssid, pwdinfo->p2p_wildcard_ssid, P2P_WILDCARD_SSID_LEN );
			pwdinfo->tx_prov_disc_info.ssid.SsidLength= P2P_WILDCARD_SSID_LEN;
		}

		set_channel_bwmode(padapter, uintPeerChannel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);

		mod_timer(&pwdinfo->pre_tx_scan_timer,
			  jiffies + msecs_to_jiffies(P2P_TX_PRESCAN_TIMEOUT));

		mod_timer(&pwdinfo->restore_p2p_state_timer,
			  jiffies + msecs_to_jiffies(P2P_PROVISION_TIMEOUT));
	} else {
		DBG_8723A( "[%s] NOT Found in the Scanning Queue!\n", __func__ );
	}
exit:

	return ret;
}

//	Added by Albert 20110328
//	This function is used to inform the driver the user had specified the pin code value or pbc
//	to application.

static int rtw_p2p_got_wpsinfo(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct rtw_adapter				*padapter = netdev_priv(dev);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_8723A( "[%s] data = %s\n", __func__, extra );
	//	Added by Albert 20110328
	//	if the input data is P2P_NO_WPSINFO -> reset the wpsinfo
	//	if the input data is P2P_GOT_WPSINFO_PEER_DISPLAY_PIN -> the utility just input the PIN code got from the peer P2P device.
	//	if the input data is P2P_GOT_WPSINFO_SELF_DISPLAY_PIN -> the utility just got the PIN code from itself.
	//	if the input data is P2P_GOT_WPSINFO_PBC -> the utility just determine to use the PBC

	if ( *extra == '0' )
	{
		pwdinfo->ui_got_wps_info = P2P_NO_WPSINFO;
	}
	else if ( *extra == '1' )
	{
		pwdinfo->ui_got_wps_info = P2P_GOT_WPSINFO_PEER_DISPLAY_PIN;
	}
	else if ( *extra == '2' )
	{
		pwdinfo->ui_got_wps_info = P2P_GOT_WPSINFO_SELF_DISPLAY_PIN;
	}
	else if ( *extra == '3' )
	{
		pwdinfo->ui_got_wps_info = P2P_GOT_WPSINFO_PBC;
	}
	else
	{
		pwdinfo->ui_got_wps_info = P2P_NO_WPSINFO;
	}

	return ret;

}

#endif //CONFIG_8723AU_P2P

static void mac_reg_dump(struct rtw_adapter *padapter)
{
	int i,j=1;
	printk("\n======= MAC REG =======\n");
	for(i=0x0;i<0x300;i+=4)
	{
		if(j%4==1)	printk("0x%02x",i);
		printk(" 0x%08x ",rtw_read32(padapter,i));
		if((j++)%4 == 0)	printk("\n");
	}
	for(i=0x400;i<0x800;i+=4)
	{
		if(j%4==1)	printk("0x%02x",i);
		printk(" 0x%08x ",rtw_read32(padapter,i));
		if((j++)%4 == 0)	printk("\n");
	}
}
void bb_reg_dump(struct rtw_adapter *padapter)
{
	int i,j=1;
	printk("\n======= BB REG =======\n");
	for(i=0x800;i<0x1000;i+=4)
	{
		if(j%4==1) printk("0x%02x",i);

		printk(" 0x%08x ",rtw_read32(padapter,i));
		if((j++)%4 == 0)	printk("\n");
	}
}
void rf_reg_dump(struct rtw_adapter *padapter)
{
	int i,j=1,path;
	u32 value;
	u8 rf_type,path_nums = 0;
	rtw_hal_get_hwreg(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));

	printk("\n======= RF REG =======\n");
	if((RF_1T2R == rf_type) ||(RF_1T1R ==rf_type ))
		path_nums = 1;
	else
		path_nums = 2;

	for(path=0;path<path_nums;path++)
	{
		printk("\nRF_Path(%x)\n",path);
		for(i=0;i<0x100;i++)
		{
			//value = PHY_QueryRFReg(padapter, (RF_RADIO_PATH_E)path,i, bMaskDWord);
			value = rtw_hal_read_rfreg(padapter, path, i, 0xffffffff);
			if(j%4==1)	printk("0x%02x ",i);
			printk(" 0x%08x ",value);
			if((j++)%4==0)	printk("\n");
		}
	}
}

static int wpa_set_param(struct net_device *dev, u8 name, u32 value)
{
	uint ret=0;
	u32 flags;
	struct rtw_adapter *padapter = netdev_priv(dev);

	switch (name){
	case IEEE_PARAM_WPA_ENABLED:

		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X; //802.1x

		//ret = ieee80211_wpa_enable(ieee, value);

		switch((value)&0xff)
		{
			case 1 : //WPA
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeWPAPSK; //WPA_PSK
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case 2: //WPA2
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeWPA2PSK; //WPA2_PSK
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
		}

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("wpa_set_param:padapter->securitypriv.ndisauthtype=%d\n", padapter->securitypriv.ndisauthtype));

		break;

	case IEEE_PARAM_TKIP_COUNTERMEASURES:
		//ieee->tkip_countermeasures=value;
		break;

	case IEEE_PARAM_DROP_UNENCRYPTED:
	{
		/* HACK:
		 *
		 * wpa_supplicant calls set_wpa_enabled when the driver
		 * is loaded and unloaded, regardless of if WPA is being
		 * used.  No other calls are made which can be used to
		 * determine if encryption will be used or not prior to
		 * association being expected.  If encryption is not being
		 * used, drop_unencrypted is set to false, else true -- we
		 * can use this to determine if the CAP_PRIVACY_ON bit should
		 * be set.
		 */

		break;
	}
	case IEEE_PARAM_PRIVACY_INVOKED:

		//ieee->privacy_invoked=value;

		break;

	case IEEE_PARAM_AUTH_ALGS:

		ret = wpa_set_auth_algs(dev, value);

		break;

	case IEEE_PARAM_IEEE_802_1X:

		//ieee->ieee802_1x=value;

		break;

	case IEEE_PARAM_WPAX_SELECT:

		// added for WPA2 mixed mode
		//DBG_8723A(KERN_WARNING "------------------------>wpax value = %x\n", value);
		/*
		spin_lock_irqsave(&ieee->wpax_suitlist_lock,flags);
		ieee->wpax_type_set = 1;
		ieee->wpax_type_notify = value;
		spin_unlock_irqrestore(&ieee->wpax_suitlist_lock,flags);
		*/

		break;

	default:



		ret = -EOPNOTSUPP;


		break;

	}

	return ret;

}

static int wpa_mlme(struct net_device *dev, u32 command, u32 reason)
{
	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);

	switch (command)
	{
		case IEEE_MLME_STA_DEAUTH:

			if(!rtw_set_802_11_disassociate(padapter))
				ret = -1;

			break;

		case IEEE_MLME_STA_DISASSOC:

			if(!rtw_set_802_11_disassociate(padapter))
				ret = -1;

			break;

		default:
			ret = -EOPNOTSUPP;
			break;
	}

	return ret;

}

static int wpa_supplicant_ioctl(struct net_device *dev, struct iw_point *p)
{
	struct ieee_param *param;
	uint ret=0;

	//down(&ieee->wx_sem);

	if (p->length < sizeof(struct ieee_param) || !p->pointer){
		ret = -EINVAL;
		goto out;
	}

	param = (struct ieee_param *)kmalloc(p->length, GFP_KERNEL);
	if (param == NULL)
	{
		ret = -ENOMEM;
		goto exit;
	}

	if (copy_from_user(param, p->pointer, p->length))
	{
		ret = -EFAULT;
		goto out;
	}

	switch (param->cmd) {

	case IEEE_CMD_SET_WPA_PARAM:
		ret = wpa_set_param(dev, param->u.wpa_param.name, param->u.wpa_param.value);
		break;

	case IEEE_CMD_SET_WPA_IE:
		//ret = wpa_set_wpa_ie(dev, param, p->length);
		ret =  rtw_set_wpa_ie(netdev_priv(dev), (char*)param->u.wpa_ie.data, (u16)param->u.wpa_ie.len);
		break;

	case IEEE_CMD_SET_ENCRYPTION:
		ret = wpa_set_encryption(dev, param, p->length);
		break;

	case IEEE_CMD_MLME:
		ret = wpa_mlme(dev, param->u.mlme.command, param->u.mlme.reason_code);
		break;

	default:
		DBG_8723A("Unknown WPA supplicant request: %d\n", param->cmd);
		ret = -EOPNOTSUPP;
		break;

	}

	if (ret == 0 && copy_to_user(p->pointer, param, p->length))
		ret = -EFAULT;

out:
	kfree(param);
exit:
	//up(&ieee->wx_sem);

	return ret;

}

#ifdef CONFIG_8723AU_AP_MODE
static u8 set_pairwise_key(struct rtw_adapter *padapter, struct sta_info *psta)
{
	struct cmd_obj*			ph2c;
	struct set_stakey_parm	*psetstakey_para;
	struct cmd_priv				*pcmdpriv=&padapter->cmdpriv;
	u8	res=_SUCCESS;

	ph2c = (struct cmd_obj*)kzalloc(sizeof(struct cmd_obj), GFP_KERNEL);
	if ( ph2c == NULL){
		res= _FAIL;
		goto fail1;
	}

	psetstakey_para = (struct set_stakey_parm*)kzalloc(sizeof(struct set_stakey_parm), GFP_KERNEL);
	if(psetstakey_para==NULL){
		kfree(ph2c);
		res=_FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, psetstakey_para, _SetStaKey_CMD_);


	psetstakey_para->algorithm = (u8)psta->dot118021XPrivacy;

	memcpy(psetstakey_para->addr, psta->hwaddr, ETH_ALEN);

	memcpy(psetstakey_para->key, &psta->dot118021x_UncstKey, 16);


	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

	kfree(psetstakey_para);
exit:
	kfree(ph2c);
fail1:
	return res;

}

static int set_group_key(struct rtw_adapter *padapter, u8 *key, u8 alg, int keyid)
{
	u8 keylen;
	struct cmd_obj* pcmd;
	struct setkey_parm *psetkeyparm;
	struct cmd_priv	*pcmdpriv=&(padapter->cmdpriv);
	int res=_SUCCESS;

	DBG_8723A("%s\n", __func__);

	pcmd = (struct cmd_obj*)kzalloc(sizeof(struct cmd_obj), GFP_KERNEL);
	if (!pcmd) {
		res= _FAIL;
		goto fail;
	}
	psetkeyparm=(struct setkey_parm*)kzalloc(sizeof(struct setkey_parm),
						 GFP_KERNEL);
	if (!psetkeyparm) {
		res= _FAIL;
		goto exit;
	}

	psetkeyparm->keyid=(u8)keyid;
	if (is_wep_enc(alg))
		padapter->mlmepriv.key_mask |= BIT(psetkeyparm->keyid);
	psetkeyparm->algorithm = alg;

	psetkeyparm->set_tx = 1;

	switch(alg) {
	case _WEP40_:
		keylen = 5;
		break;
	case _WEP104_:
		keylen = 13;
		break;
	case _TKIP_:
	case _TKIP_WTMIC_:
	case _AES_:
	default:
		keylen = 16;
	}
	memcpy(&(psetkeyparm->key[0]), key, keylen);
	pcmd->cmdcode = _SetKey_CMD_;
	pcmd->parmbuf = (u8 *)psetkeyparm;
	pcmd->cmdsz =  (sizeof(struct setkey_parm));
	pcmd->rsp = NULL;
	pcmd->rspsz = 0;

	INIT_LIST_HEAD(&pcmd->list);

	res = rtw_enqueue_cmd(pcmdpriv, pcmd);

	kfree(psetkeyparm);
exit:
	kfree(pcmd);
fail:
	return res;
}

static int set_wep_key(struct rtw_adapter *padapter, u8 *key, u8 keylen, int keyid)
{
	u8 alg;

	switch(keylen) {
	case 5:
		alg =_WEP40_;
		break;
	case 13:
		alg =_WEP104_;
		break;
	default:
		alg =_NO_PRIVACY_;
	}

	return set_group_key(padapter, key, alg, keyid);
}


static int rtw_set_encryption(struct net_device *dev, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len,wep_total_len;
	NDIS_802_11_WEP	 *pwep = NULL;
	struct sta_info *psta = NULL, *pbcmc_sta = NULL;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_8723A("%s\n", __func__);

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	//sizeof(struct ieee_param) = 64 bytes;
	//if (param_len !=  (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len)
	if (param_len !=  sizeof(struct ieee_param) + param->u.crypt.key_len)
	{
		ret =  -EINVAL;
		goto exit;
	}

	if (is_broadcast_ether_addr(param->sta_addr)) {
		if (param->u.crypt.idx >= WEP_KEYS)
		{
			ret = -EINVAL;
			goto exit;
		}
	}
	else
	{
		psta = rtw_get_stainfo(pstapriv, param->sta_addr);
		if(!psta)
		{
			//ret = -EINVAL;
			DBG_8723A("rtw_set_encryption(), sta has already been removed or never been added\n");
			goto exit;
		}
	}

	if (strcmp(param->u.crypt.alg, "none") == 0 && (psta==NULL))
	{
		//todo:clear default encryption keys

		DBG_8723A("clear default encryption keys, keyid=%d\n", param->u.crypt.idx);

		goto exit;
	}


	if (strcmp(param->u.crypt.alg, "WEP") == 0 && (psta==NULL))
	{
		DBG_8723A("r871x_set_encryption, crypt.alg = WEP\n");

		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;

		DBG_8723A("r871x_set_encryption, wep_key_idx=%d, len=%d\n", wep_key_idx, wep_key_len);

		if((wep_key_idx >= WEP_KEYS) || (wep_key_len<=0))
		{
			ret = -EINVAL;
			goto exit;
		}


		if (wep_key_len > 0)
		{
			wep_key_len = wep_key_len <= 5 ? 5 : 13;
			wep_total_len = wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
			pwep = (NDIS_802_11_WEP *)kzalloc(wep_total_len,
							  GFP_KERNEL);
			if(pwep == NULL){
				DBG_8723A(" r871x_set_encryption: pwep allocate fail !!!\n");
				goto exit;
			}

			pwep->KeyLength = wep_key_len;
			pwep->Length = wep_total_len;

		}

		pwep->KeyIndex = wep_key_idx;

		memcpy(pwep->KeyMaterial,  param->u.crypt.key, pwep->KeyLength);

		if(param->u.crypt.set_tx)
		{
			DBG_8723A("wep, set_tx=1\n");

			psecuritypriv->ndisencryptstatus = Ndis802_11Encryption1Enabled;
			psecuritypriv->dot11PrivacyAlgrthm=_WEP40_;
			psecuritypriv->dot118021XGrpPrivacy=_WEP40_;

			if(pwep->KeyLength==13)
			{
				psecuritypriv->dot11PrivacyAlgrthm=_WEP104_;
				psecuritypriv->dot118021XGrpPrivacy=_WEP104_;
			}


			psecuritypriv->dot11PrivacyKeyIndex = wep_key_idx;

			memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);

			psecuritypriv->dot11DefKeylen[wep_key_idx]=pwep->KeyLength;

			set_wep_key(padapter, pwep->KeyMaterial, pwep->KeyLength, wep_key_idx);


		}
		else
		{
			DBG_8723A("wep, set_tx=0\n");

			//don't update "psecuritypriv->dot11PrivacyAlgrthm" and
			//"psecuritypriv->dot11PrivacyKeyIndex=keyid", but can rtw_set_key to cam

			memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);

			psecuritypriv->dot11DefKeylen[wep_key_idx] = pwep->KeyLength;

			set_wep_key(padapter, pwep->KeyMaterial, pwep->KeyLength, wep_key_idx);

		}

		goto exit;

	}


	if(!psta && check_fwstate(pmlmepriv, WIFI_AP_STATE)) // //group key
	{
		if(param->u.crypt.set_tx ==1)
		{
			if(strcmp(param->u.crypt.alg, "WEP") == 0)
			{
				DBG_8723A("%s, set group_key, WEP\n", __func__);

				memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
				if(param->u.crypt.key_len==13)
				{
						psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
				}

			}
			else if(strcmp(param->u.crypt.alg, "TKIP") == 0)
			{
				DBG_8723A("%s, set group_key, TKIP\n", __func__);

				psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

				memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
				//set mic key
				memcpy(psecuritypriv->dot118021XGrptxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[16]), 8);
				memcpy(psecuritypriv->dot118021XGrprxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[24]), 8);

				psecuritypriv->busetkipkey = true;

			}
			else if(strcmp(param->u.crypt.alg, "CCMP") == 0)
			{
				DBG_8723A("%s, set group_key, CCMP\n", __func__);

				psecuritypriv->dot118021XGrpPrivacy = _AES_;

				memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
			}
			else
			{
				DBG_8723A("%s, set group_key, none\n", __func__);

				psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
			}

			psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

			psecuritypriv->binstallGrpkey = true;

			psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;//!!!

			set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);

			pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
			if(pbcmc_sta)
			{
				pbcmc_sta->ieee8021x_blocked = false;
				pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;//rx will use bmc_sta's dot118021XPrivacy
			}

		}

		goto exit;

	}

	if(psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X && psta) // psk/802_1x
	{
		if(check_fwstate(pmlmepriv, WIFI_AP_STATE))
		{
			if(param->u.crypt.set_tx ==1)
			{
				memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				if(strcmp(param->u.crypt.alg, "WEP") == 0)
				{
					DBG_8723A("%s, set pairwise key, WEP\n", __func__);

					psta->dot118021XPrivacy = _WEP40_;
					if(param->u.crypt.key_len==13)
					{
						psta->dot118021XPrivacy = _WEP104_;
					}
				}
				else if(strcmp(param->u.crypt.alg, "TKIP") == 0)
				{
					DBG_8723A("%s, set pairwise key, TKIP\n", __func__);

					psta->dot118021XPrivacy = _TKIP_;

					//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
					//set mic key
					memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
					memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = true;

				}
				else if(strcmp(param->u.crypt.alg, "CCMP") == 0)
				{

					DBG_8723A("%s, set pairwise key, CCMP\n", __func__);

					psta->dot118021XPrivacy = _AES_;
				}
				else
				{
					DBG_8723A("%s, set pairwise key, none\n", __func__);

					psta->dot118021XPrivacy = _NO_PRIVACY_;
				}

				set_pairwise_key(padapter, psta);

				psta->ieee8021x_blocked = false;

			}
			else//group key???
			{
				if(strcmp(param->u.crypt.alg, "WEP") == 0)
				{
					memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
					if(param->u.crypt.key_len==13)
					{
						psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
					}
				}
				else if(strcmp(param->u.crypt.alg, "TKIP") == 0)
				{
					psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

					memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
					//set mic key
					memcpy(psecuritypriv->dot118021XGrptxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[16]), 8);
					memcpy(psecuritypriv->dot118021XGrprxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = true;

				}
				else if(strcmp(param->u.crypt.alg, "CCMP") == 0)
				{
					psecuritypriv->dot118021XGrpPrivacy = _AES_;

					memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
				}
				else
				{
					psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
				}

				psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

				psecuritypriv->binstallGrpkey = true;

				psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;//!!!

				set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);

				pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
				if(pbcmc_sta)
				{
					pbcmc_sta->ieee8021x_blocked = false;
					pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;//rx will use bmc_sta's dot118021XPrivacy
				}

			}

		}

	}

exit:

	kfree(pwep);

	return ret;

}

static int rtw_set_beacon(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	unsigned char *pbuf = param->u.bcn_ie.buf;


	DBG_8723A("%s, len=%d\n", __func__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	memcpy(&pstapriv->max_num_sta, param->u.bcn_ie.reserved, 2);

	if((pstapriv->max_num_sta>NUM_STA) || (pstapriv->max_num_sta<=0))
		pstapriv->max_num_sta = NUM_STA;


	if(rtw_check_beacon_data(padapter, pbuf,  (len-12-2)) == _SUCCESS)// 12 = param header, 2:no packed
		ret = 0;
	else
		ret = -EINVAL;


	return ret;

}

static int rtw_hostapd_sta_flush(struct net_device *dev)
{
	//struct list_head	*phead, *plist;
	int ret=0;
	//struct sta_info *psta = NULL;
	struct rtw_adapter *padapter = netdev_priv(dev);
	//struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_8723A("%s\n", __func__);

	flush_all_cam_entry(padapter);	//clear CAM

	ret = rtw_sta_flush(padapter);

	return ret;

}

static int rtw_add_sta(struct net_device *dev, struct ieee_param *param)
{
	int ret=0;
	struct sta_info *psta = NULL;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_8723A("rtw_add_sta(aid=%d)=" MAC_FMT "\n", param->u.add_sta.aid, MAC_ARG(param->sta_addr));

	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != true)
	{
		return -EINVAL;
	}

	if (is_broadcast_ether_addr(param->sta_addr))
		return -EINVAL;

/*
	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		DBG_8723A("rtw_add_sta(), free has been added psta=%p\n", psta);
		spin_lock_bh(&(pstapriv->sta_hash_lock));
		rtw_free_stainfo(padapter,  psta);
		spin_unlock_bh(&(pstapriv->sta_hash_lock));

		psta = NULL;
	}
*/
	//psta = rtw_alloc_stainfo(pstapriv, param->sta_addr);
	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		int flags = param->u.add_sta.flags;

		//DBG_8723A("rtw_add_sta(), init sta's variables, psta=%p\n", psta);

		psta->aid = param->u.add_sta.aid;//aid=1~2007

		memcpy(psta->bssrateset, param->u.add_sta.tx_supp_rates, 16);


		//check wmm cap.
		if(WLAN_STA_WME&flags)
			psta->qos_option = 1;
		else
			psta->qos_option = 0;

		if(pmlmepriv->qospriv.qos_option == 0)
			psta->qos_option = 0;


		//chec 802.11n ht cap.
		if(WLAN_STA_HT&flags)
		{
			psta->htpriv.ht_option = true;
			psta->qos_option = 1;
			memcpy((void*)&psta->htpriv.ht_cap, (void*)&param->u.add_sta.ht_cap, sizeof(struct ieee80211_ht_cap));
		}
		else
		{
			psta->htpriv.ht_option = false;
		}

		if(pmlmepriv->htpriv.ht_option == false)
			psta->htpriv.ht_option = false;


		update_sta_info_apmode(padapter, psta);


	}
	else
	{
		ret = -ENOMEM;
	}

	return ret;

}

static int rtw_del_sta(struct net_device *dev, struct ieee_param *param)
{
	int ret=0;
	struct sta_info *psta = NULL;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_8723A("rtw_del_sta=" MAC_FMT "\n", MAC_ARG(param->sta_addr));

	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != true)
	{
		return -EINVAL;
	}

	if (is_broadcast_ether_addr(param->sta_addr))
		return -EINVAL;

	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		u8 updated;

		//DBG_8723A("free psta=%p, aid=%d\n", psta, psta->aid);

		spin_lock_bh(&pstapriv->asoc_list_lock);
		if (!list_empty(&psta->asoc_list)) {
			list_del_init(&psta->asoc_list);
			pstapriv->asoc_list_cnt--;
			updated = ap_free_sta(padapter, psta, true,
					      WLAN_REASON_DEAUTH_LEAVING);
		}
		spin_unlock_bh(&pstapriv->asoc_list_lock);

		associated_clients_update(padapter, updated);

		psta = NULL;

	}
	else
	{
		DBG_8723A("rtw_del_sta(), sta has already been removed or never been added\n");

		//ret = -1;
	}


	return ret;

}

static int rtw_ioctl_get_sta_data(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct sta_info *psta = NULL;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct ieee_param_ex *param_ex = (struct ieee_param_ex *)param;
	struct sta_data *psta_data = (struct sta_data *)param_ex->data;

	DBG_8723A("rtw_ioctl_get_sta_info, sta_addr: " MAC_FMT "\n", MAC_ARG(param_ex->sta_addr));

	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != true)
	{
		return -EINVAL;
	}

	if (is_broadcast_ether_addr(param_ex->sta_addr))
		return -EINVAL;

	psta = rtw_get_stainfo(pstapriv, param_ex->sta_addr);
	if(psta)
	{
		psta_data->aid = (u16)psta->aid;
		psta_data->capability = psta->capability;
		psta_data->flags = psta->flags;

/*
		nonerp_set : BIT(0)
		no_short_slot_time_set : BIT(1)
		no_short_preamble_set : BIT(2)
		no_ht_gf_set : BIT(3)
		no_ht_set : BIT(4)
		ht_20mhz_set : BIT(5)
*/

		psta_data->sta_set =((psta->nonerp_set) |
							(psta->no_short_slot_time_set <<1) |
							(psta->no_short_preamble_set <<2) |
							(psta->no_ht_gf_set <<3) |
							(psta->no_ht_set <<4) |
							(psta->ht_20mhz_set <<5));

		psta_data->tx_supp_rates_len =  psta->bssratelen;
		memcpy(psta_data->tx_supp_rates, psta->bssrateset, psta->bssratelen);
		memcpy(&psta_data->ht_cap, &psta->htpriv.ht_cap, sizeof(struct ieee80211_ht_cap));
		psta_data->rx_pkts = psta->sta_stats.rx_data_pkts;
		psta_data->rx_bytes = psta->sta_stats.rx_bytes;
		psta_data->rx_drops = psta->sta_stats.rx_drops;

		psta_data->tx_pkts = psta->sta_stats.tx_pkts;
		psta_data->tx_bytes = psta->sta_stats.tx_bytes;
		psta_data->tx_drops = psta->sta_stats.tx_drops;


	}
	else
	{
		ret = -1;
	}

	return ret;

}

static int rtw_get_sta_wpaie(struct net_device *dev, struct ieee_param *param)
{
	int ret=0;
	struct sta_info *psta = NULL;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_8723A("rtw_get_sta_wpaie, sta_addr: " MAC_FMT "\n", MAC_ARG(param->sta_addr));

	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != true)
	{
		return -EINVAL;
	}

	if (is_broadcast_ether_addr(param->sta_addr))
		return -EINVAL;

	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		if((psta->wpa_ie[0] == WLAN_EID_RSN) || (psta->wpa_ie[0] == WLAN_EID_VENDOR_SPECIFIC))
		{
			int wpa_ie_len;
			int copy_len;

			wpa_ie_len = psta->wpa_ie[1];

			copy_len = ((wpa_ie_len+2) > sizeof(psta->wpa_ie)) ? (sizeof(psta->wpa_ie)):(wpa_ie_len+2);

			param->u.wpa_ie.len = copy_len;

			memcpy(param->u.wpa_ie.reserved, psta->wpa_ie, copy_len);
		}
		else
		{
			//ret = -1;
			DBG_8723A("sta's wpa_ie is NONE\n");
		}
	}
	else
	{
		ret = -1;
	}

	return ret;

}

static int rtw_set_wps_beacon(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	unsigned char wps_oui[4]={0x0,0x50,0xf2,0x04};
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	int ie_len;

	DBG_8723A("%s, len=%d\n", __func__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed


	kfree(pmlmepriv->wps_beacon_ie);
	pmlmepriv->wps_beacon_ie = NULL;

	if(ie_len>0)
	{
		pmlmepriv->wps_beacon_ie = kmalloc(ie_len, GFP_KERNEL);
		pmlmepriv->wps_beacon_ie_len = ie_len;
		if ( pmlmepriv->wps_beacon_ie == NULL) {
			DBG_8723A("%s()-%d: rtw_malloc() ERROR!\n", __func__, __LINE__);
			return -EINVAL;
		}

		memcpy(pmlmepriv->wps_beacon_ie, param->u.bcn_ie.buf, ie_len);

		update_beacon(padapter, _VENDOR_SPECIFIC_IE_, wps_oui, true);

		pmlmeext->bstart_bss = true;
	}

	return ret;
}

static int rtw_set_wps_probe_resp(struct net_device *dev,
				  struct ieee_param *param, int len)
{
	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	DBG_8723A("%s, len=%d\n", __func__, len);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed

	kfree(pmlmepriv->wps_probe_resp_ie);
	pmlmepriv->wps_probe_resp_ie = NULL;

	if (ie_len > 0) {
		pmlmepriv->wps_probe_resp_ie = kmalloc(ie_len, GFP_KERNEL);
		pmlmepriv->wps_probe_resp_ie_len = ie_len;
		if (!pmlmepriv->wps_probe_resp_ie) {
			DBG_8723A("%s()-%d: rtw_malloc() ERROR!\n",
				  __func__, __LINE__);
			return -EINVAL;
		}
		memcpy(pmlmepriv->wps_probe_resp_ie, param->u.bcn_ie.buf,
		       ie_len);
	}

	return ret;
}

static int rtw_set_wps_assoc_resp(struct net_device *dev,
				  struct ieee_param *param, int len)
{
	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	DBG_8723A("%s, len=%d\n", __func__, len);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed


	kfree(pmlmepriv->wps_assoc_resp_ie);
	pmlmepriv->wps_assoc_resp_ie = NULL;

	if (ie_len > 0) {
		pmlmepriv->wps_assoc_resp_ie = kmalloc(ie_len, GFP_KERNEL);
		pmlmepriv->wps_assoc_resp_ie_len = ie_len;
		if (pmlmepriv->wps_assoc_resp_ie == NULL) {
			DBG_8723A("%s()-%d: rtw_malloc() ERROR!\n",
				  __func__, __LINE__);
			return -EINVAL;
		}

		memcpy(pmlmepriv->wps_assoc_resp_ie, param->u.bcn_ie.buf,
		       ie_len);
	}

	return ret;
}

static int rtw_set_hidden_ssid(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct rtw_adapter *adapter = netdev_priv(dev);
	struct mlme_priv *mlmepriv = &(adapter->mlmepriv);
	struct mlme_ext_priv	*mlmeext = &(adapter->mlmeextpriv);
	struct mlme_ext_info	*mlmeinfo = &(mlmeext->mlmext_info);
	int ie_len;
	u8 *ssid_ie;
	char ssid[NDIS_802_11_LENGTH_SSID + 1];
	int ssid_len;
	u8 ignore_broadcast_ssid;

	if(check_fwstate(mlmepriv, WIFI_AP_STATE) != true)
		return -EPERM;

	if (param->u.bcn_ie.reserved[0] != 0xea)
		return -EINVAL;

	mlmeinfo->hidden_ssid_mode = ignore_broadcast_ssid = param->u.bcn_ie.reserved[1];

	ie_len = len-12-2;// 12 = param header, 2:no packed
	ssid_ie = rtw_get_ie(param->u.bcn_ie.buf,  WLAN_EID_SSID, &ssid_len, ie_len);

	if (ssid_ie && ssid_len) {
		WLAN_BSSID_EX *pbss_network = &mlmepriv->cur_network.network;
		WLAN_BSSID_EX *pbss_network_ext = &mlmeinfo->network;

		memcpy(ssid, ssid_ie+2, ssid_len);
		ssid[ssid_len>NDIS_802_11_LENGTH_SSID?NDIS_802_11_LENGTH_SSID:ssid_len] = 0x0;

		if(0)
		DBG_8723A(FUNC_ADPT_FMT" ssid:(%s,%d), from ie:(%s,%d), (%s,%d)\n", FUNC_ADPT_ARG(adapter),
			ssid, ssid_len,
			pbss_network->Ssid.Ssid, pbss_network->Ssid.SsidLength,
			pbss_network_ext->Ssid.Ssid, pbss_network_ext->Ssid.SsidLength);

		memcpy(pbss_network->Ssid.Ssid, (void *)ssid, ssid_len);
		pbss_network->Ssid.SsidLength = ssid_len;
		memcpy(pbss_network_ext->Ssid.Ssid, (void *)ssid, ssid_len);
		pbss_network_ext->Ssid.SsidLength = ssid_len;

		if(0)
		DBG_8723A(FUNC_ADPT_FMT" after ssid:(%s,%d), (%s,%d)\n", FUNC_ADPT_ARG(adapter),
			pbss_network->Ssid.Ssid, pbss_network->Ssid.SsidLength,
			pbss_network_ext->Ssid.Ssid, pbss_network_ext->Ssid.SsidLength);
	}

	DBG_8723A(FUNC_ADPT_FMT" ignore_broadcast_ssid:%d, %s,%d\n", FUNC_ADPT_ARG(adapter),
		ignore_broadcast_ssid, ssid, ssid_len);

	return ret;
}

static int rtw_ioctl_acl_remove_sta(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	if (is_broadcast_ether_addr(param->sta_addr))
		return -EINVAL;

	ret = rtw_acl_remove_sta(padapter, param->sta_addr);

	return ret;
}

static int rtw_ioctl_acl_add_sta(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	if (is_broadcast_ether_addr(param->sta_addr))
		return -EINVAL;

	ret = rtw_acl_add_sta(padapter, param->sta_addr);

	return ret;

}

static int rtw_ioctl_set_macaddr_acl(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	rtw_set_macaddr_acl(padapter, param->u.mlme.command);

	return ret;
}

static int rtw_hostapd_ioctl(struct net_device *dev, struct iw_point *p)
{
	struct ieee_param *param;
	int ret = 0;
	struct rtw_adapter *padapter = netdev_priv(dev);

	//DBG_8723A("%s\n", __func__);

	/*
	* this function is expect to call in master mode, which allows no power saving
	* so, we just check hw_init_completed
	*/

	if (padapter->hw_init_completed==false){
		ret = -EPERM;
		goto fail;
	}


	//if (p->length < sizeof(struct ieee_param) || !p->pointer){
	if(!p->pointer){
		ret = -EINVAL;
		goto fail;
	}

	param = (struct ieee_param *)kmalloc(p->length, GFP_KERNEL);
	if (param == NULL)
	{
		ret = -ENOMEM;
		goto fail;
	}

	if (copy_from_user(param, p->pointer, p->length))
	{
		ret = -EFAULT;
		goto out;
	}

	//DBG_8723A("%s, cmd=%d\n", __func__, param->cmd);

	switch (param->cmd)
	{
		case RTL871X_HOSTAPD_FLUSH:

			ret = rtw_hostapd_sta_flush(dev);

			break;

		case RTL871X_HOSTAPD_ADD_STA:

			ret = rtw_add_sta(dev, param);

			break;

		case RTL871X_HOSTAPD_REMOVE_STA:

			ret = rtw_del_sta(dev, param);

			break;

		case RTL871X_HOSTAPD_SET_BEACON:

			ret = rtw_set_beacon(dev, param, p->length);

			break;

		case RTL871X_SET_ENCRYPTION:

			ret = rtw_set_encryption(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_GET_WPAIE_STA:

			ret = rtw_get_sta_wpaie(dev, param);

			break;

		case RTL871X_HOSTAPD_SET_WPS_BEACON:

			ret = rtw_set_wps_beacon(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_SET_WPS_PROBE_RESP:

			ret = rtw_set_wps_probe_resp(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_SET_WPS_ASSOC_RESP:

			ret = rtw_set_wps_assoc_resp(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_SET_HIDDEN_SSID:

			ret = rtw_set_hidden_ssid(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_GET_INFO_STA:

			ret = rtw_ioctl_get_sta_data(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_SET_MACADDR_ACL:

			ret = rtw_ioctl_set_macaddr_acl(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_ACL_ADD_STA:

			ret = rtw_ioctl_acl_add_sta(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_ACL_REMOVE_STA:

			ret = rtw_ioctl_acl_remove_sta(dev, param, p->length);

			break;

		default:
			DBG_8723A("Unknown hostapd request: %d\n", param->cmd);
			ret = -EOPNOTSUPP;
			break;

	}

	if (ret == 0 && copy_to_user(p->pointer, param, p->length))
		ret = -EFAULT;


out:
	kfree(param);
fail:
	return ret;
}
#endif


int rtw_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct iwreq *wrq = (struct iwreq *)rq;
	int ret=0;

	switch (cmd)
	{
		case RTL_IOCTL_WPA_SUPPLICANT:
			ret = wpa_supplicant_ioctl(dev, &wrq->u.data);
			break;
#ifdef CONFIG_8723AU_AP_MODE
		case RTL_IOCTL_HOSTAPD:
			ret = rtw_hostapd_ioctl(dev, &wrq->u.data);
			break;
#endif // CONFIG_8723AU_AP_MODE
		default:
			ret = -EOPNOTSUPP;
			break;
	}

	return ret;
}
