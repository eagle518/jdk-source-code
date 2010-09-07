/*
 * @(#)JarFacade.h	1.2 04/06/09 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Parse the given manifest of the given jar file, and return the values
 * of the given attribute if they exist. Attribute value will be returned as
 * NULL if it doesn't exist.
 */
int parseJarFile(const char* name, int attributeCount, char** attributes, char**values);

