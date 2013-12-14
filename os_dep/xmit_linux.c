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
#define _XMIT_OSDEP_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>

#include <linux/if_ether.h>
#include <linux/ip.h>
#include <wifi.h>
#include <mlme_osdep.h>
#include <xmit_osdep.h>
#include <osdep_intf.h>
#include <circ_buf.h>

uint rtw_remainder_len(struct pkt_file *pfile)
{
	return (pfile->buf_len - ((unsigned long)(pfile->cur_addr) - (unsigned long)(pfile->buf_start)));
}

void _rtw_open_pktfile (struct sk_buff *pktptr, struct pkt_file *pfile)
{
_func_enter_;

	pfile->pkt = pktptr;
	pfile->cur_addr = pfile->buf_start = pktptr->data;
	pfile->pkt_len = pfile->buf_len = pktptr->len;

	pfile->cur_buffer = pfile->buf_start ;

_func_exit_;
}

uint _rtw_pktfile_read (struct pkt_file *pfile, u8 *rmem, uint rlen)
{
	uint	len = 0;

_func_enter_;

       len =  rtw_remainder_len(pfile);
	len = (rlen > len)? len: rlen;

       if(rmem)
	  skb_copy_bits(pfile->pkt, pfile->buf_len-pfile->pkt_len, rmem, len);

       pfile->cur_addr += len;
       pfile->pkt_len -= len;

_func_exit_;

	return len;
}

int rtw_endofpktfile(struct pkt_file *pfile)
{
_func_enter_;

	if (pfile->pkt_len == 0) {
_func_exit_;
		return _TRUE;
	}

_func_exit_;

	return _FALSE;
}

void rtw_set_tx_chksum_offload(struct sk_buff *pkt, struct pkt_attrib *pattrib)
{

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	struct sk_buff *skb = (struct sk_buff *)pkt;
	pattrib->hw_tcp_csum = 0;

	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		if (skb_shinfo(skb)->nr_frags == 0)
		{
                        const struct iphdr *ip = ip_hdr(skb);
                        if (ip->protocol == IPPROTO_TCP) {
                                // TCP checksum offload by HW
                                DBG_8723A("CHECKSUM_PARTIAL TCP\n");
                                pattrib->hw_tcp_csum = 1;
                        } else if (ip->protocol == IPPROTO_UDP) {
                                skb_checksum_help(skb);
                        } else {
				DBG_8723A("%s-%d TCP CSUM offload Error!!\n", __FUNCTION__, __LINE__);
                                WARN_ON(1);     /* we need a WARN() */
			    }
		}
		else { // IP fragmentation case
			DBG_8723A("%s-%d nr_frags != 0, using skb_checksum_help(skb);!!\n", __FUNCTION__, __LINE__);
			skb_checksum_help(skb);
		}
	}
#endif

}

int rtw_os_xmit_resource_alloc(struct rtw_adapter *padapter, struct xmit_buf *pxmitbuf,u32 alloc_sz)
{
	int i;
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(padapter);
	struct usb_device	*pusbd = pdvobjpriv->pusbdev;

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_TX
	pxmitbuf->pallocated_buf = rtw_usb_buffer_alloc(pusbd, (size_t)alloc_sz, &pxmitbuf->dma_transfer_addr);
	pxmitbuf->pbuf = pxmitbuf->pallocated_buf;
	if(pxmitbuf->pallocated_buf == NULL)
		return _FAIL;
#else // CONFIG_USE_USB_BUFFER_ALLOC_TX

	pxmitbuf->pallocated_buf = kzalloc(alloc_sz, GFP_KERNEL);
	if (pxmitbuf->pallocated_buf == NULL) {
		return _FAIL;
	}

	pxmitbuf->pbuf = PTR_ALIGN(pxmitbuf->pallocated_buf, XMITBUF_ALIGN_SZ);
	pxmitbuf->dma_transfer_addr = 0;

#endif // CONFIG_USE_USB_BUFFER_ALLOC_TX

	for(i=0; i<8; i++)
	{
		pxmitbuf->pxmit_urb[i] = usb_alloc_urb(0, GFP_KERNEL);
		if(pxmitbuf->pxmit_urb[i] == NULL)
		{
			DBG_8723A("pxmitbuf->pxmit_urb[i]==NULL");
			return _FAIL;
		}

	}
	return _SUCCESS;
}

void rtw_os_xmit_resource_free(struct rtw_adapter *padapter, struct xmit_buf *pxmitbuf,u32 free_sz)
{
	int i;
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(padapter);
	struct usb_device	*pusbd = pdvobjpriv->pusbdev;

	for(i=0; i<8; i++)
	{
		if(pxmitbuf->pxmit_urb[i])
		{
			//usb_kill_urb(pxmitbuf->pxmit_urb[i]);
			usb_free_urb(pxmitbuf->pxmit_urb[i]);
		}
	}

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_TX
	rtw_usb_buffer_free(pusbd, (size_t)free_sz, pxmitbuf->pallocated_buf, pxmitbuf->dma_transfer_addr);
	pxmitbuf->pallocated_buf =  NULL;
	pxmitbuf->dma_transfer_addr = 0;
#else	// CONFIG_USE_USB_BUFFER_ALLOC_TX
	if(pxmitbuf->pallocated_buf)
		kfree(pxmitbuf->pallocated_buf);
#endif	// CONFIG_USE_USB_BUFFER_ALLOC_TX

}

#define WMM_XMIT_THRESHOLD	(NR_XMITFRAME*2/5)

void rtw_os_pkt_complete(struct rtw_adapter *padapter, struct sk_buff *pkt)
{
	u16	queue;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	queue = skb_get_queue_mapping(pkt);
	if (padapter->registrypriv.wifi_spec) {
		if(__netif_subqueue_stopped(padapter->pnetdev, queue) &&
			(pxmitpriv->hwxmits[queue].accnt < WMM_XMIT_THRESHOLD))
		{
			netif_wake_subqueue(padapter->pnetdev, queue);
		}
	} else {
		if(__netif_subqueue_stopped(padapter->pnetdev, queue))
			netif_wake_subqueue(padapter->pnetdev, queue);
	}
	dev_kfree_skb_any(pkt);
}

void rtw_os_xmit_complete(struct rtw_adapter *padapter, struct xmit_frame *pxframe)
{
	if(pxframe->pkt)
	{
		//RT_TRACE(_module_xmit_osdep_c_,_drv_err_,("linux : rtw_os_xmit_complete, dev_kfree_skb()\n"));

		//dev_kfree_skb_any(pxframe->pkt);
		rtw_os_pkt_complete(padapter, pxframe->pkt);

	}

	pxframe->pkt = NULL;
}

void rtw_os_xmit_schedule(struct rtw_adapter *padapter)
{
	struct rtw_adapter *pri_adapter = padapter;
	struct xmit_priv *pxmitpriv;

	if(!padapter)
		return;

	pxmitpriv = &padapter->xmitpriv;

	spin_lock_bh(&pxmitpriv->lock);

	if(rtw_txframes_pending(padapter))
	{
		tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
	}

	spin_unlock_bh(&pxmitpriv->lock);
}

static void rtw_check_xmit_resource(struct rtw_adapter *padapter, struct sk_buff *pkt)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	u16	queue;

	queue = skb_get_queue_mapping(pkt);
	if (padapter->registrypriv.wifi_spec) {
		/* No free space for Tx, tx_worker is too slow */
		if (pxmitpriv->hwxmits[queue].accnt > WMM_XMIT_THRESHOLD) {
			//DBG_8723A("%s(): stop netif_subqueue[%d]\n", __FUNCTION__, queue);
			netif_stop_subqueue(padapter->pnetdev, queue);
		}
	} else {
		if(pxmitpriv->free_xmitframe_cnt<=4) {
			if (!netif_tx_queue_stopped(netdev_get_tx_queue(padapter->pnetdev, queue)))
				netif_stop_subqueue(padapter->pnetdev, queue);
		}
	}
}

int rtw_xmit_entry(struct sk_buff *pkt, struct net_device *pnetdev)
{
	struct rtw_adapter *padapter = netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	s32 res = 0;
	u16 queue;

_func_enter_;

	RT_TRACE(_module_rtl871x_mlme_c_, _drv_info_, ("+xmit_enry\n"));

	if (rtw_if_up(padapter) == _FALSE) {
		RT_TRACE(_module_xmit_osdep_c_, _drv_err_, ("rtw_xmit_entry: rtw_if_up fail\n"));
		#ifdef DBG_TX_DROP_FRAME
		DBG_8723A("DBG_TX_DROP_FRAME %s if_up fail\n", __FUNCTION__);
		#endif
		goto drop_packet;
	}

	rtw_check_xmit_resource(padapter, pkt);

	res = rtw_xmit(padapter, &pkt);
	if (res < 0) {
		#ifdef DBG_TX_DROP_FRAME
		DBG_8723A("DBG_TX_DROP_FRAME %s rtw_xmit fail\n", __FUNCTION__);
		#endif
		goto drop_packet;
	}

	pxmitpriv->tx_pkts++;
	RT_TRACE(_module_xmit_osdep_c_, _drv_info_, ("rtw_xmit_entry: tx_pkts=%d\n", (u32)pxmitpriv->tx_pkts));
	goto exit;

drop_packet:
	pxmitpriv->tx_drop++;
	dev_kfree_skb_any(pkt);
	RT_TRACE(_module_xmit_osdep_c_, _drv_notice_, ("rtw_xmit_entry: drop, tx_drop=%d\n", (u32)pxmitpriv->tx_drop));

exit:

_func_exit_;

	return 0;
}
