/*
 * @(#)mlib_ImageRowTable.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef MLIB_IMAGE_ROWTABLE_H
#define MLIB_IMAGE_ROWTABLE_H

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageRowTable.h	1.2	02/02/22 SMI"
#endif /* __SUNPRO_C */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void *mlib_ImageCreateRowTable(mlib_image *image);
void mlib_ImageDeleteRowTable(mlib_image *image);

static void *mlib_ImageGetRowTable(mlib_image *img)
{
  return img->state;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* MLIB_IMAGE_ROWTABLE_H */

