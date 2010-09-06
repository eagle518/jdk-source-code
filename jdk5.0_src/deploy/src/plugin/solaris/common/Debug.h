/*
 * @(#)Debug.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __cplusplus
extern "C" {
#endif
    void trace(const char *format, ...);
    void trace_verbose(const char *format, ...);
    extern int tracing;
#ifdef __cplusplus
}
#endif
