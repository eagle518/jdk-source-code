/*
 * @(#)WindowsFlags.cpp	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#include <jni.h>
#include <awt.h>
#include "Trace.h"
#include "WindowsFlags.h"

BOOL      accelReset;         // reset registry 2d acceleration settings
BOOL      useD3D = TRUE;      // d3d enabled flag
                              // initially is TRUE to allow D3D preloading
BOOL      forceD3DUsage;      // force d3d on or off
jboolean  g_offscreenSharing; // JAWT accelerated surface sharing
BOOL      checkRegistry;      // Diagnostic tool: outputs 2d registry settings
BOOL      disableRegistry;    // Diagnostic tool: disables registry interaction
BOOL      setHighDPIAware;    // Whether to set the high-DPI awareness flag

extern WCHAR *j2dAccelKey;       // Name of java2d root key
extern WCHAR *j2dAccelDriverKey; // Name of j2d per-device key

static jfieldID d3dEnabledID;
static jfieldID d3dSetID;
static jclass   wFlagsClassID;

void SetIDs(JNIEnv *env, jclass wFlagsClass) 
{
    wFlagsClassID = (jclass)env->NewGlobalRef(wFlagsClass);
    d3dEnabledID = env->GetStaticFieldID(wFlagsClass, "d3dEnabled", "Z");
    d3dSetID = env->GetStaticFieldID(wFlagsClass, "d3dSet", "Z");
}

BOOL GetStaticBoolean(JNIEnv *env, jclass wfClass, const char *fieldName)
{
    jfieldID fieldID = env->GetStaticFieldID(wfClass, fieldName, "Z");
    return env->GetStaticBooleanField(wfClass, fieldID);
}

jobject GetStaticObject(JNIEnv *env, jclass wfClass, const char *fieldName,
                        const char *signature)
{
    jfieldID fieldID = env->GetStaticFieldID(wfClass, fieldName, signature);
    return env->GetStaticObjectField(wfClass, fieldID);
}

void GetFlagValues(JNIEnv *env, jclass wFlagsClass)
{
    jboolean d3dEnabled = env->GetStaticBooleanField(wFlagsClass, d3dEnabledID);
    jboolean d3dSet = env->GetStaticBooleanField(wFlagsClass, d3dSetID);
    if (!d3dSet) {
	// Only check environment variable if user did not set Java
	// command-line parameter; values of sun.java2d.d3d override
	// any setting of J2D_D3D environment variable.
	char *d3dEnv = getenv("J2D_D3D");
	if (d3dEnv) {
	    if (strcmp(d3dEnv, "false") == 0) {
		// printf("Java2D Direct3D usage disabled by J2D_D3D env\n");
		d3dEnabled = FALSE;
		d3dSet = TRUE;
		SetD3DEnabledFlag(env, d3dEnabled, d3dSet);
	    } else if (strcmp(d3dEnv, "true") == 0) {
		// printf("Java2D Direct3D usage forced on by J2D_D3D env\n");
		d3dEnabled = TRUE;
		d3dSet = TRUE;
		SetD3DEnabledFlag(env, d3dEnabled, d3dSet);
	    }
	}
    }
    useD3D = d3dEnabled;
    forceD3DUsage = d3dSet;
    g_offscreenSharing = GetStaticBoolean(env, wFlagsClass, 
					  "offscreenSharingEnabled");
    accelReset = GetStaticBoolean(env, wFlagsClass, "accelReset");
    checkRegistry = GetStaticBoolean(env, wFlagsClass, "checkRegistry");
    disableRegistry = GetStaticBoolean(env, wFlagsClass, "disableRegistry");
    jstring javaVersionString = (jstring)GetStaticObject(env, wFlagsClass, 
                                                         "javaVersion",
							 "Ljava/lang/String;");
#if 0
    // REMIND: may need something like this in the future (but
    //         probably not here)
    jboolean isCopy;
    const jchar *javaVersion = env->GetStringChars(javaVersionString, 
                                             &isCopy);
    jsize versionLength = env->GetStringLength(javaVersionString);
    size_t j2dRootKeyLength = wcslen(J2D_ACCEL_KEY_ROOT);
    j2dAccelKey = (WCHAR *)safe_Calloc((j2dRootKeyLength + versionLength + 2),
                                       sizeof(WCHAR));
    wcscpy(j2dAccelKey, J2D_ACCEL_KEY_ROOT);
    wcscat(j2dAccelKey, javaVersion);
    wcscat(j2dAccelKey, L"\\");
    j2dAccelDriverKey = 
        (WCHAR *)safe_Calloc((wcslen(j2dAccelKey) + 
                              wcslen(J2D_ACCEL_DRIVER_SUBKEY) + 1),
                             sizeof(WCHAR));
    wcscpy(j2dAccelDriverKey, j2dAccelKey);
    wcscat(j2dAccelDriverKey, J2D_ACCEL_DRIVER_SUBKEY);
    env->ReleaseStringChars(javaVersionString, javaVersion);
#endif

    setHighDPIAware = 
        (IS_WINVISTA && GetStaticBoolean(env, wFlagsClass, "setHighDPIAware"));

    J2dTraceLn(J2D_TRACE_INFO, "WindowsFlags (native):");
    J2dTraceLn1(J2D_TRACE_INFO, "  d3dEnabled = %s", 
		(useD3D ? "true" : "false"));
    J2dTraceLn1(J2D_TRACE_INFO, "  d3dSet = %s", 
		(forceD3DUsage ? "true" : "false"));
    J2dTraceLn1(J2D_TRACE_INFO, "  offscreenSharing = %s", 
		(g_offscreenSharing ? "true" : "false"));
    J2dTraceLn1(J2D_TRACE_INFO, "  accelReset = %s", 
		(accelReset ? "true" : "false"));
    J2dTraceLn1(J2D_TRACE_INFO, "  checkRegistry = %s", 
		(checkRegistry ? "true" : "false"));
    J2dTraceLn1(J2D_TRACE_INFO, "  disableRegistry = %s", 
		(disableRegistry ? "true" : "false"));
    J2dTraceLn1(J2D_TRACE_INFO, "  setHighDPIAware = %s",
                (setHighDPIAware ? "true" : "false"));
}

void SetD3DEnabledFlag(JNIEnv *env, BOOL d3dEnabled, BOOL d3dSet)
{
    useD3D = d3dEnabled;
    forceD3DUsage = d3dSet;
    if (env == NULL) {
	env = (JNIEnv * ) JNU_GetEnv(jvm, JNI_VERSION_1_2);
    }
    env->SetStaticBooleanField(wFlagsClassID, d3dEnabledID, d3dEnabled);
    if (d3dSet) {
	env->SetStaticBooleanField(wFlagsClassID, d3dSetID, d3dSet);
    }
}

BOOL IsD3DEnabled() {
    return useD3D;
}

BOOL IsD3DForced() {
    return forceD3DUsage;
}

extern "C" {

/**
 * This function is called from WindowsFlags.initFlags() and initializes
 * the native side of our runtime flags.  There are a couple of important
 * things that happen at the native level after we set the Java flags:
 * - set native variables based on the java flag settings (such as useDD
 * based on whether ddraw was enabled by a runtime flag)
 * - override java level settings if there user has set an environment
 * variable but not a runtime flag.  For example, if the user runs
 * with sun.java2d.d3d=true but also uses the J2D_D3D=false environment
 * variable, then we use the java-level true value.  But if they do
 * not use the runtime flag, then the env variable will force d3d to
 * be disabled.  Any native env variable overriding must up-call to
 * Java to change the java level flag settings.
 * - A later error in initialization may result in disabling some
 * native property that must be propagated to the Java level.  For
 * example, d3d is enabled by default, but we may find later that
 * we must disable it do to some runtime configuration problem (such as
 * a bad video card).  This will happen through mechanisms in this native
 * file to change the value of the known Java flags (in this d3d example,
 * we would up-call to set the value of d3dEnabled to Boolean.FALSE).
 */
JNIEXPORT void JNICALL
Java_sun_java2d_windows_WindowsFlags_initNativeFlags(JNIEnv *env,
                                                     jclass wFlagsClass)
{
    SetIDs(env, wFlagsClass);
    GetFlagValues(env, wFlagsClass);
}

} // extern "C"

