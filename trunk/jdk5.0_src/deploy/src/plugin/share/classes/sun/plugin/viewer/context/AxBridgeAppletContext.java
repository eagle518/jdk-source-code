/*
 * @(#)AxBridgeAppletContext.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.context;

import java.awt.Image;
import java.net.URL;
import java.util.Enumeration;
import java.io.InputStream;
import java.io.IOException;
import java.util.Iterator;
import java.applet.AppletContext;
import java.applet.Applet;
import java.applet.AudioClip;
import sun.applet.AppletPanel;


public class AxBridgeAppletContext implements PluginAppletContext 
{
    public AxBridgeAppletContext(){}

    public AudioClip getAudioClip(URL url) { return null;}

    public Image getImage(URL url) { return null;}

    public Applet getApplet(String name){ return null;}

    public Enumeration getApplets(){ return null;}

    public void showDocument(URL url) {}

    public void showDocument(URL url, String target){}

    public void showStatus(String status){}

    public void setStream(String key, InputStream stream)throws IOException{}

    public InputStream getStream(String key){ return null;}

    public Iterator getStreamKeys(){ return null;}

    public netscape.javascript.JSObject getJSObject(){ return null;}

    public void addAppletPanelInContext(AppletPanel panel){}

    public void removeAppletPanelFromContext(AppletPanel panel){}

    public void setAppletContextHandle(int handle){}
}

