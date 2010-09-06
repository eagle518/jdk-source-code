/*
 * @(#)AnnotationTypeBuilder.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.builders;

import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.javadoc.*;
import java.io.*;
import java.util.*;
import java.lang.reflect.*;

/**
 * Builds the summary for a given annotation type.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */
public class AnnotationTypeBuilder extends AbstractBuilder {
    
    /**
     * The root element of the annotation type XML is {@value}.
     */
    public static final String ROOT = "AnnotationTypeDoc";
    
    /**
     * The annotation type being documented.
     */
    private AnnotationTypeDoc annotationTypeDoc;
    
    /**
     * The doclet specific writer.
     */
    private AnnotationTypeWriter writer;
    
    /**
     * Construct a new ClassBuilder.
     *
     * @param configuration the current configuration of the
     *                      doclet.
     */
    private AnnotationTypeBuilder(Configuration configuration) {
        super(configuration);
    }
    
    /**
     * Construct a new ClassBuilder.
     *
     * @param configuration     the current configuration of the doclet.
     * @param annotationTypeDoc the class being documented.
     * @param writer            the doclet specific writer.
     */
    public static AnnotationTypeBuilder getInstance(Configuration configuration, 
        AnnotationTypeDoc annotationTypeDoc, AnnotationTypeWriter writer) 
    throws Exception {
        AnnotationTypeBuilder builder = new AnnotationTypeBuilder(configuration);
        builder.configuration = configuration;
        builder.annotationTypeDoc = annotationTypeDoc;
        builder.writer = writer;
        if(containingPackagesSeen == null) {
            containingPackagesSeen = new HashSet();
        }
        return builder;
    }
    
    /**
     * {@inheritDoc}
     */
    public void invokeMethod(String methodName, Class[] paramClasses, 
            Object[] params) 
    throws Exception {
        if (DEBUG) {
            configuration.root.printError("DEBUG: " + this.getClass().getName() 
                + "." + methodName);
        }
        Method method = this.getClass().getMethod(methodName, paramClasses);
        method.invoke(this, params);
    }
    
    /**
     * {@inheritDoc}
     */
    public void build() throws IOException {
        build(LayoutParser.getInstance(configuration).parseXML(ROOT));
    }
    
    /**
     * {@inheritDoc}
     */
    public String getName() {
        return ROOT;
    }
     
     /**
      * Handles the &lt;AnnotationTypeDoc> tag.
      *
      * @param elements the XML elements that specify how to document a class.
      */
     public void buildAnnotationTypeDoc(List elements) throws Exception {
        build(elements);
        writer.close();
        copyDocFiles(); 
     }
    
    
     /**
      * Copy the doc files for the current ClassDoc if necessary.
      */
     private void copyDocFiles() {
        PackageDoc containingPackage = annotationTypeDoc.containingPackage();
        if((configuration.packages == null ||
                Arrays.binarySearch(configuration.packages,
                                    containingPackage) < 0) &&
           ! containingPackagesSeen.contains(containingPackage.name())){
            //Only copy doc files dir if the containing package is not
            //documented AND if we have not documented a class from the same
            //package already. Otherwise, we are making duplicate copies.
            Util.copyDocFiles(configuration,
                Util.getPackageSourcePath(configuration,
                    annotationTypeDoc.containingPackage()) +
                DirectoryManager.getDirectoryPath(
                    annotationTypeDoc.containingPackage())
                    + File.separator, DocletConstants.DOC_FILES_DIR_NAME, true);
            containingPackagesSeen.add(containingPackage.name());
        } 
     }
    
    /**
     * Build the header of the page.
     */
    public void buildAnnotationTypeHeader() {        		
        writer.writeHeader(configuration.getText("doclet.AnnotationType") + 
            " " + annotationTypeDoc.name());
    }
    
    /**
     * If this class is deprecated, print the appropriate information.
     */
    public void buildDeprecationInfo () {
        writer.writeAnnotationTypeDeprecationInfo();
    }
    
    /**
     * Build the signature of the current annotation type.
     */
    public void buildAnnotationTypeSignature() {
        StringBuffer modifiers = new StringBuffer(
            annotationTypeDoc.modifiers() + " ");
        writer.writeAnnotationTypeSignature(
            Util.replaceText(
                modifiers.toString(), "interface", "@interface"));
    }
    
    /**
     * Build the class description.
     */
    public void buildAnnotationTypeDescription() {
       writer.writeAnnotationTypeDescription();
    }
    
    /**
     * Build the tag information for the current class.
     */
    public void buildAnnotationTypeTagInfo() {
       writer.writeAnnotationTypeTagInfo();
    }
    
    /**
     * Build the contents of the page.
     *
     * @param elements the XML elements that specify how a member summary is
     *                 documented.
     */
    public void buildMemberSummary(List elements) throws Exception {
        configuration.getBuilderFactory().
            getMemberSummaryBuilder(writer).build(elements);
        writer.completeMemberSummaryBuild();
    }
    
    /**
     * Build the annotation type optional member documentation.
     *
     * @param elements the XML elements that specify how a annotation type
     *                 members are documented.
     */
    public void buildAnnotationTypeOptionalMemberDetails(List elements) 
    throws Exception {
        configuration.getBuilderFactory().
            getAnnotationTypeOptionalMemberBuilder(writer).build(elements);
    }
    
    /**
     * Build the annotation type required member documentation.
     *
     * @param elements the XML elements that specify how a annotation type
     *                 members are documented.
     */
    public void buildAnnotationTypeRequiredMemberDetails(List elements) 
    throws Exception {
        configuration.getBuilderFactory().
            getAnnotationTypeRequiredMemberBuilder(writer).build(elements);
    }
    
    
    /**
     * Build the footer of the page.
     */
    public void buildAnnotationTypeFooter() {
        writer.writeFooter();
    }
}
