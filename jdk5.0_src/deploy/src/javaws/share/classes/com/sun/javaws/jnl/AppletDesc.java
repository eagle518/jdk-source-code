/*
 * @(#)AppletDesc.java	1.11 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import java.net.URL;
import java.util.Properties;
import java.util.Enumeration;
import com.sun.deploy.xml.*;

/**
 *   Description of parameters to an applet
 *
 */
public class AppletDesc implements XMLable {
    private String     _name;
    private String     _appletClass;
    private URL        _documentBase;
    private int        _width;
    private int        _height;
    private Properties _params;
    
    public AppletDesc(String name, String appletClass, URL documentBase, int width, int height, Properties params) {
	_name = name;
	_appletClass = appletClass;
	_documentBase = documentBase;
	_width = width;
	_height = height;
	_params = params;
    }
    
    /** Returns the name of the applet */
    public String getName() { return _name; }
    
    /** Get the main class for the Applet */
    public String getAppletClass() { return _appletClass; };
    
    /** Returns documentbase for the applet. */
    public URL getDocumentBase() { return _documentBase; };
    
    /** Returns width of Applet */
    public int getWidth() { return _width; };
    
    /** Returns height of Applet */
    public int getHeight() { return _height; };
    
    /** Returns parameters */
    public Properties getParameters() { return _params; };
    
    /** Returns contents as XML */
    public XMLNode asXML() {
	XMLAttributeBuilder ab = new XMLAttributeBuilder();
	ab.add("name", _name);
	ab.add("code", _appletClass);
	ab.add("documentbase", _documentBase);
	ab.add("width", _width);
	ab.add("height", _height);
	XMLNodeBuilder nb = new XMLNodeBuilder("applet-desc", ab.getAttributeList());
	if (_params != null) {
	    Enumeration keys = _params.keys();
	    while(keys.hasMoreElements()) {
		String name  = (String)keys.nextElement();
		String value = (String)_params.getProperty(name);
		nb.add(new XMLNode("param", new XMLAttribute("name", name, new XMLAttribute("value", value)), null, null));
	    }
	}
	return nb.getNode();
    }
}

