/*
 * @(#)PluginCookieSelector.java	1.5 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.net.URL;
import com.sun.deploy.net.cookie.*;

import sun.plugin2.applet.*;
import sun.plugin2.util.SystemUtil;

public class PluginCookieSelector extends DeployCookieSelector {
    private static final boolean DEBUG   = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);
    /** 
     *  Reset cookie selector.
     */
    public static void initialize()
    {
        // Set cookie selector
        java.net.CookieHandler.setDefault(new PluginCookieSelector());
    }

    protected void initializeImpl() {
    }

    protected void setCookieInBrowser(URL u, String value) throws CookieUnavailableException {
        Applet2ExecutionContext ctx = Plugin2Manager.getCurrentAppletExecutionContext();
        if (ctx != null) {
            ctx.setCookie(u, value);
        }
    }    
    
    protected String getCookieFromBrowser(URL u) throws CookieUnavailableException {
        Applet2ExecutionContext ctx = Plugin2Manager.getCurrentAppletExecutionContext();
        if (ctx != null) {
            return ctx.getCookie(u);
        }
        return null;
    }
}
