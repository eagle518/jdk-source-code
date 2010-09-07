/*
 * @(#)CNSAdapter_JavaPlugin.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_JavaPlugin.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_JavaPlugin.h : Declaration of the Adapter for nsIPluginInstance.
//
#ifndef _CNSAdapter_JavaPlugin_h__
#define _CNSAdapter_JavaPlugin_h__

#include "nsIPluginInstance.h"
#include "nsIJVMPluginInstance.h"

class nsIPluginInstancePeer;
class IPluginInstance;

class CNSAdapter_JavaPlugin : public nsIPluginInstance,
			      public nsIJVMPluginInstance
{
public:
    //nsISupports
    NS_DECL_ISUPPORTS

    //nsIPluginInstance
    NS_IMETHOD
    Initialize(nsIPluginInstancePeer* peer);

    /**
     * Returns a reference back to the plugin instance peer. This method is
     * used whenever the browser needs to obtain the peer back from a plugin
     * instance. The implementation of this method should be sure to increment
     * the reference count on the peer by calling AddRef.
     *
     * @param resultingPeer - the resulting plugin instance peer
     * @result - NS_OK if this operation was successful
     */
    NS_IMETHOD
    GetPeer(nsIPluginInstancePeer* *resultingPeer);

    /**
     * Called to instruct the plugin instance to start. This will be called after
     * the plugin is first created and initialized, and may be called after the
     * plugin is stopped (via the Stop method) if the plugin instance is returned
     * to in the browser window's history.
     *
     * @result - NS_OK if this operation was successful
     */
    NS_IMETHOD
    Start(void);

    /**
     * Called to instruct the plugin instance to stop, thereby suspending its state.
     * This method will be called whenever the browser window goes on to display
     * another page and the page containing the plugin goes into the window's history
     * list.
     *
     * @result - NS_OK if this operation was successful
     */
    NS_IMETHOD
    Stop(void);

    /**
     * Called to instruct the plugin instance to destroy itself. This is called when
     * it become no longer possible to return to the plugin instance, either because 
     * the browser window's history list of pages is being trimmed, or because the
     * window containing this page in the history is being closed.
     *
     * @result - NS_OK if this operation was successful
     */
    NS_IMETHOD
    Destroy(void);

    /**
     * Called when the window containing the plugin instance changes.
     *
     * (Corresponds to NPP_SetWindow.)
     *
     * @param window - the plugin window structure
     * @result - NS_OK if this operation was successful
     */
    NS_IMETHOD
    SetWindow(nsPluginWindow* window);

    /**
     * Called to tell the plugin that the initial src/data stream is
	 * ready.  Expects the plugin to return a nsIPluginStreamListener.
     *
     * (Corresponds to NPP_NewStream.)
     *
     * @param listener - listener the browser will use to give the plugin the data
     * @result - NS_OK if this operation was successful
     */
    NS_IMETHOD
    NewStream(nsIPluginStreamListener** listener);
    /**
     * Called to instruct the plugin instance to print itself to a printer.
     *
     * (Corresponds to NPP_Print.)
     *
     * @param platformPrint - platform-specific printing information
     * @result - NS_OK if this operation was successful
     */
    NS_IMETHOD
    Print(nsPluginPrint* platformPrint);

    /**
     * Returns the value of a variable associated with the plugin instance.
     *
     * @param variable - the plugin instance variable to get
     * @param value - the address of where to store the resulting value
     * @result - NS_OK if this operation was successful
     */
    NS_IMETHOD
    GetValue(nsPluginInstanceVariable variable, void *value);

    /**
     * Handles an event. An nsIEventHandler can also get registered with with
     * nsIPluginManager2::RegisterWindow and will be called whenever an event
     * comes in for that window.
     *
     * Note that for Unix and Mac the nsPluginEvent structure is different
     * from the old NPEvent structure -- it's no longer the native event
     * record, but is instead a struct. This was done for future extensibility,
     * and so that the Mac could receive the window argument too. For Windows
     * and OS2, it's always been a struct, so there's no change for them.
     *
     * (Corresponds to NPP_HandleEvent.)
     *
     * @param event - the event to be handled
     * @param handled - set to PR_TRUE if event was handled
     * @result - NS_OK if this operation was successful
     */
    NS_IMETHOD
    HandleEvent(nsPluginEvent* event, PRBool* handled);


    // nsIJVMPluginInstance
    NS_IMETHOD GetJavaObject(jobject *result);

    /* [noscript] void GetText (in nChar result); */
    NS_IMETHOD GetText(const char ** result);

    //CJavaPluginAdapter methods
    CNSAdapter_JavaPlugin(IPluginInstance* pJavaPlugin);

    virtual ~CNSAdapter_JavaPlugin(void);
protected:
    IPluginInstance*	    m_pJavaPlugin;
    nsIPluginInstancePeer*  m_peer;
};

#endif //__CNSAdapter_JavaPlugin_h__
