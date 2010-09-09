/*
 * @(#)EncodingSupport.h	1.4 10/03/23 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Return length of UTF-8 in modified UTF-8.
 */
int modifiedUtf8LengthOfUtf8(char* utf_str, int utf8_len);

/**
 * Convert UTF-8 to modified UTF-8.
 */
void convertUtf8ToModifiedUtf8(char* utf8_str, int utf8_len, char* mutf8_str, int mutf8_len);

/**
 * Convert UTF-8 to a platform string 
 */
int convertUft8ToPlatformString(char* utf8_str, int utf8_len, char* platform_str, int platform_len);


