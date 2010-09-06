/*
 * @(#)AnnotationTypeOptionalMemberBuilder.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.builders;


import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.javadoc.*;
import java.util.*;
import java.lang.reflect.*;

/**
 * Builds documentation for optional annotation type members.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */
public class AnnotationTypeOptionalMemberBuilder extends 
    AnnotationTypeRequiredMemberBuilder {

    
    /**
     * Construct a new AnnotationTypeMemberBuilder.
     *
     * @param configuration the current configuration of the
     *                      doclet.
     */
    private AnnotationTypeOptionalMemberBuilder(Configuration configuration) {
        super(configuration);
    }
    
        
    /**
     * Construct a new AnnotationTypeMemberBuilder.
     *
     * @param configuration the current configuration of the doclet.
     * @param classDoc the class whoses members are being documented.
     * @param writer the doclet specific writer.
     */
    public static AnnotationTypeOptionalMemberBuilder getInstance(
            Configuration configuration, ClassDoc classDoc,
            AnnotationTypeOptionalMemberWriter writer) {
        AnnotationTypeOptionalMemberBuilder builder = 
            new AnnotationTypeOptionalMemberBuilder(configuration);
        builder.classDoc = classDoc;
        builder.writer = writer;
        builder.visibleMemberMap = new VisibleMemberMap(classDoc, 
            VisibleMemberMap.ANNOTATION_TYPE_MEMBER_OPTIONAL, configuration.nodeprecated);
        builder.members = new ArrayList(
            builder.visibleMemberMap.getMembersFor(classDoc));
        if (configuration.getMemberComparator() != null) {
            Collections.sort(builder.members, 
                configuration.getMemberComparator());
        }
        return builder;
    }
    
    /**
     * {@inheritDoc}
     */
    public String getName() {
        return "AnnotationTypeOptionalMemberDetails";
    }
    
    /**
     * Build the member documentation.
     *
     * @param elements the XML elements that specify how to construct this 
     *                documentation.
     */
    public void buildAnnotationTypeOptionalMember(List elements) {
        if (writer == null) {
            return;
        }
        for (currentMemberIndex = 0; currentMemberIndex < members.size(); 
            currentMemberIndex++) {
            build(elements);
        }
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
     * Document the default value for this optional member.
     */
    public void buildDefaultValueInfo() {
        ((AnnotationTypeOptionalMemberWriter) writer).writeDefaultValueInfo(
            (MemberDoc) members.get(currentMemberIndex));
    }   
    
    /**
     * {@inheritDoc}
     */
    public AnnotationTypeRequiredMemberWriter getWriter() {
        return writer;
    }
}

