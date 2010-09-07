/*
 *  @(#)cmemory.h	1.3 03/12/19
 *
 * (C) Copyright IBM Corp. 1998, 1999 - All Rights Reserved
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
* File CMEMORY.H
*
*  Contains stdlib.h/string.h memory functions
*
* @author       Bertrand A. Damiba
*
* Modification History:
*
*   Date        Name        Description
*   6/20/98     Bertrand    Created.
*  05/03/99     stephen     Changed from functions to macros.
*
*******************************************************************************
*/

#ifndef CMEMORY_H
#define CMEMORY_H

#include <stdlib.h>
#include <string.h>

#define icu_malloc(size) malloc(size)
#define icu_realloc(buffer, size) realloc(buffer, size)
#define icu_free(buffer) free(buffer)
#define icu_memcpy(dst, src, size) memcpy(dst, src, size)
#define icu_memmove(dst, src, size) memmove(dst, src, size)
#define icu_memset(buffer, mark, size) memset(buffer, mark, size)
#define icu_memcmp(buffer1, buffer2, size) memcmp(buffer1, buffer2,size)

#endif

