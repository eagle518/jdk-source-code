/*
 * @(#)mlib_ImageCreate.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef __MLIB_IMAGECREATE_H
#define __MLIB_IMAGECREATE_H

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageCreate.h	1.1	02/03/07 SMI"
#endif /* __SUNPRO_C */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

mlib_image* mlib_ImageSet(mlib_image *image,
                          mlib_type  type,
                          mlib_s32   channels,
                          mlib_s32   width,
                          mlib_s32   height,
                          mlib_s32   stride,
                          const void *data);

mlib_image *mlib_ImageSetSubimage(mlib_image       *dst,
                                  const mlib_image *src,
                                  mlib_s32         x,
                                  mlib_s32         y,
                                  mlib_s32         w,
                                  mlib_s32         h);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_IMAGECREATE_H */
