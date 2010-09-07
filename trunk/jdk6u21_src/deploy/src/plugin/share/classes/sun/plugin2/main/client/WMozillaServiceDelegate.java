/*
 * @(#)WMozillaServiceDelegate.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.io.*;
import java.security.*;
import sun.plugin2.util.NativeLibLoader;

/** This class largely serves to pick up the MSCredentialManager. */

public class WMozillaServiceDelegate extends MozillaServiceDelegate {
    /**
     * Return the Crendential Manager.
     */
    public com.sun.deploy.security.CredentialManager getCredentialManager() 
    {
        // return the Windows Credential Manager   
        return com.sun.deploy.security.MSCredentialManager.getInstance();
    }

    //----------------------------------------------------------------------
    // These methods are only present in support of disconnected
    // applets (those dragged out of the web browser)

    // Note that here we mimic the implementation in
    // WIExplorerBrowserService, because we prefer not to depend on
    // that class here
    
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()
    {
	return new com.sun.deploy.net.proxy.WIExplorerProxyConfig();
    }

    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler()
    {
	return new com.sun.deploy.net.proxy.WIExplorerAutoProxyHandler();
    }
}
