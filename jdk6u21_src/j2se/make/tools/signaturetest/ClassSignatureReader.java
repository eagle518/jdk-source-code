/*
 * @(#)ClassSignatureReader.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.Vector;

/** This is class for reading signature or API signature file **/
public class ClassSignatureReader implements SignatureConstants {
    /** reads line from signature file **/
    private BufferedReader in;
    /** the last line which are read from signature file **/
    private String currentLine;
    /** the lines of the current class **/
    private Vector definitions;
    /** the converter which converts lines **/
    private DefinitionFormat converter;
    /** the java version of the signature file **/
    String javaVersion = " N/A";
    /** determines if the throws clause is required to be tracked.**/
    boolean isThrowsTracked = true;

    /** Creates new ClassSignatureReader for given URL
     * @param fileURL given URL which contains signature file **/
    public ClassSignatureReader(URL fileURL) throws IOException {
	in = new BufferedReader(new InputStreamReader(fileURL.openStream()));
	definitions = new Vector();
	currentLine = null;
	//read the first class name
	while (((currentLine = in.readLine()) != null) && 
	       (!currentLine.startsWith(CLASS))) {
            if (currentLine.startsWith("#Version"))
                javaVersion = currentLine.substring("#Version ".length());
            if (currentLine.startsWith("#Throws clause not tracked."))
                isThrowsTracked = false;
	}
    }

    /** set the definition converter
     *  @param converter given DefinitionFormat. **/
    public void setDefinitionConverter(DefinitionFormat converter) {
        this.converter = converter;
    }

    /** reads definition of the class from signature file and returns
     *  TableOfClass. This TableOfClass contains definitions of the class
     *  members with the short name and can used for SignatureTest only. **/
    public TableOfClass nextClass() throws IOException {
	if ((in == null) || (currentLine == null))
	    return null;
	TableOfClass retClass = new TableOfClass(currentLine, converter);	
	definitions = new Vector();
	while (((currentLine = in.readLine()) != null) && 
	       (!currentLine.startsWith(CLASS))) {
	    definitions.addElement(currentLine);
	}
	retClass.createMembers(definitions.elements());
	return retClass;
    }

    /** reads definition of the class from signature file and returns
     *  TableOfClass for APIChangesTest with definitions of the class members.**/
    public TableOfClass nextAPIClass() throws IOException {
	if ((in == null) || (currentLine == null))
	    return null;
	TableOfClass retClass = new TableOfClass(currentLine, converter);	
	definitions = new Vector();
	while (((currentLine = in.readLine()) != null) && 
	       (!currentLine.startsWith(CLASS))) {
	    definitions.addElement(currentLine);
	}
	retClass.createMembers(definitions.elements());
	return retClass;
    }    
}
	    
