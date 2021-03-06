/*
 * @(#)WebRowSetXmlReader.java	1.5 04/05/29  
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved. 
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.rowset.internal;

import java.sql.*;
import javax.sql.*;
import java.io.*;

import org.xml.sax.*;
import org.xml.sax.helpers.*;
import javax.xml.parsers.*;

import com.sun.rowset.*;
import javax.sql.rowset.*;
import javax.sql.rowset.spi.*;
import com.sun.rowset.providers.*;

/**
 * An implementation of the <code>XmlReader</code> interface, which
 * reads and parses an XML formatted <code>WebRowSet</code> object.
 * This implementation uses an <code>org.xml.sax.Parser</code> object
 * as its parser.
 */
public class WebRowSetXmlReader implements XmlReader, Serializable {

    /**
     * Parses the given <code>WebRowSet</code> object, getting its input from
     * the given <code>java.io.Reader</code> object.  The parser will send
     * notifications of parse events to the rowset's 
     * <code>XmlReaderDocHandler</code>, which will build the rowset as
     * an XML document.
     * <P>
     * This method is called internally by the method
     * <code>WebRowSet.readXml</code>.
     * <P>
     * If a parsing error occurs, the exception thrown will include
     * information for locating the error in the original XML document.
     * 
     * @param caller the <code>WebRowSet</code> object to be parsed, whose
     *        <code>xmlReader</code> field must contain a reference to 
     *        this <code>XmlReader</code> object
     * @param reader the <code>java.io.Reader</code> object from which
     *        the parser will get its input
     * @exception SQLException if a database access error occurs or
     *            this <code>WebRowSetXmlReader</code> object is not the 
     *            reader for the given rowset
     * @see XmlReaderContentHandler
     */
    public void readXML(WebRowSet caller, java.io.Reader reader) throws SQLException {
        try {
            // Crimson Parser(as in J2SE 1.4.1 is NOT able to handle 
            // Reader(s)(FileReader).
            //
            // But getting the file as a Stream works fine. So we are going to take 
            // the reader but send it as a InputStream to the parser. Note that this
            // functionality needs to work against any parser
            // Crimson(J2SE 1.4.x) / Xerces(J2SE 1.5.x).
	    InputSource is = new InputSource(reader);
	    DefaultHandler dh = new XmlErrorHandler();
            XmlReaderContentHandler hndr = new XmlReaderContentHandler((RowSet)caller);
	    
	    SAXParserFactory factory = SAXParserFactory.newInstance();
	    factory.setNamespaceAware(true);
	    factory.setValidating(true);
	    
	    SAXParser parser = factory.newSAXParser() ;
	    
	    parser.setProperty(
			       "http://java.sun.com/xml/jaxp/properties/schemaLanguage", "http://www.w3.org/2001/XMLSchema");
                     
            XMLReader reader1 = parser.getXMLReader() ;
            reader1.setEntityResolver(new XmlResolver()); 
            reader1.setContentHandler(hndr);
         
            reader1.setErrorHandler(dh);            

            reader1.parse(is);

        } catch (SAXParseException err) {
            System.out.println ("** Parsing error" 
                                + ", line " + err.getLineNumber ()
                                + ", uri " + err.getSystemId ());
            System.out.println("   " + err.getMessage ());
            err.printStackTrace();
	    throw new SQLException(err.getMessage());
            
        } catch (SAXException e) {
            Exception   x = e;
            if (e.getException () != null)
                x = e.getException();
            x.printStackTrace ();
	    throw new SQLException(x.getMessage());
            
        } 
        
        // Will be here if trying to write beyond the RowSet limits
        
         catch (ArrayIndexOutOfBoundsException aie) {
	      throw new SQLException("End of Rowset reached. Invalid cursor position");	
        }     
        catch (Throwable e) {
	    throw new SQLException("readXML: " + e.getMessage());
	}            
        
    } 


    /**
     * Parses the given <code>WebRowSet</code> object, getting its input from
     * the given <code>java.io.InputStream</code> object.  The parser will send
     * notifications of parse events to the rowset's 
     * <code>XmlReaderDocHandler</code>, which will build the rowset as
     * an XML document.
     * <P>
     * Using streams is a much faster way than using <code>java.io.Reader</code>
     * <P>
     * This method is called internally by the method
     * <code>WebRowSet.readXml</code>.
     * <P>
     * If a parsing error occurs, the exception thrown will include
     * information for locating the error in the original XML document.
     * 
     * @param caller the <code>WebRowSet</code> object to be parsed, whose
     *        <code>xmlReader</code> field must contain a reference to 
     *        this <code>XmlReader</code> object
     * @param iStream the <code>java.io.InputStream</code> object from which
     *        the parser will get its input
     * @throws SQLException if a database access error occurs or
     *            this <code>WebRowSetXmlReader</code> object is not the 
     *            reader for the given rowset
     * @see XmlReaderContentHandler
     */
    public void readXML(WebRowSet caller, java.io.InputStream iStream) throws SQLException {
        try {
	    InputSource is = new InputSource(iStream);
	    DefaultHandler dh = new XmlErrorHandler();
	    
	    XmlReaderContentHandler hndr = new XmlReaderContentHandler((RowSet)caller);
	    SAXParserFactory factory = SAXParserFactory.newInstance();
	    factory.setNamespaceAware(true);
	    factory.setValidating(true);
	    
	    SAXParser parser = factory.newSAXParser() ;
	    
	    parser.setProperty("http://java.sun.com/xml/jaxp/properties/schemaLanguage", 
                     "http://www.w3.org/2001/XMLSchema");
                     
            XMLReader reader1 = parser.getXMLReader() ;
            reader1.setEntityResolver(new XmlResolver()); 
            reader1.setContentHandler(hndr);
         
            reader1.setErrorHandler(dh);            

            reader1.parse(is);

        } catch (SAXParseException err) {
            System.out.println ("** Parsing error" 
                                + ", line " + err.getLineNumber ()
                                + ", uri " + err.getSystemId ());
            System.out.println("   " + err.getMessage ());
            err.printStackTrace();
	    throw new SQLException(err.getMessage());
            
        } catch (SAXException e) {
            Exception   x = e;
            if (e.getException () != null)
                x = e.getException();
            x.printStackTrace ();
	    throw new SQLException(x.getMessage());
            
        } 
        
        // Will be here if trying to write beyond the RowSet limits
        
         catch (ArrayIndexOutOfBoundsException aie) {
	      throw new SQLException("End of Rowset reached. Invalid cursor position");	
        }     
        
        catch (Throwable e) {
	    throw new SQLException("readXML: " + e.getMessage());
	}
    } 

    /**
     * For code coverage purposes only right now
     *
     */
    
    public void readData(RowSetInternal caller) {
    }
								        
}
