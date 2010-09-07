/*
 * @(#)util.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * UTIL.C
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * Author: Sampo Kaasila
 *
 * This software is the property of Type Solutions, Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * This information in this software is subject to change without notice
 */
#include "syshead.h"

#include "dtypes.h"
#include "config.h"
#include "tsimem.h"
#include "util.h"


/*
 *
 */
unsigned char *ReadFileIntoMemory( tsiMemObject *mem, const char *fname,tt_uint32 *size )
{
	unsigned char *data;
	int error;
	size_t count;
	FILE *fp;
	
	assert( fname != NULL );
	fp		= fopen(fname, "rb"); assert( fp != NULL );
	error	= fseek( fp, 0L, SEEK_END ); assert( error == 0 );
	*size	= ftell( fp ); assert( ferror(fp) == 0 );
	error	= fseek( fp, 0L, SEEK_SET ); assert( error == 0 ); /* rewind */
	data	= (unsigned char *)tsi_AllocMem( mem, sizeof( char ) * *size ); assert( data != NULL );
	count	= fread( data, sizeof( char ), *size, fp ); assert( ferror(fp) == 0 && count == *size );
	error	= fclose( fp ); assert( error == 0 );
	return	data; /******/
}

#ifdef ENABLE_WRITE
/*
 *
 */
void WriteDataToFile( const char *fname, void *dataIn, tt_uint32 size )
{
	int error;
	size_t count;
	FILE *fp	= fopen( fname, "wb" ); assert( fp != NULL );
	count		= fwrite( dataIn, sizeof( char ), size, fp ); assert( ferror(fp) == 0 && count == size );
	error		= fclose( fp ); assert( error == 0 );
}
#endif


/*
 * Slow Bubbles everywhere
 */
void util_SortShortArray( short *a, tt_int32 n )
{
	register short change, tmp;
	register tt_int32 i;
	
	n--;
	for ( change = 1; change; ) {
		change = 0;
		for ( i = 0; i < n; i++ ) {
			if ( a[i+1] < a[i] ) {
				change = 1;
				/* swap */
				tmp = a[i];
				a[i] = a[i+1];
				a[i+1] = tmp;
			}
		}
	}
}

/* 
 * Return mA * mB
 * This is an integer implementation!
 */
F16Dot16 util_FixMul( F16Dot16 mA, F16Dot16 mB )
{
	tt_uint16 mA_Hi, mA_Lo;
	tt_uint16 mB_Hi, mB_Lo;
	tt_uint32 d1, d2, d3;
	F16Dot16 result;
	int sign;
	
	
	if ( mA < 0 ) {
		mA = -mA;
		sign = -1;
		if ( mB < 0 ) {
			sign = 1;
			mB = -mB;
		}
	} else {
		sign = 1;
		if ( mB < 0 ) {
			sign = -1;
			mB = -mB;
		}
	}
	/*
	result = (F16Dot16)((float)mA/65536.0 * (float)mB);
	result *= sign;
	return result;
	*/

	mA_Hi = (tt_uint16)(mA>>16);
	mA_Lo = (tt_uint16)(mA);
	mB_Hi = (tt_uint16)(mB>>16);
	mB_Lo = (tt_uint16)(mB);
	
	/*
			mB_Hi 	mB_Lo
	  X		mA_Hi 	mA_Lo
	------------------
	d1		d2		d3
	*/
	d3  = mA_Lo * mB_Lo;		/* <<  0 */
	d2  = mA_Lo * mB_Hi;		/* << 16 */
	d2 += mA_Hi * mB_Lo;		/* << 16 */
	d1  = mA_Hi * mB_Hi;		/* << 32 */
	
	result 	 = (d1 << 16) + d2 + (d3 >> 16);
	result	*= sign;
	return result; /*****/
}



/* 
 * Return mA / mB
 * This is an integer implementation!
 * It is actually 32% faster than the old floating point implementation on even a 604 PPC !!! :-)
 */
F16Dot16 util_FixDiv( F16Dot16 mA, F16Dot16 mB )
{
	int sign;
	tt_uint32 high16, low16;
	F16Dot16 Q;

	if ( mA < 0 ) {
		mA = -mA;
		sign = -1;
		if ( mB < 0 ) {
			sign = 1;
			mB = -mB;
		}
	} else {
		sign = 1;
		if ( mB < 0 ) {
			sign = -1;
			mB = -mB;
		}
	}

	high16 = (tt_uint32)mA / (tt_uint32)mB;
	low16  = (tt_uint32)mA % (tt_uint32)mB;
	/* assert( high16 * mB + low16 == mA ); */
	/* mA / mB = high16 + low16/mB :-) !!! */
	high16 <<= 16;
	while ( low16 > 0xffff ) {
		low16 >>= 1;
		mB >>= 1;
		/* mB is always > low16, and low16 > 0xffff/2 => mB is never zero because of this loop */
	}
	low16  <<= 16;
	low16   /= mB;
	
	Q = high16 + low16;
	Q *= sign;
	return Q; /*****/
}

#ifdef OLD
/*
 * Returns sin( in * pi/180.0 )
 * Valid for input ranges between 0 and 90 degrees.
 * Approximates the sinus function with just 7 FixMuls :-)
 * The max error is about 0.002640 for in = 90.0
 */
F16Dot16 util_FixSinOLD( F16Dot16 in )
{
	F16Dot16 node1pow1, node3;  /* working variables */
	F16Dot16 node1pow2;
	F16Dot16 node1pow3;
	F16Dot16 out;
	
	if ( in > 90 * 0x10000 ) in = 90 * 0x10000;
	else if ( in < 0 ) in = 0;

	node1pow1   = -113041 + util_FixMul( 2512, in);
	node1pow2	= util_FixMul(node1pow1,node1pow1);
	node1pow3	= util_FixMul(node1pow1,node1pow2);
	
	node3    = 14830
			   + util_FixMul(68243, node1pow1)
			   - util_FixMul(14871, node1pow2)
	           - util_FixMul( 2280, node1pow3);
	
	out =  41697 + util_FixMul( 20249, node3 );
	if ( out < 0 ) out = 0;
	return out; /*****/
}
#endif

/*
 * Returns sin( in * pi/180.0 )
 * Valid for input ranges between 0 and 90 degrees.
 * Approximates the sin function with just 9 multiplies and one divide.
 * The max error is = 0.000063 ( => Roughly 14 out of 16 bits are correct )
 * for this genetically (!) evolved function.
 */
F16Dot16 util_FixSin( F16Dot16 in )
{
	F16Dot16 node1pow1;  /* working variables */
	F16Dot16 node1pow2;
	F16Dot16 node1pow3;
	F16Dot16 node1pow4;
	F16Dot16 node1pow5;
	F16Dot16 out;
	
	if ( in > 90 * 0x10000 ) in = 90 * 0x10000;
	else if ( in < 0 ) in = 0; /* 0.0 -- 90.0 */
	
	node1pow1   = in;
	node1pow1  /= 90; /* => +0 --- 1.0 */
	node1pow1  -= 0x8000; /* -0.5 .. 0.5 */

	node1pow2	= (node1pow1 * node1pow1) >> 15; /* 0.5 -- 0.5 */
	node1pow3	= (node1pow1 * node1pow2) >> 15; /* 0.5 -- 0.5 */
	node1pow4	= (node1pow2 * node1pow2) >> 15; /* 0.5 -- 0.5 */
	node1pow5	= (node1pow2 * node1pow3) >> 15; /* 0.5 -- 0.5 */

	out    	 = 46343 +  
				+ (( node1pow1 *  18198 ) >> 14)
				+ (( node1pow2 * -14284 ) >> 15)
				+ (( node1pow3 *  -3742 ) >> 15)
				+ (( node1pow4 *    711 ) >> 15)
				+ (( node1pow5 *    114 ) >> 15);
				
	if ( out > 0x10000 ) out = 0x10000;
	return out; /*****/
}


#ifdef OLDOLD

/* Return N / D */
static F16Dot16 util_FixDivOld( F16Dot16 N, F16Dot16 D )
{
	F16Dot16 Q;
	/* I think we need to eliminate this floating point math and make our own FixDiv instead */
	/* Q = (long)(((float)N / (float)D ) * (float)ONE16Dot16); */
	Q = (tt_int32)(((double)N / (double)D ) * (double)ONE16Dot16);
	return Q; /*****/
	
}

void TESTFIXDIV( void )
{
	F16Dot16 N, D, Q1, Q2, Q3;
	float fQ;
	tt_int32 tick1, tick2, testCount;
	
	
	tick1 = TickCount();
	for ( N = 0; N < 1000; N++ ) {
		for ( D = 0; D < 4000; D++ ) {
			Q2 = util_FixMul( N, D );
		}
	}
	tick2 = TickCount();
	printf("util_FixMul ticks = %d\n", tick2-tick1 );
	tick1 = TickCount();
	for ( N = 0; N < 1000; N++ ) {
		for ( D = 0; D < 4000; D++ ) {
			Q2 = util_FixDiv( N, D );
		}
	}
	tick2 = TickCount();
	printf("util_FixDiv ticks = %d\n", tick2-tick1 );
	tick1 = TickCount();
	for ( N = 0; N < 1000; N++ ) {
		for ( D = 0; D < 4000; D++ ) {
			Q2 = util_FixDivOld( N, D );
		}
	}
	tick2 = TickCount();
	printf("util_FixDivOld ticks = %d\n", tick2-tick1 );
	
	printf("Testing FixDiv\n");
	testCount = 0;
	for ( N = -20000000L; N <= 20000000L; N += 1003 ) {
		for ( fQ = 0.01; fQ < 100.0;  fQ *= 1.01 ) {
			Q1 = ONE16Dot16 * fQ;
			D = N / fQ;
			if ( D == 0 ) continue; /*****/
			Q2 = util_FixDiv( N, D );
			Q3 = util_FixDivOld( N, D );
			if ( false && Q2 != Q3 ) {
				printf("%d / %d : Q1 = %d, Q2 = %d\, Q3 = %d\n", N, D, Q1, Q2, Q3 );
			}
			assert( Q2 >= Q3 -2 && Q2 <= Q3 + 2 );

			Q1 = ONE16Dot16 * (-fQ);
			D = N / (-fQ);
			Q2 = util_FixDiv( N, D );
			Q3 = util_FixDivOld( N, D );
			if ( false && Q2 != Q3 ) {
				printf("%d / %d : Q1 = %d, Q2 = %d\, Q3 = %d\n", N, D, Q1, Q2, Q3 );
			}
			assert( Q2 >= Q3 -2 && Q2 <= Q3 + 2 );
			testCount += 2;
		}
	}
	printf("OK FixDiv, did %d tests\n", testCount );
}
#endif /* OLDOLD */

/*
 * Description:		returns sqrt( A*A + B*B );
 * How used:		Call with the data (dx, and dy).
 * Side Effects: 	None.
 * Return value: 	sqrt( A*A + B*B ).
 */
F16Dot16 util_EuclidianDistance( register F16Dot16 A, register F16Dot16 B )
{
	F16Dot16 root;
		
	if ( A < 0 ) A = -A;
	if ( B < 0 ) B = -B;
	
	if ( A == 0 ) {
		return B; /*****/
	} else if ( B == 0 ) {
		return A; /*****/
	} else {
		root	= A > B ? A + (B>>1) : B + (A>>1); /* Do an initial approximation, in root */

		/* Ok, now enter the Newton Raphson iteration sequence */
		root = (root + util_FixMul( A, util_FixDiv( A, root) ) + util_FixMul( B, util_FixDiv( B, root) ) + 1) >> 1; 
		root = (root + util_FixMul( A, util_FixDiv( A, root) ) + util_FixMul( B, util_FixDiv( B, root) ) + 1) >> 1; 
		root = (root + util_FixMul( A, util_FixDiv( A, root) ) + util_FixMul( B, util_FixDiv( B, root) ) + 1) >> 1; 
		/* Now the root should be correct, so get out of here! */
#ifdef OLD
	F16Dot16 square;
		square	= util_FixMul( A, A) + FixMul( B, B);

		do {
			/* root = ((old_root = root) + util_FixDiv( square, root ) + 1 ) >> 1; we may have an overflow in square, so do it as below instead */
printf("root = %d\n", root );
			root = ((old_root = root) + util_FixMul( A, util_FixDiv( A, root) ) + util_FixMul( B, util_FixDiv( B, root) ) + 1) >> 1; 
		} while (old_root != root );
		assert( util_FixMul( root, root ) < util_FixMul( square, 65536+65 ) ); 
		assert( util_FixMul( root, root ) > util_FixMul( square, 65536-65 ) );
#endif
		return root; /*****/
	}
}
