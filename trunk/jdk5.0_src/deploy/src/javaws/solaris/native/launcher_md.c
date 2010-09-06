/*
 * @(#)launcher_md.c	1.8 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"

static char * SetExecname(char **argv);

/* 
 *
 */

void LauncherSetup_md(char **argv) {
    SetExecname(argv);
}
/* always 1 (pass) for EULA on solaris/linux */
int EULA_md(int argc, char** argv, int isPlayer) {
  return 1;
}

