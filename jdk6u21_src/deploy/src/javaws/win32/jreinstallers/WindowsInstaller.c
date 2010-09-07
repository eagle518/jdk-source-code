/*
 * @(#)WindowsInstaller.c	1.9 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include <windows.h>

extern int isValidMSVCRTVersion(DWORD versionMS, DWORD versionLS);


/*
 * Class:     com_sun_javaws_installers_WindowsInstaller
 * Method:    needsReboot
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_javaws_installers_WindowsInstaller_needsReboot
          (JNIEnv *env, jobject this, jlong versionMS, jlong versionLS) {
    if (!isValidMSVCRTVersion((DWORD)versionMS, (DWORD)versionLS)) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

/*
 * Class:     com_sun_javaws_installers_WindowsInstaller
 * Method:    reboot
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_sun_javaws_installers_WindowsInstaller_reboot
                       (JNIEnv *env, jobject this) {
    ExitWindowsEx(EWX_REBOOT, 0);
}
