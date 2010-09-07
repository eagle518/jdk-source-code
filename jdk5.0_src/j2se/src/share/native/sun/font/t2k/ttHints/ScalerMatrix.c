/*
 * @(#)ScalerMatrix.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
/*  
	scaler matrix.c
**
**	Routines to apply a 3x3 transformation matrix to a scalerBitmap.
**	Also a simple bit mover.
**	This code is shared by the NFNT scaler and the sbit scaler.
**
**	Copyright:	© 1992-1994 by Apple Computer, Inc., all rights reserved.
**
*/

 	#include "Hint.h"
 
boolean NonInvertibleMapping(const gxMapping* map)
{
	#ifdef DoWideNonInvertibleMapping
		/* This routine overlooks almost all cases where the matrix is 
				so distorted that hinting should be off. Therefore,
				whey even do it?  
	    */
		wide temp1, temp2;

		WideMultiply(map->map[0][0], map->map[1][1], &temp1);
		WideMultiply(map->map[0][1], map->map[1][0], &temp2);

		return temp1.hi == temp2.hi && temp1.lo == temp2.lo;
	#else
		return(0);
	#endif
}


/*
 *	Input gxMapping is  state->baseMap
 *
 *	Decompose state->baseMap into stretch and remaining
 *
 *	Returns TRUE if hints should be turned OFF 'cause the stretchBase is too small.
*/
boolean DecomposeMapping(transformState* state, boolean useIntegerScaling)
{
	Fixed xStretch, yStretch;

	/* MTE: IfDebugMessage(MxFlags(&state->baseMap) >= perspectiveState, "baseMap has perspective", 0);*/
	
	xStretch = ComputeMappingStretch(&state->baseMap, &yStretch);

	if (xStretch && yStretch)
	{	
		ResetMapping(&state->stretchBase);
		if (useIntegerScaling)
		{	state->stretchBase.map[0][0] = RoundedFixed(xStretch);
			state->stretchBase.map[1][1] = RoundedFixed(yStretch);
		}
		else
		{	state->stretchBase.map[0][0] = xStretch;
			state->stretchBase.map[1][1] = yStretch;
		}

		/*
		 *	Divide by the (unrounded) stretch factors
		*/
		ResetMapping(&state->remainingBase);
		{	register Fixed* remain = &state->remainingBase.map[0][0];
			register Fixed* base = &state->baseMap.map[0][0];
			
			*remain++ = FixedDivide(*base++, xStretch);
			*remain++ = FixedDivide(*base++, xStretch);
			++remain, ++base;		/* ignore translate X */
			*remain++ = FixedDivide(*base++, yStretch);
			*remain++ = FixedDivide(*base++, yStretch);
		}
		return false;
	}
	return true;	/* signal to turn hints off 'cause we're so small */
}


/*
 *	This came from something Richard read in a book
 *	Keep yStretch positive so that we don't confuse IUP and other instructions
*/
Fixed ComputeMappingStretch(const gxMapping* map, Fixed* yStretch)
{
	Fixed mag = Magnitude(map->map[0][0], map->map[0][1]);
	
	if (yStretch)
	{	Fixed detOverMag = MultiplyDivide(map->map[0][0], map->map[1][1], mag)
						- MultiplyDivide(map->map[1][0], map->map[0][1], mag);

		if (detOverMag < 0)
			detOverMag = -detOverMag;
		*yStretch = detOverMag;
	}
	return mag;
}



/* rwb 1/7/94
  * Invert a matrix that is the remainder part after decomposing into stretch and remainder.
  * rwb 7/22/94 BUGFIX - Because the stretch matrix is forced to have only positive entries, the 
  * entries in this inverse need to be multiplied by a sign factor that accounts for the absolute
  * magnitude in origianlly calculating the strectch factors.
  */
gxMapping *InvertRemainder( gxMapping* inverse, const gxMapping* remainder )
{
	int	signValue = 1;

	if (FixedMultiply(remainder->map[0][0], remainder->map[1][1]) < FixedMultiply(remainder->map[1][0], remainder->map[0][1]))
		signValue = -signValue;

	*inverse = *remainder;
	inverse->map[0][0] = signValue * remainder->map[1][1];
	inverse->map[1][0] = -signValue * remainder->map[1][0];
	inverse->map[0][1] = -signValue * remainder->map[0][1];
	inverse->map[1][1] = signValue * remainder->map[0][0];

	return inverse;
}	
