/*
 * @(#)TimeZone_md.h	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _TIMEZONE_MD_H
#define _TIMEZONE_MD_H

char *findJavaTZ_md(const char *java_home_dir, const char *region);
char *getGMTOffsetID();

#endif
