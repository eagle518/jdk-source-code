/*
 * @(#)java_md.h	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JAVA_MD_H
#define JAVA_MD_H

#include <windows.h>

#define PATH_SEPARATOR	';'
#define FILE_SEPARATOR	'\\'
#define MAXPATHLEN      MAX_PATH

#ifdef JAVA_ARGS
/*
 * ApplicationHome is prepended to each of these entries; the resulting
 * strings are concatenated (seperated by PATH_SEPARATOR) and used as the
 * value of -cp option to the launcher.
 */
#ifndef APP_CLASSPATH
#define APP_CLASSPATH        { "\\lib\\tools.jar", "\\classes" }
#endif
#endif

/*
 * Support for doing cheap, accurate interval timing.
 */
extern jlong CounterGet(void);
extern jlong Counter2Micros(jlong counts);

#ifdef JAVAW
#define main _main
extern int _main(int argc, char **argv);
#endif

#endif
