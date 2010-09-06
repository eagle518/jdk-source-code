/*
 * @(#)CNS6Adapter_JavaPluginFactory.cpp	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS6Adapter_JavaPluginFactory.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS6Adapter_JavaPluginFactory.cpp: Implementation of adapter for nsIPlugin,
// See CNSAdapter_JavaPluginFactory.h for details.
//
#include "StdAfx.h"
#include "jni.h"
#include "nsIPluginInstance.h"
#include "nsIJVMPluginInstance.h"
#include "IPluginInstance.h"
#include "IPlugin.h"
#include "IJVMPluginInstance.h"
#include "CNS6Adapter_JavaPluginFactory.h"
#include "CNSAdapter_JavaPlugin.h"
#include "Debug.h"

static NS_DEFINE_IID(kIPluginInstanceIID, NS_IPLUGININSTANCE_IID);
static NS_DEFINE_IID(kIJVMPluginInstanceIID, NS_IJVMPLUGININSTANCE_IID);
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

static JD_DEFINE_IID(jIPluginInstanceIID, IPLUGININSTANCE_IID);
static JD_DEFINE_IID(jIJVMPluginInstanceIID, IJVMPLUGININSTANCE_IID);
static JD_DEFINE_IID(jISupportsIID, ISUPPORTS_IID);
///=--------------------------------------------------------------------------=
// CNS6Adapter_JavaPluginFactory::CNS6Adapter_JavaPluginFactory
//=---------------------------------------------------------------------------=
// Implements the CNSAdapter_JavaPluginFactory object for encapsulating the win32 plugin
// factory.
//
// parameters :
//
// return :
//
// notes :
//
CNS6Adapter_JavaPluginFactory::CNS6Adapter_JavaPluginFactory(IFactory* p_JavaPluginFactory) : CNSAdapter_JavaPluginFactory(p_JavaPluginFactory) {}

///=--------------------------------------------------------------------------=
// CNS6Adapter_JavaPluginFactory::~CNS6Adapter_JavaPluginFactory
//=---------------------------------------------------------------------------=
CNS6Adapter_JavaPluginFactory::~CNS6Adapter_JavaPluginFactory() {}

///=--------------------------------------------------------------------------=
// CNS6Adapter_JavaPluginFactory::CreateInstance
//=---------------------------------------------------------------------------=
// CreateInstance is called when a new plugin instance needs to be created.
//
// parameters :
//		nsISupports* [in]    Outer object for aggregation
//		REFNSIID     [in]    IID of the interface requested
//		void**	     [out]   Result
//
// return :
//		nsresult     [out]   Success code
//
// notes :
//
NS_IMETHODIMP CNS6Adapter_JavaPluginFactory::CreateInstance(nsISupports *outer, REFNSIID iid, void **result)
{
    TRACE("CNS6Adapter_JavaPluginFactory::CreateInstance\n");
     if (m_pIPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    JDIID  requestIID;

    // classid conversion and this is what Adapter means
    if (iid.Equals(kIPluginInstanceIID))
	requestIID = jIPluginInstanceIID;
    else if (iid.Equals(kIJVMPluginInstanceIID))
	requestIID = jIJVMPluginInstanceIID;
    else if (iid.Equals(kISupportsIID))
	requestIID = jISupportsIID;
    else
	return NS_NOINTERFACE;

    JDSmartPtr<IPluginInstance> spJavaPluginInst;
    nsresult err = m_pIPlugin->CreateInstance(NULL, requestIID, (void**)&spJavaPluginInst);

    if (NS_SUCCEEDED(err) && spJavaPluginInst != NULL)
    {
	JDSmartPtr<nsIPluginInstance> spJavaPluginInstAdapter(new CNSAdapter_JavaPlugin(spJavaPluginInst));
	if (spJavaPluginInstAdapter == NULL)
	    return NS_ERROR_OUT_OF_MEMORY;

	return spJavaPluginInstAdapter->QueryInterface(iid, result);
    }

    return err;

}
