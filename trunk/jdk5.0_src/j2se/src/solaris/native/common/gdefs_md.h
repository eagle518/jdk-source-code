/*
 * @(#)gdefs_md.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Solaris dependent type definitions  includes intptr_t, etc 
 */


#include <sys/types.h>
/*
 * Linux version of <sys/types.h> does not define intptr_t
 */
#ifdef __linux__
#include <stdint.h>
#include <malloc.h>
#endif /* __linux__ */

