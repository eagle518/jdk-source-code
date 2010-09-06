
/*
 * @(#)splashFile.c	1.9 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "launchFile.h"
#include "configurationFile.h"

/*
 * Locates the jpeg files to be used in the splash screen.
 *
 */

static char path1[MAXPATHLEN];
static char path2[MAXPATHLEN];


void getDefaultSplashFiles(int playerMode, char **splash1, char **splash2) {

    char *name = "miniSplash." ;
    
    sprintf(path1, "%s%c%s%s", sysGetJavawsResourcesLib(),
        FILE_SEPARATOR, name, sysGetSplashExtension());
    *splash1 = path1;

    *splash2 = NULL;

}

getAppSplashFiles(JNLFile *jnlFile, char **splash1, char **splash2) {
    /* Try to get application defined splash screen */
    *splash1 = getSplashFile(jnlFile->canonicalHome);
    *splash2 = NULL;

    /* if none - use our default */
    if ((*splash1 == NULL) || (!endsWith(*splash1, sysGetSplashExtension()))) {

        getDefaultSplashFiles(FALSE, splash1, splash2);
    }
}

int endsWith(char *str1, char *str2) {
    if ((str1 != NULL) && (str2 != NULL)) {
        size_t len1,len2;
        len1 = strlen(str1);
        len2 = strlen(str2);
        if (len1 >= len2) {
            if (strcmp(str1+(len1-len2), str2) == 0) {
                return TRUE;
            }
        }
    }
    return FALSE;
}
    



