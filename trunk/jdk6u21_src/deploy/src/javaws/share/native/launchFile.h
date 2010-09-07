/*
 * @(#)launchFile.h	1.21 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Interprets a JNL file an populates
 * the JNLFile structure. This interface
 * separates the structure of the JNL file from
 * the logic of the launcing.
 *
 */

typedef struct _jnlFile {
    char* jreVersion;   /* Version String for JRE */
    char* jreLocation;  /* URL for JRE or NULL if none specified */
    int   isPlayer;     /* TRUE, if is player JNLP */
    int   splashPref;   /* 0 = show, 1 = never show, 2 = show only custom */
    char* jnlp_url;	/* codebase+href (or null if href dosn't exist) */
    char* canonicalHome;
    char* initialHeap;
    char* maxHeap;
    int   auxArgCount;
    char* auxArg[20];   /* be carful not to overfill */
    int   auxPropCount;
    char* auxProp[20];
} JNLFile;

/* valuse for splashPref */
#define SPLASH_ALWAYS 0
#define SPLASH_NEVER 1
#define SPLASH_CUSTOM_ONLY 2

/*
 * Parse a JNL file, and returns a structure with the information
 * needed by the C launcher 
 *
 * The interpretation of the JNL file is completly encapsulated into
 * this method, so several formats such as XML and property files can
 * be supported at the same time 
 */
JNLFile* ParseJNLFile(char *s, int verbose);

/* Release all memory allocated to a JNLFile */
void FreeJNLFile(JNLFile* jnlfile);

int isSecureProperty(char *key);
int isSecureVmArg(char *arg);

