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

#ifndef __OSDEP_INTF_H_
#define __OSDEP_INTF_H_

#include <osdep_service.h>
#include <drv_types.h>

int rtw_hw_suspend(struct rtw_adapter *padapter);
int rtw_hw_resume(struct rtw_adapter *padapter);

u8 rtw_init_drv_sw(struct rtw_adapter *padapter);
u8 rtw_free_drv_sw(struct rtw_adapter *padapter);
u8 rtw_reset_drv_sw(struct rtw_adapter *padapter);

u32 rtw_start_drv_threads(struct rtw_adapter *padapter);
void rtw_stop_drv_threads (struct rtw_adapter *padapter);
#ifdef CONFIG_WOWLAN
void rtw_cancel_dynamic_chk_timer(struct rtw_adapter *padapter);
#endif
void rtw_cancel_all_timer(struct rtw_adapter *padapter);

int rtw_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);

int rtw_init_netdev_name(struct net_device *pnetdev, const char *ifname);
struct net_device *rtw_init_netdev(struct rtw_adapter *padapter);

u16 rtw_recv_select_queue(struct sk_buff *skb);

#ifdef CONFIG_PROC_DEBUG
void rtw_proc_init_one(struct net_device *dev);
void rtw_proc_remove_one(struct net_device *dev);
#else /* CONFIG_PROC_DEBUG */
static void rtw_proc_init_one(struct net_device *dev){}
static void rtw_proc_remove_one(struct net_device *dev){}
#endif /* CONFIG_PROC_DEBUG */

void rtw_ips_dev_unload(struct rtw_adapter *padapter);

int rtw_ips_pwr_up(struct rtw_adapter *padapter);
void rtw_ips_pwr_down(struct rtw_adapter *padapter);

int rtw_drv_register_netdev(struct rtw_adapter *padapter);
void rtw_ndev_destructor(struct net_device *ndev);

#endif	/* _OSDEP_INTF_H_ */
