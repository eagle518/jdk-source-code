/*
 * @(#)JavaVM.c	1.8 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "JavaVM.h"
#include <dlfcn.h>
#include <link.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// Private typedef for function pointer casting
typedef jint (JNICALL *JVM_CREATE)(JavaVM **, JNIEnv **, void *);

static int initialized = 0;
// No particular reason to hold on to this at the moment (never unloaded)
static void* jvmLibHandle = NULL;
// The created JVM
static JavaVM* jvm = NULL;

static const char* pluginDSOName = "libnpjp2.so";

///////////////////////
// Function prototypes
static int InitializeJVM();
static int GetDirectoryForModuleContainingAddress(void* address,
                                                  char* directoryContainingModule);
static int FindJVM(char* jvmPath);
//
///////////////////////

// It's probably unnecessary to have excessive synchronization around
// this initialization
JNIEnv* JavaVM_GetJNIEnv()
{
    JNIEnv* env = NULL;
    jint res = 0;
    if (!initialized) {
        initialized = 1;
        if (!InitializeJVM()) {
            return NULL;
        }
    }
    res = (*jvm)->AttachCurrentThread(jvm, (void**) &env, NULL);
    if (res < 0)
        return NULL;
    return env;
}

void JavaVM_DetachCurrentThread() {
    if (initialized) {
        (*jvm)->DetachCurrentThread(jvm);
    }
}

static int InitializeJVM()
{
    char javaHomeLibPath[PATH_MAX + 1];
    char jvmPath[PATH_MAX + 1];
    char* lastSlash;
    char bootClassPath[2 * PATH_MAX + 1]; // only approximate
    int i;
    int foundPluginFileName = 0;
    int foundJVM = 0;
    JavaVMInitArgs jvmArgs;
    JavaVMOption options[5];
    JVM_CREATE createProc;
    JNIEnv* tmpEnv;

    initialized = 1;

    // Instead of consulting deployment.properties,
    // we can figure out which JRE we live in by finding out the
    // path name to the loaded module (DSO) we are in right now.
    if (!GetDirectoryForModuleContainingAddress((void*) &InitializeJVM, jvmPath)) {
        // Should not happen
        // FIXME: report localized error
        return 0;
    }

    // Copy this off for later boot classpath manipulation
    strcpy(javaHomeLibPath, jvmPath);
    // Note that javaHomeLibPath doesn't yet contain the real Java
    // home/lib directory; it ends with the $arch (e.g., "i386")
    lastSlash = strrchr(javaHomeLibPath, '/');
    assert(lastSlash != NULL); // Stop in debug builds
    if (lastSlash == NULL) {
        // Should not happen
        // FIXME: report localized error
        return 0;
    }
    *lastSlash = '\0';
        
    // OK, now we know where for example libnpjp2.so lives. The
    // JVM lives in a subdirectory of this one.
    foundJVM = FindJVM(jvmPath);
    assert(foundJVM); // Stop in debug builds
    if (!foundJVM) {
        // Should not happen
        // FIXME: report localized error
        return 0;
    }

    // Dynamically load the JVM
    jvmLibHandle = dlopen(jvmPath, RTLD_LAZY | RTLD_LOCAL);
    if (jvmLibHandle == NULL) {
        // Should not happen
        // FIXME: report localized error
        return 0;
    }

    // Compute the boot class path
    snprintf(bootClassPath, sizeof(bootClassPath),
             "-Xbootclasspath/a:%s/deploy.jar:%s/javaws.jar:%s/plugin.jar",
             javaHomeLibPath, javaHomeLibPath, javaHomeLibPath);

    // Set up the VM init args
    jvmArgs.version = JNI_VERSION_1_2;
    options[0].optionString = bootClassPath;
    options[1].optionString = "-Xmx32m";
    options[2].optionString = "-Djava.awt.headless=true";
    options[3].optionString = "-XX:MaxDirectMemorySize=64m";

    jvmArgs.options = options;
    jvmArgs.nOptions = 4;
    jvmArgs.ignoreUnrecognized = JNI_TRUE;

    // Create the JVM
    createProc = (JVM_CREATE) dlsym(jvmLibHandle, "JNI_CreateJavaVM");
    if (createProc == NULL) {
        // Should not happen
        // FIXME: report localized error
        return 0;
    }
    if ((*createProc)(&jvm, &tmpEnv, &jvmArgs) < 0) {
        // Should not happen
        // FIXME: report localized error
        return 0;
    }
    return 1;
}

static int GetDirectoryForModuleContainingAddress(void* addr,
                                                  char* directoryContainingModule)
{
    // Attempts to find the full path to the shared object (.so) which
    // contains the given address. There is no trailing slash ('/') on
    // the returned path. Returns 1 if successful, 0 if not.
    // directoryContainingModule may be mutated regardless.
    Dl_info info;
    int res;
    char* lastSlash = NULL;

    res = dladdr(addr, &info);
    if (res == 0)
        return 0;
    strcpy(directoryContainingModule, info.dli_fname);
    lastSlash = strrchr(directoryContainingModule, '/');
    if (lastSlash == NULL)
        return 0;
    *lastSlash = '\0';
    return 1;
}

static int FindJVM(char* jvmPath) {
    // The incoming path is the "lib/$arch" directory in the JRE; the one
    // containing libnpjp2.so, for example. The HotSpot JVM lives in a
    // subdirectory of this one.

    char tmpPath[PATH_MAX + 1];
    struct stat s;

    // FIXME: should parse jvm.cfg out of the JRE and use it to
    // manually select which jvm.dll to load -- this is important for
    // the Java Kernel, where it will probably be jkernel/jvm.dll that
    // we're looking for

    snprintf(tmpPath, sizeof(tmpPath), "%s/client/libjvm.so", jvmPath);
    if (stat(tmpPath, &s) == 0) {
        strcpy(jvmPath, tmpPath);
        return 1;
    }

    // Try again with the server JVM
    snprintf(tmpPath, sizeof(tmpPath), "%s/server/libjvm.so", jvmPath);
    if (stat(tmpPath, &s) == 0) {
        strcpy(jvmPath, tmpPath);
        return 1;
    }

    printf("Didn't find JVM under %s\n", jvmPath);

    // No luck
    return 0;
}
