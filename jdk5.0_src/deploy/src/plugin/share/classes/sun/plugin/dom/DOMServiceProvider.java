/*
 * @(#)DOMServiceProvider.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom;

import java.applet.Applet;
import com.sun.java.browser.dom.*;
import netscape.javascript.JSException;
import netscape.javascript.JSObject;

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
	return new sun.plugin.dom.DOMImplementation();
    }
}            
