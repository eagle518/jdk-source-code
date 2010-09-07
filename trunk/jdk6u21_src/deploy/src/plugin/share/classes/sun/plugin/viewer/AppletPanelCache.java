/*
 * @(#)AppletPanelCache.java	1.20 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer;

import java.lang.ref.SoftReference;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import sun.applet.AppletPanel;

/*
 * Applet panel cache for all the running applets
 */
public class AppletPanelCache
{    
    /**
     * Return a collection of all the accessible applet panels object
     */
    public static Object[] getAppletPanels()
    {
    	synchronized(appletPanels)
	{    
	    ArrayList arrayList = new ArrayList();
	    Collection collection = appletPanels.values();

	    for (Iterator iter = collection.iterator() ; iter.hasNext() ;) 
	    {
		SoftReference ref = (SoftReference) iter.next();

		if (ref != null)
		{
		    AppletPanel p = (AppletPanel) ref.get();

		    if (p != null) 
			arrayList.add(p);
		}
	    }

	    return arrayList.toArray();
	}
    }


    /* 
     * Add a appletPanel in this object cache
     * 
     * @param appletPanel the appletPanel to add
     */
    public static void add(AppletPanel appletPanel) {
	synchronized(appletPanels)
	{
	    appletPanels.put(new Integer(appletPanel.hashCode()), new SoftReference(appletPanel));
	}
    }

    /* 
     * Remove appletPanel from object cache
     * 
     * @param appletPanel appletPanel to remove
     */
    public static void remove(AppletPanel appletPanel) {
	synchronized(appletPanels)
	{
	    appletPanels.remove(new Integer(appletPanel.hashCode()));
	}
    }

    /**
     * Check if there is any valid instance in the cache
     */
    public static boolean hasValidInstance()
    {
	synchronized(appletPanels)
	{
	    Collection collection = appletPanels.values();

	    for (Iterator iter = collection.iterator() ; iter.hasNext() ;) 
	    {
		SoftReference ref = (SoftReference) iter.next();

		if (ref != null && ref.get() != null)
		    return true;
	    }

	    return false;
	}
    }

    private static HashMap appletPanels = new HashMap();
}   

