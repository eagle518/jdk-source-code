/*
 *  @(#)uchardir.h	1.4 03/12/19
 *
 * (C) Copyright IBM Corp. 1999-2003 - All Rights Reserved
 *
 * Portions Copyright 2004 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 *
 * The original version of this source code and documentation is
 * copyrighted and owned by IBM. These materials are provided
 * under terms of a License Agreement between IBM and Sun.
 * This technology is protected by multiple US and International
 * patents. This notice and attribution to IBM may not be removed.
 */

/*
*
* File UCHARDIR.H
*
* Modification History:
*
*   Date          Name        Description
*   11/30/1999    dfelt       Creation.  Copied UCharDirection from uchar.h
********************************************************************************
*/

#ifndef UCHARDIR_H
#define UCHARDIR_H

#include "utypes.h"

/*===========================================================================*/
/* Unicode version number                                                    */
/*===========================================================================*/
#define UNICODE_VERSION  "3.0.0.beta"

enum UCharDirection   { 
    U_LEFT_TO_RIGHT               = 0, 
    U_RIGHT_TO_LEFT               = 1, 
    U_EUROPEAN_NUMBER             = 2,
    U_EUROPEAN_NUMBER_SEPARATOR   = 3,
    U_EUROPEAN_NUMBER_TERMINATOR  = 4,
    U_ARABIC_NUMBER               = 5,
    U_COMMON_NUMBER_SEPARATOR     = 6,
    U_BLOCK_SEPARATOR             = 7,
    U_SEGMENT_SEPARATOR           = 8,
    U_WHITE_SPACE_NEUTRAL         = 9, 
    U_OTHER_NEUTRAL               = 10, 
    U_LEFT_TO_RIGHT_EMBEDDING     = 11,
    U_LEFT_TO_RIGHT_OVERRIDE      = 12,
    U_RIGHT_TO_LEFT_ARABIC        = 13,
    U_RIGHT_TO_LEFT_EMBEDDING     = 14,
    U_RIGHT_TO_LEFT_OVERRIDE      = 15,
    U_POP_DIRECTIONAL_FORMAT      = 16,
    U_DIR_NON_SPACING_MARK        = 17,
    U_BOUNDARY_NEUTRAL            = 18,
    U_CHAR_DIRECTION_COUNT
};

typedef enum UCharDirection UCharDirection;

/**
  * Returns the linguistic direction property of a character.
  * <P>
  * Returns the linguistic direction property of a character.
  * For example, 0x0041 (letter A) has the LEFT_TO_RIGHT directional 
  * property.
  * @see UCharDirection
  */
U_CAPI UCharDirection U_EXPORT2
u_charDirection(UChar c);

U_CAPI UCharDirection U_EXPORT2
u_getDirection(uint32_t cp);

U_CAPI UCharDirection U_EXPORT2
u_surrogatePairDirection(UChar lead, UChar trail);

#endif /*_UCHAR*/
/*eof*/

