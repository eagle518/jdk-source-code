/*
 * @(#)mlib_v_ImageLogic_proto.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __MLIB_V_IMAGELOGIC_PROTO_H
#define __MLIB_V_IMAGELOGIC_PROTO_H

#pragma ident	"@(#)mlib_v_ImageLogic_proto.h	1.3	03/01/31 SMI"

#include <mlib_types.h>
#include <mlib_image_types.h>
#include <mlib_status.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void mlib_v_ImageNot_na(mlib_u8  *sa,
                        mlib_u8  *da,
                        mlib_s32 size);
mlib_status mlib_v_ImageNot_Bit(mlib_image       *dst,
                                const mlib_image *src);
void mlib_v_ImageNot_blk(const void *src,
                         void       *dst,
                         mlib_s32   size);

mlib_status mlib_v_ImageAnd_Bit(mlib_image       *dst,
                                const mlib_image *src1,
                                const mlib_image *src2);
mlib_status mlib_v_ImageAndNot_Bit(mlib_image       *dst,
                                   const mlib_image *src1,
                                   const mlib_image *src2);

mlib_status mlib_v_ImageConstAnd_Bit(mlib_image       *dst,
                                     const mlib_image *src1,
                                     const mlib_s32   *c);
mlib_status mlib_v_ImageConstAndNot_Bit(mlib_image       *dst,
                                        const mlib_image *src1,
                                        const mlib_s32   *c);
mlib_status mlib_v_ImageConstNotAnd_Bit(mlib_image       *dst,
                                        const mlib_image *src1,
                                        const mlib_s32   *c);
mlib_status mlib_v_ImageConstNotOr_Bit(mlib_image       *dst,
                                       const mlib_image *src1,
                                       const mlib_s32   *c);
mlib_status mlib_v_ImageConstNotXor_Bit(mlib_image       *dst,
                                        const mlib_image *src1,
                                        const mlib_s32   *c);
mlib_status mlib_v_ImageConstOr_Bit(mlib_image       *dst,
                                    const mlib_image *src1,
                                    const mlib_s32   *c);
mlib_status mlib_v_ImageConstOrNot_Bit(mlib_image       *dst,
                                       const mlib_image *src1,
                                       const mlib_s32   *c);
mlib_status mlib_v_ImageConstXor_Bit(mlib_image       *dst,
                                     const mlib_image *src1,
                                     const mlib_s32   *c);

mlib_status mlib_v_ImageNotAnd_Bit(mlib_image       *dst,
                                   const mlib_image *src1,
                                   const mlib_image *src2);
mlib_status mlib_v_ImageNotOr_Bit(mlib_image       *dst,
                                  const mlib_image *src1,
                                  const mlib_image *src2);
mlib_status mlib_v_ImageNotXor_Bit(mlib_image       *dst,
                                   const mlib_image *src1,
                                   const mlib_image *src2);
mlib_status mlib_v_ImageOr_Bit(mlib_image       *dst,
                               const mlib_image *src1,
                               const mlib_image *src2);
mlib_status mlib_v_ImageOrNot_Bit(mlib_image       *dst,
                                  const mlib_image *src1,
                                  const mlib_image *src2);
mlib_status mlib_v_ImageXor_Bit(mlib_image       *dst,
                                const mlib_image *src1,
                                const mlib_image *src2);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_V_IMAGELOGIC_PROTO_H */
