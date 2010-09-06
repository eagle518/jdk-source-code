#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)java_md.h	1.5 03/12/23 16:37:28 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef JAVA_MD_H
#define JAVA_MD_H

#include <limits.h>

#define PATH_SEPARATOR		':'
#define FILE_SEPARATOR		'/'

// PATH_MAX should include the '\0' terminator, unfortunately some
// Linux versions (e.g. Redhat 6.x) define PATH_MAX without it.
// Use PATH_MAX + 1 so we always have enough buffer space.
#define MAXPATHLEN		(PATH_MAX + 1)

#ifdef JAVA_ARGS
/*
 * ApplicationHome is prepended to each of these entries; the resulting
 * strings are concatenated (seperated by PATH_SEPARATOR) and used as the
 * value of -cp option to the launcher.
 */
#ifndef APP_CLASSPATH
#define APP_CLASSPATH        { "/lib/tools.jar", "/classes" }
#endif
#endif

#ifdef HAVE_GETHRTIME
/*
 * Support for doing cheap, accurate interval timing.
 */
#include <sys/time.h>
#define CounterGet()           	  (gethrtime()/1000)
#define Counter2Micros(counts) 	  (counts)
#else
#define CounterGet()		  (0)
#define Counter2Micros(counts)	  (1)
#endif /* HAVE_GETHRTIME */

#endif
