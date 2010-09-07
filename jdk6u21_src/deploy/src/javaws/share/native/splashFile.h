/*
 * @(#)splashFile.h	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef SPLASH_FILE_H
#define SPLASH_FILE_H

/*
 * Locates the jpeg files to be used in the splash screen.
 *
 */

void getDefaultSplashFiles(int playerMode, char **splash);

void getAppSplashFiles(JNLFile *jnlFile, char **splash);

#endif




