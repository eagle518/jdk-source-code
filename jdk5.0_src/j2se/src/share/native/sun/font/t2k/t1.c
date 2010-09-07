/*
 * @(#)t1.c	1.37 04/05/28
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * T1.c
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * Author: Sampo Kaasila
 *.
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

#include "config.h"
#include "dtypes.h"
#include "tsimem.h"
#include "t2kstrm.h"
#include "glyph.h"
#include "truetype.h"
#include "t1.h"
#include "util.h"

#include <ctype.h>

#ifdef T1_OR_T2_IS_ENABLED
/* T1 or T2 (cff) */

extern int debugOn;
/* #define LOG_CMD( s, d ) if (debugOn)  printf("%s %d\n", s, d ) */
/* #define LOG_CMD( s, d ) printf("%s %d\n", s, d ) */
/* #define LOG_CMD( s, d ) NULL */

#define LOG_CMD( s, d ) {}

static short GetGlyphYMax( GlyphClass *glyph )
{
    register int i, limit = glyph->pointCount;
    register short *ooy = glyph->ooy;
    register short ymax = ooy[0];

    for ( i = 1; i < limit; i++ ) {
        if ( ooy[i] > ymax ) {
            ymax = ooy[i];
        }
    }
    return ymax; /*****/
}

static short GetGlyphYMin( GlyphClass *glyph )
{
    register int i, limit = glyph->pointCount;
    register short *ooy = glyph->ooy;
    register short ymin = ooy[0];

    for ( i = 1; i < limit; i++ ) {
        if ( ooy[i] < ymin ) {
            ymin = ooy[i];
        }
    }
    return ymin; /*****/
}

static short GetGlyphXMin( GlyphClass *glyph )
{
    register int i, limit = glyph->pointCount;
    register short *oox = glyph->oox;
    register short xmin = oox[0];

    for ( i = 1; i < limit; i++ ) {
        if ( oox[i] < xmin ) {
            xmin = oox[i];
        }
    }
    return xmin; /*****/
}


/*
 *
 */
static void FlipContourDirection(GlyphClass *glyph)
{
    short	ctr, j;
    short	*oox = 	glyph->oox;
    short	*ooy = 	glyph->ooy;
    tt_uint8 	*onCurve = glyph->onCurve;

    for ( ctr = 0; ctr < glyph->contourCount; ctr++ ) {
        short	flips, start, end;

        start	= glyph->sp[ctr];
        end		= glyph->ep[ctr];

        flips = (short)((end - start)/2);
        start++;
        for ( j = 0; j < flips; j++ ) {
            tt_int16	tempX, tempY;
            tt_uint8	pointType;
            tt_int16   indexA = (tt_int16)(start + j);
            tt_int16   indexB = (tt_int16)(end   - j);

            tempX				= oox[indexA];
            tempY				= ooy[indexA];
            pointType			= onCurve[indexA];

            oox[indexA]			= oox[indexB];
            ooy[indexA]			= ooy[indexB];
            onCurve[indexA]		= onCurve[indexB];

            oox[indexB]			= tempX;
            ooy[indexB]			= tempY;
            onCurve[indexB]		= pointType;
        }
    }
}

#endif /* T1_OR_T2_IS_ENABLED */

#ifdef ENABLE_T1

#ifdef ENABLE_MAC_T1
#include <Resources.h>
#endif

/*
TTD:
..
SomeDay start using InputStream....
..

DONE:
Make lenIV variable
Reverse outline direction
Parse composites
T3 scan-conversion

*/

/* Black Book */
const unsigned short int c1 = 52845;
const unsigned short int c2 = 22719;

/* Unknown unicode character */
#define UNKNOWN_CHARACTER 0xFFFF

/* Maximum number of mapping from a Postscript char name to unicode value */
#define MAX_NUM_MAPPING 3

typedef struct {
    tt_uint16 unicodeIndex;
    tt_uint8  numMapping;
    char *name;
} psNameToUnicode;

/* The following table maps PS names to its corresponding unicode values.
   This includes most glyphs listed in AGL (Adobe Glyph List)
   without the glyphs in the private use area (0xe000-0xf8ff),
   and some special names used in fonts made by others, mainly Monotype and B&H.

   However, since many character names are not standarized beyond 0x7F,
   some Type1 font characters may not map correctly to the exact glyph,
   or the glyph may be reported to be missing.

   For more info, see
   http://partners.adobe.com/asn/developer/typeforum/unicodegn.html */

static const psNameToUnicode psNameToUnicodeTable[] = {
    { 0x0041, 1, "A" },                    /* LATIN CAPITAL LETTER A */
    { 0x00c6, 1, "AE" },                   /* LATIN CAPITAL LETTER AE */
    { 0x01fc, 1, "AEacute" },              /* LATIN CAPITAL LETTER AE WITH ACUTE */
    { 0x00c1, 1, "Aacute" },               /* LATIN CAPITAL LETTER A WITH ACUTE */
    { 0x0102, 1, "Abreve" },               /* LATIN CAPITAL LETTER A WITH BREVE */
    { 0x00c2, 1, "Acircumflex" },          /* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
    { 0x00c4, 1, "Adieresis" },            /* LATIN CAPITAL LETTER A WITH DIAERESIS */
    { 0x00c0, 1, "Agrave" },               /* LATIN CAPITAL LETTER A WITH GRAVE */
    { 0x0391, 1, "Alpha" },                /* GREEK CAPITAL LETTER ALPHA */
    { 0x0386, 1, "Alphatonos" },           /* GREEK CAPITAL LETTER ALPHA WITH TONOS */
    { 0x0100, 1, "Amacron" },              /* LATIN CAPITAL LETTER A WITH MACRON */
    { 0x0104, 1, "Aogonek" },              /* LATIN CAPITAL LETTER A WITH OGONEK */
    { 0x00c5, 1, "Aring" },                /* LATIN CAPITAL LETTER A WITH RING ABOVE */
    { 0x01fa, 1, "Aringacute" },           /* LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE */
    { 0x00c3, 1, "Atilde" },               /* LATIN CAPITAL LETTER A WITH TILDE */
    { 0x0042, 1, "B" },                    /* LATIN CAPITAL LETTER B */
    { 0x0392, 1, "Beta" },                 /* GREEK CAPITAL LETTER BETA */
    { 0x0043, 1, "C" },                    /* LATIN CAPITAL LETTER C */
    { 0x0106, 1, "Cacute" },               /* LATIN CAPITAL LETTER C WITH ACUTE */
    { 0x010c, 1, "Ccaron" },               /* LATIN CAPITAL LETTER C WITH CARON */
    { 0x00c7, 1, "Ccedilla" },             /* LATIN CAPITAL LETTER C WITH CEDILLA */
    { 0x0108, 1, "Ccircumflex" },          /* LATIN CAPITAL LETTER C WITH CIRCUMFLEX */
    { 0x010a, 1, "Cdot" },                 /* LATIN CAPITAL LETTER C WITH DOT ABOVE (Non-Adobe) */
    { 0x010a, 1, "Cdotaccent" },           /* LATIN CAPITAL LETTER C WITH DOT ABOVE */
    { 0x03a7, 1, "Chi" },                  /* GREEK CAPITAL LETTER CHI */
    { 0x0044, 1, "D" },                    /* LATIN CAPITAL LETTER D */
    { 0x010e, 1, "Dcaron" },               /* LATIN CAPITAL LETTER D WITH CARON */
    { 0x0110, 1, "Dcroat" },               /* LATIN CAPITAL LETTER D WITH STROKE */
    { 0x2206, 2, "Delta" },                /* INCREMENT & GREEK CAPITAL LETTER DELTA (Double mapping) */
    { 0x0394, 2, "Delta" },
    { 0x0045, 1, "E" },                    /* LATIN CAPITAL LETTER E */
    { 0x00c9, 1, "Eacute" },               /* LATIN CAPITAL LETTER E WITH ACUTE */
    { 0x0114, 1, "Ebreve" },               /* LATIN CAPITAL LETTER E WITH BREVE */
    { 0x011a, 1, "Ecaron" },               /* LATIN CAPITAL LETTER E WITH CARON */
    { 0x00ca, 1, "Ecircumflex" },          /* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
    { 0x00cb, 1, "Edieresis" },            /* LATIN CAPITAL LETTER E WITH DIAERESIS */
    { 0x0116, 1, "Edot" },                 /* LATIN CAPITAL LETTER E WITH DOT ABOVE (Non-Adobe) */
    { 0x0116, 1, "Edotaccent" },           /* LATIN CAPITAL LETTER E WITH DOT ABOVE  */
    { 0x00c8, 1, "Egrave" },               /* LATIN CAPITAL LETTER E WITH GRAVE */
    { 0x0112, 1, "Emacron" },              /* LATIN CAPITAL LETTER E WITH MACRON */
    { 0x014a, 1, "Eng" },                  /* LATIN CAPITAL LETTER ENG */
    { 0x0118, 1, "Eogonek" },              /* LATIN CAPITAL LETTER E WITH OGONEK */
    { 0x0395, 1, "Epsilon" },              /* GREEK CAPITAL LETTER EPSILON */
    { 0x0388, 1, "Epsilontonos" },         /* GREEK CAPITAL LETTER EPSILON WITH TONOS */
    { 0x0397, 1, "Eta" },                  /* GREEK CAPITAL LETTER ETA */
    { 0x0389, 1, "Etatonos" },             /* GREEK CAPITAL LETTER ETA WITH TONOS */
    { 0x00d0, 1, "Eth" },                  /* LATIN CAPITAL LETTER ETH */
    { 0x20ac, 1, "Euro" },                 /* EURO SIGN */
    { 0x0046, 1, "F" },                    /* LATIN CAPITAL LETTER F */
    { 0x0047, 1, "G" },                    /* LATIN CAPITAL LETTER G */
    { 0x0393, 1, "Gamma" },                /* GREEK CAPITAL LETTER GAMMA */
    { 0x011e, 1, "Gbreve" },               /* LATIN CAPITAL LETTER G WITH BREVE */
    { 0x01e6, 1, "Gcaron" },               /* LATIN CAPITAL LETTER G WITH CARON */
    { 0x0122, 1, "Gcedilla" },             /* LATIN CAPITAL LETTER G WITH CEDILLA (Non-Adobe) */
    { 0x011c, 1, "Gcircumflex" },          /* LATIN CAPITAL LETTER G WITH CIRCUMFLEX */
    { 0x0122, 1, "Gcommaaccent" },         /* LATIN CAPITAL LETTER G WITH CEDILLA */
    { 0x0120, 1, "Gdot" },                 /* LATIN CAPITAL LETTER G WITH DOT ABOVE (Non-Adobe) */
    { 0x0120, 1, "Gdotaccent" },           /* LATIN CAPITAL LETTER G WITH DOT ABOVE  */
    { 0x0048, 1, "H" },                    /* LATIN CAPITAL LETTER H */
    { 0x25cf, 1, "H18533" },               /* BLACK CIRCLE */
    { 0x25aa, 1, "H18543" },               /* BLACK SMALL SQUARE */
    { 0x25ab, 1, "H18551" },               /* WHITE SMALL SQUARE */
    { 0x25a1, 1, "H22073" },               /* WHITE SQUARE */
    { 0x0126, 1, "Hbar" },                 /* LATIN CAPITAL LETTER H WITH STROKE */
    { 0x0124, 1, "Hcircumflex" },          /* LATIN CAPITAL LETTER H WITH CIRCUMFLEX */
    { 0x0049, 1, "I" },                    /* LATIN CAPITAL LETTER I */
    { 0x0132, 1, "IJ" },                   /* LATIN CAPITAL LIGATURE IJ */
    { 0x00cd, 1, "Iacute" },               /* LATIN CAPITAL LETTER I WITH ACUTE */
    { 0x012c, 1, "Ibreve" },               /* LATIN CAPITAL LETTER I WITH BREVE */
    { 0x00ce, 1, "Icircumflex" },          /* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
    { 0x00cf, 1, "Idieresis" },            /* LATIN CAPITAL LETTER I WITH DIAERESIS */
    { 0x0130, 1, "Idot" },                 /* LATIN CAPITAL LETTER I WITH DOT ABOVE (Non-Adobe) */
    { 0x0130, 1, "Idotaccent" },           /* LATIN CAPITAL LETTER I WITH DOT ABOVE */
    { 0x2111, 1, "Ifraktur" },             /* BLACK-LETTER CAPITAL I */
    { 0x00cc, 1, "Igrave" },               /* LATIN CAPITAL LETTER I WITH GRAVE */
    { 0x012a, 1, "Imacron" },              /* LATIN CAPITAL LETTER I WITH MACRON */
    { 0x012e, 1, "Iogonek" },              /* LATIN CAPITAL LETTER I WITH OGONEK */
    { 0x0399, 1, "Iota" },                 /* GREEK CAPITAL LETTER IOTA */
    { 0x03aa, 1, "Iotadieresis" },         /* GREEK CAPITAL LETTER IOTA WITH DIALYTIKA */
    { 0x038a, 1, "Iotatonos" },            /* GREEK CAPITAL LETTER IOTA WITH TONOS */
    { 0x0128, 1, "Itilde" },               /* LATIN CAPITAL LETTER I WITH TILDE */
    { 0x004a, 1, "J" },                    /* LATIN CAPITAL LETTER J */
    { 0x0134, 1, "Jcircumflex" },          /* LATIN CAPITAL LETTER J WITH CIRCUMFLEX */
    { 0x004b, 1, "K" },                    /* LATIN CAPITAL LETTER K */
    { 0x039a, 1, "Kappa" },                /* GREEK CAPITAL LETTER KAPPA */
    { 0x0136, 1, "Kcedilla" },             /* LATIN CAPITAL LETTER K WITH CEDILLA (Non-Adobe) */
    { 0x0136, 1, "Kcommaaccent" },         /* LATIN CAPITAL LETTER K WITH CEDILLA */
    { 0x004c, 1, "L" },                    /* LATIN CAPITAL LETTER L */
    { 0x0139, 1, "Lacute" },               /* LATIN CAPITAL LETTER L WITH ACUTE */
    { 0x039b, 1, "Lambda" },               /* GREEK CAPITAL LETTER LAMDA */
    { 0x013d, 1, "Lcaron" },               /* LATIN CAPITAL LETTER L WITH CARON */
    { 0x013b, 1, "Lcedilla" },             /* LATIN CAPITAL LETTER L WITH CEDILLA (Non-Adobe) */
    { 0x013b, 1, "Lcommaaccent" },         /* LATIN CAPITAL LETTER L WITH CEDILLA */
    { 0x013f, 1, "Ldot" },                 /* LATIN CAPITAL LETTER L WITH MIDDLE DOT */
    { 0x0141, 1, "Lslash" },               /* LATIN CAPITAL LETTER L WITH STROKE */
    { 0x004d, 1, "M" },                    /* LATIN CAPITAL LETTER M */
    { 0x039c, 1, "Mu" },                   /* GREEK CAPITAL LETTER MU */
    { 0x004e, 1, "N" },                    /* LATIN CAPITAL LETTER N */
    { 0x0143, 1, "Nacute" },               /* LATIN CAPITAL LETTER N WITH ACUTE */
    { 0x0147, 1, "Ncaron" },               /* LATIN CAPITAL LETTER N WITH CARON */
    { 0x0145, 1, "Ncedilla" },             /* LATIN CAPITAL LETTER N WITH CEDILLA (Non-Adobe) */
    { 0x0145, 1, "Ncommaaccent" },         /* LATIN CAPITAL LETTER N WITH CEDILLA */
    { 0x00d1, 1, "Ntilde" },               /* LATIN CAPITAL LETTER N WITH TILDE */
    { 0x039d, 1, "Nu" },                   /* GREEK CAPITAL LETTER NU */
    { 0x004f, 1, "O" },                    /* LATIN CAPITAL LETTER O */
    { 0x0152, 1, "OE" },                   /* LATIN CAPITAL LIGATURE OE */
    { 0x00d3, 1, "Oacute" },               /* LATIN CAPITAL LETTER O WITH ACUTE */
    { 0x014e, 1, "Obreve" },               /* LATIN CAPITAL LETTER O WITH BREVE */
    { 0x00d4, 1, "Ocircumflex" },          /* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
    { 0x0150, 1, "Odblacute" },            /* LATIN CAPITAL LETTER O WITH DOUBLE ACUTE (Non-Adobe) */
    { 0x00d6, 1, "Odieresis" },            /* LATIN CAPITAL LETTER O WITH DIAERESIS */
    { 0x00d2, 1, "Ograve" },               /* LATIN CAPITAL LETTER O WITH GRAVE */
    { 0x01a0, 1, "Ohorn" },                /* LATIN CAPITAL LETTER O WITH HORN */
    { 0x0150, 1, "Ohungarumlaut" },        /* LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */
    { 0x014c, 1, "Omacron" },              /* LATIN CAPITAL LETTER O WITH MACRON */
    { 0x2126, 2, "Omega" },                /* OHM SIGN & GREEK CAPITAL LETTER OMEGA (Double mapping) */
    { 0x03a9, 2, "Omega" },
    { 0x038f, 1, "Omegatonos" },           /* GREEK CAPITAL LETTER OMEGA WITH TONOS */
    { 0x039f, 1, "Omicron" },              /* GREEK CAPITAL LETTER OMICRON */
    { 0x038c, 1, "Omicrontonos" },         /* GREEK CAPITAL LETTER OMICRON WITH TONOS */
    { 0x00d8, 1, "Oslash" },               /* LATIN CAPITAL LETTER O WITH STROKE */
    { 0x01fe, 1, "Oslashacute" },          /* LATIN CAPITAL LETTER O WITH STROKE AND ACUTE */
    { 0x00d5, 1, "Otilde" },               /* LATIN CAPITAL LETTER O WITH TILDE */
    { 0x0050, 1, "P" },                    /* LATIN CAPITAL LETTER P */
    { 0x03a6, 1, "Phi" },                  /* GREEK CAPITAL LETTER PHI */
    { 0x03a0, 1, "Pi" },                   /* GREEK CAPITAL LETTER PI */
    { 0x03a8, 1, "Psi" },                  /* GREEK CAPITAL LETTER PSI */
    { 0x0051, 1, "Q" },                    /* LATIN CAPITAL LETTER Q */
    { 0x0052, 1, "R" },                    /* LATIN CAPITAL LETTER R */
    { 0x0154, 1, "Racute" },               /* LATIN CAPITAL LETTER R WITH ACUTE */
    { 0x0158, 1, "Rcaron" },               /* LATIN CAPITAL LETTER R WITH CARON */
    { 0x0156, 1, "Rcedilla" },             /* LATIN CAPITAL LETTER R WITH CEDILLA (Non-Adobe) */
    { 0x0156, 1, "Rcommaaccent" },         /* LATIN CAPITAL LETTER R WITH CEDILLA */
    { 0x211c, 1, "Rfraktur" },             /* BLACK-LETTER CAPITAL R */
    { 0x03a1, 1, "Rho" },                  /* GREEK CAPITAL LETTER RHO */
    { 0x0053, 1, "S" },                    /* LATIN CAPITAL LETTER S */
    { 0x250c, 1, "SF010000" },             /* BOX DRAWINGS LIGHT DOWN AND RIGHT */
    { 0x2514, 1, "SF020000" },             /* BOX DRAWINGS LIGHT UP AND RIGHT */
    { 0x2510, 1, "SF030000" },             /* BOX DRAWINGS LIGHT DOWN AND LEFT */
    { 0x2518, 1, "SF040000" },             /* BOX DRAWINGS LIGHT UP AND LEFT */
    { 0x253c, 1, "SF050000" },             /* BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
    { 0x252c, 1, "SF060000" },             /* BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
    { 0x2534, 1, "SF070000" },             /* BOX DRAWINGS LIGHT UP AND HORIZONTAL */
    { 0x251c, 1, "SF080000" },             /* BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
    { 0x2524, 1, "SF090000" },             /* BOX DRAWINGS LIGHT VERTICAL AND LEFT */
    { 0x2500, 1, "SF100000" },             /* BOX DRAWINGS LIGHT HORIZONTAL */
    { 0x2502, 1, "SF110000" },             /* BOX DRAWINGS LIGHT VERTICAL */
    { 0x2561, 1, "SF190000" },             /* BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE */
    { 0x2562, 1, "SF200000" },             /* BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE */
    { 0x2556, 1, "SF210000" },             /* BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE */
    { 0x2555, 1, "SF220000" },             /* BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE */
    { 0x2563, 1, "SF230000" },             /* BOX DRAWINGS DOUBLE VERTICAL AND LEFT */
    { 0x2551, 1, "SF240000" },             /* BOX DRAWINGS DOUBLE VERTICAL */
    { 0x2557, 1, "SF250000" },             /* BOX DRAWINGS DOUBLE DOWN AND LEFT */
    { 0x255d, 1, "SF260000" },             /* BOX DRAWINGS DOUBLE UP AND LEFT */
    { 0x255c, 1, "SF270000" },             /* BOX DRAWINGS UP DOUBLE AND LEFT SINGLE */
    { 0x255b, 1, "SF280000" },             /* BOX DRAWINGS UP SINGLE AND LEFT DOUBLE */
    { 0x255e, 1, "SF360000" },             /* BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE */
    { 0x255f, 1, "SF370000" },             /* BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE */
    { 0x255a, 1, "SF380000" },             /* BOX DRAWINGS DOUBLE UP AND RIGHT */
    { 0x2554, 1, "SF390000" },             /* BOX DRAWINGS DOUBLE DOWN AND RIGHT */
    { 0x2569, 1, "SF400000" },             /* BOX DRAWINGS DOUBLE UP AND HORIZONTAL */
    { 0x2566, 1, "SF410000" },             /* BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL */
    { 0x2560, 1, "SF420000" },             /* BOX DRAWINGS DOUBLE VERTICAL AND RIGHT */
    { 0x2550, 1, "SF430000" },             /* BOX DRAWINGS DOUBLE HORIZONTAL */
    { 0x256c, 1, "SF440000" },             /* BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL */
    { 0x2567, 1, "SF450000" },             /* BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE */
    { 0x2568, 1, "SF460000" },             /* BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE */
    { 0x2564, 1, "SF470000" },             /* BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE */
    { 0x2565, 1, "SF480000" },             /* BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE */
    { 0x2559, 1, "SF490000" },             /* BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE */
    { 0x2558, 1, "SF500000" },             /* BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE */
    { 0x2552, 1, "SF510000" },             /* BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE */
    { 0x2553, 1, "SF520000" },             /* BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE */
    { 0x256b, 1, "SF530000" },             /* BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SINGLE */
    { 0x256a, 1, "SF540000" },             /* BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE */
    { 0x015a, 1, "Sacute" },               /* LATIN CAPITAL LETTER S WITH ACUTE */
    { 0x0160, 1, "Scaron" },               /* LATIN CAPITAL LETTER S WITH CARON */
    { 0x015e, 1, "Scedilla" },             /* LATIN CAPITAL LETTER S WITH CEDILLA */
    { 0x015c, 1, "Scircumflex" },          /* LATIN CAPITAL LETTER S WITH CIRCUMFLEX */
    { 0x0218, 1, "Scommaaccent" },         /* LATIN CAPITAL LETTER S WITH COMMA BELOW
                                              NOTE: Adobe maps this character to 0x0218, but
                                              some non-Adobe fonts actually maps this name to
                                              glyph that actually should have unicode value
                                              0x015e (EX: LucidaSansLat2).
                                              Since there is no way to check this,
                                              this mapping is left as Adobe made. */
    { 0x03a3, 1, "Sigma" },                /* GREEK CAPITAL LETTER SIGMA */
    { 0x0054, 1, "T" },                    /* LATIN CAPITAL LETTER T */
    { 0x03a4, 1, "Tau" },                  /* GREEK CAPITAL LETTER TAU */
    { 0x0166, 1, "Tbar" },                 /* LATIN CAPITAL LETTER T WITH STROKE */
    { 0x0164, 1, "Tcaron" },               /* LATIN CAPITAL LETTER T WITH CARON */
    { 0x0162, 1, "Tcedilla" },             /* LATIN CAPITAL LETTER T WITH CEDILLA (Non-Adobe) */
    { 0x021a, 1, "Tcedilla1" },            /* LATIN CAPITAL LETTER T WITH COMMA BELOW (Non-Adobe) */
    { 0x0162, 2, "Tcommaaccent" },         /* LATIN CAPITAL LETTER T WITH CEDILLA &
                                              LATIN CAPITAL LETTER T WITH COMMA BELOW
                                              (Adobe double mapping) */
    { 0x021a, 2, "Tcommaaccent" },
    { 0x0398, 1, "Theta" },                /* GREEK CAPITAL LETTER THETA */
    { 0x00de, 1, "Thorn" },                /* LATIN CAPITAL LETTER THORN */
    { 0x0055, 1, "U" },                    /* LATIN CAPITAL LETTER U */
    { 0x00da, 1, "Uacute" },               /* LATIN CAPITAL LETTER U WITH ACUTE */
    { 0x016c, 1, "Ubreve" },               /* LATIN CAPITAL LETTER U WITH BREVE */
    { 0x00db, 1, "Ucircumflex" },          /* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
    { 0x0170, 1, "Udblacute" },            /* LATIN CAPITAL LETTER U WITH DOUBLE ACUTE (Non-Adobe) */
    { 0x00dc, 1, "Udieresis" },            /* LATIN CAPITAL LETTER U WITH DIAERESIS */
    { 0x00d9, 1, "Ugrave" },               /* LATIN CAPITAL LETTER U WITH GRAVE */
    { 0x01af, 1, "Uhorn" },                /* LATIN CAPITAL LETTER U WITH HORN */
    { 0x0170, 1, "Uhungarumlaut" },        /* LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */
    { 0x016a, 1, "Umacron" },              /* LATIN CAPITAL LETTER U WITH MACRON */
    { 0x0172, 1, "Uogonek" },              /* LATIN CAPITAL LETTER U WITH OGONEK */
    { 0x03a5, 1, "Upsilon" },              /* GREEK CAPITAL LETTER UPSILON */
    { 0x03d2, 1, "Upsilon1" },             /* GREEK UPSILON WITH HOOK SYMBOL */
    { 0x03ab, 1, "Upsilondieresis" },      /* GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA */
    { 0x038e, 1, "Upsilontonos" },         /* GREEK CAPITAL LETTER UPSILON WITH TONOS */
    { 0x016e, 1, "Uring" },                /* LATIN CAPITAL LETTER U WITH RING ABOVE */
    { 0x0168, 1, "Utilde" },               /* LATIN CAPITAL LETTER U WITH TILDE */
    { 0x0056, 1, "V" },                    /* LATIN CAPITAL LETTER V */
    { 0x0057, 1, "W" },                    /* LATIN CAPITAL LETTER W */
    { 0x1e82, 1, "Wacute" },               /* LATIN CAPITAL LETTER W WITH ACUTE */
    { 0x0174, 1, "Wcircumflex" },          /* LATIN CAPITAL LETTER W WITH CIRCUMFLEX */
    { 0x1e84, 1, "Wdieresis" },            /* LATIN CAPITAL LETTER W WITH DIAERESIS */
    { 0x1e80, 1, "Wgrave" },               /* LATIN CAPITAL LETTER W WITH GRAVE */
    { 0x0058, 1, "X" },                    /* LATIN CAPITAL LETTER X */
    { 0x039e, 1, "Xi" },                   /* GREEK CAPITAL LETTER XI */
    { 0x0059, 1, "Y" },                    /* LATIN CAPITAL LETTER Y */
    { 0x00dd, 1, "Yacute" },               /* LATIN CAPITAL LETTER Y WITH ACUTE */
    { 0x0176, 1, "Ycircumflex" },          /* LATIN CAPITAL LETTER Y WITH CIRCUMFLEX */
    { 0x0178, 1, "Ydieresis" },            /* LATIN CAPITAL LETTER Y WITH DIAERESIS */
    { 0x1ef2, 1, "Ygrave" },               /* LATIN CAPITAL LETTER Y WITH GRAVE */
    { 0x005a, 1, "Z" },                    /* LATIN CAPITAL LETTER Z */
    { 0x0179, 1, "Zacute" },               /* LATIN CAPITAL LETTER Z WITH ACUTE */
    { 0x017d, 1, "Zcaron" },               /* LATIN CAPITAL LETTER Z WITH CARON */
    { 0x017b, 1, "Zdot" },                 /* LATIN CAPITAL LETTER Z WITH DOT ABOVE (Non-Adobe) */
    { 0x017b, 1, "Zdotaccent" },           /* LATIN CAPITAL LETTER Z WITH DOT ABOVE */
    { 0x0396, 1, "Zeta" },                 /* GREEK CAPITAL LETTER ZETA */
    { 0x0061, 1, "a" },                    /* LATIN SMALL LETTER A */
    { 0x2701, 1, "a1" },                   /* UPPER BLADE SCISSORS */
    { 0x2702, 1, "a2" },                   /* BLACK SCISSORS */
    { 0x2703, 1, "a202" },                 /* LOWER BLADE SCISSORS */
    { 0x2704, 1, "a3" },                   /* WHITE SCISSORS */
    { 0x260e, 1, "a4" },                   /* BLACK TELEPHONE */
    { 0x2706, 1, "a5" },                   /* TELEPHONE LOCATION SIGN */
    { 0x2707, 1, "a119" },                 /* TAPE DRIVE */
    { 0x2708, 1, "a118" },                 /* AIRPLANE */
    { 0x2709, 1, "a117" },                 /* ENVELOPE */
    { 0x261b, 1, "a11" },                  /* BLACK RIGHT POINTING INDEX */
    { 0x261e, 1, "a12" },                  /* WHITE RIGHT POINTING INDEX */
    { 0x270c, 1, "a13" },                  /* VICTORY HAND */
    { 0x270d, 1, "a14" },                  /* WRITING HAND */
    { 0x270e, 1, "a15" },                  /* LOWER RIGHT PENCIL */
    { 0x270f, 1, "a16" },                  /* PENCIL */
    { 0x2710, 1, "a105" },                 /* UPPER RIGHT PENCIL */
    { 0x2711, 1, "a17" },                  /* WHITE NIB */
    { 0x2712, 1, "a18" },                  /* BLACK NIB */
    { 0x2713, 1, "a19" },                  /* CHECK MARK */
    { 0x2714, 1, "a20" },                  /* HEAVY CHECK MARK */
    { 0x2715, 1, "a21" },                  /* MULTIPLICATION X */
    { 0x2716, 1, "a22" },                  /* HEAVY MULTIPLICATION X */
    { 0x2717, 1, "a23" },                  /* BALLOT X */
    { 0x2718, 1, "a24" },                  /* HEAVY BALLOT X */
    { 0x2719, 1, "a25" },                  /* OUTLINED GREEK CROSS */
    { 0x271a, 1, "a26" },                  /* HEAVY GREEK CROSS */
    { 0x271b, 1, "a27" },                  /* OPEN CENTRE CROSS */
    { 0x271c, 1, "a28" },                  /* HEAVY OPEN CENTRE CROSS */
    { 0x271d, 1, "a6" },                   /* LATIN CROSS */
    { 0x271e, 1, "a7" },                   /* SHADOWED WHITE LATIN CROSS */
    { 0x271f, 1, "a8" },                   /* OUTLINED LATIN CROSS */
    { 0x2720, 1, "a9" },                   /* MALTESE CROSS */
    { 0x2721, 1, "a10" },                  /* STAR OF DAVID */
    { 0x2722, 1, "a29" },                  /* FOUR TEARDROP-SPOKED ASTERISK */
    { 0x2723, 1, "a30" },                  /* FOUR BALLOON-SPOKED ASTERISK */
    { 0x2724, 1, "a31" },                  /* HEAVY FOUR BALLOON-SPOKED ASTERISK */
    { 0x2725, 1, "a32" },                  /* FOUR CLUB-SPOKED ASTERISK */
    { 0x2726, 1, "a33" },                  /* BLACK FOUR POINTED STAR */
    { 0x2727, 1, "a34" },                  /* WHITE FOUR POINTED STAR */
    { 0x2605, 1, "a35" },                  /* BLACK STAR */
    { 0x2729, 1, "a36" },                  /* STRESS OUTLINED WHITE STAR */
    { 0x272a, 1, "a37" },                  /* CIRCLED WHITE STAR */
    { 0x272b, 1, "a38" },                  /* OPEN CENTRE BLACK STAR */
    { 0x272c, 1, "a39" },                  /* BLACK CENTRE WHITE STAR */
    { 0x272d, 1, "a40" },                  /* OUTLINED BLACK STAR */
    { 0x272e, 1, "a41" },                  /* HEAVY OUTLINED BLACK STAR */
    { 0x272f, 1, "a42" },                  /* PINWHEEL STAR */
    { 0x2730, 1, "a43" },                  /* SHADOWED WHITE STAR */
    { 0x2731, 1, "a44" },                  /* HEAVY ASTERISK */
    { 0x2732, 1, "a45" },                  /* OPEN CENTRE ASTERISK */
    { 0x2733, 1, "a46" },                  /* EIGHT SPOKED ASTERISK */
    { 0x2734, 1, "a47" },                  /* EIGHT POINTED BLACK STAR */
    { 0x2735, 1, "a48" },                  /* EIGHT POINTED PINWHEEL STAR */
    { 0x2736, 1, "a49" },                  /* SIX POINTED BLACK STAR */
    { 0x2737, 1, "a50" },                  /* EIGHT POINTED RECTILINEAR BLACK STAR */
    { 0x2738, 1, "a51" },                  /* HEAVY EIGHT POINTED RECTILINEAR BLACK STAR */
    { 0x2739, 1, "a52" },                  /* TWELVE POINTED BLACK STAR */
    { 0x273a, 1, "a53" },                  /* SIXTEEN POINTED ASTERISK */
    { 0x273b, 1, "a54" },                  /* TEARDROP-SPOKED ASTERISK */
    { 0x273c, 1, "a55" },                  /* OPEN CENTRE TEARDROP-SPOKED ASTERISK */
    { 0x273d, 1, "a56" },                  /* HEAVY TEARDROP-SPOKED ASTERISK */
    { 0x273e, 1, "a57" },                  /* SIX PETALLED BLACK AND WHITE FLORETTE */
    { 0x273f, 1, "a58" },                  /* BLACK FLORETTE */
    { 0x2740, 1, "a59" },                  /* WHITE FLORETTE */
    { 0x2741, 1, "a60" },                  /* EIGHT PETALLED OUTLINED BLACK FLORETTE */
    { 0x2742, 1, "a61" },                  /* CIRCLED OPEN CENTRE EIGHT POINTED STAR */
    { 0x2743, 1, "a62" },                  /* HEAVY TEARDROP-SPOKED PINWHEEL ASTERISK */
    { 0x2744, 1, "a63" },                  /* SNOWFLAKE */
    { 0x2745, 1, "a64" },                  /* TIGHT TRIFOLIATE SNOWFLAKE */
    { 0x2746, 1, "a65" },                  /* HEAVY CHEVRON SNOWFLAKE */
    { 0x2747, 1, "a66" },                  /* SPARKLE */
    { 0x2748, 1, "a67" },                  /* HEAVY SPARKLE */
    { 0x2749, 1, "a68" },                  /* BALLOON-SPOKED ASTERISK */
    { 0x274a, 1, "a69" },                  /* EIGHT TEARDROP-SPOKED PROPELLER ASTERISK */
    { 0x274b, 1, "a70" },                  /* HEAVY EIGHT TEARDROP-SPOKED PROPELLER ASTERISK */
    { 0x25cf, 1, "a71" },                  /* BLACK CIRCLE */
    { 0x274d, 1, "a72" },                  /* SHADOWED WHITE CIRCLE */
    { 0x25a0, 1, "a73" },                  /* BLACK SQUARE */
    { 0x274f, 1, "a74" },                  /* LOWER RIGHT DROP-SHADOWED WHITE SQUARE */
    { 0x2750, 1, "a203" },                 /* UPPER RIGHT DROP-SHADOWED WHITE SQUARE */
    { 0x2751, 1, "a75" },                  /* LOWER RIGHT SHADOWED WHITE SQUARE */
    { 0x2752, 1, "a204" },                 /* UPPER RIGHT SHADOWED WHITE SQUARE */
    { 0x25b2, 1, "a76" },                  /* BLACK UP-POINTING TRIANGLE */
    { 0x25bc, 1, "a77" },                  /* BLACK DOWN-POINTING TRIANGLE */
    { 0x25c6, 1, "a78" },                  /* BLACK DIAMOND */
    { 0x2756, 1, "a79" },                  /* BLACK DIAMOND MINUS WHITE X */
    { 0x25d7, 1, "a81" },                  /* RIGHT HALF BLACK CIRCLE */
    { 0x2758, 1, "a82" },                  /* LIGHT VERTICAL BAR */
    { 0x2759, 1, "a83" },                  /* MEDIUM VERTICAL BAR */
    { 0x275a, 1, "a84" },                  /* HEAVY VERTICAL BAR */
    { 0x275b, 1, "a97" },                  /* HEAVY SINGLE TURNED COMMA QUOTATION MARK ORNAMENT */
    { 0x275c, 1, "a98" },                  /* HEAVY SINGLE COMMA QUOTATION MARK ORNAMENT */
    { 0x275d, 1, "a99" },                  /* HEAVY DOUBLE TURNED COMMA QUOTATION MARK ORNAMENT */
    { 0x275e, 1, "a100" },                 /* HEAVY DOUBLE COMMA QUOTATION MARK ORNAMENT */
    { 0x2761, 1, "a101" },                 /* CURVED STEM PARAGRAPH SIGN ORNAMENT */
    { 0x2762, 1, "a102" },                 /* HEAVY EXCLAMATION MARK ORNAMENT */
    { 0x2763, 1, "a103" },                 /* HEAVY HEART EXCLAMATION MARK ORNAMENT */
    { 0x2764, 1, "a104" },                 /* HEAVY BLACK HEART */
    { 0x2765, 1, "a106" },                 /* ROTATED HEAVY BLACK HEART BULLET */
    { 0x2766, 1, "a107" },                 /* FLORAL HEART */
    { 0x2767, 1, "a108" },                 /* ROTATED FLORAL HEART BULLET */
    { 0x2663, 1, "a112" },                 /* BLACK CLUB SUIT */
    { 0x2666, 1, "a111" },                 /* BLACK DIAMOND SUIT */
    { 0x2665, 1, "a110" },                 /* BLACK HEART SUIT */
    { 0x2660, 1, "a109" },                 /* BLACK SPADE SUIT */
    { 0x2460, 1, "a120" },                 /* CIRCLED DIGIT ONE */
    { 0x2461, 1, "a121" },                 /* CIRCLED DIGIT TWO */
    { 0x2462, 1, "a122" },                 /* CIRCLED DIGIT THREE */
    { 0x2463, 1, "a123" },                 /* CIRCLED DIGIT FOUR */
    { 0x2464, 1, "a124" },                 /* CIRCLED DIGIT FIVE */
    { 0x2465, 1, "a125" },                 /* CIRCLED DIGIT SIX */
    { 0x2466, 1, "a126" },                 /* CIRCLED DIGIT SEVEN */
    { 0x2467, 1, "a127" },                 /* CIRCLED DIGIT EIGHT */
    { 0x2468, 1, "a128" },                 /* CIRCLED DIGIT NINE */
    { 0x2469, 1, "a129" },                 /* CIRCLED NUMBER TEN */
    { 0x2776, 1, "a130" },                 /* DINGBAT NEGATIVE CIRCLED DIGIT ONE */
    { 0x2777, 1, "a131" },                 /* DINGBAT NEGATIVE CIRCLED DIGIT TWO */
    { 0x2778, 1, "a132" },                 /* DINGBAT NEGATIVE CIRCLED DIGIT THREE */
    { 0x2779, 1, "a133" },                 /* DINGBAT NEGATIVE CIRCLED DIGIT FOUR */
    { 0x277a, 1, "a134" },                 /* DINGBAT NEGATIVE CIRCLED DIGIT FIVE */
    { 0x277b, 1, "a135" },                 /* DINGBAT NEGATIVE CIRCLED DIGIT SIX */
    { 0x277c, 1, "a136" },                 /* DINGBAT NEGATIVE CIRCLED DIGIT SEVEN */
    { 0x277d, 1, "a137" },                 /* DINGBAT NEGATIVE CIRCLED DIGIT EIGHT */
    { 0x277e, 1, "a138" },                 /* DINGBAT NEGATIVE CIRCLED DIGIT NINE */
    { 0x277f, 1, "a139" },                 /* DINGBAT NEGATIVE CIRCLED NUMBER TEN */
    { 0x2780, 1, "a140" },                 /* DINGBAT CIRCLED SANS-SERIF DIGIT ONE */
    { 0x2781, 1, "a141" },                 /* DINGBAT CIRCLED SANS-SERIF DIGIT TWO */
    { 0x2782, 1, "a142" },                 /* DINGBAT CIRCLED SANS-SERIF DIGIT THREE */
    { 0x2783, 1, "a143" },                 /* DINGBAT CIRCLED SANS-SERIF DIGIT FOUR */
    { 0x2784, 1, "a144" },                 /* DINGBAT CIRCLED SANS-SERIF DIGIT FIVE */
    { 0x2785, 1, "a145" },                 /* DINGBAT CIRCLED SANS-SERIF DIGIT SIX */
    { 0x2786, 1, "a146" },                 /* DINGBAT CIRCLED SANS-SERIF DIGIT SEVEN */
    { 0x2787, 1, "a147" },                 /* DINGBAT CIRCLED SANS-SERIF DIGIT EIGHT */
    { 0x2788, 1, "a148" },                 /* DINGBAT CIRCLED SANS-SERIF DIGIT NINE */
    { 0x2789, 1, "a149" },                 /* DINGBAT CIRCLED SANS-SERIF NUMBER TEN */
    { 0x278a, 1, "a150" },                 /* DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT ONE */
    { 0x278b, 1, "a151" },                 /* DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT TWO */
    { 0x278c, 1, "a152" },                 /* DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT THREE */
    { 0x278d, 1, "a153" },                 /* DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT FOUR */
    { 0x278e, 1, "a154" },                 /* DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT FIVE */
    { 0x278f, 1, "a155" },                 /* DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT SIX */
    { 0x2790, 1, "a156" },                 /* DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT SEVEN */
    { 0x2791, 1, "a157" },                 /* DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT EIGHT */
    { 0x2792, 1, "a158" },                 /* DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT NINE */
    { 0x2793, 1, "a159" },                 /* DINGBAT NEGATIVE CIRCLED SANS-SERIF NUMBER TEN */
    { 0x2794, 1, "a160" },                 /* HEAVY WIDE-HEADED RIGHTWARDS ARROW */
    { 0x2192, 1, "a161" },                 /* RIGHTWARDS ARROW */
    { 0x2194, 1, "a163" },                 /* LEFT RIGHT ARROW */
    { 0x2195, 1, "a164" },                 /* UP DOWN ARROW */
    { 0x2798, 1, "a196" },                 /* HEAVY SOUTH EAST ARROW */
    { 0x2799, 1, "a165" },                 /* HEAVY RIGHTWARDS ARROW */
    { 0x279a, 1, "a192" },                 /* HEAVY NORTH EAST ARROW */
    { 0x279b, 1, "a166" },                 /* DRAFTING POINT RIGHTWARDS ARROW */
    { 0x279c, 1, "a167" },                 /* HEAVY ROUND-TIPPED RIGHTWARDS ARROW */
    { 0x279d, 1, "a168" },                 /* TRIANGLE-HEADED RIGHTWARDS ARROW */
    { 0x279e, 1, "a169" },                 /* HEAVY TRIANGLE-HEADED RIGHTWARDS ARROW */
    { 0x279f, 1, "a170" },                 /* DASHED TRIANGLE-HEADED RIGHTWARDS ARROW */
    { 0x27a0, 1, "a171" },                 /* HEAVY DASHED TRIANGLE-HEADED RIGHTWARDS ARROW */
    { 0x27a1, 1, "a172" },                 /* BLACK RIGHTWARDS ARROW */
    { 0x27a2, 1, "a173" },                 /* THREE-D TOP-LIGHTED RIGHTWARDS ARROWHEAD */
    { 0x27a3, 1, "a162" },                 /* THREE-D BOTTOM-LIGHTED RIGHTWARDS ARROWHEAD */
    { 0x27a4, 1, "a174" },                 /* BLACK RIGHTWARDS ARROWHEAD */
    { 0x27a5, 1, "a175" },                 /* HEAVY BLACK CURVED DOWNWARDS AND RIGHTWARDS ARROW */
    { 0x27a6, 1, "a176" },                 /* HEAVY BLACK CURVED UPWARDS AND RIGHTWARDS ARROW */
    { 0x27a7, 1, "a177" },                 /* SQUAT BLACK RIGHTWARDS ARROW */
    { 0x27a8, 1, "a178" },                 /* HEAVY CONCAVE-POINTED BLACK RIGHTWARDS ARROW */
    { 0x27a9, 1, "a179" },                 /* RIGHT-SHADED WHITE RIGHTWARDS ARROW */
    { 0x27aa, 1, "a193" },                 /* LEFT-SHADED WHITE RIGHTWARDS ARROW */
    { 0x27ab, 1, "a180" },                 /* BACK-TILTED SHADOWED WHITE RIGHTWARDS ARROW */
    { 0x27ac, 1, "a199" },                 /* FRONT-TILTED SHADOWED WHITE RIGHTWARDS ARROW */
    { 0x27ad, 1, "a181" },                 /* HEAVY LOWER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW */
    { 0x27ae, 1, "a200" },                 /* HEAVY UPPER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW */
    { 0x27af, 1, "a182" },                 /* NOTCHED LOWER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW */
    { 0x27b1, 1, "a201" },                 /* NOTCHED UPPER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW */
    { 0x27b2, 1, "a183" },                 /* CIRCLED HEAVY WHITE RIGHTWARDS ARROW */
    { 0x27b3, 1, "a184" },                 /* WHITE-FEATHERED RIGHTWARDS ARROW */
    { 0x27b4, 1, "a197" },                 /* BLACK-FEATHERED SOUTH EAST ARROW */
    { 0x27b5, 1, "a185" },                 /* BLACK-FEATHERED RIGHTWARDS ARROW */
    { 0x27b6, 1, "a194" },                 /* BLACK-FEATHERED NORTH EAST ARROW */
    { 0x27b7, 1, "a198" },                 /* HEAVY BLACK-FEATHERED SOUTH EAST ARROW */
    { 0x27b8, 1, "a186" },                 /* HEAVY BLACK-FEATHERED RIGHTWARDS ARROW */
    { 0x27b9, 1, "a195" },                 /* HEAVY BLACK-FEATHERED NORTH EAST ARROW */
    { 0x27ba, 1, "a187" },                 /* TEARDROP-BARBED RIGHTWARDS ARROW */
    { 0x27bb, 1, "a188" },                 /* HEAVY TEARDROP-SHANKED RIGHTWARDS ARROW */
    { 0x27bc, 1, "a189" },                 /* WEDGE-TAILED RIGHTWARDS ARROW */
    { 0x27bd, 1, "a190" },                 /* HEAVY WEDGE-TAILED RIGHTWARDS ARROW */
    { 0x00e1, 1, "aacute" },               /* LATIN SMALL LETTER A WITH ACUTE */
    { 0x0103, 1, "abreve" },               /* LATIN SMALL LETTER A WITH BREVE */
    { 0x00e2, 1, "acircumflex" },          /* LATIN SMALL LETTER A WITH CIRCUMFLEX */
    { 0x00b4, 1, "acute" },                /* ACUTE ACCENT */
    { 0x0301, 1, "acutecomb" },            /* COMBINING ACUTE ACCENT */
    { 0x00e4, 1, "adieresis" },            /* LATIN SMALL LETTER A WITH DIAERESIS */
    { 0x00e6, 1, "ae" },                   /* LATIN SMALL LETTER AE */
    { 0x01fd, 1, "aeacute" },              /* LATIN SMALL LETTER AE WITH ACUTE */
    { 0x2015, 1, "afii00208" },            /* HORIZONTAL BAR */
    { 0x0410, 1, "afii10017" },            /* CYRILLIC CAPITAL LETTER A */
    { 0x0411, 1, "afii10018" },            /* CYRILLIC CAPITAL LETTER BE */
    { 0x0412, 1, "afii10019" },            /* CYRILLIC CAPITAL LETTER VE */
    { 0x0413, 1, "afii10020" },            /* CYRILLIC CAPITAL LETTER GHE */
    { 0x0414, 1, "afii10021" },            /* CYRILLIC CAPITAL LETTER DE */
    { 0x0415, 1, "afii10022" },            /* CYRILLIC CAPITAL LETTER IE */
    { 0x0401, 1, "afii10023" },            /* CYRILLIC CAPITAL LETTER IO */
    { 0x0416, 1, "afii10024" },            /* CYRILLIC CAPITAL LETTER ZHE */
    { 0x0417, 1, "afii10025" },            /* CYRILLIC CAPITAL LETTER ZE */
    { 0x0418, 1, "afii10026" },            /* CYRILLIC CAPITAL LETTER I */
    { 0x0419, 1, "afii10027" },            /* CYRILLIC CAPITAL LETTER SHORT I */
    { 0x041a, 1, "afii10028" },            /* CYRILLIC CAPITAL LETTER KA */
    { 0x041b, 1, "afii10029" },            /* CYRILLIC CAPITAL LETTER EL */
    { 0x041c, 1, "afii10030" },            /* CYRILLIC CAPITAL LETTER EM */
    { 0x041d, 1, "afii10031" },            /* CYRILLIC CAPITAL LETTER EN */
    { 0x041e, 1, "afii10032" },            /* CYRILLIC CAPITAL LETTER O */
    { 0x041f, 1, "afii10033" },            /* CYRILLIC CAPITAL LETTER PE */
    { 0x0420, 1, "afii10034" },            /* CYRILLIC CAPITAL LETTER ER */
    { 0x0421, 1, "afii10035" },            /* CYRILLIC CAPITAL LETTER ES */
    { 0x0422, 1, "afii10036" },            /* CYRILLIC CAPITAL LETTER TE */
    { 0x0423, 1, "afii10037" },            /* CYRILLIC CAPITAL LETTER U */
    { 0x0424, 1, "afii10038" },            /* CYRILLIC CAPITAL LETTER EF */
    { 0x0425, 1, "afii10039" },            /* CYRILLIC CAPITAL LETTER HA */
    { 0x0426, 1, "afii10040" },            /* CYRILLIC CAPITAL LETTER TSE */
    { 0x0427, 1, "afii10041" },            /* CYRILLIC CAPITAL LETTER CHE */
    { 0x0428, 1, "afii10042" },            /* CYRILLIC CAPITAL LETTER SHA */
    { 0x0429, 1, "afii10043" },            /* CYRILLIC CAPITAL LETTER SHCHA */
    { 0x042a, 1, "afii10044" },            /* CYRILLIC CAPITAL LETTER HARD SIGN */
    { 0x042b, 1, "afii10045" },            /* CYRILLIC CAPITAL LETTER YERU */
    { 0x042c, 1, "afii10046" },            /* CYRILLIC CAPITAL LETTER SOFT SIGN */
    { 0x042d, 1, "afii10047" },            /* CYRILLIC CAPITAL LETTER E */
    { 0x042e, 1, "afii10048" },            /* CYRILLIC CAPITAL LETTER YU */
    { 0x042f, 1, "afii10049" },            /* CYRILLIC CAPITAL LETTER YA */
    { 0x0490, 1, "afii10050" },            /* CYRILLIC CAPITAL LETTER GHE WITH UPTURN */
    { 0x0402, 1, "afii10051" },            /* CYRILLIC CAPITAL LETTER DJE */
    { 0x0403, 1, "afii10052" },            /* CYRILLIC CAPITAL LETTER GJE */
    { 0x0404, 1, "afii10053" },            /* CYRILLIC CAPITAL LETTER UKRAINIAN IE */
    { 0x0405, 1, "afii10054" },            /* CYRILLIC CAPITAL LETTER DZE */
    { 0x0406, 1, "afii10055" },            /* CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I */
    { 0x0407, 1, "afii10056" },            /* CYRILLIC CAPITAL LETTER YI */
    { 0x0408, 1, "afii10057" },            /* CYRILLIC CAPITAL LETTER JE */
    { 0x0409, 1, "afii10058" },            /* CYRILLIC CAPITAL LETTER LJE */
    { 0x040a, 1, "afii10059" },            /* CYRILLIC CAPITAL LETTER NJE */
    { 0x040b, 1, "afii10060" },            /* CYRILLIC CAPITAL LETTER TSHE */
    { 0x040c, 1, "afii10061" },            /* CYRILLIC CAPITAL LETTER KJE */
    { 0x040e, 1, "afii10062" },            /* CYRILLIC CAPITAL LETTER SHORT U */
    { 0x0430, 1, "afii10065" },            /* CYRILLIC SMALL LETTER A */
    { 0x0431, 1, "afii10066" },            /* CYRILLIC SMALL LETTER BE */
    { 0x0432, 1, "afii10067" },            /* CYRILLIC SMALL LETTER VE */
    { 0x0433, 1, "afii10068" },            /* CYRILLIC SMALL LETTER GHE */
    { 0x0434, 1, "afii10069" },            /* CYRILLIC SMALL LETTER DE */
    { 0x0435, 1, "afii10070" },            /* CYRILLIC SMALL LETTER IE */
    { 0x0451, 1, "afii10071" },            /* CYRILLIC SMALL LETTER IO */
    { 0x0436, 1, "afii10072" },            /* CYRILLIC SMALL LETTER ZHE */
    { 0x0437, 1, "afii10073" },            /* CYRILLIC SMALL LETTER ZE */
    { 0x0438, 1, "afii10074" },            /* CYRILLIC SMALL LETTER I */
    { 0x0439, 1, "afii10075" },            /* CYRILLIC SMALL LETTER SHORT I */
    { 0x043a, 1, "afii10076" },            /* CYRILLIC SMALL LETTER KA */
    { 0x043b, 1, "afii10077" },            /* CYRILLIC SMALL LETTER EL */
    { 0x043c, 1, "afii10078" },            /* CYRILLIC SMALL LETTER EM */
    { 0x043d, 1, "afii10079" },            /* CYRILLIC SMALL LETTER EN */
    { 0x043e, 1, "afii10080" },            /* CYRILLIC SMALL LETTER O */
    { 0x043f, 1, "afii10081" },            /* CYRILLIC SMALL LETTER PE */
    { 0x0440, 1, "afii10082" },            /* CYRILLIC SMALL LETTER ER */
    { 0x0441, 1, "afii10083" },            /* CYRILLIC SMALL LETTER ES */
    { 0x0442, 1, "afii10084" },            /* CYRILLIC SMALL LETTER TE */
    { 0x0443, 1, "afii10085" },            /* CYRILLIC SMALL LETTER U */
    { 0x0444, 1, "afii10086" },            /* CYRILLIC SMALL LETTER EF */
    { 0x0445, 1, "afii10087" },            /* CYRILLIC SMALL LETTER HA */
    { 0x0446, 1, "afii10088" },            /* CYRILLIC SMALL LETTER TSE */
    { 0x0447, 1, "afii10089" },            /* CYRILLIC SMALL LETTER CHE */
    { 0x0448, 1, "afii10090" },            /* CYRILLIC SMALL LETTER SHA */
    { 0x0449, 1, "afii10091" },            /* CYRILLIC SMALL LETTER SHCHA */
    { 0x044a, 1, "afii10092" },            /* CYRILLIC SMALL LETTER HARD SIGN */
    { 0x044b, 1, "afii10093" },            /* CYRILLIC SMALL LETTER YERU */
    { 0x044c, 1, "afii10094" },            /* CYRILLIC SMALL LETTER SOFT SIGN */
    { 0x044d, 1, "afii10095" },            /* CYRILLIC SMALL LETTER E */
    { 0x044e, 1, "afii10096" },            /* CYRILLIC SMALL LETTER YU */
    { 0x044f, 1, "afii10097" },            /* CYRILLIC SMALL LETTER YA */
    { 0x0491, 1, "afii10098" },            /* CYRILLIC SMALL LETTER GHE WITH UPTURN */
    { 0x0452, 1, "afii10099" },            /* CYRILLIC SMALL LETTER DJE */
    { 0x0453, 1, "afii10100" },            /* CYRILLIC SMALL LETTER GJE */
    { 0x0454, 1, "afii10101" },            /* CYRILLIC SMALL LETTER UKRAINIAN IE */
    { 0x0455, 1, "afii10102" },            /* CYRILLIC SMALL LETTER DZE */
    { 0x0456, 1, "afii10103" },            /* CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I */
    { 0x0457, 1, "afii10104" },            /* CYRILLIC SMALL LETTER YI */
    { 0x0458, 1, "afii10105" },            /* CYRILLIC SMALL LETTER JE */
    { 0x0459, 1, "afii10106" },            /* CYRILLIC SMALL LETTER LJE */
    { 0x045a, 1, "afii10107" },            /* CYRILLIC SMALL LETTER NJE */
    { 0x045b, 1, "afii10108" },            /* CYRILLIC SMALL LETTER TSHE */
    { 0x045c, 1, "afii10109" },            /* CYRILLIC SMALL LETTER KJE */
    { 0x045e, 1, "afii10110" },            /* CYRILLIC SMALL LETTER SHORT U */
    { 0x040f, 1, "afii10145" },            /* CYRILLIC CAPITAL LETTER DZHE */
    { 0x0462, 1, "afii10146" },            /* CYRILLIC CAPITAL LETTER YAT */
    { 0x0472, 1, "afii10147" },            /* CYRILLIC CAPITAL LETTER FITA */
    { 0x0474, 1, "afii10148" },            /* CYRILLIC CAPITAL LETTER IZHITSA */
    { 0x045f, 1, "afii10193" },            /* CYRILLIC SMALL LETTER DZHE */
    { 0x0463, 1, "afii10194" },            /* CYRILLIC SMALL LETTER YAT */
    { 0x0473, 1, "afii10195" },            /* CYRILLIC SMALL LETTER FITA */
    { 0x0475, 1, "afii10196" },            /* CYRILLIC SMALL LETTER IZHITSA */
    { 0x04d9, 1, "afii10846" },            /* CYRILLIC SMALL LETTER SCHWA */
    { 0x200e, 1, "afii299" },              /* LEFT-TO-RIGHT MARK */
    { 0x200f, 1, "afii300" },              /* RIGHT-TO-LEFT MARK */
    { 0x200d, 1, "afii301" },              /* ZERO WIDTH JOINER */
    { 0x066a, 1, "afii57381" },            /* ARABIC PERCENT SIGN */
    { 0x060c, 1, "afii57388" },            /* ARABIC COMMA */
    { 0x0660, 1, "afii57392" },            /* ARABIC-INDIC DIGIT ZERO */
    { 0x0661, 1, "afii57393" },            /* ARABIC-INDIC DIGIT ONE */
    { 0x0662, 1, "afii57394" },            /* ARABIC-INDIC DIGIT TWO */
    { 0x0663, 1, "afii57395" },            /* ARABIC-INDIC DIGIT THREE */
    { 0x0664, 1, "afii57396" },            /* ARABIC-INDIC DIGIT FOUR */
    { 0x0665, 1, "afii57397" },            /* ARABIC-INDIC DIGIT FIVE */
    { 0x0666, 1, "afii57398" },            /* ARABIC-INDIC DIGIT SIX */
    { 0x0667, 1, "afii57399" },            /* ARABIC-INDIC DIGIT SEVEN */
    { 0x0668, 1, "afii57400" },            /* ARABIC-INDIC DIGIT EIGHT */
    { 0x0669, 1, "afii57401" },            /* ARABIC-INDIC DIGIT NINE */
    { 0x061b, 1, "afii57403" },            /* ARABIC SEMICOLON */
    { 0x061f, 1, "afii57407" },            /* ARABIC QUESTION MARK */
    { 0x0621, 1, "afii57409" },            /* ARABIC LETTER HAMZA */
    { 0x0622, 1, "afii57410" },            /* ARABIC LETTER ALEF WITH MADDA ABOVE */
    { 0x0623, 1, "afii57411" },            /* ARABIC LETTER ALEF WITH HAMZA ABOVE */
    { 0x0624, 1, "afii57412" },            /* ARABIC LETTER WAW WITH HAMZA ABOVE */
    { 0x0625, 1, "afii57413" },            /* ARABIC LETTER ALEF WITH HAMZA BELOW */
    { 0x0626, 1, "afii57414" },            /* ARABIC LETTER YEH WITH HAMZA ABOVE */
    { 0x0627, 1, "afii57415" },            /* ARABIC LETTER ALEF */
    { 0x0628, 1, "afii57416" },            /* ARABIC LETTER BEH */
    { 0x0629, 1, "afii57417" },            /* ARABIC LETTER TEH MARBUTA */
    { 0x062a, 1, "afii57418" },            /* ARABIC LETTER TEH */
    { 0x062b, 1, "afii57419" },            /* ARABIC LETTER THEH */
    { 0x062c, 1, "afii57420" },            /* ARABIC LETTER JEEM */
    { 0x062d, 1, "afii57421" },            /* ARABIC LETTER HAH */
    { 0x062e, 1, "afii57422" },            /* ARABIC LETTER KHAH */
    { 0x062f, 1, "afii57423" },            /* ARABIC LETTER DAL */
    { 0x0630, 1, "afii57424" },            /* ARABIC LETTER THAL */
    { 0x0631, 1, "afii57425" },            /* ARABIC LETTER REH */
    { 0x0632, 1, "afii57426" },            /* ARABIC LETTER ZAIN */
    { 0x0633, 1, "afii57427" },            /* ARABIC LETTER SEEN */
    { 0x0634, 1, "afii57428" },            /* ARABIC LETTER SHEEN */
    { 0x0635, 1, "afii57429" },            /* ARABIC LETTER SAD */
    { 0x0636, 1, "afii57430" },            /* ARABIC LETTER DAD */
    { 0x0637, 1, "afii57431" },            /* ARABIC LETTER TAH */
    { 0x0638, 1, "afii57432" },            /* ARABIC LETTER ZAH */
    { 0x0639, 1, "afii57433" },            /* ARABIC LETTER AIN */
    { 0x063a, 1, "afii57434" },            /* ARABIC LETTER GHAIN */
    { 0x0640, 1, "afii57440" },            /* ARABIC TATWEEL */
    { 0x0641, 1, "afii57441" },            /* ARABIC LETTER FEH */
    { 0x0642, 1, "afii57442" },            /* ARABIC LETTER QAF */
    { 0x0643, 1, "afii57443" },            /* ARABIC LETTER KAF */
    { 0x0644, 1, "afii57444" },            /* ARABIC LETTER LAM */
    { 0x0645, 1, "afii57445" },            /* ARABIC LETTER MEEM */
    { 0x0646, 1, "afii57446" },            /* ARABIC LETTER NOON */
    { 0x0648, 1, "afii57448" },            /* ARABIC LETTER WAW */
    { 0x0649, 1, "afii57449" },            /* ARABIC LETTER ALEF MAKSURA */
    { 0x064a, 1, "afii57450" },            /* ARABIC LETTER YEH */
    { 0x064b, 1, "afii57451" },            /* ARABIC FATHATAN */
    { 0x064c, 1, "afii57452" },            /* ARABIC DAMMATAN */
    { 0x064d, 1, "afii57453" },            /* ARABIC KASRATAN */
    { 0x064e, 1, "afii57454" },            /* ARABIC FATHA */
    { 0x064f, 1, "afii57455" },            /* ARABIC DAMMA */
    { 0x0650, 1, "afii57456" },            /* ARABIC KASRA */
    { 0x0651, 1, "afii57457" },            /* ARABIC SHADDA */
    { 0x0652, 1, "afii57458" },            /* ARABIC SUKUN */
    { 0x0647, 1, "afii57470" },            /* ARABIC LETTER HEH */
    { 0x06a4, 1, "afii57505" },            /* ARABIC LETTER VEH */
    { 0x067e, 1, "afii57506" },            /* ARABIC LETTER PEH */
    { 0x0686, 1, "afii57507" },            /* ARABIC LETTER TCHEH */
    { 0x0698, 1, "afii57508" },            /* ARABIC LETTER JEH */
    { 0x06af, 1, "afii57509" },            /* ARABIC LETTER GAF */
    { 0x0679, 1, "afii57511" },            /* ARABIC LETTER TTEH */
    { 0x0688, 1, "afii57512" },            /* ARABIC LETTER DDAL */
    { 0x0691, 1, "afii57513" },            /* ARABIC LETTER RREH */
    { 0x06ba, 1, "afii57514" },            /* ARABIC LETTER NOON GHUNNA */
    { 0x06d2, 1, "afii57519" },            /* ARABIC LETTER YEH BARREE */
    { 0x06d5, 1, "afii57534" },            /* ARABIC LETTER AE */
    { 0x20aa, 1, "afii57636" },            /* NEW SHEQEL SIGN */
    { 0x05be, 1, "afii57645" },            /* HEBREW PUNCTUATION MAQAF */
    { 0x05c3, 1, "afii57658" },            /* HEBREW PUNCTUATION SOF PASUQ */
    { 0x05d0, 1, "afii57664" },            /* HEBREW LETTER ALEF */
    { 0x05d1, 1, "afii57665" },            /* HEBREW LETTER BET */
    { 0x05d2, 1, "afii57666" },            /* HEBREW LETTER GIMEL */
    { 0x05d3, 1, "afii57667" },            /* HEBREW LETTER DALET */
    { 0x05d4, 1, "afii57668" },            /* HEBREW LETTER HE */
    { 0x05d5, 1, "afii57669" },            /* HEBREW LETTER VAV */
    { 0x05d6, 1, "afii57670" },            /* HEBREW LETTER ZAYIN */
    { 0x05d7, 1, "afii57671" },            /* HEBREW LETTER HET */
    { 0x05d8, 1, "afii57672" },            /* HEBREW LETTER TET */
    { 0x05d9, 1, "afii57673" },            /* HEBREW LETTER YOD */
    { 0x05da, 1, "afii57674" },            /* HEBREW LETTER FINAL KAF */
    { 0x05db, 1, "afii57675" },            /* HEBREW LETTER KAF */
    { 0x05dc, 1, "afii57676" },            /* HEBREW LETTER LAMED */
    { 0x05dd, 1, "afii57677" },            /* HEBREW LETTER FINAL MEM */
    { 0x05de, 1, "afii57678" },            /* HEBREW LETTER MEM */
    { 0x05df, 1, "afii57679" },            /* HEBREW LETTER FINAL NUN */
    { 0x05e0, 1, "afii57680" },            /* HEBREW LETTER NUN */
    { 0x05e1, 1, "afii57681" },            /* HEBREW LETTER SAMEKH */
    { 0x05e2, 1, "afii57682" },            /* HEBREW LETTER AYIN */
    { 0x05e3, 1, "afii57683" },            /* HEBREW LETTER FINAL PE */
    { 0x05e4, 1, "afii57684" },            /* HEBREW LETTER PE */
    { 0x05e5, 1, "afii57685" },            /* HEBREW LETTER FINAL TSADI */
    { 0x05e6, 1, "afii57686" },            /* HEBREW LETTER TSADI */
    { 0x05e7, 1, "afii57687" },            /* HEBREW LETTER QOF */
    { 0x05e8, 1, "afii57688" },            /* HEBREW LETTER RESH */
    { 0x05e9, 1, "afii57689" },            /* HEBREW LETTER SHIN */
    { 0x05ea, 1, "afii57690" },            /* HEBREW LETTER TAV */
    { 0xfb2a, 1, "afii57694" },            /* HEBREW LETTER SHIN WITH SHIN DOT */
    { 0xfb2b, 1, "afii57695" },            /* HEBREW LETTER SHIN WITH SIN DOT */
    { 0xfb4b, 1, "afii57700" },            /* HEBREW LETTER VAV WITH HOLAM */
    { 0xfb1f, 1, "afii57705" },            /* HEBREW LIGATURE YIDDISH YOD YOD PATAH */
    { 0x05f0, 1, "afii57716" },            /* HEBREW LIGATURE YIDDISH DOUBLE VAV */
    { 0x05f1, 1, "afii57717" },            /* HEBREW LIGATURE YIDDISH VAV YOD */
    { 0x05f2, 1, "afii57718" },            /* HEBREW LIGATURE YIDDISH DOUBLE YOD */
    { 0xfb35, 1, "afii57723" },            /* HEBREW LETTER VAV WITH DAGESH */
    { 0x05b4, 1, "afii57793" },            /* HEBREW POINT HIRIQ */
    { 0x05b5, 1, "afii57794" },            /* HEBREW POINT TSERE */
    { 0x05b6, 1, "afii57795" },            /* HEBREW POINT SEGOL */
    { 0x05bb, 1, "afii57796" },            /* HEBREW POINT QUBUTS */
    { 0x05b8, 1, "afii57797" },            /* HEBREW POINT QAMATS */
    { 0x05b7, 1, "afii57798" },            /* HEBREW POINT PATAH */
    { 0x05b0, 1, "afii57799" },            /* HEBREW POINT SHEVA */
    { 0x05b2, 1, "afii57800" },            /* HEBREW POINT HATAF PATAH */
    { 0x05b1, 1, "afii57801" },            /* HEBREW POINT HATAF SEGOL */
    { 0x05b3, 1, "afii57802" },            /* HEBREW POINT HATAF QAMATS */
    { 0x05c2, 1, "afii57803" },            /* HEBREW POINT SIN DOT */
    { 0x05c1, 1, "afii57804" },            /* HEBREW POINT SHIN DOT */
    { 0x05b9, 1, "afii57806" },            /* HEBREW POINT HOLAM */
    { 0x05bc, 1, "afii57807" },            /* HEBREW POINT DAGESH OR MAPIQ */
    { 0x05bd, 1, "afii57839" },            /* HEBREW POINT METEG */
    { 0x05bf, 1, "afii57841" },            /* HEBREW POINT RAFE */
    { 0x05c0, 1, "afii57842" },            /* HEBREW PUNCTUATION PASEQ */
    { 0x02bc, 1, "afii57929" },            /* MODIFIER LETTER APOSTROPHE */
    { 0x2105, 1, "afii61248" },            /* CARE OF */
    { 0x2113, 1, "afii61289" },            /* SCRIPT SMALL L */
    { 0x2116, 1, "afii61352" },            /* NUMERO SIGN */
    { 0x202c, 1, "afii61573" },            /* POP DIRECTIONAL FORMATTING */
    { 0x202d, 1, "afii61574" },            /* LEFT-TO-RIGHT OVERRIDE */
    { 0x202e, 1, "afii61575" },            /* RIGHT-TO-LEFT OVERRIDE */
    { 0x200c, 1, "afii61664" },            /* ZERO WIDTH NON-JOINER */
    { 0x066d, 1, "afii63167" },            /* ARABIC FIVE POINTED STAR */
    { 0x02bd, 1, "afii64937" },            /* MODIFIER LETTER REVERSED COMMA */
    { 0x00e0, 1, "agrave" },               /* LATIN SMALL LETTER A WITH GRAVE */
    { 0x05d0, 1, "alef" },                 /* HEBREW LETTER ALEF (B&H) */
    { 0x2135, 1, "aleph" },                /* ALEF SYMBOL */
    { 0x03b1, 1, "alpha" },                /* GREEK SMALL LETTER ALPHA */
    { 0x03ac, 1, "alphatonos" },           /* GREEK SMALL LETTER ALPHA WITH TONOS */
    { 0x0101, 1, "amacron" },              /* LATIN SMALL LETTER A WITH MACRON */
    { 0x0026, 1, "ampersand" },            /* AMPERSAND */
    { 0x2220, 1, "angle" },                /* ANGLE */
    { 0x2329, 1, "angleleft" },            /* LEFT-POINTING ANGLE BRACKET */
    { 0x232a, 1, "angleright" },           /* RIGHT-POINTING ANGLE BRACKET */
    { 0x0387, 1, "anoteleia" },            /* GREEK ANO TELEIA */
    { 0x0105, 1, "aogonek" },              /* LATIN SMALL LETTER A WITH OGONEK */
    { 0x2248, 1, "approxequal" },          /* ALMOST EQUAL TO */
    { 0x00e5, 1, "aring" },                /* LATIN SMALL LETTER A WITH RING ABOVE */
    { 0x01fb, 1, "aringacute" },           /* LATIN SMALL LETTER A WITH RING ABOVE AND ACUTE */
    { 0x2194, 1, "arrowboth" },            /* LEFT RIGHT ARROW */
    { 0x21d4, 1, "arrowdblboth" },         /* LEFT RIGHT DOUBLE ARROW */
    { 0x21d3, 1, "arrowdbldown" },         /* DOWNWARDS DOUBLE ARROW */
    { 0x21d0, 1, "arrowdblleft" },         /* LEFTWARDS DOUBLE ARROW */
    { 0x21d2, 1, "arrowdblright" },        /* RIGHTWARDS DOUBLE ARROW */
    { 0x21d1, 1, "arrowdblup" },           /* UPWARDS DOUBLE ARROW */
    { 0x2193, 1, "arrowdown" },            /* DOWNWARDS ARROW */
    { 0x2190, 1, "arrowleft" },            /* LEFTWARDS ARROW */
    { 0x2192, 1, "arrowright" },           /* RIGHTWARDS ARROW */
    { 0x2191, 1, "arrowup" },              /* UPWARDS ARROW */
    { 0x2195, 1, "arrowupdn" },            /* UP DOWN ARROW */
    { 0x21a8, 1, "arrowupdnbse" },         /* UP DOWN ARROW WITH BASE */
    { 0x005e, 1, "asciicircum" },          /* CIRCUMFLEX ACCENT */
    { 0x007e, 1, "asciitilde" },           /* TILDE */
    { 0x002a, 1, "asterisk" },             /* ASTERISK */
    { 0x2217, 1, "asteriskmath" },         /* ASTERISK OPERATOR */
    { 0x0040, 1, "at" },                   /* COMMERCIAL AT */
    { 0x00e3, 1, "atilde" },               /* LATIN SMALL LETTER A WITH TILDE */
    { 0x05e2, 1, "ayin" },                 /* HEBREW LETTER AYIN (B&H) */
    { 0x0062, 1, "b" },                    /* LATIN SMALL LETTER B */
    { 0x005c, 1, "backslash" },            /* REVERSE SOLIDUS */
    { 0x007c, 1, "bar" },                  /* VERTICAL LINE */
    { 0x05d1, 1, "bet" },                  /* HEBREW LETTER BET (B&H) */
    { 0x03b2, 1, "beta" },                 /* GREEK SMALL LETTER BETA */
    { 0x2588, 1, "block" },                /* FULL BLOCK */
    { 0x007b, 1, "braceleft" },            /* LEFT CURLY BRACKET */
    { 0x007d, 1, "braceright" },           /* RIGHT CURLY BRACKET */
    { 0x005b, 1, "bracketleft" },          /* LEFT SQUARE BRACKET */
    { 0x005d, 1, "bracketright" },         /* RIGHT SQUARE BRACKET */
    { 0x02d8, 1, "breve" },                /* BREVE */
    { 0x00a6, 1, "brokenbar" },            /* BROKEN BAR */
    { 0x2022, 1, "bullet" },               /* BULLET */
    { 0x0063, 1, "c" },                    /* LATIN SMALL LETTER C */
    { 0x0107, 1, "cacute" },               /* LATIN SMALL LETTER C WITH ACUTE */
    { 0x02c7, 1, "caron" },                /* CARON */
    { 0x21b5, 1, "carriagereturn" },       /* DOWNWARDS ARROW WITH CORNER LEFTWARDS */
    { 0x010d, 1, "ccaron" },               /* LATIN SMALL LETTER C WITH CARON */
    { 0x00e7, 1, "ccedilla" },             /* LATIN SMALL LETTER C WITH CEDILLA */
    { 0x0109, 1, "ccircumflex" },          /* LATIN SMALL LETTER C WITH CIRCUMFLEX */
    { 0x010b, 1, "cdot" },                 /* LATIN SMALL LETTER C WITH DOT ABOVE (Non-Adobe) */
    { 0x010b, 1, "cdotaccent" },           /* LATIN SMALL LETTER C WITH DOT ABOVE */
    { 0x00b8, 1, "cedilla" },              /* CEDILLA */
    { 0x00a2, 1, "cent" },                 /* CENT SIGN */
    { 0x03c7, 1, "chi" },                  /* GREEK SMALL LETTER CHI */
    { 0x25cb, 1, "circle" },               /* WHITE CIRCLE */
    { 0x2297, 1, "circlemultiply" },       /* CIRCLED TIMES */
    { 0x2295, 1, "circleplus" },           /* CIRCLED PLUS */
    { 0x02c6, 1, "circumflex" },           /* MODIFIER LETTER CIRCUMFLEX ACCENT */
    { 0x2663, 1, "club" },                 /* BLACK CLUB SUIT */
    { 0x003a, 1, "colon" },                /* COLON */
    { 0x20a1, 1, "colonmonetary" },        /* COLON SIGN */
    { 0x002c, 1, "comma" },                /* COMMA */
    { 0x2245, 1, "congruent" },            /* APPROXIMATELY EQUAL TO */
    { 0x00a9, 1, "copyright" },            /* COPYRIGHT SIGN */
    { 0x00a4, 1, "currency" },             /* CURRENCY SIGN */
    { 0x0064, 1, "d" },                    /* LATIN SMALL LETTER D */
    { 0x2020, 1, "dagger" },               /* DAGGER */
    { 0x2021, 1, "daggerdbl" },            /* DOUBLE DAGGER */
    { 0x05d3, 1, "dalet" },                /* HEBREW LETTER DALET (B&H) */
    { 0x010f, 1, "dcaron" },               /* LATIN SMALL LETTER D WITH CARON */
    { 0x0111, 1, "dcroat" },               /* LATIN SMALL LETTER D WITH STROKE */
    { 0x00b0, 1, "degree" },               /* DEGREE SIGN */
    { 0x03b4, 1, "delta" },                /* GREEK SMALL LETTER DELTA */
    { 0x2666, 1, "diamond" },              /* BLACK DIAMOND SUIT */
    { 0x00a8, 1, "dieresis" },             /* DIAERESIS */
    { 0x0385, 1, "dieresistonos" },        /* GREEK DIALYTIKA TONOS */
    { 0x00f7, 1, "divide" },               /* DIVISION SIGN */
    { 0x2593, 1, "dkshade" },              /* DARK SHADE */
    { 0x2584, 1, "dnblock" },              /* LOWER HALF BLOCK */
    { 0x0024, 1, "dollar" },               /* DOLLAR SIGN */
    { 0x20ab, 1, "dong" },                 /* DONG SIGN */
    { 0x02d9, 1, "dotaccent" },            /* DOT ABOVE */
    { 0x0323, 1, "dotbelowcomb" },         /* COMBINING DOT BELOW */
    { 0x0131, 1, "dotlessi" },             /* LATIN SMALL LETTER DOTLESS I */
    { 0x22c5, 1, "dotmath" },              /* DOT OPERATOR */
    { 0x0065, 1, "e" },                    /* LATIN SMALL LETTER E */
    { 0x00e9, 1, "eacute" },               /* LATIN SMALL LETTER E WITH ACUTE */
    { 0x0115, 1, "ebreve" },               /* LATIN SMALL LETTER E WITH BREVE */
    { 0x011b, 1, "ecaron" },               /* LATIN SMALL LETTER E WITH CARON */
    { 0x00ea, 1, "ecircumflex" },          /* LATIN SMALL LETTER E WITH CIRCUMFLEX */
    { 0x00eb, 1, "edieresis" },            /* LATIN SMALL LETTER E WITH DIAERESIS */
    { 0x0117, 1, "edot" },                 /* LATIN SMALL LETTER E WITH DOT ABOVE (Non-Adobe) */
    { 0x0117, 1, "edotaccent" },           /* LATIN SMALL LETTER E WITH DOT ABOVE */
    { 0x00e8, 1, "egrave" },               /* LATIN SMALL LETTER E WITH GRAVE */
    { 0x0038, 1, "eight" },                /* DIGIT EIGHT */
    { 0x2088, 1, "eightinferior" },        /* SUBSCRIPT EIGHT */
    { 0x2078, 1, "eightsuperior" },        /* SUPERSCRIPT EIGHT */
    { 0x2208, 1, "element" },              /* ELEMENT OF */
    { 0x2026, 1, "ellipsis" },             /* HORIZONTAL ELLIPSIS */
    { 0x0113, 1, "emacron" },              /* LATIN SMALL LETTER E WITH MACRON */
    { 0x2014, 1, "emdash" },               /* EM DASH */
    { 0x2205, 1, "emptyset" },             /* EMPTY SET */
    { 0x2013, 1, "endash" },               /* EN DASH */
    { 0x014b, 1, "eng" },                  /* LATIN SMALL LETTER ENG */
    { 0x0119, 1, "eogonek" },              /* LATIN SMALL LETTER E WITH OGONEK */
    { 0x03b5, 1, "epsilon" },              /* GREEK SMALL LETTER EPSILON */
    { 0x03ad, 1, "epsilontonos" },         /* GREEK SMALL LETTER EPSILON WITH TONOS */
    { 0x003d, 1, "equal" },                /* EQUALS SIGN */
    { 0x2261, 1, "equivalence" },          /* IDENTICAL TO */
    { 0x212e, 1, "estimated" },            /* ESTIMATED SYMBOL */
    { 0x03b7, 1, "eta" },                  /* GREEK SMALL LETTER ETA */
    { 0x03ae, 1, "etatonos" },             /* GREEK SMALL LETTER ETA WITH TONOS */
    { 0x00f0, 1, "eth" },                  /* LATIN SMALL LETTER ETH */
    { 0x0021, 1, "exclam" },               /* EXCLAMATION MARK */
    { 0x203c, 1, "exclamdbl" },            /* DOUBLE EXCLAMATION MARK */
    { 0x00a1, 1, "exclamdown" },           /* INVERTED EXCLAMATION MARK */
    { 0x2203, 1, "existential" },          /* THERE EXISTS */
    { 0x0066, 1, "f" },                    /* LATIN SMALL LETTER F */
    { 0x2640, 1, "female" },               /* FEMALE SIGN */
    { 0xfb00, 1, "ff" },                   /* LATIN SMALL LIGATURE FF */
    { 0xfb03, 1, "ffi" },                  /* LATIN SMALL LIGATURE FFI */
    { 0xfb04, 1, "ffl" },                  /* LATIN SMALL LIGATURE FFL */
    { 0xfb01, 1, "fi" },                   /* LATIN SMALL LIGATURE FI */
    { 0x2012, 1, "figuredash" },           /* FIGURE DASH */
    { 0x25a0, 1, "filledbox" },            /* BLACK SQUARE */
    { 0x25ac, 1, "filledrect" },           /* BLACK RECTANGLE */
    { 0x0035, 1, "five" },                 /* DIGIT FIVE */
    { 0x215d, 1, "fiveeighths" },          /* VULGAR FRACTION FIVE EIGHTHS */
    { 0x2085, 1, "fiveinferior" },         /* SUBSCRIPT FIVE */
    { 0x2075, 1, "fivesuperior" },         /* SUPERSCRIPT FIVE */
    { 0xfb02, 1, "fl" },                   /* LATIN SMALL LIGATURE FL */
    { 0x0192, 1, "florin" },               /* LATIN SMALL LETTER F WITH HOOK */
    { 0x0034, 1, "four" },                 /* DIGIT FOUR */
    { 0x2084, 1, "fourinferior" },         /* SUBSCRIPT FOUR */
    { 0x2074, 1, "foursuperior" },         /* SUPERSCRIPT FOUR */
    { 0x2044, 2, "fraction" },             /* FRACTION SLASH & DIVISION SLASH (Double mapping) */
    { 0x2215, 2, "fraction" },
    { 0x20a3, 1, "franc" },                /* FRENCH FRANC SIGN */
    { 0x0067, 1, "g" },                    /* LATIN SMALL LETTER G */
    { 0x03b3, 1, "gamma" },                /* GREEK SMALL LETTER GAMMA */
    { 0x011f, 1, "gbreve" },               /* LATIN SMALL LETTER G WITH BREVE */
    { 0x01e7, 1, "gcaron" },               /* LATIN SMALL LETTER G WITH CARON */
    { 0x0123, 1, "gcedilla" },             /* LATIN SMALL LETTER G WITH CEDILLA (Non-Adobe) */
    { 0x011d, 1, "gcircumflex" },          /* LATIN SMALL LETTER G WITH CIRCUMFLEX */
    { 0x0123, 1, "gcommaaccent" },         /* LATIN SMALL LETTER G WITH CEDILLA */
    { 0x0121, 1, "gdot" },                 /* LATIN SMALL LETTER G WITH DOT ABOVE (Non-Adobe) */
    { 0x0121, 1, "gdotaccent" },           /* LATIN SMALL LETTER G WITH DOT ABOVE */
    { 0x00df, 1, "germandbls" },           /* LATIN SMALL LETTER SHARP S */
    { 0x05d2, 1, "gimel" },                /* HEBREW LETTER GIMEL (B&H) */
    { 0x2207, 1, "gradient" },             /* NABLA */
    { 0x0060, 1, "grave" },                /* GRAVE ACCENT */
    { 0x0300, 1, "gravecomb" },            /* COMBINING GRAVE ACCENT */
    { 0x003e, 1, "greater" },              /* GREATER-THAN SIGN */
    { 0x2265, 1, "greaterequal" },         /* GREATER-THAN OR EQUAL TO */
    { 0x00ab, 1, "guillemotleft" },        /* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
    { 0x00bb, 1, "guillemotright" },       /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
    { 0x2039, 1, "guilsinglleft" },        /* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
    { 0x203a, 1, "guilsinglright" },       /* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
    { 0x0068, 1, "h" },                    /* LATIN SMALL LETTER H */
    { 0x0127, 1, "hbar" },                 /* LATIN SMALL LETTER H WITH STROKE */
    { 0x0125, 1, "hcircumflex" },          /* LATIN SMALL LETTER H WITH CIRCUMFLEX */
    { 0x05d4, 1, "he" },                   /* HEBREW LETTER HE (B&H) */
    { 0x2665, 1, "heart" },                /* BLACK HEART SUIT */
    { 0x05d7, 1, "het" },                  /* HEBREW LETTER HET (B&H) */
    { 0x0309, 1, "hookabovecomb" },        /* COMBINING HOOK ABOVE */
    { 0x2302, 1, "house" },                /* HOUSE */
    { 0x02dd, 1, "hungarumlaut" },         /* DOUBLE ACUTE ACCENT */
    { 0x002d, 2, "hyphen" },               /* HYPHEN-MINUS & SOFT HYPHEN (Adobe double mapping) */
    { 0x00ad, 2, "hyphen" },
    { 0x0069, 1, "i" },                    /* LATIN SMALL LETTER I */
    { 0x00ed, 1, "iacute" },               /* LATIN SMALL LETTER I WITH ACUTE */
    { 0x012d, 1, "ibreve" },               /* LATIN SMALL LETTER I WITH BREVE */
    { 0x00ee, 1, "icircumflex" },          /* LATIN SMALL LETTER I WITH CIRCUMFLEX */
    { 0x00ef, 1, "idieresis" },            /* LATIN SMALL LETTER I WITH DIAERESIS */
    { 0x00ec, 1, "igrave" },               /* LATIN SMALL LETTER I WITH GRAVE */
    { 0x0133, 1, "ij" },                   /* LATIN SMALL LIGATURE IJ */
    { 0x012b, 1, "imacron" },              /* LATIN SMALL LETTER I WITH MACRON */
    { 0x221e, 1, "infinity" },             /* INFINITY */
    { 0x222b, 1, "integral" },             /* INTEGRAL */
    { 0x2321, 1, "integralbt" },           /* BOTTOM HALF INTEGRAL */
    { 0x2320, 1, "integraltp" },           /* TOP HALF INTEGRAL */
    { 0x2229, 1, "intersection" },         /* INTERSECTION */
    { 0x25d8, 1, "invbullet" },            /* INVERSE BULLET */
    { 0x25d9, 1, "invcircle" },            /* INVERSE WHITE CIRCLE */
    { 0x263b, 1, "invsmileface" },         /* BLACK SMILING FACE */
    { 0x012f, 1, "iogonek" },              /* LATIN SMALL LETTER I WITH OGONEK */
    { 0x03b9, 1, "iota" },                 /* GREEK SMALL LETTER IOTA */
    { 0x03ca, 1, "iotadieresis" },         /* GREEK SMALL LETTER IOTA WITH DIALYTIKA */
    { 0x0390, 1, "iotadieresistonos" },    /* GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS */
    { 0x03af, 1, "iotatonos" },            /* GREEK SMALL LETTER IOTA WITH TONOS */
    { 0x0129, 1, "itilde" },               /* LATIN SMALL LETTER I WITH TILDE */
    { 0x006a, 1, "j" },                    /* LATIN SMALL LETTER J */
    { 0x0135, 1, "jcircumflex" },          /* LATIN SMALL LETTER J WITH CIRCUMFLEX */
    { 0x006b, 1, "k" },                    /* LATIN SMALL LETTER K */
    { 0x05db, 1, "kaf" },                  /* HEBREW LETTER KAF (B&H) */
    { 0x05da, 1, "kaffinal" },             /* HEBREW LETTER FINAL KAF (B&H) */
    { 0x03ba, 1, "kappa" },                /* GREEK SMALL LETTER KAPPA */
    { 0x0137, 1, "kcedilla" },             /* LATIN SMALL LETTER K WITH CEDILLA (Non-Adobe) */
    { 0x0137, 1, "kcommaaccent" },         /* LATIN SMALL LETTER K WITH CEDILLA */
    { 0x0138, 1, "kgreenlandic" },         /* LATIN SMALL LETTER KRA */
    { 0x006c, 1, "l" },                    /* LATIN SMALL LETTER L */
    { 0x013a, 1, "lacute" },               /* LATIN SMALL LETTER L WITH ACUTE */
    { 0x03bb, 1, "lambda" },               /* GREEK SMALL LETTER LAMDA */
    { 0x05dc, 1, "lamed" },                /* HEBREW LETTER LAMED (B&H) */
    { 0x013e, 1, "lcaron" },               /* LATIN SMALL LETTER L WITH CARON */
    { 0x013c, 1, "lcedilla" },             /* LATIN SMALL LETTER L WITH CEDILLA (Non-Adobe) */
    { 0x013c, 1, "lcommaaccent" },         /* LATIN SMALL LETTER L WITH CEDILLA */
    { 0x0140, 1, "ldot" },                 /* LATIN SMALL LETTER L WITH MIDDLE DOT */
    { 0x003c, 1, "less" },                 /* LESS-THAN SIGN */
    { 0x2264, 1, "lessequal" },            /* LESS-THAN OR EQUAL TO */
    { 0x258c, 1, "lfblock" },              /* LEFT HALF BLOCK */
    { 0x20a4, 1, "lira" },                 /* LIRA SIGN */
    { 0x2227, 1, "logicaland" },           /* LOGICAL AND */
    { 0x00ac, 1, "logicalnot" },           /* NOT SIGN */
    { 0x2228, 1, "logicalor" },            /* LOGICAL OR */
    { 0x017f, 1, "longs" },                /* LATIN SMALL LETTER LONG S */
    { 0x25ca, 1, "lozenge" },              /* LOZENGE */
    { 0x0142, 1, "lslash" },               /* LATIN SMALL LETTER L WITH STROKE */
    { 0x2591, 1, "ltshade" },              /* LIGHT SHADE */
    { 0x006d, 1, "m" },                    /* LATIN SMALL LETTER M */
    { 0x00af, 2, "macron" },               /* MACRON & MODIFIER LETTER MACRON (Double mapping) */
    { 0x02c9, 2, "macron" },
    { 0x2642, 1, "male" },                 /* MALE SIGN */
    { 0x05de, 1, "mem" },                  /* HEBREW LETTER MEM (B&H) */
    { 0x05dd, 1, "memfinal" },             /* HEBREW LETTER FINAL MEM (B&H) */
    { 0x2212, 1, "minus" },                /* MINUS SIGN */
    { 0x2032, 1, "minute" },               /* PRIME */
    { 0x00b5, 2, "mu" },                   /* MICRO SIGN & GREEK SMALL LETTER MU (Double mapping) */
    { 0x03bc, 2, "mu" },
    { 0x00d7, 1, "multiply" },             /* MULTIPLICATION SIGN */
    { 0x266a, 1, "musicalnote" },          /* EIGHTH NOTE */
    { 0x266b, 1, "musicalnotedbl" },       /* BEAMED EIGHTH NOTES */
    { 0x006e, 1, "n" },                    /* LATIN SMALL LETTER N */
    { 0x0144, 1, "nacute" },               /* LATIN SMALL LETTER N WITH ACUTE */
    { 0x0149, 1, "napostrophe" },          /* LATIN SMALL LETTER N PRECEDED BY APOSTROPHE */
    { 0x00a0, 1, "nbspace" },              /* NO-BREAK SPACE (Non-Adobe) */
    { 0x0148, 1, "ncaron" },               /* LATIN SMALL LETTER N WITH CARON */
    { 0x0146, 1, "ncedilla" },             /* LATIN SMALL LETTER N WITH CEDILLA (Non-Adobe) */
    { 0x0146, 1, "ncommaaccent" },         /* LATIN SMALL LETTER N WITH CEDILLA */
    { 0x0039, 1, "nine" },                 /* DIGIT NINE */
    { 0x2089, 1, "nineinferior" },         /* SUBSCRIPT NINE */
    { 0x2079, 1, "ninesuperior" },         /* SUPERSCRIPT NINE */
    { 0x2209, 1, "notelement" },           /* NOT AN ELEMENT OF */
    { 0x2260, 1, "notequal" },             /* NOT EQUAL TO */
    { 0x2284, 1, "notsubset" },            /* NOT A SUBSET OF */
    { 0x207f, 1, "nsuperior" },            /* SUPERSCRIPT LATIN SMALL LETTER N */
    { 0x00f1, 1, "ntilde" },               /* LATIN SMALL LETTER N WITH TILDE */
    { 0x03bd, 1, "nu" },                   /* GREEK SMALL LETTER NU */
    { 0x0023, 1, "numbersign" },           /* NUMBER SIGN */
    { 0x05e0, 1, "nun" },                  /* HEBREW LETTER NUN (B&H) */
    { 0x05df, 1, "nunfinal" },             /* HEBREW LETTER FINAL NUN (B&H) */
    { 0x006f, 1, "o" },                    /* LATIN SMALL LETTER O */
    { 0x00f3, 1, "oacute" },               /* LATIN SMALL LETTER O WITH ACUTE */
    { 0x014f, 1, "obreve" },               /* LATIN SMALL LETTER O WITH BREVE */
    { 0x00f4, 1, "ocircumflex" },          /* LATIN SMALL LETTER O WITH CIRCUMFLEX */
    { 0x0151, 1, "odblacute" },            /* LATIN SMALL LETTER O WITH DOUBLE ACUTE (Non-Adobe) */
    { 0x00f6, 1, "odieresis" },            /* LATIN SMALL LETTER O WITH DIAERESIS */
    { 0x0153, 1, "oe" },                   /* LATIN SMALL LIGATURE OE */
    { 0x02db, 1, "ogonek" },               /* OGONEK */
    { 0x00f2, 1, "ograve" },               /* LATIN SMALL LETTER O WITH GRAVE */
    { 0x01a1, 1, "ohorn" },                /* LATIN SMALL LETTER O WITH HORN */
    { 0x0151, 1, "ohungarumlaut" },        /* LATIN SMALL LETTER O WITH DOUBLE ACUTE */
    { 0x014d, 1, "omacron" },              /* LATIN SMALL LETTER O WITH MACRON */
    { 0x03c9, 1, "omega" },                /* GREEK SMALL LETTER OMEGA */
    { 0x03d6, 1, "omega1" },               /* GREEK PI SYMBOL */
    { 0x03ce, 1, "omegatonos" },           /* GREEK SMALL LETTER OMEGA WITH TONOS */
    { 0x03bf, 1, "omicron" },              /* GREEK SMALL LETTER OMICRON */
    { 0x03cc, 1, "omicrontonos" },         /* GREEK SMALL LETTER OMICRON WITH TONOS */
    { 0x0031, 1, "one" },                  /* DIGIT ONE */
    { 0x2024, 1, "onedotenleader" },       /* ONE DOT LEADER */
    { 0x215b, 1, "oneeighth" },            /* VULGAR FRACTION ONE EIGHTH */
    { 0x00bd, 1, "onehalf" },              /* VULGAR FRACTION ONE HALF */
    { 0x2081, 1, "oneinferior" },          /* SUBSCRIPT ONE */
    { 0x00bc, 1, "onequarter" },           /* VULGAR FRACTION ONE QUARTER */
    { 0x00b9, 1, "onesuperior" },          /* SUPERSCRIPT ONE */
    { 0x2153, 1, "onethird" },             /* VULGAR FRACTION ONE THIRD */
    { 0x25e6, 1, "openbullet" },           /* WHITE BULLET */
    { 0x00aa, 1, "ordfeminine" },          /* FEMININE ORDINAL INDICATOR */
    { 0x00ba, 1, "ordmasculine" },         /* MASCULINE ORDINAL INDICATOR */
    { 0x221f, 1, "orthogonal" },           /* RIGHT ANGLE */
    { 0x00f8, 1, "oslash" },               /* LATIN SMALL LETTER O WITH STROKE */
    { 0x01ff, 1, "oslashacute" },          /* LATIN SMALL LETTER O WITH STROKE AND ACUTE */
    { 0x00f5, 1, "otilde" },               /* LATIN SMALL LETTER O WITH TILDE */
    { 0x0070, 1, "p" },                    /* LATIN SMALL LETTER P */
    { 0x00b6, 1, "paragraph" },            /* PILCROW SIGN */
    { 0x0028, 1, "parenleft" },            /* LEFT PARENTHESIS */
    { 0x208d, 1, "parenleftinferior" },    /* SUBSCRIPT LEFT PARENTHESIS */
    { 0x207d, 1, "parenleftsuperior" },    /* SUPERSCRIPT LEFT PARENTHESIS */
    { 0x0029, 1, "parenright" },           /* RIGHT PARENTHESIS */
    { 0x208e, 1, "parenrightinferior" },   /* SUBSCRIPT RIGHT PARENTHESIS */
    { 0x207e, 1, "parenrightsuperior" },   /* SUPERSCRIPT RIGHT PARENTHESIS */
    { 0x2202, 1, "partialdiff" },          /* PARTIAL DIFFERENTIAL */
    { 0x05e4, 1, "pe" },                   /* HEBREW LETTER PE (B&H) */
    { 0x05e3, 1, "pefinal" },              /* HEBREW LETTER FINAL PE (B&H) */
    { 0x0025, 1, "percent" },              /* PERCENT SIGN */
    { 0x002e, 1, "period" },               /* FULL STOP */
    { 0x00b7, 2, "periodcentered" },       /* MIDDLE DOT & BULLET OPERATOR (Double mapping) */
    { 0x22c5, 2, "periodcentered" },
    { 0x22a5, 1, "perpendicular" },        /* UP TACK */
    { 0x2030, 1, "perthousand" },          /* PER MILLE SIGN */
    { 0x20a7, 1, "peseta" },               /* PESETA SIGN */
    { 0x03c6, 1, "phi" },                  /* GREEK SMALL LETTER PHI */
    { 0x03d5, 1, "phi1" },                 /* GREEK PHI SYMBOL */
    { 0x03c0, 1, "pi" },                   /* GREEK SMALL LETTER PI */
    { 0x002b, 1, "plus" },                 /* PLUS SIGN */
    { 0x00b1, 1, "plusminus" },            /* PLUS-MINUS SIGN */
    { 0x211e, 1, "prescription" },         /* PRESCRIPTION TAKE */
    { 0x220f, 1, "product" },              /* N-ARY PRODUCT */
    { 0x2282, 1, "propersubset" },         /* SUBSET OF */
    { 0x2283, 1, "propersuperset" },       /* SUPERSET OF */
    { 0x221d, 1, "proportional" },         /* PROPORTIONAL TO */
    { 0x03c8, 1, "psi" },                  /* GREEK SMALL LETTER PSI */
    { 0x0071, 1, "q" },                    /* LATIN SMALL LETTER Q */
    { 0x05e7, 1, "qof" },                  /* HEBREW LETTER QOF (B&H) */
    { 0x003f, 1, "question" },             /* QUESTION MARK */
    { 0x00bf, 1, "questiondown" },         /* INVERTED QUESTION MARK */
    { 0x0022, 1, "quotedbl" },             /* QUOTATION MARK */
    { 0x201e, 1, "quotedblbase" },         /* DOUBLE LOW-9 QUOTATION MARK */
    { 0x201c, 1, "quotedblleft" },         /* LEFT DOUBLE QUOTATION MARK */
    { 0x201d, 1, "quotedblright" },        /* RIGHT DOUBLE QUOTATION MARK */
    { 0x2018, 1, "quoteleft" },            /* LEFT SINGLE QUOTATION MARK */
    { 0x201b, 1, "quotereversed" },        /* SINGLE HIGH-REVERSED-9 QUOTATION MARK */
    { 0x2019, 1, "quoteright" },           /* RIGHT SINGLE QUOTATION MARK */
    { 0x201a, 1, "quotesinglbase" },       /* SINGLE LOW-9 QUOTATION MARK */
    { 0x0027, 1, "quotesingle" },          /* APOSTROPHE */
    { 0x0072, 1, "r" },                    /* LATIN SMALL LETTER R */
    { 0x0155, 1, "racute" },               /* LATIN SMALL LETTER R WITH ACUTE */
    { 0x221a, 1, "radical" },              /* SQUARE ROOT */
    { 0x0159, 1, "rcaron" },               /* LATIN SMALL LETTER R WITH CARON */
    { 0x0157, 1, "rcedilla" },             /* LATIN SMALL LETTER R WITH CEDILLA (Non-Adobe) */
    { 0x0157, 1, "rcommaaccent" },         /* LATIN SMALL LETTER R WITH CEDILLA */
    { 0x2286, 1, "reflexsubset" },         /* SUBSET OF OR EQUAL TO */
    { 0x2287, 1, "reflexsuperset" },       /* SUPERSET OF OR EQUAL TO */
    { 0x00ae, 1, "registered" },           /* REGISTERED SIGN */
    { 0x05e8, 1, "resh" },                 /* HEBREW LETTER RESH (B&H) */
    { 0x2310, 1, "revlogicalnot" },        /* REVERSED NOT SIGN */
    { 0x03c1, 1, "rho" },                  /* GREEK SMALL LETTER RHO */
    { 0x02da, 1, "ring" },                 /* RING ABOVE */
    { 0x2590, 1, "rtblock" },              /* RIGHT HALF BLOCK */
    { 0x0073, 1, "s" },                    /* LATIN SMALL LETTER S */
    { 0x015b, 1, "sacute" },               /* LATIN SMALL LETTER S WITH ACUTE */
    { 0x05e1, 1, "samekh" },               /* HEBREW LETTER SAMEKH (B&H) */
    { 0x0161, 1, "scaron" },               /* LATIN SMALL LETTER S WITH CARON */
    { 0x015f, 1, "scedilla" },             /* LATIN SMALL LETTER S WITH CEDILLA */
    { 0x015d, 1, "scircumflex" },          /* LATIN SMALL LETTER S WITH CIRCUMFLEX */
    { 0x0219, 1, "scommaaccent" },         /* LATIN SMALL LETTER S WITH COMMA BELOW
                                              NOTE: Adobe maps this character to 0x0219, but
                                              some non-Adobe fonts actually maps this name to
                                              glyph that actually should have unicode value
                                              0x015f (EX: LucidaSansLat2).
                                              Since there is no way to check this,
                                              this mapping is left as Adobe made. */
    { 0x2033, 1, "second" },               /* DOUBLE PRIME */
    { 0x00a7, 1, "section" },              /* SECTION SIGN */
    { 0x003b, 1, "semicolon" },            /* SEMICOLON */
    { 0x0037, 1, "seven" },                /* DIGIT SEVEN */
    { 0x215e, 1, "seveneighths" },         /* VULGAR FRACTION SEVEN EIGHTHS */
    { 0x2087, 1, "seveninferior" },        /* SUBSCRIPT SEVEN */
    { 0x2077, 1, "sevensuperior" },        /* SUPERSCRIPT SEVEN */
    { 0x00ad, 1, "sfthyphen" },            /* SOFT HYPHEN (Non-Adobe) */
    { 0x2592, 1, "shade" },                /* MEDIUM SHADE */
    { 0x05e9, 1, "shin" },                 /* HEBREW LETTER SHIN (B&H) */
    { 0x03c3, 1, "sigma" },                /* GREEK SMALL LETTER SIGMA */
    { 0x03c2, 1, "sigma1" },               /* GREEK SMALL LETTER FINAL SIGMA */
    { 0x223c, 1, "similar" },              /* TILDE OPERATOR */
    { 0x0036, 1, "six" },                  /* DIGIT SIX */
    { 0x2086, 1, "sixinferior" },          /* SUBSCRIPT SIX */
    { 0x2076, 1, "sixsuperior" },          /* SUPERSCRIPT SIX */
    { 0x002f, 1, "slash" },                /* SOLIDUS */
    { 0x263a, 1, "smileface" },            /* WHITE SMILING FACE */
    { 0x0020, 2, "space" },                /* SPACE & NO-BREAK SPACE (Adobe double mapping) */
    { 0x00a0, 2, "space" },
    { 0x2660, 1, "spade" },                /* BLACK SPADE SUIT */
    { 0x00a3, 1, "sterling" },             /* POUND SIGN */
    { 0x220b, 1, "suchthat" },             /* CONTAINS AS MEMBER */
    { 0x2211, 1, "summation" },            /* N-ARY SUMMATION */
    { 0x263c, 1, "sun" },                  /* WHITE SUN WITH RAYS */
    { 0x0074, 1, "t" },                    /* LATIN SMALL LETTER T */
    { 0x03c4, 1, "tau" },                  /* GREEK SMALL LETTER TAU */
    { 0x05ea, 1, "tav" },                  /* HEBREW LETTER TAV (B&H) */
    { 0x0167, 1, "tbar" },                 /* LATIN SMALL LETTER T WITH STROKE */
    { 0x0165, 1, "tcaron" },               /* LATIN SMALL LETTER T WITH CARON */
    { 0x0163, 1, "tcedilla" },             /* LATIN SMALL LETTER T WITH CEDILLA (Non-Adobe) */
    { 0x021b, 1, "tcedilla1" },            /* LATIN SMALL LETTER T WITH COMMA BELOW (Non-Adobe) */
    { 0x0163, 2, "tcommaaccent" },         /* LATIN SMALL LETTER T WITH CEDILLA &
                                              LATIN SMALL LETTER T WITH COMMA BELOW
                                              (Adobe double mapping) */
    { 0x021b, 2, "tcommaaccent" },
    { 0x05d8, 1, "tet" },                  /* HEBREW LETTER TET (B&H) */
    { 0x2234, 1, "therefore" },            /* THEREFORE */
    { 0x03b8, 1, "theta" },                /* GREEK SMALL LETTER THETA */
    { 0x03d1, 1, "theta1" },               /* GREEK THETA SYMBOL */
    { 0x00fe, 1, "thorn" },                /* LATIN SMALL LETTER THORN */
    { 0x0033, 1, "three" },                /* DIGIT THREE */
    { 0x215c, 1, "threeeighths" },         /* VULGAR FRACTION THREE EIGHTHS */
    { 0x2083, 1, "threeinferior" },        /* SUBSCRIPT THREE */
    { 0x00be, 1, "threequarters" },        /* VULGAR FRACTION THREE QUARTERS */
    { 0x00b3, 1, "threesuperior" },        /* SUPERSCRIPT THREE */
    { 0x02dc, 1, "tilde" },                /* SMALL TILDE */
    { 0x0303, 1, "tildecomb" },            /* COMBINING TILDE */
    { 0x0384, 1, "tonos" },                /* GREEK TONOS */
    { 0x2122, 1, "trademark" },            /* TRADE MARK SIGN */
    { 0x25bc, 1, "triagdn" },              /* BLACK DOWN-POINTING TRIANGLE */
    { 0x25c4, 1, "triaglf" },              /* BLACK LEFT-POINTING POINTER */
    { 0x25ba, 1, "triagrt" },              /* BLACK RIGHT-POINTING POINTER */
    { 0x25b2, 1, "triagup" },              /* BLACK UP-POINTING TRIANGLE */
    { 0x05e6, 1, "tsadi" },                /* HEBREW LETTER TSADI (B&H) */
    { 0x05e5, 1, "tsadifinal" },           /* HEBREW LETTER FINAL TSADI (B&H) */
    { 0x0032, 1, "two" },                  /* DIGIT TWO */
    { 0x2025, 1, "twodotenleader" },       /* TWO DOT LEADER */
    { 0x2082, 1, "twoinferior" },          /* SUBSCRIPT TWO */
    { 0x00b2, 1, "twosuperior" },          /* SUPERSCRIPT TWO */
    { 0x2154, 1, "twothirds" },            /* VULGAR FRACTION TWO THIRDS */
    { 0x0075, 1, "u" },                    /* LATIN SMALL LETTER U */
    { 0x00fa, 1, "uacute" },               /* LATIN SMALL LETTER U WITH ACUTE */
    { 0x016d, 1, "ubreve" },               /* LATIN SMALL LETTER U WITH BREVE */
    { 0x00fb, 1, "ucircumflex" },          /* LATIN SMALL LETTER U WITH CIRCUMFLEX */
    { 0x0171, 1, "udblacute" },            /* LATIN SMALL LETTER U WITH DOUBLE ACUTE (Non-Adobe) */
    { 0x00fc, 1, "udieresis" },            /* LATIN SMALL LETTER U WITH DIAERESIS */
    { 0x00f9, 1, "ugrave" },               /* LATIN SMALL LETTER U WITH GRAVE */
    { 0x01b0, 1, "uhorn" },                /* LATIN SMALL LETTER U WITH HORN */
    { 0x0171, 1, "uhungarumlaut" },        /* LATIN SMALL LETTER U WITH DOUBLE ACUTE */
    { 0x016b, 1, "umacron" },              /* LATIN SMALL LETTER U WITH MACRON */
    { 0x005f, 1, "underscore" },           /* LOW LINE */
    { 0x2017, 1, "underscoredbl" },        /* DOUBLE LOW LINE */
    { 0x222a, 1, "union" },                /* UNION */
    { 0x2200, 1, "universal" },            /* FOR ALL */
    { 0x0173, 1, "uogonek" },              /* LATIN SMALL LETTER U WITH OGONEK */
    { 0x2580, 1, "upblock" },              /* UPPER HALF BLOCK */
    { 0x03c5, 1, "upsilon" },              /* GREEK SMALL LETTER UPSILON */
    { 0x03cb, 1, "upsilondieresis" },      /* GREEK SMALL LETTER UPSILON WITH DIALYTIKA */
    { 0x03b0, 1, "upsilondieresistonos" }, /* GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS */
    { 0x03cd, 1, "upsilontonos" },         /* GREEK SMALL LETTER UPSILON WITH TONOS */
    { 0x016f, 1, "uring" },                /* LATIN SMALL LETTER U WITH RING ABOVE */
    { 0x0169, 1, "utilde" },               /* LATIN SMALL LETTER U WITH TILDE */
    { 0x0076, 1, "v" },                    /* LATIN SMALL LETTER V */
    { 0x05d5, 1, "vav" },                  /* HEBREW LETTER VAV (B&H) */
    { 0x0077, 1, "w" },                    /* LATIN SMALL LETTER W */
    { 0x1e83, 1, "wacute" },               /* LATIN SMALL LETTER W WITH ACUTE */
    { 0x0175, 1, "wcircumflex" },          /* LATIN SMALL LETTER W WITH CIRCUMFLEX */
    { 0x1e85, 1, "wdieresis" },            /* LATIN SMALL LETTER W WITH DIAERESIS */
    { 0x2118, 1, "weierstrass" },          /* SCRIPT CAPITAL P */
    { 0x1e81, 1, "wgrave" },               /* LATIN SMALL LETTER W WITH GRAVE */
    { 0x0078, 1, "x" },                    /* LATIN SMALL LETTER X */
    { 0x03be, 1, "xi" },                   /* GREEK SMALL LETTER XI */
    { 0x0079, 1, "y" },                    /* LATIN SMALL LETTER Y */
    { 0x00fd, 1, "yacute" },               /* LATIN SMALL LETTER Y WITH ACUTE */
    { 0x0177, 1, "ycircumflex" },          /* LATIN SMALL LETTER Y WITH CIRCUMFLEX */
    { 0x00ff, 1, "ydieresis" },            /* LATIN SMALL LETTER Y WITH DIAERESIS */
    { 0x00a5, 1, "yen" },                  /* YEN SIGN */
    { 0x05d9, 1, "yod" },                  /* HEBREW LETTER YOD (B&H) */
    { 0x1ef3, 1, "ygrave" },               /* LATIN SMALL LETTER Y WITH GRAVE */
    { 0x007a, 1, "z" },                    /* LATIN SMALL LETTER Z */
    { 0x017a, 1, "zacute" },               /* LATIN SMALL LETTER Z WITH ACUTE */
    { 0x05d6, 1, "zayin" },                /* HEBREW LETTER ZAYIN (B&H) */
    { 0x017e, 1, "zcaron" },               /* LATIN SMALL LETTER Z WITH CARON */
    { 0x017c, 1, "zdot" },                 /* LATIN SMALL LETTER Z WITH DOT ABOVE (Non-Adobe) */
    { 0x017c, 1, "zdotaccent" },           /* LATIN SMALL LETTER Z WITH DOT ABOVE */
    { 0x0030, 1, "zero" },                 /* DIGIT ZERO */
    { 0x2080, 1, "zeroinferior" },         /* SUBSCRIPT ZERO */
    { 0x2070, 1, "zerosuperior" },         /* SUPERSCRIPT ZERO */
    { 0x03b6, 1, "zeta" },                 /* GREEK SMALL LETTER ZETA */
};

static const short psNameToUnicodeTableSize =
sizeof(psNameToUnicodeTable) / sizeof(psNameToUnicode);

/* These constants below help keep track of where the glyphs with
   their names starting with the respective characters (A-Z, a-z) begin,
   which is stored in array NUM_PS_GLYPHS.
   This array is crucial to speeding up unicode-glyph mapping.
   If number of entries in the mapping table above is modified,
   make sure to adjust the number stored in these constants accordingly.

   Numbers of entries for each constant can be checked by using C program below.
   Copy the psNameToUnicodeTable definition to a separate file and run this program with it.

#include <stdio.h>

void main() {
    char fileName[20] = "";
    char line[255];
    int i = 0, j = 0, total = 0;
    int counter[52];
    int sourceCharNum = 0;
    FILE *source = NULL;

    printf("File to check: ");
    fgets(fileName, 20, stdin);

    fileName[strlen(fileName) - 1] = 0;

    source = fopen(fileName, "r+");

    if (source == NULL)
      exit(1);

    for ( i = 0; i < 52; i++ )
      counter[i] = 0;

    while (!feof(source)) {
        while (fscanf(source, "%c", &line[sourceCharNum]) && line[sourceCharNum] != '\n')
          sourceCharNum++;

        for (i = 0; i <= sourceCharNum; i++)
          if (line[i] == '\"')
            if (line[i - 1] == ' ') {
                int charval;
                if (isupper(line[i + 1]))
                  charval = line[i + 1] - 'A';
                else
                  charval = line[i + 1] - 'a' + 26;

                counter[charval]++;
            }

        sourceCharNum = 0;
    }

    fclose(source);

    for ( i = 0; i < 52; i++ ){
        if ( i < 26 )
          printf("%c: %d\n", (i + 'A'), counter[i]);
        else
          printf("%c: %d\n", (i + 'a' - 26), counter[i]);
        total += counter[i];
    }

    printf("Total valid lines: %d\n", total);
}

*/

#define NUM_GLYPHS_A 15
#define NUM_GLYPHS_B 2
#define NUM_GLYPHS_C 8
#define NUM_GLYPHS_D 5
#define NUM_GLYPHS_E 18
#define NUM_GLYPHS_F 1
#define NUM_GLYPHS_G 9
#define NUM_GLYPHS_H 7
#define NUM_GLYPHS_I 16
#define NUM_GLYPHS_J 2
#define NUM_GLYPHS_K 4
#define NUM_GLYPHS_L 8
#define NUM_GLYPHS_M 2
#define NUM_GLYPHS_N 7
#define NUM_GLYPHS_O 19
#define NUM_GLYPHS_P 4
#define NUM_GLYPHS_Q 1
#define NUM_GLYPHS_R 7
#define NUM_GLYPHS_S 47
#define NUM_GLYPHS_T 10
#define NUM_GLYPHS_U 17
#define NUM_GLYPHS_V 1
#define NUM_GLYPHS_W 5
#define NUM_GLYPHS_X 2
#define NUM_GLYPHS_Y 5
#define NUM_GLYPHS_Z 6
#define NUM_GLYPHS_a 469
#define NUM_GLYPHS_b 13
#define NUM_GLYPHS_c 23
#define NUM_GLYPHS_d 20
#define NUM_GLYPHS_e 32
#define NUM_GLYPHS_f 21
#define NUM_GLYPHS_g 20
#define NUM_GLYPHS_h 11
#define NUM_GLYPHS_i 22
#define NUM_GLYPHS_j 2
#define NUM_GLYPHS_k 7
#define NUM_GLYPHS_l 19
#define NUM_GLYPHS_m 13
#define NUM_GLYPHS_n 19
#define NUM_GLYPHS_o 32
#define NUM_GLYPHS_p 29
#define NUM_GLYPHS_q 13
#define NUM_GLYPHS_r 14
#define NUM_GLYPHS_s 32
#define NUM_GLYPHS_t 34
#define NUM_GLYPHS_u 22
#define NUM_GLYPHS_v 2
#define NUM_GLYPHS_w 6
#define NUM_GLYPHS_x 2
#define NUM_GLYPHS_y 7
#define NUM_GLYPHS_z 10

#define TABLE_ENTRY_INDEX_A 0
#define TABLE_ENTRY_INDEX_B ( NUM_GLYPHS_A + TABLE_ENTRY_INDEX_A )
#define TABLE_ENTRY_INDEX_C ( NUM_GLYPHS_B + TABLE_ENTRY_INDEX_B )
#define TABLE_ENTRY_INDEX_D ( NUM_GLYPHS_C + TABLE_ENTRY_INDEX_C )
#define TABLE_ENTRY_INDEX_E ( NUM_GLYPHS_D + TABLE_ENTRY_INDEX_D )
#define TABLE_ENTRY_INDEX_F ( NUM_GLYPHS_E + TABLE_ENTRY_INDEX_E )
#define TABLE_ENTRY_INDEX_G ( NUM_GLYPHS_F + TABLE_ENTRY_INDEX_F )
#define TABLE_ENTRY_INDEX_H ( NUM_GLYPHS_G + TABLE_ENTRY_INDEX_G )
#define TABLE_ENTRY_INDEX_I ( NUM_GLYPHS_H + TABLE_ENTRY_INDEX_H )
#define TABLE_ENTRY_INDEX_J ( NUM_GLYPHS_I + TABLE_ENTRY_INDEX_I )
#define TABLE_ENTRY_INDEX_K ( NUM_GLYPHS_J + TABLE_ENTRY_INDEX_J )
#define TABLE_ENTRY_INDEX_L ( NUM_GLYPHS_K + TABLE_ENTRY_INDEX_K )
#define TABLE_ENTRY_INDEX_M ( NUM_GLYPHS_L + TABLE_ENTRY_INDEX_L )
#define TABLE_ENTRY_INDEX_N ( NUM_GLYPHS_M + TABLE_ENTRY_INDEX_M )
#define TABLE_ENTRY_INDEX_O ( NUM_GLYPHS_N + TABLE_ENTRY_INDEX_N )
#define TABLE_ENTRY_INDEX_P ( NUM_GLYPHS_O + TABLE_ENTRY_INDEX_O )
#define TABLE_ENTRY_INDEX_Q ( NUM_GLYPHS_P + TABLE_ENTRY_INDEX_P )
#define TABLE_ENTRY_INDEX_R ( NUM_GLYPHS_Q + TABLE_ENTRY_INDEX_Q )
#define TABLE_ENTRY_INDEX_S ( NUM_GLYPHS_R + TABLE_ENTRY_INDEX_R )
#define TABLE_ENTRY_INDEX_T ( NUM_GLYPHS_S + TABLE_ENTRY_INDEX_S )
#define TABLE_ENTRY_INDEX_U ( NUM_GLYPHS_T + TABLE_ENTRY_INDEX_T )
#define TABLE_ENTRY_INDEX_V ( NUM_GLYPHS_U + TABLE_ENTRY_INDEX_U )
#define TABLE_ENTRY_INDEX_W ( NUM_GLYPHS_V + TABLE_ENTRY_INDEX_V )
#define TABLE_ENTRY_INDEX_X ( NUM_GLYPHS_W + TABLE_ENTRY_INDEX_W )
#define TABLE_ENTRY_INDEX_Y ( NUM_GLYPHS_X + TABLE_ENTRY_INDEX_X )
#define TABLE_ENTRY_INDEX_Z ( NUM_GLYPHS_Y + TABLE_ENTRY_INDEX_Y )
#define TABLE_ENTRY_INDEX_a ( NUM_GLYPHS_Z + TABLE_ENTRY_INDEX_Z )
#define TABLE_ENTRY_INDEX_b ( NUM_GLYPHS_a + TABLE_ENTRY_INDEX_a )
#define TABLE_ENTRY_INDEX_c ( NUM_GLYPHS_b + TABLE_ENTRY_INDEX_b )
#define TABLE_ENTRY_INDEX_d ( NUM_GLYPHS_c + TABLE_ENTRY_INDEX_c )
#define TABLE_ENTRY_INDEX_e ( NUM_GLYPHS_d + TABLE_ENTRY_INDEX_d )
#define TABLE_ENTRY_INDEX_f ( NUM_GLYPHS_e + TABLE_ENTRY_INDEX_e )
#define TABLE_ENTRY_INDEX_g ( NUM_GLYPHS_f + TABLE_ENTRY_INDEX_f )
#define TABLE_ENTRY_INDEX_h ( NUM_GLYPHS_g + TABLE_ENTRY_INDEX_g )
#define TABLE_ENTRY_INDEX_i ( NUM_GLYPHS_h + TABLE_ENTRY_INDEX_h )
#define TABLE_ENTRY_INDEX_j ( NUM_GLYPHS_i + TABLE_ENTRY_INDEX_i )
#define TABLE_ENTRY_INDEX_k ( NUM_GLYPHS_j + TABLE_ENTRY_INDEX_j )
#define TABLE_ENTRY_INDEX_l ( NUM_GLYPHS_k + TABLE_ENTRY_INDEX_k )
#define TABLE_ENTRY_INDEX_m ( NUM_GLYPHS_l + TABLE_ENTRY_INDEX_l )
#define TABLE_ENTRY_INDEX_n ( NUM_GLYPHS_m + TABLE_ENTRY_INDEX_m )
#define TABLE_ENTRY_INDEX_o ( NUM_GLYPHS_n + TABLE_ENTRY_INDEX_n )
#define TABLE_ENTRY_INDEX_p ( NUM_GLYPHS_o + TABLE_ENTRY_INDEX_o )
#define TABLE_ENTRY_INDEX_q ( NUM_GLYPHS_p + TABLE_ENTRY_INDEX_p )
#define TABLE_ENTRY_INDEX_r ( NUM_GLYPHS_q + TABLE_ENTRY_INDEX_q )
#define TABLE_ENTRY_INDEX_s ( NUM_GLYPHS_r + TABLE_ENTRY_INDEX_r )
#define TABLE_ENTRY_INDEX_t ( NUM_GLYPHS_s + TABLE_ENTRY_INDEX_s )
#define TABLE_ENTRY_INDEX_u ( NUM_GLYPHS_t + TABLE_ENTRY_INDEX_t )
#define TABLE_ENTRY_INDEX_v ( NUM_GLYPHS_u + TABLE_ENTRY_INDEX_u )
#define TABLE_ENTRY_INDEX_w ( NUM_GLYPHS_v + TABLE_ENTRY_INDEX_v )
#define TABLE_ENTRY_INDEX_x ( NUM_GLYPHS_w + TABLE_ENTRY_INDEX_w )
#define TABLE_ENTRY_INDEX_y ( NUM_GLYPHS_x + TABLE_ENTRY_INDEX_x )
#define TABLE_ENTRY_INDEX_z ( NUM_GLYPHS_y + TABLE_ENTRY_INDEX_y )
#define TABLE_ENTRY_END     ( NUM_GLYPHS_z + TABLE_ENTRY_INDEX_z )

const int PSNAME_START_INDEX[] = {
    TABLE_ENTRY_INDEX_A,
    TABLE_ENTRY_INDEX_B,
    TABLE_ENTRY_INDEX_C,
    TABLE_ENTRY_INDEX_D,
    TABLE_ENTRY_INDEX_E,
    TABLE_ENTRY_INDEX_F,
    TABLE_ENTRY_INDEX_G,
    TABLE_ENTRY_INDEX_H,
    TABLE_ENTRY_INDEX_I,
    TABLE_ENTRY_INDEX_J,
    TABLE_ENTRY_INDEX_K,
    TABLE_ENTRY_INDEX_L,
    TABLE_ENTRY_INDEX_M,
    TABLE_ENTRY_INDEX_N,
    TABLE_ENTRY_INDEX_O,
    TABLE_ENTRY_INDEX_P,
    TABLE_ENTRY_INDEX_Q,
    TABLE_ENTRY_INDEX_R,
    TABLE_ENTRY_INDEX_S,
    TABLE_ENTRY_INDEX_T,
    TABLE_ENTRY_INDEX_U,
    TABLE_ENTRY_INDEX_V,
    TABLE_ENTRY_INDEX_W,
    TABLE_ENTRY_INDEX_X,
    TABLE_ENTRY_INDEX_Y,
    TABLE_ENTRY_INDEX_Z,
    TABLE_ENTRY_INDEX_a,
    TABLE_ENTRY_INDEX_b,
    TABLE_ENTRY_INDEX_c,
    TABLE_ENTRY_INDEX_d,
    TABLE_ENTRY_INDEX_e,
    TABLE_ENTRY_INDEX_f,
    TABLE_ENTRY_INDEX_g,
    TABLE_ENTRY_INDEX_h,
    TABLE_ENTRY_INDEX_i,
    TABLE_ENTRY_INDEX_j,
    TABLE_ENTRY_INDEX_k,
    TABLE_ENTRY_INDEX_l,
    TABLE_ENTRY_INDEX_m,
    TABLE_ENTRY_INDEX_n,
    TABLE_ENTRY_INDEX_o,
    TABLE_ENTRY_INDEX_p,
    TABLE_ENTRY_INDEX_q,
    TABLE_ENTRY_INDEX_r,
    TABLE_ENTRY_INDEX_s,
    TABLE_ENTRY_INDEX_t,
    TABLE_ENTRY_INDEX_u,
    TABLE_ENTRY_INDEX_v,
    TABLE_ENTRY_INDEX_w,
    TABLE_ENTRY_INDEX_x,
    TABLE_ENTRY_INDEX_y,
    TABLE_ENTRY_INDEX_z,
    TABLE_ENTRY_END
  };

/* Size of unicode->glyph index hash table */
#define	MAPPING_HASHTABLE_SIZE 256

#ifdef ENABLE_MAC_T1
typedef void (*DoFuncPtr) ( tt_uint32 *length, Handle hPOST, char *pCumulative );

static void ComputeLength( tt_uint32 *length, Handle hPOST, char *pCumulative )
{
    pCumulative;
    assert( hPOST != 0L );
    assert( *hPOST != 0L );

    *length += GetHandleSize( hPOST ) - 2;		/* The first two bytes "don't count" */
}

static void AppendData(tt_uint32 *length, Handle hPOST, char *pCumulative)
{
    tt_int32	start;

    assert( pCumulative != 0L );
    assert( hPOST != 0L );
    assert( *hPOST != 0L );

    start = *length;
    ComputeLength( length, hPOST, 0L );			/* This figures the end of the data */

    assert( (*length - start + 2L) == GetHandleSize( hPOST ));

    BlockMove( (*hPOST) + 2L, pCumulative + start, (*length - start) );
}

static void ForAllEncodedPOSTDo( DoFuncPtr fPtr, tt_uint32 *length, char *pCumulative, short refNum )
{
    short	rID;
    Handle	hRsrc;
    short	curRefNum;

    curRefNum = CurResFile(  ); /* save */
    for (rID = 501; rID < 32767; rID++ ) {
        UseResFile( refNum );
        hRsrc = Get1Resource( 'POST', rID );
        if ( hRsrc != NULL ) {
            short postType;

            postType = *((short*)*hRsrc);
            if ( postType != 0x0300 ) { /* Type 1 or 2 binary data */
                (*fPtr)( length, hRsrc, pCumulative );
            }
            ReleaseResource( hRsrc );
            if ( postType == 0x0300 ) break; /*****/ /* End of File */
        } else {
            break; /*****/
        }
    }
    UseResFile( curRefNum ); /* restore */
}


/* The caller has to free the returned pointer */
char *ExtractPureT1FromMacPOSTResources( tsiMemObject *mem, short refNum, tt_uint32 *length )
{
    char *returnData = 0L;

    *length = 0;
    ForAllEncodedPOSTDo( ComputeLength, length, returnData, refNum );
    if ( *length != 0 ) {
        returnData = (char *)tsi_AllocMem( mem, *length );
        *length = 0L;
        ForAllEncodedPOSTDo( AppendData, length, returnData, refNum );
    }
    return returnData; /*****/
}

/*
short refNum;
Str255 pascalname;

pascalName[0] = strlen( "AVReg.mac" );
strcpy( (char *)&pascalName[1], "AVReg.mac" );
refNum = OpenResFile( Str255 fileName );
assert( ResError() == noErr );
CloseResFile( refNum );
*/

#endif /* ENABLE_MAC_T1 */



unsigned char *ExtractPureT1FromPCType1( unsigned char *src, tt_uint32 *length )
{
    unsigned char *dest, *base;
    unsigned char b1, b2;
    unsigned int i, segmentLength ;

    assert( length != 0 );

    dest = base = src;

    while ( (tt_uint32)(src-base) + 6 <= *length ) {
        b1 = src[0];
        b2 = src[1];
        assert( b1 == 128 );
        if ( b2 == 3 ) break; /*****/ /* End of file indication */

        segmentLength = src[5];
        segmentLength <<= 8;
        segmentLength |= src[4];
        segmentLength <<= 8;
        segmentLength |= src[3];
        segmentLength <<= 8;
        segmentLength |= src[2];
        src += 6;

        if ((tt_uint32)(src-base) + segmentLength > *length) {
		  return NULL;
        }

        memmove(dest, src, segmentLength);
        dest += segmentLength;
        src  += segmentLength;

        /* if this was text segment that has trailing new line character */
        if (b2 == 1) {
          if ((*(dest-1) == '\r' || *(dest-1) == '\n')) { 
            /* strip trailing '\r' and '\n' except one (see 4907066) */
            while ( *(dest-1) == '\r' || *(dest-1) == '\n' ) {
              dest--;
            }
          } else {
            /* if we had no linebreaks at all then add one */
            *dest = '\n';
          }
          dest++;
        }
    }

    *length = dest - base;
    return base;
}



#ifdef ENABLE_DECRYPT
static unsigned char Decrypt( unsigned char cipher, tt_uint16 *r )
{
    unsigned char plain;

    plain	= (unsigned char)(cipher ^ (*r>>8));
    *r		= (tt_uint16)((cipher + *r) * c1 + c2);
    return	plain; /*****/
}
#endif

#ifdef ENABLE_ENCRYPT
static unsigned char Encrypt( unsigned char plain, tt_uint16 *r )
{
    unsigned char cipher;

    cipher	= (unsigned char)(plain ^ (*r>>8));
    *r		= (tt_uint16)((cipher + *r) * c1 + c2);
    return	cipher; /*****/
}
#endif


#ifdef ENABLE_EITHER_CRYPT

static short ATOI( register const tt_uint8 *data )
{
    register short num = 0;
    register tt_uint8 c;

    for (;;) {
        c = *data;
        if ( !( (c >= '0' && c <= '9') || c == '-' ) ) {
            data++;
            continue; /*****/
        }
        if ( c == '-' ) data++;
        break;/*****/
    }
    while ((*data >= '0') && (*data <= '9')) {
        num *= 10;
        num = (short)(num + *data - '0');
        data++;
    }
    return (short)(c == '-' ? -num: num); /*****/
}

static short backwardsATOI( register const char *data )
{
    register short num = 0;

    data++;

    while (*data == ' ') {
        data--;					/* Skip initial white-space */
    }

    while ((*data >= '0') && (*data <= '9'))
      data--;

    data++;

    while ((*data >= '0') && (*data <= '9')) {
        num *= 10;
        num = (short)(num + *data - '0');
        data++;
    }

    return num; /*****/
}
#endif

/* parses 10, 10.5, -10.9, .185339E-3 etc.. */
/* We also multiply the result by 10^expAdd  */
static F16Dot16 ATOFixed( register tt_uint8 *data, tt_int32 expAdd )
{
    register tt_int32 inum = 0;
    register tt_int32 num, denom, exponent;
    register tt_uint8 c;
    F16Dot16 result;

    for (;;) {
        c = *data;
        if ( !( (c >= '0' && c <= '9') || c == '-' || c == '.' ) ) {
            data++;
            continue; /*****/
        }
        if ( c == '-' ) data++;
        break;/*****/
    }
    while ((*data >= '0') && (*data <= '9')) {
        inum *= 10;
        inum = inum + (*data - '0');
        data++;
    }
    result = inum << 16;
    if ( *data == '.' ) {
        data++;
        for ( num = 0, denom = 1; (*data >= '0') && (*data <= '9'); ) {
            if ( denom < 100000000L ) {
                num *= 10; denom *= 10;
                num = num + (*data - '0');
            }
            data++;
        }
        exponent = ( *data == 'E' || *data == 'e' ) ? ATOI( ++data ) : 0;
        /* printf("inum = %d, num = %d, denom = %d, exponent = %d\n", inum, num, denom, exponent ); */
        /* The value is num/demom * 10 ^ exponent */
        exponent += expAdd; /* Multiply by 10^expAdd */
        for ( ; exponent > 0; exponent-- ) num   *= 10;
        for ( ; exponent < 0; exponent++ ) denom *= 10;
        result += util_FixDiv( (F16Dot16)num, (F16Dot16)denom );
    }
    return (c == '-' ? -result: result); /*****/
}



#ifdef ENABLE_DECRYPT
static int IsHex( register char c )
{
    return ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') );
}

static tt_uint8 MapHex( register tt_uint8 c )
{
    if ( c <= '9' ) {
        return (tt_uint8)(c - '0');
    } else if ( c <= 'F' ) {
        return (tt_uint8)(c - 'A' + 10);
    } else {
        return (tt_uint8)(c - 'a' + 10);
    }
}

static tt_int32 DecryptData( tt_uint8 *data, tt_int32 dataLen )
{
    tt_int32	i;
    tt_uint16	r1, r2, byteCount;
    tt_uint8   c_0, c_1, c_2, c_3;

    assert( dataLen >= 4 );

    r1			= 55665;	/* Magic starting decryption number - p.63 Black Book */
    byteCount	= 0;

    c_2 = c_1 = c_0 = 0;

    if ( IsHex( data[0] ) && IsHex( data[1] ) && IsHex( data[2] ) && IsHex( data[3] ) ) {
        tt_int32 j = 0;
        tt_uint8 binData, h1, h2;
        /* ascii form form */
        for ( i = 0; i < dataLen; ) {
            do { /* get data and skip white space */
                h1 = data[i++];
            } while ( h1 == ' ' || h1 == '\n' || h1 == '\r' || h1 == '\t' );
            do { /* get data and skip white space */
                h2 = data[i++];
            } while ( h2 == ' ' || h2 == '\n' || h2 == '\r' || h2 == '\t' );
            binData = MapHex( h1 );
            binData <<= 4;
            binData |= MapHex( h2 );

            if ( byteCount ) {			/* Decrypt a byte run which is doubly encoded */
                c_3 = c_2; c_2 = c_1; c_1 = c_0;
                data[j] = c_0 = Decrypt(Decrypt(binData, &r1), &r2);
                byteCount--;
            } else {
                c_3 = c_2; c_2 = c_1; c_1 = c_0;
                data[j] = c_0 = Decrypt(binData, &r1);
                if ( c_3 == ' ' && c_0 == ' ' && ((c_2 == 'R' && c_1 == 'D') || (c_2 == '-' && c_1 == '|')) ) {
                    byteCount	= backwardsATOI( (char*)&data[j-4] );
                    r2			= 4330;	/* charstring encryption has this initial R - p.64 Black Book */
                }
            }
            j++;
        }
        dataLen = i;
    } else {
        /* binary form */
        for ( i = 0; i < dataLen; i++ ) {
            if ( byteCount ) {			/* Decrypt a byte run which is doubly encoded */
                c_3 = c_2; c_2 = c_1; c_1 = c_0;
                data[i] = c_0 = Decrypt(Decrypt(data[i], &r1), &r2);
                byteCount--;
            } else {
                c_3 = c_2; c_2 = c_1; c_1 = c_0;
                data[i] = c_0 = Decrypt(data[i], &r1);
                if ( c_3 == ' ' && c_0 == ' ' && ((c_2 == 'R' && c_1 == 'D') || (c_2 == '-' && c_1 == '|')) ) {
                    byteCount	= backwardsATOI( (char*)&data[i-4] );
                    r2			= 4330;	/* charstring encryption has this initial R - p.64 Black Book */
                }
            }
        }
        assert( byteCount == 0);
        assert( i == dataLen );
    }
    return dataLen; /*****/
}
#endif

#ifdef ENABLE_ENCRYPT
/* Caller has to deallocate the returned pointer !!! */
static tt_uint8 *EncryptData( register T1Class *t, tt_uint8 *decryptedData, tt_int32 dataLen )
{
    tt_uint8	*data;
    tt_int32	i;
    tt_uint16	r1, r2, byteCount;

    assert( dataLen >= 4 );
    data = (tt_uint8 *)tsi_AllocMem(t->mem, dataLen);

    r1 			= 55665;	/* Magic starting decryption number - p.63 Black Book */
    byteCount	= 0;

    for ( i = 0; i < dataLen; i++ ) {
        if ( byteCount ) {			/* Copying a byte run which is doubly encoded */
            data[i] = Encrypt(Encrypt(decryptedData[i], &r2), &r1);
            byteCount--;
        } else {
            data[i] = Encrypt(decryptedData[i], &r1);
            if ( i >= 4 && (decryptedData[i-3] == ' ') && (decryptedData[i] == ' ') ) {
                tt_uint8 c_2 = decryptedData[i-2];
                tt_uint8 c_1 = decryptedData[i-1];
                if ( ((c_2 == 'R') && (c_1 == 'D')) || ((c_2 == '-') && (c_1 == '|')) ) {
                    byteCount	= backwardsATOI( (char*)&decryptedData[i-4] );
                    r2			= 4330;	/* charstring encryption has this initial R - p.64 Black Book */
                }
            }
        }
    }
    assert( byteCount == 0);
    assert( i == dataLen );
    return data; /*****/
}
#endif

/* Searches [start, limit] */
/* We might eventually want to use a Boyer-Moore Search for greater speed. ---Sampo */
static tt_uint8 *tsi_T1Find( T1Class *t, const tt_uint8 *param, int start, int limit )
{
    register int i, j, length;
    register tt_uint8 c0, *result, *p = t->decryptedData;
    assert( t->decryptedData != NULL );


    result = NULL;
    assert( p != NULL );
    assert( param != NULL );
    length = strlen( (const char *)param );
    c0 = param[0];
    for ( i = start; i < limit; i++ ) {
        if ( c0 == p[i] ) {
            int match = true;
            for ( j = 1; j < length; j++ ) {
                if ( p[i+j] != param[j] ) {
                    match = false;
                    break; /*****/
                }
            }
            if ( match ) {
                result = &p[i+j];
                break;/*****/
            }
        }
    }
    return result; /*****/
}


#define transform_decrypt 20
#define transform_encrypt 21
/*
 *
 */
static void TransformData( register T1Class *t, int tranform, tt_uint8 *data, tt_int32 dataLen, int *errCode )
{
    tt_uint8  *p;

    t->dataInPtr		= data;
    t->decryptedData	= data;
    t->dataLen			= dataLen;
#ifdef ROM_BASED_T1
    if ( tranform == transform_decrypt ) {
        assert( t->decryptedData == NULL );
        t->decryptedData	= (tt_uint8 *)tsi_AllocMem(t->mem, dataLen);
        t->dataLen			= dataLen;
        memcpy( t->decryptedData, data, dataLen );
    }
#endif

    t->eexecGO = dataLen;
    /* scan for (curre)ntfile eexec */
    p = tsi_T1Find( t, (tt_uint8 *)"ntfile eexec", 0, t->eexecGO );

    if (p == NULL) { /* Fail and return */
	if (errCode != NULL) {
	    *errCode = 1;
	}
	return;
    }

    t->eexecGO = 0;
    if ( p != NULL && (*p == '\r' || *p == '\n')) {
        /* Assume we have exactly one trailing new line here
           Rest is stripped in ExtractPureT1FromPCType1.
           NB: ExtractPureT1MacPOSTResources probably has different behavior! */
        p++;
        t->eexecGO = p - t->decryptedData;
    }
    if ( tranform == transform_decrypt ) {
        if ( t->eexecGO > 0 ) {
            /* ExtractPureT1FromPCType1 eliminates the need for this PC specific hack */
#ifdef OLD
            t->eexecGO += 6; /* skip the 6 header bytes in the pfb data */
#endif
            t->dataLen = DecryptData( &t->decryptedData[t->eexecGO], dataLen - t->eexecGO ) + t->eexecGO;
        }
    }

}

static void SetMetrics( T1Class *t, tt_int32 lsbx, tt_int32 lsby, tt_int32 awx, tt_int32 awy )
{
    t->lsbx = lsbx;
    t->lsby = lsby;
    t->awx 	= awx;
    t->awy 	= awy;
}

/* Converts PostScript character name to equivalent unicode value

   Parameters: PSName - Postscript character name to convert
               charCodes - Array in which the code will be stored
   Returns: Number of unicode value that maps to this PS character */

static int PSNameToUnicode( char *PSName, tt_uint16* charCodes ) {
    register int i, j;
    char firstChar = *PSName;
    int lookupIndex, startIndex, endIndex;

    /* Finds the index to look up in PSNAME_START_INDEX */

    if ( firstChar >= 'A' && firstChar <= 'Z' )
      lookupIndex = firstChar - 'A';
    else
      lookupIndex = firstChar - 'a' + 26;


    startIndex = PSNAME_START_INDEX[ lookupIndex ];
    endIndex = PSNAME_START_INDEX[ lookupIndex + 1 ];

    for ( i = startIndex; i < endIndex; i++ ) {
        if ( !strcmp( psNameToUnicodeTable[i].name, (char *)PSName ) ) {
            int numCodes = psNameToUnicodeTable[i].numMapping;
            if ( numCodes > 1 ) {
                /* This glyph can be mapped to multiple unicode value */
                for ( j = 0; j < numCodes; j++ )
                  charCodes[j] = psNameToUnicodeTable[i + j].unicodeIndex;
                return numCodes;
            }
            else {
                /* Single mapping */
                charCodes[0] = psNameToUnicodeTable[i].unicodeIndex;
                return 1;
            }
        }
    }

    /* Check for special "uniXXXX" form names */

    if ( strlen( PSName ) == 7 )
      if ( firstChar == 'u' && PSName[1] == 'n' && PSName[2] == 'i' ) {
          for ( i = 3; i < 7; i++ )
            /* Make sure "XXXX" part follows Adobe's definition... */
            if (!(( toupper( PSName[i] ) >= '0' && toupper( PSName[i] ) <= '9' ) ||
                  ( toupper( PSName[i] ) >= 'A' && toupper( PSName[i] ) <= 'F' )))
              return 0;

          charCodes[0] = (tt_uint16)strtoul( &PSName[3], (char **)NULL, 16 );
          return 1;
      }

    return 0;
}

tt_uint16 hashUnicodeValue( tt_uint16 charCode ) {
    return (tt_uint16)(charCode % MAPPING_HASHTABLE_SIZE);
}

/* Makes a table mapping unicode values to font glyph index */

void tsi_T1AddUnicodeToGIMapping( T1Class *t,
                                  char* PSName, int glyphIndex ) {
    if ( *PSName == '.' )
      /* This is a ".notdef" character */
      t->notdefGlyphIndex = glyphIndex;
    else {
        /* Regular character */
        tt_uint16 charCodes[MAX_NUM_MAPPING];
        unicodeToGI **mappingList = t->unicodeToGITable;

        int numCodes = PSNameToUnicode( PSName, charCodes );
        int i;

        for ( i = 0; i < numCodes; i++ ) {
            int k;
            unicodeToGI **mappingNode = &mappingList[hashUnicodeValue( charCodes[i] )];
            fflush(stdout);
            while ( *mappingNode != NULL ) {
                if ( (*mappingNode)->unicodeIndex == charCodes[i] ) {
                    /* If there is a duplicate entry, then give priority to
                       one which is not doubly mapped.
                       EX: If "sfthyphen" is defined, give it priority to be mapped to 0x00ad
                           over "hyphen" being doubly mapped to 0x002d and 0x00ad */
                    if ( numCodes == 1 )
                      (*mappingNode)->glyphIndex = glyphIndex;
                    break;
                }

                /* The compiler throws warning here for incompatible type...
                   This is left as is since force casting this causes compilation error... */
                mappingNode = (unicodeToGI **) &(*mappingNode)->next;
            }

            /* If new entry is needed... */
            if ( *mappingNode == NULL) {
                /* Here, malloc is called instead of the usual tsi_AllocMem,
                   as tsi_AllocMem has upper limit on the number of pointers. (256?)
                   (Since my hash table is a table with chained lists, it can potentially
                    have a very large number of pointers.
                    Please take a look at tsi_DeAllocChainedList too) */

                (*mappingNode) = malloc( sizeof( unicodeToGI ) );
                (*mappingNode)->unicodeIndex = charCodes[i];
                (*mappingNode)->glyphIndex = glyphIndex;
                (*mappingNode)->next = NULL;
            }
        }
    }
}

/* Returns the glyph index of the given unicode character value */
static tt_uint16 INVISIBLE_GLYPH_ID = 0xffff;
tt_uint16 tsi_T1GetGlyphIndex( T1Class *t, register tt_uint32 charCode )
{
    register unicodeToGI *mappingNode = t->unicodeToGITable[ hashUnicodeValue( (tt_uint16)charCode ) ];

    if (charCode < 0x0010) {
      switch (charCode) {
      case 0x0009:
      case 0x000a:
      case 0x000d: return INVISIBLE_GLYPH_ID;
      }
    } else if (charCode >= 0x200c) {
	if ((charCode <= 0x200f) ||
	    (charCode >= 0x2028 && charCode <= 0x202e) ||
	    (charCode >= 0x206a && charCode <= 0x206f)) {
	    return INVISIBLE_GLYPH_ID;
	}
    }

    while ( mappingNode != NULL ) {
        tt_uint16 unicodeIndex = mappingNode->unicodeIndex;
        if ( charCode == unicodeIndex )
          return mappingNode->glyphIndex;
        else
          mappingNode = mappingNode->next;
    }

    return (tt_uint16)t->notdefGlyphIndex;
}

/* 
   This table maps character codes from Adobe StandardEncoding Encoding Vector
     (see Appendix E.6 of Postscript Language Reference Manual)
	 to Unicode (consult www.unicode.org to identify missing glyphs) */

#define NOTDEF 0x0000 /* glyph is not defined in StandardEncoding vector */
#define TBI    0x0000 /* target glyph code is "to be indentified" in Unicode code charts) 
                         NB: someday we will identify all glyphs and this macro will gone */

static tt_uint16 adobeCharCodeMapping[128] = {
  /*0x80*/  NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF,
  /*0x88*/  NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF,
  /*0x90*/  NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF,
  /*0x98*/  NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF,
  /*0xa0*/  NOTDEF, 0x00a1,    TBI, 0x00a3,    TBI, 0x00a5,    TBI, 0x00a7,
  /*0xa8*/     TBI,    TBI,    TBI, 0x00ab,    TBI,    TBI,    TBI,    TBI,
  /*0xb0*/  NOTDEF,    TBI,    TBI,    TBI,    TBI, NOTDEF,    TBI,    TBI,
  /*0xb8*/     TBI,    TBI,    TBI, 0x00bb,    TBI,    TBI, NOTDEF,    TBI,
  /*0xc0*/  NOTDEF, 0x0060, 0x00b4, 0x02c6, 0x02dc, 0x00af, 0x02d8, 0x02d9,
  /*0xc8*/  0x00a8, NOTDEF, 0x02da, 0x00b8, NOTDEF, 0x02dd, 0x02d8, 0x02c7,
  /*0xd0*/     TBI, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF,
  /*0xd8*/  NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF,
  /*0xe0*/  NOTDEF, 0x00c6, NOTDEF,    TBI, NOTDEF, NOTDEF, NOTDEF, NOTDEF,
  /*0xe8*/  0x0141,    TBI, 0x0152,    TBI, NOTDEF, NOTDEF, NOTDEF, NOTDEF,
  /*0xf0*/  NOTDEF, 0x00e6, NOTDEF, NOTDEF, NOTDEF, 0x0131, NOTDEF, NOTDEF,
  /*0xf8*/  0x0142,    TBI, 0x0153,    TBI, NOTDEF, NOTDEF, NOTDEF, NOTDEF
};

/* Conversion from Adobe character code to glyph index
   Used in SEAC rendering */
static tt_uint16 tsi_T1GetGlyphIndexFromAdobeCode( T1Class *t,
                                                   tt_uint16 adobeCharCode ) {
    tt_uint16 code;
    if (adobeCharCode >= 0x20 && adobeCharCode <= 127) /* from space to ~ codes are the same */
      code = adobeCharCode;
    else if (adobeCharCode >= 128 && adobeCharCode <= 255) 
      code = adobeCharCodeMapping[adobeCharCode-128];
    else 
	  code = NOTDEF; /* codes lower than 0x20 or higher than 0xff are not defined */

    return tsi_T1GetGlyphIndex(t, code);
}

#define STACK_BOTTOM 0


/* CasloBooBEReg */

static void Type1BuildChar( T1Class *t, tt_uint8 *p, int byteCount, int nestingLevel )
{
    register int i, v1, v2;
    register tt_int32 num;
    register int verbose = true;
    tt_int32 x = t->x; /* cache x and y in local variables */
    tt_int32 y = t->y;


    for ( i = 0; i < byteCount; ) {
        v1 = p[i++];
        /* printf("%d : %d\n", i, (int)v1 ); */
        /* LOG_CMD( "byte #", i ); 			*/
        /* LOG_CMD( "byte v1", (int)v1 ); 	*/
        if ( v1 < 32 ) {
            /* A command */
            /* LOG_CMD( "CMD v1", (int)v1 );	*/

            switch( v1 ) {
              case  0:
                break;
              case  1:	/* hstem */
                LOG_CMD( "hstem", t->gNumStackValues );
                /* this check looks odd pending future hinting support */
                if ( t->gNumStackValues < 2 ) break;
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case  2:
                break;
              case  3:	/* vstem */
                LOG_CMD( "vstem", t->gNumStackValues );
                /* this check looks odd pending future hinting support */
                if ( t->gNumStackValues < 2 ) break;
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case  4:	/* vmoveto */
                LOG_CMD( "vmoveto", t->gNumStackValues );
                if ( t->gNumStackValues < 1 ) break;
                y += t->gStackValues[ STACK_BOTTOM + 0 ];
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case  5:	/* rlineto */
                LOG_CMD( "rlineto", t->gNumStackValues );
                if ( t->gNumStackValues < 2 ) break;
                glyph_StartLine( t->glyph, x, y );
                x += t->gStackValues[ STACK_BOTTOM + 0 ];
                y += t->gStackValues[ STACK_BOTTOM + 1 ];
                glyph_AddPoint( t->glyph, x, y, 1 );
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case  6:	/* hlineto */
                LOG_CMD( "hlineto", t->gNumStackValues );
                if ( t->gNumStackValues < 1 ) break;
                glyph_StartLine( t->glyph, x, y );
                x += t->gStackValues[ STACK_BOTTOM + 0 ];
                glyph_AddPoint( t->glyph, x, y, 1 );
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case  7:	/* vlineto */
                LOG_CMD( "vlineto", t->gNumStackValues );
                if ( t->gNumStackValues < 1 ) break;
                glyph_StartLine( t->glyph, x, y );
                y += t->gStackValues[ STACK_BOTTOM + 0 ];
                glyph_AddPoint( t->glyph, x, y, 1 );
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case  8:	/* rrcurveto */
                LOG_CMD( "rrcurveto", t->gNumStackValues );
                if ( t->gNumStackValues < 6 ) break;
                glyph_StartLine( t->glyph, x, y );
                x += t->gStackValues[ STACK_BOTTOM + 0 ];
                y += t->gStackValues[ STACK_BOTTOM + 1 ];
                glyph_AddPoint( t->glyph, x, y, 0 );
                x += t->gStackValues[ STACK_BOTTOM + 2 ];
                y += t->gStackValues[ STACK_BOTTOM + 3 ];
                glyph_AddPoint( t->glyph, x, y, 0 );
                x += t->gStackValues[ STACK_BOTTOM + 4 ];
                y += t->gStackValues[ STACK_BOTTOM + 5 ];
                glyph_AddPoint( t->glyph, x, y, 1 );
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case  9:	/* closepath */
                LOG_CMD( "closepath", t->gNumStackValues );
                glyph_CloseContour( t->glyph );
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case 10:	/* callsubr */
                LOG_CMD( "callsubr", t->gNumStackValues );
                if ( t->gNumStackValues < 1 ) break;
                t->gNumStackValues -= 1;
                {
                    int fnum = t->gStackValues[ t->gNumStackValues + 0 ]; /* topmost element */
                    int fByteCount;

                    if ( fnum >= 0 && fnum < t->numSubrs ) {
                        fByteCount	= backwardsATOI( (char *)(t->subrsData[fnum]-5) );
                        LOG_CMD("***callsubr number =  ", fnum);
                        LOG_CMD("***callsubr fByteCount = ", fByteCount);
                        LOG_CMD("***BEGIN CALL = ", fnum );
                        /* Type1BuildChar( t, x, y, t->subrsData[fnum], fByteCount ); */
                        fByteCount -= t->lenIV;
                        if ( fByteCount > 0 && nestingLevel < 10 ) {
                            t->x = x;
                            t->y = y;
                            Type1BuildChar( t, t->subrsData[fnum] + t->lenIV, fByteCount, nestingLevel+1 );
                            x = t->x;
                            y = t->y;
                        }
                        LOG_CMD("***END CALL = ", fnum );
                    }
                    assert( t->gNumStackValues >= 0 );
                    assert( t->gNumStackValues <= kMaxStackValues );
                }
                /* Do not clear the stack */
                /* t->gNumStackValues = 0; */
                break;
              case 11:	/* return  (from callsubr) */
                LOG_CMD( "return", t->gNumStackValues );
                t->x = x;
                t->y = y;
                return; /*****/
                /* Do not clear the stack */
              case 12:	/* escape */
                v2 = p[i++];
                switch( v2 ) {
                  case 0:	 	/* dotsections */
                    LOG_CMD( "dotsections", t->gNumStackValues );
                    /* this check looks odd pending future hinting support */
                    t->gNumStackValues = 0; /* clear the stack */
                    break;
                  case 1: 	/* vstem3 */
                    LOG_CMD( "vstem3", t->gNumStackValues );
                    /* this check looks odd pending future hinting support */
                    if ( t->gNumStackValues < 6 ) break;
                    t->gNumStackValues = 0; /* clear the stack */
                    break;
                  case 2: 	/* hstem3 */
                    LOG_CMD( "hstem3", t->gNumStackValues );
                    if ( t->gNumStackValues < 6 ) break;
                    /* this check looks odd pending future hinting support */
                    t->gNumStackValues = 0; /* clear the stack */
                    break;
                  case 6: 	/* seac (standard encoding accented character) */
                    {
                        tt_int32 asb, adx, ady, leftSideBearing;

                        tt_uint16 bchar, achar;
                        tt_uint16 flags;
                        GlyphClass *glyph = t->glyph;


                        LOG_CMD( "seac", t->gNumStackValues );
                        if ( t->gNumStackValues < 5 ) break;
                        asb = t->gStackValues[ STACK_BOTTOM + 0 ];
                        adx = t->gStackValues[ STACK_BOTTOM + 1 ]; /* The offset of the left side bearing points */
                        ady = t->gStackValues[ STACK_BOTTOM + 2 ];
                        bchar = (tt_uint16)t->gStackValues[ STACK_BOTTOM + 3 ];
                        achar = (tt_uint16)t->gStackValues[ STACK_BOTTOM + 4 ];

                        LOG_CMD( "achar code: ", (int)achar );
                        LOG_CMD( "bchar code: ", (int)bchar );

                        /* map to glyph index */
                        achar = tsi_T1GetGlyphIndexFromAdobeCode( t, achar );
                        bchar = tsi_T1GetGlyphIndexFromAdobeCode( t, bchar );
                        LOG_CMD( "achar index: ", (int)achar );
                        LOG_CMD( "bchar index: ", (int)bchar );

                        leftSideBearing  = achar < t->NumCharStrings ? t->noDelete_hmtx->lsb[achar] : 0; /* Added 7/17/98 ---Sampo */
                        leftSideBearing -= bchar < t->NumCharStrings ? t->noDelete_hmtx->lsb[bchar] : 0;

                        glyph->componentSizeMax	= 64;
                        glyph->componentData	= (short*) tsi_AllocMem( t->mem, glyph->componentSizeMax * sizeof(short) );
                        glyph->componentSize	= 0;
                        flags = 0;
                        flags |= ARG_1_AND_2_ARE_WORDS;
                        flags |= MORE_COMPONENTS;
                        flags |= (ARGS_ARE_XY_VALUES | ROUND_XY_TO_GRID);
                        glyph->componentData[ glyph->componentSize++] = (short)flags;
                        glyph->componentData[ glyph->componentSize++] = (short)bchar; /* base charcode not glyph index */
                        glyph->componentData[ glyph->componentSize++] = 0;
                        glyph->componentData[ glyph->componentSize++] = 0;
                        flags = 0;
                        flags |= ARG_1_AND_2_ARE_WORDS;
                        flags |= (ARGS_ARE_XY_VALUES | ROUND_XY_TO_GRID);
                        glyph->componentData[ glyph->componentSize++] = (short)flags;
                        glyph->componentData[ glyph->componentSize++] = (short)achar; /* accent charcode not glyph index */
                        glyph->componentData[ glyph->componentSize++] = (short)(adx - leftSideBearing); /* 7/17/98 changed from (short)adx since lsb impact this */
                        glyph->componentData[ glyph->componentSize++] = (short)ady;

                        glyph->contourCount = -1;
                    }
                    t->gNumStackValues = 0; /* clear the stack */
                    break;
                  case 7: 	/* sbw */
                    {
                        tt_int32 lsbx, lsby, awx, awy;
                        LOG_CMD( "sbw", t->gNumStackValues );
                        if ( t->gNumStackValues < 4 ) break;
                        lsbx = t->gStackValues[ STACK_BOTTOM + 0 ];
                        lsby = t->gStackValues[ STACK_BOTTOM + 1 ];
                        awx  = t->gStackValues[ STACK_BOTTOM + 2 ];
                        awy  = t->gStackValues[ STACK_BOTTOM + 3 ];
                        x = lsbx;
                        y = lsby;
                        SetMetrics( t, lsbx, lsby, awx, awy );
                    }
                    t->gNumStackValues = 0; /* clear the stack */
                    break;
                  case 12: 	/* div */
                    LOG_CMD( "div", t->gNumStackValues );
                    /* Warning this is different from Scott's old code ---Sampo */
                    /* We also simulate div with an integer idiv operation */
                    if ( t->gNumStackValues < 2 ) break;
                    t->gStackValues[t->gNumStackValues-2] /= t->gStackValues[t->gNumStackValues-1];
                    t->gNumStackValues--;
                    /* Do not clear the stack */
                    break;
                  case 16: 	/* callothersubr */
                    {
                        tt_int32 otherNumber, PS_StackArgCount;
                        LOG_CMD( "callothersubr", t->gNumStackValues );

                        LOG_CMD("stack-top+1 = ", t->gStackValues[ t->gNumStackValues -0 ] );
                        LOG_CMD("stack-top-0 = ", t->gStackValues[ t->gNumStackValues -1 ] );
                        LOG_CMD("stack-top-1 = ", t->gStackValues[ t->gNumStackValues -2 ] );
                        LOG_CMD("stack-top-2 = ", t->gStackValues[ t->gNumStackValues -3 ] );
                        LOG_CMD("stack-top-3 = ", t->gStackValues[ t->gNumStackValues -4 ] );
                        LOG_CMD("stack-top-4 = ", t->gStackValues[ t->gNumStackValues -5 ] );

                        t->gNumStackValues--;
                        otherNumber = t->gStackValues[ t->gNumStackValues ]; /* arg1, arg2, # */
                        t->gNumStackValues--;
                        PS_StackArgCount = t->gStackValues[ t->gNumStackValues ];
                        switch ( otherNumber ) {

                          case 0:
                            /* end flex */
                            t->flexOn = false;
                            t->gNumStackValues -= 2; /* ppem, x, y */
                            break;/*****/
                          case 1:
                            /* start flex */
                            t->flexOn = true;
                            t->flexCount = 0;
                            glyph_StartLine( t->glyph, x, y );
                            break;/*****/
                          case 2:
                            if ( t->flexOn ) {
                                switch( t->flexCount ) {
                                  case 0:
                                    /* The flat mid-point, flex-dist = dist(point(0)-point(3))*/
                                    break; /*****/
                                  case 1:
                                    glyph_AddPoint( t->glyph, x, y, 0 );
                                    break; /*****/
                                  case 2:
                                    glyph_AddPoint( t->glyph, x, y, 0 );
                                    break; /*****/
                                  case 3:
                                    glyph_AddPoint( t->glyph, x, y, 1 );
                                    break; /*****/
                                  case 4:
                                    glyph_AddPoint( t->glyph, x, y, 0 );
                                    break; /*****/
                                  case 5:
                                    glyph_AddPoint( t->glyph, x, y, 0 );
                                    break; /*****/
                                  case 6:
                                    glyph_AddPoint( t->glyph, x, y, 1 );
                                    break; /*****/
                                  default:
                                    assert( false );
                                    break; /*****/
                                }
                                t->flexCount++;
                            }
                            break;/*****/
                          case 3:
                            /* Hint replacement mechanism */
                            /* t->gNumStackValues--; */
                            t->gNumStackValues = (short)(t->gNumStackValues - PS_StackArgCount);
                            /* assert( PS_StackArgCount == 1 ); */
                            break;/*****/
                            /* 12 Counter Control 12, clear stack */
                            /* 13 Counter Control 13, clear stack */
                          case 12:
                          case 13:
                            t->gNumStackValues = 0;
                            break;/*****/
                            /* 14-18 relates to MM fonts */
                          case 14: /* Maps numMasters * 1 values to 1 value */
                          case 15: /* Maps numMasters * 2 values to 2 values */
                          case 16: /* Maps numMasters * 3 values to 3 values */
                          case 17: /* Maps numMasters * 4 values to 4 values */
                          case 18: /* Maps numMasters * 6 values to 6 values */
                            {
                                tt_int32 dNum, *data, N, loop;
                                F16Dot16 result;

                                N = otherNumber == 18 ? 6 : otherNumber - 13; /* N = [1,2,3,4,6] */

                                assert( PS_StackArgCount == t->numMasters * N );
                                t->gNumStackValues = (short)(t->gNumStackValues - t->numMasters * N);
                                if ( t->gNumStackValues < 0 ) break;

                                data = &t->gStackValues[ t->gNumStackValues + N ];
                                for ( loop = 0; loop < N; loop++ ) {
                                    result = t->gStackValues[ t->gNumStackValues ];
                                    for ( dNum = 1; dNum < t->numMasters; dNum++ ) {
                                        result += util_FixMul(*data++, t->WeightVector[dNum] );
                                    }
                                    t->gStackValues[ t->gNumStackValues++ ] = result;
                                }
                                t->gNumStackValues = (short)(t->gNumStackValues - N);
                            }
                            break;/*****/
                          default:
                            /* UnKnown */
                            t->gNumStackValues = (short)(t->gNumStackValues - PS_StackArgCount);
                            LOG_CMD("* * * * * UnKnown callothersubr number = ", otherNumber );
                            assert( false );
                            break;/*****/
                        }
                        if ( t->gNumStackValues < 0 ) t->gNumStackValues = 0;
                        LOG_CMD("* * * * * callothersubr number = ", otherNumber );
                        LOG_CMD("* * * * * x = ", x );
                        LOG_CMD("* * * * * y = ", y );
                        /* calls the PS interpreter (!) */
                    }
                    /* Do not clear the stack */
                    /*t->gNumStackValues = 0; */
                    break;
                  case 17: 	/* pop */
                    LOG_CMD( "pop", t->gNumStackValues );
                    /* From the PS interpreter */
#ifdef OLD
                    t->gStackValues[ t->gNumStackValues ] = 3;
#endif
                    t->gNumStackValues++; /* just pass on arguments from callothersubr */
                    /* Do not clear the stack */
                    break;
                  case 33: 	/* setcurrentpoint */
                    LOG_CMD( "setcurrentpoint", t->gNumStackValues );
                    if ( t->gNumStackValues < 2 ) break;
                    /*
						...Only used with othersubr, => skip it...
						x = t->gStackValues[ STACK_BOTTOM + 0 ];
						y = t->gStackValues[ STACK_BOTTOM + 1 ];
						*/
                    glyph_StartLine( t->glyph, x, y );
                    t->gNumStackValues = 0; /* clear the stack */
                    break;
                  default:
                    break;
                }
                break;
              case 13:	/* hsbw */
                {
                    tt_int32 lsbx, lsby, awx, awy;
                    LOG_CMD( "hsbw", t->gNumStackValues );
                    if ( t->gNumStackValues < 2 ) break;
                    lsbx = t->gStackValues[ STACK_BOTTOM + 0 ];
                    lsby = 0;
                    awx  = t->gStackValues[ STACK_BOTTOM + 1 ];
                    awy  = 0;
                    x = lsbx;
                    y = lsby;
                    SetMetrics( t, lsbx, lsby, awx, awy );
                    t->gNumStackValues = 0; /* clear the stack */
                }
                break;
              case 14:	/* endchar */
                /* endchar is the last command for normal characters and seac is the last one for accented characters */
                LOG_CMD( "endchar", t->gNumStackValues );
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case 15:
                break;
              case 16:
                break;
              case 17:
                break;
              case 18:
                break;
              case 19:
                break;
              case 20:
                break;
              case 21:	/* rmoveto */
                LOG_CMD( "rmoveto", t->gNumStackValues );
                if ( t->gNumStackValues < 2 ) break;
                x += t->gStackValues[ STACK_BOTTOM + 0 ];
                y += t->gStackValues[ STACK_BOTTOM + 1 ];
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case 22:	/* hmoveto */
                LOG_CMD( "hmoveto", t->gNumStackValues );
                if ( t->gNumStackValues < 1 ) break;
                x += t->gStackValues[ STACK_BOTTOM + 0 ];
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case 23:
                break;
              case 24:
                break;
              case 25:
                break;
              case 26:
                break;
              case 27:
                break;
              case 28:
                break;
              case 29:
                break;
              case 30:	/* vhcurveto */
                LOG_CMD( "vhcurveto", t->gNumStackValues );
                if ( t->gNumStackValues < 4 ) break;
                glyph_StartLine( t->glyph, x, y );
                x += 0;
                y += t->gStackValues[ STACK_BOTTOM + 0 ];
                glyph_AddPoint( t->glyph, x, y, 0 );
                x += t->gStackValues[ STACK_BOTTOM + 1 ];
                y += t->gStackValues[ STACK_BOTTOM + 2 ];
                glyph_AddPoint( t->glyph, x, y, 0 );
                x += t->gStackValues[ STACK_BOTTOM + 3 ];
                y += 0;
                glyph_AddPoint( t->glyph, x, y, 1 );
                t->gNumStackValues = 0; /* clear the stack */
                break;
              case 31:	/* hvcurveto */
                LOG_CMD( "hvcurveto", t->gNumStackValues );
                if ( t->gNumStackValues < 4 ) break;
                glyph_StartLine( t->glyph, x, y );
                x += t->gStackValues[ STACK_BOTTOM + 0 ];
                y += 0;
                glyph_AddPoint( t->glyph, x, y, 0 );
                x += t->gStackValues[ STACK_BOTTOM + 1 ];
                y += t->gStackValues[ STACK_BOTTOM + 2 ];
                glyph_AddPoint( t->glyph, x, y, 0 );
                x += 0;
                y += t->gStackValues[ STACK_BOTTOM + 3 ];
                glyph_AddPoint( t->glyph, x, y, 1 );
                t->gNumStackValues = 0; /* clear the stack */
                break;
            }
        } else {
            /* >= 32 => A number */
            if ( v1 <= 246 ) {
                num = v1 - 139; 					/* 1 byte: [-107, +107] */
            } else if ( v1 <= 250 ) {
                v2 = p[i++];
                num = ((v1-247)*256) + v2 + 108;	/* 2 bytes: [108, 1131] */
            } else if ( v1 <= 254 ) {
                v2 = p[i++];
                num = -((v1-251)*256) - v2 - 108;	/* 2 bytes: [-108, -1131] */
            } else {
                /* v1 == 255 */					 	/* 5 bytes: +-32 bit signed number  */
                num = p[i++];
                num <<= 8;
                num |= p[i++];
                num <<= 8;
                num |= p[i++];
                num <<= 8;
                num |= p[i++];
            }
#ifdef OLDDBUG
            if ( debugOn ) {
                printf("stack: ");
            }
#endif
            if ( t->gNumStackValues < kMaxStackValues ) {
                t->gStackValues[t->gNumStackValues++] = num;
            }
#ifdef OLDDBUG
            if ( debugOn ) {
                int k;

                for ( k = 0; k < t->gNumStackValues; k++ ) {
                    printf(" %d ", t->gStackValues[k] );
                }
                printf("\n");
            }
#endif

        }
    }
    LOG_CMD( "RETURNING FROM Type1BuildChar", t->gNumStackValues );
    t->x = x;
    t->y = y;

}

/*
 *
 */

GlyphClass *tsi_T1GetGlyphByIndex( T1Class *t, tt_uint16 index, tt_uint16 *aw )
{
    int byteCount, limit = t->NumCharStrings;
    tt_uint8 *p;
    GlyphClass *glyph = NULL;

    t->glyph = New_EmptyGlyph( t->mem, 0, 0 );
    t->glyph->curveType = 3;
    t->gNumStackValues = 0;

    if ( (tt_uint16)index < limit ) {
        p = t->charData[index];
        if ( p != NULL ) {
            byteCount = backwardsATOI( (char *)( p-5 ) );
            t->x = 0;
            t->y = 0;
            t->flexOn = false;
            LOG_CMD( "tsi_T1GetGlyphByIndex:", index );
            Type1BuildChar( t, p + t->lenIV, byteCount - t->lenIV, 0 );
            if ( t->glyph->contourCount == 0 ) {
                glyph_CloseContour( t->glyph );
            }
        }
    }
    glyph = t->glyph;

    glyph->ooy[glyph->pointCount + 0] = 0;
    glyph->oox[glyph->pointCount + 0] = 0;

    glyph->ooy[glyph->pointCount + 1] = (short)t->awy;
    glyph->oox[glyph->pointCount + 1] = (short)t->awx;

    *aw = (tt_uint16)t->awx;
    t->glyph = NULL;
    FlipContourDirection( glyph );
    return glyph; /*****/
}


/*
 *
 */
GlyphClass *tsi_T1GetGlyphByCharCode( T1Class *t, tt_uint32 charCode, tt_uint16 *aw )
{
    tt_uint16 index = tsi_T1GetGlyphIndex( t, charCode );
    return tsi_T1GetGlyphByIndex( t, index, aw ); /*****/
}


/*
 *
 */
tt_int32 tsi_T1GetParam( T1Class *t, const tt_uint8 *param, tt_int32 defaultValue )
{
    register tt_uint8 *start;
    tt_int32 result = defaultValue;

    start = tsi_T1Find( t, param, 0, t->charstringsGO );

    if ( start != 0 ) {
        result = ATOI( start );
    }
    return result; /*****/
}

/*
 *
 */
F16Dot16 tsi_T1GetFixedParam( T1Class *t, const tt_uint8 *param, F16Dot16 defaultValue )
{
    register tt_uint8 *start;
    F16Dot16 result = defaultValue;

    start = tsi_T1Find( t, param, 0, t->charstringsGO );

    if ( start != 0 ) {
        result = ATOFixed( start, 0 );
    }
    return result; /*****/
}

/*
 * returns numDimension
 */
static F16Dot16 tsi_T1GetFixedArray( T1Class *t, const tt_uint8 *param, int maxDim, F16Dot16 *arr )
{
    register tt_uint8 *start, *p;
    tt_int32 count, limit, numDimension = 0;

    start = tsi_T1Find( t, param, 0, t->charstringsGO );

    /* /WeightVector [0.17 0.08 0.52 0.23 ] def */

    if ( (p = start) != NULL ) {
        count = 0; limit = 32;
        for ( ; *p != '[' && count < limit; p++ ) count++; 	p++; 	/* skip the '[' */
        limit = 512;
        for ( numDimension = 0; numDimension < maxDim; ) {
            for ( ; *p == ' ' && count < limit; p++ ) count++; 	/* skip spaces */
            if ( *p == ']' ) break; /***** We are Done *****/
            arr[numDimension++] = ATOFixed( p, 0 );
            for ( ; *p != ' ' && count < limit; p++ ) count++; 	/* skip the number */
        }
    }
    return numDimension; /*****/
}


/*
 *
 */
static void GetT1FontMatrix( T1Class *t )
{
    register tt_uint8 *start, *p;
    tt_int32 count, limit;

    t->m00 = ONE16Dot16;	t->m01 = 0;				/* Oblique */
    t->m10 = 0;				t->m11 = ONE16Dot16;

    /* /FontMatrix [0.001000 0 .185339E-3 0.001 0 0] readonly def */
    /* /FontMatrix [0.001 0 0 0.001 0 0] readonly def */
    start = tsi_T1Find( t, (const tt_uint8 *)"/FontMatrix ", 0, t->charstringsGO );
    if ( (p = start) != NULL ) {
        count = 0; limit = 256;
        for ( ; *p != '[' && count < limit; p++ ) count++; 	p++; 	/* skip the '[' */
        for ( ; *p == ' ' && count < limit; p++ ) count++; 	/* skip spaces */
        for ( ; *p != ' ' && count < limit; p++ ) count++; 	/* skip number 1 */
        for ( ; *p == ' ' && count < limit; p++ ) count++; 	/* skip spaces */
        for ( ; *p != ' ' && count < limit; p++ ) count++; 	/* skip number 2 */
        for ( ; *p == ' ' && count < limit; p++ ) count++; 	/* skip spaces */
        /* now we point at number 3 */
        if ( count < limit ) {
            t->m01 = ATOFixed( p, 3 ); /* Scale up by a factor of 10^3 == 1000 */
        }
    }
}





static void BuildMetricsEtc( T1Class *t )
{
    tt_uint16 gIndex;
    GlyphClass *glyph;
    tt_uint16 aw;
    tt_int32 maxAW;

    tt_uint16 glyphIndexM = tsi_T1GetGlyphIndex( t, (tt_uint16)'M' ); /* Used to be 'f'... */
    tt_uint16 glyphIndexg = tsi_T1GetGlyphIndex( t, (tt_uint16)'g' );

    t->numAxes	  = 0;
    t->numMasters = tsi_T1GetFixedArray( t, (unsigned char *)"/WeightVector ", T1_MAX_MASTERS, t->WeightVector );

#ifdef OLD
    if ( t->numMasters != 0 ) {
        int i;
        for ( i = 0; i < t->numMasters; i++ ) {
            printf("WeightVector[%d] = %f\n", i, (double)t->WeightVector[i]/65536.0 );
        }
    }
#endif

    t->upem = tsi_T1GetParam( t, (tt_uint8 *)"/em ", 1000 );
    t->maxPointCount = 0;
    t->ascent = tsi_T1GetParam( t, (tt_uint8 *)"/ascent ", 0x7fff );
    t->descent = -tsi_T1GetParam( t, (tt_uint8 *)"/descent ", -0x7fff );
    GetT1FontMatrix( t );
    t->italicAngle = tsi_T1GetFixedParam( t, (tt_uint8 *)"/ItalicAngle ", 0 );
    t->hmtx = New_hmtxEmptyClass( t->mem, t->NumCharStrings, t->NumCharStrings );
    t->noDelete_hmtx = t->hmtx; /* Initialize our no delete reference */
    for ( gIndex = 0; gIndex < t->NumCharStrings; gIndex++ ) {
        t->hmtx->lsb[gIndex] = 0; /* The seac command needs this preinitialized */
    }

    maxAW = 0;
    for ( gIndex = 0; gIndex < t->NumCharStrings; gIndex++ ) {
        glyph = tsi_T1GetGlyphByIndex( t, gIndex, &aw);
        if ( t->ascent == 0x7fff && gIndex == glyphIndexM ) {
            t->ascent = GetGlyphYMax( glyph );
        }
        if ( t->descent == 0x7fff && gIndex == glyphIndexg ) {
            t->descent = GetGlyphYMin( glyph );
        }
        t->hmtx->aw[gIndex]  = (tt_uint16)(t->awx);
        if ( t->awx > maxAW ) {
            maxAW = t->awx;
        }
        t->hmtx->lsb[gIndex] = (tt_int16)(t->lsbx);
        if ( glyph->pointCount > t->maxPointCount ) {
            t->maxPointCount = glyph->pointCount;
        }
        Delete_GlyphClass( glyph );
    }
    t->advanceWidthMax = maxAW;

    if ( t->ascent == 0x7fff )  t->ascent  =  750;
    if ( t->descent == 0x7fff ) t->descent = -250;
    t->lineGap = t->upem - (t->ascent - t->descent);
    if ( t->lineGap < 0 ) t->lineGap = 0;

}



static void BuildSubrs( T1Class *t )
{
    register tt_uint8 *start, *p, *pStartSub;
    int i, fnum, byteCount, limit = t->dataLen;

    t->numSubrs		= 0;
    t->subrsData	= NULL;

    start = tsi_T1Find( t, (tt_uint8 *)"/Subrs ", t->eexecGO, t->dataLen );
    if ( start == NULL ) return; /*****/
    t->numSubrs = ATOI( start );

    t->subrsData = (tt_uint8  **)tsi_AllocMem( t->mem, sizeof(tt_uint8  *) * t->numSubrs );
    for ( i = 0; i < t->numSubrs; i++ ) {
        t->subrsData[i] = NULL;
    }
    for ( p = start, i = 0; i < t->numSubrs; i++ ) {
        /* dup 0 15 RD .....  NP */
        /* ... */
        /* dup 15 9 RD .....  NP */
        /* They are not always consequtive */
        p = tsi_T1Find( t, (tt_uint8 *)"dup ", p - t->decryptedData, limit );
        fnum = ATOI( p );
        pStartSub = tsi_T1Find( t, (tt_uint8 *)"RD ", p - t->decryptedData, p - t->decryptedData + 16 );
        if ( pStartSub == NULL ) {
            pStartSub = tsi_T1Find( t, (tt_uint8 *)"-| ", p - t->decryptedData, p - t->decryptedData + 16 );
        }
        assert( pStartSub != NULL );
        p = pStartSub;

        byteCount = backwardsATOI((char *)(p - 5));
        assert( fnum >= 0 && fnum < t->numSubrs );
        t->subrsData[fnum] = p; /* byteCount	= backwardsATOI( t->subrsData[index]-5 ); */
        p += byteCount;
        /*
printf("%d : Subr, byteCount = %d\n", fnum, byteCount );
*/

    }
    /*****/
}



static void BuildCMAP( T1Class *t )
{
    register tt_uint8 *start, *name,  *p = t->decryptedData;
    register int index, i, j, byteCount, limit = t->dataLen;
    tt_uint8 c_0, c_1, c_2, c_3;
    tt_uint8 PSName[64];


    /* /Encoding StandardEncoding def */
    /* Or, Encoding # array etc.. */

    t->encoding = tsi_T1Find( t, (tt_uint8 *)"/Encoding ", 0, t->dataLen );
    start = tsi_T1Find( t, (tt_uint8 *)"/CharStrings ", t->eexecGO, t->dataLen );
    assert( start != NULL );
    t->charstringsGO = start - t->decryptedData;

    t->NumCharStrings	= ATOI( start );
    t->charData		= (tt_uint8 **)tsi_AllocMem( t->mem,
                                                     sizeof(tt_uint8  *) * t->NumCharStrings );
    t->unicodeToGITable = tsi_AllocMem( t->mem, sizeof(unicodeToGI) * MAPPING_HASHTABLE_SIZE );
    /* printf("t->NumCharStrings = %d\n", t->NumCharStrings ); */

    /* Initialize */
    for ( i = 0; i < t->NumCharStrings; i++ )
      t->charData[i] = NULL;

    for ( i = 0; i < MAPPING_HASHTABLE_SIZE; i++ )
      t->unicodeToGITable[i] = NULL;

    c_2 = c_1 = c_0 = 0;
    index = byteCount = 0;

    name = NULL;
    for ( i = start - t->decryptedData; i < limit; i++ ) {
        c_3 = c_2; c_2 = c_1; c_1 = c_0;
        c_0 = p[i];

        if ( byteCount ) {
            byteCount--;
            continue; /*****/
        }

        if ( c_0 == '/' ) {
            name = &p[i] + 1;
        } else if ( c_3 == ' ' && c_0 == ' ' && ((c_2 == 'R' && c_1 == 'D') || (c_2 == '-' && c_1 == '|')) ) {

            byteCount	= backwardsATOI( (char*)&p[i-4] );
            assert( index < t->NumCharStrings );

            for ( j = 0; *name != ' ' && j < 63; j++ ) {
                PSName[j] = *name++;
            }
            PSName[j] = 0; /* Zero terminate */
            assert( j < 64 );
            t->charData[index] = &p[i] + 1; /* byteCount	= backwardsATOI( t->charData[index]-5 ); */

            tsi_T1AddUnicodeToGIMapping( t, (char *)PSName, index );

            index++;
            /* Break to avoid out of bounds writes for corrupt font data */
            if ( index >= t->NumCharStrings ) break;
        }
    }
}



T1Class *tsi_NewT1Class( tsiMemObject *mem, tt_uint8 *data, tt_int32 dataLen )
{
    int errCode = 0;
    register T1Class *t = (T1Class *)tsi_AllocMem( mem, sizeof( T1Class ) );
    t->mem = mem;

    t->glyph = NULL;
    t->decryptedData = NULL; t->dataLen = 0;

    TransformData( t, transform_decrypt, data, dataLen, &errCode );
    if (errCode != 0) {
	tsi_DeAllocMem(t->mem, (char*)t);
	return NULL;
    }
    /* WriteDataToFile( "AvantGarBooObl.txt", t->decryptedData, (tt_uint32) dataLen ); */
    /* WriteDataToFile( "MyriaMM.txt", t->decryptedData, (tt_uint32) dataLen ); */
    /* WriteDataToFile( "times.txt", t->decryptedData, tt_uint32) t->dataLen ); */

    t->notdefGlyphIndex = 0;
    t->unicodeToGITable = NULL;
    t->charData		= NULL;
    t->subrsData	= NULL;
    t->hmtx             = NULL;

    t->charstringsGO = t->dataLen;
    t->eexecGO = 0;

    BuildCMAP( t );
    BuildSubrs( t );
    t->lenIV = (short)tsi_T1GetParam( t, (tt_uint8 *)"/lenIV ", 4 );
    BuildMetricsEtc( t );
    return t; /*****/
}

/* Recursively deallocates all charCode to glyphIndex hashtable entries.

   NOTE:
   As of now, T1Entry inside t2kScalerContext
   (where the parent of this T1Class reside) never gets cleaned up,
   unless the removal of its entry in Strike cache takes place.
   Since the mapping table could have potentially large size,
   It would be best if this function could be called when Java exits */

void tsi_DeAllocChainedList( unicodeToGI *mappingNode ) {
    if ( mappingNode == NULL )
      return;
    else {
        tsi_DeAllocChainedList( mappingNode->next );
        free( (char *)mappingNode );
    }
}

void tsi_DeleteT1Class( T1Class *t )
{
    if ( t != NULL ) {
        int i;

        Delete_GlyphClass( t->glyph );
        if ( t->decryptedData != t->dataInPtr ) {
            tsi_DeAllocMem( t->mem, (char *)t->decryptedData );
        }
        tsi_DeAllocMem( t->mem, (char *)t->charData );
        tsi_DeAllocMem( t->mem, (char *)t->subrsData );
        Delete_hmtxClass( t->hmtx );

        for ( i = 0; i < MAPPING_HASHTABLE_SIZE; i++ )
          tsi_DeAllocChainedList( t->unicodeToGITable[i] );

        tsi_DeAllocMem( t->mem, (char *)t->unicodeToGITable );
        tsi_DeAllocMem( t->mem, (char *)t );
    }
}

#endif /* ENABLE_T1 */

#ifdef ENABLE_CFF





static tt_uint32 ReadOfffset1( InputStream *in )
{
    return ReadUnsignedByteMacro( in ); /*****/
}

static tt_uint32 ReadOfffset2( InputStream *in )
{
    tt_uint32 offset = ReadUnsignedByteMacro( in );
    offset      <<= 8;
    offset       |= ReadUnsignedByteMacro( in );
    return offset; /*****/
}

static tt_uint32 ReadOfffset3( InputStream *in )
{
    tt_uint32 offset = ReadUnsignedByteMacro( in );
    offset      <<= 8;
    offset       |= ReadUnsignedByteMacro( in );
    offset      <<= 8;
    offset       |= ReadUnsignedByteMacro( in );
    return offset; /*****/
}

static tt_uint32 ReadOfffset4( InputStream *in )
{
    tt_uint32 offset = ReadUnsignedByteMacro( in );
    offset      <<= 8;
    offset       |= ReadUnsignedByteMacro( in );
    offset      <<= 8;
    offset       |= ReadUnsignedByteMacro( in );
    offset      <<= 8;
    offset       |= ReadUnsignedByteMacro( in );
    return offset; /*****/
}

typedef tt_uint32 (*PF_READ_OFFSET) ( InputStream *in );

static PF_READ_OFFSET GetOffsetFunction( OffSize offSize )
{
    PF_READ_OFFSET f;

    assert( offSize >= 1 && offSize <= 4 );
    if ( offSize == 1 ) {
        f = ReadOfffset1;
    } else if ( offSize == 2 ) {
        f = ReadOfffset2;
    } else if ( offSize == 3 ) {
        f = ReadOfffset3;
    } else {
        f = ReadOfffset4;
    }

    return f;/*****/
}

#define READ_SID(in) ((SID)ReadInt16( in ))
#define IS_OPERATOR( v1 )	(v1 <= 27 || v1 == 31 )
#define IS_NUMBER( v1 )		(v1 >  27 && v1 != 31 )

static F16Dot16 READ_REAL( /* int v1, */ InputStream *in )
{
    unsigned char nibble1, nibble2;
    F16Dot16 value = 0;

    /* assert( v1 == 30 ); */

    do {
        nibble1 = ReadUnsignedByteMacro( in );
        nibble2 = (tt_uint8)(nibble1 & 0x0f);
        nibble1 = (tt_uint8)(nibble1 & 0x0f);
        /* parse it... */

    } while ( nibble1 != 0x0f && nibble2 != 0x0f );
    return value; /*****/
}


static tt_int32 READ_INTEGER( int v1, InputStream *in )
{
    tt_int32 num = 0;
    int v2;

    if ( v1 == 28 ) {
        num  = ReadUnsignedByteMacro( in );
        num <<= 8;
        num |= ReadUnsignedByteMacro( in );
    } else if ( v1 == 29 ) {
        num  = ReadUnsignedByteMacro( in );
        num <<= 8;
        num |= ReadUnsignedByteMacro( in );
        num <<= 8;
        num |= ReadUnsignedByteMacro( in );
        num <<= 8;
        num |= ReadUnsignedByteMacro( in );
    } else {
        assert( v1 >= 32 );

        if ( v1 <= 246 ) {
            num = v1 - 139; 					/* 1 byte: [-107, +107] */
        } else if ( v1 <= 250 ) {
            v2 = ReadUnsignedByteMacro( in );
            num = ((v1-247)*256) + v2 + 108;	/* 2 bytes: [108, 1131] */
        } else if ( v1 <= 254 ) {
            v2 = ReadUnsignedByteMacro( in );
            num = -((v1-251)*256) - v2 - 108;	/* 2 bytes: [-108, -1131] */
        } else {
            assert( false );
        }
    }
    return num; /*****/
}


static CFFIndexClass *tsi_NewCFFIndexClass( tsiMemObject *mem, InputStream *in )
{
    register CFFIndexClass *t = (CFFIndexClass *)tsi_AllocMem( mem, sizeof( CFFIndexClass ) );
    PF_READ_OFFSET ReadOfffset;
    tt_int32 i, max;

    t->mem 			= mem;
    t->offsetArray	= NULL;

    t->count 		= (Card16)ReadInt16( in );
    if ( t->count != 0 ) {
        t->offSize 		= ReadUnsignedByteMacro( in );
        t->offsetArray	= (tt_uint32 *)tsi_AllocMem( mem, sizeof( tt_uint32 ) * (t->count + 1) );
        ReadOfffset		= GetOffsetFunction( t->offSize );

        max = t->count; /* [0 .. t->count] */
        for ( i = 0; i <= max; i++ ) {
            t->offsetArray[i] = ReadOfffset( in );
        }
        /* Here is the data */
        t->baseDataOffset = Tell_InputStream( in ) - 1;
        /* offsetArray[0] == 1 */

        /* Skip past data. */
        Seek_InputStream( in, t->baseDataOffset + t->offsetArray[t->count] );
    }
    return t; /*****/
}

static void tsi_DeleteCFFIndexClass( CFFIndexClass *t )
{
    if ( t != NULL ) {
        tsi_DeAllocMem( t->mem, (char *)t->offsetArray );
        tsi_DeAllocMem( t->mem, (char *)t );
    }
}

static void tsi_ParsePrivateDictData( CFFClass *t )
{
    InputStream *in = t->in;
    tt_uint32 pos, limit, savepos;
    tt_int32 stack[ CFF_MAX_STACK ];
    tt_int32 number, stackCount = 0;


    savepos = Tell_InputStream( in );

    /* Set default values */
    t->privateDictData.Subr = 0;
    t->privateDictData.SubrOffset = 0;
    t->privateDictData.defaultWidthX = 0;
    t->privateDictData.nominalWidthX = 0;

    Seek_InputStream( in, t->topDictData.PrivateDictOffset );
    limit = t->topDictData.PrivateDictOffset + t->topDictData.PrivateDictSize;
    while ( (pos = Tell_InputStream( in )) < limit ) {
        tt_uint8 v1, v2;

        v1 = ReadUnsignedByteMacro( in );
        if ( IS_NUMBER( v1 ) ) {
            if ( v1 == 30 ) {
                number = READ_REAL( in );
            } else {
                number = READ_INTEGER( v1, in );
            }
            assert( stackCount < CFF_MAX_STACK );
            stack[ stackCount++ ] = number;
        } else {
            if ( v1 == 12 ) {
                v2 = ReadUnsignedByteMacro( in );
            } else {
                v2 = 0;
                switch ( v1 ) {
                  case 19:	t->privateDictData.Subr = stack[0];
                    break; /*****/
                  case 20:	t->privateDictData.defaultWidthX = stack[0];
                    break; /*****/
                  case 21:	t->privateDictData.nominalWidthX = stack[0];
                    break; /*****/
                  default:
                    break; /*****/
                }
            }
            stackCount = 0;
        }
    }
    if ( t->privateDictData.Subr != 0 ) {
        t->privateDictData.SubrOffset = t->topDictData.PrivateDictOffset + t->privateDictData.Subr;
    }
    Seek_InputStream( in, savepos );
}

static void tsi_ParseCFFTopDict( CFFIndexClass *t, InputStream *in, TopDictInfo *topDictData, tt_int32 n )
{
    tt_uint32 pos, limit, savepos;
    tt_int32 stack[ CFF_MAX_STACK ];
    tt_int32 number, stackCount = 0;


    savepos = Tell_InputStream( in );
    /* Set default values. */
    topDictData->bbox_xmax = 0; topDictData->bbox_xmin = 0;
    topDictData->bbox_ymax = 0; topDictData->bbox_ymin = 0;
    topDictData->italicAngle = 0;
    topDictData->m00 = ONE16Dot16;	topDictData->m01 = 0;				/* Oblique */
    topDictData->m10 = 0;			topDictData->m11 = ONE16Dot16;

    topDictData->charset 			= 0;
    topDictData->Encoding			= 0;
    topDictData->PrivateDictSize	= 0; topDictData->PrivateDictOffset	= 0;

    topDictData->numAxes			= 0;
    topDictData->numMasters			= 1;
    topDictData->lenBuildCharArray	= 0;
    topDictData->buildCharArray		= NULL;

    Seek_InputStream( in, t->baseDataOffset + t->offsetArray[n] );

    limit = t->baseDataOffset + t->offsetArray[n+1];
    while ( (pos = Tell_InputStream( in )) < limit ) {
        tt_uint8 v1, v2;

        v1 = ReadUnsignedByteMacro( in );
        if ( IS_NUMBER( v1 ) ) {
            if ( v1 == 30 ) {
                number = READ_REAL( in );
            } else {
                number = READ_INTEGER( v1, in );
            }
            assert( stackCount < CFF_MAX_STACK );
            stack[ stackCount++ ] = number;
        } else {
            if ( v1 == 12 ) {
                v2 = ReadUnsignedByteMacro( in );
                switch ( v2 ) {
                    int loop;

                  case 2: topDictData->italicAngle = stack[0];
                    break; /*****/
                  case 24: /* MultipleMaster */
                    topDictData->numAxes 	= stackCount - 4;
                    assert( topDictData->numAxes <= 16 );
                    topDictData->numMasters	= stack[0];
                    /* UDV Array, stack[1]..stack[1 + numAxes-1], == default instance */
                    for ( loop = 0; loop < topDictData->numAxes; loop ++ ) {
                        topDictData->defaultWeight[loop] = stack[1+loop];
                    }
                    topDictData->lenBuildCharArray = stack[1 + topDictData->numAxes + 0];
                    topDictData->buildCharArray	   = (F16Dot16 *)tsi_AllocMem( t->mem, topDictData->lenBuildCharArray * sizeof(F16Dot16) );
                    topDictData->NDV = (tt_uint16)stack[1 + topDictData->numAxes + 1];
                    topDictData->CDV = (tt_uint16)stack[1 + topDictData->numAxes + 2];


                  case 7:  /* FontMatrix */
                    topDictData->m00 = util_FixMul( stack[0], 1000L << 16 );
                    topDictData->m10 = util_FixMul( stack[0], 1000L << 16 );
                    topDictData->m01 = util_FixMul( stack[0], 1000L << 16 );
                    topDictData->m11 = util_FixMul( stack[0], 1000L << 16 );
                    break; /*****/
                  default:
                    break; /*****/
                }
            } else {
                v2 = 0;
                switch ( v1 ) {
                  case 0:		topDictData->version = (tt_uint16)stack[0];
                    break; /*****/
                  case 1:		topDictData->Notice = (tt_uint16)stack[0];
                    break; /*****/
                  case 2:		topDictData->FullName = (tt_uint16)stack[0];
                    break; /*****/
                  case 3:		topDictData->FamilyName = (tt_uint16)stack[0];
                    break; /*****/
                  case 4:		topDictData->Weight = (tt_uint16)stack[0];
                    break; /*****/
                  case 5:		topDictData->bbox_xmin = stack[0];
                    topDictData->bbox_ymin = stack[1];
                    topDictData->bbox_xmax = stack[2];
                    topDictData->bbox_ymax = stack[3];
                    break; /*****/
                  case 13:	topDictData->UniqueId = stack[0];
                    break; /*****/
                  case 15:	topDictData->charset = stack[0];
                    break; /*****/
                  case 16:	topDictData->Encoding = stack[0];
                    break; /*****/
                  case 17:	topDictData->Charstrings = stack[0];
                    break; /*****/
                  case 18:	topDictData->PrivateDictSize	= stack[0];
                    topDictData->PrivateDictOffset	= stack[1];
                    break; /*****/
                  default:
                    break; /*****/
                }
            }
            stackCount = 0;
        }
    }

    Seek_InputStream( in, savepos );
}

static void Type2BuildChar( CFFClass *t, InputStream *in, int byteCount, int nestingLevel )
{
    register int i, pos, v1, v2;
    tt_int32 endpos   = Tell_InputStream( in ) + byteCount;

    register F16Dot16 	x = t->x; /* cache x, and y in local variables */
    register F16Dot16 	y = t->y;
    register F16Dot16 	*gStackValues 	= t->gStackValues; /* Cache the "stack"-array in a local variable */
    register tt_int32 		gNumStackValues = t->gNumStackValues; /* Cache gNumStackValues */


    /* The group of operators that can participate in the width specification are:
	 * [ hstem,vstem,hstemhm,vstemhm,cntrmask,hintmask,hmoveto,vmoveto,rmoveto,endchar]
	 */

    while ( (i=Tell_InputStream( in )) < endpos  ) {
        v1 = ReadUnsignedByteMacro( in );
        if ( v1 < 32 ) {
            LOG_CMD( "cmd", v1 );
            LOG_CMD( "#stack", gNumStackValues );
            /* if ( debugOn ) printf("pointCount = %d, x,y = (%d,%d)\n", t->glyph->pointCount , x>>16, y>>16 ); */
            switch( v1 ) {
                /* 0 Reserved */
              case  1:	/* hstem */
              case  3:	/* vstem */
                LOG_CMD( (v1 == 1 ? "hstem" : "vstem"), gNumStackValues );
                if ( gNumStackValues < 2 ) break;
                /* takes 2*n arguments */
                pos = 0;
                if ( !t->widthDone && (gNumStackValues & 1) ) {
                    t->widthDone	= true;
                    t->awx 			= t->privateDictData.nominalWidthX + (gStackValues[ pos++ ]>>16);
                }
                t->numStemHints += (gNumStackValues-pos)/2;
                gNumStackValues = 0; /* clear the stack */
                break;
                /* 2 Reserved */
              case  4:	/* vmoveto */
                LOG_CMD( "vmoveto", gNumStackValues );
                pos = 0;
                if ( !t->widthDone && gNumStackValues > 1 ) {
                    t->widthDone	= true;
                    t->awx 			= t->privateDictData.nominalWidthX + (gStackValues[ pos++ ]>>16);
                }
                if ( gNumStackValues <= pos ) break;
                y += gStackValues[ pos++ ];
                gNumStackValues = 0; /* clear the stack */
                if ( t->pointAdded ) {
                    glyph_CloseContour( t->glyph );
                }
                break;
              case  5:	/* rlineto */
                LOG_CMD( "rlineto", gNumStackValues );
                if ( gNumStackValues < 2 ) break;
                glyph_StartLine( t->glyph, x>>16, y>>16 );
                for ( pos = 0; pos < gNumStackValues; ) {
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                }
                t->pointAdded   = 1;
                gNumStackValues = 0; /* clear the stack */
                break;
              case  6:	/* hlineto */
                LOG_CMD( "hlineto", gNumStackValues );
                if ( gNumStackValues < 1 ) break;
                glyph_StartLine( t->glyph, x>>16, y>>16 );
                for ( pos = 0; pos < gNumStackValues; ) {
                    x += gStackValues[ pos++ + 0 ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                    if ( pos >= gNumStackValues ) break; /*****/
                    y += gStackValues[ pos++ + 0 ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                }
                t->pointAdded   = 1;
                gNumStackValues = 0; /* clear the stack */
                break;
              case  7:	/* vlineto */
                LOG_CMD( "vlineto", gNumStackValues );
                if ( gNumStackValues < 1 ) break;
                glyph_StartLine( t->glyph, x>>16, y>>16 );
                for ( pos = 0; pos < gNumStackValues; ) {
                    y += gStackValues[ pos++ + 0 ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                    if ( pos >= gNumStackValues ) break; /*****/
                    x += gStackValues[ pos++ + 0 ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                }
                t->pointAdded   = 1;
                gNumStackValues = 0; /* clear the stack */
                break;
              case  8:	/* rrcurveto */
                LOG_CMD( "rrcurveto", gNumStackValues );
                if ( gNumStackValues < 6 || gNumStackValues % 6 != 0) break;
                glyph_StartLine( t->glyph, x>>16, y>>16 );
                for ( pos = 0; pos < gNumStackValues; ) {
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                }
                t->pointAdded   = 1;
                gNumStackValues = 0; /* clear the stack */
                break;
                /* 9 Reserved */
              case 29:	/* callgsubr */
              case 10:	/* callsubr */
                if ( gNumStackValues < 1 ) break;
                gNumStackValues -= 1;
                {
                    int fnum = gStackValues[ gNumStackValues + 0 ] >> 16; /* topmost element */
                    tt_int32 savepos;
                    int fByteCount;
                    CFFIndexClass *subr;

                    if ( v1 == 10 ) {
                        LOG_CMD( "callsubr", gNumStackValues );
                        fnum = fnum + t->lSubrBias;
                        subr = t->lSubr;
                    } else {
                        LOG_CMD( "callgsubr", gNumStackValues );
                        fnum = fnum + t->gSubrBias;
                        subr = t->gSubr;
                    }

                    if ( subr != NULL && fnum >= 0 && fnum < subr->count ) {

                        savepos = Tell_InputStream( in );
                        Seek_InputStream( t->in, subr->baseDataOffset + subr->offsetArray[ fnum ] );
                        LOG_CMD("***callsubr number =  ", fnum);
                        LOG_CMD("***callsubr fByteCount = ", fByteCount);
                        LOG_CMD("***BEGIN CALL = ", fnum );
                        fByteCount = subr->offsetArray[ fnum + 1 ] - subr->offsetArray[ fnum ];
                        if ( fByteCount > 0 && nestingLevel < 10 ) {
                            /* Save state from registers. */
                            t->x = x;
                            t->y = y;
                            t->gNumStackValues = gNumStackValues;
                            Type2BuildChar( t, in, fByteCount, nestingLevel+1 );
                            /* Cache state in registers. */
                            x = t->x;
                            y = t->y;
                            gNumStackValues = t->gNumStackValues;
                        }
                        LOG_CMD("***END CALL = ", fnum );
                        Seek_InputStream( t->in, savepos );
                    }
                    assert( gNumStackValues >= 0 );
                    assert( gNumStackValues <= CFF_MAX_STACK );
                }
                /* Do not clear the stack */
                /* gNumStackValues = 0; */
                break;
              case 11:	/* return  (from callsubr) */
                LOG_CMD( "return", gNumStackValues );
                /* Save state from registers. */
                t->x = x;
                t->y = y;
                t->gNumStackValues = gNumStackValues;
                return; /*****/
                /* Do not clear the stack */
              case 12:	/* escape */
                v2 = ReadUnsignedByteMacro( in );
                LOG_CMD( "v2", v2 );
                switch( v2 ) {
                    F16Dot16 dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4, dx5, dy5, dx6, dy6;
                  case 0: /* reserved but happens == old T1 dotsection */
                    gNumStackValues = 0; /* clear the stack */
                    break;

                    /* 0,1,2 are reserved */
                  case 3: 	/* and */
                    LOG_CMD( "and", gNumStackValues );
                    if ( gNumStackValues < 2 ) break;
                    gNumStackValues -= 2;

                    gStackValues[ gNumStackValues + 0 ] = 	gStackValues[ gNumStackValues + 0 ] &&
                      gStackValues[ gNumStackValues + 1 ] ? ONE16Dot16 : 0;
                    gNumStackValues++;
                    break;
                  case 4: 	/* or */
                    LOG_CMD( "or", gNumStackValues );
                    if ( gNumStackValues < 2 ) break;
                    gNumStackValues -= 2;

                    gStackValues[ gNumStackValues + 0 ] = 	gStackValues[ gNumStackValues + 0 ] ||
                      gStackValues[ gNumStackValues + 1 ] ? ONE16Dot16 : 0;
                    gNumStackValues++;
                    break;
                  case 5: 	/* not */
                    LOG_CMD( "not", gNumStackValues );
                    if ( gNumStackValues < 1 ) break;
                    gNumStackValues--;
                    gStackValues[ gNumStackValues + 0 ] =	gStackValues[ gNumStackValues + 0 ] == 0 ? ONE16Dot16 : 0;
                    gNumStackValues++;
                    break;
                    /* 6, 7 are reserved */
                  case 8: 	/* store */
                    LOG_CMD( "store", gNumStackValues );
                    if ( gNumStackValues < 4 ) break;
                    gNumStackValues -= 4;
                    assert( t->topDictData.buildCharArray != NULL );
                    {
                        int index, j, regItem, count, i1;

                        regItem	= gStackValues[ gNumStackValues + 0 ] >> 16;
                        j		= gStackValues[ gNumStackValues + 1 ] >> 16;
                        index	= gStackValues[ gNumStackValues + 2 ] >> 16;
                        count	= gStackValues[ gNumStackValues + 3 ] >> 16;

                        assert( index >= 0 && index < t->topDictData.lenBuildCharArray );
                        switch ( regItem ) {
                          case 0:
                            for ( i1 = 0; i1 < count; i1++ ) {
                                t->topDictData.reg_WeightVector[j + i1] = t->topDictData.buildCharArray[index + i1];
                            }
                            break;
                          case 1:
                            for ( i1 = 0; i1 < count; i1++ ) {
                                t->topDictData.reg_NormalizedDesignVector[j + i1] = t->topDictData.buildCharArray[index + i1];
                            }
                            break;
                          case 2:
                            for ( i1 = 0; i1 < count; i1++ ) {
                                t->topDictData.reg_UserDesignVector[j + i1] = t->topDictData.buildCharArray[index + i1];
                            }
                            break;

                          default:
                            break;

                        }
                    }
                    break;
                  case 9: 	/* abs */
                    LOG_CMD( "abs", gNumStackValues );
                    if ( gNumStackValues >= 1 ) {
                        F16Dot16 value;
                        gNumStackValues--;

                        value = gStackValues[ gNumStackValues + 0 ];
                        if ( value < 0 ) {
                            gStackValues[ gNumStackValues + 0 ] = -value;
                        }
                        gNumStackValues++;
                    }
                    break;
                  case 10: 	/* add */
                    LOG_CMD( "add", gNumStackValues );
                    if ( gNumStackValues < 2 ) break;
                    gNumStackValues -= 2;

                    gStackValues[ gNumStackValues + 0 ] = 	gStackValues[ gNumStackValues + 0 ] +
                      gStackValues[ gNumStackValues + 1 ];
                    gNumStackValues++;
                    break;
                  case 11: 	/* sub */
                    LOG_CMD( "sub", gNumStackValues );
                    if ( gNumStackValues < 2 ) break;
                    gNumStackValues -= 2;

                    gStackValues[ gNumStackValues + 0 ] =	gStackValues[ gNumStackValues + 0 ] -
                      gStackValues[ gNumStackValues + 1 ];
                    gNumStackValues++;
                    break;
                  case 12: 	/* div */
                    LOG_CMD( "div", gNumStackValues );
                    if ( gNumStackValues < 2 ) break;
                    gNumStackValues -= 2;

                    gStackValues[ gNumStackValues + 0 ] = util_FixDiv(	gStackValues[ gNumStackValues + 0 ],
                                                                        gStackValues[ gNumStackValues + 1 ] );
                    gNumStackValues++;
                    break;
                  case 13: 	/* load */
                    LOG_CMD( "load", gNumStackValues );
                    if ( gNumStackValues >= 3 && t->topDictData.buildCharArray != NULL) {
                        int index, regItem, count, i1;

                        gNumStackValues -= 3;

                        regItem	= gStackValues[ gNumStackValues + 0 ] >> 16;
                        index	= gStackValues[ gNumStackValues + 1 ] >> 16;
                        count	= gStackValues[ gNumStackValues + 2 ] >> 16;

                        assert( index >= 0 && index < t->topDictData.lenBuildCharArray );
                        switch ( regItem ) {
                          case 0:
                            for ( i1 = 0; i1 < count; i1++ ) {
                                t->topDictData.buildCharArray[index + i1] = t->topDictData.reg_WeightVector[i1];
                            }
                            break;
                          case 1:
                            for ( i1 = 0; i1 < count; i1++ ) {
                                t->topDictData.buildCharArray[index + i1] = t->topDictData.reg_NormalizedDesignVector[i1];
                            }
                            break;
                          case 2:
                            for ( i1 = 0; i1 < count; i1++ ) {
                                t->topDictData.buildCharArray[index + i1] = t->topDictData.reg_UserDesignVector[i1];
                            }
                            break;

                          default:
                            break;

                        }
                    }
                    break;


                  case 14: 	/* neg */
                    LOG_CMD( "neg", gNumStackValues );
                    if ( gNumStackValues < 1 ) break;
                    gNumStackValues--;
                    gStackValues[ gNumStackValues + 0 ] = -gStackValues[ gNumStackValues + 0 ];
                    gNumStackValues++;
                    break;
                  case 15: 	/* eq */
                    LOG_CMD( "eq", gNumStackValues );
                    if ( gNumStackValues < 2 ) break;
                    gNumStackValues -= 2;

                    gStackValues[ gNumStackValues + 0 ] = 	gStackValues[ gNumStackValues + 0 ] ==
                      gStackValues[ gNumStackValues + 1 ] ? ONE16Dot16 : 0;
                    gNumStackValues++;
                    break;

                    /* 16 is reserved */
                    /* 17 is not the pop operator and is reserved. */

                  case 18: 	/* drop */
                    LOG_CMD( "drop", gNumStackValues );
                    if ( gNumStackValues < 1 ) break;
                    gNumStackValues--;
                    break;

                    /* 19 is reserved */
                    /* t->topDictData.buildCharArray */
                  case 20: 	/* put */
                    LOG_CMD( "put", gNumStackValues );
                    if ( gNumStackValues >= 2 && t->topDictData.buildCharArray != NULL) {
                        F16Dot16 value;
                        int index;

                        gNumStackValues -= 2;
                        value = gStackValues[ gNumStackValues + 0 ];
                        index = gStackValues[ gNumStackValues + 1 ] >> 16;
                        assert( index >= 0 && index < t->topDictData.lenBuildCharArray );
                        t->topDictData.buildCharArray[index] = value;
                    }
                    break;

                  case 21: 	/* get */
                    LOG_CMD( "get", gNumStackValues );
                    if ( gNumStackValues >= 1 && t->topDictData.buildCharArray != NULL) {
                        F16Dot16 value;
                        int index;

                        gNumStackValues--;
                        index = gStackValues[ gNumStackValues + 0 ] >> 16;
                        assert( index >= 0 && index < t->topDictData.lenBuildCharArray );
                        value = t->topDictData.buildCharArray[index];
                        gStackValues[ gNumStackValues + 0 ] = value;
                    }
                    gNumStackValues++;
                    break;
                  case 22: 	/* ifelse */
                    LOG_CMD( "ifelse", gNumStackValues );
                    if ( gNumStackValues < 4 ) break;
                    gNumStackValues -= 4;

                    gStackValues[ gNumStackValues + 0 ] =	gStackValues[ gNumStackValues + 2 ] <=
                      gStackValues[ gNumStackValues + 3 ] ?
                        gStackValues[ gNumStackValues + 0 ] :
                    gStackValues[ gNumStackValues + 1 ];
                    gNumStackValues++;
                    break;

                  case 23: 	/* random */
                    LOG_CMD( "random", gNumStackValues );
                    if ( gNumStackValues >= 0 ) {
                        F16Dot16 value;

                        value   =	util_FixMul( gStackValues[ 0 ], gStackValues[ 1 ] ) ^
                          util_FixMul( gStackValues[ 2 ], gStackValues[ 3 ] );
                        value  ^= (~(gNumStackValues<<10) ^ gStackValues[ 4 ] );
                        t->seed = (tt_uint16)(58653 * t->seed + 13849);
                        value  ^= t->seed;
                        value  &= 0x0000ffff;
                        value++; /* greater than 0 and less than or equal to one */

                        gStackValues[ gNumStackValues + 0 ] = value;
                        gNumStackValues++;
                    }
                    break;
                  case 24: 	/* mul */
                    LOG_CMD( "mul", gNumStackValues );
                    assert( gNumStackValues >= 2 );
                    gNumStackValues -= 2;

                    gStackValues[ gNumStackValues + 0 ] = util_FixMul(	gStackValues[ gNumStackValues + 0 ],
                                                                        gStackValues[ gNumStackValues + 1 ] );
                    gNumStackValues++;
                    break;
                    /* 25 is reserved */
                  case 26: 	/* sqrt */
                    LOG_CMD( "sqrt", gNumStackValues );
                    if ( gNumStackValues < 1 ) break;
                    gNumStackValues--;
                    {
                        int loop = 0;
                        F16Dot16 square, root, old_root;
                        square	= gStackValues[ gNumStackValues + 0 ];
                        root 	= square;

                        /* A Newton Raphson loop */
                        do {
                            root = ((old_root = root) + util_FixDiv( square, root ) + 1 ) >> 1;
                        } while (old_root != root && loop++ < 10 );

                        gStackValues[ gNumStackValues + 0 ] = root;
                    }
                    gNumStackValues++;
                    break;

                  case 27: 	/* dup */
                    LOG_CMD( "dup", gNumStackValues );
                    if ( gNumStackValues < 1 ) break;
                    gNumStackValues--;

                    /* gStackValues[ gNumStackValues + 0 ] = gStackValues[ gNumStackValues + 0 ]; */
                    gStackValues[ gNumStackValues + 1 ] = gStackValues[ gNumStackValues + 0 ];

                    gNumStackValues += 2;
                    break;
                  case 28: 	/* exch */
                    LOG_CMD( "exch", gNumStackValues );
                    if ( gNumStackValues < 2 ) break;
                    gNumStackValues -= 2;
                    {
                        F16Dot16 val0, val1;

                        val0 = gStackValues[ gNumStackValues + 0 ];
                        val1 = gStackValues[ gNumStackValues + 1 ];
                        gStackValues[ gNumStackValues + 0 ] = val1;
                        gStackValues[ gNumStackValues + 1 ] = val0;
                    }

                    gNumStackValues += 2;
                    break;
                  case 29: 	/* index */
                    LOG_CMD( "index", gNumStackValues );
                    if ( gNumStackValues >= 2 ) {
                        tt_int32 index;

                        index = gStackValues[ gNumStackValues - 1 ] >> 16; /* top element */
                        if ( index < 0 ) {
                            index = 0;
                        } else if ( index > gNumStackValues - 2 ) {
                            index = gNumStackValues - 2; /* to avoid out of bounds access */
                        }
                        gStackValues[ gNumStackValues - 1 ] = gStackValues[ gNumStackValues - 2 - index ];
                    }
                    break;
                  case 30: 	/* roll */
                    LOG_CMD( "roll", gNumStackValues );
                    if ( gNumStackValues < 2 ) break;
                    gNumStackValues -= 2;

                    {
                        tt_int32 N, J, i1, i2, tmp;

                        N = gStackValues[ gNumStackValues + 0 ] >> 16;
                        if ( N < 0 ) N = 0;
                        J = gStackValues[ gNumStackValues + 1 ] >> 16;

                        /* numi == gStackValues[ gNumStackValues - 1 - i ]; */
                        if ( J >= 0 ) {
                            for ( i1 = 0; i1 < J; i1++ ) {
                                tmp = gStackValues[ gNumStackValues - 1 - 0];
                                for ( i2 = 1; i2 < N; i2++ ) {
                                    gStackValues[ gNumStackValues - 1 - (i2 - 1)] = gStackValues[ gNumStackValues - 1 - i2];
                                }
                                gStackValues[ gNumStackValues - 1 - (N-1)] = tmp;
                            }
                        } else {
                            J = -J;
                            for ( i1 = 0; i1 < J; i1++ ) {
                                tmp = gStackValues[ gNumStackValues - 1 - (N-1)];
                                for ( i2 = N-1; i2 > 0; i2-- ) {
                                    gStackValues[ gNumStackValues - 1 - i2] = gStackValues[ gNumStackValues - 1 - (i2 - 1)];
                                }
                                gStackValues[ gNumStackValues - 1 - 0] = tmp;
                            }
                        }

                    }
                    break;
                    /* 31 is reserved */
                    /* 32 is reserved */
                    /* 33 is reserved */

                  case 34: 	/* hflex */
                    LOG_CMD( "hflex", gNumStackValues );
                    if ( gNumStackValues < 7 ) break;
                    gNumStackValues -= 7;
                    glyph_StartLine( t->glyph, x>>16, y>>16 );
                    {

                        dx1 = gStackValues[ gNumStackValues + 0 ];
                        dy1 = 0;

                        dx2 = gStackValues[ gNumStackValues + 1 ];
                        dy2 = gStackValues[ gNumStackValues + 2 ];

                        dx3 = gStackValues[ gNumStackValues + 3 ];
                        dy3 = 0;

                        dx4 = gStackValues[ gNumStackValues + 4 ];
                        dy4 = 0;

                        dx5 = gStackValues[ gNumStackValues + 5 ];
                        dy5 = -dy2;

                        dx6 = gStackValues[ gNumStackValues + 6 ];
                        dy6 = 0;

                        x += dx1; y += dy1;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx2; y += dy2;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx3; y += dy3;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                        x += dx4; y += dy4;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx5; y += dy5;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx6; y += dy6;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                        t->pointAdded = 1;
                    }
                    break;
                  case 35: 	/* flex */
                    LOG_CMD( "flex", gNumStackValues );
                    if ( gNumStackValues < 13 ) break;
                    gNumStackValues -= 13;
                    glyph_StartLine( t->glyph, x>>16, y>>16 );
                    {

                        dx1 = gStackValues[ gNumStackValues + 0 ];
                        dy1 = gStackValues[ gNumStackValues + 1 ];

                        dx2 = gStackValues[ gNumStackValues + 2 ];
                        dy2 = gStackValues[ gNumStackValues + 3 ];

                        dx3 = gStackValues[ gNumStackValues + 4 ];
                        dy3 = gStackValues[ gNumStackValues + 5 ];

                        dx4 = gStackValues[ gNumStackValues + 6 ];
                        dy4 = gStackValues[ gNumStackValues + 7 ];

                        dx5 = gStackValues[ gNumStackValues + 8 ];
                        dy5 = gStackValues[ gNumStackValues + 9 ];

                        dx6 = gStackValues[ gNumStackValues + 10 ];
                        dy6 = gStackValues[ gNumStackValues + 11 ];
                        /* fd = gStackValues[ gNumStackValues + 12 ]; */

                        x += dx1; y += dy1;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx2; y += dy2;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx3; y += dy3;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                        x += dx4; y += dy4;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx5; y += dy5;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx6; y += dy6;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                        t->pointAdded = 1;
                    }
                    break;
                  case 36: 	/* hflex1 */
                    LOG_CMD( "hflex1", gNumStackValues );
                    if ( gNumStackValues < 9 ) break;
                    gNumStackValues -= 9;
                    glyph_StartLine( t->glyph, x>>16, y>>16 );
                    {

                        dx1 = gStackValues[ gNumStackValues + 0 ];
                        dy1 = gStackValues[ gNumStackValues + 1 ];

                        dx2 = gStackValues[ gNumStackValues + 2 ];
                        dy2 = gStackValues[ gNumStackValues + 3 ];

                        dx3 = gStackValues[ gNumStackValues + 4 ];
                        dy3 = 0;
                        /*---*/
                        dx4 = gStackValues[ gNumStackValues + 5 ];
                        dy4 = 0;

                        dx5 = gStackValues[ gNumStackValues + 6 ];
                        dy5 = gStackValues[ gNumStackValues + 7 ];

                        dx6 = gStackValues[ gNumStackValues + 8 ];
                        dy6 = -(dy1+dy2+dy5);

                        x += dx1; y += dy1;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx2; y += dy2;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx3; y += dy3;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                        x += dx4; y += dy4;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx5; y += dy5;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx6; y += dy6;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                        t->pointAdded = 1;
                    }
                    break;
                  case 37: 	/* flex1 */
                    LOG_CMD( "flex1", gNumStackValues );
                    if ( gNumStackValues < 11 ) break;
                    gNumStackValues -= 11;
                    glyph_StartLine( t->glyph, x>>16, y>>16 );
                    {
                        F16Dot16 sumDx, sumDy;
                        F16Dot16 sumDxAbs, sumDyAbs;

                        dx1 = gStackValues[ gNumStackValues + 0 ];
                        dy1 = gStackValues[ gNumStackValues + 1 ];

                        dx2 = gStackValues[ gNumStackValues + 2 ];
                        dy2 = gStackValues[ gNumStackValues + 3 ];

                        dx3 = gStackValues[ gNumStackValues + 4 ];
                        dy3 = gStackValues[ gNumStackValues + 5 ];

                        dx4 = gStackValues[ gNumStackValues + 6 ];
                        dy4 = gStackValues[ gNumStackValues + 7 ];

                        dx5 = gStackValues[ gNumStackValues + 8 ];
                        dy5 = gStackValues[ gNumStackValues + 9 ];

                        sumDx = dx1 + dx2 + dx3 + dx4 + dx5;
                        sumDy = dy1 + dy2 + dy3 + dy4 + dy5;
                        sumDxAbs = sumDx;
                        sumDyAbs = sumDy;
                        if ( sumDxAbs < 0 ) sumDxAbs = -sumDxAbs;
                        if ( sumDyAbs < 0 ) sumDyAbs = -sumDyAbs;


                        if ( sumDxAbs > sumDyAbs ) {
                            /* First and last y are equal */
                            dx6 = gStackValues[ gNumStackValues + 10 ];
                            dy6 = -sumDy;
                        } else {
                            /* First and last x are equal */
                            dx6 = -sumDx;
                            dy6 = gStackValues[ gNumStackValues + 10 ];
                        }

                        x += dx1; y += dy1;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx2; y += dy2;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx3; y += dy3;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                        x += dx4; y += dy4;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx5; y += dy5;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                        x += dx6; y += dy6;
                        glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                        t->pointAdded = 1;
                    }
                    break;
                    /* 38 - 255 is reserved */
                  default:
                    LOG_CMD( "12 - reserved", (int)v2);
                    assert( false );
                    break;
                }
                break;
                /* 13 is reserved */
              case 14:	/* endchar */
                /* endchar is the last command for normal characters and seac is the last one for accented characters */
                LOG_CMD( "endchar", gNumStackValues );
                /* Save state from registers. */
                t->x = x; /* set x, and y since this may mean return in a subroutine ! */
                t->y = y;
                t->gNumStackValues = gNumStackValues;
                pos = 0;
                if ( !t->widthDone && gNumStackValues > 0 ) {
                    t->widthDone	= true;
                    t->awx 			= t->privateDictData.nominalWidthX + (gStackValues[ pos++ ]>>16);
                }
                gNumStackValues = 0; /* clear the stack */
                break;
                /* 15 is reserved */
              case 16: 	/* blend */
                /* for k master designs produces n interpolated results value(s) from n*k arguments. */
                /* INPUT: k groups of n arguments, n */
                /* The values in the second a subsequent groups are expressed as deltas to the values in the first group. */
                LOG_CMD( "blend", gNumStackValues );
                if ( gNumStackValues >= 2 ) {
                    int k, n, i1, i2;

                    k = t->topDictData.numMasters;

                    gNumStackValues--;
                    n = gStackValues[ gNumStackValues + 0 ] >> 16;
                    gNumStackValues -= k * n;
                    assert( gNumStackValues >= 0 );

                    for ( i1 = 0; i1 < n; i1++ ) {
                        F16Dot16 value = gStackValues[ gNumStackValues + i1 + 0 ];
                        F16Dot16 weight;
                        for ( i2 = 1; i2 < k; i2++ ) {
                            weight = t->topDictData.defaultWeight[i2];
                            value += util_FixMul( weight, gStackValues[ gNumStackValues + i1 + i2*k ]  );
                        }
                        gStackValues[ gNumStackValues + i1 + 0 ] = value;
                    }
                    gNumStackValues += n;
                }
                break;
                /* 17 is reserved */

              case 18: /* hstemhm */
              case 23: /* vstemhm */
                LOG_CMD( (v1 == 18 ? "hstemhm" : "vstemhm"), gNumStackValues );
                /* takes 2*n arguments */
                pos = 0;
                if ( !t->widthDone && (gNumStackValues & 1) ) {
                    t->widthDone	= true;
                    t->awx 			= t->privateDictData.nominalWidthX + (gStackValues[ pos++ ]>>16);
                }
                t->numStemHints += (gNumStackValues-pos)/2;
                gNumStackValues = 0;
                break;
              case 19: /* hintmask */
              case 20: /* cntrmask */
                LOG_CMD( "hintmask/cntrmask", gNumStackValues );
                pos = 0;
                if ( !t->widthDone && gNumStackValues > 0 ) {
                    t->widthDone	= true;
                    t->awx 			= t->privateDictData.nominalWidthX + (gStackValues[ pos++ ]>>16);
                }
                while ( pos < gNumStackValues ) {
                    pos += 2; /* consume vstem hints */
                    t->numStemHints++;
                }
                /* Consume the mask multibyte sequence */
                {
                    int count = (t->numStemHints + 7) >> 3;

                    LOG_CMD( "t->numStemHints", t->numStemHints );
                    while ( count-- > 0 ) {
                        v2 = ReadUnsignedByteMacro( in );
                        LOG_CMD( "CONSUMED", (int)v2 );
                    }
                }

                gNumStackValues = 0;
                break;
              case 21:	/* rmoveto */
                LOG_CMD( "rmoveto", gNumStackValues );
                pos = 0;
                if ( !t->widthDone && gNumStackValues > 2 ) {
                    t->widthDone	= true;
                    t->awx 			= t->privateDictData.nominalWidthX + (gStackValues[ pos++ ]>>16);
                }
                if ( gNumStackValues - pos < 2 ) break;
                x += gStackValues[ pos++ ];
                y += gStackValues[ pos++ ];
                gNumStackValues = 0; /* clear the stack */
                if ( t->pointAdded ) {
                    glyph_CloseContour( t->glyph );
                }
                break;
              case 22:	/* hmoveto */
                LOG_CMD( "hmoveto", gNumStackValues );
                pos = 0;
                if ( !t->widthDone && gNumStackValues > 1 ) {
                    t->widthDone	= true;
                    t->awx 			= t->privateDictData.nominalWidthX + (gStackValues[ pos++ ]>>16);
                }
                if ( gNumStackValues - pos < 1 ) break;
                x += gStackValues[ pos++ ];
                gNumStackValues = 0; /* clear the stack */
                if ( t->pointAdded ) {
                    glyph_CloseContour( t->glyph );
                }
                break;
              case 24: /* rcurveline */
                LOG_CMD( "rcurveline", gNumStackValues );
                if ( gNumStackValues < 8 ) break;
                glyph_StartLine( t->glyph, x>>16, y>>16 );
                for ( pos = 0; pos+6 <= gNumStackValues; ) {
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                }
                x += gStackValues[ pos++ ];
                y += gStackValues[ pos++ ];
                glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                gNumStackValues = 0; /* clear the stack */
                t->pointAdded = 1;
                break;
              case 25: /* rlinecurve */
                LOG_CMD( "rlinecurve", gNumStackValues );
                if ( gNumStackValues < 8 ) break;
                glyph_StartLine( t->glyph, x>>16, y>>16 );

                for ( pos = 0; pos+6 < gNumStackValues; ) {
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                }

                x += gStackValues[ pos++ ];
                y += gStackValues[ pos++ ];
                glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                x += gStackValues[ pos++ ];
                y += gStackValues[ pos++ ];
                glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                x += gStackValues[ pos++ ];
                y += gStackValues[ pos++ ];
                glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );

                t->pointAdded   = 1;
                gNumStackValues = 0; /* clear the stack */
                break;
              case 26: /* vvcurveto */
                LOG_CMD( "vvcurveto", gNumStackValues );
                glyph_StartLine( t->glyph, x>>16, y>>16 );
                pos = 0;
                if ( gNumStackValues & 1 ) {
                    x += gStackValues[ pos++ ];
                }
                while ( pos+4 <= gNumStackValues ) {
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                }
                t->pointAdded   = 1;
                gNumStackValues = 0; /* clear the stack */
                break;
              case 27: /* hhcurveto */
                LOG_CMD( "hhcurveto", gNumStackValues );
                glyph_StartLine( t->glyph, x>>16, y>>16 );
                pos = 0;
                if ( gNumStackValues & 1 ) {
                    y += gStackValues[ pos++ ];
                }
                while ( pos+4 <= gNumStackValues ) {
                    x += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );
                }
                t->pointAdded   = 1;
                gNumStackValues = 0; /* clear the stack */
                break;
              case 28: /* shortint */
                {
                    register F16Dot16 num;

                    num = ReadUnsignedByteMacro( in );	/* 3 bytes 2 bytes 16.0 number */
                    num <<= 8;
                    num |= ReadUnsignedByteMacro( in );
                    num <<= 16;
                    if ( gNumStackValues < CFF_MAX_STACK ) {
                        gStackValues[gNumStackValues++] = num;
                    }
                }
                break;

                /* 29 see above around 10 */
              case 30:	/* vhcurveto */
                LOG_CMD( "vhcurveto", gNumStackValues );
                if ( gNumStackValues < 4 ) break;
                glyph_StartLine( t->glyph, x>>16, y>>16 );
                for ( pos = 0; pos+4 <= gNumStackValues; ) {
                    x += 0;
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += 0;
                    if ( pos + 1 == gNumStackValues ) {
                        y += gStackValues[ pos++ ];
                    }
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );

                    if ( pos+4 > gNumStackValues ) break; /*****/

                    x += gStackValues[ pos++ ];
                    y += 0;
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += 0;
                    y += gStackValues[ pos++ ];
                    if ( pos + 1 == gNumStackValues ) {
                        x += gStackValues[ pos++ ];
                    }
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );

                }
                t->pointAdded   = 1;
                gNumStackValues = 0; /* clear the stack */
                break;

              case 31:	/* hvcurveto */
                LOG_CMD( "hvcurveto", gNumStackValues );
                if ( gNumStackValues < 4 ) break;
                glyph_StartLine( t->glyph, x>>16, y>>16 );
                for ( pos = 0; pos+4 <= gNumStackValues; ) {
                    x += gStackValues[ pos++ ];
                    y += 0;
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += 0;
                    y += gStackValues[ pos++ ];
                    if ( pos + 1 == gNumStackValues ) {
                        x += gStackValues[ pos++ ];;
                    }
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );

                    if ( pos+4 > gNumStackValues ) break; /*****/

                    x += 0;
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += gStackValues[ pos++ ];
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 0 );
                    x += gStackValues[ pos++ ];
                    y += 0;
                    if ( pos + 1 == gNumStackValues ) {
                        y += gStackValues[ pos++ ];
                    }
                    glyph_AddPoint( t->glyph, x>>16, y>>16, 1 );

                }
                t->pointAdded   = 1;
                gNumStackValues = 0; /* clear the stack */
                break;

              default:
                LOG_CMD( "reserved cmd", (int)v1);
                assert( false );

            }
        } else {
            register F16Dot16 num = 0;

            /* v1 == 28 is already handled */
            if ( v1 <= 246 ) {
                /* >= 32 => A number */
                num = v1 - 139; 					/* 1 byte: [-107, +107] */
                num <<= 16;
            } else if ( v1 <= 250 ) {
                v2 = ReadUnsignedByteMacro( in );
                num = ((v1-247)*256) + v2 + 108;	/* 2 bytes: [108, 1131] */
                num <<= 16;
            } else if ( v1 <= 254 ) {
                v2 = ReadUnsignedByteMacro( in );
                num = -((v1-251)*256) - v2 - 108;	/* 2 bytes: [-108, -1131] */
                num <<= 16;
            } else {
                /* v1 == 255 */					 	/* 5 bytes: +-16.16 bit signed number  */
                num = ReadUnsignedByteMacro( in );
                num <<= 8;
                num |= ReadUnsignedByteMacro( in );
                num <<= 8;
                num |= ReadUnsignedByteMacro( in );
                num <<= 8;
                num |= ReadUnsignedByteMacro( in );
            }
#ifdef OLDDBUG
            if ( debugOn ) {
                printf("stack: ");
            }
#endif
            if ( gNumStackValues < CFF_MAX_STACK ) {
                gStackValues[gNumStackValues++] = num;
            }
        }
    }
    /* Save state from registers. */
    t->x = x;
    t->y = y;
    t->gNumStackValues = gNumStackValues;
}




tt_uint16 tsi_T2GetGlyphIndex( CFFClass *t, tt_uint32 charCode )
{
    tt_int32 i;
    SID stringID;
    tt_uint16 gIndex = 0;

    {
        assert( charCode < 256 );
        stringID = t->charCodeToSID[charCode];

        if ( stringID < sidLimit ) {
            gIndex = t->SIDToGIndex[stringID];
        } else {
            register SID *gIndexToSID = t->gIndexToSID;

            for ( i = 0; i < t->NumCharStrings; i++ ) {
                if ( gIndexToSID[i] == stringID ) {
                    gIndex = (tt_uint16)i;
                    break; /*****/
                }

            }
        }
    }

    return gIndex; /*****/
}

static void BuildT2MetricsEtc( CFFClass *t )
{
    tt_uint16 gIndex;
    GlyphClass *glyph;
    tt_uint16 aw;
    tt_int32 maxAW;
    tt_uint16 fGIndex, gGIndex;

    t->NumCharStrings	= t->CharStrings->count;
    t->upem				= 1000;
    t->maxPointCount	= 0;
    t->ascent			= 0x7fff;
    t->descent			= 0x7fff;
    /* GetT1FontMatrix( t ); */
    t->italicAngle = t->topDictData.italicAngle;
    t->hmtx = New_hmtxEmptyClass( t->mem, t->NumCharStrings, t->NumCharStrings );

    maxAW = 0;

    fGIndex = tsi_T2GetGlyphIndex( t, 'f' );
    gGIndex = tsi_T2GetGlyphIndex( t, 'g' );
    for ( gIndex = 0; gIndex < t->NumCharStrings; gIndex++ ) {
        glyph = tsi_T2GetGlyphByIndex( t, gIndex, &aw);

        if ( t->ascent == 0x7fff && gIndex == fGIndex && fGIndex != 0 ) {
            t->ascent = GetGlyphYMax( glyph );
        }
        if ( t->descent == 0x7fff && gIndex == gGIndex && gGIndex != 0 ) {
            t->descent = GetGlyphYMin( glyph );
        }

        t->hmtx->aw[gIndex]  = (tt_uint16)(t->awx);
        if ( t->awx > maxAW ) {
            maxAW = t->awx;
        }
        t->hmtx->lsb[gIndex] = (tt_int16)(t->lsbx);
        if ( glyph->pointCount > t->maxPointCount ) {
            t->maxPointCount = glyph->pointCount;
        }
        Delete_GlyphClass( glyph );
    }
    t->advanceWidthMax = maxAW;

    if ( t->ascent == 0x7fff )  t->ascent  =  750;
    if ( t->descent == 0x7fff ) t->descent = -250;
    t->lineGap = t->upem - (t->ascent - t->descent);
    if ( t->lineGap < 0 ) t->lineGap = 0;
}

/*** Begin Predefined Charsets ***/
/* Maps gIndex -> SID */

static SID ISOAdobeSID[] = {1,228,
    0, 0};

static SID ExpertSID[] 	 = {1,	1,
    229,238,
    13,15,
    99,99,
    239,248,
    27,28,
    249,266,
    109,110,
    267,318,
    158,158,
    155,155,
    163,163,
    319,326,
    150,150,
    164,164,
    169,169,
    327,378,
    0, 	0};

static SID ExpertSubsetSID[] = {	1,	1,
    231, 232,
    235, 238,
    13, 15,
    99, 99,
    239, 248,
    27, 28,
    249, 251,
    253, 266,
    109, 110,
    267, 270,
    272, 272,
    300, 302,
    305, 305,
    314, 315,
    158, 158,
    155, 155,
    163, 163,
    320, 326,
    150, 150,
    164, 164,
    169, 169,
    327, 346,
    0, 	0};

/*** End Predefined Charsets ***/
/*** Begin Predefined Encodings ***/
/* Maps charcode to ->SID */
static tt_uint16 standarEncodingData[] = { 0,  31,  0,   0, 	/* charcode: 0-31    maps to the SID: range 0-0 */
    32, 126, 1,   95, 	/* charcode: 32-126  maps to the SID: range 1-95 */
    127,160, 0,   0, 	/* charcode: 127-160 maps to the SID: range 0-0 */
    161,175, 96,  110,	/* charcode: 127-160 maps to the SID: range 0-0 */
    176,176, 0,   0,
    177,180, 111, 114,
    181,181, 0,   0,
    182,189, 115, 122,
    190,190, 0,   0,
    191,191, 123, 123,
    192,192, 0,   0,
    193,200, 124, 131,
    201,201, 0,   0,
    202,203, 132, 133,
    204,204, 0,   0,
    205,208, 134, 137,
    209,224, 0,   0,
    225,225, 138, 138,
    226,226, 0,   0,
    227,227, 139, 139,
    228,231, 0,   0,
    232,235, 140, 143,
    236,240, 0,   0,
    241,241, 144, 144,
    242,244, 0,   0,
    245,245, 145, 145,
    245,245, 145, 145,
    246,247, 0,    0,
    248,251, 146,  149,
    252,255, 0,    0 };

static tt_uint16 expertEncodingData[] = {   0, 31, 0,   0, 	/* charcode: 0-31    maps to the SID: range 0-0 */
    32, 32, 1,   1, 	/* charcode: 32-32   maps to the SID: range 1-1 */
    33, 34, 229, 230, 	/* charcode: 33-34   maps to the SID: range 229-230 */
    36, 43, 231, 238,
    44, 46,  13, 15,
    47, 47,  99, 99,
    48, 57, 239, 248,
    58, 59,  27,  28,
    60, 63,  249, 252,
    64, 64,    0,   0,
    65, 69,  253, 257,
    70, 72,    0,   0,
    73, 73,  258, 258,
    74, 75,    0,   0,
    76, 79,  259, 262,
    80, 81,    0,   0,
    82, 84,  263, 265,
    85, 85,    0,   0,
    86, 86,  266, 266,
    87, 88,  109, 110,
    89, 91,  267, 269,
    92, 92,    0,   0,
    93, 126, 270, 303,
    127, 160,   0,   0,
    161, 163, 304, 306,
    164, 165,   0,   0,
    166, 170, 307, 311,
    171, 171,   0,   0,
    172, 172, 312, 312,
    173, 174,   0,   0,
    175, 175, 313, 313,
    176, 177,   0,   0,
    178, 179, 314, 315,
    180, 181,   0,   0,
    182, 184, 316, 318,
    185, 187,   0,   0,
    188, 188, 158, 158,
    189, 189, 155, 155,
    190, 190, 163, 163,
    191, 197, 319, 325,
    198, 199,   0,   0,
    200, 200, 326, 326,
    201, 201, 150, 150,
    202, 202, 164, 164,
    203, 203, 169, 169,
    204, 255, 327, 378 };


/*** End Predefined Encodings ***/


static void BuildT2CMAP( CFFClass *t )
{
    tt_int32 i, j, k;
    tt_uint8 format, format7F;

    t->NumCharStrings	= t->CharStrings->count;

    t->gIndexToSID		= (SID *)tsi_AllocMem( t->mem, sizeof(SID) * t->NumCharStrings );

    /* gIndex To SID */
    if ( t->topDictData.charset < 3 ) {
        SID *range = NULL, first, last;


        /* A predefined charset */
        switch ( t->topDictData.charset ) {
          case 0:
            range = ISOAdobeSID;		/* 0 = ISOAdobe */
            break;
          case 1:
            range = ExpertSID;			/* 1 = Expert */
            break;
          case 2:
            range = ExpertSubsetSID;	/* 2 = ExpertSubset */
            break;
        }
        i = 0;
        t->gIndexToSID[i++] = 0; /* notdef */
        for ( k = 0; i < t->NumCharStrings; k += 2 ) {
            first = range[k];
            last  = range[k+1];
            assert( last >= first );
            if ( first == 0 && last == 0 ) break; /*****/
            for ( j = first; j <= last && i < t->NumCharStrings; j++ ) {
                t->gIndexToSID[i++] = (tt_uint16)j;
            }
        }
    } else {
        Seek_InputStream( t->in, t->topDictData.charset );
        format = ReadUnsignedByteMacro( t->in );


        i = 0;
        t->gIndexToSID[i++] = 0; /* notdef */
        if ( format == 0 ) {
            while ( i < t->NumCharStrings ) {
                t->gIndexToSID[i++] = (tt_uint16)ReadUnsignedByteMacro( t->in );
            }
        } else if ( format == 1 || format == 2 ) {
            SID 	first;
            tt_uint16	nLeft; /* could be tt_uint8 or tt_uint16 */

            while ( i < t->NumCharStrings ) {
                first = (SID)READ_SID( t->in );
                nLeft = (tt_uint16)(format == 1 ? (tt_uint16)ReadUnsignedByteMacro( t->in ) : (tt_uint16)ReadInt16( t->in ));

                for ( j = 0; j <= nLeft && i < t->NumCharStrings ; j++ ) {
                    t->gIndexToSID[i++] = (tt_uint16)(first + j);
                }
            }
        } else {
            assert( false );
        }
    }

    /* ccode -> SID, or gIndex-> charcode */
    if ( t->topDictData.Encoding < 2 ) {
        tt_uint16 firstCode, lastCode;
        SID firstSID, lastSID;

        tt_uint16 *encodingData = t->topDictData.Encoding == 0 ? standarEncodingData : expertEncodingData;

        i = 0;
        do {
            firstCode = encodingData[i+0];
            lastCode  = encodingData[i+1];
            firstSID  = encodingData[i+2];
            lastSID   = encodingData[i+3]; i += 4;

            if ( firstSID == lastSID ) {
                for ( j = firstCode; j <= lastCode; j++ ) {
                    /* ccode j maps to SID firstSID */
                    t->charCodeToSID[j] = firstSID;
                }
            } else {
                assert( (lastCode-firstCode) == (lastSID-firstSID) );
                for ( k = 0, j = firstCode; j <= lastCode; j++, k++ ) {
                    /* ccode j maps to SID firstSID + k */
                    t->charCodeToSID[j] = (tt_uint16)(firstSID + k);
                }
            }

        } while ( lastCode < 255 );

    } else {
        Seek_InputStream( t->in, t->topDictData.Encoding );
        format = ReadUnsignedByteMacro( t->in );

        format7F = (tt_uint8)(format & 0x7f);



        if ( format7F == 0 ) {
            tt_uint8 nCodes, ccode;

            nCodes = ReadUnsignedByteMacro( t->in );
            for ( i = 0; i < nCodes; i++ ) {
                ccode = ReadUnsignedByteMacro( t->in );
                t->charCodeToSID[ccode] = t->gIndexToSID[i+1];
                /* t->gIndexToCharCode[i+1] = ccode; */
            }
        } else if ( format7F == 1 ) {
            tt_uint8 nRanges, first, nLeft,ccode;

            i = 0;
            nRanges = ReadUnsignedByteMacro( t->in );
            /* t->gIndexToCharCode[0] = 0xffff; */
            for ( j = 0; j < nRanges && i < 255; j++ ) {
                first = ReadUnsignedByteMacro( t->in );
                nLeft = ReadUnsignedByteMacro( t->in );
                for ( k = 0; k <= nLeft && i < 255; k++ ) {
                    ccode =  (tt_uint8)(first + k);
                    i++;
                    /* t->gIndexToCharCode[i] = ccode; */
                    t->charCodeToSID[ccode] = t->gIndexToSID[i];
                }
            }
        } else {
            assert( false );;
        }
        if ( format & 0x80 ) {
            /* supplemental encoding data. */
            tt_uint8 nSups, ccode;

            nSups = ReadUnsignedByteMacro( t->in );
            for ( i = 0; i < nSups; i++ ) {
                ccode = ReadUnsignedByteMacro( t->in );
                t->charCodeToSID[ccode] = READ_SID(t->in);
            }
        }
    }

    /* Now fill in the SIDToGIndex array */
    for ( i = 0; i < sidLimit; i++ ) {
        t->SIDToGIndex[i] = 0;
    }
    for ( i = 0; i < t->NumCharStrings; i++ ) {
        j = t->gIndexToSID[i];
        if ( j < sidLimit ) {
            t->SIDToGIndex[j] = (tt_uint16)i;
        }
    }

}



CFFClass *tsi_NewCFFClass( tsiMemObject *mem, InputStream *in, tt_int32 fontNum )
{
    register CFFClass *t = NULL;
    Card8	tmpCard8;


    /*** Read the header ***/
    tmpCard8 = ReadUnsignedByteMacro( in );
    /* See if we understand this version */
    tsi_Assert( mem, tmpCard8 == 1, T2K_UNKNOWN_CFF_VERSION );

    t = (CFFClass *)tsi_AllocMem( mem, sizeof( CFFClass ) );
    t->mem		= mem;
    t->major	= tmpCard8;
    t->minor	= ReadUnsignedByteMacro( in );
    t->hdrSize	= ReadUnsignedByteMacro( in );
    t->offSize 	= ReadUnsignedByteMacro( in );

    /* Skip data from future formats that we do not understand. */
    Seek_InputStream( in, t->hdrSize );
    /*** Done with the header ***/

    t->in = in;

    t->name		= tsi_NewCFFIndexClass( mem, in );	/* Name Index */
    t->topDict	= tsi_NewCFFIndexClass( mem, in );	/* Top DICT Index */
    t->string	= tsi_NewCFFIndexClass( mem, in );	/* String Index */

    t->gSubr	= tsi_NewCFFIndexClass( mem, in );	/* Global Subr Index */

    if ( t->gSubr->count < 1240 ) {
        t->gSubrBias = 107;
    } else if ( t->gSubr->count < 33900 ) {
        t->gSubrBias = 1131;
    } else {
        t->gSubrBias = 32768;
    }

    tsi_ParseCFFTopDict( t->topDict, in, &t->topDictData, fontNum );




    Seek_InputStream( in, t->topDictData.Charstrings );

    t->CharStrings = tsi_NewCFFIndexClass( mem, in );	/* CharStrings */


    /* Private DICT, per-font */
    tsi_ParsePrivateDictData( t );

    /* Local soubroutines, per-font */

    t->lSubr = NULL;
    t->lSubrBias = 0;
    if ( t->privateDictData.Subr != 0 ) {
        Seek_InputStream( in, t->privateDictData.SubrOffset );
        t->lSubr = tsi_NewCFFIndexClass( mem, in );	/* Local Subr Index */
        if ( t->lSubr->count < 1240 ) {
            t->lSubrBias = 107;
        } else if ( t->lSubr->count < 33900 ) {
            t->lSubrBias = 1131;
        } else {
            t->lSubrBias = 32768;
        }
    }
    BuildT2CMAP( t );
    BuildT2MetricsEtc( t );
    return t; /*****/
}



void tsi_DeleteCFFClass( CFFClass *t )
{
    if ( t != NULL ) {
        tsi_DeleteCFFIndexClass( t->name );

        tsi_DeAllocMem( t->mem, (char *)t->topDictData.buildCharArray );
        tsi_DeleteCFFIndexClass( t->topDict );

        tsi_DeleteCFFIndexClass( t->string );
        tsi_DeleteCFFIndexClass( t->gSubr );
        tsi_DeleteCFFIndexClass( t->CharStrings );
        tsi_DeleteCFFIndexClass( t->lSubr );
        tsi_DeAllocMem( t->mem, (char *)t->gIndexToSID );
        tsi_DeAllocMem( t->mem, (char *)t );
    }
}


/*
 *
 */
GlyphClass *tsi_T2GetGlyphByIndex( CFFClass *t, tt_uint16 index, tt_uint16 *aw )
{
    int byteCount, limit = t->CharStrings->count;
    GlyphClass *glyph = NULL;

    t->glyph = New_EmptyGlyph( t->mem, 0, 0 );
    t->glyph->curveType = 3;
    t->gNumStackValues = 0;

    if ( index < limit ) {
        /* Initialize the Type2BuildChar state. */
        t->awx 	= t->privateDictData.defaultWidthX;
        t->awy 	= 0;
        t->lsbx = 0;
        t->lsby = 0;
        t->x = 0;
        t->y = 0;
        t->numStemHints	= 0;
        t->pointAdded	= 0;
        t->widthDone	= false;

        /* Find the data */
        byteCount = t->CharStrings->offsetArray[index+1] - t->CharStrings->offsetArray[index];
        Seek_InputStream( t->in, t->CharStrings->baseDataOffset + t->CharStrings->offsetArray[index] );

        /* Go! */
        Type2BuildChar( t, t->in, byteCount, 0 );

        /* Wrap up the contour */
        glyph_CloseContour( t->glyph );
        LOG_CMD("ep[contourCount-1] = ", t->glyph->ep[t->glyph->contourCount-1] );
        LOG_CMD("t->glyph->contourCount", t->glyph->contourCount );
        t->lsbx = GetGlyphXMin( t->glyph );
    }
    glyph = t->glyph;

    glyph->ooy[glyph->pointCount + 0] = 0;
    glyph->oox[glyph->pointCount + 0] = 0;

    glyph->ooy[glyph->pointCount + 1] = (short)t->awy;
    glyph->oox[glyph->pointCount + 1] = (short)t->awx;

    *aw = (tt_uint16)t->awx;
    t->glyph = NULL;
    FlipContourDirection( glyph );
    /* Return the glyph data */
    return glyph; /*****/
}


/*
 *
 */
GlyphClass *tsi_T2GetGlyphByCharCode( CFFClass *t, tt_uint32 charCode, tt_uint16 *aw )
{
    tt_uint16 index = tsi_T2GetGlyphIndex( t, charCode );
    return tsi_T2GetGlyphByIndex( t, index, aw ); /*****/
}

#endif /* ENABLE_CFF */

