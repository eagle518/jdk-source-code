/*
 * @(#)BasicService.java	1.14 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;
import java.net.URL;

/**
 *  The <code>BasicService</code> interface provides access to the codebase
 * of the application, if an application is run in offline mode,
 *  and simple interaction with the native browser on the
 *  given platform.
 *  <p>
 *  This interface mimics loosely the <code>AppletContext</code>
 *  functionality.
 *
 * @since 1.0
 */
public interface BasicService {
    
    /**
     * Returns the codebase for the application. The codebase is either
     * specified directly in the JNLP file, or it is the location of the JAR
     * file containing the main class of the application.
     *
     *  @return a URL with the codebase of the application
     */
    public URL getCodeBase();
        
    /**
     * Determines if the system is offline.  The return value represents the JNLP client's
     * "best guess" at the online / offline state of the client system.  The return value
     * is does not have to be guaranteed to be reliable, as it is sometimes difficult to
     * ascertain the true online / offline state of a client system.
     *
     * @return  <code>true</code> if the system is offline, otherwise <code>false</code>
     */
    public boolean isOffline();
    
     /**
     * Directs a browser on the client to show the given URL. This will typically
     * replace the page currently being viewed in a browser with the given URL, or
     * cause a browser to be launched that will show the given URL.
     *
     * @param   url   an URL giving the location of the document. A relative URL
     *                will be relative to the codebase.
     * @return  <code>true</code> if the request succeded, otherwise <code>false</code>
     */
    public boolean showDocument(URL url);

    /**
     * Checks if a Web browser is supported on the current platform and by the
     * given JNLP Client. If this is <i>not</i> the case, then <code>{@link #showDocument}</code>
     * will always return <code>false</code>.
     *
     * @return <code>true</code> if a Web browser is supported, otherwise <code>false</code>
     */
    public boolean isWebBrowserSupported();
}

