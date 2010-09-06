/*
 * @(#)AnnotationTypeOptionalMemberWriterImpl.java	1.6 04/04/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.javadoc.*;

import java.io.*;

/**
 * Writes annotation type optional member documentation in HTML format.
 *
 * @author Jamie Ho
 */
public class AnnotationTypeOptionalMemberWriterImpl extends 
        AnnotationTypeRequiredMemberWriterImpl 
    implements AnnotationTypeOptionalMemberWriter, MemberSummaryWriter {
    
    /**
     * Construct a new AnnotationTypeOptionalMemberWriterImpl.
     * 
     * @param writer         the writer that will write the output.
     * @param annotationType the AnnotationType that holds this member.
     */   
    public AnnotationTypeOptionalMemberWriterImpl(SubWriterHolderWriter writer, 
        AnnotationTypeDoc annotationType) {
        super(writer, annotationType);
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeMemberSummaryHeader(ClassDoc classDoc) {
        writer.println("<!-- =========== ANNOTATION TYPE OPTIONAL MEMBER SUMMARY =========== -->"); 
        writer.println();
        writer.printSummaryHeader(this, classDoc);
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeDefaultValueInfo(MemberDoc member) {
        writer.dl();
        writer.dt();
        writer.bold(ConfigurationImpl.getInstance().
            getText("doclet.Default"));
        writer.dd();
        writer.print(((AnnotationTypeElementDoc) member).defaultValue());
        writer.ddEnd();
        writer.dlEnd();
    }
    
    /**
     * {@inheritDoc}
     */
    public void close() throws IOException {
        writer.close();
    }
    
    /**
     * {@inheritDoc}
     */
    public void printSummaryLabel(ClassDoc cd) {
        writer.boldText("doclet.Annotation_Type_Optional_Member_Summary");
    }
    
    /**
     * {@inheritDoc}
     */
    public void printSummaryAnchor(ClassDoc cd) {
        writer.anchor("annotation_type_optional_element_summary");
    }
    
    /**
     * {@inheritDoc}
     */
    protected void printNavSummaryLink(ClassDoc cd, boolean link) {
        if (link) {
            writer.printHyperLink("", "annotation_type_optional_element_summary",
                    configuration().getText("doclet.navAnnotationTypeOptionalMember"));        
        } else {
            writer.printText("doclet.navAnnotationTypeOptionalMember");
        }
    }
}


