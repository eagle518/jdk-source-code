/*
 * @(#)ghints.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * File:		GHINTS.H
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
#ifndef __T2K_GHINTS__
#define __T2K_GHINTS__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

void ComputeGlobalHints( sfntClass *font, ag_HintHandleType hintHandle, ag_GlobalDataType *gHints, int kanji );
void ReadGHints( ag_GlobalDataType *gHints, InputStream *in );
void WriteGHints( ag_GlobalDataType *gHints, OutputStream *out );


#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* __T2K_GHINTS__ */
