/*
 * @(#)BeansViewer.java	1.20 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/*
 * A recipient to show a Java Beans in.
 * This class implements the AppletContext used by all applets loaded
 * within a running instance of the bridge.
 *
 * @version 	1.0
 * @author	Stanley Man-Kit Ho
 */

import java.beans.Beans; 
import java.applet.Applet;
import java.awt.Component;
import java.io.IOException;
import sun.applet.AppletClassLoader;
import java.net.URL;
import java.io.ByteArrayInputStream;
import java.beans.XMLDecoder;
import java.net.MalformedURLException;
import com.sun.deploy.util.URLUtil;
import javax.swing.JComponent;
import javax.swing.LayoutFocusTraversalPolicy;
                       
public class BeansViewer extends sun.plugin.AppletViewer 
{
    public final static int APPLET_DISPOSE = 0;
    public final static int APPLET_ERROR = 7;

    static public Applet createJavaBeanComponent(AppletViewer appletViewer, AppletClassLoader loader) throws ClassNotFoundException,
	IllegalAccessException, IOException, InstantiationException, InterruptedException {

	Applet applet;
	String serName = appletViewer.getSerializedObject();
	String code = appletViewer.getCode();

	if (code != null && serName != null) {
            System.err.println(AppletViewer.getMessage("bean_code_and_ser"));
	    return null;
	}
	if (code == null && serName == null) {
            String msg = "nocode";
	    appletViewer.setStatus(APPLET_ERROR);
	    appletViewer.showAppletStatus(msg);
	    appletViewer.showAppletLog(msg);
	    appletViewer.repaint();
	}
	if (code != null) {
	    Object bean = Beans.instantiate(loader, code);
            applet = createApplet(bean);
	    appletViewer.setDoInit(true);
	} else {
	    // serName is not null;
	    Object bean = Beans.instantiate(loader, serName);
            applet = createApplet(bean);
	    appletViewer.setDoInit(false); // skip over the first init
	}
	if (Thread.interrupted()) {
	    try {
		appletViewer.setStatus(APPLET_DISPOSE); // APPLET_ERROR?
		applet = null;
		// REMIND: This may not be exactly the right thing: the
		// status is set by the stop button and not necessarily
		// here.
		appletViewer.showAppletStatus("death");
	    } finally {
		Thread.currentThread().interrupt(); // resignal interrupt
	    }
            return null;
	}

	return applet;
    }


    /**
     * <p> Create a JavaBeans by putting it into an applet wrapper.
     * </p>
     *
     * @param loader Applet classLoader.
     * @returns applet wrapper which hosts the JavaBeans.
     */
    protected Applet createApplet(AppletClassLoader loader) throws ClassNotFoundException,
    IllegalAccessException, IOException, InstantiationException, InterruptedException 
    {
	if(bytes == null)
	    return createJavaBeanComponent(this, loader);
	else {
	    //System.out.println("Loading ......");
	    ByteArrayInputStream bis = new ByteArrayInputStream(bytes);
	    XMLDecoder xml = new XMLDecoder(bis);
	    Object bean = xml.readObject();
            Applet applet = createApplet(bean);
	    bis.close();
	    setDoInit(true);
	    return applet;
	}
    }

    /*
     * <p>
     * @return the java component displayed by this viewer class
     * </p>
     */
    protected String getHandledType() {
	return "JavaBeans";
    }

   /**
     * Get the base url.
     */
    public URL getCodeBase() {
        if (codeBaseInit == false) {
            String att = getParameter("java_codebase");
            if (att == null)
                att = getParameter("codebase");
		
	    if (att != null) {
		if (!att.equals(".") && !att.endsWith("/")) {
		    att += "/";
		}

		// Canonicalize URL in case the URL is in some
		// weird form only recognized by the browser
		try {
		    baseURL = new URL(URLUtil.canonicalize(att));
		} catch (MalformedURLException e) {
		    // ignore exception
		}
	    }
            codeBaseInit = true;
	}
	return baseURL;
    }

    public URL getDocumentBase() {
        return null;
    }

    public void setByteStream(byte[] bArray) {
	bytes = bArray;
    }

    static Applet createApplet(Object bean) {
	Applet applet = new BeansApplet(bean);
	//If the javabean is Swing based, set the focus traversal policy to the
	//swing's default and make it policy provider, bug #4947607 
	if(bean instanceof JComponent) {
	    applet.setFocusTraversalPolicy(new LayoutFocusTraversalPolicy());
	    applet.setFocusTraversalPolicyProvider(true);
	}
	return applet;
    }

    private byte[] bytes = null;
}
