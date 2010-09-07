/*
 * @(#)SMOD_Volume_Scaler.c	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "SMOD_Volume_Scaler.c"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**
**
** Sound Modifier:  Amplifier/Volume Scaler.
**
** Parameter 1 is the multiplication factor (as a whole number.)  Set this
** number higher for more amplification.
**
** Parameter 2 is the division factor (also a whole number.)  Set this
** number higher for lower volume.
**
** The intermediate results are calculated to 32 bit precision.  Volume
** scaling of, for example, 100 / 99, will work as expected (a 1% rise in
** volume.)
**
**
** Written by James L. Nitchals.
**
** 'C' version by Steve Hales.
**
** Modification History:
**
**	10/5/95		Created
**	6/30/96		Changed font and retabbed
**	12/30/96	Changed copyright
**	2/11/98		Changed copyright, and did some house cleaning
*/
/*****************************************************************************/

#include "GenSnd.h"
#include "GenPriv.h"

#include "SMOD.h"

void VolumeAmpScaler(register unsigned char *pSample, INT32 length, INT32 param1, INT32 param2)
{
    register INT32	count, scaleCount, scale;
    unsigned char	scaledLookup[256];

    if (pSample && length && param1 && param2 && (param1 != param2))
	{
	    // build new scaling table
	    scaleCount = param2 / 2;
	    for (count = 0; count < 256; count++)
		{
		    scale = (128 - count) * param1;
		    if (scale < 0)
			{
			    scale -= scaleCount;
			}
		    else
			{
			    scale += scaleCount;
			}
		    scale = scale / param2;
		    // Clip samples to max headroom
		    if (scale > 127)
			{
			    scale = 127;
			}
		    if (scale < -128)
			{
			    scale = -128;
			}
		    scaledLookup[count] = scale + 128;
		}
	    // Scale the samples via a the new lookup table
	    for (count = 0; count < length; count++)
		{
		    scale = pSample[count];
		    pSample[count] = scaledLookup[scale];
		}
	}
}

// EOF of SMOD_Volume_Scaler.c
