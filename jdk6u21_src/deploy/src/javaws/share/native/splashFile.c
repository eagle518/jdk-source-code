
/*
 * @(#)splashFile.c	1.17 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "launchFile.h"
#include "configurationFile.h"

/*
 * Locates the jpeg files to be used in the splash screen.
 *
 */

static char path[MAXPATHLEN];

void getDefaultSplashFiles(int playerMode, char **splash) {

    char *name = "splash." ;
    
    sysStrNPrintF(path, sizeof(path), "%s%c%s%s", sysGetJavawsResourcesLib(),
        FILE_SEPARATOR, name, sysGetSplashExtension());
    *splash = path;
}

void getAppSplashFiles(JNLFile *jnlFile, char **splash) {
    /* Try to get application defined splash screen */
    char *key = jnlFile->canonicalHome;
    if (key == NULL) {
	key = jnlFile->jnlp_url;
    }
    *splash = getSplashFile(key);

    /* if none - use our default */
    if ((*splash == NULL) && (jnlFile->splashPref != SPLASH_CUSTOM_ONLY)) {
        getDefaultSplashFiles(FALSE, splash);
    }
}


