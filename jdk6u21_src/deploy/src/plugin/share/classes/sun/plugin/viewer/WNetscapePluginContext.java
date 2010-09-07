/*
 * @(#)WNetscapePluginContext.java	1.40 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Entry point for the plugin native code to call the JVM.
 *
 * @author Stanley Man-Kit Ho
 */

package sun.plugin.viewer;

import java.lang.ref.SoftReference;
import java.util.Iterator;
import java.util.HashMap;
import com.sun.deploy.perf.DeployPerfUtil;


/** 
 * <p> WNetscapePluginContext is a class that contains all the methods that are not
 * plugin instance specific inside Win32 plugin.
 * </p>
 */
public class WNetscapePluginContext
{
	private static String PLUGIN_UNIQUE_ID = "A8F70EB5-AAEF-11d6-95A4-0050BAAC8BD3";
    /**
     *  This class must be a static class.
     */
    private WNetscapePluginContext() {
    }
     
    /**
     * <p> Creates a new Panel according to the mimeType. It is called by the 
     * Plugin when NPP_SetWindow is called.
     * </p>
     *
     * @param mimeType MIME type.
     * @param k Array of param names.
     * @param v Array of param values.
     * @param instance Plugin instance.
     * @return New Java Plugin object. 
     */
    static WNetscapePluginObject createPluginObject(String mimeType, String[] k,       
						    String[] v, int instance)     
    {
        DeployPerfUtil.put("START - Java   - ENV - create browser plugin object (Mozilla:Windows)");

	boolean isBeans = (mimeType.indexOf("application/x-java-bean") >= 0);

	WNetscapePluginObject p = new WNetscapePluginObject(instance, isBeans, 
							    LifeCycleManager.getIdentifier(k, v));

        // Put all elements in the parameter list
	for (int i = 0; i < k.length; i++)
	{
	    if (k[i] != null && !PLUGIN_UNIQUE_ID.equals(k[i]))
			p.setParameter(k[i], v[i]);
	}
        
        /*
         * Set background/foreground and progress bar color for the applet viewer.
         * Do it here - after passing all HTML params to applet viewer.
         */
        p.setBoxColors();

        DeployPerfUtil.put("END   - Java   - ENV - create browser plugin object (Mozilla:Windows)");

	return p;
    }
}
