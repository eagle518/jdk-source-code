/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OS_DEFS_HPP
#define OS_DEFS_HPP

#include <io.h>

typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;

typedef unsigned short   unicodechar_t;


#define FILE_SEPARATOR_CHAR '\\'


#define FORMAT64_MODIFIER "I64"

#define INT32_FORMAT "%d"
#define UINT32_FORMAT "%u"
#define PTR32_FORMAT "0x%08x"
#define INT64_FORMAT "%" FORMAT64_MODIFIER "d"
#define UINT64_FORMAT "%" FORMAT64_MODIFIER "u"
#define PTR64_FORMAT "0x%016" FORMAT64_MODIFIER "x"

#ifdef _WIN64
#define PTR_FORMAT PTR64_FORMAT
#define UINT_FORMAT UINT64_FORMAT
#define INT_FORMAT INT64_FORMAT
#define SIZET_FORMAT UINT64_FORMAT
#else
#define PTR_FORMAT PTR32_FORMAT
#define UINT_FORMAT UINT32_FORMAT
#define INT_FORMAT INT32_FORMAT
#define SIZET_FORMAT UINT32_FORMAT
#endif


#endif
