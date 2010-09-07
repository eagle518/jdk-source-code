/*
 * @(#)FontMath.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
	File:		FontMath.c
 	Copyright:	© 1990 by Apple Computer, Inc., all rights reserved.

 */
#include "Hint.h"

#include "FSCdefs.h"
#include "FontMath.h"
/* MTE Lost: #include "math_routines.h" */

#ifndef applec

#ifdef asm68000
#ifndef asm68020
#define asm68000only
#endif
#endif

tt_int32 ShortFracMul(tt_int32 a, shortFrac b)
{
#ifdef asm68000
	asm {
	;	d0			hilong
	;	d1			midlong
	;	d2			lowlong
	;	d3			a
	;	d4			b
	;	d5			sign
	;
		movem.l		d3-d5,-(sp)
		move.l		a,d3				; get a
		smi			d5				; set sign
		bpl.s			@1
		neg.l			d3				; a = -a
	@1:
		move.w		b,d4				; get b
		bpl.s			@2
		not.b			d5				; change sign
		neg.w		d4				; b = -b
	@2:
		move.l		d3,d1			; get a
		swap			d1				; a.hi in d1.w
		mulu.w		d4,d1			; compute a.hi * b
		swap			d1				; (a.hi * b).lo in d1.hi and vice-versa
		move.w		d1,d0			; grab (a.hi * b).hi in d0.w
		move.w		#0x2000,d1		; d1 + (1 << 13) { like clr.w and add.l }
		move.w		d3,d2			; get a.lo
		mulu.w		d4,d2			; compute a.lo * b
		add.l			d1,d2			; a.lo * b + ((a.hi * b).lo).hi + (1 << 13)
		cmp.l		d2,d1			; check for overflow
		bls.s			@3
		addq			#1,d0			; carry bit
	@3:
		swap			d0
		clr.w		d0
		lsl.l			#2,d0			; d0 <<= 18
		andi.w		#0xc000,d2		; kill low 14 bits
		rol.l			#2,d2
		swap			d2				; d2 >>= 14
		add.l			d2,d0			; here is the unsigned answer
		tst.b			d5				; check sign
		beq.s			@done
		neg.l			d0
	@done:
		movem.l		(sp)+,d3-d5
	}
#else
	int negative = false;
	tt_uint16 al, ah;
	tt_uint32 lowlong, midlong, hilong;

	if (a < 0) { a = -a; negative = true; }
	if (b < 0) { b = -b; negative ^= true; }

	al = LOWORD(a); ah = HIWORD(a);

	midlong = USHORTMUL(ah, b);
	hilong = midlong & 0xFFFF0000;
	midlong <<= 16;
	midlong += 1 << 13;
	lowlong = USHORTMUL(al, b) + midlong;
	if (lowlong < midlong)
		hilong += fixed1;

	midlong = (lowlong >> 14) | (hilong << 2);
	return negative ? -(tt_int32) midlong : (tt_int32)midlong;
#endif
}

tt_int32 ShortMulDiv(tt_int32 a, tt_int16 b, tt_int16 c)
{
#ifdef asm68000only
	asm {
	;
	;	d3			a
	;	d4			b
	;	d5			c
	;	d6			sign
	;
		movem.l	d3-d6,-(sp)
		move.l	a,d3				; get a
		smi		d6				; set sign
		bpl.s		@1
		neg.l		d3				; a = -a
	@1:
		move.w	b,d4				; get b
		ext.l		d4				; make it a long
		bpl.s		@2
		not.b		d6				; change sign
		neg.l		d4				; b = -b
	@2:
		move.w	c,d5				; get c
		ext.l		d5				; make it a long
		bpl.s		@3
		not.b		d6				; change sign
		neg.l		d5				; c = -c
	@3:
		move.w	d3,d0			; get a.lo
		mulu.w	d4,d0			; compute a.lo * b
		move.l	d5,d1			; get c
		asr.l		#1,d1			; compute c / 2
		add.l		d1,d0			; compute a.lo * b + c / 2
		move.l	d0,d1
		clr.w	d0
		swap		d0				; get (a.lo * b + c / 2).hi
		move.l	d3,d2
		swap		d2				; get a.hi
		mulu.w	d4,d2			; compute a.hi * b
		add.l		d2,d0			; compute a.hi * b + (a.lo * b + c / 2).hi
		divu.w	d5,d0			; compute the hi word of the result
		bvs.s	@div0
		bmi.s	@div0
		swap		d0				; get the remainder of the division
		swap		d1
		move.w	d0,d1
		swap		d1				; put the remainder in hi word
		divu.w	d5,d1			; compute lo word of result
		move.w	d1,d0			;join result
		tst.b		d6				; check sign
		beq.s		@done
		neg.l		d0
		bra.s		@done
	@div0:
		moveq	#1,d0
		ror.l		#1,d0			; negative infinity
		tst.b		d6				; check sign
		bne.s		@done
		not.l		d0				; positive infinity
	@done:
		movem.l		(sp)+,d3-d6
	}
#else
	return MultiplyDivide(a, b, c);
#endif
}

#endif

shortFrac ShortFracDot(shortFrac a, shortFrac b)
{
    return( SHORTMUL(a,b) + (1 << 13)) >> 14;
}

shortFrac ShortFracDivide(shortFrac a, shortFrac b)
{
	tt_int32 aa = a;
	shortFrac quo;
	boolean neg = false;

	if (aa < 0)
		aa = -aa, neg ^= true;
	if (b < 0)
		b = -b, neg ^= true;

	quo = ((aa << 14) + (aa >> 1)) / b;
	return neg ? -quo : quo;
}

#define FASTMUL26LIMIT		46340
/*
 *	Total precision routine to multiply two 26.6 numbers		<3>
 */
F26Dot6 Mul26Dot6(F26Dot6 a, F26Dot6 b)
{
	int negative = false;
	tt_uint16 al, bl, ah, bh;
	tt_uint32 lowlong, midlong, hilong;

	if ((a <= FASTMUL26LIMIT) && (b <= FASTMUL26LIMIT) && (a >= -FASTMUL26LIMIT) && (b >= -FASTMUL26LIMIT))
		return ((a * b + (1 << 5)) >> 6);
	
	/* fast case */
	if (a < 0) { a = -a; negative = true; }
	if (b < 0) { b = -b; negative ^= true; }

	al = LOWORD(a); ah = HIWORD(a);
	bl = LOWORD(b); bh = HIWORD(b);

	midlong = USHORTMUL(al, bh) + USHORTMUL(ah, bl);
	hilong = USHORTMUL(ah, bh) + HIWORD(midlong);
	midlong <<= 16;
	midlong += 1 << 5;
	lowlong = USHORTMUL(al, bl) + midlong;
	hilong += lowlong < midlong;

	midlong = (lowlong >> 6) | (hilong << 26);
	return negative ? -(F26Dot6) midlong : (F26Dot6)midlong;
}

#define FASTDIV26LIMIT	(1L << 25)
/*
 *	Total precision routine to divide two 26.6 numbers			<3>
 */
F26Dot6 Div26Dot6(F26Dot6 num, F26Dot6 den)
{
	int negative = false;
	register tt_uint32 hinum, lownum, hiden, lowden, result, place;

	if (den == 0) return (num < 0 ) ? NEGINFINITY : POSINFINITY;

	if ( (num <= FASTDIV26LIMIT) && (num >= -FASTDIV26LIMIT) )			/* fast case */
		return (num << 6) / den;

	if (num < 0) { num = -num; negative = true; }
	if (den < 0) { den = -den; negative ^= true; }

	hinum = ((tt_uint32)num >> 26);
	lownum = ((tt_uint32)num << 6);
	hiden = den;
	lowden = 0;
	result = 0;
	place = HIBITSET;

	if (hinum >= hiden) return negative ? NEGINFINITY : POSINFINITY;

	while (place)
	{
		lowden >>= 1;
		if (hiden & 1) lowden += HIBITSET;
		hiden >>= 1;
		if (hiden < hinum)
		{
			hinum -= hiden;
			hinum -= lowden > lownum;
			lownum -= lowden;
			result += place;
		}
		else if (hiden == hinum && lowden <= lownum)
		{
			hinum = 0;
			lownum -= lowden;
			result += place;
		}
		place >>= 1;
	}

	return negative ? -(F26Dot6)result : (F26Dot6)result;
}

