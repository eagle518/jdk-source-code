/*
 * @(#)XmlResolver.java	1.3 03/12/19 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.rowset.internal;

import org.xml.sax.*;

import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;

/**
 * An implementation of the <code>EntityResolver</code> interface, which
 * reads and parses an XML formatted <code>WebRowSet</code> object.
 * This is an implementation of org.xml.sax
 * 
 */
public class XmlResolver implements EntityResolver {
	
	public InputSource resolveEntity(String publicId, String systemId) {
           String schemaName = systemId.substring(systemId.lastIndexOf("/"));

	   if(systemId.startsWith("http://java.sun.com/xml/ns/jdbc")) { 
	       return new InputSource(this.getClass().getResourceAsStream(schemaName));

	   } else {
	      // use the default behaviour
	      return null;
	   }
	   
	  
	   
	
       }
}
