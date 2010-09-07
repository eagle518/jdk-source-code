/*
 * @(#)kplib.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*

	File:           mem.c

	Contains:
		This module contains routines to handle blcoks of memory.
		These routine are similar to the ANSI functions memcpy,
		memcmp and memset.  The difference is that for WIN16 and DOS
		they handle HUGE data.

	Written by:     Midnight KCMS Team

	Copyright:      (c) 1991-1998 by Eastman Kodak Company, all rights
			reserved.

   Change History (most recent first):

	Windows Change History
		$Header$

	SCCSID = @(#)kplib.c	1.6   12/01/98
	To Do:
*/


#include "kcms_sys.h"
#include "limits.h"
#include "string.h"

#if defined (KPWIN16) || defined (KPDOS)

/*------------------------------------------------------------------------*/
static bool KpChkSeg (char KPHUGE *m1, char KPHUGE *m2, KpInt32_t cnt)
{
	long	m1offset;
	long	m2offset;

	m1offset = (unsigned short) (long) m1 + cnt;
	if (m1offset > 0x0000FFFF)
		return TRUE;

	m2offset = (unsigned short) (long) m2 + cnt;
	if (m2offset > 0x0000FFFF)
		return TRUE;

	return FALSE;
}

/*------------------------------------------------------------------------*/
void *KpMemCpy (void KPHUGE *dest, void KPHUGE *src, KpInt32_t cnt)
{
	unsigned char KPHUGE *p1;
	unsigned char KPHUGE *p2;

	if ((cnt < UINT_MAX) && !KpChkSeg (dest, src, cnt))
		return memcpy ((void FAR *) dest, (void FAR *) src, (size_t) cnt);

	for (p1 = dest, p2 = src; cnt; cnt--)
		*p1++ = *p2++;

	return dest;
}

/*------------------------------------------------------------------------*/
int KpMemCmp (void KPHUGE *s1, void KPHUGE *s2, KpInt32_t cnt)
{
	unsigned char KPHUGE *p1;
	unsigned char KPHUGE *p2;

	if ((cnt < UINT_MAX) && !KpChkSeg (s1, s2, cnt))
		return memcmp ((void FAR *) s1, (void FAR *) s2, (size_t) cnt);

	for (p1 = s1, p2 = s2; cnt; p1++, p2++, cnt--) {
		if (*p2 != *p1)
			return *p2 - *p1;
	}

	return 0;
}

/*------------------------------------------------------------------------*/
void *KpMemSet (void KPHUGE *ptr, int value, KpInt32_t cnt)
{
	unsigned char KPHUGE *p1;

	if ((cnt < UINT_MAX) && !KpChkSeg (ptr, ptr, cnt))
		return memset ((void FAR *) ptr, value, (size_t) cnt);

	for (p1 = ptr; cnt; cnt--)
		*p1++ = '\0';

	return ptr;
}

#else

/*------------------------------------------------------------------------*/
void *KpMemCpy (void KPHUGE *dest, void KPHUGE *src, KpInt32_t cnt)
{
	return memcpy (dest, src, (size_t) cnt);
}

/*------------------------------------------------------------------------*/
KpInt32_t KpMemCmp (void KPHUGE *s1, void KPHUGE *s2, KpInt32_t cnt)
{
	return memcmp (s1, s2, (size_t) cnt);
}

/*------------------------------------------------------------------------*/
void *KpMemSet (void KPHUGE *ptr, KpInt32_t value, KpInt32_t cnt)
{
	return memset (ptr, value, (size_t) cnt);
}
#endif

/*------------------------------------------------------------------------
 * Kp_swab16 and Kp_swab32 swap the bytes in an array of 16 bit and 32 bit
 * integers, respectively.  This is necessary to convert data read from files
 * written on a machine of the opposite architecture (IBM vs. DEC).
 *
 * Note that we can't use the C library fucntion swab(3I) since this is
 * not guaranteed to work "in place".
 */

void Kp_swab16 (KpGenericPtr_t buf, KpInt32_t nitems)
{
	KpUInt8_t	t;
	KpUInt8_t	*p;

	p = (KpUInt8_t *) buf;
	while (--nitems >= 0) {
		t = p[0];
		p[0] = p[1];
		p[1] = t;
		p += 2;
	}
}

/*------------------------------------------------------------------------*/
void Kp_swab32 (
			KpGenericPtr_t	buf,
			KpInt32_t		nitems)
{
	KpUInt8_t	t;
	KpUInt8_t	*p;

	p = (KpUInt8_t *) buf;
	while (--nitems >= 0) {
		t = p[0];
		p[0] = p[3];
		p[3] = t;
		t = p[1];
		p[1] = p[2];
		p[2] = t;
		p += 4;
	}
}
