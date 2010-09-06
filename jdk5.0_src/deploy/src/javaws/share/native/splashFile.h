/*
 * @(#)splashFile.h	1.4 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef SPLASH_FILE_H
#define SPLASH_FILE_H

/*
 * Locates the jpeg files to be used in the splash screen.
 *
 */

char *getDefaultSplashFiles(int playerMode, char **splash1, char **splash2);

char *getAppSplashFiles(JNLFile *jnlFile, char **splash1, char **splash2);

#endif




