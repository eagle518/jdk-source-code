/*
 * @(#)convert1.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)convert1.c	2.11 97/12/22
 *
	File:		convert.c

	Written by:	drivin' team

	Copyright:	(c) 1991-2003 by Eastman Kodak Company, all rights reserved.

	Change History (most recent first):

		 <1>	  8/9/91	blh		restore trashed db
		 <2>	 7/17/91	blh		take out Ctype.h stuff
									- relocation of A5World kills it
		 <1>	 7/15/91	blh		first checked in

	SCCS History:
		@(#)convert1.c	2.6 6/2/94

	To Do:
*/


#if !defined(lint) && defined(sun)
static char sccsid[] = "@(#)convert1.c	2.6 6/2/94";
#endif	/* lint */

#include "kcms_sys.h"

#if defined(KPMAC)
#if !defined(KPTHINK)
#include <Strings.h>
#endif
#endif
	
#include <string.h>

/* local prototypes */
static void reverse (char FAR *s);

/*----------------------------------------------------------------------*/
/* reverse string s in place */
static void reverse (char FAR *s)
{
	int		i, j;
	char	c;

	j = (int)strlen (s) - 1;
	for (i = 0; i < j; i++, j--) {
		c = s [i];
		s [i] = s [j];
		s [j] = c;
	}
}

/*----------------------------------------------------------------------*/
/* convert an integer value to ascii character string */
char FAR *KpItoa (KpInt32_t Value, char FAR *Buf)
{
	char	FAR *Ptr;
	char	Sign;

	Ptr = Buf;

	if (Value < 0) {
		Sign = '-';
		Value = -Value;
	}
	else
		Sign = '\0';

	do {
		*Ptr++ = (char) (Value % 10 + '0');
	} while ((Value /= 10) > 0);

	if (Sign != 0)
		*Ptr++ = Sign;
	*Ptr = '\0';
	reverse (Buf);

	return Buf;
}

/*----------------------------------------------------------------------*/
/* convert an ascii string to an integer */
KpInt32_t KpAtoi (char FAR *Buf)
{
	int			Sign;
	KpInt32_t	Value;

	Sign = (*Buf == '-') ? -1 : 1;
	if ((*Buf == '+') || (*Buf == '-'))
		Buf++;

	Value = 0;
	for (; *Buf; Buf++) {
		if ((*Buf >= '0') && (*Buf <= '9'))
			Value = 10 * Value + (*Buf - '0');
	}
	return Sign * Value;
}

/*----------------------------------------------------------------------*/
/* convert an integer value to ascii character hex string */
char FAR *KpLtos (KpInt32_t Value, char FAR * Buf)
{
	char		FAR *Ptr;
	KpUInt32_t	Mask = 0xF0000000L;
	int			Shift;

	for (Shift = 28, Ptr = Buf;
			Shift >= 0;
					Shift -= 4, Mask >>= 4, Ptr++) {
		*Ptr = (char) (((Value & Mask) >> Shift) + '0');
		if (*Ptr > '9')
			*Ptr += ('A' - '9' -1);
	}
	*Ptr = '\0';

	return Buf;
}
