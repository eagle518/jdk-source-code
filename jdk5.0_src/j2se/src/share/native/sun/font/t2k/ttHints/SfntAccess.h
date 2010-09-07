/*
 * @(#)SfntAccess.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
 *	TrueType
 *	Copyright 1987-1991 Apple Computer, Inc.
 */
#define ACCENTS
#ifndef sfntAccessIncludes
#define sfntAccessIncludes

#ifndef fsglueIncludes
	#include "FSglue.h"
#endif

#ifdef ACCENTS
#define accentAttachmentTag	0x61636e74	/* 'acnt' */
#endif

 void* GetSfntGlyphPtr( fsg_SplineKey* key, tt_uint16 glyphIndex );

#endif
