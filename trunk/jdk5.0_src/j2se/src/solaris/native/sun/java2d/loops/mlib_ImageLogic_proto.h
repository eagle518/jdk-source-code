/*
 * @(#)mlib_ImageLogic_proto.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __MLIB_IMAGELOGIC_H
#define __MLIB_IMAGELOGIC_H

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageLogic_proto.h	1.2	03/01/31 SMI"
#endif /* __SUNPRO_C */

#include <mlib_types.h>
#include <mlib_image_types.h>
#include <mlib_status.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

mlib_status mlib_ImageAnd_Bit(mlib_image       *dst,
                              const mlib_image *src1,
                              const mlib_image *src2);
mlib_status mlib_ImageAndNot_Bit(mlib_image       *dst,
                                 const mlib_image *src1,
                                 const mlib_image *src2);
mlib_status mlib_ImageNot_Bit(mlib_image       *dst,
                              const mlib_image *src);
mlib_status mlib_ImageNotAnd_Bit(mlib_image       *dst,
                                 const mlib_image *src1,
                                 const mlib_image *src2);
mlib_status mlib_ImageNotOr_Bit(mlib_image       *dst,
                                const mlib_image *src1,
                                const mlib_image *src2);
mlib_status mlib_ImageNotXor_Bit(mlib_image       *dst,
                                 const mlib_image *src1,
                                 const mlib_image *src2);
mlib_status mlib_ImageOr_Bit(mlib_image       *dst,
                             const mlib_image *src1,
                             const mlib_image *src2);
mlib_status mlib_ImageOrNot_Bit(mlib_image       *dst,
                                const mlib_image *src1,
                                const mlib_image *src2);
mlib_status mlib_ImageXor_Bit(mlib_image       *dst,
                              const mlib_image *src1,
                              const mlib_image *src2);

mlib_status mlib_ImageConstAnd_Bit(mlib_image       *dst,
                                   const mlib_image *src,
                                   const mlib_s32   *c);
mlib_status mlib_ImageConstAndNot_Bit(mlib_image       *dst,
                                      const mlib_image *src,
                                      const mlib_s32   *c);
mlib_status mlib_ImageConstNotAnd_Bit(mlib_image       *dst,
                                      const mlib_image *src,
                                      const mlib_s32   *c);
mlib_status mlib_ImageConstNotOr_Bit(mlib_image       *dst,
                                     const mlib_image *src,
                                     const mlib_s32   *c);
mlib_status mlib_ImageConstNotXor_Bit(mlib_image       *dst,
                                      const mlib_image *src,
                                      const mlib_s32   *c);
mlib_status mlib_ImageConstOr_Bit(mlib_image       *dst,
                                  const mlib_image *src,
                                  const mlib_s32   *c);
mlib_status mlib_ImageConstOrNot_Bit(mlib_image       *dst,
                                     const mlib_image *src,
                                     const mlib_s32   *c);
mlib_status mlib_ImageConstXor_Bit(mlib_image       *dst,
                                   const mlib_image *src,
                                   const mlib_s32   *c);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_IMAGELOGIC_H */
