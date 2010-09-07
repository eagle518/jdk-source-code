/*
 * @(#)awt_UNIXToolkit.c	1.3 04/07/26
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jni.h>
#include "sun_awt_UNIXToolkit.h"

#ifndef HEADLESS
#include "GLXSurfaceData.h"
extern OGLContext *sharedContext;
#endif /* !HEADLESS */

/*
 * Class:     sun_awt_UNIXToolkit
 * Method:    readGTKIconData
 * Signature: (Ljava/lang/String;[Ljava/lang/String;)[B
 */
JNIEXPORT jbyteArray JNICALL
Java_sun_awt_UNIXToolkit_readGTKIconData(JNIEnv *env, jobject obj,
                            jstring path_obj, jobjectArray icon_names)
{
  char **args;
  char *path;
  int args_len;
  int i;

  /* Parse icon names, and fill out args */
  {
    jsize count;
    int j;

    i = 0;
    j = 1;
    count = (*env)->GetArrayLength(env, icon_names);
    args_len = count * 3 + 2;
    args = (char**) malloc(args_len * sizeof(char*));

    path = (*env)->GetStringUTFChars(env, path_obj, NULL);
    args[0] = path;

    while (i < count) {
      jstring name_obj;
      char *name, *orientation, *tmp;

      name_obj = (jstring) (*env)->GetObjectArrayElement(env, icon_names, i++);
      name = (*env)->GetStringUTFChars(env, name_obj, NULL);

      if (name == NULL) {
        return NULL;         /* also throws OutOfMemoryError */
      }

      tmp = (char*) malloc(strlen(name) + 1);
      strcpy(tmp, name);
      (*env)->ReleaseStringUTFChars(env, name_obj, name);

      /* name looks like 'gtk-add.3.ltr' */
      name = tmp;
      tmp = strrchr(name, '.');
      *tmp++ = '\0';
      orientation = tmp;
        
      tmp = strrchr(name, '.');
      *tmp++ = '\0';
      args[j++] = name;
      args[j++] = tmp;
      args[j++] = orientation;
    }
    args[j] = NULL;
  }

  /* spawn the process, and collect its output */
  {
    int pipefd[2];
    jbyte *buf, *p;
    int curlen, n;
    int blocklen = 4096;
    int buflen = 50000;
    jbyteArray res = NULL;

    pipe(pipefd);

    switch (vfork()) {
    case -1:
      close(pipefd[0]);
      close(pipefd[1]);
      return NULL;

    case 0:
      close(1);
      close(2);
      close(pipefd[0]);
      dup(pipefd[1]);
      execvp(path, args);
      /* Error launching gtkhelper */
      close(pipefd[1]);
      return NULL;

    default:
      close(pipefd[1]);
      buf = (char*) malloc(buflen);
      p = buf;

      do {
        if (p - buf + blocklen > buflen) {
          curlen = p - buf;
          buflen += buflen/2;
          buf = realloc(buf, buflen);
          p = buf + curlen;
        }
        n = read(pipefd[0], p, 4096);
        p += n;
        /* Make sure we do not block forever. 3 sec should be enough */
      } while (n > 0);

      if (n == 0) {
        curlen = p - buf;
        res = (*env)->NewByteArray(env, curlen);
        (*env)->SetByteArrayRegion(env, res, 0, curlen, buf);
      }

      /* Free all memory chunks */
      (*env)->ReleaseStringUTFChars(env, path_obj, path);
      free(buf);
      for (i=1; i<args_len; i+=3) {
        free(args[i]);
      }
      free(args);

      return res;
    }
  }
}

/*
 * Class:     sun_awt_UNIXToolkit
 * Method:    sync
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_UNIXToolkit_sync(JNIEnv *env, jobject this) 
{
#ifndef HEADLESS
    AWT_LOCK();
    if (sharedContext != NULL) {
        /* GLX has been properly initialized, so let's flush the GL pipe */
        j2d_glXWaitGL();
    }
    XSync(awt_display, False);
    AWT_UNLOCK();
#endif /* !HEADLESS */
}
