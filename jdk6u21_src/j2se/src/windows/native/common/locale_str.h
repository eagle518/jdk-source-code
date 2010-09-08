/*
 * @(#)locale_str.h	1.35 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#define DllExport __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

DllExport const char * getEncodingFromLangID(LANGID langID);
DllExport const char * getJavaIDFromLangID(LANGID langID);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
