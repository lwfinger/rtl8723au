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
#define _HCI_OPS_OS_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <osdep_intf.h>
#include <usb_ops.h>
#include <circ_buf.h>
#include <recv_osdep.h>
//#include <rtl8192c_hal.h>
#include <rtl8723a_hal.h>
#include <rtl8723a_recv.h>

static int usbctrl_vendorreq(struct intf_hdl *pintfhdl, u8 request, u16 value, u16 index, void *pdata, u16 len, u8 requesttype)
{
	struct rtw_adapter		*padapter = pintfhdl->padapter ;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct usb_device *udev=pdvobjpriv->pusbdev;

	unsigned int pipe;
	int status = 0;
	u32 tmp_buflen=0;
	u8 reqtype;
	u8 *pIo_buf;
	int vendorreq_times = 0;

#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_DYNAMIC_ALLOCATE
	u8 *tmp_buf;
#else // use stack memory
	u8 tmp_buf[MAX_USB_IO_CTL_SIZE];
#endif

#ifdef CONFIG_CONCURRENT_MODE
	if(padapter->adapter_type > PRIMARY_ADAPTER)
	{
		padapter = padapter->pbuddy_adapter;
		pdvobjpriv = adapter_to_dvobj(padapter);
		udev = pdvobjpriv->pusbdev;
	}
#endif


	//DBG_8723A("%s %s:%d\n",__FUNCTION__, current->comm, current->pid);

	if((padapter->bSurpriseRemoved) ||(padapter->pwrctrlpriv.pnp_bstop_trx)){
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usbctrl_vendorreq:(padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n"));
		status = -EPERM;
		goto exit;
	}

	if(len>MAX_VENDOR_REQ_CMD_SIZE){
		DBG_8723A( "[%s] Buffer len error ,vendor request failed\n", __FUNCTION__ );
		status = -EINVAL;
		goto exit;
	}

	#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	mutex_lock(&pdvobjpriv->usb_vendor_req_mutex);
	#endif


	// Acquire IO memory for vendorreq
#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	pIo_buf = pdvobjpriv->usb_vendor_req_buf;
#else
#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_DYNAMIC_ALLOCATE
	tmp_buf = kmalloc((u32)len + ALIGNMENT_UNIT, GFP_KERNEL);
	tmp_buflen =  (u32)len + ALIGNMENT_UNIT;
#else // use stack memory
	tmp_buflen = MAX_USB_IO_CTL_SIZE;
#endif

	// Added by Albert 2010/02/09
	// For mstar platform, mstar suggests the address for USB IO should be 16 bytes alignment.
	// Trying to fix it here.
	pIo_buf = (tmp_buf==NULL)?NULL:tmp_buf + ALIGNMENT_UNIT -((unsigned long)(tmp_buf) & 0x0f );
#endif

	if ( pIo_buf== NULL) {
		DBG_8723A( "[%s] pIo_buf == NULL \n", __FUNCTION__ );
		status = -ENOMEM;
		goto release_mutex;
	}

	while(++vendorreq_times<= MAX_USBCTRL_VENDORREQ_TIMES)
	{
		memset(pIo_buf, 0, len);

		if (requesttype == 0x01)
		{
			pipe = usb_rcvctrlpipe(udev, 0);//read_in
			reqtype =  REALTEK_USB_VENQT_READ;
		}
		else
		{
			pipe = usb_sndctrlpipe(udev, 0);//write_out
			reqtype =  REALTEK_USB_VENQT_WRITE;
			memcpy(pIo_buf, pdata, len);
		}

		status = rtw_usb_control_msg(udev, pipe, request, reqtype, value, index, pIo_buf, len, RTW_USB_CONTROL_MSG_TIMEOUT);

		if ( status == len)   // Success this control transfer.
		{
			rtw_reset_continual_urb_error(pdvobjpriv);
			if ( requesttype == 0x01 )
			{   // For Control read transfer, we have to copy the read data from pIo_buf to pdata.
				memcpy(pdata, pIo_buf,  len );
			}
		}
		else { // error cases
			DBG_8723A("reg 0x%x, usb %s %u fail, status:%d value=0x%x, vendorreq_times:%d\n"
				, value,(requesttype == 0x01)?"read":"write" , len, status, *(u32*)pdata, vendorreq_times);

			if (status < 0) {
				if(status == (-ESHUTDOWN)	|| status == -ENODEV	)
				{
					padapter->bSurpriseRemoved = _TRUE;
				} else {
					#ifdef DBG_CONFIG_ERROR_DETECT
					{
						HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
						pHalData->srestpriv.Wifi_Error_Status = USB_VEN_REQ_CMD_FAIL;
					}
					#endif
				}
			}
			else // status != len && status >= 0
			{
				if(status > 0) {
					if ( requesttype == 0x01 )
					{   // For Control read transfer, we have to copy the read data from pIo_buf to pdata.
						memcpy(pdata, pIo_buf,  len );
					}
				}
			}

			if(rtw_inc_and_chk_continual_urb_error(pdvobjpriv) == _TRUE ){
				padapter->bSurpriseRemoved = _TRUE;
				break;
			}

		}

		// firmware download is checksumed, don't retry
		if( (value >= FW_8723A_START_ADDRESS && value <= FW_8723A_END_ADDRESS) || status == len )
			break;

	}

	// release IO memory used by vendorreq
#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_DYNAMIC_ALLOCATE
	kfree(tmp_buf);
#endif

release_mutex:
	#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	mutex_unlock(&pdvobjpriv->usb_vendor_req_mutex);
	#endif
exit:
	return status;

}

static u8 usb_read8(struct intf_hdl *pintfhdl, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 data=0;

	_func_enter_;

	request = 0x05;
	requesttype = 0x01;//read_in
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	_func_exit_;

	return data;

}

static u16 usb_read16(struct intf_hdl *pintfhdl, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u16 data=0;

	_func_enter_;

	request = 0x05;
	requesttype = 0x01;//read_in
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	_func_exit_;

	return data;

}

static u32 usb_read32(struct intf_hdl *pintfhdl, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data=0;

	_func_enter_;

	request = 0x05;
	requesttype = 0x01;//read_in
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	_func_exit_;

	return data;

}

static int usb_write8(struct intf_hdl *pintfhdl, u32 addr, u8 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 data;
	int ret;

	_func_enter_;

	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;

	data = val;

	 ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	_func_exit_;

	return ret;

}

static int usb_write16(struct intf_hdl *pintfhdl, u32 addr, u16 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u16 data;
	int ret;

	_func_enter_;

	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;

	data = val;

	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	_func_exit_;

	return ret;

}

static int usb_write32(struct intf_hdl *pintfhdl, u32 addr, u32 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;
	int ret;

	_func_enter_;

	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;
	data = val;


	ret =usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	_func_exit_;

	return ret;

}

static int usb_writeN(struct intf_hdl *pintfhdl, u32 addr, u32 length, u8 *pdata)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 buf[VENDOR_CMD_MAX_DATA_LEN]={0};
	int ret;

	_func_enter_;

	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = length;
	 memcpy(buf, pdata, len );

	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, buf, len, requesttype);

	_func_exit_;

	return ret;

}

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
//
// Description:
//	Recognize the interrupt content by reading the interrupt register or content and masking interrupt mask (IMR)
//	if it is our NIC's interrupt. After recognizing, we may clear the all interrupts (ISR).
// Arguments:
//	[in] Adapter -
//		The adapter context.
//	[in] pContent -
//		Under PCI interface, this field is ignord.
//		Under USB interface, the content is the interrupt content pointer.
//		Under SDIO interface, this is the interrupt type which is Local interrupt or system interrupt.
//	[in] ContentLen -
//		The length in byte of pContent.
// Return:
//	If any interrupt matches the mask (IMR), return TRUE, and return FALSE otherwise.
//
bool
InterruptRecognized8723AU(struct rtw_adapter *Adapter, void *pContent,
			  u32 ContentLen)
{
	HAL_DATA_TYPE	*pHalData=GET_HAL_DATA(Adapter);
	u8 *			buffer = (u8 *)pContent;
//	RT_PRINT_DATA(COMP_RECV, DBG_LOUD, ("InterruptRecognized8723AU Interrupt buffer \n"), buffer, MAX_RECEIVE_INTERRUPT_BUFFER_SIZE(Adapter));

	memcpy(&(pHalData->IntArray[0]), &(buffer[USB_INTR_CONTENT_HISR_OFFSET]), 4);
//	PlatformMoveMemory(&(pHalData->IntArray[0]), &(buffer[USB_INTR_CONTENT_HISR_OFFSET]), sizeof(u32));
//	DBG_8723A("InterruptRecognized8723AU HISR = 0x%x HIMR = 0x%x\n", pHalData->IntArray[0],pHalData->IntrMask[0]);
	pHalData->IntArray[0] &= pHalData->IntrMask[0];

	//For HISR extension. Added by tynli. 2009.10.07.
	memcpy(&(pHalData->IntArray[1]), &(buffer[USB_INTR_CONTENT_HISRE_OFFSET]), 4);
//	PlatformMoveMemory(&(pHalData->IntArray[1]), &(buffer[USB_INTR_CONTENT_HISRE_OFFSET]), sizeof(u32));
//	DBG_8723A("InterruptRecognized8192CUsb HISRE = 0x%x HIMRE = 0x%x\n", pHalData->IntArray[1], pHalData->IntrMask[1]);
	pHalData->IntArray[1] &= pHalData->IntrMask[1];

	// We sholud remove this function later because DDK suggest not to executing too many operations in MPISR
//	if(pHalData->IntArray[0] != 0)
//		LogInterruptHistory8723AU(Adapter);

	{
		struct reportpwrstate_parm report;
		memcpy(&report.state, &(buffer[USB_INTR_CPWM_OFFSET]), 1);
#ifdef CONFIG_LPS_LCLK
		if( ((pHalData->IntArray[0])&UHIMR_CPWM)){
//			DBG_8723A("%s HIMR=0x%x\n",__func__,pHalData->IntArray[0]);
			//cpwm_int_hdl(Adapter, &report);
			schedule_work(&Adapter->pwrctrlpriv.cpwm_event);
			pHalData->IntArray[0]&= ~UHIMR_CPWM;
//			DBG_8723A("%s HIMR=0x%x\n",__func__,pHalData->IntArray[0]);
		}
#endif
	}
	return (((pHalData->IntArray[0])&pHalData->IntrMask[0])!=0 ||
		((pHalData->IntArray[1])&pHalData->IntrMask[1])!=0);

}


static void usb_read_interrupt_complete(struct urb *purb, struct pt_regs *regs)
{
	int	err;
	struct rtw_adapter *padapter = (struct rtw_adapter *)purb->context;


	if(padapter->bSurpriseRemoved || padapter->bDriverStopped||padapter->bReadPortCancel)
	{
		DBG_8723A("%s() RX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bReadPortCancel(%d)\n",
		__FUNCTION__,padapter->bDriverStopped, padapter->bSurpriseRemoved,padapter->bReadPortCancel);
		return;
	}

	if (purb->status == 0)//SUCCESS
	{
		struct c2h_evt_hdr *c2h_evt = (struct c2h_evt_hdr *)purb->transfer_buffer;

		if (purb->actual_length > USB_INTR_CONTENT_LENGTH) {
			DBG_8723A("usb_read_interrupt_complete: purb->actual_length > USB_INTR_CONTENT_LENGTH\n");
			goto urb_submit;
		}

		InterruptRecognized8723AU(padapter, purb->transfer_buffer, purb->actual_length);

		if (c2h_evt_exist(c2h_evt)) {
			if (0)
				DBG_8723A("%s C2H == %d\n", __func__, c2h_evt->id);
			if (c2h_id_filter_ccx_8723a(c2h_evt->id)) {
				/* Handle CCX report here */
				handle_txrpt_ccx_8723a(padapter, (void *)(c2h_evt->payload));
				/* Replace with special pointer to trigger c2h_evt_clear */
				if (rtw_cbuf_push(padapter->evtpriv.c2h_queue, (void*)&padapter->evtpriv) != _SUCCESS)
					DBG_8723A("%s rtw_cbuf_push fail\n", __func__);
				schedule_work(&padapter->evtpriv.c2h_wk);
			} else if ((c2h_evt = (struct c2h_evt_hdr *)rtw_malloc(16)) != NULL) {
				memcpy(c2h_evt, purb->transfer_buffer, 16);
				if (rtw_cbuf_push(padapter->evtpriv.c2h_queue, (void*)c2h_evt) != _SUCCESS)
					DBG_8723A("%s rtw_cbuf_push fail\n", __func__);
				schedule_work(&padapter->evtpriv.c2h_wk);
			} else {
				/* Error handling for malloc fail */
				if (rtw_cbuf_push(padapter->evtpriv.c2h_queue, (void*)NULL) != _SUCCESS)
					DBG_8723A("%s rtw_cbuf_push fail\n", __func__);
				schedule_work(&padapter->evtpriv.c2h_wk);
			}
		}

urb_submit:
		err = usb_submit_urb(purb, GFP_ATOMIC);
		if ((err) && (err != (-EPERM)))
		{
			DBG_8723A("cannot submit interrupt in-token(err = 0x%08x),urb_status = %d\n",err, purb->status);
		}
	}
	else
	{
		DBG_8723A("###=> usb_read_interrupt_complete => urb status(%d)\n", purb->status);

		switch (purb->status)
		{
			case -EINVAL:
			case -EPIPE:
			case -ENODEV:
			case -ESHUTDOWN:
				//padapter->bSurpriseRemoved = _TRUE;
				RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete:bSurpriseRemoved=TRUE\n"));
			case -ENOENT:
				padapter->bDriverStopped = _TRUE;
				RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete:bDriverStopped=TRUE\n"));
				break;
			case -EPROTO:
				break;
			case -EINPROGRESS:
				DBG_8723A("ERROR: URB IS IN PROGRESS!/n");
				break;
			default:
				break;
		}
	}
}

static u32 usb_read_interrupt(struct intf_hdl *pintfhdl, u32 addr)
{
	int	err;
	unsigned int pipe;
	u32	ret = _SUCCESS;
	struct rtw_adapter			*adapter = pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct recv_priv	*precvpriv = &adapter->recvpriv;
	struct usb_device	*pusbd = pdvobj->pusbdev;

_func_enter_;

	//translate DMA FIFO addr to pipehandle
	pipe = ffaddr2pipehdl(pdvobj, addr);

	usb_fill_int_urb(precvpriv->int_in_urb, pusbd, pipe,
					precvpriv->int_in_buf,
					USB_INTR_CONTENT_LENGTH,
					usb_read_interrupt_complete,
					adapter,
					1);

	err = usb_submit_urb(precvpriv->int_in_urb, GFP_ATOMIC);
	if((err) && (err != (-EPERM)))
	{
		DBG_8723A("cannot submit interrupt in-token(err = 0x%08x),urb_status = %d\n",err, precvpriv->int_in_urb->status);
		ret = _FAIL;
	}

_func_exit_;

	return ret;
}
#endif

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
static int recvbuf2recvframe(struct rtw_adapter *padapter, struct recv_buf *precvbuf)
{
	u8	*pbuf;
	u8	shift_sz = 0;
	u16	pkt_cnt, drvinfo_sz;
	u32	pkt_offset, skb_len, alloc_sz;
	s32	transfer_len;
	struct recv_stat	*prxstat;
	struct phy_stat	*pphy_info = NULL;
	struct sk_buff		*pkt_copy = NULL;
	union recv_frame	*precvframe = NULL;
	struct rx_pkt_attrib	*pattrib = NULL;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(padapter);
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	_queue			*pfree_recv_queue = &precvpriv->free_recv_queue;


	transfer_len = (s32)precvbuf->transfer_len;
	pbuf = precvbuf->pbuf;

	prxstat = (struct recv_stat *)pbuf;
	pkt_cnt = (le32_to_cpu(prxstat->rxdw2)>>16) & 0xff;

	do{
		RT_TRACE(_module_rtl871x_recv_c_, _drv_info_,
			 ("recvbuf2recvframe: rxdesc=offsset 0:0x%08x, 4:0x%08x, 8:0x%08x, C:0x%08x\n",
			  prxstat->rxdw0, prxstat->rxdw1, prxstat->rxdw2, prxstat->rxdw4));

		prxstat = (struct recv_stat *)pbuf;

		precvframe = rtw_alloc_recvframe(pfree_recv_queue);
		if(precvframe==NULL)
		{
			RT_TRACE(_module_rtl871x_recv_c_,_drv_err_,("recvbuf2recvframe: precvframe==NULL\n"));
			DBG_8723A("%s()-%d: rtw_alloc_recvframe() failed! RX Drop!\n", __FUNCTION__, __LINE__);
			goto _exit_recvbuf2recvframe;
		}

		INIT_LIST_HEAD(&precvframe->u.hdr.list);
		precvframe->u.hdr.precvbuf = NULL;	//can't access the precvbuf for new arch.
		precvframe->u.hdr.len=0;

//		rtl8192c_query_rx_desc_status(precvframe, prxstat);
		update_recvframe_attrib(precvframe, prxstat);

		pattrib = &precvframe->u.hdr.attrib;

		if(pattrib->crc_err){
			DBG_8723A("%s()-%d: RX Warning! rx CRC ERROR !!\n", __FUNCTION__, __LINE__);
			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		pkt_offset = RXDESC_SIZE + pattrib->drvinfo_sz + pattrib->shift_sz + pattrib->pkt_len;

		if((pattrib->pkt_len<=0) || (pkt_offset>transfer_len))
		{
			RT_TRACE(_module_rtl871x_recv_c_,_drv_info_,("recvbuf2recvframe: pkt_len<=0\n"));
			DBG_8723A("%s()-%d: RX Warning!\n", __FUNCTION__, __LINE__);
			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		//	Modified by Albert 20101213
		//	For 8 bytes IP header alignment.
		if (pattrib->qos)	//	Qos data, wireless lan header length is 26
		{
			shift_sz = 6;
		}
		else
		{
			shift_sz = 0;
		}

		skb_len = pattrib->pkt_len;

		// for first fragment packet, driver need allocate 1536+drvinfo_sz+RXDESC_SIZE to defrag packet.
		// modify alloc_sz for recvive crc error packet by thomas 2011-06-02
		if((pattrib->mfrag == 1)&&(pattrib->frag_num == 0)){
			//alloc_sz = 1664;	//1664 is 128 alignment.
			if(skb_len <= 1650)
				alloc_sz = 1664;
			else
				alloc_sz = skb_len + 14;
		}
		else {
			alloc_sz = skb_len;
			//	6 is for IP header 8 bytes alignment in QoS packet case.
			//	8 is for skb->data 4 bytes alignment.
			alloc_sz += 14;
		}

		pkt_copy = netdev_alloc_skb(padapter->pnetdev, alloc_sz);
		if(pkt_copy)
		{
			pkt_copy->dev = padapter->pnetdev;
			precvframe->u.hdr.pkt = pkt_copy;
			precvframe->u.hdr.rx_head = pkt_copy->data;
			precvframe->u.hdr.rx_end = pkt_copy->data + alloc_sz;
			skb_reserve(pkt_copy, 8 - ((unsigned long)(pkt_copy->data) & 7));//force pkt_copy->data at 8-byte alignment address
			skb_reserve( pkt_copy, shift_sz );//force ip_hdr at 8-byte alignment address according to shift_sz.
			memcpy(pkt_copy->data, (pbuf + pattrib->shift_sz + pattrib->drvinfo_sz + RXDESC_SIZE), skb_len);
			precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pkt_copy->data;
		}
		else
		{
			DBG_8723A("recvbuf2recvframe:can not allocate memory for skb copy\n");
			//precvframe->u.hdr.pkt = skb_clone(pskb, GFP_ATOMIC);
			//precvframe->u.hdr.rx_head = precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pbuf;
			//precvframe->u.hdr.rx_end = pbuf + (pkt_offset>1612?pkt_offset:1612);

			precvframe->u.hdr.pkt = NULL;
			rtw_free_recvframe(precvframe, pfree_recv_queue);

			goto _exit_recvbuf2recvframe;
		}

		recvframe_put(precvframe, skb_len);
		//recvframe_pull(precvframe, drvinfo_sz + RXDESC_SIZE);

		if (pattrib->physt)
		{
			pphy_info = (struct phy_stat*)(pbuf + RXDESC_OFFSET);
			update_recvframe_phyinfo(precvframe, pphy_info);
		}

#ifdef CONFIG_USB_RX_AGGREGATION
		switch(pHalData->UsbRxAggMode)
		{
			case USB_RX_AGG_DMA:
			case USB_RX_AGG_MIX:
				pkt_offset = (u16)_RND128(pkt_offset);
				break;
				case USB_RX_AGG_USB:
				pkt_offset = (u16)_RND4(pkt_offset);
				break;
			case USB_RX_AGG_DISABLE:
			default:
				break;
		}
#endif

		if(rtw_recv_entry(precvframe) != _SUCCESS)
		{
			RT_TRACE(_module_rtl871x_recv_c_,_drv_err_,("recvbuf2recvframe: rtw_recv_entry(precvframe) != _SUCCESS\n"));
		}

		pkt_cnt--;
		transfer_len -= pkt_offset;
		pbuf += pkt_offset;
		precvframe = NULL;
		pkt_copy = NULL;

		if(transfer_len>0 && pkt_cnt==0)
			pkt_cnt = (le32_to_cpu(prxstat->rxdw2)>>16) & 0xff;

	}while((transfer_len>0) && (pkt_cnt>0));

_exit_recvbuf2recvframe:

	return _SUCCESS;
}

void rtl8192cu_recv_tasklet(void *priv)
{
	struct recv_buf *precvbuf = NULL;
	struct rtw_adapter	*padapter = (struct rtw_adapter*)priv;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	while (NULL != (precvbuf = rtw_dequeue_recvbuf(&precvpriv->recv_buf_pending_queue)))
	{
		if ((padapter->bDriverStopped == _TRUE)||(padapter->bSurpriseRemoved== _TRUE))
		{
			DBG_8723A("recv_tasklet => bDriverStopped or bSurpriseRemoved \n");

			break;
		}


		recvbuf2recvframe(padapter, precvbuf);

		rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
	}

}

static void usb_read_port_complete(struct urb *purb, struct pt_regs *regs)
{
	struct recv_buf	*precvbuf = (struct recv_buf *)purb->context;
	struct rtw_adapter			*padapter =(struct rtw_adapter *)precvbuf->adapter;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete!!!\n"));

	precvpriv->rx_pending_cnt --;

	if(padapter->bSurpriseRemoved || padapter->bDriverStopped||padapter->bReadPortCancel)
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bDriverStopped(%d) OR bSurpriseRemoved(%d)\n", padapter->bDriverStopped, padapter->bSurpriseRemoved));

		goto exit;
	}

	if(purb->status==0)//SUCCESS
	{
		if ((purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE))
		{
			RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete: (purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE)\n"));

			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
		}
		else
		{
			rtw_reset_continual_urb_error(adapter_to_dvobj(padapter));

			precvbuf->transfer_len = purb->actual_length;

			//rtw_enqueue_rx_transfer_buffer(precvpriv, rx_transfer_buf);
			rtw_enqueue_recvbuf(precvbuf, &precvpriv->recv_buf_pending_queue);

			tasklet_schedule(&precvpriv->recv_tasklet);
		}
	}
	else
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete : purb->status(%d) != 0 \n", purb->status));

		DBG_8723A("###=> usb_read_port_complete => urb status(%d)\n", purb->status);

		if(rtw_inc_and_chk_continual_urb_error(adapter_to_dvobj(padapter)) == _TRUE ){
			padapter->bSurpriseRemoved = _TRUE;
		}

		switch(purb->status) {
			case -EINVAL:
			case -EPIPE:
			case -ENODEV:
			case -ESHUTDOWN:
				//padapter->bSurpriseRemoved=_TRUE;
				RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bSurpriseRemoved=TRUE\n"));
			case -ENOENT:
				padapter->bDriverStopped=_TRUE;
				RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bDriverStopped=TRUE\n"));
				break;
			case -EPROTO:
			case -EOVERFLOW:
				#ifdef DBG_CONFIG_ERROR_DETECT
				{
					HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
					pHalData->srestpriv.Wifi_Error_Status = USB_READ_PORT_FAIL;
				}
				#endif
				rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
				break;
			case -EINPROGRESS:
				DBG_8723A("ERROR: URB IS IN PROGRESS!/n");
				break;
			default:
				break;
		}

	}

exit:

_func_exit_;

}

static u32 usb_read_port(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *rmem)
{
	int err;
	unsigned int pipe;
	u32 ret = _SUCCESS;
	struct urb *purb = NULL;
	struct recv_buf	*precvbuf = (struct recv_buf *)rmem;
	struct rtw_adapter		*adapter = pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct recv_priv	*precvpriv = &adapter->recvpriv;
	struct usb_device	*pusbd = pdvobj->pusbdev;

_func_enter_;

	if(adapter->bDriverStopped || adapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port:( padapter->bDriverStopped ||padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n"));
		return _FAIL;
	}

	if(precvbuf !=NULL)
	{
		rtl8192cu_init_recvbuf(adapter, precvbuf);

		if(precvbuf->pbuf)
		{
			precvpriv->rx_pending_cnt++;

			purb = precvbuf->purb;

			//translate DMA FIFO addr to pipehandle
			pipe = ffaddr2pipehdl(pdvobj, addr);

			usb_fill_bulk_urb(purb, pusbd, pipe,
						precvbuf->pbuf,
						MAX_RECVBUF_SZ,
						usb_read_port_complete,
						precvbuf);//context is precvbuf

			purb->transfer_dma = precvbuf->dma_transfer_addr;
			purb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

			err = usb_submit_urb(purb, GFP_ATOMIC);
			if((err) && (err != (-EPERM)))
			{
				RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("cannot submit rx in-token(err=0x%.8x), URB_STATUS =0x%.8x", err, purb->status));
				DBG_8723A("cannot submit rx in-token(err = 0x%08x),urb_status = %d\n",err,purb->status);
				ret = _FAIL;
			}

		}

	}
	else
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port:precvbuf ==NULL\n"));
		ret = _FAIL;
	}

_func_exit_;

	return ret;
}
#else	// CONFIG_USE_USB_BUFFER_ALLOC_RX
static s32 pre_recv_entry(union recv_frame *precvframe, struct recv_stat *prxstat, struct phy_stat *pphy_info)
{
	s32 ret=_SUCCESS;
#ifdef CONFIG_CONCURRENT_MODE
	u8 *primary_myid, *secondary_myid, *paddr1;
	union recv_frame	*precvframe_if2 = NULL;
	struct rtw_adapter *primary_padapter = precvframe->u.hdr.adapter;
	struct rtw_adapter *secondary_padapter = primary_padapter->pbuddy_adapter;
	struct recv_priv *precvpriv = &primary_padapter->recvpriv;
	_queue *pfree_recv_queue = &precvpriv->free_recv_queue;
	u8	*pbuf = precvframe->u.hdr.rx_data;

	if(!secondary_padapter)
		return ret;

	paddr1 = GetAddr1Ptr(precvframe->u.hdr.rx_data);

	if(!is_multicast_ether_addr(paddr1))//unicast packets
	{
		//primary_myid = myid(&primary_padapter->eeprompriv);
		secondary_myid = myid(&secondary_padapter->eeprompriv);

		if (!memcmp(paddr1, secondary_myid, ETH_ALEN))
		{
			//change to secondary interface
			precvframe->u.hdr.adapter = secondary_padapter;
		}

		//ret = recv_entry(precvframe);

	}
	else // Handle BC/MC Packets
	{
		u8 clone = _TRUE;

		if(_TRUE == clone)
		{
			//clone/copy to if2
			u8 shift_sz = 0;
			u32 alloc_sz, skb_len;
			struct sk_buff	 *pkt_copy = NULL;
			struct rx_pkt_attrib *pattrib = NULL;

			precvframe_if2 = rtw_alloc_recvframe(pfree_recv_queue);
			if(precvframe_if2)
			{
				precvframe_if2->u.hdr.adapter = secondary_padapter;

				INIT_LIST_HEAD(&precvframe_if2->u.hdr.list);
				precvframe_if2->u.hdr.precvbuf = NULL;	//can't access the precvbuf for new arch.
				precvframe_if2->u.hdr.len=0;

				memcpy(&precvframe_if2->u.hdr.attrib, &precvframe->u.hdr.attrib, sizeof(struct rx_pkt_attrib));

				pattrib = &precvframe_if2->u.hdr.attrib;

				//	Modified by Albert 20101213
				//	For 8 bytes IP header alignment.
				if (pattrib->qos)	//	Qos data, wireless lan header length is 26
				{
					shift_sz = 6;
				}
				else
				{
					shift_sz = 0;
				}

				skb_len = pattrib->pkt_len;

				// for first fragment packet, driver need allocate 1536+drvinfo_sz+RXDESC_SIZE to defrag packet.
				// modify alloc_sz for recvive crc error packet by thomas 2011-06-02
				if((pattrib->mfrag == 1)&&(pattrib->frag_num == 0)){
					//alloc_sz = 1664;	//1664 is 128 alignment.
					if(skb_len <= 1650)
						alloc_sz = 1664;
					else
						alloc_sz = skb_len + 14;
				}
				else {
					alloc_sz = skb_len;
					//	6 is for IP header 8 bytes alignment in QoS packet case.
					//	8 is for skb->data 4 bytes alignment.
					alloc_sz += 14;
				}

				pkt_copy = netdev_alloc_skb(secondary_padapter->pnetdev, alloc_sz);
				if(pkt_copy)
				{
					pkt_copy->dev = secondary_padapter->pnetdev;
					precvframe_if2->u.hdr.pkt = pkt_copy;
					precvframe_if2->u.hdr.rx_head = pkt_copy->data;
					precvframe_if2->u.hdr.rx_end = pkt_copy->data + alloc_sz;
					skb_reserve( pkt_copy, 8 - ((unsigned long)(pkt_copy->data) & 7));//force pkt_copy->data at 8-byte alignment address
					skb_reserve( pkt_copy, shift_sz );//force ip_hdr at 8-byte alignment address according to shift_sz.
					memcpy(pkt_copy->data, pbuf, skb_len);
					precvframe_if2->u.hdr.rx_data = precvframe_if2->u.hdr.rx_tail = pkt_copy->data;


					recvframe_put(precvframe_if2, skb_len);
					//recvframe_pull(precvframe_if2, drvinfo_sz + RXDESC_SIZE);
					if(pphy_info)
					update_recvframe_phyinfo(precvframe_if2, pphy_info);
					//rtl8192c_translate_rx_signal_stuff(precvframe_if2, pphy_info);

					ret = rtw_recv_entry(precvframe_if2);

				} else {
					rtw_free_recvframe(precvframe_if2, pfree_recv_queue);
					DBG_8723A("%s()-%d: alloc_skb() failed!\n", __FUNCTION__, __LINE__);
				}

			}

		}

	}

	ret = rtw_recv_entry(precvframe);

#endif

	return ret;

}

static int recvbuf2recvframe(struct rtw_adapter *padapter, struct sk_buff *pskb)
{
	u8	*pbuf;
	u8	shift_sz = 0;
	u16	pkt_cnt;
	u32	pkt_offset, skb_len, alloc_sz;
	s32	transfer_len;
	struct recv_stat	*prxstat;
	struct phy_stat	*pphy_info = NULL;
	struct sk_buff		*pkt_copy = NULL;
	union recv_frame	*precvframe = NULL;
	struct rx_pkt_attrib	*pattrib = NULL;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	_queue			*pfree_recv_queue = &precvpriv->free_recv_queue;


	transfer_len = (s32)pskb->len;
	pbuf = pskb->data;

	prxstat = (struct recv_stat *)pbuf;
	pkt_cnt = (le32_to_cpu(prxstat->rxdw2)>>16) & 0xff;

	do{
		RT_TRACE(_module_rtl871x_recv_c_, _drv_info_,
			 ("recvbuf2recvframe: rxdesc=offsset 0:0x%08x, 4:0x%08x, 8:0x%08x, C:0x%08x\n",
			  prxstat->rxdw0, prxstat->rxdw1, prxstat->rxdw2, prxstat->rxdw4));

		prxstat = (struct recv_stat *)pbuf;

		precvframe = rtw_alloc_recvframe(pfree_recv_queue);
		if(precvframe==NULL)
		{
			RT_TRACE(_module_rtl871x_recv_c_,_drv_err_,("recvbuf2recvframe: precvframe==NULL\n"));
			DBG_8723A("%s()-%d: rtw_alloc_recvframe() failed! RX Drop!\n", __FUNCTION__, __LINE__);
			goto _exit_recvbuf2recvframe;
		}

		INIT_LIST_HEAD(&precvframe->u.hdr.list);
		precvframe->u.hdr.precvbuf = NULL;	//can't access the precvbuf for new arch.
		precvframe->u.hdr.len=0;

//		rtl8192c_query_rx_desc_status(precvframe, prxstat);
		update_recvframe_attrib(precvframe, prxstat);

		pattrib = &precvframe->u.hdr.attrib;

		if(pattrib->crc_err){
			DBG_8723A("%s()-%d: RX Warning! rx CRC ERROR !!\n", __FUNCTION__, __LINE__);
			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		pkt_offset = RXDESC_SIZE + pattrib->drvinfo_sz + pattrib->shift_sz + pattrib->pkt_len;

		if((pattrib->pkt_len<=0) || (pkt_offset>transfer_len))
		{
			RT_TRACE(_module_rtl871x_recv_c_,_drv_info_,("recvbuf2recvframe: pkt_len<=0\n"));
			DBG_8723A("%s()-%d: RX Warning!\n", __FUNCTION__, __LINE__);
			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		//	Modified by Albert 20101213
		//	For 8 bytes IP header alignment.
		if (pattrib->qos)	//	Qos data, wireless lan header length is 26
		{
			shift_sz = 6;
		}
		else
		{
			shift_sz = 0;
		}

		skb_len = pattrib->pkt_len;

		// for first fragment packet, driver need allocate 1536+drvinfo_sz+RXDESC_SIZE to defrag packet.
		// modify alloc_sz for recvive crc error packet by thomas 2011-06-02
		if((pattrib->mfrag == 1)&&(pattrib->frag_num == 0)){
			//alloc_sz = 1664;	//1664 is 128 alignment.
			if(skb_len <= 1650)
				alloc_sz = 1664;
			else
				alloc_sz = skb_len + 14;
		}
		else {
			alloc_sz = skb_len;
			//	6 is for IP header 8 bytes alignment in QoS packet case.
			//	8 is for skb->data 4 bytes alignment.
			alloc_sz += 14;
		}

		pkt_copy = netdev_alloc_skb(padapter->pnetdev, alloc_sz);
		if (pkt_copy)
		{
			pkt_copy->dev = padapter->pnetdev;
			precvframe->u.hdr.pkt = pkt_copy;
			precvframe->u.hdr.rx_head = pkt_copy->data;
			precvframe->u.hdr.rx_end = pkt_copy->data + alloc_sz;
			skb_reserve( pkt_copy, 8 - ((unsigned long)( pkt_copy->data ) & 7 ));//force pkt_copy->data at 8-byte alignment address
			skb_reserve( pkt_copy, shift_sz );//force ip_hdr at 8-byte alignment address according to shift_sz.
			memcpy(pkt_copy->data, (pbuf + pattrib->shift_sz + pattrib->drvinfo_sz + RXDESC_SIZE), skb_len);
			precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pkt_copy->data;
		}
		else
		{
			if((pattrib->mfrag == 1)&&(pattrib->frag_num == 0))
			{
				DBG_8723A("recvbuf2recvframe: alloc_skb fail , drop frag frame \n");
				rtw_free_recvframe(precvframe, pfree_recv_queue);
				goto _exit_recvbuf2recvframe;
			}

			precvframe->u.hdr.pkt = skb_clone(pskb, GFP_ATOMIC);
			if(precvframe->u.hdr.pkt)
			{
				precvframe->u.hdr.rx_head = precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail
					= pbuf+ pattrib->drvinfo_sz + RXDESC_SIZE;
				precvframe->u.hdr.rx_end =  pbuf +pattrib->drvinfo_sz + RXDESC_SIZE+ alloc_sz;
			}
			else
			{
				DBG_8723A("recvbuf2recvframe: skb_clone fail\n");
				rtw_free_recvframe(precvframe, pfree_recv_queue);
				goto _exit_recvbuf2recvframe;
			}

		}

		recvframe_put(precvframe, skb_len);
		//recvframe_pull(precvframe, drvinfo_sz + RXDESC_SIZE);

		if (pattrib->physt)
		{
			pphy_info = (struct phy_stat*)(pbuf + RXDESC_OFFSET);
			update_recvframe_phyinfo(precvframe, pphy_info);
		}

#ifdef CONFIG_USB_RX_AGGREGATION
		switch(pHalData->UsbRxAggMode)
		{
			case USB_RX_AGG_DMA:
			case USB_RX_AGG_MIX:
				pkt_offset = (u16)_RND128(pkt_offset);
				break;
				case USB_RX_AGG_USB:
				pkt_offset = (u16)_RND4(pkt_offset);
				break;
			case USB_RX_AGG_DISABLE:
			default:
				break;
		}
#endif

#ifdef CONFIG_CONCURRENT_MODE
		if(pre_recv_entry(precvframe, prxstat, pphy_info) != _SUCCESS)
		{
			RT_TRACE(_module_rtl871x_recv_c_,_drv_err_,("recvbuf2recvframe: recv_entry(precvframe) != _SUCCESS\n"));
		}
#else
		if(rtw_recv_entry(precvframe) != _SUCCESS)
		{
			RT_TRACE(_module_rtl871x_recv_c_,_drv_err_,("recvbuf2recvframe: rtw_recv_entry(precvframe) != _SUCCESS\n"));
		}
#endif

		pkt_cnt--;
		transfer_len -= pkt_offset;
		pbuf += pkt_offset;
		precvframe = NULL;
		pkt_copy = NULL;

		if(transfer_len>0 && pkt_cnt==0)
			pkt_cnt = (le32_to_cpu(prxstat->rxdw2)>>16) & 0xff;

	}while((transfer_len>0) && (pkt_cnt>0));

_exit_recvbuf2recvframe:

	return _SUCCESS;
}

void rtl8192cu_recv_tasklet(void *priv)
{
	struct sk_buff		*pskb;
	struct rtw_adapter		*padapter = (struct rtw_adapter*)priv;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	while (NULL != (pskb = skb_dequeue(&precvpriv->rx_skb_queue)))
	{
		if ((padapter->bDriverStopped == _TRUE)||(padapter->bSurpriseRemoved== _TRUE))
		{
			DBG_8723A("recv_tasklet => bDriverStopped or bSurpriseRemoved \n");
			dev_kfree_skb_any(pskb);
			break;
		}

		recvbuf2recvframe(padapter, pskb);

#ifdef CONFIG_PREALLOC_RECV_SKB

		skb_reset_tail_pointer(pskb);

		pskb->len = 0;

		skb_queue_tail(&precvpriv->free_recv_skb_queue, pskb);

#else
		dev_kfree_skb_any(pskb);
#endif

	}

}


static void usb_read_port_complete(struct urb *purb, struct pt_regs *regs)
{
	uint isevt, *pbuf;
	struct recv_buf	*precvbuf = (struct recv_buf *)purb->context;
	struct rtw_adapter			*padapter =(struct rtw_adapter *)precvbuf->adapter;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete!!!\n"));

	precvpriv->rx_pending_cnt --;

	//if(precvpriv->rx_pending_cnt== 0)
	//{
	//	RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete: rx_pending_cnt== 0, set allrxreturnevt!\n"));
	//	up(&precvpriv->allrxreturnevt);
	//}

	if(padapter->bSurpriseRemoved || padapter->bDriverStopped||padapter->bReadPortCancel)
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bDriverStopped(%d) OR bSurpriseRemoved(%d)\n", padapter->bDriverStopped, padapter->bSurpriseRemoved));

	#ifdef CONFIG_PREALLOC_RECV_SKB
		precvbuf->reuse = _TRUE;
	#else
		if(precvbuf->pskb){
			DBG_8723A("==> free skb(%p)\n",precvbuf->pskb);
			dev_kfree_skb_any(precvbuf->pskb);
		}
	#endif
		DBG_8723A("%s()-%d: RX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bReadPortCancel(%d)\n",
		__FUNCTION__, __LINE__,padapter->bDriverStopped, padapter->bSurpriseRemoved,padapter->bReadPortCancel);
		goto exit;
	}

	if(purb->status==0)//SUCCESS
	{
		if ((purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE))
		{
			RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete: (purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE)\n"));
			precvbuf->reuse = _TRUE;
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
			DBG_8723A("%s()-%d: RX Warning!\n", __FUNCTION__, __LINE__);
		}
		else
		{
			rtw_reset_continual_urb_error(adapter_to_dvobj(padapter));

			precvbuf->transfer_len = purb->actual_length;
			skb_put(precvbuf->pskb, purb->actual_length);
			skb_queue_tail(&precvpriv->rx_skb_queue, precvbuf->pskb);

			if (skb_queue_len(&precvpriv->rx_skb_queue)<=1)
				tasklet_schedule(&precvpriv->recv_tasklet);

			precvbuf->pskb = NULL;
			precvbuf->reuse = _FALSE;
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
		}
	}
	else
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete : purb->status(%d) != 0 \n", purb->status));
		skb_put(precvbuf->pskb, purb->actual_length);
		precvbuf->pskb = NULL;

		DBG_8723A("###=> usb_read_port_complete => urb status(%d)\n", purb->status);

		if(rtw_inc_and_chk_continual_urb_error(adapter_to_dvobj(padapter)) == _TRUE ){
			padapter->bSurpriseRemoved = _TRUE;
		}

		switch(purb->status) {
			case -EINVAL:
			case -EPIPE:
			case -ENODEV:
			case -ESHUTDOWN:
				//padapter->bSurpriseRemoved=_TRUE;
				RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bSurpriseRemoved=TRUE\n"));
			case -ENOENT:
				padapter->bDriverStopped=_TRUE;
				RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bDriverStopped=TRUE\n"));
				break;
			case -EPROTO:
			case -EOVERFLOW:
				#ifdef DBG_CONFIG_ERROR_DETECT
				{
					HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
					pHalData->srestpriv.Wifi_Error_Status = USB_READ_PORT_FAIL;
				}
				#endif
				precvbuf->reuse = _TRUE;
				rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
				break;
			case -EINPROGRESS:
				DBG_8723A("ERROR: URB IS IN PROGRESS!/n");
				break;
			default:
				break;
		}

	}

exit:

_func_exit_;

}

static u32 usb_read_port(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *rmem)
{
	int err;
	unsigned int pipe;
	unsigned long tmpaddr = 0;
	unsigned long alignment = 0;
	u32 ret = _SUCCESS;
	struct urb *purb = NULL;
	struct recv_buf	*precvbuf = (struct recv_buf *)rmem;
	struct rtw_adapter		*adapter = pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct recv_priv	*precvpriv = &adapter->recvpriv;
	struct usb_device	*pusbd = pdvobj->pusbdev;


_func_enter_;

	if(adapter->bDriverStopped || adapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_,
			 ("usb_read_port:( padapter->bDriverStopped ||padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n"));
		return _FAIL;
	}

	if (!precvbuf) {
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port:precvbuf ==NULL\n"));
		return _FAIL;
	}

#ifdef CONFIG_PREALLOC_RECV_SKB
	if((precvbuf->reuse == _FALSE) || (precvbuf->pskb == NULL)) {
		if (NULL != (precvbuf->pskb = skb_dequeue(&precvpriv->free_recv_skb_queue)))
			precvbuf->reuse = _TRUE;
	}
#endif


	rtl8192cu_init_recvbuf(adapter, precvbuf);

	//re-assign for linux based on skb
	if((precvbuf->reuse == _FALSE) || (precvbuf->pskb == NULL)) {
		//precvbuf->pskb = alloc_skb(MAX_RECVBUF_SZ, GFP_ATOMIC);//don't use this after v2.6.25
		precvbuf->pskb = netdev_alloc_skb(adapter->pnetdev, MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);
		if (precvbuf->pskb == NULL) {
			RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("init_recvbuf(): alloc_skb fail!\n"));
			return _FAIL;
		}

		tmpaddr = (unsigned long)precvbuf->pskb->data;
		alignment = tmpaddr & (RECVBUFF_ALIGN_SZ-1);
		skb_reserve(precvbuf->pskb, (RECVBUFF_ALIGN_SZ - alignment));

		precvbuf->phead = precvbuf->pskb->head;
		precvbuf->pdata = precvbuf->pskb->data;
		precvbuf->ptail = skb_tail_pointer(precvbuf->pskb);
		precvbuf->pend = skb_end_pointer(precvbuf->pskb);
		precvbuf->pbuf = precvbuf->pskb->data;
	} else { //reuse skb
		precvbuf->phead = precvbuf->pskb->head;
		precvbuf->pdata = precvbuf->pskb->data;
		precvbuf->ptail = skb_tail_pointer(precvbuf->pskb);
		precvbuf->pend = skb_end_pointer(precvbuf->pskb);
	precvbuf->pbuf = precvbuf->pskb->data;

		precvbuf->reuse = _FALSE;
	}

	precvpriv->rx_pending_cnt++;

	purb = precvbuf->purb;

	//translate DMA FIFO addr to pipehandle
	pipe = ffaddr2pipehdl(pdvobj, addr);

	usb_fill_bulk_urb(purb, pusbd, pipe,
					precvbuf->pbuf,
					MAX_RECVBUF_SZ,
					usb_read_port_complete,
					precvbuf);//context is precvbuf

	err = usb_submit_urb(purb, GFP_ATOMIC);
	if((err) && (err != (-EPERM))) {
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("cannot submit rx in-token(err=0x%.8x), URB_STATUS =0x%.8x", err, purb->status));
		DBG_8723A("cannot submit rx in-token(err = 0x%08x),urb_status = %d\n",err,purb->status);
		ret = _FAIL;
	}

_func_exit_;
	return ret;
}
#endif	// CONFIG_USE_USB_BUFFER_ALLOC_RX

void rtl8192cu_xmit_tasklet(void *priv)
{
	int ret = _FALSE;
	struct rtw_adapter *padapter = (struct rtw_adapter*)priv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	if(check_fwstate(&padapter->mlmepriv, _FW_UNDER_SURVEY) == _TRUE)
		return;

	while(1)
	{
		if ((padapter->bDriverStopped == _TRUE)||(padapter->bSurpriseRemoved== _TRUE) || (padapter->bWritePortCancel == _TRUE))
		{
			DBG_8723A("xmit_tasklet => bDriverStopped or bSurpriseRemoved or bWritePortCancel\n");
			break;
		}

		ret = rtl8192cu_xmitframe_complete(padapter, pxmitpriv, NULL);

		if(ret==_FALSE)
			break;

	}

}

void rtl8723au_set_intf_ops(struct _io_ops	*pops)
{
	_func_enter_;

	memset((u8 *)pops, 0, sizeof(struct _io_ops));

	pops->_read8 = &usb_read8;
	pops->_read16 = &usb_read16;
	pops->_read32 = &usb_read32;
	pops->_read_mem = &usb_read_mem;
	pops->_read_port = &usb_read_port;

	pops->_write8 = &usb_write8;
	pops->_write16 = &usb_write16;
	pops->_write32 = &usb_write32;
	pops->_writeN = &usb_writeN;

#ifdef CONFIG_USB_SUPPORT_ASYNC_VDN_REQ
	pops->_write8_async= &usb_async_write8;
	pops->_write16_async = &usb_async_write16;
	pops->_write32_async = &usb_async_write32;
#endif
	pops->_write_mem = &usb_write_mem;
	pops->_write_port = &usb_write_port;

	pops->_read_port_cancel = &usb_read_port_cancel;
	pops->_write_port_cancel = &usb_write_port_cancel;

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	pops->_read_interrupt = &usb_read_interrupt;
#endif

	_func_exit_;

}

void rtl8723au_set_hw_type(struct rtw_adapter *padapter)
{
	padapter->chip_type = RTL8723A;
	padapter->HardwareType = HARDWARE_TYPE_RTL8723AU;
	DBG_8723A("CHIP TYPE: RTL8723A\n");
}
