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
#define _HCI_INTF_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <xmit_osdep.h>
#include <hal_intf.h>
#include <rtw_version.h>

#include <usb_vendor_req.h>
#include <usb_ops.h>
#include <usb_osintf.h>
#include <usb_hal.h>

#ifdef CONFIG_80211N_HT
extern int rtw_ht_enable;
extern int rtw_cbw40_enable;
extern int rtw_ampdu_enable;//for enable tx_ampdu
#endif

#ifdef CONFIG_GLOBAL_UI_PID
int ui_pid[3] = {0, 0, 0};
#endif


extern int pm_netdev_open(struct net_device *pnetdev,u8 bnormal);
static int rtw_suspend(struct usb_interface *intf, pm_message_t message);
static int rtw_resume(struct usb_interface *intf);
int rtw_resume_process(struct rtw_adapter *padapter);


static int rtw_drv_init(struct usb_interface *pusb_intf,const struct usb_device_id *pdid);
static void rtw_disconnect(struct usb_interface *pusb_intf);


#define USB_VENDER_ID_REALTEK		0x0BDA

#define RTL8723A_USB_IDS \
	{USB_DEVICE_AND_INTERFACE_INFO(USB_VENDER_ID_REALTEK, 0x8724,0xff,0xff,0xff)}, /* 8723AU 1*1 */ \
	{USB_DEVICE_AND_INTERFACE_INFO(USB_VENDER_ID_REALTEK, 0x1724,0xff,0xff,0xff)}, /* 8723AU 1*1 */ \
	{USB_DEVICE_AND_INTERFACE_INFO(USB_VENDER_ID_REALTEK, 0x0724,0xff,0xff,0xff)}, /* 8723AU 1*1 */


static struct usb_device_id rtw_usb_id_tbl[] ={
	RTL8723A_USB_IDS
	{}	/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, rtw_usb_id_tbl);

//int const rtw_usb_id_len = sizeof(rtw_usb_id_tbl) / sizeof(struct usb_device_id);

static struct specific_device_id specific_device_id_tbl[] = {
	{.idVendor=USB_VENDER_ID_REALTEK, .idProduct=0x8177, .flags=SPEC_DEV_ID_DISABLE_HT},//8188cu 1*1 dongole, (b/g mode only)
	{.idVendor=USB_VENDER_ID_REALTEK, .idProduct=0x817E, .flags=SPEC_DEV_ID_DISABLE_HT},//8188CE-VAU USB minCard (b/g mode only)
	{.idVendor=0x0b05, .idProduct=0x1791, .flags=SPEC_DEV_ID_DISABLE_HT},
	{.idVendor=0x13D3, .idProduct=0x3311, .flags=SPEC_DEV_ID_DISABLE_HT},
	{.idVendor=0x13D3, .idProduct=0x3359, .flags=SPEC_DEV_ID_DISABLE_HT},//Russian customer -Azwave (8188CE-VAU  g mode)
	{}
};

#ifdef CONFIG_RTL8192C
static struct usb_device_id rtl8192c_usb_id_tbl[] ={
	RTL8192C_USB_IDS
	{}	/* Terminating entry */
};

struct usb_driver rtl8192c_usb_drv = {
	.name = (char*)"rtl8192cu",
	.probe = rtw_drv_init,
	.disconnect = rtw_disconnect,
	.id_table = rtl8192c_usb_id_tbl,
	.suspend = rtw_suspend,
	.resume = rtw_resume,
	.reset_resume  = rtw_resume,
#ifdef CONFIG_AUTOSUSPEND
	.supports_autosuspend = 1,
#endif
};

static struct usb_driver *usb_drv = &rtl8192c_usb_drv;
#endif /* CONFIG_RTL8192C */

#ifdef CONFIG_RTL8192D
static struct usb_device_id rtl8192d_usb_id_tbl[] ={
	RTL8192D_USB_IDS
	{}	/* Terminating entry */
};

struct usb_driver rtl8192d_usb_drv = {
	.name = (char*)"rtl8192du",
	.probe = rtw_drv_init,
	.disconnect = rtw_disconnect,
	.id_table = rtl8192d_usb_id_tbl,
	.suspend = rtw_suspend,
	.resume = rtw_resume,
	.reset_resume  = rtw_resume,
#ifdef CONFIG_AUTOSUSPEND
	.supports_autosuspend = 1,
#endif
};
static struct usb_driver *usb_drv = &rtl8192d_usb_drv;
#endif /* CONFIG_RTL8192D */

#ifdef CONFIG_RTL8723A
static struct usb_device_id rtl8723a_usb_id_tbl[] ={
	RTL8723A_USB_IDS
	{}	/* Terminating entry */
};

static struct usb_driver rtl8723a_usb_drv = {
	.name = (char*)"rtl8723au",
	.probe = rtw_drv_init,
	.disconnect = rtw_disconnect,
	.id_table = rtl8723a_usb_id_tbl,
	.suspend = rtw_suspend,
	.resume = rtw_resume,
	.reset_resume  = rtw_resume,
#ifdef CONFIG_AUTOSUSPEND
	.supports_autosuspend = 1,
#endif
};

static struct usb_driver *usb_drv = &rtl8723a_usb_drv;
#endif /* CONFIG_RTL8723A */

#ifdef CONFIG_RTL8188E
static struct usb_device_id rtl8188e_usb_id_tbl[] ={
	RTL8188E_USB_IDS
	{}	/* Terminating entry */
};

struct usb_driver rtl8188e_usb_drv = {
	.name = (char*)"rtl8188eu",
	.probe = rtw_drv_init,
	.disconnect = rtw_disconnect,
	.id_table = rtl8188e_usb_id_tbl,
	.suspend =  rtw_suspend,
	.resume = rtw_resume,
	.reset_resume   = rtw_resume,
#ifdef CONFIG_AUTOSUSPEND
	.supports_autosuspend = 1,
#endif
};

static struct usb_driver *usb_drv = &rtl8188e_usb_drv;
#endif /* CONFIG_RTL8188E */

static inline int RT_usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
}

static inline int RT_usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

static inline int RT_usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT);
}

static inline int RT_usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK);
}

static inline int RT_usb_endpoint_is_bulk_in(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_in(epd));
}

static inline int RT_usb_endpoint_is_bulk_out(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_out(epd));
}

static inline int RT_usb_endpoint_is_int_in(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_int(epd) && RT_usb_endpoint_dir_in(epd));
}

static inline int RT_usb_endpoint_num(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
}

static u8 rtw_init_intf_priv(struct dvobj_priv *dvobj)
{
	u8 rst = _SUCCESS;

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	mutex_init(&dvobj->usb_vendor_req_mutex);
#endif


#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	dvobj->usb_alloc_vendor_req_buf = kzalloc(MAX_USB_IO_CTL_SIZE,
						  GFP_KERNEL);
	if (dvobj->usb_alloc_vendor_req_buf == NULL) {
		DBG_8723A("alloc usb_vendor_req_buf failed... /n");
		rst = _FAIL;
		goto exit;
	}
	dvobj->usb_vendor_req_buf =
		PTR_ALIGN(dvobj->usb_alloc_vendor_req_buf, ALIGNMENT_UNIT);
exit:
#endif

	return rst;

}

static u8 rtw_deinit_intf_priv(struct dvobj_priv *dvobj)
{
	u8 rst = _SUCCESS;

#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	if(dvobj->usb_vendor_req_buf)
		kfree(dvobj->usb_alloc_vendor_req_buf);
#endif

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	mutex_destroy(&dvobj->usb_vendor_req_mutex);
#endif

	return rst;
}

static struct dvobj_priv *usb_dvobj_init(struct usb_interface *usb_intf)
{
	int	i;
	u8	val8;
	int	status = _FAIL;
	struct dvobj_priv *pdvobjpriv;
	struct usb_device_descriptor	*pdev_desc;
	struct usb_host_config			*phost_conf;
	struct usb_config_descriptor		*pconf_desc;
	struct usb_host_interface		*phost_iface;
	struct usb_interface_descriptor	*piface_desc;
	struct usb_host_endpoint		*phost_endp;
	struct usb_endpoint_descriptor	*pendp_desc;
	struct usb_device				*pusbd;

_func_enter_;

	if ((pdvobjpriv = (struct dvobj_priv*)kzalloc(sizeof(*pdvobjpriv), GFP_KERNEL)) == NULL) {
		goto exit;
	}

	mutex_init(&pdvobjpriv->hw_init_mutex);
	mutex_init(&pdvobjpriv->h2c_fwcmd_mutex);
	mutex_init(&pdvobjpriv->setch_mutex);
	mutex_init(&pdvobjpriv->setbw_mutex);


	pdvobjpriv->pusbintf = usb_intf ;
	pusbd = pdvobjpriv->pusbdev = interface_to_usbdev(usb_intf);
	usb_set_intfdata(usb_intf, pdvobjpriv);

	pdvobjpriv->RtNumInPipes = 0;
	pdvobjpriv->RtNumOutPipes = 0;

	pdev_desc = &pusbd->descriptor;

	phost_conf = pusbd->actconfig;
	pconf_desc = &phost_conf->desc;

	phost_iface = &usb_intf->altsetting[0];
	piface_desc = &phost_iface->desc;

	pdvobjpriv->NumInterfaces = pconf_desc->bNumInterfaces;
	pdvobjpriv->InterfaceNumber = piface_desc->bInterfaceNumber;
	pdvobjpriv->nr_endpoint = piface_desc->bNumEndpoints;

	for (i = 0; i < pdvobjpriv->nr_endpoint; i++) {
		phost_endp = phost_iface->endpoint + i;
		if (phost_endp) {
			pendp_desc = &phost_endp->desc;

			DBG_8723A("\nusb_endpoint_descriptor(%d):\n", i);
			DBG_8723A("bLength=%x\n",pendp_desc->bLength);
			DBG_8723A("bDescriptorType=%x\n",pendp_desc->bDescriptorType);
			DBG_8723A("bEndpointAddress=%x\n",pendp_desc->bEndpointAddress);
			DBG_8723A("wMaxPacketSize=%d\n",le16_to_cpu(pendp_desc->wMaxPacketSize));
			DBG_8723A("bInterval=%x\n",pendp_desc->bInterval);

			if (RT_usb_endpoint_is_bulk_in(pendp_desc)) {
				DBG_8723A("RT_usb_endpoint_is_bulk_in = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtInPipe[pdvobjpriv->RtNumInPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumInPipes++;
			} else if (RT_usb_endpoint_is_int_in(pendp_desc)) {
				DBG_8723A("RT_usb_endpoint_is_int_in = %x, Interval = %x\n", RT_usb_endpoint_num(pendp_desc),pendp_desc->bInterval);
				pdvobjpriv->RtInPipe[pdvobjpriv->RtNumInPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumInPipes++;
			} else if (RT_usb_endpoint_is_bulk_out(pendp_desc)) {
				DBG_8723A("RT_usb_endpoint_is_bulk_out = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtOutPipe[pdvobjpriv->RtNumOutPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumOutPipes++;
			}
			pdvobjpriv->ep_num[i] = RT_usb_endpoint_num(pendp_desc);
		}
	}

	DBG_8723A("nr_endpoint=%d, in_num=%d, out_num=%d\n\n", pdvobjpriv->nr_endpoint, pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);

	if (pusbd->speed == USB_SPEED_HIGH) {
		pdvobjpriv->ishighspeed = _TRUE;
		DBG_8723A("USB_SPEED_HIGH\n");
	} else {
		pdvobjpriv->ishighspeed = _FALSE;
		DBG_8723A("NON USB_SPEED_HIGH\n");
	}

	if (rtw_init_intf_priv(pdvobjpriv) == _FAIL) {
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't INIT rtw_init_intf_priv\n"));
		goto free_dvobj;
	}

	//.3 misc
	sema_init(&(pdvobjpriv->usb_suspend_sema), 0);
	rtw_reset_continual_urb_error(pdvobjpriv);

	usb_get_dev(pusbd);

	status = _SUCCESS;

free_dvobj:
	if (status != _SUCCESS && pdvobjpriv) {
		usb_set_intfdata(usb_intf, NULL);
		mutex_destroy(&pdvobjpriv->hw_init_mutex);
		mutex_destroy(&pdvobjpriv->h2c_fwcmd_mutex);
		mutex_destroy(&pdvobjpriv->setch_mutex);
		mutex_destroy(&pdvobjpriv->setbw_mutex);
		kfree(pdvobjpriv);
		pdvobjpriv = NULL;
	}
exit:
_func_exit_;
	return pdvobjpriv;
}

static void usb_dvobj_deinit(struct usb_interface *usb_intf)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(usb_intf);

_func_enter_;

	usb_set_intfdata(usb_intf, NULL);
	if (dvobj) {
		//Modify condition for 92DU DMDP 2010.11.18, by Thomas
		if ((dvobj->NumInterfaces != 2 && dvobj->NumInterfaces != 3)
			|| (dvobj->InterfaceNumber == 1)) {
			if (interface_to_usbdev(usb_intf)->state != USB_STATE_NOTATTACHED) {
				//If we didn't unplug usb dongle and remove/insert modlue, driver fails on sitesurvey for the first time when device is up .
				//Reset usb port for sitesurvey fail issue. 2009.8.13, by Thomas
				DBG_8723A("usb attached..., try to reset usb device\n");
				usb_reset_device(interface_to_usbdev(usb_intf));
			}
		}
		rtw_deinit_intf_priv(dvobj);
		mutex_destroy(&dvobj->hw_init_mutex);
		mutex_destroy(&dvobj->h2c_fwcmd_mutex);
		mutex_destroy(&dvobj->setch_mutex);
		mutex_destroy(&dvobj->setbw_mutex);
		kfree(dvobj);
	}

	usb_put_dev(interface_to_usbdev(usb_intf));

_func_exit_;
}

static void decide_chip_type_by_usb_device_id(struct rtw_adapter *padapter, const struct usb_device_id *pdid)
{
	padapter->chip_type = NULL_CHIP_TYPE;
	hal_set_hw_type(padapter);
}

static void usb_intf_start(struct rtw_adapter *padapter)
{

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+usb_intf_start\n"));

	rtw_hal_inirp_init(padapter);

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-usb_intf_start\n"));

}

static void usb_intf_stop(struct rtw_adapter *padapter)
{

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+usb_intf_stop\n"));

	//disabel_hw_interrupt
	if(padapter->bSurpriseRemoved == _FALSE)
	{
		//device still exists, so driver can do i/o operation
		//TODO:
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("SurpriseRemoved==_FALSE\n"));
	}

	//cancel in irp
	rtw_hal_inirp_deinit(padapter);

	//cancel out irp
	rtw_write_port_cancel(padapter);

	//todo:cancel other irps

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-usb_intf_stop\n"));

}

static void rtw_dev_unload(struct rtw_adapter *padapter)
{
	struct net_device *pnetdev= (struct net_device*)padapter->pnetdev;
	u8 val8;
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+rtw_dev_unload\n"));

	if(padapter->bup == _TRUE)
	{
		DBG_8723A("===> rtw_dev_unload\n");

		padapter->bDriverStopped = _TRUE;
		#ifdef CONFIG_XMIT_ACK
		if (padapter->xmitpriv.ack_tx)
			rtw_ack_tx_done(&padapter->xmitpriv, RTW_SCTX_DONE_DRV_STOP);
		#endif

		//s3.
		if(padapter->intf_stop)
		{
			padapter->intf_stop(padapter);
		}

		//s4.
		if(!padapter->pwrctrlpriv.bInternalAutoSuspend )
		rtw_stop_drv_threads(padapter);


		//s5.
		if(padapter->bSurpriseRemoved == _FALSE)
		{
#ifdef CONFIG_WOWLAN
			if((padapter->pwrctrlpriv.bSupportRemoteWakeup==_TRUE)&&(padapter->pwrctrlpriv.wowlan_mode==_TRUE)){
				DBG_8723A("%s bSupportWakeOnWlan==_TRUE  do not run rtw_hal_deinit()\n",__FUNCTION__);
			}
			else
#endif //CONFIG_WOWLAN
			{
				rtw_hal_deinit(padapter);
			}
			padapter->bSurpriseRemoved = _TRUE;
		}

		padapter->bup = _FALSE;
#ifdef CONFIG_WOWLAN
		padapter->hw_init_completed=_FALSE;
#endif //CONFIG_WOWLAN
	}
	else
	{
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("r871x_dev_unload():padapter->bup == _FALSE\n" ));
	}

	DBG_8723A("<=== rtw_dev_unload\n");

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-rtw_dev_unload\n"));

}

static void process_spec_devid(const struct usb_device_id *pdid)
{
	u16 vid, pid;
	u32 flags;
	int i;
	int num = sizeof(specific_device_id_tbl)/sizeof(struct specific_device_id);

	for(i=0; i<num; i++)
	{
		vid = specific_device_id_tbl[i].idVendor;
		pid = specific_device_id_tbl[i].idProduct;
		flags = specific_device_id_tbl[i].flags;

#ifdef CONFIG_80211N_HT
		if((pdid->idVendor==vid) && (pdid->idProduct==pid) && (flags&SPEC_DEV_ID_DISABLE_HT))
		{
			 rtw_ht_enable = 0;
			 rtw_cbw40_enable = 0;
			 rtw_ampdu_enable = 0;
		}
#endif
	}
}

#ifdef SUPPORT_HW_RFOFF_DETECTED
int rtw_hw_suspend(struct rtw_adapter *padapter )
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct usb_interface *pusb_intf = adapter_to_dvobj(padapter)->pusbintf;
	struct net_device *pnetdev = padapter->pnetdev;

	_func_enter_;

	if((!padapter->bup) || (padapter->bDriverStopped)||(padapter->bSurpriseRemoved))
	{
		DBG_8723A("padapter->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			padapter->bup, padapter->bDriverStopped,padapter->bSurpriseRemoved);
		goto error_exit;
	}

	if(padapter)//system suspend
	{
		LeaveAllPowerSaveMode(padapter);

		DBG_8723A("==> rtw_hw_suspend\n");
		down(&pwrpriv->lock);
		pwrpriv->bips_processing = _TRUE;
		//padapter->net_closed = _TRUE;
		//s1.
		if(pnetdev)
		{
			netif_carrier_off(pnetdev);
			rtw_netif_stop_queue(pnetdev);
		}

		//s2.
		rtw_disassoc_cmd(padapter, 500, _FALSE);

		//s2-2.  indicate disconnect to os
		//rtw_indicate_disconnect(padapter);
		{
			struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

			if(check_fwstate(pmlmepriv, _FW_LINKED))
			{
				_clr_fwstate_(pmlmepriv, _FW_LINKED);

				rtw_led_control(padapter, LED_CTL_NO_LINK);

				rtw_os_indicate_disconnect(padapter);

				#ifdef CONFIG_LPS
				//donnot enqueue cmd
				rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_DISCONNECT, 0);
				#endif
			}

		}
		//s2-3.
		rtw_free_assoc_resources(padapter, 1);

		//s2-4.
		rtw_free_network_queue(padapter,_TRUE);
		#ifdef CONFIG_IPS
		rtw_ips_dev_unload(padapter);
		#endif
		pwrpriv->rf_pwrstate = rf_off;
		pwrpriv->bips_processing = _FALSE;

		up(&pwrpriv->lock);
	}
	else
		goto error_exit;

	_func_exit_;
	return 0;

error_exit:
	DBG_8723A("%s, failed \n",__FUNCTION__);
	return (-1);

}

int rtw_hw_resume(struct rtw_adapter *padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct usb_interface *pusb_intf = adapter_to_dvobj(padapter)->pusbintf;
	struct net_device *pnetdev = padapter->pnetdev;

	_func_enter_;

	if(padapter)//system resume
	{
		DBG_8723A("==> rtw_hw_resume\n");
		down(&pwrpriv->lock);
		pwrpriv->bips_processing = _TRUE;
		rtw_reset_drv_sw(padapter);

		if(pm_netdev_open(pnetdev,_FALSE) != 0)
		{
			up(&pwrpriv->lock);
			goto error_exit;
		}

		netif_device_attach(pnetdev);
		netif_carrier_on(pnetdev);

		if(!rtw_netif_queue_stopped(pnetdev))
			rtw_netif_start_queue(pnetdev);
		else
			rtw_netif_wake_queue(pnetdev);

		pwrpriv->bkeepfwalive = _FALSE;
		pwrpriv->brfoffbyhw = _FALSE;

		pwrpriv->rf_pwrstate = rf_on;
		pwrpriv->bips_processing = _FALSE;

		up(&pwrpriv->lock);
	}
	else
	{
		goto error_exit;
	}

	_func_exit_;

	return 0;
error_exit:
	DBG_8723A("%s, Open net dev failed \n",__FUNCTION__);
	return (-1);
}
#endif

static int rtw_suspend(struct usb_interface *pusb_intf, pm_message_t message)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(pusb_intf);
	struct rtw_adapter *padapter = dvobj->if1;
	struct net_device *pnetdev = padapter->pnetdev;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct usb_device *usb_dev = interface_to_usbdev(pusb_intf);
#ifdef CONFIG_WOWLAN
	struct wowlan_ioctl_param poidparam;
#endif // CONFIG_WOWLAN

	int ret = 0;
	u32 start_time = rtw_get_current_time();

	_func_enter_;

	DBG_8723A("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);

#ifdef CONFIG_WOWLAN
	if (check_fwstate(pmlmepriv, _FW_LINKED))
		padapter->pwrctrlpriv.wowlan_mode = _TRUE;
	else
		padapter->pwrctrlpriv.wowlan_mode = _FALSE;
#endif

	if((!padapter->bup) || (padapter->bDriverStopped)||(padapter->bSurpriseRemoved))
	{
		DBG_8723A("padapter->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			padapter->bup, padapter->bDriverStopped,padapter->bSurpriseRemoved);
		goto exit;
	}

	if(pwrpriv->bInternalAutoSuspend )
	{
	#ifdef CONFIG_AUTOSUSPEND
	#ifdef SUPPORT_HW_RFOFF_DETECTED
		// The FW command register update must after MAC and FW init ready.
		if((padapter->bFWReady) && ( padapter->pwrctrlpriv.bHWPwrPindetect ) && (padapter->registrypriv.usbss_enable ))
		{
			u8 bOpen = _TRUE;
			rtw_interface_ps_func(padapter,HAL_USB_SELECT_SUSPEND,&bOpen);
			//rtl8192c_set_FwSelectSuspend_cmd(padapter,_TRUE ,500);//note fw to support hw power down ping detect
		}
	#endif
	#endif
	}
	pwrpriv->bInSuspend = _TRUE;
	rtw_cancel_all_timer(padapter);
	LeaveAllPowerSaveMode(padapter);

	down(&pwrpriv->lock);
	//padapter->net_closed = _TRUE;
	//s1.
	if(pnetdev)
	{
		netif_carrier_off(pnetdev);
		rtw_netif_stop_queue(pnetdev);
	}

#ifdef CONFIG_WOWLAN
	if(padapter->pwrctrlpriv.bSupportRemoteWakeup==_TRUE&&padapter->pwrctrlpriv.wowlan_mode==_TRUE){
		//set H2C command
		poidparam.subcode=WOWLAN_ENABLE;
		padapter->HalFunc.SetHwRegHandler(padapter,HW_VAR_WOWLAN,(u8 *)&poidparam);
	}
	else
#else
	{
	//s2.
	rtw_disassoc_cmd(padapter, 0, _FALSE);
	}
#endif //CONFIG_WOWLAN

#ifdef CONFIG_LAYER2_ROAMING_RESUME
	if(check_fwstate(pmlmepriv, WIFI_STATION_STATE) && check_fwstate(pmlmepriv, _FW_LINKED) )
	{
		//DBG_8723A("%s:%d assoc_ssid:%s\n", __FUNCTION__, __LINE__, pmlmepriv->assoc_ssid.Ssid);
		DBG_8723A("%s:%d %s( %pM ), length:%d assoc_ssid.length:%d\n",__FUNCTION__, __LINE__,
			  pmlmepriv->cur_network.network.Ssid.Ssid,
			  pmlmepriv->cur_network.network.MacAddress,
			  pmlmepriv->cur_network.network.Ssid.SsidLength,
			  pmlmepriv->assoc_ssid.SsidLength);

		rtw_set_roaming(padapter, 1);
	}
#endif
	//s2-2.  indicate disconnect to os
	rtw_indicate_disconnect(padapter);
	//s2-3.
	rtw_free_assoc_resources(padapter, 1);
#ifdef CONFIG_AUTOSUSPEND
	if(!pwrpriv->bInternalAutoSuspend )
#endif
	//s2-4.
	rtw_free_network_queue(padapter, _TRUE);

	rtw_dev_unload(padapter);
#ifdef CONFIG_AUTOSUSPEND
	pwrpriv->rf_pwrstate = rf_off;
	pwrpriv->bips_processing = _FALSE;
#endif
	up(&pwrpriv->lock);

	if(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY))
		rtw_indicate_scan_done(padapter, 1);

	if(check_fwstate(pmlmepriv, _FW_UNDER_LINKING))
		rtw_indicate_disconnect(padapter);

exit:
	DBG_8723A("<===  %s return %d.............. in %dms\n", __FUNCTION__
		, ret, rtw_get_passing_time_ms(start_time));

	_func_exit_;
	return ret;
}

static int rtw_resume(struct usb_interface *pusb_intf)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(pusb_intf);
	struct rtw_adapter *padapter = dvobj->if1;
	struct net_device *pnetdev = padapter->pnetdev;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	int ret = 0;

	if(pwrpriv->bInternalAutoSuspend ){
		ret = rtw_resume_process(padapter);
	} else {
#ifdef CONFIG_RESUME_IN_WORKQUEUE
		rtw_resume_in_workqueue(pwrpriv);
#else
		ret = rtw_resume_process(padapter);
#endif /* CONFIG_RESUME_IN_WORKQUEUE */
	}

	return ret;

}

int rtw_resume_process(struct rtw_adapter *padapter)
{
	struct net_device *pnetdev;
	struct pwrctrl_priv *pwrpriv;
	int ret = -1;
	u32 start_time = rtw_get_current_time();
#ifdef CONFIG_BT_COEXIST
	u8 pm_cnt;
#endif	//#ifdef CONFIG_BT_COEXIST
	_func_enter_;

	DBG_8723A("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);

	if(padapter) {
		pnetdev= padapter->pnetdev;
		pwrpriv = &padapter->pwrctrlpriv;
	} else {
		goto exit;
	}

	down(&pwrpriv->lock);
#ifdef CONFIG_BT_COEXIST
#ifdef CONFIG_AUTOSUSPEND
	DBG_8723A("%s...pm_usage_cnt(%d)  pwrpriv->bAutoResume=%x.  ....\n",__func__,atomic_read(&(adapter_to_dvobj(padapter)->pusbintf->pm_usage_cnt)),pwrpriv->bAutoResume);
	pm_cnt=atomic_read(&(adapter_to_dvobj(padapter)->pusbintf->pm_usage_cnt));

	DBG_8723A("pwrpriv->bAutoResume (%x)\n",pwrpriv->bAutoResume );
	if( _TRUE == pwrpriv->bAutoResume ){
		pwrpriv->bInternalAutoSuspend = _FALSE;
		pwrpriv->bAutoResume=_FALSE;
		DBG_8723A("pwrpriv->bAutoResume (%x)  pwrpriv->bInternalAutoSuspend(%x)\n",pwrpriv->bAutoResume,pwrpriv->bInternalAutoSuspend );

	}
#endif //#ifdef CONFIG_AUTOSUSPEND
#endif //#ifdef CONFIG_BT_COEXIST
	rtw_reset_drv_sw(padapter);
	pwrpriv->bkeepfwalive = _FALSE;

	DBG_8723A("bkeepfwalive(%x)\n",pwrpriv->bkeepfwalive);
	if(pm_netdev_open(pnetdev,_TRUE) != 0)
		goto exit;

	netif_device_attach(pnetdev);
	netif_carrier_on(pnetdev);

#ifdef CONFIG_AUTOSUSPEND
	if(pwrpriv->bInternalAutoSuspend )
	{
		#ifdef CONFIG_AUTOSUSPEND
		#ifdef SUPPORT_HW_RFOFF_DETECTED
			// The FW command register update must after MAC and FW init ready.
		if((padapter->bFWReady) && ( padapter->pwrctrlpriv.bHWPwrPindetect ) && (padapter->registrypriv.usbss_enable ))
		{
			//rtl8192c_set_FwSelectSuspend_cmd(padapter,_FALSE ,500);//note fw to support hw power down ping detect
			u8 bOpen = _FALSE;
			rtw_interface_ps_func(padapter,HAL_USB_SELECT_SUSPEND,&bOpen);
		}
		#endif
		#endif
#ifdef CONFIG_BT_COEXIST
		DBG_8723A("pwrpriv->bAutoResume (%x)\n",pwrpriv->bAutoResume );
		if( _TRUE == pwrpriv->bAutoResume ){
		pwrpriv->bInternalAutoSuspend = _FALSE;
			pwrpriv->bAutoResume=_FALSE;
			DBG_8723A("pwrpriv->bAutoResume (%x)  pwrpriv->bInternalAutoSuspend(%x)\n",pwrpriv->bAutoResume,pwrpriv->bInternalAutoSuspend );
		}

#else	//#ifdef CONFIG_BT_COEXIST
		pwrpriv->bInternalAutoSuspend = _FALSE;
#endif	//#ifdef CONFIG_BT_COEXIST
		pwrpriv->brfoffbyhw = _FALSE;
		DBG_8723A("enc_algorithm(%x),wepkeymask(%x)\n",
			padapter->securitypriv.dot11PrivacyAlgrthm,pwrpriv->wepkeymask);
		if ((_WEP40_ == padapter->securitypriv.dot11PrivacyAlgrthm) ||
		    (_WEP104_ == padapter->securitypriv.dot11PrivacyAlgrthm)) {
			int keyid;
			for (keyid = 0; keyid < 4; keyid++){
				if(pwrpriv->wepkeymask & BIT(keyid)) {
					if(keyid == padapter->securitypriv.dot11PrivacyKeyIndex)
						rtw_set_key(padapter,&padapter->securitypriv, keyid, 1);
					else
						rtw_set_key(padapter,&padapter->securitypriv, keyid, 0);
				}
			}
		}
	}
#endif
	up(&pwrpriv->lock);

	if( padapter->pid[1]!=0) {
		DBG_8723A("pid[1]:%d\n",padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}

	#ifdef CONFIG_LAYER2_ROAMING_RESUME
	rtw_roaming(padapter, NULL);
	#endif

	ret = 0;
exit:
	#ifdef CONFIG_RESUME_IN_WORKQUEUE
	rtw_unlock_suspend();
	#endif //CONFIG_RESUME_IN_WORKQUEUE

	if (pwrpriv)
		pwrpriv->bInSuspend = _FALSE;
	DBG_8723A("<===  %s return %d.............. in %dms\n", __FUNCTION__,
		  ret, rtw_get_passing_time_ms(start_time));

	_func_exit_;
	return ret;
}

#ifdef CONFIG_AUTOSUSPEND
void autosuspend_enter(struct rtw_adapter* padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	DBG_8723A("==>autosuspend_enter...........\n");

	pwrpriv->bInternalAutoSuspend = _TRUE;
	pwrpriv->bips_processing = _TRUE;

	if(rf_off == pwrpriv->change_rfpwrstate ) {
#ifndef	CONFIG_BT_COEXIST
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
		usb_enable_autosuspend(dvobj->pusbdev);
		#else
		dvobj->pusbdev->autosuspend_disabled = 0;//autosuspend disabled by the user
		#endif

		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			usb_autopm_put_interface(dvobj->pusbintf);
		#else
			usb_autopm_enable(dvobj->pusbintf);
		#endif
#else	//#ifndef	CONFIG_BT_COEXIST
		if(1==pwrpriv->autopm_cnt){
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
		usb_enable_autosuspend(dvobj->pusbdev);
		#else
		dvobj->pusbdev->autosuspend_disabled = 0;//autosuspend disabled by the user
		#endif

		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			usb_autopm_put_interface(dvobj->pusbintf);
		#else
			usb_autopm_enable(dvobj->pusbintf);
		#endif
			pwrpriv->autopm_cnt --;
		}
		else
		DBG_8723A("0!=pwrpriv->autopm_cnt[%d]   didn't usb_autopm_put_interface\n", pwrpriv->autopm_cnt);

#endif	//#ifndef	CONFIG_BT_COEXIST
	}
	DBG_8723A("...pm_usage_cnt(%d).....\n", atomic_read(&(dvobj->pusbintf->pm_usage_cnt)));
}
int autoresume_enter(struct rtw_adapter* padapter)
{
	int result = _SUCCESS;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	DBG_8723A("====> autoresume_enter \n");

	if(rf_off == pwrpriv->rf_pwrstate )
	{
		pwrpriv->ps_flag = _FALSE;
#ifndef	CONFIG_BT_COEXIST
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			if (usb_autopm_get_interface(dvobj->pusbintf) < 0)
			{
				DBG_8723A( "can't get autopm: %d\n", result);
				result = _FAIL;
				goto error_exit;
			}
		#else
			usb_autopm_disable(dvobj->pusbintf);
		#endif

		DBG_8723A("...pm_usage_cnt(%d).....\n", atomic_read(&(dvobj->pusbintf->pm_usage_cnt)));
#else	//#ifndef	CONFIG_BT_COEXIST
		pwrpriv->bAutoResume=_TRUE;
		if(0==pwrpriv->autopm_cnt){
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			if (usb_autopm_get_interface(dvobj->pusbintf) < 0)
			{
				DBG_8723A( "can't get autopm: %d\n", result);
				result = _FAIL;
				goto error_exit;
			}
		#else
			usb_autopm_disable(dvobj->pusbintf);
		#endif
			DBG_8723A("...pm_usage_cnt(%d).....\n", atomic_read(&(dvobj->pusbintf->pm_usage_cnt)));
			pwrpriv->autopm_cnt++;
		}
		else
			DBG_8723A("0!=pwrpriv->autopm_cnt[%d]   didn't usb_autopm_get_interface\n",pwrpriv->autopm_cnt);
#endif //#ifndef	CONFIG_BT_COEXIST
	}
	DBG_8723A("<==== autoresume_enter \n");
error_exit:

	return result;
}
#endif

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
*/
static struct rtw_adapter *rtw_usb_if1_init(struct dvobj_priv *dvobj,
					    struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	struct rtw_adapter *padapter = NULL;
	struct net_device *pnetdev = NULL;
	int status = _FAIL;

	if ((padapter = (struct rtw_adapter *)rtw_zvmalloc(sizeof(*padapter))) == NULL) {
		goto exit;
	}
	padapter->dvobj = dvobj;
	dvobj->if1 = padapter;

	padapter->bDriverStopped=_TRUE;

	dvobj->padapters[dvobj->iface_nums++] = padapter;
	padapter->iface_id = IFACE_ID0;

#if defined(CONFIG_CONCURRENT_MODE) || defined(CONFIG_DUALMAC_CONCURRENT)
	//set adapter_type/iface type for primary padapter
	padapter->isprimary = _TRUE;
	padapter->adapter_type = PRIMARY_ADAPTER;
	#ifndef CONFIG_HWPORT_SWAP
	padapter->iface_type = IFACE_PORT0;
	#else
	padapter->iface_type = IFACE_PORT1;
	#endif
#endif

	#ifndef RTW_DVOBJ_CHIP_HW_TYPE
	//step 1-1., decide the chip_type via vid/pid
	padapter->interface_type = RTW_USB;
	decide_chip_type_by_usb_device_id(padapter, pdid);
	#endif

	if (rtw_handle_dualmac(padapter, 1) != _SUCCESS)
		goto free_adapter;

	if((pnetdev = rtw_init_netdev(padapter)) == NULL) {
		goto handle_dualmac;
	}
	SET_NETDEV_DEV(pnetdev, dvobj_to_dev(dvobj));
	padapter = rtw_netdev_priv(pnetdev);

#ifdef CONFIG_IOCTL_CFG80211
	if(rtw_wdev_alloc(padapter, dvobj_to_dev(dvobj)) != 0) {
		goto handle_dualmac;
	}
#endif


	//step 2. hook HalFunc, allocate HalData
	hal_set_hal_ops(padapter);

	padapter->intf_start=&usb_intf_start;
	padapter->intf_stop=&usb_intf_stop;

	//step init_io_priv
	rtw_init_io_priv(padapter, usb_set_intf_ops);

	//step read_chip_version
	rtw_hal_read_chip_version(padapter);

	//step usb endpoint mapping
	rtw_hal_chip_configure(padapter);

	//step read efuse/eeprom data and get mac_addr
	rtw_hal_read_chip_info(padapter);

	//step 5.
	if(rtw_init_drv_sw(padapter) ==_FAIL) {
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("Initialize driver software resource Failed!\n"));
		goto free_hal_data;
	}

#ifdef CONFIG_PM
	if(padapter->pwrctrlpriv.bSupportRemoteWakeup)
	{
		dvobj->pusbdev->do_remote_wakeup=1;
		pusb_intf->needs_remote_wakeup = 1;
		device_init_wakeup(&pusb_intf->dev, 1);
		DBG_8723A("\n  padapter->pwrctrlpriv.bSupportRemoteWakeup~~~~~~\n");
		DBG_8723A("\n  padapter->pwrctrlpriv.bSupportRemoteWakeup~~~[%d]~~~\n",device_may_wakeup(&pusb_intf->dev));
	}
#endif

#ifdef CONFIG_AUTOSUSPEND
	if( padapter->registrypriv.power_mgnt != PS_MODE_ACTIVE )
	{
		if(padapter->registrypriv.usbss_enable ){	/* autosuspend (2s delay) */
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,38))
			dvobj->pusbdev->dev.power.autosuspend_delay = 0 * HZ;//15 * HZ; idle-delay time
#else
			dvobj->pusbdev->autosuspend_delay = 0 * HZ;//15 * HZ; idle-delay time
#endif

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
			usb_enable_autosuspend(dvobj->pusbdev);
#elif  (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE<=KERNEL_VERSION(2,6,34))
			padapter->bDisableAutosuspend = dvobj->pusbdev->autosuspend_disabled ;
			dvobj->pusbdev->autosuspend_disabled = 0;//autosuspend disabled by the user
#endif

			//usb_autopm_get_interface(adapter_to_dvobj(padapter)->pusbintf );//init pm_usage_cnt ,let it start from 1

			DBG_8723A("%s...pm_usage_cnt(%d).....\n",__FUNCTION__,atomic_read(&(dvobj->pusbintf ->pm_usage_cnt)));
		}
	}
#endif
	//2012-07-11 Move here to prevent the 8723AS-VAU BT auto suspend influence
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			if (usb_autopm_get_interface(pusb_intf) < 0)
				{
					DBG_8723A( "can't get autopm: \n");
				}
#endif
#ifdef	CONFIG_BT_COEXIST
	padapter->pwrctrlpriv.autopm_cnt=1;
#endif

	// set mac addr
	rtw_macaddr_cfg(padapter->eeprompriv.mac_addr);
	rtw_init_wifidirect_addrs(padapter, padapter->eeprompriv.mac_addr, padapter->eeprompriv.mac_addr);

	DBG_8723A("bDriverStopped:%d, bSurpriseRemoved:%d, bup:%d, hw_init_completed:%d\n"
		, padapter->bDriverStopped
		, padapter->bSurpriseRemoved
		, padapter->bup
		, padapter->hw_init_completed
	);

	status = _SUCCESS;

free_hal_data:
	if(status != _SUCCESS && padapter->HalData)
		kfree(padapter->HalData);
free_wdev:
	if(status != _SUCCESS) {
		#ifdef CONFIG_IOCTL_CFG80211
		rtw_wdev_unregister(padapter->rtw_wdev);
		rtw_wdev_free(padapter->rtw_wdev);
		#endif
	}
handle_dualmac:
	if (status != _SUCCESS)
		rtw_handle_dualmac(padapter, 0);
free_adapter:
	if (status != _SUCCESS) {
		if (pnetdev)
			rtw_free_netdev(pnetdev);
		else if (padapter)
			rtw_vmfree((u8*)padapter, sizeof(*padapter));
		padapter = NULL;
	}
exit:
	return padapter;
}

static void rtw_usb_if1_deinit(struct rtw_adapter *if1)
{
	struct net_device *pnetdev = if1->pnetdev;
	struct mlme_priv *pmlmepriv= &if1->mlmepriv;

	if(check_fwstate(pmlmepriv, _FW_LINKED))
		rtw_disassoc_cmd(if1, 0, _FALSE);


#ifdef CONFIG_AP_MODE
	free_mlme_ap_info(if1);
	#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_unload(if1);
	#endif
#endif

	if(if1->DriverState != DRIVER_DISAPPEAR) {
		if(pnetdev) {
			unregister_netdev(pnetdev); //will call netdev_close()
			rtw_proc_remove_one(pnetdev);
		}
	}

	rtw_cancel_all_timer(if1);

#ifdef CONFIG_WOWLAN
	if1->pwrctrlpriv.wowlan_mode=_FALSE;
#endif //CONFIG_WOWLAN

	rtw_dev_unload(if1);

	DBG_8723A("+r871xu_dev_remove, hw_init_completed=%d\n", if1->hw_init_completed);

	rtw_handle_dualmac(if1, 0);

#ifdef CONFIG_IOCTL_CFG80211
	if(if1->rtw_wdev)
	{
		rtw_wdev_unregister(if1->rtw_wdev);
		rtw_wdev_free(if1->rtw_wdev);
	}
#endif

#ifdef CONFIG_BT_COEXIST
	if(1 == if1->pwrctrlpriv.autopm_cnt){
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			usb_autopm_put_interface(adapter_to_dvobj(if1)->pusbintf);
		#else
			usb_autopm_enable(adapter_to_dvobj(if1)->pusbintf);
		#endif
		if1->pwrctrlpriv.autopm_cnt --;
	}
#endif

	rtw_free_drv_sw(if1);

	if(pnetdev)
		rtw_free_netdev(pnetdev);
}

static void dump_usb_interface(struct usb_interface *usb_intf)
{
	int	i;
	u8	val8;

	struct usb_device				*udev = interface_to_usbdev(usb_intf);
	struct usb_device_descriptor	*dev_desc = &udev->descriptor;

	struct usb_host_config			*act_conf = udev->actconfig;
	struct usb_config_descriptor	*act_conf_desc = &act_conf->desc;

	struct usb_host_interface		*host_iface;
	struct usb_interface_descriptor	*iface_desc;
	struct usb_host_endpoint		*host_endp;
	struct usb_endpoint_descriptor	*endp_desc;

#if 1 /* The usb device this usb interface belongs to */
	DBG_8723A("usb_interface:%p, usb_device:%p(num:%d, path:%s), usb_device_descriptor:%p\n", usb_intf, udev, udev->devnum, udev->devpath, dev_desc);
	DBG_8723A("bLength:%u\n", dev_desc->bLength);
	DBG_8723A("bDescriptorType:0x%02x\n", dev_desc->bDescriptorType);
	DBG_8723A("bcdUSB:0x%04x\n", le16_to_cpu(dev_desc->bcdUSB));
	DBG_8723A("bDeviceClass:0x%02x\n", dev_desc->bDeviceClass);
	DBG_8723A("bDeviceSubClass:0x%02x\n", dev_desc->bDeviceSubClass);
	DBG_8723A("bDeviceProtocol:0x%02x\n", dev_desc->bDeviceProtocol);
	DBG_8723A("bMaxPacketSize0:%u\n", dev_desc->bMaxPacketSize0);
	DBG_8723A("idVendor:0x%04x\n", le16_to_cpu(dev_desc->idVendor));
	DBG_8723A("idProduct:0x%04x\n", le16_to_cpu(dev_desc->idProduct));
	DBG_8723A("bcdDevice:0x%04x\n", le16_to_cpu(dev_desc->bcdDevice));
	DBG_8723A("iManufacturer:0x02%x\n", dev_desc->iManufacturer);
	DBG_8723A("iProduct:0x%02x\n", dev_desc->iProduct);
	DBG_8723A("iSerialNumber:0x%02x\n", dev_desc->iSerialNumber);
	DBG_8723A("bNumConfigurations:%u\n", dev_desc->bNumConfigurations);
#endif


#if 1 /* The acting usb_config_descriptor */
	DBG_8723A("\nact_conf_desc:%p\n", act_conf_desc);
	DBG_8723A("bLength:%u\n", act_conf_desc->bLength);
	DBG_8723A("bDescriptorType:0x%02x\n", act_conf_desc->bDescriptorType);
	DBG_8723A("wTotalLength:%u\n", le16_to_cpu(act_conf_desc->wTotalLength));
	DBG_8723A("bNumInterfaces:%u\n", act_conf_desc->bNumInterfaces);
	DBG_8723A("bConfigurationValue:0x%02x\n", act_conf_desc->bConfigurationValue);
	DBG_8723A("iConfiguration:0x%02x\n", act_conf_desc->iConfiguration);
	DBG_8723A("bmAttributes:0x%02x\n", act_conf_desc->bmAttributes);
	DBG_8723A("bMaxPower=%u\n", act_conf_desc->bMaxPower);
#endif


	DBG_8723A("****** num of altsetting = (%d) ******/\n", usb_intf->num_altsetting);
	/* Get he host side alternate setting (the current alternate setting) for this interface*/
	host_iface = usb_intf->cur_altsetting;
	iface_desc = &host_iface->desc;

#if 1 /* The current alternate setting*/
	DBG_8723A("\nusb_interface_descriptor:%p:\n", iface_desc);
	DBG_8723A("bLength:%u\n", iface_desc->bLength);
	DBG_8723A("bDescriptorType:0x%02x\n", iface_desc->bDescriptorType);
	DBG_8723A("bInterfaceNumber:0x%02x\n", iface_desc->bInterfaceNumber);
	DBG_8723A("bAlternateSetting=%x\n", iface_desc->bAlternateSetting);
	DBG_8723A("bNumEndpoints=%x\n", iface_desc->bNumEndpoints);
	DBG_8723A("bInterfaceClass=%x\n", iface_desc->bInterfaceClass);
	DBG_8723A("bInterfaceSubClass=%x\n", iface_desc->bInterfaceSubClass);
	DBG_8723A("bInterfaceProtocol=%x\n", iface_desc->bInterfaceProtocol);
	DBG_8723A("iInterface=%x\n", iface_desc->iInterface);
#endif


#if 1
	//DBG_8723A("\ndump usb_endpoint_descriptor:\n");

	for (i = 0; i < iface_desc->bNumEndpoints; i++)
	{
		host_endp = host_iface->endpoint + i;
		if (host_endp)
		{
			endp_desc = &host_endp->desc;

			DBG_8723A("\nusb_endpoint_descriptor(%d):\n", i);
			DBG_8723A("bLength=%x\n",endp_desc->bLength);
			DBG_8723A("bDescriptorType=%x\n",endp_desc->bDescriptorType);
			DBG_8723A("bEndpointAddress=%x\n",endp_desc->bEndpointAddress);
			DBG_8723A("bmAttributes=%x\n",endp_desc->bmAttributes);
			DBG_8723A("wMaxPacketSize=%x\n",endp_desc->wMaxPacketSize);
			DBG_8723A("wMaxPacketSize=%x\n",le16_to_cpu(endp_desc->wMaxPacketSize));
			DBG_8723A("bInterval=%x\n",endp_desc->bInterval);
			//DBG_8723A("bRefresh=%x\n",pendp_desc->bRefresh);
			//DBG_8723A("bSynchAddress=%x\n",pendp_desc->bSynchAddress);

			if (RT_usb_endpoint_is_bulk_in(endp_desc))
			{
				DBG_8723A("RT_usb_endpoint_is_bulk_in = %x\n", RT_usb_endpoint_num(endp_desc));
				//pdvobjpriv->RtNumInPipes++;
			}
			else if (RT_usb_endpoint_is_int_in(endp_desc))
			{
				DBG_8723A("RT_usb_endpoint_is_int_in = %x, Interval = %x\n", RT_usb_endpoint_num(endp_desc),endp_desc->bInterval);
				//pdvobjpriv->RtNumInPipes++;
			}
			else if (RT_usb_endpoint_is_bulk_out(endp_desc))
			{
				DBG_8723A("RT_usb_endpoint_is_bulk_out = %x\n", RT_usb_endpoint_num(endp_desc));
				//pdvobjpriv->RtNumOutPipes++;
			}
			//pdvobjpriv->ep_num[i] = RT_usb_endpoint_num(pendp_desc);
		}
	}

	//DBG_8723A("nr_endpoint=%d, in_num=%d, out_num=%d\n\n", pdvobjpriv->nr_endpoint, pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);
#endif

	if (udev->speed == USB_SPEED_HIGH)
		DBG_8723A("USB_SPEED_HIGH\n");
	else
		DBG_8723A("NON USB_SPEED_HIGH\n");

}


static int rtw_drv_init(struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	int i;
	struct rtw_adapter *if1 = NULL, *if2 = NULL;
	int status;
	struct dvobj_priv *dvobj;

	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("+rtw_drv_init\n"));
	//DBG_8723A("+rtw_drv_init\n");

	//step 0.
	process_spec_devid(pdid);

	/* Initialize dvobj_priv */
	if ((dvobj = usb_dvobj_init(pusb_intf)) == NULL) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("initialize device object priv Failed!\n"));
		goto exit;
	}

	#ifdef RTW_DVOBJ_CHIP_HW_TYPE
	decide_chip_type_by_usb_device_id(dvobj, pdid);
	#endif

	if ((if1 = rtw_usb_if1_init(dvobj, pusb_intf, pdid)) == NULL) {
		DBG_8723A("rtw_init_primary_adapter Failed!\n");
		goto free_dvobj;
	}

#ifdef CONFIG_CONCURRENT_MODE
	if((if2 = rtw_drv_if2_init(if1, usb_set_intf_ops)) == NULL) {
		goto free_if1;
	}
#endif

#ifdef CONFIG_GLOBAL_UI_PID
	if(ui_pid[1]!=0) {
		DBG_8723A("ui_pid[1]:%d\n",ui_pid[1]);
		rtw_signal_process(ui_pid[1], SIGUSR2);
	}
#endif

	//dev_alloc_name && register_netdev
	if((status = rtw_drv_register_netdev(if1)) != _SUCCESS) {
		goto free_if2;
	}

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_init(if1);
#endif

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-871x_drv - drv_init, success!\n"));

	status = _SUCCESS;

free_if2:
	if(status != _SUCCESS && if2) {
		#ifdef CONFIG_CONCURRENT_MODE
		rtw_drv_if2_stop(if2);
		rtw_drv_if2_free(if2);
		#endif
	}
free_if1:
	if (status != _SUCCESS && if1) {
		rtw_usb_if1_deinit(if1);
	}
free_dvobj:
	if (status != _SUCCESS)
		usb_dvobj_deinit(pusb_intf);
exit:
	return status == _SUCCESS?0:-ENODEV;
}

/*
 * dev_remove() - our device is being removed
*/
static void rtw_disconnect(struct usb_interface *pusb_intf)
{
	struct dvobj_priv *dvobj;
	struct rtw_adapter *padapter;
	struct net_device *pnetdev;
	struct mlme_priv *pmlmepriv;

	dvobj = usb_get_intfdata(pusb_intf);
	if (!dvobj)
		return;

	padapter = dvobj->if1;
	pnetdev = padapter->pnetdev;
	pmlmepriv= &padapter->mlmepriv;

	usb_set_intfdata(pusb_intf, NULL);

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+dev_remove()\n"));

	rtw_pm_set_ips(padapter, IPS_NONE);
	rtw_pm_set_lps(padapter, PS_MODE_ACTIVE);

	LeaveAllPowerSaveMode(padapter);

#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_if2_stop(dvobj->if2);
#endif
	rtw_usb_if1_deinit(padapter);

#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_if2_free(dvobj->if2);
#endif
	usb_dvobj_deinit(pusb_intf);

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-dev_remove()\n"));
	DBG_8723A("-r871xu_dev_remove, done\n");

	return;

}

extern int console_suspend_enabled;

static int __init rtw_drv_entry(void)
{
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+rtw_drv_entry\n"));

	rtw_suspend_lock_init();

	return usb_register(usb_drv);
}

static void __exit rtw_drv_halt(void)
{
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+rtw_drv_halt\n"));
	DBG_8723A("+rtw_drv_halt\n");

	rtw_suspend_lock_uninit();

	usb_deregister(usb_drv);

	DBG_8723A("-rtw_drv_halt\n");
}


module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);
