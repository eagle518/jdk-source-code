/*
 * @(#)DefaultPluginAppletContext.java	1.44 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.context;

/*
 * Helper class that implement most of the AppletContext
 *
 * @version 	1.2.1
 * @author	Jerome Dochez
 */

import java.applet.Applet;
import java.applet.AudioClip;
import java.awt.Image;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.ref.SoftReference;
import java.net.URL;
import java.net.SocketPermission;
import java.util.Collection;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import netscape.javascript.JSObject;
import sun.applet.AppletPanel;
import sun.plugin.javascript.JSContext;
import sun.plugin.viewer.AppletPanelCache;
import java.util.ArrayList;
import java.security.Permission;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

public abstract class DefaultPluginAppletContext implements PluginAppletContext
{    
    /*
     * Get an audio clip.
     *
     * @param url url of the desired audio clip
     */
    public AudioClip getAudioClip(final URL url) 
    {
	// Fixed #4508940: MSVM takes null URL and return null
	//
	if (url == null)
	    return null;

	SecurityManager sm = System.getSecurityManager();
	if (sm != null) {
	    final Permission perm;
	    try {
		perm = url.openConnection().getPermission();
	    } catch (IOException exc) {
		// Deny
		return null;
	    }

	    if (perm == null) {
		// null permissions implies no permissions required
	    }
	    else {
	        sm.checkPermission(perm);
	    }
	}

	SoftReference ref = null;

	synchronized (audioClipStore)
	{
	    // Obtain audio list according to applet context
	    HashMap audioClips = (HashMap) audioClipStore.get(appletPanel.getCodeBase());
	    
	    if (audioClips == null)
	    {
		audioClips = new HashMap();
		audioClipStore.put(appletPanel.getCodeBase(), audioClips);
	    }

	    ref = (SoftReference) audioClips.get(url);

	    if (ref == null || ref.get() == null)
	    {
		ref = new SoftReference(AppletAudioClipFactory.createAudioClip(url));
		audioClips.put(url, ref);
	    }
	}

	// Get audio clip outside sync block
	AudioClip clip = (AudioClip) ref.get();

	Trace.msgPrintln("appletcontext.audio.loaded", new Object[] {url}, TraceLevel.BASIC);

	return clip;
    }


    /*
     * Get an image.
     * 
     * @param url of the desired image
     */
    public Image getImage(URL url) 
    {
	// Fixed #4508940: MSVM takes null URL and return null
	//
	if (url == null)
	    return null;

	SecurityManager sm = System.getSecurityManager();
	if (sm != null) {
	    final Permission perm;
	    try {
		perm = url.openConnection().getPermission();
	    } catch (IOException exc) {
		// Deny
		return null;
	    }

	    if (perm == null) {
		// null permissions implies no permissions required
	    }
	    else {
	        sm.checkPermission(perm);
	    }
	}

	SoftReference ref = null;

	synchronized (imageRefs) 
	{
	    ref = (SoftReference) imageRefs.get(url);

	    if (ref == null || ref.get() == null) 
	    {
		ref = new SoftReference(AppletImageFactory.createImage(url));
		
		imageRefs.put(url, ref);
	    }

	}

	Image image = (Image) ref.get();

	Trace.msgPrintln("appletcontext.image.loaded", new Object[] {url}, TraceLevel.BASIC);

	return image;
    }


    /**
     * Get an applet by name.
     */
    public Applet getApplet(String name) 
    {
	name = name.toLowerCase();

	Object[] appletPanels = AppletPanelCache.getAppletPanels();

	for (int i=0; i < appletPanels.length; i++) 
	{
	    AppletPanel p = (AppletPanel) appletPanels[i];

	    if (p != null)
	    {
		// Skip applet if it is not active and not itself
		if (!p.isActive() && appletPanel != p)
		    continue;

		String param = p.getParameter("name");
		if (param != null) {
		    param = param.toLowerCase();
		}
		if (name.equals(param) && 
		    p.getDocumentBase().equals(appletPanel.getDocumentBase())) {
		    try {
			if (checkConnect(appletPanel.getCodeBase().getHost(),
				     p.getCodeBase().getHost())==false)
			    return null;
		    } catch (InvocationTargetException ee) {
			showStatus(ee.getTargetException().getMessage());
			return null;
		    } catch (Exception ee) {
			showStatus(ee.getMessage());
			return null;
		    }
		    return p.getApplet();
		}
	    }
	}
	return null;
    }

    /**
     * Return an enumeration of all the accessible
     * applets on this page.
     */
    public Enumeration getApplets() 
    {
	Vector v = new Vector();

	Object[] appletPanels = AppletPanelCache.getAppletPanels();

	for (int i=0; i < appletPanels.length; i++) 
	{
	    AppletPanel p = (AppletPanel) appletPanels[i];

		// p.isActive() is added to prevent premature call to p.getDocumentBase(). Since
	    // on Netscape 4.x, we use javascript to obtain the document base.
	    // However, this should be reviewed later on. isActive is defined as status = APPLET_START,
	    // and javascript capability should be ready after APPLET_INIT.
	    // To minmumize regression, current fix only adds calling applet itself to 
	    // the result, no matter what state of the applet. 
	    
	    if (p != null && p.isActive() && p.getDocumentBase().equals(appletPanel.getDocumentBase())) 
	    {
		try {
		    if(checkConnect(appletPanel.getCodeBase().getHost(), p.getCodeBase().getHost())) {
				v.addElement(p.getApplet());
		    }
		} catch (InvocationTargetException ee) {
		    showStatus(ee.getTargetException().getMessage());
		} catch (Exception ee) {
		    showStatus(ee.getMessage());
		}
	    }
	}

	// applet should always get itself
	Applet applet = appletPanel.getApplet();
	if(!v.contains(applet))
	    v.addElement(applet);

	return v.elements();
    }

    /*
     * <p>
     * Check applet connection rights.
     * </p>
     * @param sourceHostName host originating the applet
     * @param targetHostName host the applet requested a connection to
     * 
     * @return true if the applet originating from sourceHostName can make
     * a connection to targetHostName
     *
     */
    private boolean checkConnect(String sourceHostName, String targetHostName)
	throws Exception
    {   
	SocketPermission panelSp = 
	    new SocketPermission(sourceHostName,
		       		 "connect");
	SocketPermission sp =
	    new SocketPermission(targetHostName,
	       			 "connect");
	if (panelSp.implies(sp)) {
	    return true;
	}	
	return false;
    }

    /*
     * Replaces the Web page currently being viewed with the given URL
     *
     * @param url the address to transfer to
     */
    public void showDocument(URL url)
    {
	showDocument(url, "_top");
    }


    /*
     * Requests that the browser or applet viewer show the Web page
     * indicated by the url argument. 
     *
     * @param url the address to transfer to
     * @param target One of the value
     *	"_self"  show in the current frame
     *  "_parent"show in the parent frame
     *  "_top"   show in the topmost frame
     *  "_blank" show in a new unnamed top-level windownameshow in a 
     *           new top-level window named name
     */
    public void showDocument(URL url, String target)
    {
	// The target name MUST be either alphanumeric or 
	// underscore character, or the "open" call won't
	// work. So it is VERY important to do the proper
	// conversion.
	//
	StringBuffer buffer = new StringBuffer();
	for (int i=0; i < target.length(); i++)
	{
	    char c = target.charAt(i);

	    if (Character.isLetterOrDigit(c) || c == '_')
		buffer.append(c);
	    else
		buffer.append('_');
	}			    

	// Call template method
	doShowDocument(url, buffer.toString());
    }


    /*
     * Template method for show document
     *
     * @param url the address to transfer to
     * @param target One of the value
     *	"_self"  show in the current frame
     *  "_parent"show in the parent frame
     *  "_top"   show in the topmost frame
     *  "_blank" show in a new unnamed top-level windownameshow in a 
     *           new top-level window named name
     */
    public void doShowDocument(final URL url, final String target)
    {
	// A thread is spawned for each showDocument call 
	// because we will call back to JSObject, which
	// may block the caller.
	//

	final PluginAppletContext pac = this;

	Thread showDocThread = new Thread(new Runnable()
	{
	    public void run()
	    {
		try
		{
		    JSObject win = getJSObject();
		    
		    if (win != null)
		    {
			Object[] args = new Object[2];
			args[0] = url.toString();	    
			args[1] = target;

			win.call("open", args);
		    }
		}
		catch (Throwable e)
		{
		    e.printStackTrace();
		}
	    }
	}, "showDocument Thread");

	showDocThread.start();
    }

    /*
     * Show status.
     * 
     * @param status status message
     */ 
    public void showStatus(String status)
    {
	if (status == null)
	    return;

	// Strip off "\n"
	int index = status.indexOf("\n");

	// Call template method
	if (index != -1)
	    doShowStatus(status.substring(0, index));
	else
	    doShowStatus(status);
    }


    /*
     * Template method for show status.
     * 
     * @param status status message
     */ 
    protected void doShowStatus(final String status)
    {
	// A thread is spawned for each status call 
	// because we will call back to JSObject, which
	// may block the caller.
	//

	final PluginAppletContext pac = this;

	Thread statusThread = new Thread(new Runnable()
	{
	    public void run()
	    {
		try
		{
		    JSObject win = pac.getJSObject();

		    // Use timer in javascript to make sure the status is 
		    // displayed properly.

		    // Use "javascipt:void(xxxxx)" to avoid
		    // return value
		    //
	//	    win.eval("function setStatus() { self.status='" + status + "';"
	//		    + "setTimeout(\"setStatus()\", 1500);};void(setStatus())");
    		    
		    if (win != null)
		    {
			win.eval("function setStatus() { self.status='" + status + "';};"
				+ "void(setTimeout(\"setStatus()\", 1500))");
		    }	    
		}
		catch (Throwable e)
		{
		    e.printStackTrace();
		}
	    }
	}, "showStatus Thread");

	statusThread.start();
    }

    /* 
     * Add a appletPanel in this applet context
     * 
     * @param appletPanel the appletPanel to add
     */
    public void addAppletPanelInContext(AppletPanel appletPanel) {
	this.appletPanel = appletPanel;
	AppletPanelCache.add(appletPanel);
    }

    /* 
     * Remove appletPanel from this applet context
     * 
     * @param appletPanel appletPanel to remove
     */
    public void removeAppletPanelFromContext(AppletPanel appletPanel) {
	AppletPanelCache.remove(appletPanel);
    }
    
    public void setStream(String name, InputStream is)throws IOException
    {
	HashMap streamMap = (HashMap) streamStore.get(appletPanel.getCodeBase());

	if (streamMap == null)
	{
	    streamMap = new HashMap();
	    streamStore.put(appletPanel.getCodeBase(), streamMap);
	}

	synchronized(streamMap)
	{
	    if(is != null)
	    {
		byte[] data = (byte[]) streamMap.get(name);

		if (data == null)
		{
		    int streamSize = is.available();
		    if(streamSize < persistStreamMaxSize)
		    {
			data = new byte[streamSize];
			is.read(data, 0, streamSize);
			streamMap.put(name, data);
		    }
		    else
		    {
			throw new IOException("Stream size exceeds the maximum limit");
		    }
		}
		else
		{
		    //If PeristStream already exists with the same name,
		    //replace the older byte stream by the newer one
		    streamMap.remove(name);
		    setStream(name, is);
		}
	    }
	    else
	    {
		//remove the persist stream from the map
		streamMap.remove(name);
	    }
	}
    }

    public InputStream getStream(String name)//throws StreamNotFoundException
    {
	ByteArrayInputStream bAIS = null;
	HashMap streamMap = (HashMap) streamStore.get(appletPanel.getCodeBase());

	if (streamMap != null)
	{
	    synchronized(streamMap)
	    {
		byte[] data = (byte[]) streamMap.get(name);

		if (data != null)
		    bAIS = new ByteArrayInputStream(data);
	    }
	}
	return bAIS;
    }

    public Iterator getStreamKeys()
    {
    	Iterator iter = null;
	HashMap streamMap = (HashMap) streamStore.get(appletPanel.getCodeBase());
	if(streamMap != null)
	{
	    synchronized(streamMap)
	    {
		iter = streamMap.keySet().iterator();
	    }
	}
	return iter;
    }


    private String getNameFromURL(URL url)
    {
	try
	{
	    String file = url.getFile();
	    int last = file.lastIndexOf('/');

	    return file.substring(last + 1);
	}
	catch (Throwable e)
	{
	    return null;
	}
    }

     /*
     * <p>
     * Add a JSObject to the list of exported JSObjects so it gets properly
     * released upon applet destruction
     * </p>
     * @param JSObject instance to add
     */
    public void addJSObjectToExportedList(netscape.javascript.JSObject jsObject) 
    {
	synchronized (exported)
	{
	    exported.add(new SoftReference(jsObject));
	}
    }

    public void onClose() {
	synchronized(exported)
	{
	    for (java.util.Iterator iter = exported.iterator(); iter.hasNext();) {
		SoftReference ref = (SoftReference) iter.next();

		if (ref != null)
		{
		    JSObject obj = (JSObject) ref.get();
		    
		    if (obj != null
			&& obj instanceof sun.plugin.javascript.JSObject) {
			((sun.plugin.javascript.JSObject)obj).cleanup();
		    }
		}
	    }

	    exported.clear();
	}
    }


    private ArrayList exported = new ArrayList();

    protected AppletPanel appletPanel;
    
    private static HashMap imageRefs = new HashMap();
    private static HashMap audioClipStore = new HashMap();

    private int persistStreamMaxSize = 65536;
    private static HashMap streamStore = new HashMap();
}   

