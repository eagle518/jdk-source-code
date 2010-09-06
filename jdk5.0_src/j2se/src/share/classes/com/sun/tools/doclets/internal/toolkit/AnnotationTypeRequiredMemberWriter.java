/*
 * @(#)AnnotationTypeRequiredMemberWriter.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit;

import java.io.*;
import com.sun.javadoc.*;

/**
 * The interface for writing annotation type required member output.
 * 
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */

public interface AnnotationTypeRequiredMemberWriter {
    
    /**
     * Write the header for the member documentation.
     *
     * @param classDoc the annotation type that the members belong to.
     * @param header the header to write.
     */
    public void writeHeader(ClassDoc classDoc, String header);
    
    /**
     * Write the member header for the given member.
     *
     * @param member the member being documented.
     * @param isFirst the flag to indicate whether or not the member is 
     *                the first to be documented.
     */
    public void writeMemberHeader(MemberDoc member, boolean isFirst);
    
    /**
     * Write the signature for the given member.
     *
     * @param member the member being documented.
     */
    public void writeSignature(MemberDoc member);
    
    /**
     * Write the deprecated output for the given member.
     *
     * @param member the member being documented.
     */
    public void writeDeprecated(MemberDoc member);
    
    /**
     * Write the comments for the given member.
     *
     * @param member the member being documented.
     */
    public void writeComments(MemberDoc member);
    
    /**
     * Write the tag output for the given member.
     *
     * @param member the member being documented.
     */
    public void writeTags(MemberDoc member);
    
    /**
     * Write the member footer.
     */
    public void writeMemberFooter();
    
    /**
     * Write the footer for the member documentation.
     *
     * @param classDoc the class that the member belong to.
     */
    public void writeFooter(ClassDoc classDoc);
    
    /**
     * Close the writer.
     */
    public void close() throws IOException;
}

