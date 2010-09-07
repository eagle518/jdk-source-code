/*
 * @(#)SMOD.h	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#define SMOD_COUNT	4

extern void VolumeAmpScaler(unsigned char *pSample, INT32 length, INT32 param1, INT32 param2);
extern void SMOD_1(unsigned char *pSample, INT32 length, INT32 param1, INT32 param2);
extern void SMOD_2(unsigned char *pSample, INT32 length, INT32 param1, INT32 param2);
extern void SMOD_3(unsigned char *pSample, INT32 length, INT32 param1, INT32 param2);

