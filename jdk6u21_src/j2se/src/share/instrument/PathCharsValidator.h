/*
 * @(#)PathCharsValidator.h	1.4 10/03/23 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*
 * Validates that the given URI path component does not contain any 
 * illegal characters. Returns 0 if only validate characters are present.
 */
int validatePathChars(const char* path);

