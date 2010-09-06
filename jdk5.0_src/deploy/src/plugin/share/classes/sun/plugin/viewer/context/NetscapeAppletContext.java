/*
 * @(#)NetscapeAppletContext.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.context;


import sun.plugin.services.BrowserService;
import sun.plugin.javascript.navig5.JSObject;

/**
 * This class corresponds to an applet's environment: the
 * document containing the applet and the other applets in the same
 * document in Netscape.
 * <p>
 * The methods in this interface can be used by an applet to obtain
 * information about its environment.
 *
 */
public class NetscapeAppletContext extends DefaultPluginAppletContext
{
    protected int instance = -1;

    /**
     * Create a NetscapeAppletContext object.
     */
    public NetscapeAppletContext()
    {
    }

    /** 
     * <p> Return the JSObject implementation for this applet
     * </p>
     *
     * @return JSObject for the window object
     */
    public netscape.javascript.JSObject getJSObject()  
    {
	// If instance is less than 0, the applet has been 
	// stopped but still running in the background.
	if (instance < 0)
	    return null;

	BrowserService service = (BrowserService) com.sun.deploy.services.ServiceManager.getService();

	if (service.getBrowserVersion() >= 5.0f) {
	     JSObject jsObj = new sun.plugin.javascript.navig5.JSObject(instance);
	     jsObj.setNetscapeAppletContext(this);
	     return jsObj;
	}
	else if (service.getBrowserVersion() < 4.0f)
	    return new sun.plugin.javascript.navig.Window(instance);
	else
	    return new sun.plugin.javascript.navig4.Window(instance);
    }


    /**
     * Set the underlying handle of the Applet context
     * 
     * @param handle Handle
     */
    public void setAppletContextHandle(int instance)
    {
	this.instance = instance;
    }

     /**
      * <p>Get the underlying handle of the Applet context
      * </p>
      * @param handle Handle
      */
    public int getAppletContextHandle()
    {
	return instance;
    }

    
}

