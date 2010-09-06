/*
 * %W% %E% 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.rowset.internal;

import org.xml.sax.*;
import org.xml.sax.helpers.DefaultHandler;

import com.sun.rowset.*;
import javax.sql.rowset.*;


/**
 * An implementation of the <code>DefaultHandler</code> interface, which
 * handles all the errors, fatalerrors and warnings while reading the xml file.
 * This is the ErrorHandler which helps <code>WebRowSetXmlReader</code>
 * to handle any errors while reading the xml data.
 */


public class XmlErrorHandler extends DefaultHandler {
       public int errorCounter = 0;

       public void error(SAXParseException e) throws SAXException {
           errorCounter++;
       
       }
       
       public void fatalError(SAXParseException e) throws SAXException {
	   errorCounter++;
       
       }
       
       public void warning(SAXParseException exception) throws SAXException {
       
       }	
}  
    