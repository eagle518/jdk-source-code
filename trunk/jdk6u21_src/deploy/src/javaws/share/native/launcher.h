/*
 * @(#)launcher.h	1.30 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "util.h"
#include "system.h"
#include "configurationFile.h"
#include "launchFile.h"
#include "splashFile.h"
#include "msgString.h"

#define JAVAWS_MAIN_CLASS   "com.sun.javaws.Main"

/*
 * Declarations
 */

int   EULA_md                     (int argc, char **argv, int player);

void  LauncherSetup_md            (char **argv);

#endif /* LAUNCHER_H */
