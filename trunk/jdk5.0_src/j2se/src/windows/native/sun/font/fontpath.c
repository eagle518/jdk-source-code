/*
 * @(#)fontpath.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *
 *  Author: Alex D. Gelfenbain
 *
 */

#include <windows.h>
#include <stdio.h>

#include <jni.h>
#include <jni_util.h>
#include <sun_font_FontManager.h>

#define BSIZE (max(512, MAX_PATH+1))


JNIEXPORT jstring JNICALL Java_sun_font_FontManager_getFontPath(JNIEnv *env, jclass obj, jboolean noType1)
{
    char windir[BSIZE];
    char sysdir[BSIZE];
    char fontpath[BSIZE*2];
    char *end;

    /* Locate fonts directories relative to the Windows System directory.
     * If Windows System location is different than the user's window
     * directory location, as in a shared Windows installation,
     * return both locations as potential font directories
     */
    GetSystemDirectory(sysdir, BSIZE);
    end = strrchr(sysdir,'\\');
    if (end && (stricmp(end,"\\System") || stricmp(end,"\\System32"))) {
        *end = 0;
         strcat(sysdir, "\\Fonts");
    }

    GetWindowsDirectory(windir, BSIZE);
    if (strlen(windir) > BSIZE-7) {
        *windir = 0;
    } else {
        strcat(windir, "\\Fonts");
    }
    
    strcpy(fontpath,sysdir);
    if (stricmp(sysdir,windir)) {
        strcat(fontpath,";");
        strcat(fontpath,windir);
    }

    return JNU_NewStringPlatform(env, fontpath);
}
