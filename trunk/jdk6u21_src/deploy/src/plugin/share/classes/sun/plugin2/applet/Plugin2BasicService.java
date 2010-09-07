/*
 * @(#)Plugin2BasicService.java	1.5 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin2.applet;

import java.applet.AppletContext;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;
import javax.jnlp.BasicService;
import sun.plugin.perf.PluginRollup;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

import com.sun.deploy.net.offline.DeployOfflineManager;

import sun.awt.AppContext;
import com.sun.deploy.perf.DeployPerfUtil;
import java.security.AccessController;
import java.security.CodeSource;
import java.security.PrivilegedAction;

public final class Plugin2BasicService 
    implements javax.jnlp.BasicService
{
    AppletContext _ac;
    URL _cb;
    protected Plugin2BasicService(URL cb) 
    { 
        _ac = (AppletContext) AppContext.getAppContext().get(
                Applet2Manager.APPCONTEXT_APPLETCONTEXT_KEY);
        _cb= cb; 
    }

    public URL getCodeBase() {
        return _cb;
    }

    public boolean isOffline() {
         return DeployOfflineManager.isGlobalOffline();
    }

    public boolean isWebBrowserSupported() {
	// append performance data to a file if performance data collection
	// is enabled
	if (DeployPerfUtil.isEnabled() && DeployPerfUtil.isDeployFirstframePerfEnabled()) {
	    DeployPerfUtil.put("Plugin2BasicService.isWebBrowserSupported called");

	    // this method is called from applet and usually doesn't have privilege
	    // to perform file I/O. Therefore, a doPrivileged block is required to
	    // write performance data to a file.
	    AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
		    try {
			DeployPerfUtil.write(new PluginRollup(), true);
			Trace.println("completed perf rollup", TraceLevel.BASIC);
		    } catch (IOException ioe) {
			// ignore exception
		    } 
		    return null;
		}
	    });
	}
        return true;
    }

    public boolean showDocument(URL url) {
        if (_ac != null) {

            // Convert to absolute URL
            try {
                url = new URL(_cb, url.toString());
            } catch(MalformedURLException mue) { /* just ignore */ }

            _ac.showDocument(url, "_blank");
            return true;
        }
        return false;
    }
}

