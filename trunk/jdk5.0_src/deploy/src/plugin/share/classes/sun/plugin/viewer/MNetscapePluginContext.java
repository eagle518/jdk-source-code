/*
 * @(#)MNetscapePluginContext.java	1.5 01/12/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer;


/** 
 * <p> MNetscapePluginContext is a class that contains all the methods that 
 * are not plugin instance specific inside Unix plugin.
 * </p>
 */
public class MNetscapePluginContext
{
	private static String PLUGIN_UNIQUE_ID = "A8F70EB5-AAEF-11d6-95A4-0050BAAC8BD3";
    /**
     *  This class must be a static class.
     */
    private MNetscapePluginContext() {
    }
     
    /**
     * <p> Creates a new Panel according to the mimeType. It is called by the 
     * Plugin when NPP_SetWindow is called.
     * </p>
     *
     * @param isBeans Is JavaBeans
     * @param k Array of param names.
     * @param v Array of param values.
     * @param instance Plugin instance.
     * @return New Java Plugin object. 
     */
    public static MNetscapePluginObject createPluginObject(boolean isBeans, 
							   String[] k,       
							   String[] v, 
							   int instance)     
    {
	MNetscapePluginObject p = new MNetscapePluginObject(instance, isBeans, 
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

	return p;
    }

}

 


