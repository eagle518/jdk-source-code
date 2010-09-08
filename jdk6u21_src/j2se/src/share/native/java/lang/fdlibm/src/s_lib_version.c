
 /* @(#)s_lib_version.c	1.10 10/03/23           */
/*
 * @(#)s_lib_version.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * MACRO for standards
 */

#include "fdlibm.h"

/*
 * define and initialize _LIB_VERSION
 */
#ifdef _POSIX_MODE
_LIB_VERSION_TYPE _LIB_VERSION = _POSIX_;
#else
#ifdef _XOPEN_MODE
_LIB_VERSION_TYPE _LIB_VERSION = _XOPEN_;
#else
#ifdef _SVID3_MODE
_LIB_VERSION_TYPE _LIB_VERSION = _SVID_;
#else					/* default _IEEE_MODE */
_LIB_VERSION_TYPE _LIB_VERSION = _IEEE_;
#endif
#endif
#endif
