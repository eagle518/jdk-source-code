/*
 * @(#)awt_Desktop.c	1.3 05/07/18 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include <dlfcn.h>

typedef int gboolean;

gboolean (*gnome_url_show) (const char *url, void **error);

int init(){
    void *vfs_handle;
    void *gnome_handle;
    gboolean (*gnome_vfs_init) (void);
    const char *errmsg;

    vfs_handle = dlopen("libgnomevfs-2.so.0", RTLD_LAZY);
    if (vfs_handle == NULL) {
#ifdef INTERNAL_BUILD
        fprintf(stderr, "can not load libgnomevfs-2.so\n");
#endif
        return 0;
    }
    dlerror(); /* Clear errors */
    gnome_vfs_init = dlsym(vfs_handle, "gnome_vfs_init");
    if ((errmsg = dlerror()) != NULL) {
#ifdef INTERNAL_BUILD
        fprintf(stderr, "can not find symble gnome_vfs_init\n");
#endif
        return 0;
    }
    // call gonme_vfs_init()
    (*gnome_vfs_init)();

    gnome_handle = dlopen("libgnome-2.so.0", RTLD_LAZY);
    if (gnome_handle == NULL) {
#ifdef INTERNAL_BUILD
        fprintf(stderr, "can not load libgnome-2.so\n");
#endif
        return 0;
    }
    dlerror(); /* Clear errors */
    gnome_url_show = dlsym(gnome_handle, "gnome_url_show");
    if ((errmsg = dlerror()) != NULL) {
#ifdef INTERNAL_BUILD
        fprintf(stderr, "can not find symble gnome_url_show\n");
#endif
        return 0;
    }

    return 1;
}

/*
 * Class:     sun_awt_X11_XDesktopPeer
 * Method:    init
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_sun_awt_X11_XDesktopPeer_init
  (JNIEnv *env, jclass cls)
{
    int init_ok = init(); 
    return init_ok ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     sun_awt_X11_XDesktopPeer
 * Method:    gnome_url_show
 * Signature: (Ljava/lang/[B;)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_awt_X11_XDesktopPeer_gnome_1url_1show
  (JNIEnv *env, jobject obj, jbyteArray url_j)
{
    gboolean success;

    const char* url_c = (*env)->GetByteArrayElements(env, url_j, NULL);

    if (gnome_url_show == NULL) return JNI_FALSE;

    // call gnome_url_show(const char* , GError**)
    success = (*gnome_url_show)(url_c, NULL);

    (*env)->ReleaseByteArrayElements(env, url_j, (signed char*)url_c, 0);

    return success ? JNI_TRUE : JNI_FALSE;
}

