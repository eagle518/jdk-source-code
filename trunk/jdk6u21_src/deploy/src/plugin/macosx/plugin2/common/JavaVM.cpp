/*
 * @(#)JavaVM.cpp	1.5 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "JavaVM.h"
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <assert.h>

static int initialized = 0;
// The created JVM
static JavaVM* jvm = NULL;

///////////////////////
// Function prototypes
static int InitializeJVM();
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
    res = jvm->AttachCurrentThread((void**) &env, NULL);
    if (res < 0)
        return NULL;
    return env;
}

void JavaVM_DetachCurrentThread() {
    if (initialized) {
        jvm->DetachCurrentThread();
    }
}

static int InitializeJVM()
{
    initialized = 1;

    // The bootstrapping path for the new plug-in on Mac OS X is
    // different than on other platforms for a few reasons. First, we
    // do not need to dynamically load the JVM; we can link this code
    // against JavaVM.framework, since the presence of this framework
    // is guaranteed on OS X. As a matter of fact, trying to *not*
    // link against JavaVM.framework would be quite difficult.
    //
    // Second, the packaging of the new plug-in currently differs
    // significantly from that on other platforms. On Solaris, Linux
    // and Windows platforms, the .so or DLL corresponding to the new
    // plug-in is contained inside the JRE. On Mac OS X, the current
    // expectation is that it will not be. We do not currently conform
    // to the "switchable plugin" concept on OS X; if we did, the
    // rules might be different. Regardless, we need to find the
    // locations of our associated libraries (deploy.jar, javaws.jar,
    // plugin.jar, libdeploy.jnilib, libjp2safari.jnilib). Right now
    // we include these in our new plug-in bundle. If or when the new
    // plug-in officially ships on Mac OS X, the logic which sets up
    // the boot classpath should probably change.
    //
    // Third, we use CFBundleGetBundleWithIdentifier rather than
    // dladdr to figure out where we live on disk, in order to locate
    // our dependent resources as described above. It is unclear at
    // this point whether dladdr would work. Note that exactly how we
    // will build a Java Plug-In for Firefox on Mac OS X is not clear
    // at this point (for example, would it use the same bundle format
    // as the Cocoa plugin for Safari? Or would it simply be a .dylib?
    // Where would the plugin be installed? If under /Library/Internet
    // Plug-ins/, then what about conflicts between the Cocoa plugin
    // for Safari and the NPRuntime-based plugin for Firefox?  etc.).
    // It might be the case that JavaVM.c can not be shared between
    // the Safari and Firefox plug-ins on OS X, in which case it would
    // need to be copied into browser-specific source directories.

    // Figure out our current bundle
    // FIXME: may need to generalize this to other bundle names
    CFBundleRef currentBundle = CFBundleGetBundleWithIdentifier(CFSTR("com.sun.JavaPlugin2"));
    if (currentBundle == NULL) {
        printf("CFBundleGetBundleWithIdentifier(\"com.sun.JavaPlugin2\") failed\n");
        return 0;
    }

    // Find the resources we need in it
    CFURLRef deployJar = CFBundleCopyResourceURL(currentBundle, CFSTR("deploy.jar"), NULL, NULL);
    if (deployJar == NULL) {
        printf("CFBundleCopyResourceURL for deploy.jar failed\n");
        return 0;
    }
    CFStringRef deployJarPath = CFURLCopyFileSystemPath(deployJar, kCFURLPOSIXPathStyle);
    CFURLRef javawsJar = CFBundleCopyResourceURL(currentBundle, CFSTR("javaws.jar"), NULL, NULL);
    if (javawsJar == NULL) {
        printf("CFBundleCopyResourceURL for javaws.jar failed\n");
        return 0;
    }
    CFStringRef javawsJarPath = CFURLCopyFileSystemPath(javawsJar, kCFURLPOSIXPathStyle);
    CFURLRef pluginJar = CFBundleCopyResourceURL(currentBundle, CFSTR("plugin.jar"), NULL, NULL);
    if (pluginJar == NULL) {
        printf("CFBundleCopyResourceURL for plugin.jar failed\n");
        return 0;
    }
    CFStringRef pluginJarPath = CFURLCopyFileSystemPath(pluginJar, kCFURLPOSIXPathStyle);
    // This is only temporary
    CFStringRef sharedWindowJarPath = NULL;    
    CFURLRef sharedWindowJar = CFBundleCopyResourceURL(currentBundle, CFSTR("SharedWindow.jar"), NULL, NULL);
    if (sharedWindowJar != NULL) {
        sharedWindowJarPath = CFURLCopyFileSystemPath(sharedWindowJar, kCFURLPOSIXPathStyle);
    }

    // Construct the boot class path for the created JVM
    CFStringRef bootClassPath;
    if (sharedWindowJarPath != NULL) {
        bootClassPath = CFStringCreateWithFormat(NULL, NULL,
                                                 CFSTR("-Xbootclasspath/a:%@:%@:%@:%@"),
                                                 deployJarPath,
                                                 javawsJarPath,
                                                 pluginJarPath,
                                                 sharedWindowJarPath);
    } else {
        bootClassPath = CFStringCreateWithFormat(NULL, NULL,
                                                 CFSTR("-Xbootclasspath/a:%@:%@:%@"),
                                                 deployJarPath,
                                                 javawsJarPath,
                                                 pluginJarPath);
    }
    int bootClassPathLen = 1 + CFStringGetMaximumSizeForEncoding(CFStringGetLength(bootClassPath), kCFStringEncodingUTF8);
    char* bootClassPathCStr = (char*) calloc(bootClassPathLen, sizeof(char));
    if (bootClassPathCStr == NULL) {
        printf("Allocation of bootClassPathCStr failed\n");
        return 0;
    }
    if (!CFStringGetCString(bootClassPath, bootClassPathCStr, bootClassPathLen, kCFStringEncodingUTF8)) {
        printf("Error getting C string for boot class path\n");
        return 0;
    }

    // Set up the VM init args
    JavaVMInitArgs jvmArgs;
    JavaVMOption options[4];
    jvmArgs.version = JNI_VERSION_1_2;
    options[0].optionString = bootClassPathCStr;
    options[1].optionString = "-Xmx32m";
    options[2].optionString = "-Djava.awt.headless=true";
    options[3].optionString = "-XX:MaxDirectMemorySize=64m";

    jvmArgs.options = options;
    jvmArgs.nOptions = 4;
    jvmArgs.ignoreUnrecognized = JNI_TRUE;

    // Create the JVM
    JNIEnv* tmpEnv;
    if (JNI_CreateJavaVM(&jvm, (void **) &tmpEnv, &jvmArgs) < 0) {
        // Should not happen
        // FIXME: report localized error
        printf("Creation of JVM failed\n");
        return 0;
    }
    return 1;
}
