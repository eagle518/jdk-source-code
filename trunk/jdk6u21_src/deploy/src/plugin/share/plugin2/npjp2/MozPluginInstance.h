/*
 * @(#)MozPluginInstance.h	1.11 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __MOZPLUGININSTANCE_H_
#define __MOZPLUGININSTANCE_H_

#include <jni.h>
#include "npapi.h"
#include "npruntime.h"

class MozPluginInstance {

 public:
    MozPluginInstance(NPP instance, 
                      const char* pluginMimeType, 
                      short argc, char* argn[], char* argv[]);

    ~MozPluginInstance();

    /* Perform global initialization. This includes starting up the VM etc. */
    static bool Initialize();
    static bool GlobalInitialize();
    // Indicates after the fact whether initialization failed
    static bool InitFailed();

    NPP GetNPP() { return m_pluginInstance; }
    bool SetWindow(void* window, uint x, uint y, uint width, uint height,
                   uint clipTop, uint clipLeft, uint clipBottom, uint clipRight);
    bool Print(NPPrint* platformPrint);
    void Destroy();

    // Helper method for converting NPVariants to Java objects
    jobject variantToJObject(JNIEnv* env, NPVariant* variant);

    // Helper methods for JavaScript -> Java support; see JavaObject.cpp/h
    jobject getJavaMozillaPluginInstance();
    NPObject* getAppletNPObject();

    static jcharArray pdAuthInfoToCharArray(JNIEnv* env,
				     int len,
				     const char* szName,
				     const char* szPassword);

    // Helper method to update the applet's location -- used only on Mac OS X
    void updateLocationAndClip();

 private:
    static jclass    mozPluginClass;
    static jmethodID mozPluginCtorID;
    static jmethodID mozPluginAddParametersID;
    static jmethodID mozPluginSetWindowID;
    static jmethodID mozPluginDestroyID;
    static jmethodID mozPluginPrintID;
    static jmethodID mozPluginUpdateLocationAndClipID;

    jobject m_pluginObject;     // MozillaPlugin Java instance

    NPP     m_pluginInstance;

    NPObject* m_appletNPObject;

    // Subclasses must provide implementations for these methods
    void pdConstruct();
    void pdDelete();
    void pdSetWindow(void* window, uint x, uint y, uint width, uint height,
                     uint clipTop, uint clipLeft, uint clipBottom, uint clipRight);
    // Serving embedded printing mode
    bool pdPrintEmbedded(NPEmbedPrint& embedPrintInfo);

    // Allocates storage which must be deleted via delete[] afterward
    char* GetDocumentBase();

#include "MozPluginInstance_pd.h"
};

#endif  //__MOZPLUGININSTANCE_H_
