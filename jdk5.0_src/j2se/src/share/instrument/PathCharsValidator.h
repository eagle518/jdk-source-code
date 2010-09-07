/*
 * @(#)PathCharsValidator.h	1.2 04/06/14 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*
 * Validates that the given URI path component does not contain any 
 * illegal characters. Returns 0 if only validate characters are present.
 */
int validatePathChars(const char* path);

