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
 *
 ******************************************************************************/
/*! \file */
#ifndef __INC_ETHERNET_H
#define __INC_ETHERNET_H

#define LLC_HEADER_SIZE			6	/*  LLC Header Length */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
static inline void ether_addr_copy(u8 *dst, const u8 *src)
{
        u16 *a = (u16 *)dst;
        const u16 *b = (const u16 *)src;

        a[0] = b[0];
        a[1] = b[1];
        a[2] = b[2];
}
#endif

#endif /*  #ifndef __INC_ETHERNET_H */
