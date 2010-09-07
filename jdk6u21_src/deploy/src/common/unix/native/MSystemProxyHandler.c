/*
 * @(#)MSystemProxyHandler.c	1.1 04/01/19
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __linux__
#define _GNU_SOURCE
#include <string.h>
#else
#include <strings.h>
#endif

#include "jni.h"
#include "com_sun_deploy_net_proxy_MSystemProxyHandler.h"
#include <dlfcn.h>
#include <stdio.h>

/* Whenever this code is changed, please make sure we change the same
   file in j2se workspace as well (and vice versa)

   j2se/src/solaris/native/sun/net/spi/DefaultProxySelector.c
*/

/**
 * These functions are used by the com_sun_deploy_net_proxy_MSystemProxyHandler class
 * to access some platform specific settings.
 * This is the Solaris/Linux Gnome 2.x code using the GConf-2 library.
 * Everything is loaded dynamically so no hard link with any library exists.
 * The GConf-2 settings used are:
 * - /system/http_proxy/use_http_proxy		boolean
 * - /system/http_proxy/use_authentcation	boolean
 * - /system/http_proxy/host			string
 * - /system/http_proxy/authentication_user	string
 * - /system/http_proxy/authentication_password	string
 * - /system/http_proxy/port			int
 * - /system/proxy/socks_host			string
 * - /system/proxy/mode				string
 * - /system/proxy/ftp_host			string
 * - /system/proxy/secure_host			string
 * - /system/proxy/socks_port			int
 * - /system/proxy/ftp_port			int
 * - /system/proxy/secure_port			int
 * - /system/proxy/no_proxy_for			list
 * - /system/proxy/gopher_host			string
 * - /system/proxy/gopher_port			int
 */

/**
 * @seealso sun.net.spi.DefaultProxySelector
 */
typedef void* gconf_client_get_default_func();
typedef char* gconf_client_get_string_func(void *, char *, void**);
typedef int   gconf_client_get_int_func(void*, char *, void**);
typedef int   gconf_client_get_bool_func(void*, char *, void**);
typedef int   gconf_init_func(int, char**, void**);
typedef void  g_type_init_func ();
gconf_client_get_default_func* my_get_default_func = NULL;
gconf_client_get_string_func* my_get_string_func = NULL;
gconf_client_get_int_func* my_get_int_func = NULL;
gconf_client_get_bool_func* my_get_bool_func = NULL;
gconf_init_func* my_gconf_init_func = NULL;
g_type_init_func* my_g_type_init_func = NULL;

static int gconf_ver = 0;
static void* gconf_client = NULL;

#define CHECK_NULL(X) { if ((X) == NULL) { fprintf (stderr,"JNI errror at line %d\n", __LINE__); return 0; }} 

/*
 * Class:     
 * Method:    init
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL 
Java_com_sun_deploy_net_proxy_MSystemProxyHandler_init(JNIEnv *env, jclass clazz) {

  /**
   * Let's try to load le GConf-2 library
   */
  if (dlopen("/usr/lib/libgconf-2.so", RTLD_GLOBAL | RTLD_LAZY) != NULL ||
      dlopen("/usr/lib/libgconf-2.so.4", RTLD_GLOBAL | RTLD_LAZY) != NULL) {
    gconf_ver = 2;
  } 
  if (gconf_ver > 0) {
    /*
     * Now let's get pointer to the functions we need.
     */
    my_g_type_init_func = (g_type_init_func*) dlsym(RTLD_DEFAULT, "g_type_init");
    my_get_default_func = (gconf_client_get_default_func*) dlsym(RTLD_DEFAULT, "gconf_client_get_default");
    if (my_g_type_init_func != NULL && my_get_default_func != NULL) {
      /**
       * Try to connect to GConf.
       */
      (*my_g_type_init_func)();
      gconf_client = (*my_get_default_func)();
      if (gconf_client != NULL) {
	my_get_string_func = (gconf_client_get_string_func*) dlsym(RTLD_DEFAULT, "gconf_client_get_string");
	my_get_int_func = (gconf_client_get_int_func*) dlsym(RTLD_DEFAULT, "gconf_client_get_int");
	my_get_bool_func = (gconf_client_get_bool_func*) dlsym(RTLD_DEFAULT, "gconf_client_get_bool");
	if (my_get_int_func != NULL && my_get_string_func != NULL &&
	    my_get_bool_func != NULL) {
	  /**
	   * We did get all we need. Let's enable the System Proxy Settings.
	   */
	  return JNI_TRUE;
	}
      }
    }
  }
  return JNI_FALSE;
}


/*
 * Class:     
 * Method:    getSystemProxy
 * Signature: ([Ljava/lang/String;Ljava/lang/String;)Ljava/net/Proxy;
 */
JNIEXPORT jstring JNICALL 
Java_com_sun_deploy_net_proxy_MSystemProxyHandler_getSystemProxy(JNIEnv *env,
						     jobject this,
						     jstring proto,
						     jstring host)
{
  char *phost = NULL;
  char *mode = NULL;
  int pport = 0;
  int use_proxy;
  const char* urlhost;
  const char *cproto;
  jboolean isCopy;

  if (gconf_ver > 0) {
    if (gconf_client == NULL) {
      (*my_g_type_init_func)(); 
      gconf_client = (*my_get_default_func)();
    }
    if (gconf_client != NULL) {
      cproto = (*env)->GetStringUTFChars(env, proto, &isCopy);
      if (cproto != NULL) {
	/**
	 * We will have to check protocol by protocol as they do use different
	 * entries.
	 */

	/**
	 * HTTP:
	 * /system/http_proxy/use_http_proxy (boolean)
	 * /system/http_proxy/host (string)
	 * /system/http_proxy/port (integer)
	 */
	if (strcasecmp(cproto, "http") == 0) {
	  use_proxy = (*my_get_bool_func)(gconf_client, "/system/http_proxy/use_http_proxy", NULL);
	  if (use_proxy) {
	    phost = (*my_get_string_func)(gconf_client, "/system/http_proxy/host", NULL);
	    pport = (*my_get_int_func)(gconf_client, "/system/http_proxy/port", NULL);
	  }
	}

	/**
	 * HTTPS:
	 * /system/proxy/mode (string) [ "manual" means use proxy settings ]
	 * /system/proxy/secure_host (string)
	 * /system/proxy/secure_port (integer)
	 */
	if (strcasecmp(cproto, "https") == 0) {
	  mode =  (*my_get_string_func)(gconf_client, "/system/proxy/mode", NULL);
	  if (mode != NULL && (strcasecmp(mode,"manual") == 0)) {
	    phost = (*my_get_string_func)(gconf_client, "/system/proxy/secure_host", NULL);
	    pport = (*my_get_int_func)(gconf_client, "/system/proxy/secure_port", NULL);
	    use_proxy = (phost != NULL);
	  }
	}

	/**
	 * FTP:
	 * /system/proxy/mode (string) [ "manual" means use proxy settings ]
	 * /system/proxy/ftp_host (string)
	 * /system/proxy/ftp_port (integer)
	 */
	if (strcasecmp(cproto, "ftp") == 0) {
	  mode =  (*my_get_string_func)(gconf_client, "/system/proxy/mode", NULL);
	  if (mode != NULL && (strcasecmp(mode,"manual") == 0)) {
	    phost = (*my_get_string_func)(gconf_client, "/system/proxy/ftp_host", NULL);
	    pport = (*my_get_int_func)(gconf_client, "/system/proxy/ftp_port", NULL);
	    use_proxy = (phost != NULL);
	  }
	}

	/**
	 * GOPHER:
	 * /system/proxy/mode (string) [ "manual" means use proxy settings ]
	 * /system/proxy/gopher_host (string)
	 * /system/proxy/gopher_port (integer)
	 */
	if (strcasecmp(cproto, "gopher") == 0) {
	  mode =  (*my_get_string_func)(gconf_client, "/system/proxy/mode", NULL);
	  if (mode != NULL && (strcasecmp(mode,"manual") == 0)) {
	    phost = (*my_get_string_func)(gconf_client, "/system/proxy/gopher_host", NULL);
	    pport = (*my_get_int_func)(gconf_client, "/system/proxy/gopher_port", NULL);
	    use_proxy = (phost != NULL);
	  }
	}

	/**
	 * SOCKS:
	 * /system/proxy/mode (string) [ "manual" means use proxy settings ]
	 * /system/proxy/socks_host (string)
	 * /system/proxy/socks_port (integer)
	 */
	if (strcasecmp(cproto, "socks") == 0) {
	  mode =  (*my_get_string_func)(gconf_client, "/system/proxy/mode", NULL);
	  if (mode != NULL && (strcasecmp(mode,"manual") == 0)) {
	    phost = (*my_get_string_func)(gconf_client, "/system/proxy/socks_host", NULL);
	    pport = (*my_get_int_func)(gconf_client, "/system/proxy/socks_port", NULL);
	    use_proxy = (phost != NULL);
	  }
	}

	if (isCopy == JNI_TRUE)
	  (*env)->ReleaseStringUTFChars(env, proto, cproto);

	if (use_proxy && (phost != NULL)) {
	  jstring jhost;
	  char *noproxyfor;
	  char *s;

	  /**
	   * check for the exclude list (aka "No Proxy For" list).
	   * It's a list of comma separated suffixes (e.g. domain name).
	   */
	  noproxyfor = (*my_get_string_func)(gconf_client, "/system/proxy/no_proxy_for", NULL);
	  if (noproxyfor != NULL) {
	    char *tmpbuf[512];

	    s = strtok_r(noproxyfor, ", ", tmpbuf);
	    urlhost = (*env)->GetStringUTFChars(env, host, &isCopy);
	    if (urlhost != NULL) {
	      while (s != NULL && strlen(s) <= strlen(urlhost)) {
		if (strcasecmp(urlhost+(strlen(urlhost) - strlen(s)), s) == 0) {
		  /**
		   * the URL host name matches with one of the sufixes,
		   * therefore we have to use a direct connection.
		   */
		  use_proxy = 0;
		  break;
		}
		s = strtok_r(NULL, ", ", tmpbuf);
	      }
	      if (isCopy == JNI_TRUE)
		(*env)->ReleaseStringUTFChars(env, host, urlhost);
	    }
	  }
	  if (use_proxy) {
	    char proxy[512];
	    snprintf(proxy,sizeof(proxy),"%s:%d", phost,pport);
	    return (*env)-> NewStringUTF(env,proxy);	
	  }
	}
      }
    }
  }
  return NULL;
}

