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
/*! \file */
#ifndef __INC_ETHERNET_H
#define __INC_ETHERNET_H

#define LLC_HEADER_SIZE						6		//!< LLC Header Length

#define RT_ETH_IS_MULTICAST(_pAddr)	((((UCHAR *)(_pAddr))[0]&0x01)!=0)		//!< Is Multicast Address?
#define RT_ETH_IS_BROADCAST(_pAddr)	(										\
											((UCHAR *)(_pAddr))[0]==0xff	&&		\
											((UCHAR *)(_pAddr))[1]==0xff	&&		\
											((UCHAR *)(_pAddr))[2]==0xff	&&		\
											((UCHAR *)(_pAddr))[3]==0xff	&&		\
											((UCHAR *)(_pAddr))[4]==0xff	&&		\
											((UCHAR *)(_pAddr))[5]==0xff		)	//!< Is Broadcast Address?


#endif // #ifndef __INC_ETHERNET_H
