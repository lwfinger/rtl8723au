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


#define _MLME_OSDEP_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <mlme_osdep.h>
#include <rtw_ioctl_set.h>

void rtw_os_indicate_connect(struct rtw_adapter *adapter)
{

_func_enter_;

	rtw_cfg80211_indicate_connect(adapter);

	netif_carrier_on(adapter->pnetdev);

	if(adapter->pid[2] !=0)
		rtw_signal_process(adapter->pid[2], SIGALRM);

_func_exit_;
}

void rtw_os_indicate_scan_done(struct rtw_adapter *padapter, bool aborted)
{
	rtw_cfg80211_indicate_scan_done(wdev_to_priv(padapter->rtw_wdev), aborted);
}

static RT_PMKID_LIST   backupPMKIDList[ NUM_PMKID_CACHE ];
void rtw_reset_securitypriv(struct rtw_adapter *adapter)
{
	u8	backupPMKIDIndex = 0;
	u8	backupTKIPCountermeasure = 0x00;
	u32	backupTKIPcountermeasure_time = 0;

	if(adapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X)//802.1x
	{
		// Added by Albert 2009/02/18
		// We have to backup the PMK information for WiFi PMK Caching test item.
		//
		// Backup the btkip_countermeasure information.
		// When the countermeasure is trigger, the driver have to disconnect with AP for 60 seconds.

		memset(&backupPMKIDList[0], 0x00, sizeof(RT_PMKID_LIST) * NUM_PMKID_CACHE);

		memcpy(&backupPMKIDList[ 0 ], &adapter->securitypriv.PMKIDList[ 0 ], sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
		backupPMKIDIndex = adapter->securitypriv.PMKIDIndex;
		backupTKIPCountermeasure = adapter->securitypriv.btkip_countermeasure;
		backupTKIPcountermeasure_time = adapter->securitypriv.btkip_countermeasure_time;

		memset((unsigned char *)&adapter->securitypriv, 0, sizeof (struct security_priv));
		// Added by Albert 2009/02/18
		// Restore the PMK information to securitypriv structure for the following connection.
		memcpy(&adapter->securitypriv.PMKIDList[ 0 ], &backupPMKIDList[ 0 ], sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
		adapter->securitypriv.PMKIDIndex = backupPMKIDIndex;
		adapter->securitypriv.btkip_countermeasure = backupTKIPCountermeasure;
		adapter->securitypriv.btkip_countermeasure_time = backupTKIPcountermeasure_time;

		adapter->securitypriv.ndisauthtype = Ndis802_11AuthModeOpen;
		adapter->securitypriv.ndisencryptstatus = Ndis802_11WEPDisabled;

	}
	else //reset values in securitypriv
	{
		struct security_priv *psec_priv=&adapter->securitypriv;

		psec_priv->dot11AuthAlgrthm =dot11AuthAlgrthm_Open;  //open system
		psec_priv->dot11PrivacyAlgrthm = _NO_PRIVACY_;
		psec_priv->dot11PrivacyKeyIndex = 0;

		psec_priv->dot118021XGrpPrivacy = _NO_PRIVACY_;
		psec_priv->dot118021XGrpKeyid = 1;

		psec_priv->ndisauthtype = Ndis802_11AuthModeOpen;
		psec_priv->ndisencryptstatus = Ndis802_11WEPDisabled;
	}
}

void rtw_os_indicate_disconnect(struct rtw_adapter *adapter)
{
   //RT_PMKID_LIST   backupPMKIDList[ NUM_PMKID_CACHE ];

_func_enter_;

	netif_carrier_off(adapter->pnetdev); // Do it first for tx broadcast pkt after disconnection issue!

	rtw_cfg80211_indicate_disconnect(adapter);

	rtw_reset_securitypriv( adapter );

_func_exit_;
}

void rtw_report_sec_ie(struct rtw_adapter *adapter,u8 authmode,u8 *sec_ie)
{
	uint	len;
	u8	*buff,*p,i;
	union iwreq_data wrqu;

_func_enter_;

	RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("+rtw_report_sec_ie, authmode=%d\n", authmode));

	buff = NULL;
	if(authmode==_WPA_IE_ID_)
	{
		RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("rtw_report_sec_ie, authmode=%d\n", authmode));

		buff = kzalloc(IW_CUSTOM_MAX, GFP_KERNEL);
if (!buff)
			goto out;
		p=buff;

		p+=sprintf(p,"ASSOCINFO(ReqIEs=");

		len = sec_ie[1]+2;
		len =  (len < IW_CUSTOM_MAX) ? len:IW_CUSTOM_MAX;

		for(i=0;i<len;i++){
			p+=sprintf(p,"%02x",sec_ie[i]);
		}

		p+=sprintf(p,")");

		memset(&wrqu, 0, sizeof(wrqu));

		wrqu.data.length=p-buff;

		wrqu.data.length = (wrqu.data.length<IW_CUSTOM_MAX) ? wrqu.data.length:IW_CUSTOM_MAX;

		kfree(buff);
	}
out:

_func_exit_;

}


#ifdef CONFIG_8723AU_AP_MODE

void rtw_indicate_sta_assoc_event(struct rtw_adapter *padapter, struct sta_info *psta)
{
	union iwreq_data wrqu;
	struct sta_priv *pstapriv = &padapter->stapriv;

	if(psta==NULL)
		return;

	if(psta->aid > NUM_STA)
		return;

	if(pstapriv->sta_aid[psta->aid - 1] != psta)
		return;


	wrqu.addr.sa_family = ARPHRD_ETHER;

	memcpy(wrqu.addr.sa_data, psta->hwaddr, ETH_ALEN);

	DBG_8723A("+rtw_indicate_sta_assoc_event\n");
}

void rtw_indicate_sta_disassoc_event(struct rtw_adapter *padapter, struct sta_info *psta)
{
	union iwreq_data wrqu;
	struct sta_priv *pstapriv = &padapter->stapriv;

	if(psta==NULL)
		return;

	if(psta->aid > NUM_STA)
		return;

	if(pstapriv->sta_aid[psta->aid - 1] != psta)
		return;


	wrqu.addr.sa_family = ARPHRD_ETHER;

	memcpy(wrqu.addr.sa_data, psta->hwaddr, ETH_ALEN);

	DBG_8723A("+rtw_indicate_sta_disassoc_event\n");
}


#ifdef CONFIG_HOSTAPD_MLME

static int mgnt_xmit_entry(struct sk_buff *skb, struct net_device *pnetdev)
{
	struct hostapd_priv *phostapdpriv = netdev_priv(pnetdev);
	struct rtw_adapter *padapter = (struct rtw_adapter *)phostapdpriv->padapter;

	//DBG_8723A("%s\n", __FUNCTION__);

	return rtw_hal_hostap_mgnt_xmit_entry(padapter, skb);
}

static int mgnt_netdev_open(struct net_device *pnetdev)
{
	struct hostapd_priv *phostapdpriv = netdev_priv(pnetdev);

	DBG_8723A("mgnt_netdev_open: MAC Address:" MAC_FMT "\n", MAC_ARG(pnetdev->dev_addr));


	init_usb_anchor(&phostapdpriv->anchored);

	if(!rtw_netif_queue_stopped(pnetdev))
		rtw_netif_start_queue(pnetdev);
	else
		rtw_netif_wake_queue(pnetdev);


	netif_carrier_on(pnetdev);

	//rtw_write16(phostapdpriv->padapter, 0x0116, 0x0100);//only excluding beacon

	return 0;
}
static int mgnt_netdev_close(struct net_device *pnetdev)
{
	struct hostapd_priv *phostapdpriv = netdev_priv(pnetdev);

	DBG_8723A("%s\n", __FUNCTION__);

	usb_kill_anchored_urbs(&phostapdpriv->anchored);

	netif_carrier_off(pnetdev);

	if (!rtw_netif_queue_stopped(pnetdev))
		rtw_netif_stop_queue(pnetdev);

	//rtw_write16(phostapdpriv->padapter, 0x0116, 0x3f3f);

	return 0;
}

static const struct net_device_ops rtl871x_mgnt_netdev_ops = {
	.ndo_open = mgnt_netdev_open,
	.ndo_stop = mgnt_netdev_close,
	.ndo_start_xmit = mgnt_xmit_entry,
	//.ndo_set_mac_address = r871x_net_set_mac_address,
	//.ndo_get_stats = r871x_net_get_stats,
	//.ndo_do_ioctl = r871x_mp_ioctl,
};

int hostapd_mode_init(struct rtw_adapter *padapter)
{
	unsigned char mac[ETH_ALEN];
	struct hostapd_priv *phostapdpriv;
	struct net_device *pnetdev;

	pnetdev = alloc_etherdev(sizeof(struct hostapd_priv));
	if (!pnetdev)
		return -ENOMEM;

	//SET_MODULE_OWNER(pnetdev);
	ether_setup(pnetdev);

	//pnetdev->type = ARPHRD_IEEE80211;

	phostapdpriv = netdev_priv(pnetdev);
	phostapdpriv->pmgnt_netdev = pnetdev;
	phostapdpriv->padapter= padapter;
	padapter->phostapdpriv = phostapdpriv;

	//pnetdev->init = NULL;

	DBG_8723A("register rtl871x_mgnt_netdev_ops to netdev_ops\n");

	pnetdev->netdev_ops = &rtl871x_mgnt_netdev_ops;

	pnetdev->watchdog_timeo = HZ; /* 1 second timeout */

	//pnetdev->wireless_handlers = NULL;

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	pnetdev->features |= NETIF_F_IP_CSUM;
#endif

	if(dev_alloc_name(pnetdev,"mgnt.wlan%d") < 0)
	{
		DBG_8723A("hostapd_mode_init(): dev_alloc_name, fail! \n");
	}

	//SET_NETDEV_DEV(pnetdev, pintfpriv->udev);

	mac[0]=0x00;
	mac[1]=0xe0;
	mac[2]=0x4c;
	mac[3]=0x87;
	mac[4]=0x11;
	mac[5]=0x12;

	memcpy(pnetdev->dev_addr, mac, ETH_ALEN);

	netif_carrier_off(pnetdev);

	/* Tell the network stack we exist */
	if (register_netdev(pnetdev) != 0)
	{
		DBG_8723A("hostapd_mode_init(): register_netdev fail!\n");

		if (pnetdev)
		{
			free_netdev(pnetdev);
		}
	}

	return 0;

}

void hostapd_mode_unload(struct rtw_adapter *padapter)
{
	struct hostapd_priv *phostapdpriv = padapter->phostapdpriv;
	struct net_device *pnetdev = phostapdpriv->pmgnt_netdev;

	unregister_netdev(pnetdev);
	free_netdev(pnetdev);

}

#endif
#endif
