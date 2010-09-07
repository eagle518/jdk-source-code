/*
 * @(#)mlib_ImageDivTables.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef __MLIB_IMAGEDIVTABLES_H
#define __MLIB_IMAGEDIVTABLES_H

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageDivTables.h	1.1	02/03/07 SMI"
#endif /* __SUNPRO_C */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __DIV_TABLE_DEFINED

#ifdef __SUNPRO_C
#pragma align 64 (mlib_div6_tab)
#pragma align 64 (mlib_div1_tab)
#pragma align 64 (mlib_HSL2RGB_L2)
#pragma align 64 (mlib_HSL2RGB_F)
#pragma align 64 (mlib_U82F32)
#pragma align 64 (mlib_FlipAndFixRotateTable)
#endif /* __SUNPRO_C */

const mlib_u16 mlib_div6_tab[];
const mlib_u16 mlib_div1_tab[];
const mlib_f32 mlib_HSL2RGB_L2[];
const mlib_f32 mlib_HSL2RGB_F[];
const mlib_f32 mlib_U82F32[];
const mlib_d64 mlib_U82D64[];
const mlib_u32 mlib_FlipAndFixRotateTable[];

#else

extern const mlib_u16 mlib_div6_tab[];
extern const mlib_u16 mlib_div1_tab[];
extern const mlib_f32 mlib_HSL2RGB_L2[];
extern const mlib_f32 mlib_HSL2RGB_F[];
extern const mlib_f32 mlib_U82F32[];
extern const mlib_d64 mlib_U82D64[];
extern const mlib_u32 mlib_FlipAndFixRotateTable[];

#endif /* __DIV_TABLE_DEFINED */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_IMAGEDIVTABLES_H */

/***************************************************************/
