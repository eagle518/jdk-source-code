/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "stdafx.h"
#include "JQSIEStartDetectorImpl.h"
#include "jqs_api_client.hpp"
#include "sockets.hpp"
#include "os_utils.hpp"


// CJQSIEStartDetectorImpl

/*
 * The IE calls this method to with non NULL site when it loads the plugin
 * and with a NULL site on unload.
 * The implementation sends JMK_Notify command to the JQS service using the JQS API.
 */
STDMETHODIMP CJQSIEStartDetectorImpl::SetSite(IUnknown* pUnkSite)
{
    TRY {
        if (pUnkSite != NULL) {
            // Browser is being started
            initSocketLibrary ();
            sendJQSAPICommand (JMK_Notify);
        } else {
            // Browser is being closed
            cleanupSocketLibrary ();
        }

    } CATCH_SYSTEM_EXCEPTIONS {
        // ignore exceptions
    }
 
    // Return the base class implementation
    return IObjectWithSiteImpl<CJQSIEStartDetectorImpl>::SetSite(pUnkSite);
}
