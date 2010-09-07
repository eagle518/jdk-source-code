/*
 * @(#)X_Decompress.c	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	X_Decompress.c
**
**	Tools for decompressing/encrypting/decrypting audio data
**	Confidential-- Internal use only
**
**	History	-
**	8/18/96		Created
**				Moved XExpandMace1to3 & XExpandMace1to6 from X_API.c
**	9/22/96		Changed XExpandMacADPCMto16BitLinear to XExpandMacADPCMtoXBitLinear
**				to support 8 bit expansion
**	11/14/96	Removed dependancy on MacSpecificSMS.h
**	12/19/96	Fixed an unassgined variable in IMA decoder
**	12/30/96	Changed copyright
**	1/12/97		Added XDecryptData and XEncryptData
**	1/20/97		Added XEncryptedStrCpy && XEncryptedStrLen
**	1/22/97		Changed strcpy functions to XStrCpy
**	1/23/97		Removed platform include files
**	2/21/97		Fixed stereo IMA decoding bug
**	5/3/97		Fixed a few potential problems that the MOT compiler found
**	5/15/97		Put in code for ADPCM decoding for WAVE files & AIFF files
**	9/30/97		Wrapped XEncryptData with USE_CREATION_API == TRUE
**	12/16/97	Moe: removed compiler warnings
**	2/11/98		Put code wrappers around XExpandMace1to6 & XExpandMace1to3 & 
**				XGetCompressionName to eliminate it from the codebase if not used.
**	3/12/98		MOE: Moved the IMA(ADPCM) stuff to X_IMA.c
**	2/12/99		Renamed USE_HAE_FOR_MPEG to USE_MPEG_DECODER
**	3/1/99		Added XGetShortCompressionName
**	
*/
/*****************************************************************************/
#include "X_API.h"
#include "X_Formats.h"

#if USE_CREATION_API == TRUE
#include "g72x.h"
#endif

#if USE_CREATION_API == TRUE
void XGetShortCompressionName(INT32 compressionType, void *cName)
{

    switch (compressionType)
	{
	case C_IMA4:
	    XStrCpy((char *)cName, "IMA 4:1");
	    break;
	case C_MACE3:
	    XStrCpy((char *)cName, "MACE 3:1");
	    break;
	case C_MACE6:
	    XStrCpy((char *)cName, "MACE 6:1");
	    break;
	case C_ULAW:
	    XStrCpy((char *)cName, "ulaw");
	    break;
	case C_ALAW:
	    XStrCpy((char *)cName, "alaw");
	    break;
#if USE_MPEG_DECODER != 0
	case C_MPEG_32:
	    XStrCpy((char *)cName, "MPEG 32k");
	    break;
	case C_MPEG_40:
	    XStrCpy((char *)cName, "MPEG 40k");
	    break;
	case C_MPEG_48:
	    XStrCpy((char *)cName, "MPEG 48k");
	    break;
	case C_MPEG_56:
	    XStrCpy((char *)cName, "MPEG 56k");
	    break;
	case C_MPEG_64:
	    XStrCpy((char *)cName, "MPEG 64k");
	    break;
	case C_MPEG_80:
	    XStrCpy((char *)cName, "MPEG 80k");
	    break;
	case C_MPEG_96:
	    XStrCpy((char *)cName, "MPEG 96k");
	    break;
	case C_MPEG_112:
	    XStrCpy((char *)cName, "MPEG 112k");
	    break;
	case C_MPEG_128:
	    XStrCpy((char *)cName, "MPEG 128k");
	    break;
	case C_MPEG_160:
	    XStrCpy((char *)cName, "MPEG 160k");
	    break;
	case C_MPEG_192:
	    XStrCpy((char *)cName, "MPEG 192k");
	    break;
	case C_MPEG_224:
	    XStrCpy((char *)cName, "MPEG 224k");
	    break;
	case C_MPEG_256:
	    XStrCpy((char *)cName, "MPEG 256k");
	    break;
	case C_MPEG_320:
	    XStrCpy((char *)cName, "MPEG 320k");
	    break;
#endif
	}
}

void XGetCompressionName(INT32 compressionType, void *cName)
{
    switch (compressionType)
	{
	case C_NONE:
	    XStrCpy((char *)cName, "no compression");
	    break;
	case C_IMA4:
	    //			XStrCpy((char *)cName, "CCITT G.721 ADPCM compression (IMA 4 to 1)");
	    XStrCpy((char *)cName, "ADPCM compression (IMA 4 to 1)");
	    break;
	case C_MACE3:
	    XStrCpy((char *)cName, "Apple MACE 3 to 1");
	    break;
	case C_MACE6:
	    XStrCpy((char *)cName, "Apple MACE 6 to 1");
	    break;
	case C_ULAW:
	    XStrCpy((char *)cName, "ulaw 2 to 1");
	    break;
	case C_ALAW:
	    XStrCpy((char *)cName, "alaw 2 to 1");
	    break;
#if USE_MPEG_DECODER != 0
	case C_MPEG_32:
	    XStrCpy((char *)cName, "MPEG I layer 3, 32k bits");
	    break;
	case C_MPEG_40:
	    XStrCpy((char *)cName, "MPEG I layer 3, 40k bits");
	    break;
	case C_MPEG_48:
	    XStrCpy((char *)cName, "MPEG I layer 3, 48k bits");
	    break;
	case C_MPEG_56:
	    XStrCpy((char *)cName, "MPEG I layer 3, 56k bits");
	    break;
	case C_MPEG_64:
	    XStrCpy((char *)cName, "MPEG I layer 3, 64k bits");
	    break;
	case C_MPEG_80:
	    XStrCpy((char *)cName, "MPEG I layer 3, 80k bits");
	    break;
	case C_MPEG_96:
	    XStrCpy((char *)cName, "MPEG I layer 3, 96k bits");
	    break;
	case C_MPEG_112:
	    XStrCpy((char *)cName, "MPEG I layer 3, 112k bits");
	    break;
	case C_MPEG_128:
	    XStrCpy((char *)cName, "MPEG I layer 3, 128k bits");
	    break;
	case C_MPEG_160:
	    XStrCpy((char *)cName, "MPEG I layer 3, 160k bits");
	    break;
	case C_MPEG_192:
	    XStrCpy((char *)cName, "MPEG I layer 3, 192k bits");
	    break;
	case C_MPEG_224:
	    XStrCpy((char *)cName, "MPEG I layer 3, 224k bits");
	    break;
	case C_MPEG_256:
	    XStrCpy((char *)cName, "MPEG I layer 3, 256k bits");
	    break;
	case C_MPEG_320:
	    XStrCpy((char *)cName, "MPEG I layer 3, 320k bits");
	    break;
#endif
	}
}
#endif

#if X_PLATFORM == X_MACINTOSH
// expand data 1 to 3 use MacOS MACE compression
void XExpandMace1to3(void *inBuffer, void *outBuffer, UINT32 length, 
		     void * inState, void * outState, 
		     UINT32 numChannels, UINT32 whichChannel)
{
    Exp1to3(inBuffer, outBuffer, length, (StateBlockPtr)inState, (StateBlockPtr)outState, numChannels, whichChannel);
}

// expand data 1 to 6 use MacOS MACE compression
void XExpandMace1to6(void *inBuffer, void *outBuffer, UINT32 length, 
		     void * inState, void * outState, 
		     UINT32 numChannels, UINT32 whichChannel)
{
    Exp1to6(inBuffer, outBuffer, length, (StateBlockPtr)inState, (StateBlockPtr)outState, numChannels, whichChannel);
}
#endif

#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	SEG_MASK	(0x70)		/* Segment field mask. */
#define	SEG_SHIFT	(4)		/* Left shift for segment number. */
#define	SIGN_BIT	(0x80)		/* Sign bit for a A-law byte. */

/*
 * st_alaw_to_linear() - Convert an A-law value to 16-bit linear PCM
 *
 */
static int st_alaw_to_linear(unsigned char	a_val)
{
    int		t;
    int		seg;

    a_val ^= 0x55;

    t = (a_val & QUANT_MASK) << 4;
    seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
    switch (seg) {
    case 0:
	t += 8;
	break;
    case 1:
	t += 0x108;
	break;
    default:
	t += 0x108;
	t <<= seg - 1;
    }
    return ((a_val & SIGN_BIT) ? t : -t);
}

/*
** This routine converts from ulaw to 16 bit linear.
**
** Craig Reese: IDA/Supercomputing Research Center
** 29 September 1989
**
** References:
** 1) CCITT Recommendation G.711  (very difficult to follow)
** 2) MIL-STD-188-113,"Interoperability and Performance Standards
**     for Analog-to_Digital Conversion Techniques,"
**     17 February 1987
**
** Input: 8 bit ulaw sample
** Output: signed 16 bit linear sample
*/
#if 0
static short int st_ulaw_to_linear(unsigned char ulawbyte)
{
    static short int exp_lut[8] = { 0, 132, 396, 924, 1980, 4092, 8316, 16764 };
    short int sign, exponent, mantissa, sample;

    ulawbyte = ~ ulawbyte;
    sign = ( ulawbyte & 0x80 );
    exponent = ( ulawbyte >> 4 ) & 0x07;
    mantissa = ulawbyte & 0x0F;
    sample = exp_lut[exponent] + ( mantissa << ( exponent + 3 ) );
    if ( sign != 0 )
	{
	    sample = -sample;
	}
    return sample;
}
#else
static short int st_ulaw_to_linear(unsigned char ulawbyte)
{
    static const short int ulaw_exp_table[256] = {
	-32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,
	-23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,
	-15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,
	-11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,
	-7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
	-5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
	-3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
	-2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
	-1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
	-1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
	-876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
	-620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
	-372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
	-244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
	-120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
	-56,   -48,   -40,   -32,   -24,   -16,    -8,     0,
	32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
	23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
	15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
	11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
	7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
	5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
	3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
	2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
	1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
	1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
	876,   844,   812,   780,   748,   716,   684,   652,
	620,   588,   556,   524,   492,   460,   428,   396,
	372,   356,   340,   324,   308,   292,   276,   260,
	244,   228,   212,   196,   180,   164,   148,   132,
	120,   112,   104,    96,    88,    80,    72,    64,
	56,    48,    40,    32,    24,    16,     8,     0};

    return ulaw_exp_table[ulawbyte];
}


#endif

void XExpandULawto16BitLinear(unsigned char *pSource, short int *pDest, INT32 frames, INT32 channels)
{
    INT32	count;

    frames *= channels;
    for (count = 0; count < frames; count++)
	{
	    *pDest = st_ulaw_to_linear(*pSource);
	    pSource++;
	    pDest++;
	}
}


void XExpandALawto16BitLinear(unsigned char *pSource, short int *pDest, INT32 frames, INT32 channels)
{
    INT32	count;

    frames *= channels;
    for (count = 0; count < frames; count++)
	{
	    *pDest = (short int)st_alaw_to_linear(*pSource);
	    pSource++;
	    pDest++;
	}
}

#if USE_CREATION_API == TRUE
// compress ALaw or ULaw
void XCompressLaw(SndCompressionType compressionType, short int *pSource, char *pDest, 
		  UINT32 frames, UINT32 channels)
{
    UINT32	count;

    if (pSource && pDest && frames)
	{
	    frames *= channels;
	    for (count = 0; count < frames; count++)
		{
		    switch (compressionType)
			{
			case C_ULAW:
			    *pDest = linear2ulaw(*pSource);
			    break;
			case C_ALAW:
			    *pDest = linear2alaw(*pSource);
			    break;
			}
		    pSource++;
		    pDest++;
		}
	}
}
#endif

#define INIT_R		(unsigned short int)56549
#define C1			(unsigned short int)52845
#define C2			(unsigned short int)22719

static unsigned int R;

static INLINE unsigned char PV_Decrypt(register unsigned char cipher)
{
    unsigned char plain;

    plain = (cipher ^ (R >> 8));
    R = (cipher + R) * C1 + C2;
    return plain;
}

static INLINE unsigned char PV_Encrypt(register unsigned char plain)
{
    unsigned char cipher;

    cipher = (plain ^ (R >> 8));
    R = (cipher + R) * C1 + C2;
    return cipher;
}

short int XEncryptedStrLen(char *src)
{
    short int len;

    len = -1;
    if (src == NULL)
	{
	    src = "";
	}

    // decrypt
    R = INIT_R;
    do
	{
	    len++;
	} while  (PV_Decrypt(*src++));

    return len;
}


// standard strcpy, but with crypto controls
// Copies 'C' string s2 into s1
char * XEncryptedStrCpy(char *dest, char *src, short int copy)
{
    char *sav;
    char data;

    if (src == NULL)
	{
	    src = "";
	}

    // decrypt
    R = INIT_R;
    sav = dest;
    if (dest)
	{
	    while (1)
		{
		    data = PV_Decrypt(*src);
		    if (data)
			{
			    switch (copy)
				{
				case X_SOURCE_ENCRYPTED:
				    *dest++ = data;
				    break;
				case X_SOURCE_DEST_ENCRYPTED:
				    *dest++ = *src;
				    break;
				}
			}
		    else
			{
			    break;
			}
		    src++;
		}
	    *dest = 0;
	}
    return sav;
}

// duplicate encrypted string, but decrypt clone
char * XDecryptAndDuplicateStr(char *src)
{
    char *dup;

    dup = NULL;
    if (src)
	{
	    dup = (char *)XNewPtr(XEncryptedStrLen(src)+1);
	    if (dup)
		{
		    // decrypt and copy
		    XEncryptedStrCpy(dup, src, X_SOURCE_ENCRYPTED);
		}
	}
    return dup;
}

#if USE_CREATION_API == TRUE
// Encrypt a block of data. This should be U.S. munitions safe. ie below 40 bit
void XEncryptData(void *pData, UINT32 size)
{
    register unsigned char *pByte, *pEnd;

    if (pData && size)
	{
	    // encrypt
	    R = INIT_R;
	    pByte = (unsigned char *)pData;
	    pEnd = pByte + size;
	    while (pByte < pEnd)
		{
		    *pByte = PV_Encrypt(*pByte);
		    pByte++;
		}
	}
}
#endif	// USE_CREATION_API == TRUE

// Decrypt a block of data. This should be U.S. munitions safe. ie below 40 bit
void XDecryptData(void *pData, UINT32 size)
{
    register unsigned char *pByte, *pEnd;

    if (pData && size)
	{
	    // decrypt
	    R = INIT_R;
	    pByte = (unsigned char *)pData;
	    pEnd = pByte + size;
	    while (pByte < pEnd)
		{
		    *pByte = PV_Decrypt(*pByte);
		    pByte++;
		}
	}
}


// EOF of X_Decompress.c

