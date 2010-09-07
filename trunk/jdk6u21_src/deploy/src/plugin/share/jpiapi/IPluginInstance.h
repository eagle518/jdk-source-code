/*
 * @(#)IPluginInstance.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IPluginInstance.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
// Contains the interface for service from Java Plug-in
//
#ifndef _IPLUGININSTANCE_H_
#define _IPLUGININSTANCE_H_

#include "ISupports.h"

class IPluginInstancePeer;
class IPluginStreamListener;

//{7A168FD3-A576-11d6-9A82-00B0D0A18D51}
#define IPLUGININSTANCE_IID			     \
{   0x7A168FD3,                                      \
    0xA576,                                          \
    0x11d6,                                          \
    {0x9A, 0x82, 0x00, 0xB0, 0xD0, 0xA1, 0x8d, 0x51} \
}

enum JDPluginInstanceVariable {
    JDPluginInstanceVariable_WindowlessBool          = 3,
    JDPluginInstanceVariable_TransparentBool         = 4,
    JDPluginInstanceVariable_DoCacheBool             = 5,
    JDPluginInstanceVariable_CallSetWindowAfterDestroyBool = 6,
    JDPluginInstanceVariable_ScriptableInstance      = 10,
    JDPluginInstanceVariable_ScriptableIID           = 11,
    JDPluginInstanceVariable_NeedsXEmbed             = 14
};


class IPluginInstance : public ISupports {
public:

    JD_DEFINE_STATIC_IID_ACCESSOR(IPLUGININSTANCE_IID)
	
    /**
     * Initializes a newly created plugin instance, passing to it the plugin
     * instance peer which it should use for all communication back to the browser.
     * 
     * @param peer - the corresponding plugin instance peer
     * @result - NS_OK if this operation was successful
     */

    JD_IMETHOD
    Initialize(IPluginInstancePeer* peer) = 0;

    /**
     * Returns a reference back to the plugin instance peer. This method is
     * used whenever the browser needs to obtain the peer back from a plugin
     * instance. The implementation of this method should be sure to increment
     * the reference count on the peer by calling AddRef.
     *
     * @param resultingPeer - the resulting plugin instance peer
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    GetPeer(IPluginInstancePeer* *resultingPeer) = 0;

    /**
     * Called to instruct the plugin instance to start. This will be called after
     * the plugin is first created and initialized, and may be called after the
     * plugin is stopped (via the Stop method) if the plugin instance is returned
     * to in the browser window's history.
     *
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    Start(void) = 0;

    /**
     * Called to instruct the plugin instance to stop, thereby suspending its state.
     * This method will be called whenever the browser window goes on to display
     * another page and the page containing the plugin goes into the window's history
     * list.
     *
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    Stop(void) = 0;

    /**
     * Called to instruct the plugin instance to destroy itself. This is called when
     * it become no longer possible to return to the plugin instance, either because 
     * the browser window's history list of pages is being trimmed, or because the
     * window containing this page in the history is being closed.
     *
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    Destroy(void) = 0;

    /**
     * Called when the window containing the plugin instance changes.
     *
     * (Corresponds to NPP_SetWindow.)
     *
     * @param window - the plugin window structure
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    SetWindow(JDPluginWindow* window) = 0;
    
    /**
     * Called to instruct the plugin instance to print itself to a printer.
     *
     * (Corresponds to NPP_Print.)
     *
     * @param platformPrint - platform-specific printing information
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    Print(JDPluginPrint* platformPrint) = 0;

    /**
     * Returns the value of a variable associated with the plugin instance.
     *
     * @param variable - the plugin instance variable to get
     * @param value - the address of where to store the resulting value
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    GetValue(JDPluginInstanceVariable variable, void *value) = 0;
};

////////////////////////////////////////////////////////////////////////////////

#endif /* _IPluginInstance_h___ */
