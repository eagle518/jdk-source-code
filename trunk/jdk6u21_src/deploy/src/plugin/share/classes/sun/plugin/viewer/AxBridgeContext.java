/*
 * @(#)AxBridgeContext.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer;

import java.lang.ref.SoftReference;
import java.util.Iterator;
import java.util.HashMap;
import java.util.Set;

/** 
 * <p> PluginContext is a class that contains all the methods that are not
 * plugin instance specific inside Win32 plugin.
 * </p>
 */
public class AxBridgeContext
{
    /**
     *  This class must be a static class.
     */
    private AxBridgeContext() {
    }
   
    /**
     * @param isBeans indicates JavaBean
     * @param k Array of param names.
     * @param v Array of param values.
     * @param instance Plugin instance.
     * @return New Java Plugin object. 
     */
    static AxBridgeObject createBeansObject(boolean isBeans, String[] k, String[] v, int instance)     
    {
	//System.out.println("CrateBeansObject called");
	AxBridgeObject p = new AxBridgeObject(instance, isBeans, 
					    LifeCycleManager.getIdentifier(k, v));

        // Put all elements in the parameter list
	for (int i=0; i < k.length; i++) { 
	    // array element could be null
	    if (k[i] != null)
		p.setParameter(k[i], v[i]);
	}

        /*
         * Set background/foreground and progress bar color for the applet viewer.
         * Do it here - after passing all HTML params to applet viewer.
         */
        p.setBoxColors();
        
	return p; 
    }
}

 


