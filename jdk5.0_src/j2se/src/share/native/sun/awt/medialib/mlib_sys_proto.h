/*
 * @(#)mlib_sys_proto.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef __ORIG_MLIB_SYS_PROTO_H
#define __ORIG_MLIB_SYS_PROTO_H

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_sys_proto.h	1.10	02/12/26 SMI"
#endif /* __SUNPRO_C */

#if defined ( __MEDIALIB_OLD_NAMES_ADDED )
#include <../include/mlib_sys_proto.h>
#endif /* defined ( __MEDIALIB_OLD_NAMES_ADDED ) */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined ( _MSC_VER )
#if ! defined ( __MEDIALIB_OLD_NAMES )
#define __MEDIALIB_OLD_NAMES
#endif /* ! defined ( __MEDIALIB_OLD_NAMES ) */
#endif /* defined ( _MSC_VER ) */


#if defined ( __MEDIALIB_OLD_NAMES )
#define __mlib_malloc mlib_malloc
#endif /* ! defined ( __MEDIALIB_OLD_NAMES ) */
void * __mlib_malloc(mlib_u32 size);

#if defined ( __MEDIALIB_OLD_NAMES )
#define __mlib_realloc mlib_realloc
#endif /* ! defined ( __MEDIALIB_OLD_NAMES ) */
void * __mlib_realloc(void *ptr,
                      mlib_u32 size);

#if defined ( __MEDIALIB_OLD_NAMES )
#define __mlib_free mlib_free
#endif /* ! defined ( __MEDIALIB_OLD_NAMES ) */
void  __mlib_free(void *ptr);

#if defined ( __MEDIALIB_OLD_NAMES )
#define __mlib_memset mlib_memset
#endif /* ! defined ( __MEDIALIB_OLD_NAMES ) */
void * __mlib_memset(void *s,
                     mlib_s32 c,
                     mlib_u32 n);

#if defined ( __MEDIALIB_OLD_NAMES )
#define __mlib_memcpy mlib_memcpy
#endif /* ! defined ( __MEDIALIB_OLD_NAMES ) */
void * __mlib_memcpy(void *s1,
                     void *s2,
                     mlib_u32 n);

#if defined ( __MEDIALIB_OLD_NAMES )
#define __mlib_memmove mlib_memmove
#endif /* ! defined ( __MEDIALIB_OLD_NAMES ) */
void * __mlib_memmove(void *s1,
                      void *s2,
                      mlib_u32 n);


#if defined ( __MEDIALIB_OLD_NAMES )
#define __mlib_version mlib_version
#endif /* ! defined ( __MEDIALIB_OLD_NAMES ) */
char * __mlib_version();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __ORIG_MLIB_SYS_PROTO_H */
