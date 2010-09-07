/*
 * @(#)ResolverConfigurationImpl.c	1.4 03/12/19 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#ifdef __solaris__
#include <sys/systeminfo.h>
#include <strings.h>
#endif

#ifdef __linux__
#include <string.h>
#endif

#include "jni.h"

#ifndef MAXDNAME
#define MAXDNAME		1025
#endif


/*
 * Class:     sun_net_dns_ResolverConfgurationImpl
 * Method:    localDomain0
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_net_dns_ResolverConfigurationImpl_localDomain0(JNIEnv *env, jclass cls)
{
    /*
     * On Solaris the LOCALDOMAIN environment variable has absolute
     * priority.
     */
#ifdef __solaris__
    {
	char *cp = getenv("LOCALDOMAIN");
	if (cp != NULL) {
	    jstring s = (*env)->NewStringUTF(env, cp);
	    return s;
	}
    }
#endif
    return (jstring)NULL;
}

/*
 * Class:     sun_net_dns_ResolverConfgurationImpl
 * Method:    loadConfig0
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_net_dns_ResolverConfigurationImpl_fallbackDomain0(JNIEnv *env, jclass cls)
{
    char buf[MAXDNAME];

    /*
     * On Solaris if domain or search directives aren't specified
     * in /etc/resolv.conf then sysinfo or gethostname is used to
     * determine the domain name.
     * 
     * On Linux if domain or search directives aren't specified
     * then gethostname is used.
     */

#ifdef __solaris__
    {
	int ret = sysinfo(SI_SRPC_DOMAIN, buf, sizeof(buf));

	if ((ret > 0) && (ret<sizeof(buf))) {
	    char *cp;
	    jstring s;

	    if (buf[0] == '+') {
		buf[0] = '.';
	    }
	    cp = strchr(buf, '.');
	    if (cp == NULL) {
		s = (*env)->NewStringUTF(env, buf);
	    } else {
		s = (*env)->NewStringUTF(env, cp+1);
	    }
	    return s;
	}
    }
#endif

    if (gethostname(buf, sizeof(buf)) == 0) {
	char *cp;

	/* gethostname doesn't null terminate if insufficient space */
	buf[sizeof(buf)-1] = '\0';

 	cp = strchr(buf, '.');
	if (cp != NULL) {
	    jstring s = (*env)->NewStringUTF(env, cp+1);
	    return s;
	}
    }

    return (jstring)NULL;
}

