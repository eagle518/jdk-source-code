/*
 * @(#)DOMServiceProvider.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom;

import java.applet.Applet;
import com.sun.java.browser.dom.*;
import netscape.javascript.JSException;
import netscape.javascript.JSObject;

import org.w3c.dom.*;
import org.w3c.dom.css.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.exception.*;

public class DOMServiceProvider extends com.sun.java.browser.dom.DOMServiceProvider
{
    /**
     * An empty constructor is provided. Implementations should 
     * provide a protected constructor so that the DOMService
     * can instantiate instances of the implementation class. 
     * Application programmers should not be able to directly
     * construct implementation subclasses of this abstract subclass.
     * The only way an application should be able to obtain a 
     * reference to a DOMServiceProvider implementation
     * instance is by using the appropriate methods of the
     * DOMService.
     */
    public DOMServiceProvider() {
    }

    /**
     * Returns true if the DOMService can determine the association
     * between the obj and the underlying DOM in the browser.
     */
    public boolean canHandle(Object obj) {
	// We only handle applet
	if (obj != null && obj instanceof java.applet.Applet)
	    return true;
	else
	    return false;
    }

    /**
     * Returns the Document object of the DOM.
     */
    public org.w3c.dom.Document getDocument(Object obj) throws DOMUnsupportedException {
	try {
	    if (canHandle(obj)) {
		JSObject win = (JSObject) JSObject.getWindow((Applet)obj);

		if (win == null)
		    throw new JSException("Unable to obtain Window object");

		JSObject doc = (JSObject) win.getMember("document");

		if (doc == null)
		    throw new JSException("Unable to obtain Document object");

		return new sun.plugin.dom.html.HTMLDocument(new DOMObject(doc), null);
	    }
	} catch (JSException e1) {
	    e1.printStackTrace();
	} catch (Exception e2) {
	    e2.printStackTrace();
	}

	throw new DOMUnsupportedException();
    }

    /**
     * Returns the DOMImplemenation object of the DOM.
     */
    public org.w3c.dom.DOMImplementation getDOMImplementation() {
        // This is a broken API -- it should take an Object as argument --
        // be backward compatible and return the same answers as before
        return new org.w3c.dom.DOMImplementation() {
            public boolean hasFeature(String feature, 
                                      String version) {
                // Check parameters
                if (feature == null)
                    return false;
                
                if (version == null)
                    version = "2.0";
                
                if (version.equals("2.0") &&
                    (feature.equalsIgnoreCase("dom")
                     || feature.equalsIgnoreCase("xml")
                     || feature.equalsIgnoreCase("html")
                     || feature.equalsIgnoreCase("stylesheets")
                     || feature.equalsIgnoreCase("views")
                     || feature.equalsIgnoreCase("css")))
                    return true;
                
                return false;		   	    
            }

            public DocumentType createDocumentType(String qualifiedName, 
                                                   String publicId, 
                                                   String systemId)
                throws DOMException {
                throw new PluginNotSupportedException("DOMImplementation.createDocumentType() is not supported");
            }

            public Document createDocument(String namespaceURI, 
                                           String qualifiedName, 
                                           DocumentType doctype)
                throws DOMException {
                throw new PluginNotSupportedException("DOMImplementation.createDocument() is not supported");
            }

            public HTMLDocument createHTMLDocument(String title) {
                throw new PluginNotSupportedException("HTMLDOMImplementation.createHTMLDocument() is not supported");
            }

            public CSSStyleSheet createCSSStyleSheet(String title, 
                                                     String media)
                throws DOMException {
                throw new PluginNotSupportedException("DOMImplementationCSS.createCSSStyleSheet() is not supported");
            }

            public Object getFeature(String feature, String version) {
                throw new PluginNotSupportedException("DOMImplementation.getFeature() is not supported.");
            }
        };
    }
}            
