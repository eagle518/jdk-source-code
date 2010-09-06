/*
 * @(#)GnomeVfsWrapper.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "com_sun_deploy_association_utility_GnomeVfsWrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <link.h>
#include "gnomevfs.h"

JNIEXPORT jstring JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_gnome_1vfs_1get_1mime_1type
  (JNIEnv *env, jclass cl, jstring url) {
  jws_gnome_vfs_init();

  const char* urlStr = (*env)->GetStringUTFChars(env, url, JNI_FALSE);
  const char* mimeTypeStr = jws_gnome_vfs_get_mime_type(urlStr);
  (*env)->ReleaseStringUTFChars(env, url, urlStr);

  if(mimeTypeStr == NULL) {
    return NULL;
  } else {
    jstring mimeType = (*env)->NewStringUTF(env, mimeTypeStr);
    return mimeType;
  }
}  


JNIEXPORT jstring JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_gnome_1vfs_1mime_1get_1value
  (JNIEnv *env, jclass cl, jstring mimeType, jstring key) {
  jws_gnome_vfs_init();	  

  const char* mimeTypeStr = (*env)->GetStringUTFChars(env, mimeType, JNI_FALSE);
  const char* keyStr = (*env)->GetStringUTFChars(env, key, JNI_FALSE);
  const char* keyValueStr = jws_gnome_vfs_mime_get_value(mimeTypeStr, keyStr);
  (*env)->ReleaseStringUTFChars(env, mimeType, mimeTypeStr);
  (*env)->ReleaseStringUTFChars(env, key, keyStr);  

  if(keyValueStr == NULL) {
    return NULL;
  } else {  
    jstring keyValue = (*env)->NewStringUTF(env, keyValueStr); 
    return keyValue;
  }   
}
  
JNIEXPORT jstring JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_gnome_1vfs_1mime_1get_1description
  (JNIEnv *env, jclass cl, jstring mimeType) {
  jws_gnome_vfs_init();

  const char* mimeTypeStr = (*env)->GetStringUTFChars(env, mimeType, JNI_FALSE);
  const char* descStr = jws_gnome_vfs_mime_get_description(mimeTypeStr);
  (*env)->ReleaseStringUTFChars(env, mimeType, mimeTypeStr);

  if(descStr == NULL) {
    return NULL;
  } else {
    jstring desc = (*env)->NewStringUTF(env, descStr);
    return desc;
  }
}

JNIEXPORT jstring JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_gnome_1vfs_1mime_1get_1icon
  (JNIEnv *env, jclass cl, jstring mimeType) {
  jws_gnome_vfs_init();

  const char* mimeTypeStr = (*env)->GetStringUTFChars(env, mimeType, JNI_FALSE);
  const char* iconFileStr = jws_gnome_vfs_mime_get_icon(mimeTypeStr);
  (*env)->ReleaseStringUTFChars(env, mimeType, mimeTypeStr);

  if(iconFileStr == NULL) {
    return NULL;
  } else {
    jstring iconFile = (*env)->NewStringUTF(env, iconFileStr);
    return iconFile;
  }
}	  

JNIEXPORT jobjectArray JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_gnome_1vfs_1mime_1get_1key_1list
  (JNIEnv *env, jclass cl, jstring mimeType) {
  int i = 0;
  jws_gnome_vfs_init();

  const char* mimeTypeStr = (*env)->GetStringUTFChars(env, mimeType, JNI_FALSE);
  GList* keyList = jws_gnome_vfs_mime_get_key_list(mimeTypeStr);
  (*env)->ReleaseStringUTFChars(env, mimeType, mimeTypeStr);

  int listLen;
  if (keyList != NULL) {
    listLen = jws_g_list_length(keyList);
  } else {
    listLen = 0;
  }
  
  if(listLen == 0) {
    return NULL;
  } else {
    jobjectArray retArray;
    const char* keyStr;
    retArray = (jobjectArray)(*env)->NewObjectArray(env, listLen, (*env)->FindClass(env, "java/lang/String"),
		    				 (*env)->NewStringUTF(env, ""));
    for(i = 0; i < listLen; i++) {
      keyStr = (const char*)jws_g_list_nth_data(keyList, i);
      (*env)->SetObjectArrayElement(env, retArray, i, (*env)->NewStringUTF(env, keyStr));
    }
  
    return retArray;
  }    
}

JNIEXPORT jstring JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_gnome_1vfs_1mime_1get_1default_1application_1command
  (JNIEnv *env, jclass cl, jstring mimeType) {
  jws_gnome_vfs_init();

  const char* mimeTypeStr = (*env)->GetStringUTFChars(env, mimeType, JNI_FALSE);
  GnomeVFSMimeApplication *mimeApp = jws_gnome_vfs_mime_get_default_application(mimeTypeStr);
  (*env)->ReleaseStringUTFChars(env, mimeType, mimeTypeStr);

  if(mimeApp == NULL) {
    return NULL;
  } else {
    const char* mimeAppCommandStr = mimeApp->command;
    if(mimeAppCommandStr == NULL) {
      return NULL;
    } else {      
      jstring mimeAppCommand = (*env)->NewStringUTF(env, mimeAppCommandStr);
      return mimeAppCommand;
    }
  }
}

JNIEXPORT jobjectArray JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_gnome_1vfs_1get_1registered_1mime_1types
  (JNIEnv *env, jclass cl) {
  int i = 0;
  jws_gnome_vfs_init();

  GList* mimeTypeList = jws_gnome_vfs_get_registered_mime_types();

  int listLen;
  if (mimeTypeList != NULL) {
    listLen = jws_g_list_length(mimeTypeList);
  } else {
    listLen = 0;
  }
  if(listLen == 0) {
    return NULL;
  } else {
    jobjectArray retArray;
    const char* mimeTypeStr;
    retArray = (jobjectArray)(*env)->NewObjectArray(env, listLen, (*env)->FindClass(env, "java/lang/String"),
			                                                   (*env)->NewStringUTF(env, ""));
    for(i = 0; i < listLen; i++) {
      mimeTypeStr = (const char*)jws_g_list_nth_data(mimeTypeList, i);
      if(mimeTypeStr != NULL) {
            (*env)->SetObjectArrayElement(env, retArray, i, (*env)->NewStringUTF(env, mimeTypeStr));
      }
    }

   return retArray;
  }
}	 

JNIEXPORT jobjectArray JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_gnome_1vfs_1mime_1get_1extensions_1list
    (JNIEnv *env, jclass cl, jstring mimeType) {
  int i = 0;
  jws_gnome_vfs_init();
  
  const char* mimeTypeStr = (*env)->GetStringUTFChars(env, mimeType, JNI_FALSE);
  GList* extList = jws_gnome_vfs_mime_get_extensions_list(mimeTypeStr);
  (*env)->ReleaseStringUTFChars(env, mimeType, mimeTypeStr);

  int listLen;
  if (extList != NULL) {
    listLen = jws_g_list_length(extList);
  } else {
    listLen = 0;
  }
  if(listLen == 0) {
    return NULL;
  } else {
    jobjectArray retArray;
    const char* extensionStr;
    retArray = (jobjectArray)(*env)->NewObjectArray(env, listLen, (*env)->FindClass(env, "java/lang/String"),			                                                 (*env)->NewStringUTF(env, ""));
    for(i = 0; i < listLen; i++) {
      extensionStr = (const char*)jws_g_list_nth_data(extList, i);
      if(extensionStr != NULL) {
        (*env)->SetObjectArrayElement(env, retArray, i, (*env)->NewStringUTF(env, extensionStr));
      }
    }

   return retArray;
  }  
}	    

JNIEXPORT jstring JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_getenv
    (JNIEnv *env, jclass cl, jstring envName) {
  const char* envNameStr = (*env)->GetStringUTFChars(env, envName, JNI_FALSE);
  const char* envValueStr = getenv(envNameStr);
  (*env)->ReleaseStringUTFChars(env, envName, envNameStr);

  return (envValueStr == NULL) ? 0 : (*env)->NewStringUTF(env, envValueStr);
}

JNIEXPORT jstring JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_getenv2
    (JNIEnv *env, jclass cl, jstring envName) {
  const char* envNameStr = (*env)->GetStringUTFChars(env, envName, JNI_FALSE);
  const char* envValueStr = getenv(envNameStr);
  (*env)->ReleaseStringUTFChars(env, envName, envNameStr);

  return (envValueStr == NULL) ? 0 : (*env)->NewStringUTF(env, envValueStr);
}

JNIEXPORT jboolean JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_openGNOMELibrary(JNIEnv *env, jclass cl)
{
    pLibGNOME = dlopen("libgnomevfs-2.so", RTLD_LAZY | RTLD_GLOBAL);

    if (pLibGNOME == NULL) {    
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_closeGNOMELibrary(JNIEnv *env, jclass cl)
{
    if (pLibGNOME != NULL) {
      if (dlclose(pLibGNOME)) {
	// error
      }
    }
}

JNIEXPORT jboolean JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_initGNOMELibrary(JNIEnv *env, jclass cl)
{ 

    if (pLibGNOME == NULL) {
        return JNI_FALSE;
    }

    jws_gnome_vfs_init =
        (gboolean (*)(void))dlsym(pLibGNOME, "gnome_vfs_init");
    if (jws_gnome_vfs_init == NULL) {   

            return JNI_FALSE;
    }

    jws_gnome_vfs_get_mime_type =
        (char* (*)(const char *text_uri))dlsym(pLibGNOME, "gnome_vfs_get_mime_type");
    if (jws_gnome_vfs_get_mime_type == NULL) {   

            return JNI_FALSE;
    }

    jws_gnome_vfs_mime_get_value =
        (const char* (*)(const char *mime_type, const char *key))dlsym(pLibGNOME, "gnome_vfs_mime_get_value");
    if (jws_gnome_vfs_mime_get_value == NULL) {  

            return JNI_FALSE;
    }

    jws_gnome_vfs_mime_get_icon =
        (const char* (*)(const char *mime_type))dlsym(pLibGNOME, "gnome_vfs_mime_get_icon");
    if (jws_gnome_vfs_mime_get_icon == NULL) {  
 
            return JNI_FALSE;
    }

    jws_gnome_vfs_mime_get_description =
        (const char* (*)(const char *mime_type))dlsym(pLibGNOME, "gnome_vfs_mime_get_description");
    if (jws_gnome_vfs_mime_get_description == NULL) { 

            return JNI_FALSE;
    }

    jws_gnome_vfs_mime_get_key_list =
        (GList* (*)(const char *mime_type))dlsym(pLibGNOME, "gnome_vfs_mime_get_key_list");
    if (jws_gnome_vfs_mime_get_key_list == NULL) {   

            return JNI_FALSE;
    }
    
    jws_gnome_vfs_mime_get_default_application =
        (GnomeVFSMimeApplication* (*)(const char *mime_type))dlsym(pLibGNOME, "gnome_vfs_mime_get_default_application");
    if (jws_gnome_vfs_mime_get_default_application == NULL) { 

            return JNI_FALSE;
    }

    jws_gnome_vfs_get_registered_mime_types =
        (GList* (*)(void))dlsym(pLibGNOME, "gnome_vfs_get_registered_mime_types");
    if (jws_gnome_vfs_get_registered_mime_types == NULL) {  

            return JNI_FALSE;
    }

    jws_gnome_vfs_mime_get_extensions_list =
        (GList* (*)(const char *mime_type))dlsym(pLibGNOME, "gnome_vfs_mime_get_extensions_list");
    if (jws_gnome_vfs_mime_get_extensions_list == NULL) { 
 
            return JNI_FALSE;
    }
    
    jws_g_list_nth_data =
        (gpointer (*)(GList *list, guint n))dlsym(pLibGNOME, "g_list_nth_data");
    if (jws_g_list_nth_data == NULL) {  
 
            return JNI_FALSE;
    }

    jws_g_list_length =
        (guint (*)(GList *list))dlsym(pLibGNOME, "g_list_length");
    if (jws_g_list_length == NULL) {

            return JNI_FALSE;
    }
    
    
    

    return JNI_TRUE;
}
