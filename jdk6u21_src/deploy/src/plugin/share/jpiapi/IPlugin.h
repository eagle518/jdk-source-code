/*
 * @(#)IPlugin.h	1.6 10/03/24  
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IPlugin.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
// Contains the interface for Plugin service browser requires from Java Plugin
//
#ifndef _IPLUGIN_H_
#define _IPLUGIN_H_

#include "ISupports.h"
#include "IFactory.h"

//{303A642B-9918-11d6-9A75-00B0D0A18D51}
#define IPLUGIN_IID \
    {0x303A642B, 0x9918, 0x11d6, {0x9A, 0x75, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51} }

//ISupports interface (A replicate of nsISupports)
class IPlugin : public IFactory {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IPLUGIN_IID);
    
    /**
     * Creates a new plugin instance, based on a MIME type. This
     * allows different impelementations to be created depending on
     * the specified MIME type.
     */

    JD_IMETHOD CreatePluginInstance(ISupports *aOuter, JDREFNSIID aIID, 
                                    const char* aPluginMIMEType,
                                    void **aResult) = 0;

    /**
     * Initializes the plugin and will be called before any new instances are
     * created. It is passed browserInterfaces on which QueryInterface
     * may be used to obtain an nsIPluginManager, and other interfaces.
     *
     * @param browserInterfaces - an object that allows access to other browser
     * interfaces via QueryInterface
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    Initialize() = 0;

    /**
     * Called when the browser is done with the plugin factory, or when
     * the plugin is disabled by the user.
     *
     * (Corresponds to NPP_Shutdown.)
     *
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    Shutdown(void) = 0;

    /**
     * Returns the MIME description for the plugin. The MIME description 
     * is a colon-separated string containg the plugin MIME type, plugin
     * data file extension, and plugin name, e.g.:
     *
     * "application/x-simple-plugin:smp:Simple LiveConnect Sample Plug-in"
     *
     * (Corresponds to NPP_GetMIMEDescription.)
     *
     * @param resultingDesc - the resulting MIME description 
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    GetMIMEDescription(const char* *resultingDesc) = 0;

    /**
     * Returns the value of a variable associated with the plugin.
     *
     * (Corresponds to NPP_GetValue.)
     *
     * @param variable - the plugin variable to get
     * @param value - the address of where to store the resulting value
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    GetValue(JDPluginVariable variable, void *value) = 0;
     
};

#endif //_IPLUGIN_H_
