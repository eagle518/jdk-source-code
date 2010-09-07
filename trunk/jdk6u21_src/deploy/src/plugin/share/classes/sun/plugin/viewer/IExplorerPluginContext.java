/*
 * @(#)IExplorerPluginContext.java	1.18 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer;

import java.lang.ref.SoftReference;
import java.util.Iterator;
import java.util.HashMap;
import java.util.Set;
import com.sun.deploy.perf.DeployPerfUtil;
import sun.plugin.viewer.frame.IExplorerEmbeddedFrame;

/** 
 * <p> PluginContext is a class that contains all the methods that are not
 * plugin instance specific inside Win32 plugin.
 * </p>
 */
public class IExplorerPluginContext
{
    /**
     *  This class must be a static class.
     */
    private IExplorerPluginContext() {
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
    static IExplorerPluginObject createPluginObject(boolean isBeans, String[] k, String[] v, int instance)     
    {
        DeployPerfUtil.put("START - Java   - ENV - create browser plugin object (IE)");

	IExplorerPluginObject p = new IExplorerPluginObject(instance, isBeans, 
    							    LifeCycleManager.getIdentifier(k, v));


        // Put all elements in the parameter list
	for (int i=0; i < k.length; i++)
	{ 
	    // array element could be null
	    if (k[i] != null)
		p.setParameter(k[i], v[i]);
	}

        /*
         * Set background/foreground and progress bar color for the applet viewer.
         * Do it here - after passing all HTML params to applet viewer.
         */
        p.setBoxColors();

        DeployPerfUtil.put("END   - Java   - ENV - create browser plugin object (IE)");

	return p; 
    }
}
