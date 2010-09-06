/*
 * @(#)BasicServiceImpl.java	1.10 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import javax.jnlp.BasicService;
import java.net.URL;
import java.net.MalformedURLException;
import com.sun.javaws.BrowserSupport;

public final class BasicServiceImpl implements BasicService {
    
    // Launch property file
    private URL        _codebase = null;
    private boolean    _isWebBrowserSupported;
    private boolean    _isOffline;
    
    // Only one instance of this will ever exist
    static private BasicServiceImpl _sharedInstance = null;
    
    /** Creates a BasicServiceImpl with information about a particular application and JNLP file */
    private BasicServiceImpl(URL codebase, boolean isOffline, boolean isWebBrowserSupported) {
        _codebase = codebase;
        _isWebBrowserSupported = isWebBrowserSupported;
        _isOffline = isOffline;
    }
    
    /** Returns singelton instance. This might return null, if this service is not supported */
    public static BasicServiceImpl getInstance() { return _sharedInstance; }
    
    /**
     *  Initialize the single instance of this class. This call is ignored if the instance is already
     *  initialized.
     */
    public static void initialize(URL codebase, boolean isOffline, boolean isWebBrowserSupported) {
        if (_sharedInstance == null) {
            _sharedInstance = new BasicServiceImpl(codebase, isOffline, isWebBrowserSupported);
        }
    }
    
    /** Returns the codebase for an application. See description in service interface */
    public URL getCodeBase() { return _codebase; }
    
    /** Determines if the system is offline. See description in service interface */
    public boolean isOffline() { return _isOffline; }
    
    /**
     * Replaces the Web page currently being viewed with the given URL, or
     * launches a browser to show the given URL.
     *
     * @param   url   an absolute URL giving the location of the document.
     * @return  <code>true</code> if the request succeded, <code>false</code> otherwise.
     */
    public boolean showDocument(final URL url) {
        if (!isWebBrowserSupported()) return false;
        
        // Do priviledged is used as we may not be able to exec if
        // running in a sandbox.
        Boolean retValue = (Boolean)java.security.AccessController.doPrivileged
            (new java.security.PrivilegedAction() {
                    public Object run() {
                        URL absURL = url; // Convert to absolute URL
                        try { absURL = new URL(_codebase, url.toString());
                        } catch(MalformedURLException mue) { /* just ignore */ }
                        return new Boolean(BrowserSupport.showDocument(absURL));
                    }
                });
        return (retValue == null) ? false : retValue.booleanValue();
    }
    
    /**
     * Checks if a Web browser is supported on the current platform. If
     * this is <i>not</i> the case, then <code>{@link #showDocument}</code>
     * will always return <code>false</code>.
     *
     * @return <code>true</code> if a Web browser is supported on the
     *          current platform and by the current JNLP Client,
     *          otherwise <code>false</code>
     */
    public boolean isWebBrowserSupported() { return _isWebBrowserSupported; }
}

