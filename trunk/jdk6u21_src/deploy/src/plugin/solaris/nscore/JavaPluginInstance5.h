/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)JavaPluginInstance5.h	1.21 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Contains declaration of the CNetscapePlugin. Modified version of
 *  Stanley's Win32 header.
 */


#ifndef JAVAPLUGININSTANCE5_H
#define JAVAPLUGININSTANCE5_H

#include "commonhdr.h"
#include "IPluginInstancePeer.h"
#include "IJVMPluginInstance.h"
#include "IPluginInstance.h"
#include "IUniqueIdentifier.h"
#include "INS4PluginInstance.h"

// Possible APPLET state
//
#define APPLET_DISPOSE	        0
#define APPLET_LOAD		        1
#define APPLET_INIT		        2
#define APPLET_START	        3
#define APPLET_STOP		        4
#define APPLET_DESTROY	        5
#define APPLET_QUIT		        6
#define APPLET_ERROR	        7

class JavaPluginFactory5;
class IPluginStreamInfo;
class IPluginStream;
class ILiveconnect;

class JavaPluginInstance5 : public IJVMPluginInstance, IPluginInstance, INS4PluginInstance, IUniqueIdentifier {
public:
    JD_DECL_ISUPPORTS
		
    JavaPluginInstance5(JavaPluginFactory5 *plugin);
    virtual	~JavaPluginInstance5(void);

    JavaPluginFactory5* GetPluginFactory(void);
    //===============================================================
    // IPluginInstance
    //===============================================================
    JD_IMETHOD Initialize(IPluginInstancePeer *peer);
    
    JD_IMETHOD Start(void);
    
    JD_IMETHOD GetPeer(IPluginInstancePeer* *resultingPeer);
    // The old NPP_Destroy call has been factored into two plugin instance 
    // methods:
    // Stop -- called when the plugin instance is to be stopped (e.g. by 
    // displaying another plugin manager window, causing the page containing 
    // the plugin to become removed from the display).
    //
    // Release -- called once, before the plugin instance peer is to be 
    // destroyed. This method is used to destroy the plugin instance.
    JD_IMETHOD Stop(void);

    JD_IMETHOD Destroy(void);

    // (Corresponds to NPP_SetWindow.)
    JD_IMETHOD SetWindow(JDPluginWindow* window);

     // (Corresponds to NPP_Print.)
    JD_IMETHOD Print(JDPluginPrint* platformPrint);

    JD_IMETHOD GetValue(JDPluginInstanceVariable var, void* val);

    //===============================================================
    // IJVMPluginInstance.h
    //===============================================================
    JD_IMETHOD GetJavaObject(jobject *result);
   
    //===============================================================
    // Other methods called internally by plugin code
    //===============================================================
    JD_IMETHOD NewStream(IPluginStreamInfo* peer, IPluginStream** stream);

    JD_IMETHOD URLNotify(const char* url, const char* target, JDPluginReason reason, void* notifyData);

    JD_IMETHOD SetDocbase(const char *url);

    JD_IMETHOD_(void) JavascriptReply(const char *reply);
    
    JD_IMETHOD_(void) EnterRequest(char* msg);
    JD_IMETHOD_(void) ExitRequest(char* msg);

    JD_IMETHOD SetUniqueId(long id);
    JD_IMETHOD GetUniqueId(long* pId);
 
    // Return the index of this plugin
    int GetPluginNumber(void) { return plugin_number; } ;

    JD_METHOD GetJSDispatcher(ILiveconnect* *pLiveconnect);
    // Mozilla changes: This will embed the given AppletWindow ID into Navigator's
    // containing browser window
    void EmbedAppletWindow(int windowID);

    void SetStatus(short status);

    short GetStatus() { return m_status; }

    bool IsDestroyPending() { return m_destroyPending; }
protected:
    IPluginInstancePeer*	instance_peer;
    JavaPluginFactory5*     plugin_factory;
    int plugin_number;
    
    // Window
    JDPluginWindow* window;

    // If non-null indicate that a request is in process
    char* current_request;

    // Need to know if we have had Destroy called on us 
    bool mIsDestroyed;

    long			m_uniqueId;
    static long		        s_uniqueId;

    ILiveconnect*               m_pLiveconnect;
    short                       m_status;
    bool                        m_destroyPending;
};

#endif 
