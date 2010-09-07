/*
 * @(#)GlyphOutline.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/

/* typedefs.h picks up uintptr_t needed for win64 */
#include "typedefs.h"

#include "Hint.h"

#ifdef ENABLE_TT_HINTING

#ifdef  debugging
#include <stdio.h>
#endif
 
#include "GlyphOutline.h"
#include "ScalerMatrix.h"
#include "FontMath.h"


/*
 *	Return the permSize
 * The buffer needs to be allocated such that alignment of pointers to
 * these types satisfies the constraints of the system.
 * This code is inherently vulnerable to alignment problems and probably
 * should be rewritten.
*/
tt_int32 ComputeElementSizes(fastInt contourCount, fastInt pointCount,  tt_int32* scratchSize)
{
  tt_int32 permSize;
  /*  oox[] + oox[y] will always be a multiple of 4 bytes in length,
   *  since although they are shorts, they are always in pairs */
  *scratchSize = 2 * pointCount * sizeof(short)	     /* oox[] + ooy[] */
                 + 2 * pointCount * sizeof(F26Dot6); /* ox[] + oy[] */

  /* sp[] + ep[] will always be a multiple of 4 bytes in length, but
   * unless pointCount is a multiple of 2, then onCurve[] + f[] will not be
   * a multiple of 4 bytes. Note the usage in SetElementPointers which
   * calculates the address of "ox" */
  permSize = 2 * pointCount * sizeof(F26Dot6)	/* x[] + y[] */
         + 2 * contourCount * sizeof(short)	/* sp[] + ep[] */
         + 2 * pointCount * sizeof(char);     	/* onCurve[] + f[] */
  permSize = ((permSize+3)>>2)<<2; /* round up to multiple of 4 bytes. */
  return permSize;
}

void ResetHintedOutline(fnt_ElementType* elem)
{
	tt_int32 size = (elem->pointCount + publicPhantomCount) * sizeof(F26Dot6);

	CopyBytes(elem->ox, elem->x, size);
	CopyBytes(elem->oy, elem->y, size);
}
 
 /*
 *	If permBuffer == nil, the use tempBuffer for everything
 *	if tempBuffer == nil, then only initialize the permanent arrays
 *
 *	Don't use outline->pointCount or outline->contourCount, since
 *	the buffers may be larger than is currently being used.
*/
void SetElementPointers(fnt_ElementType* outline, fastInt contourCount,
 fastInt pointCount, void* permBuffer, void* tempBuffer)
{
  static uintptr_t intAlignMask = ~(uintptr_t)0x3; /* mask out bottom 2 bits */

	outline->x = (F26Dot6*)(permBuffer ? permBuffer : tempBuffer);
	outline->y = outline->x + pointCount;
	outline->sp = (short*)(outline->y + pointCount);
	outline->ep = outline->sp + contourCount;
	outline->onCurve = (tt_uint8*)(outline->ep + contourCount);
	outline->f = outline->onCurve + pointCount;

	if (tempBuffer) {	
	    outline->ox = permBuffer ? (F26Dot6*)tempBuffer :
	    (F26Dot6*)((uintptr_t)(outline->f + pointCount +3) & intAlignMask);
	    outline->oy = outline->ox + pointCount;
	    outline->oox = (short*)(outline->oy + pointCount);
	    outline->ooy = outline->oox + pointCount;
	}
#ifdef debugging
	else {
	   outline->ox = kBusErrorValue;
	   outline->oy = kBusErrorValue;
	   outline->oox = kBusErrorValue;
	   outline->ooy = kBusErrorValue;
	}
#endif
}
  
#endif
 
