/*
 * @(#)launcher_md.c	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
