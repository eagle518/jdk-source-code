/*
 * @(#)secure.c	1.17 10/04/02
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"

/*
 *
 * SecureVmArgs is a list of strings that the jnlp file is allowed to include in
 * the vmargs argument to the j2se element.
 *
 */
static char *SecureVmArgs [] = {
"-d32",				/* use a 32-bit data model if available */
"-client",			/* to select the "client" VM */
"-server",			/* to select the "server" VM */
"-verbose",			/* enable verbose output */
"-version",			/* print product version and exit */
"-showversion",			/* print product version and continue */
"-help",			/* print this help message */
"-X",				/* print help on non-standard options */
"-ea",				/* enable assertions */
"-enableassertions",		/* enable assertions */
"-da",				/* disable assertions */
"-disableassertions",		/* disable assertions */
"-esa",				/* enable system assertions */
"-enablesystemassertions",	/* enable system assertions */
"-dsa",				/* disable system assertione */
"-disablesystemassertions",	/* disable system assertione */
"-Xmixed",			/* mixed mode execution (default) */
"-Xint",			/* interpreted mode execution only */
"-Xnoclassgc",			/* disable class garbage collection */
"-Xincgc",			/* enable incremental garbage collection */
"-Xbatch",			/* disable background compilation */
"-Xprof",			/* output cpu profiling data */
"-Xdebug",			/* enable remote debugging */
"-Xfuture",			/* enable strictest checks, anticipating future default */
"-Xrs",				/* reduce use of OS signals by Java/VM (see documentation) */
"-XX:+ForceTimeHighResolution", /* use high resolution timer */
"-XX:-ForceTimeHighResolution", /* use low resolution (default) */
};

/*
 *  SecureVmArgPrefixes is a list of strings the jnlp file is allow to include
 *  in the vmargs string, any string that starts with these prefixes.
 *
 *
 */
static char *SecureVmArgPrefixes [] = {
"-ea:",				/* enable assertions for classes ... */
"-enableassertions:",		/* enable assertions for classes ... */
"-da:",				/* disable assertions for classes ... */
"-disableassertions:",		/* disable assertions for classes ... */
"-verbose:",			/* enable verbose output */
"-Xmn",				/* set the young gen size 
				   this is an unspecified and undocumented
				   JVM option being added here only as
				   a convenience to end users */
"-Xms",				/* set initial Java heap size */
"-Xmx",				/* set maximum Java heap size */
"-Xss",				/* set java thread stack size */
"-XX:NewRatio",                 /* set Ratio of new/old gen sizes */
"-XX:NewSize",                  /* set initial size of new generation */
"-XX:MaxNewSize",               /* set max size of new generation */
"-XX:PermSize",                 /* set initial size of permanent gen */
"-XX:MaxPermSize",              /* set max size of permanent gen */
"-XX:MaxHeapFreeRatio",         /* heap free percentage (default 70) */
"-XX:MinHeapFreeRatio",         /* heap free percentage (default 40) */
"-XX:UseSerialGC",              /* use serial garbage collection */
"-XX:ThreadStackSize",          /* thread stack size (in KB) */
"-XX:MaxInlineSize",            /* set max num of bytecodes to inline */
"-XX:ReservedCodeCacheSize",    /* Reserved code cache size (bytes) */
"-XX:MaxDirectMemorySize",
};


/* 
 * SecurePropertyKeys is a list of keys that can be set in a jnlp file using
 * <property name="key" value="value" />  and we will pass on to the java 
 * invocation as -Dkey=value
 */
static char *SecurePropertyKeys[] =
{
    "sun.java2d.noddraw",
    "javaws.cfg.jauthenticator",
    "swing.useSystemFontSettings",
    "swing.metalTheme",
    "http.agent",
    "http.keepAlive",
    "sun.awt.noerasebackground",
    "sun.java2d.opengl",
    "sun.java2d.d3d",
    "java.awt.syncLWRequests",
    "java.awt.Window.locationByPlatform",
    "sun.awt.erasebackgroundonresize",
    "swing.noxp",
    "swing.boldMetal",
    "awt.useSystemAAFontSettings",
    "sun.java2d.dpiaware",
    "sun.awt.disableMixing",
};



int isSecureVmArg(char *arg) {
    int i;
    int count;
    char property[256];
    count = sizeof(SecureVmArgs) / sizeof(SecureVmArgs[0]); 
    for (i=0; i<count; i++) {
        if (strcmp(arg, SecureVmArgs[i]) == 0) {
            return 1;
        }
    }  
    count = sizeof(SecureVmArgPrefixes) / sizeof(SecureVmArgPrefixes[0]); 
    for (i=0; i<count; i++) {
	int len = strlen(SecureVmArgPrefixes[i]);
        if (strncmp(arg, SecureVmArgPrefixes[i], len) == 0) {
            return 1;
        }
    }  
    if ((strlen(arg) >=5) && (strncmp("-D", arg, 2) == 0)) {
        int j=0;
        for (i=2; i<strlen(arg) && i<256; i++) {
            if (arg[i] != '=') {
                property[j++] = arg[i];
            } else if (j != 0) {
                property[j] = '\0';
                if (isSecureProperty(property)) {
                    return 1;
                }
            }
        }
    }

    return 0;
}

int isDefaultSecureProperty(char *key) {
    int i;
    int count = sizeof(SecurePropertyKeys) / sizeof(SecurePropertyKeys[0]); 
    for (i=0; i<count; i++) {
        if (sysStrCaseCmp(key, SecurePropertyKeys[i]) == 0) {
            return 1;
        }
    }  
    if ((strncmp(key, "javaws.", strlen("javaws."))) == 0) {
	return 1;
    }
    if ((strncmp(key, "jnlp.", strlen("jnlp."))) == 0) {
	return 1;
    }
    if ((strncmp(key, "javapi.", strlen("javapi."))) == 0) {
	return 1;
    }
    return 0;
}




