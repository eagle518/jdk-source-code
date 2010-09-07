/*
 * @(#)mlib_ImageCheck.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageCheck.h	1.14	02/04/19 SMI"
#endif /* __SUNPRO_C */

#ifndef MLIB_IMAGECHECK_H
#define MLIB_IMAGECHECK_H

#include <stdlib.h>
#include <mlib_image.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************/

#define MLIB_IMAGE_CHECK(image)                                 \
  if (image == NULL) return MLIB_NULLPOINTER

#define MLIB_IMAGE_SIZE_EQUAL(image1,image2)                       \
  if (mlib_ImageGetWidth(image1)  != mlib_ImageGetWidth(image2) || \
      mlib_ImageGetHeight(image1) != mlib_ImageGetHeight(image2))  \
    return MLIB_FAILURE

#define MLIB_IMAGE_TYPE_EQUAL(image1,image2)                    \
  if (mlib_ImageGetType(image1) != mlib_ImageGetType(image2))   \
    return MLIB_FAILURE

#define MLIB_IMAGE_CHAN_EQUAL(image1,image2)                          \
  if (mlib_ImageGetChannels(image1) != mlib_ImageGetChannels(image2)) \
    return MLIB_FAILURE

#define MLIB_IMAGE_FULL_EQUAL(image1,image2)                    \
  MLIB_IMAGE_SIZE_EQUAL(image1,image2);                         \
  MLIB_IMAGE_TYPE_EQUAL(image1,image2);                         \
  MLIB_IMAGE_CHAN_EQUAL(image1,image2)

#define MLIB_IMAGE_HAVE_TYPE(image, type)                       \
  if (mlib_ImageGetType(image) != type)                         \
    return MLIB_FAILURE

#define MLIB_IMAGE_HAVE_CHAN(image, channels)                   \
  if (mlib_ImageGetChannels(image) != channels)                 \
    return MLIB_FAILURE

#define MLIB_IMAGE_HAVE_3_OR_4_CHAN(image)                      \
  if (mlib_ImageGetChannels(image) != 3 &&                      \
      mlib_ImageGetChannels(image) != 4)                        \
    return MLIB_FAILURE

#define MLIB_IMAGE_CHAN_SRC1_OR_EQ(src, dst)                      \
  if (mlib_ImageGetChannels(src) != 1) {                          \
    if (mlib_ImageGetChannels(src) != mlib_ImageGetChannels(dst)) \
      return MLIB_FAILURE;                                        \
  }

#define MLIB_IMAGE_TYPE_DSTBIT_OR_EQ(src, dst)                  \
  if ((mlib_ImageGetType(src) != mlib_ImageGetType(dst)) &&     \
      (mlib_ImageGetType(dst) != MLIB_BIT)) {                   \
    return MLIB_FAILURE;                                        \
  }

#define MLIB_IMAGE_AND_COLORMAP_ARE_COMPAT(image,colormap)                 \
  if ((mlib_ImageGetChannels(image) != mlib_ImageGetLutChannels(colormap)) \
    || (mlib_ImageGetLutType(colormap) != mlib_ImageGetType(image))) {     \
    return MLIB_FAILURE;                                                   \
  }

#define MLIB_IMAGE_GET_ALL_PARAMS(image, type, nchan, width, height, stride, pdata) \
  type   = mlib_ImageGetType(image);                                                \
  nchan  = mlib_ImageGetChannels(image);                                            \
  width  = mlib_ImageGetWidth(image);                                               \
  height = mlib_ImageGetHeight(image);                                              \
  stride = mlib_ImageGetStride(image);                                              \
  pdata  = (void*)mlib_ImageGetData(image)

/***************************************************************/

#ifdef __cplusplus
}
#endif
#endif  /* MLIB_IMAGECHECK_H */
