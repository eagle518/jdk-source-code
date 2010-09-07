/*
 * @(#)GnomeVfsWrapper.c	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "com_sun_deploy_association_utility_GnomeVfsWrapper.h"
#include "com_sun_deploy_association_utility_DesktopEntryFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <link.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include "gnomevfs.h"

#define DEF_DIR_PERM S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
#define DEF_FIL_PERM S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

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

JNIEXPORT jstring JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_getVersion
    (JNIEnv *env, jclass cl) {

    // get Gnome version info from /usr/lib/pkg-config/gnome-vfs-2.0.pc
    char* GNOEM_SETTING = "/usr/lib/pkgconfig/gnome-vfs-2.0.pc";
    int BUFSIZE = 1024, versionLen = 20;
    char buffer[BUFSIZE], version[versionLen];
    int gnomefd;
    char* key = "Version:";
    if((gnomefd = open(GNOEM_SETTING, O_RDONLY)) == -1){
        return 0;
     }
    read(gnomefd, buffer, BUFSIZE);
    close(gnomefd);
    char* pvalue = strstr(buffer, key);
    if(pvalue != NULL) {	  
        pvalue += strlen(key);  
        int i = 0;
	while(i < versionLen - 1) {
	    if(*pvalue == '\n'){
	        break;
	    }
	    else if(*pvalue != ' ' ){
	        version[i] = *pvalue;
	        i++;
	    }
	    pvalue++;
	}
	version[i] = 0;
    }
    return (*env)->NewStringUTF(env, version);
}

void gnome_workaround()
{
        //Ugly workaround 6267615 for gnome ,if root's home is "/"
        //However, gnome vfs will show us two warning like assert "uri != NULL
       if( 0 == getuid()) {
            struct passwd *pw;
            pw = getpwnam("root");
            if( NULL != pw &&  0 == strcmp(pw->pw_dir, "/") ){
                putenv("GNOME_VFS_VFOLDER_INFODIR=/.gnome2/vfolders");
            }
        }

}

JNIEXPORT jboolean JNICALL Java_com_sun_deploy_association_utility_GnomeVfsWrapper_openGNOMELibrary(JNIEnv *env, jclass cl)
{
    pLibGNOME = NULL;
#ifdef SOLARIS
    if( !dlopen("libgconf-2.so", RTLD_LAZY | RTLD_GLOBAL)
        || !dlopen("libxml2.so", RTLD_LAZY | RTLD_GLOBAL)) { //For solaris gnome
        return JNI_FALSE;
    }
#endif
    gnome_workaround();
    pLibGNOME = dlopen("libgnomevfs-2.so", RTLD_LAZY | RTLD_GLOBAL);
    if( NULL == pLibGNOME ){ 

        //There's only libgnomevfs-2.so.0 on lastest JDS release(linux). 
       	pLibGNOME = dlopen("libgnomevfs-2.so.0", RTLD_LAZY | RTLD_GLOBAL);
    }
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
    
    jws_gnome_vfs_get_file_info =
        (guint (*)(const char  *text_uri, void * info, gint n))dlsym(pLibGNOME, "gnome_vfs_get_file_info");
    if (jws_gnome_vfs_get_file_info == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_unlink =
        (guint (*)(const char *text_uri))dlsym(pLibGNOME, "gnome_vfs_unlink");
    if (jws_gnome_vfs_unlink == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_remove_directory =
        (guint (*)(const char *text_uri))dlsym(pLibGNOME, "gnome_vfs_remove_directory");
    if (jws_gnome_vfs_remove_directory == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_directory_open =
        (guint (*)(ghandle *handle, const char *text_uri, guint open_mode))dlsym(pLibGNOME, "gnome_vfs_directory_open");
    if (jws_gnome_vfs_directory_open == NULL) {

            return JNI_FALSE;
    }


    jws_gnome_vfs_directory_close =
        (guint (*)(ghandle *handle))dlsym(pLibGNOME, "gnome_vfs_directory_close");
    if (jws_gnome_vfs_directory_close == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_read =
        (guint (*)(ghandle *handle, gconstpointer buffer, GnomeVFSFileSize buf_size, GnomeVFSFileSize * read_size))dlsym(pLibGNOME, "gnome_vfs_read");
    if (jws_gnome_vfs_read == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_write =
        (guint (*)(ghandle *handle, gconstpointer buffer, GnomeVFSFileSize buf_size, GnomeVFSFileSize * write_size))dlsym(pLibGNOME, "gnome_vfs_write");
    if (jws_gnome_vfs_write == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_mkdir =
        (guint (*)(const char *text_uri, guint perm))dlsym(pLibGNOME, "gnome_vfs_make_directory");
    if (jws_gnome_vfs_mkdir == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_open =
        (guint (*)(ghandle *handle, const char *text_uri, guint open_mode))dlsym(pLibGNOME, "gnome_vfs_open");
    if (jws_gnome_vfs_open == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_close =
        (guint (*)(ghandle *handle))dlsym(pLibGNOME, "gnome_vfs_close");
    if (jws_gnome_vfs_close == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_create =
        (guint (*)(ghandle *, const char *, guint , gboolean , guint))dlsym(pLibGNOME, "gnome_vfs_create");
    if (jws_gnome_vfs_create == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_file_info_new =
        (gpointer (*)())dlsym(pLibGNOME, "gnome_vfs_file_info_new");
    if (jws_gnome_vfs_file_info_new == NULL) {

            return JNI_FALSE;
    }

    jws_gnome_vfs_file_info_unref=
        (void (*)(gpointer))dlsym(pLibGNOME, "gnome_vfs_file_info_unref");
    if (jws_gnome_vfs_file_info_unref == NULL) {

            return JNI_FALSE;
    }


    jws_gnome_vfs_result_to_string =
        (const char * (*)(guint n))dlsym(pLibGNOME, "gnome_vfs_result_to_string");
    if (jws_gnome_vfs_result_to_string == NULL) {

            return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT jstring JNICALL Java_com_sun_deploy_association_utility_DesktopEntryFile_gnome_1vfs_1read_1file
  (JNIEnv *env, jclass cl, jstring uri)
{
    jws_gnome_vfs_init();
    GnomeVFSResult result;
    char buffer[1024];//FIXME, should read enough bytes as the same as the file size
    GnomeVFSFileSize bytes_read = 0;
    GnomeVFSFileSize read_count = 0;
    GnomeVFSFileSize buf_size = 1024;
    ghandle handle;

    const char* text_uri = (*env)->GetStringUTFChars(env, uri, JNI_FALSE);

    result = jws_gnome_vfs_open (&handle, text_uri, GNOME_VFS_OPEN_READ);
    (*env)->ReleaseStringUTFChars(env, uri, text_uri);  

    if (result != GNOME_VFS_OK) {
        jws_throw_by_name(env, "java/io/IOException", jws_gnome_vfs_result_to_string (result));
        return NULL;
    }

    while (result == GNOME_VFS_OK) {
        result = jws_gnome_vfs_read (handle, buffer,  buf_size,  &bytes_read);
        read_count += bytes_read;
        if( read_count >= buf_size){
              break;
        }
        if(bytes_read == 0)
            break;
    }
    
    if (result != GNOME_VFS_OK) {
        jws_throw_by_name(env, "java/io/IOException", jws_gnome_vfs_result_to_string (result));
        result = jws_gnome_vfs_close (handle);
        return NULL;
    }

    result = jws_gnome_vfs_close (handle);
    jstring content = (*env)->NewStringUTF(env, buffer);
    return content;
}


JNIEXPORT jboolean JNICALL Java_com_sun_deploy_association_utility_DesktopEntryFile_gnome_1vfs_1mkdir
  (JNIEnv *env, jclass cl, jstring uri)
{
    jws_gnome_vfs_init();
    GnomeVFSResult result;

    const char* text_uri = (*env)->GetStringUTFChars(env, uri, JNI_FALSE);

    // perm: rwxr_xr_x
    result = jws_gnome_vfs_mkdir(text_uri, DEF_DIR_PERM); 
                      

    (*env)->ReleaseStringUTFChars(env, uri, text_uri);  

    if (result != GNOME_VFS_OK) {
        jws_throw_by_name(env, "java/io/IOException", jws_gnome_vfs_result_to_string (result));
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_sun_deploy_association_utility_DesktopEntryFile_gnome_1vfs_1file_1exists
  (JNIEnv *env, jclass cl, jstring uri)
{
    jws_gnome_vfs_init();
    GnomeVFSResult result;
    gpointer info;

    const char* text_uri = (*env)->GetStringUTFChars(env, uri, JNI_FALSE);

    info = jws_gnome_vfs_file_info_new();

    result = jws_gnome_vfs_get_file_info(text_uri, info, 1 << 3 );

    (*env)->ReleaseStringUTFChars(env, uri, text_uri);  
    jws_gnome_vfs_file_info_unref(info);

    if (result != GNOME_VFS_OK) {
        return JNI_FALSE;
    }

    return JNI_TRUE;

}

JNIEXPORT jboolean JNICALL Java_com_sun_deploy_association_utility_DesktopEntryFile_gnome_1vfs_1write_1file
  (JNIEnv *env, jclass cl, jstring uri, jstring content)
{
    jws_gnome_vfs_init();
    GnomeVFSResult result;
    const char * buffer;
    GnomeVFSFileSize bytes_write = 0;
    GnomeVFSFileSize write_count = 0;
    GnomeVFSFileSize buf_size = 128;
    ghandle handle;
    const char * msg;

    const char* text_uri = (*env)->GetStringUTFChars(env, uri, JNI_FALSE);


    // If file not exists, create it.
    if( JNI_FALSE == Java_com_sun_deploy_association_utility_DesktopEntryFile_gnome_1vfs_1file_1exists(
        env, cl, uri)){
         result = jws_gnome_vfs_create(&handle,
                                          text_uri, 
                                          GNOME_VFS_OPEN_WRITE, 
                                          0, //FALSE  Force creation
                                          DEF_FIL_PERM
                                          );
        if (result != GNOME_VFS_OK) {
           (*env)->ReleaseStringUTFChars(env, uri, text_uri);  
           jws_throw_by_name(env, "java/io/IOException", jws_gnome_vfs_result_to_string (result));
           return JNI_FALSE;
        }
    }else{
        result = jws_gnome_vfs_open (&handle, text_uri, GNOME_VFS_OPEN_WRITE);
        if (result != GNOME_VFS_OK) {
            (*env)->ReleaseStringUTFChars(env, uri, text_uri);  
            jws_throw_by_name(env, "java/io/IOException", jws_gnome_vfs_result_to_string (result));
            return JNI_FALSE;
        }
    }
    (*env)->ReleaseStringUTFChars(env, uri, text_uri);  

    buffer = (*env)->GetStringUTFChars(env, content, JNI_FALSE);
    buf_size = (GnomeVFSFileSize)(*env)->GetStringUTFLength(env, content);
    while (result == GNOME_VFS_OK) {
       result = jws_gnome_vfs_write (handle, buffer,  buf_size,  &bytes_write);
       write_count += bytes_write;
       if( write_count >= buf_size){
             break;
       }
    }
    
    (*env)->ReleaseStringUTFChars(env, content, buffer);  

    if (result != GNOME_VFS_OK) {
        msg = jws_gnome_vfs_result_to_string (result);
        result = jws_gnome_vfs_close (handle);
        jws_throw_by_name(env, "java/io/IOException", msg);
        return JNI_FALSE;
    }

    result = jws_gnome_vfs_close (handle);

    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_sun_deploy_association_utility_DesktopEntryFile_gnome_1vfs_1delete_1file
  (JNIEnv *env, jclass cl, jstring uri)
{

    jws_gnome_vfs_init();
    GnomeVFSResult result;
    ghandle handle;
    jboolean ret_val;

    const char* text_uri = (*env)->GetStringUTFChars(env, uri, JNI_FALSE);

    //not exists, return true
    if( JNI_FALSE == Java_com_sun_deploy_association_utility_DesktopEntryFile_gnome_1vfs_1file_1exists(
        env, cl, uri)){
        (*env)->ReleaseStringUTFChars(env, uri, text_uri);  
        return JNI_TRUE;
    }

    result = jws_gnome_vfs_directory_open(&handle, text_uri, 0);
    if (result == GNOME_VFS_OK) {
        jws_gnome_vfs_directory_close(handle);
        result = jws_gnome_vfs_remove_directory(text_uri);
        if (result == GNOME_VFS_OK) {
            ret_val = JNI_TRUE;
        }else{
            ret_val = JNI_FALSE;
        }
        (*env)->ReleaseStringUTFChars(env, uri, text_uri);  
        return ret_val;
    }

    result = jws_gnome_vfs_open (&handle, text_uri, GNOME_VFS_OPEN_WRITE);
    if (result == GNOME_VFS_OK) {
        jws_gnome_vfs_close(handle);
        result = jws_gnome_vfs_unlink(text_uri);
        if (result == GNOME_VFS_OK) {
            ret_val = JNI_TRUE;
        }else{
            ret_val = JNI_FALSE;
        }
        (*env)->ReleaseStringUTFChars(env, uri, text_uri);  
        return ret_val;
    }

    (*env)->ReleaseStringUTFChars(env, uri, text_uri);  

    return JNI_FALSE;

}

JNIEXPORT void JNICALL Java_com_sun_deploy_association_utility_DesktopEntryFile_ensure_1load_1gnome_1vfs_1lib
  (JNIEnv *env, jclass cl){
    if (pLibGNOME == NULL) {


        Java_com_sun_deploy_association_utility_GnomeVfsWrapper_openGNOMELibrary(env, cl);
        Java_com_sun_deploy_association_utility_GnomeVfsWrapper_initGNOMELibrary(env, cl);
    }
}

void jws_throw_by_name(JNIEnv *env, const char *name, const char *msg)
{
    jclass cls = (*env)->FindClass(env, name);

    if (cls != 0) /* Otherwise an exception has already been thrown */
        (*env)->ThrowNew(env, cls, msg);
}




