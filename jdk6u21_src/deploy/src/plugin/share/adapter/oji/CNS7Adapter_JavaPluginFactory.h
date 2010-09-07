/*
 * @(#)CNS7Adapter_JavaPluginFactory.h	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS7Adapter_JavaPluginFactory.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS7Adapter_JavaPluginFactory.h : Declaration of the Adapter for nsIPlugin.
// This is a service entry point JPI provided to browser
//
#ifndef __CNS7Adapter_JavaPluginFactory_H_
#define __CNS7Adapter_JavaPluginFactory_H_

#include "CNSAdapter_JavaPluginFactory.h"

class CNS7Adapter_JavaPluginFactory : public CNSAdapter_JavaPluginFactory
{
public:
    CNS7Adapter_JavaPluginFactory(IFactory* p_JavaPluginFactory);

    ~CNS7Adapter_JavaPluginFactory();
    //=--------------------------------------------------------------=
    // nsIPlugin
    //=--------------------------------------------------------------=
    /**
     * Creates a new plugin instance, based on a MIME type. This
     * allows different impelementations to be created depending on
     * the specified MIME type.
     */
    NS_IMETHOD CreateInstance(nsISupports *aOuter, 
			      REFNSIID aIID,
			      void **aResult);
};

#endif //__CNS7Adapter_JavaPluginFactory_H_
