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
#ifndef __BASIC_TYPES_H__
#define __BASIC_TYPES_H__

#define SUCCESS	0
#define FAIL	(-1)

#include <linux/types.h>
#define NDIS_OID uint
#define NDIS_STATUS uint

#define FIELD_OFFSET(s, field)	((__kernel_ssize_t)&((s *)(0))->field)

/* port from fw by thomas */
/*  TODO: Belows are Sync from SD7-Driver. It is necessary to check correctness */

/*
 *	Call endian free function when
 *		1. Read/write packet content.
 *		2. Before write integer to IO.
 *		3. After read integer from IO.
*/

/*  Byte Swapping routine. */
#define EF1Byte
#define EF2Byte		le16_to_cpu
#define EF4Byte	le32_to_cpu

/*  Read LE format data from memory */
#define ReadEF1Byte(_ptr)		EF1Byte(*((u8 *)(_ptr)))
#define ReadEF2Byte(_ptr)		EF2Byte(*((u16 *)(_ptr)))
#define ReadEF4Byte(_ptr)		EF4Byte(*((u32 *)(_ptr)))

/*  Write LE data to memory */
#define WriteEF1Byte(_ptr, _val)	((*((u8 *)(_ptr))) = EF1Byte(_val))
#define WriteEF2Byte(_ptr, _val)	((*((u16 *)(_ptr))) = EF2Byte(_val))
#define WriteEF4Byte(_ptr, _val)	((*((u32 *)(_ptr))) = EF4Byte(_val))

/*	Example: */
/*		BIT_LEN_MASK_32(0) => 0x00000000 */
/*		BIT_LEN_MASK_32(1) => 0x00000001 */
/*		BIT_LEN_MASK_32(2) => 0x00000003 */
/*		BIT_LEN_MASK_32(32) => 0xFFFFFFFF */
#define BIT_LEN_MASK_32(__bitLen) \
	(0xFFFFFFFF >> (32 - (__bitLen)))
/*	Example: */
/*		BIT_OFFSET_LEN_MASK_32(0, 2) => 0x00000003 */
/*		BIT_OFFSET_LEN_MASK_32(16, 2) => 0x00030000 */
#define BIT_OFFSET_LEN_MASK_32(__offset, __bitLen) \
	(BIT_LEN_MASK_32(__bitLen) << (__offset))

/*	Description: */
/*		Return 4-byte value in host byte ordering from */
/*		4-byte pointer in litten-endian system. */
#define LE_P4BYTE_TO_HOST_4BYTE(__start) \
	(EF4Byte(*((u32 *)(__start))))

/*	Description: */
/*		Translate subfield (continuous bits in little-endian) of 4-byte value in litten byte to */
/*		4-byte value in host byte ordering. */
#define LE_BITS_TO_4BYTE(__start, __offset, __bitLen) \
	(\
		(LE_P4BYTE_TO_HOST_4BYTE(__start) >> (__offset)) \
		& \
		BIT_LEN_MASK_32(__bitLen) \
	)

/*	Description: */
/*		Mask subfield (continuous bits in little-endian) of 4-byte value in litten byte oredering */
/*		and return the result in 4-byte value in host byte ordering. */
#define LE_BITS_CLEARED_TO_4BYTE(__start, __offset, __bitLen) \
	(\
		LE_P4BYTE_TO_HOST_4BYTE(__start) \
		& \
		(~BIT_OFFSET_LEN_MASK_32(__offset, __bitLen)) \
	)

/*	Description: */
/*		Set subfield of little-endian 4-byte value to specified value. */
#define SET_BITS_TO_LE_4BYTE(__start, __offset, __bitLen, __value) \
	*((u32 *)(__start)) = \
		EF4Byte(\
			LE_BITS_CLEARED_TO_4BYTE(__start, __offset, __bitLen) \
			| \
			((((u32)__value) & BIT_LEN_MASK_32(__bitLen)) << (__offset)) \
		);


#define BIT_LEN_MASK_16(__bitLen) \
		(0xFFFF >> (16 - (__bitLen)))

#define BIT_OFFSET_LEN_MASK_16(__offset, __bitLen) \
	(BIT_LEN_MASK_16(__bitLen) << (__offset))

#define LE_P2BYTE_TO_HOST_2BYTE(__start) \
	(EF2Byte(*((u16 *)(__start))))

#define LE_BITS_TO_2BYTE(__start, __offset, __bitLen) \
	(\
		(LE_P2BYTE_TO_HOST_2BYTE(__start) >> (__offset)) \
		& \
		BIT_LEN_MASK_16(__bitLen) \
	)

#define LE_BITS_CLEARED_TO_2BYTE(__start, __offset, __bitLen) \
	(\
		LE_P2BYTE_TO_HOST_2BYTE(__start) \
		& \
		(~BIT_OFFSET_LEN_MASK_16(__offset, __bitLen)) \
	)

#define SET_BITS_TO_LE_2BYTE(__start, __offset, __bitLen, __value) \
	*((u16 *)(__start)) = \
		EF2Byte(\
			LE_BITS_CLEARED_TO_2BYTE(__start, __offset, __bitLen) \
			| \
			((((u16)__value) & BIT_LEN_MASK_16(__bitLen)) << (__offset)) \
		);

#define BIT_LEN_MASK_8(__bitLen) \
		(0xFF >> (8 - (__bitLen)))

#define BIT_OFFSET_LEN_MASK_8(__offset, __bitLen) \
	(BIT_LEN_MASK_8(__bitLen) << (__offset))

#define LE_P1BYTE_TO_HOST_1BYTE(__start) \
	(EF1Byte(*((u8 *)(__start))))

#define LE_BITS_TO_1BYTE(__start, __offset, __bitLen) \
	(\
		(LE_P1BYTE_TO_HOST_1BYTE(__start) >> (__offset)) \
		& \
		BIT_LEN_MASK_8(__bitLen) \
	)

#define LE_BITS_CLEARED_TO_1BYTE(__start, __offset, __bitLen) \
	(\
		LE_P1BYTE_TO_HOST_1BYTE(__start) \
		& \
		(~BIT_OFFSET_LEN_MASK_8(__offset, __bitLen)) \
	)

#define SET_BITS_TO_LE_1BYTE(__start, __offset, __bitLen, __value) \
	*((u8 *)(__start)) = \
		EF1Byte(\
			LE_BITS_CLEARED_TO_1BYTE(__start, __offset, __bitLen) \
			| \
			((((u8)__value) & BIT_LEN_MASK_8(__bitLen)) << (__offset)) \
		);

/* pclint */
#define LE_BITS_CLEARED_TO_1BYTE_8BIT(__start, __offset, __bitLen) \
	( LE_P1BYTE_TO_HOST_1BYTE(__start))

/* pclint */
#define SET_BITS_TO_LE_1BYTE_8BIT(__start, __offset, __bitLen, __value) \
{									\
	*((u8 *)(__start)) =						\
		EF1Byte(LE_BITS_CLEARED_TO_1BYTE_8BIT(__start, __offset,\
			__bitLen) | ((u8)__value) 			\
		);							 \
}

/*  Get the N-bytes aligment offset from the current length */
#define N_BYTE_ALIGMENT(__value, __align)				\
	((__align == 1) ? (__value) : (((__value + __align - 1) /	\
					__align) * __align))

#endif /* __BASIC_TYPES_H__ */
