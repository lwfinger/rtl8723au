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
#ifndef __RTW_MLME_H_
#define __RTW_MLME_H_

#include <drv_conf.h>
#include <osdep_service.h>
#include <mlme_osdep.h>
#include <drv_types.h>
#include <wlan_bssdef.h>

#define	MAX_BSS_CNT	128
//#define   MAX_JOIN_TIMEOUT	2000
//#define   MAX_JOIN_TIMEOUT	2500
#define   MAX_JOIN_TIMEOUT	6500

//	Commented by Albert 20101105
//	Increase the scanning timeout because of increasing the SURVEY_TO value.

#define		SCANNING_TIMEOUT	8000

#define	SCAN_INTERVAL	(30) // unit:2sec, 30*2=60sec

#ifdef PALTFORM_OS_WINCE
#define	SCANQUEUE_LIFETIME 12000000 // unit:us
#else
#define	SCANQUEUE_LIFETIME 20 // unit:sec
#endif

#define		WIFI_NULL_STATE			0x00000000

#define	WIFI_ASOC_STATE			0x00000001		// Under Linked state...
#define		WIFI_REASOC_STATE		0x00000002
#define	WIFI_SLEEP_STATE			0x00000004
#define	WIFI_STATION_STATE		0x00000008

#define	WIFI_AP_STATE				0x00000010
#define	WIFI_ADHOC_STATE			0x00000020
#define   WIFI_ADHOC_MASTER_STATE 0x00000040
#define   WIFI_UNDER_LINKING		0x00000080

#define	WIFI_UNDER_WPS			0x00000100
//#define	WIFI_UNDER_CMD			0x00000200
//#define	WIFI_UNDER_P2P			0x00000400
#define	WIFI_STA_ALIVE_CHK_STATE	0x00000400
#define	WIFI_SITE_MONITOR			0x00000800		//to indicate the station is under site surveying

#ifdef WDS
#define	WIFI_WDS					0x00001000
#define	WIFI_WDS_RX_BEACON		0x00002000		// already rx WDS AP beacon
#endif
#ifdef AUTO_CONFIG
#define	WIFI_AUTOCONF				0x00004000
#define	WIFI_AUTOCONF_IND		0x00008000
#endif

/*
// ========== P2P Section Start ===============
#define	WIFI_P2P_LISTEN_STATE		0x00010000
#define	WIFI_P2P_GROUP_FORMATION_STATE		0x00020000
// ========== P2P Section End ===============
*/

//#ifdef UNDER_MPTEST
#define	WIFI_MP_STATE							0x00010000
#define	WIFI_MP_CTX_BACKGROUND				0x00020000	// in continous tx background
#define	WIFI_MP_CTX_ST						0x00040000	// in continous tx with single-tone
#define	WIFI_MP_CTX_BACKGROUND_PENDING	0x00080000	// pending in continous tx background due to out of skb
#define	WIFI_MP_CTX_CCK_HW					0x00100000	// in continous tx
#define	WIFI_MP_CTX_CCK_CS					0x00200000	// in continous tx with carrier suppression
#define   WIFI_MP_LPBK_STATE					0x00400000
//#endif

//#define _FW_UNDER_CMD		WIFI_UNDER_CMD
#define _FW_UNDER_LINKING	WIFI_UNDER_LINKING
#define _FW_LINKED			WIFI_ASOC_STATE
#define _FW_UNDER_SURVEY	WIFI_SITE_MONITOR


enum dot11AuthAlgrthmNum {
 dot11AuthAlgrthm_Open = 0,
 dot11AuthAlgrthm_Shared,
 dot11AuthAlgrthm_8021X,
 dot11AuthAlgrthm_Auto,
 dot11AuthAlgrthm_MaxNum
};

// Scan type including active and passive scan.
typedef enum _RT_SCAN_TYPE
{
	SCAN_PASSIVE,
	SCAN_ACTIVE,
	SCAN_MIX,
}RT_SCAN_TYPE, *PRT_SCAN_TYPE;

enum  _BAND
{
	GHZ24_50 = 0,
	GHZ_50,
	GHZ_24,
};

enum SCAN_RESULT_TYPE
{
	SCAN_RESULT_P2P_ONLY = 0,		//	Will return all the P2P devices.
	SCAN_RESULT_ALL = 1,			//	Will return all the scanned device, include AP.
	SCAN_RESULT_WFD_TYPE = 2		//	Will just return the correct WFD device.
									//	If this device is Miracast sink device, it will just return all the Miracast source devices.
};

/*

there are several "locks" in mlme_priv,
since mlme_priv is a shared resource between many threads,
like ISR/Call-Back functions, the OID handlers, and even timer functions.


Each _queue has its own locks, already.
Other items are protected by mlme_priv.lock.

To avoid possible dead lock, any thread trying to modifiying mlme_priv
SHALL not lock up more than one locks at a time!

*/


#define traffic_threshold	10
#define	traffic_scan_period	500

struct sitesurvey_ctrl {
	u64	last_tx_pkts;
	uint	last_rx_pkts;
	int	traffic_busy;
	struct timer_list	sitesurvey_ctrl_timer;
};

typedef struct _RT_LINK_DETECT_T{
	u32				NumTxOkInPeriod;
	u32				NumRxOkInPeriod;
	u32				NumRxUnicastOkInPeriod;
	bool			bBusyTraffic;
	bool			bTxBusyTraffic;
	bool			bRxBusyTraffic;
	bool			bHigherBusyTraffic; // For interrupt migration purpose.
	bool			bHigherBusyRxTraffic; // We may disable Tx interrupt according as Rx traffic.
	bool			bHigherBusyTxTraffic; // We may disable Tx interrupt according as Tx traffic.
}RT_LINK_DETECT_T, *PRT_LINK_DETECT_T;

struct profile_info {
	u8	ssidlen;
	u8	ssid[ WLAN_SSID_MAXLEN ];
	u8	peermac[ ETH_ALEN ];
};

struct tx_invite_req_info{
	u8					token;
	u8					benable;
	u8					go_ssid[ WLAN_SSID_MAXLEN ];
	u8					ssidlen;
	u8					go_bssid[ ETH_ALEN ];
	u8					peer_macaddr[ ETH_ALEN ];
	u8					operating_ch;	//	This information will be set by using the p2p_set op_ch=x
	u8					peer_ch;		//	The listen channel for peer P2P device

};

struct tx_invite_resp_info{
	u8					token;	//	Used to record the dialog token of p2p invitation request frame.
};

#ifdef CONFIG_WFD

struct wifi_display_info{
	u16							wfd_enable;			//	Eanble/Disable the WFD function.
	u16							rtsp_ctrlport;		//	TCP port number at which the this WFD device listens for RTSP messages
	u16							peer_rtsp_ctrlport;	//	TCP port number at which the peer WFD device listens for RTSP messages
													//	This filed should be filled when receiving the gropu negotiation request

	u8							peer_session_avail;	//	WFD session is available or not for the peer wfd device.
													//	This variable will be set when sending the provisioning discovery request to peer WFD device.
													//	And this variable will be reset when it is read by using the iwpriv p2p_get wfd_sa command.
	u8							ip_address[4];
	u8							peer_ip_address[4];
	u8							wfd_pc;				//	WFD preferred connection
													//	0 -> Prefer to use the P2P for WFD connection on peer side.
													//	1 -> Prefer to use the TDLS for WFD connection on peer side.

	u8							wfd_device_type;	//	WFD Device Type
													//	0 -> WFD Source Device
													//	1 -> WFD Primary Sink Device
	enum	SCAN_RESULT_TYPE	scan_result_type;	//	Used when P2P is enable. This parameter will impact the scan result.
};
#endif //CONFIG_WFD

struct tx_provdisc_req_info{
	u16					wps_config_method_request;	//	Used when sending the provisioning request frame
	u16					peer_channel_num[2];		//	The channel number which the receiver stands.
	NDIS_802_11_SSID	ssid;
	u8					peerDevAddr[ ETH_ALEN ];		//	Peer device address
	u8					peerIFAddr[ ETH_ALEN ];		//	Peer interface address
	u8					benable;					//	This provision discovery request frame is trigger to send or not
};

struct rx_provdisc_req_info{	//When peer device issue prov_disc_req first, we should store the following informations
	u8					peerDevAddr[ ETH_ALEN ];		//	Peer device address
	u8					strconfig_method_desc_of_prov_disc_req[4];	//	description for the config method located in the provisioning discovery request frame.
																	//	The UI must know this information to know which config method the remote p2p device is requiring.
};

struct tx_nego_req_info{
	u16					peer_channel_num[2];		//	The channel number which the receiver stands.
	u8					peerDevAddr[ ETH_ALEN ];		//	Peer device address
	u8					benable;					//	This negoitation request frame is trigger to send or not
};

struct group_id_info{
	u8					go_device_addr[ ETH_ALEN ];	//	The GO's device address of this P2P group
	u8					ssid[ WLAN_SSID_MAXLEN ];	//	The SSID of this P2P group
};

struct scan_limit_info{
	u8					scan_op_ch_only;			//	When this flag is set, the driver should just scan the operation channel
	u8					operation_ch[2];				//	Store the operation channel of invitation request frame
};

struct cfg80211_wifidirect_info {
	struct timer_list		remain_on_ch_timer;
	u8				restore_channel;
	struct ieee80211_channel	remain_on_ch_channel;
	enum nl80211_channel_type	remain_on_ch_type;
	u64				remain_on_ch_cookie;
	bool is_ro_ch;
};

struct wifidirect_info{
	struct rtw_adapter*				padapter;
	struct timer_list		find_phase_timer;
	struct timer_list		restore_p2p_state_timer;

	//	Used to do the scanning. After confirming the peer is availalble, the driver transmits the P2P frame to peer.
	struct timer_list		pre_tx_scan_timer;
	struct timer_list		reset_ch_sitesurvey;
	struct timer_list		reset_ch_sitesurvey2;	//	Just for resetting the scan limit function by using p2p nego
	struct tx_provdisc_req_info	tx_prov_disc_info;
	struct rx_provdisc_req_info rx_prov_disc_info;
	struct tx_invite_req_info	invitereq_info;
	struct profile_info			profileinfo[ P2P_MAX_PERSISTENT_GROUP_NUM ];	//	Store the profile information of persistent group
	struct tx_invite_resp_info	inviteresp_info;
	struct tx_nego_req_info	nego_req_info;
	struct group_id_info		groupid_info;	//	Store the group id information when doing the group negotiation handshake.
	struct scan_limit_info		rx_invitereq_info;	//	Used for get the limit scan channel from the Invitation procedure
	struct scan_limit_info		p2p_info;		//	Used for get the limit scan channel from the P2P negotiation handshake
#ifdef CONFIG_WFD
	struct wifi_display_info		*wfd_info;
#endif
	enum P2P_ROLE			role;
	enum P2P_STATE			pre_p2p_state;
	enum P2P_STATE			p2p_state;
	u8						device_addr[ETH_ALEN];	//	The device address should be the mac address of this device.
	u8						interface_addr[ETH_ALEN];
	u8						social_chan[4];
	u8						listen_channel;
	u8						operating_channel;
	u8						listen_dwell;		//	This value should be between 1 and 3
	u8						support_rate[8];
	u8						p2p_wildcard_ssid[P2P_WILDCARD_SSID_LEN];
	u8						intent;		//	should only include the intent value.
	u8						p2p_peer_interface_addr[ ETH_ALEN ];
	u8						p2p_peer_device_addr[ ETH_ALEN ];
	u8						peer_intent;	//	Included the intent value and tie breaker value.
	u8						device_name[ WPS_MAX_DEVICE_NAME_LEN ];	//	Device name for displaying on searching device screen
	u8						device_name_len;
	u8						profileindex;	//	Used to point to the index of profileinfo array
	u8						peer_operating_ch;
	u8						find_phase_state_exchange_cnt;
	u16						device_password_id_for_nego;	//	The device password ID for group negotation
	u8						negotiation_dialog_token;
	u8						nego_ssid[ WLAN_SSID_MAXLEN ];	//	SSID information for group negotitation
	u8						nego_ssidlen;
	u8						p2p_group_ssid[WLAN_SSID_MAXLEN];
	u8						p2p_group_ssid_len;
	u8						persistent_supported;		//	Flag to know the persistent function should be supported or not.
														//	In the Sigma test, the Sigma will provide this enable from the sta_set_p2p CAPI.
														//	0: disable
														//	1: enable
	u8						session_available;			//	Flag to set the WFD session available to enable or disable "by Sigma"
														//	In the Sigma test, the Sigma will disable the session available by using the sta_preset CAPI.
														//	0: disable
														//	1: enable

	u8						wfd_tdls_enable;			//	Flag to enable or disable the TDLS by WFD Sigma
														//	0: disable
														//	1: enable
	u8						wfd_tdls_weaksec;			//	Flag to enable or disable the weak security function for TDLS by WFD Sigma
														//	0: disable
														//	In this case, the driver can't issue the tdsl setup request frame.
														//	1: enable
														//	In this case, the driver can issue the tdls setup request frame
														//	even the current security is weak security.

	enum	P2P_WPSINFO		ui_got_wps_info;			//	This field will store the WPS value (PIN value or PBC) that UI had got from the user.
	u16						supported_wps_cm;			//	This field describes the WPS config method which this driver supported.
														//	The value should be the combination of config method defined in page104 of WPS v2.0 spec.
	uint						channel_list_attr_len;		//	This field will contain the length of body of P2P Channel List attribute of group negotitation response frame.
	u8						channel_list_attr[100];		//	This field will contain the body of P2P Channel List attribute of group negotitation response frame.
														//	We will use the channel_cnt and channel_list fields when constructing the group negotitation confirm frame.
#ifdef CONFIG_8723AU_P2P_PS
	enum P2P_PS_MODE		p2p_ps_mode; // indicate p2p ps mode
	enum P2P_PS_STATE		p2p_ps_state; // indicate p2p ps state
	u8						noa_index; // Identifies and instance of Notice of Absence timing.
	u8						ctwindow; // Client traffic window. A period of time in TU after TBTT.
	u8						opp_ps; // opportunistic power save.
	u8						noa_num; // number of NoA descriptor in P2P IE.
	u8						noa_count[P2P_MAX_NOA_NUM]; // Count for owner, Type of client.
	u32						noa_duration[P2P_MAX_NOA_NUM]; // Max duration for owner, preferred or min acceptable duration for client.
	u32						noa_interval[P2P_MAX_NOA_NUM]; // Length of interval for owner, preferred or max acceptable interval of client.
	u32						noa_start_time[P2P_MAX_NOA_NUM]; // schedule expressed in terms of the lower 4 bytes of the TSF timer.
#endif // CONFIG_8723AU_P2P_PS
};

struct tdls_ss_record{	//signal strength record
	u8		macaddr[ETH_ALEN];
	u8		RxPWDBAll;
	u8		is_tdls_sta;	// true: direct link sta, false: else
};

struct tdls_info{
	u8					ap_prohibited;
	uint					setup_state;
	u8					sta_cnt;
	u8					sta_maximum;	// 1:tdls sta is equal (NUM_STA-1), reach max direct link number; 0: else;
	struct tdls_ss_record	ss_record;
	u8					macid_index;	//macid entry that is ready to write
	u8					clear_cam;	//cam entry that is trying to clear, using it in direct link teardown
	u8					ch_sensing;
	u8					cur_channel;
	u8					candidate_ch;
	u8					collect_pkt_num[MAX_CHANNEL_NUM];
	spinlock_t				cmd_lock;
	spinlock_t				hdl_lock;
	u8					watchdog_count;
	u8					dev_discovered;		//WFD_TDLS: for sigma test
	u8					enable;
#ifdef CONFIG_WFD
	struct wifi_display_info		*wfd_info;
#endif
};

struct mlme_priv {

	spinlock_t	lock;
	int	fw_state;	//shall we protect this variable? maybe not necessarily...
	u8 bScanInProcess;
	u8	to_join; //flag
	u8 to_roaming; // roaming trying times

	u8	*nic_hdl;

	u8	not_indic_disco;
	_queue	scanned_queue;

	NDIS_802_11_SSID	assoc_ssid;
	u8	assoc_bssid[6];

	struct wlan_network	cur_network;

	//uint wireless_mode; no used, remove it

	u32	scan_interval;

	struct timer_list assoc_timer;

	uint assoc_by_bssid;
	uint assoc_by_rssi;

	struct timer_list scan_to_timer; // driver itself handles scan_timeout status.
	u32 scan_start_time; // used to evaluate the time spent in scanning

	#ifdef CONFIG_SET_SCAN_DENY_TIMER
	struct timer_list set_scan_deny_timer;
	atomic_t set_scan_deny; //0: allowed, 1: deny
	#endif

	struct qos_priv qospriv;

#ifdef CONFIG_80211N_HT

	/* Number of non-HT AP/stations */
	int num_sta_no_ht;

	/* Number of HT AP/stations 20 MHz */
	//int num_sta_ht_20mhz;


	int num_FortyMHzIntolerant;

	struct ht_priv	htpriv;

#endif

	RT_LINK_DETECT_T	LinkDetectInfo;
	struct timer_list	dynamic_chk_timer; //dynamic/periodic check timer

	u8	key_mask; //use for ips to set wep key after ips_leave
	u8	acm_mask; // for wmm acm mask
	u8	ChannelPlan;
	RT_SCAN_TYPE	scan_mode; // active: 1, passive: 0

	//u8 probereq_wpsie[MAX_WPS_IE_LEN];//added in probe req
	//int probereq_wpsie_len;
	u8 *wps_probe_req_ie;
	u32 wps_probe_req_ie_len;

#if defined (CONFIG_8723AU_AP_MODE) && defined (CONFIG_NATIVEAP_MLME)
	/* Number of associated Non-ERP stations (i.e., stations using 802.11b
	 * in 802.11g BSS) */
	int num_sta_non_erp;

	/* Number of associated stations that do not support Short Slot Time */
	int num_sta_no_short_slot_time;

	/* Number of associated stations that do not support Short Preamble */
	int num_sta_no_short_preamble;

	int olbc; /* Overlapping Legacy BSS Condition */

	/* Number of HT associated stations that do not support greenfield */
	int num_sta_ht_no_gf;

	/* Number of associated non-HT stations */
	//int num_sta_no_ht;

	/* Number of HT associated stations 20 MHz */
	int num_sta_ht_20mhz;

	/* Overlapping BSS information */
	int olbc_ht;

#ifdef CONFIG_80211N_HT
	u16 ht_op_mode;
#endif /* CONFIG_80211N_HT */

	u8 *assoc_req;
	u32 assoc_req_len;
	u8 *assoc_rsp;
	u32 assoc_rsp_len;

	u8 *wps_beacon_ie;
	//u8 *wps_probe_req_ie;
	u8 *wps_probe_resp_ie;
	u8 *wps_assoc_resp_ie; // for CONFIG_IOCTL_CFG80211, this IE could include p2p ie / wfd ie

	u32 wps_beacon_ie_len;
	//u32 wps_probe_req_ie_len;
	u32 wps_probe_resp_ie_len;
	u32 wps_assoc_resp_ie_len; // for CONFIG_IOCTL_CFG80211, this IE len could include p2p ie / wfd ie

	u8 *p2p_beacon_ie;
	u8 *p2p_probe_req_ie;
	u8 *p2p_probe_resp_ie;
	u8 *p2p_go_probe_resp_ie; //for GO
	u8 *p2p_assoc_req_ie;

	u32 p2p_beacon_ie_len;
	u32 p2p_probe_req_ie_len;
	u32 p2p_probe_resp_ie_len;
	u32 p2p_go_probe_resp_ie_len; //for GO
	u32 p2p_assoc_req_ie_len;
/*
#if defined(CONFIG_8723AU_P2P)
	//u8 *wps_p2p_beacon_ie;
	u8 *p2p_beacon_ie;
	u8 *wps_p2p_probe_resp_ie;
	u8 *wps_p2p_assoc_resp_ie;
	//u32 wps_p2p_beacon_ie_len;
	u32 p2p_beacon_ie_len;
	u32 wps_p2p_probe_resp_ie_len;
	u32 wps_p2p_assoc_resp_ie_len;
#endif
*/

	spinlock_t	bcn_update_lock;
	u8		update_bcn;


#endif //#if defined (CONFIG_8723AU_AP_MODE) && defined (CONFIG_NATIVEAP_MLME)

#if defined(CONFIG_WFD)

	u8 *wfd_beacon_ie;
	u8 *wfd_probe_req_ie;
	u8 *wfd_probe_resp_ie;
	u8 *wfd_go_probe_resp_ie; //for GO
	u8 *wfd_assoc_req_ie;

	u32 wfd_beacon_ie_len;
	u32 wfd_probe_req_ie_len;
	u32 wfd_probe_resp_ie_len;
	u32 wfd_go_probe_resp_ie_len; //for GO
	u32 wfd_assoc_req_ie_len;

#endif

#ifdef CONFIG_FTP_PROTECT
	u8	ftp_lock_flag;
#endif //CONFIG_FTP_PROTECT
};

#ifdef CONFIG_8723AU_AP_MODE

struct hostapd_priv
{
	struct rtw_adapter *padapter;

#ifdef CONFIG_HOSTAPD_MLME
	struct net_device *pmgnt_netdev;
	struct usb_anchor anchored;
#endif

};

int hostapd_mode_init(struct rtw_adapter *padapter);
void hostapd_mode_unload(struct rtw_adapter *padapter);
#endif


void rtw_joinbss_event_prehandle(struct rtw_adapter *adapter, u8 *pbuf);
void rtw_survey_event_callback(struct rtw_adapter *adapter, u8 *pbuf);
void rtw_surveydone_event_callback(struct rtw_adapter *adapter, u8 *pbuf);
void rtw_joinbss_event_callback(struct rtw_adapter *adapter, u8 *pbuf);
void rtw_stassoc_event_callback(struct rtw_adapter *adapter, u8 *pbuf);
void rtw_stadel_event_callback(struct rtw_adapter *adapter, u8 *pbuf);
void rtw_atimdone_event_callback(struct rtw_adapter *adapter, u8 *pbuf);
void rtw_cpwm_event_callback(struct rtw_adapter *adapter, u8 *pbuf);


int event_thread(void *context);
void rtw_join_timeout_handler(unsigned long);

void rtw_free_network_queue(struct rtw_adapter *adapter,u8 isfreeall);
int rtw_init_mlme_priv(struct rtw_adapter *adapter);// (struct mlme_priv *pmlmepriv);

void rtw_free_mlme_priv (struct mlme_priv *pmlmepriv);

int rtw_select_and_join_from_scanned_queue(struct mlme_priv *pmlmepriv);
int rtw_set_key(struct rtw_adapter *adapter,struct security_priv *psecuritypriv,int keyid, u8 set_tx);
int rtw_set_auth(struct rtw_adapter *adapter,struct security_priv *psecuritypriv);

static inline u8 *get_bssid(struct mlme_priv *pmlmepriv)
{	//if sta_mode:pmlmepriv->cur_network.network.MacAddress=> bssid
	// if adhoc_mode:pmlmepriv->cur_network.network.MacAddress=> ibss mac address
	return pmlmepriv->cur_network.network.MacAddress;
}

static inline int check_fwstate(struct mlme_priv *pmlmepriv, int state)
{
	if (pmlmepriv->fw_state & state)
		return true;

	return false;
}

static inline int get_fwstate(struct mlme_priv *pmlmepriv)
{
	return pmlmepriv->fw_state;
}

/*
 * No Limit on the calling context,
 * therefore set it to be the critical section...
 *
 * ### NOTE:#### (!!!!)
 * MUST TAKE CARE THAT BEFORE CALLING THIS FUNC, YOU SHOULD HAVE LOCKED pmlmepriv->lock
 */
static inline void set_fwstate(struct mlme_priv *pmlmepriv, int state)
{
	pmlmepriv->fw_state |= state;
	//FOR HW integration
	if(_FW_UNDER_SURVEY==state){
		pmlmepriv->bScanInProcess = true;
	}
}

static inline void _clr_fwstate_(struct mlme_priv *pmlmepriv, int state)
{
	pmlmepriv->fw_state &= ~state;
	//FOR HW integration
	if(_FW_UNDER_SURVEY==state){
		pmlmepriv->bScanInProcess = false;
	}
}

/*
 * No Limit on the calling context,
 * therefore set it to be the critical section...
 */
static inline void clr_fwstate(struct mlme_priv *pmlmepriv, int state)
{
	spin_lock_bh(&pmlmepriv->lock);
	if (check_fwstate(pmlmepriv, state) == true)
		pmlmepriv->fw_state ^= state;
	spin_unlock_bh(&pmlmepriv->lock);
}

static inline void clr_fwstate_ex(struct mlme_priv *pmlmepriv, int state)
{
	spin_lock_bh(&pmlmepriv->lock);
	_clr_fwstate_(pmlmepriv, state);
	spin_unlock_bh(&pmlmepriv->lock);
}

u16 rtw_get_capability(WLAN_BSSID_EX *bss);
void rtw_update_scanned_network(struct rtw_adapter *adapter, WLAN_BSSID_EX *target);
void rtw_disconnect_hdl_under_linked(struct rtw_adapter* adapter, struct sta_info *psta, u8 free_assoc);
void rtw_generate_random_ibss(u8 *pibss);
struct wlan_network* rtw_find_network(_queue *scanned_queue, u8 *addr);
struct wlan_network* rtw_get_oldest_wlan_network(_queue *scanned_queue);

void rtw_free_assoc_resources(struct rtw_adapter *adapter, int lock_scanned_queue);
void rtw_indicate_disconnect(struct rtw_adapter* adapter);
void rtw_indicate_connect(struct rtw_adapter* adapter);
void rtw_indicate_scan_done(struct rtw_adapter *padapter, bool aborted);
void rtw_scan_abort(struct rtw_adapter *adapter);

int rtw_restruct_sec_ie(struct rtw_adapter *adapter,u8 *in_ie,u8 *out_ie,uint in_len);
int rtw_restruct_wmm_ie(struct rtw_adapter *adapter, u8 *in_ie, u8 *out_ie, uint in_len, uint initial_out_len);
void rtw_init_registrypriv_dev_network(struct rtw_adapter *adapter);

void rtw_update_registrypriv_dev_network(struct rtw_adapter *adapter);

void rtw_get_encrypt_decrypt_from_registrypriv(struct rtw_adapter *adapter);

void rtw_scan_timeout_handler(unsigned long data);

void rtw_dynamic_check_timer_handler(unsigned long data);
#ifdef CONFIG_SET_SCAN_DENY_TIMER
bool rtw_is_scan_deny(struct rtw_adapter *adapter);
void rtw_clear_scan_deny(struct rtw_adapter *adapter);
void rtw_set_scan_deny_timer_hdl(unsigned long data);
void rtw_set_scan_deny(struct rtw_adapter *adapter, u32 ms);
#else
#define rtw_is_scan_deny(adapter) false
#define rtw_clear_scan_deny(adapter) do {} while (0)
#define rtw_set_scan_deny_timer_hdl(adapter) do {} while (0)
#define rtw_set_scan_deny(adapter, ms) do {} while (0)
#endif


int _rtw_init_mlme_priv(struct rtw_adapter *padapter);

void rtw_free_mlme_priv_ie_data(struct mlme_priv *pmlmepriv);

void _rtw_free_mlme_priv(struct mlme_priv *pmlmepriv);

struct wlan_network *rtw_alloc_network(struct mlme_priv *pmlmepriv);

void _rtw_free_network(struct mlme_priv *pmlmepriv, struct wlan_network *pnetwork, u8 isfreeall);
void _rtw_free_network_nolock(struct mlme_priv *pmlmepriv, struct wlan_network *pnetwork);

struct wlan_network* _rtw_find_network(_queue *scanned_queue, u8 *addr);

void _rtw_free_network_queue(struct rtw_adapter* padapter, u8 isfreeall);

int rtw_if_up(struct rtw_adapter *padapter);

int rtw_linked_check(struct rtw_adapter *padapter);

u8 *rtw_get_capability_from_ie(u8 *ie);
u8 *rtw_get_timestampe_from_ie(u8 *ie);
u8 *rtw_get_beacon_interval_from_ie(u8 *ie);


void rtw_joinbss_reset(struct rtw_adapter *padapter);

#ifdef CONFIG_80211N_HT
unsigned int rtw_restructure_ht_ie(struct rtw_adapter *padapter, u8 *in_ie, u8 *out_ie, uint in_len, uint *pout_len);
void rtw_update_ht_cap(struct rtw_adapter *padapter, u8 *pie, uint ie_len);
void rtw_issue_addbareq_cmd(struct rtw_adapter *padapter, struct xmit_frame *pxmitframe);
#endif

int rtw_is_same_ibss(struct rtw_adapter *adapter, struct wlan_network *pnetwork);
int is_same_network(WLAN_BSSID_EX *src, WLAN_BSSID_EX *dst);

void _rtw_roaming(struct rtw_adapter *adapter, struct wlan_network *tgt_network);
void rtw_roaming(struct rtw_adapter *adapter, struct wlan_network *tgt_network);
void rtw_set_roaming(struct rtw_adapter *adapter, u8 to_roaming);
u8 rtw_to_roaming(struct rtw_adapter *adapter);
void rtw_stassoc_hw_rpt(struct rtw_adapter *adapter,struct sta_info *psta);

#endif //__RTL871X_MLME_H_
