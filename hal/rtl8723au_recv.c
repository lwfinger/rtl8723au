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
#define _RTL8192CU_RECV_C_
#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <mlme_osdep.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
#include <ethernet.h>

#include <usb_ops.h>


#include <wifi.h>
#include <circ_buf.h>

//#include <rtl8192c_hal.h>
#include <rtl8723a_hal.h>

void rtl8192cu_init_recvbuf(struct rtw_adapter *padapter, struct recv_buf *precvbuf)
{

	precvbuf->transfer_len = 0;

	precvbuf->len = 0;

	precvbuf->ref_cnt = 0;

	if(precvbuf->pbuf)
	{
		precvbuf->pdata = precvbuf->phead = precvbuf->ptail = precvbuf->pbuf;
		precvbuf->pend = precvbuf->pdata + MAX_RECVBUF_SZ;
	}

}

int	rtl8192cu_init_recv_priv(struct rtw_adapter *padapter)
{
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	int	i, res = _SUCCESS;
	struct recv_buf *precvbuf;

#ifdef CONFIG_RECV_THREAD_MODE
	sema_init(&precvpriv->recv_sema, 0);//will be removed
	sema_init(&precvpriv->terminate_recvthread_sema, 0);//will be removed
#endif

	tasklet_init(&precvpriv->recv_tasklet,
	     (void *)rtl8192cu_recv_tasklet,
	     (unsigned long)padapter);

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	precvpriv->int_in_urb = usb_alloc_urb(0, GFP_KERNEL);
	if(precvpriv->int_in_urb == NULL){
		DBG_8723A("alloc_urb for interrupt in endpoint fail !!!!\n");
	}
	precvpriv->int_in_buf = kzalloc(USB_INTR_CONTENT_LENGTH, GFP_KERNEL);
	if(precvpriv->int_in_buf == NULL){
		DBG_8723A("alloc_mem for interrupt in endpoint fail !!!!\n");
	}
#endif

	//init recv_buf
	_rtw_init_queue(&precvpriv->free_recv_buf_queue);

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
	_rtw_init_queue(&precvpriv->recv_buf_pending_queue);
#endif	// CONFIG_USE_USB_BUFFER_ALLOC_RX

	precvpriv->pallocated_recv_buf = kzalloc(NR_RECVBUFF *sizeof(struct recv_buf) + 4, GFP_KERNEL);
	if(precvpriv->pallocated_recv_buf==NULL){
		res= _FAIL;
		RT_TRACE(_module_rtl871x_recv_c_,_drv_err_,("alloc recv_buf fail!\n"));
		goto exit;
	}

	precvpriv->precv_buf = PTR_ALIGN(precvpriv->pallocated_recv_buf, 4);

	precvbuf = (struct recv_buf*)precvpriv->precv_buf;

	for(i=0; i < NR_RECVBUFF ; i++)
	{
		INIT_LIST_HEAD(&precvbuf->list);

		spin_lock_init(&precvbuf->recvbuf_lock);

		precvbuf->alloc_sz = MAX_RECVBUF_SZ;

		res = rtw_os_recvbuf_resource_alloc(padapter, precvbuf);
		if(res==_FAIL)
			break;

		precvbuf->ref_cnt = 0;
		precvbuf->adapter =padapter;


		//list_add_tail(&precvbuf->list, &(precvpriv->free_recv_buf_queue.queue));

		precvbuf++;

	}

	precvpriv->free_recv_buf_queue_cnt = NR_RECVBUFF;

	skb_queue_head_init(&precvpriv->rx_skb_queue);

#ifdef CONFIG_PREALLOC_RECV_SKB
	{
		int i;
		unsigned long tmpaddr = 0;
		unsigned long alignment = 0;
		struct sk_buff *pskb = NULL;

		skb_queue_head_init(&precvpriv->free_recv_skb_queue);

		for(i=0; i<NR_PREALLOC_RECV_SKB; i++)
		{

	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)) // http://www.mail-archive.com/netdev@vger.kernel.org/msg17214.html
			pskb = __dev_alloc_skb(MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ, GFP_KERNEL);
	#else
			pskb = __netdev_alloc_skb(padapter->pnetdev, MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ, GFP_KERNEL);
	#endif

			if(pskb)
			{
				pskb->dev = padapter->pnetdev;

				tmpaddr = (unsigned long)pskb->data;
				alignment = tmpaddr & (RECVBUFF_ALIGN_SZ-1);
				skb_reserve(pskb, (RECVBUFF_ALIGN_SZ - alignment));

				skb_queue_tail(&precvpriv->free_recv_skb_queue, pskb);
			}

			pskb=NULL;

		}
	}
#endif

exit:

	return res;

}

void rtl8192cu_free_recv_priv (struct rtw_adapter *padapter)
{
	int	i;
	struct recv_buf	*precvbuf;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	precvbuf = (struct recv_buf *)precvpriv->precv_buf;

	for(i=0; i < NR_RECVBUFF ; i++)
	{
		rtw_os_recvbuf_resource_free(padapter, precvbuf);
		precvbuf++;
	}

	if(precvpriv->pallocated_recv_buf)
		kfree(precvpriv->pallocated_recv_buf);

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	if(precvpriv->int_in_urb)
	{
		usb_free_urb(precvpriv->int_in_urb);
	}
	if(precvpriv->int_in_buf)
		kfree(precvpriv->int_in_buf);
#endif

	if (skb_queue_len(&precvpriv->rx_skb_queue)) {
		DBG_8723A(KERN_WARNING "rx_skb_queue not empty\n");
	}

	skb_queue_purge(&precvpriv->rx_skb_queue);

#ifdef CONFIG_PREALLOC_RECV_SKB

	if (skb_queue_len(&precvpriv->free_recv_skb_queue)) {
		DBG_8723A(KERN_WARNING "free_recv_skb_queue not empty, %d\n", skb_queue_len(&precvpriv->free_recv_skb_queue));
	}

	skb_queue_purge(&precvpriv->free_recv_skb_queue);

#endif
}


void update_recvframe_attrib(
	union recv_frame *precvframe,
	struct recv_stat *prxstat)
{
	struct rx_pkt_attrib	*pattrib;
	struct recv_stat	report;
	PRXREPORT		prxreport;


	report.rxdw0 = le32_to_cpu(prxstat->rxdw0);
	report.rxdw1 = le32_to_cpu(prxstat->rxdw1);
	report.rxdw2 = le32_to_cpu(prxstat->rxdw2);
	report.rxdw3 = le32_to_cpu(prxstat->rxdw3);
	report.rxdw4 = le32_to_cpu(prxstat->rxdw4);
	report.rxdw5 = le32_to_cpu(prxstat->rxdw5);

	prxreport = (PRXREPORT)&report;

	pattrib = &precvframe->u.hdr.attrib;
	memset(pattrib, 0, sizeof(struct rx_pkt_attrib));

	// update rx report to recv_frame attribute
	pattrib->pkt_len = (u16)prxreport->pktlen;
	pattrib->drvinfo_sz = (u8)(prxreport->drvinfosize << 3);
	pattrib->physt = (u8)prxreport->physt;

	pattrib->crc_err = (u8)prxreport->crc32;
	pattrib->icv_err = (u8)prxreport->icverr;

	pattrib->bdecrypted = (u8)(prxreport->swdec ? 0 : 1);
	pattrib->encrypt = (u8)prxreport->security;

	pattrib->qos = (u8)prxreport->qos;
	pattrib->priority = (u8)prxreport->tid;

	pattrib->amsdu = (u8)prxreport->amsdu;

	pattrib->seq_num = (u16)prxreport->seq;
	pattrib->frag_num = (u8)prxreport->frag;
	pattrib->mfrag = (u8)prxreport->mf;
	pattrib->mdata = (u8)prxreport->md;

	pattrib->mcs_rate = (u8)prxreport->rxmcs;
	pattrib->rxht = (u8)prxreport->rxht;
}



/*
 * Notice:
 *	Before calling this function,
 *	precvframe->u.hdr.rx_data should be ready!
 */
void update_recvframe_phyinfo(
	union recv_frame	*precvframe,
	struct phy_stat *pphy_status)
{
	struct rtw_adapter *	padapter = precvframe->u.hdr.adapter;
	struct rx_pkt_attrib	*pattrib = &precvframe->u.hdr.attrib;
	HAL_DATA_TYPE		*pHalData= GET_HAL_DATA(padapter);
	PODM_PHY_INFO_T		pPHYInfo  = (PODM_PHY_INFO_T)(&pattrib->phy_info);
	u8					*wlanhdr;
	ODM_PACKET_INFO_T	pkt_info;
	u8 *sa;
	//_irqL		irqL;
	struct sta_priv *pstapriv;
	struct sta_info *psta;

	pkt_info.bPacketMatchBSSID =_FALSE;
	pkt_info.bPacketToSelf = _FALSE;
	pkt_info.bPacketBeacon = _FALSE;

	wlanhdr = get_recvframe_data(precvframe);

	pkt_info.bPacketMatchBSSID = ((!IsFrameTypeCtrl(wlanhdr)) &&
		!pattrib->icv_err && !pattrib->crc_err &&
		!memcmp(get_hdr_bssid(wlanhdr), get_bssid(&padapter->mlmepriv), ETH_ALEN));

	pkt_info.bPacketToSelf = pkt_info.bPacketMatchBSSID && (!memcmp(get_da(wlanhdr), myid(&padapter->eeprompriv), ETH_ALEN));

	pkt_info.bPacketBeacon = pkt_info.bPacketMatchBSSID && (GetFrameSubType(wlanhdr) == WIFI_BEACON);

	pkt_info.StationID = 0xFF;
	if(pkt_info.bPacketBeacon){
		if(check_fwstate(&padapter->mlmepriv, WIFI_STATION_STATE) == _TRUE){
			sa = padapter->mlmepriv.cur_network.network.MacAddress;
		}
		//to do Ad-hoc
	}
	else{
		sa = get_sa(wlanhdr);
	}

	pstapriv = &padapter->stapriv;
	psta = rtw_get_stainfo(pstapriv, sa);
	if (psta)
	{
             pkt_info.StationID = psta->mac_id;
		//printk("%s ==> StationID(%d)\n",__FUNCTION__,pkt_info.StationID);
	}
	pkt_info.Rate = pattrib->mcs_rate;

	#ifdef CONFIG_CONCURRENT_MODE
	//get Primary adapter's odmpriv
	if(padapter->adapter_type > PRIMARY_ADAPTER){
		pHalData = GET_HAL_DATA(padapter->pbuddy_adapter);
	}
	#endif

	//rtl8192c_query_rx_phy_status(precvframe, pphy_status);
	//spin_lock_bh(&pHalData->odm_stainfo_lock);
	 ODM_PhyStatusQuery(&pHalData->odmpriv,pPHYInfo,(u8 *)pphy_status,&(pkt_info));
	//spin_unlock_bh(&pHalData->odm_stainfo_lock);
	precvframe->u.hdr.psta = NULL;
	if (pkt_info.bPacketMatchBSSID &&
		(check_fwstate(&padapter->mlmepriv, WIFI_AP_STATE) == _TRUE))
	{
		if (psta)
		{
			precvframe->u.hdr.psta = psta;
			rtl8192c_process_phy_info(padapter, precvframe);
                }
	}
	else if (pkt_info.bPacketToSelf || pkt_info.bPacketBeacon)
	{
		if (check_fwstate(&padapter->mlmepriv, WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE) == _TRUE)
		{
			if (psta)
			{
				precvframe->u.hdr.psta = psta;
			}
		}

		rtl8192c_process_phy_info(padapter, precvframe);
	}
}
