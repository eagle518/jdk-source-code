/*
 * @(#)Netscape4CookieHandler.java	1.27 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.cookie;

import java.net.URL;
import com.sun.deploy.net.cookie.CookieHandler;
import com.sun.deploy.net.cookie.CookieUnavailableException;
import sun.plugin.viewer.AppletPanelCache;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.applet.AppletPanel;
import netscape.javascript.JSException;
import netscape.javascript.JSObject;


/**
 * <p> Netscape4CookieHandler is a class that encapsulates the mechanism for
 * obtaining the cookie value of a particular URL in Netscape 4.
 * </p>
 */
public final class Netscape4CookieHandler implements CookieHandler 
{
    /**
     * <p> Sets the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url
     * @param String contains the corresponding cookie value.
     */
    public synchronized void setCookieInfo(URL url, String value)
	throws CookieUnavailableException
    {
	if (AppletPanelCache.hasValidInstance() == false) {
	    // Cookie service is NOT available
	    throw new CookieUnavailableException("Cookie service is not available for " + url);
	}
        PluginAppletContext pac = getMatchedApplet(url.toString());
        if(null != pac) {
            try {
                JSObject win = pac.getJSObject();
		if (win == null)
		    throw new JSException("Unable to obtain Window object");

                JSObject doc = (JSObject)win.getMember("document");
		if (doc == null)
		    throw new JSException("Unable to obtain Document object");

                doc.setMember("cookie", value);
            }
            catch(JSException e) {
                e.printStackTrace();
            }
       }
    }

    /**
     * <p> Returns the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @returns String contains the corresponding cookie value.
     */
    public synchronized String getCookieInfo(URL url)
			       throws CookieUnavailableException
    {
	// Cookie service in Netscape 4 is emulated through JavaScript 
	// URL by using one of the plugin instances. Thus, there MUST 
	// be at least one plugin instance exists. Otherwise, accessing 
	// the cookie service without any plugin instance will likely 
	// to confuse the browser and fail. Thus, it is VERY important 
	// to check if there is at least one applet instance before 
	// calling back to the browser.
	//
	if (AppletPanelCache.hasValidInstance() == false) {
	    // Cookie service is NOT available
	    throw new CookieUnavailableException("Cookie service is not available for " + url);
	}

        PluginAppletContext pac = getMatchedApplet(url.toString());
        if(null == pac)
            return null;
		    
	try {
	    JSObject win = pac.getJSObject();
	    if (win == null)
		throw new JSException("Unable to obtain Window object");

	    JSObject doc = (JSObject) win.getMember("document");
	    if (doc == null)
		throw new JSException("Unable to obtain Document object");
                        
	    return (String) doc.getMember("cookie");
	} 
        catch (JSException e) {
	    e.printStackTrace();
	}

	return null;
    }

    /** ** This is a fix to the cookie security bug. **
     *
     * The idea here is to obtain the cookie for a
     * particular URL. Thus, we will try to do a best
     * match over the document base of all the plugin
     * instance and the requested URL.
     *
     * Cookie will ONLY be returned if the protocol, host,
     * and file path (without the filename) of the document
     * base of the plugin instance is a substring of the
     * requested URL.
     * 
     * For example, if there are two plugin instances
     * and their document bases are:
     * 1) http://java.sun.com/a/b/c.html 
     * 2) http://www.evil.com/x/y/z.html
     *
     * They will be converted into
     * 1) http://java.sun.com/a/b/
     * 2) http://www.evil.com/x/y/
     *
     * Requesting URL http://java.sun.com/a/b/k.html
     * will result in getting the cookie of (1) because
     * (1) is a substring of the requested URL.
     *
     * If the request URL is http://java.sun.com/a/f.html,
     * the result will be NO cookie because (1) is not
     * a substring of the URL. However, if the request URL
     * is http://java.sun.com/a/b/c/g.html, the result will
     * be cookie of (1).
     *
     *
     * In case there are multiple matches, for example
     * 1) http://java.sun.com/a/b/c.html -> http://java.sun.com/a/b/
     * 2) http://java.sun.com/a/b/k.html -> http://java.sun.com/a/b/
     * 3) http://java.sun.com/a/b/d/t.html -> http://java.sun.com/a/b/d/
     *
     * If the request URL is http://java.sun.com/a/b/d/p.html, only COOKIE
     * of (3) will be returned because it is the best match.
     * 
     * On the other hand, if the request URL is http://java.sun.com/a/b/s/g/p.html,
     * both (1) and (2) will match. However, since both
     * plugin instances are from the same host with same protocol
     * and file path, the cookie MUST be the same, according to
     * the spec. As a result, only the cookie of (1) will be 
     * returned because (1) is the first-best match.
     * This is the end 
     */
    protected PluginAppletContext getMatchedApplet(String url)
    {
	// Obtain the truncated version of the url
	String urlString = truncateURL(url);

	// Enumerate through the applet panel cache,
	// and found an applet panel that it
	Object[] appletPanels = AppletPanelCache.getAppletPanels();

	for (int i=0; i < appletPanels.length; i++) {
	    AppletPanel p = (AppletPanel) appletPanels[i];

	    if (p != null){
		URL docBase = p.getDocumentBase();
		String truncatedDocBase = truncateURL(docBase.toString());
		if (urlString.indexOf(truncatedDocBase) != -1) {
		    // Found the match
		    return (PluginAppletContext) p.getAppletContext();
                }
            }
        }
        return null;
    }
    
    /**
     * truncateURL remove all the character after the last '/'
     * in the URL.
     *
     * @param url URL to be truncated
     * @return Truncated URL string
     */
    private String truncateURL(String url)
    {
	int index = url.lastIndexOf('/');

	if (index != -1)
	    return url.substring(0, index);
	else
	    return url;
    }
}


