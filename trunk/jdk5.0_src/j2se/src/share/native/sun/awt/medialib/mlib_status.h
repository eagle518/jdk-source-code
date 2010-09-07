/*
 * @(#)mlib_status.h	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef MLIB_STATUS_H
#define MLIB_STATUS_H

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_status.h	1.11	98/06/26 SMI"
#endif /* __SUNPRO_C */

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum {
  MLIB_SUCCESS     = 0,
  MLIB_FAILURE     = 1,
  MLIB_NULLPOINTER = 2,
  MLIB_OUTOFRANGE  = 3
} mlib_status;

#ifdef	__cplusplus
}
#endif

#endif	/* MLIB_STATUS_H */
