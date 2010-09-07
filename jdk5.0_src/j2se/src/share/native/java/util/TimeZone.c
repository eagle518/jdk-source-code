/*
 * @(#)TimeZone.c	1.10 04/01/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <string.h>

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "TimeZone_md.h"

#include "java_util_TimeZone.h"

/*
 * Gets the platform defined TimeZone ID
 */
JNIEXPORT jstring JNICALL
Java_java_util_TimeZone_getSystemTimeZoneID(JNIEnv *env, jclass ign, 
                                            jstring java_home, jstring country)
{
    const char *cname;
    const char *java_home_dir;
    char *javaTZ;

    if (java_home == NULL)
	return NULL;

    java_home_dir = JNU_GetStringPlatformChars(env, java_home, 0);
    if (java_home_dir == NULL)
        return NULL;

    if (country != NULL) {
	cname = JNU_GetStringPlatformChars(env, country, 0);
	/* ignore error cases for cname */
    } else {
	cname = NULL;
    }

    /*
     * Invoke platform dependent mapping function
     */
    javaTZ = findJavaTZ_md(java_home_dir, cname);

    free((void *)java_home_dir);
    if (cname != NULL) {
	free((void *)cname);
    }

    if (javaTZ != NULL) {
	jstring jstrJavaTZ = JNU_NewStringPlatform(env, javaTZ);
	free((void *)javaTZ);
        return jstrJavaTZ;
    }
    return NULL;
}

/*
 * Gets a GMT offset-based time zone ID (e.g., "GMT-08:00")
 */
JNIEXPORT jstring JNICALL
Java_java_util_TimeZone_getSystemGMTOffsetID(JNIEnv *env, jclass ign)
{
    char *id = getGMTOffsetID();
    jstring jstrID = NULL;

    if (id != NULL) {
	jstrID = JNU_NewStringPlatform(env, id);
	free((void *)id);
    }
    return jstrID;
}
