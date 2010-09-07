/*
 * @(#)MozPluginInstance_pd.cpp	1.10 10/03/24 12:01:20
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "StdAfx.h"

#include "JavaVM.h"
#include "LocalFramePusher.h"
#include "MozPluginInstance.h"
#include "MozExports.h"
#include "AbstractPlugin.h"
#include "sun_plugin2_main_server_MozillaPlugin.h"

void MozPluginInstance::pdConstruct() {
    // Nothing special to do
}

void MozPluginInstance::pdDelete() {
    // Nothing special to do
}

void MozPluginInstance::pdSetWindow(void* window, uint x, uint y, uint width, uint height,
                                    uint clipTop, uint clipLeft, uint clipBottom, uint clipRight) {
    // Nothing special to do
}

bool MozPluginInstance::pdPrintEmbedded(NPEmbedPrint& embedPrintInfo) {

    FILE *fp;
    fp = ((NPPrintCallbackStruct *)(embedPrintInfo.platformPrint))->fp;

    NPWindow* printWindow = &(embedPrintInfo.window);
    int x, y, width, height;
    x = printWindow->x;
    y = printWindow->y;
    width = printWindow->width;
    height = printWindow->height;
    
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    return env->CallBooleanMethod(m_pluginObject,
                                  mozPluginPrintID,
                                  (jlong) fp,
                                  (jint) x, (jint) y,
                                  (jint) width, (jint) height);
}

jcharArray MozPluginInstance::pdAuthInfoToCharArray(JNIEnv* env,
					 int len,
                                         const char* szName,
					 const char* szPassword) {

    char* lpszBuf = new char[len + 2];
    snprintf(lpszBuf, len + 2, "%s:%s", szName, szPassword);
    // [FIXME] should consider add debug trace statement here
    //trace("Browser return: %s\n", lpszBuf);
    jstring jstrBuf = env->NewStringUTF(lpszBuf);
    const jchar * jcharsBuf = env->GetStringChars(jstrBuf, (jboolean*)0);
    const jsize unicode_len = env->GetStringLength(jstrBuf);
    jcharArray retJCA = env->NewCharArray(unicode_len);
    env->SetCharArrayRegion(retJCA, 0, unicode_len, (jchar *)jcharsBuf);

    env->ReleaseStringChars(jstrBuf, jcharsBuf);
    delete[] lpszBuf;

    return retJCA;
}
