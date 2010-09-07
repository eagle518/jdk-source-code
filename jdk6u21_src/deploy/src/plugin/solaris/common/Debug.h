/*
 * @(#)Debug.h	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
