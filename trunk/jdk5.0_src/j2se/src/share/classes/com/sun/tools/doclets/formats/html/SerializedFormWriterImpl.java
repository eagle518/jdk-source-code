/*
 * @(#)SerializedFormWriterImpl.java	1.30 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.javadoc.*;
import java.io.*;

/**
 * Generate the Serialized Form Information Page.
 *
 * @author Atul M Dambalkar
 */
public class SerializedFormWriterImpl extends SubWriterHolderWriter
    implements com.sun.tools.doclets.internal.toolkit.SerializedFormWriter {

    private static final String FILE_NAME = "serialized-form.html";
    
    /**
     * @throws IOException
     * @throws DocletAbortException
     */
    public SerializedFormWriterImpl() throws IOException {
        super(ConfigurationImpl.getInstance(), FILE_NAME);
    }
        
    /**
     * Writes the given header.
     *
     * @param header the header to write.
     */
    public void writeHeader(String header) {
        printHtmlHeader(header, null, true);
        navLinks(true);
        hr();
        center();
        h1();
        print(header);
        h1End();
        centerEnd();
    }
        
    /**
     * Write the given package header.
     *
     * @param packageName the package header to write.
     */
    public void writePackageHeader(String packageName) {
        hr(4, "noshade");
        tableHeader();
        thAlign("center");
        font("+2");
        boldText("doclet.Package");
        print(' ');
        bold(packageName);
        tableFooter();
    }
        
    /**
     * Write the serial UID info.
     *
     * @param header the header that will show up before the UID.
     * @param serialUID the serial UID to print.
     */
    public void writeSerialUIDInfo(String header, String serialUID) {
        bold(header + "&nbsp;");
        println(serialUID);
        p();
    }
        
    /**
     * Write the footer.
     */
    public void writeFooter() {
        p();
        hr();
        navLinks(false);
        printBottom();
        printBodyHtmlEnd();
    }
    
    
    /**
     * Write the serializable class heading.
     *
     * @param classDoc the class being processed.
     */
    public void writeClassHeader(ClassDoc classDoc) {
        String classLink = (classDoc.isPublic() || classDoc.isProtected())?
            getLink(new LinkInfoImpl(classDoc, 
                configuration.getClassName(classDoc))):
            classDoc.qualifiedName();
        p();
        anchor(classDoc.qualifiedName());
        String superClassLink = 
            classDoc.superclassType() != null ? 
                getLink(new LinkInfoImpl(LinkInfoImpl.CONTEXT_SERIALIZED_FORM, 
                    classDoc.superclassType())) : 
                null;
            
        //Print the heading.
        String className = superClassLink == null ?
            configuration.getText(
                "doclet.Class_0_implements_serializable", classLink) :
            configuration.getText(
                "doclet.Class_0_extends_implements_serializable", classLink,
                    superClassLink);        
        tableHeader();
        thAlignColspan("left", 2);
        font("+2");
        bold(className);
        tableFooter();
        p();
    }
    
    private void tableHeader() {
        tableIndexSummary();
        trBgcolorStyle("#CCCCFF", "TableSubHeadingColor");
    }
    
    private void tableFooter() {
        fontEnd();
        thEnd(); trEnd(); tableEnd();
    }
        
    /**
     * Return an instance of a SerialFieldWriter.
     *
     * @return an instance of a SerialFieldWriter.
     */
    public SerialFieldWriter getSerialFieldWriter(ClassDoc classDoc) {
        return new HtmlSerialFieldWriter(this, classDoc);
    }
    
    /**
     * Return an instance of a SerialMethodWriter.
     *
     * @return an instance of a SerialMethodWriter.
     */
    public SerialMethodWriter getSerialMethodWriter(ClassDoc classDoc) {
        return new HtmlSerialMethodWriter(this, classDoc);
    }
}
