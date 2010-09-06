/*
 * @(#)MemberSummaryWriter.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit;

import java.io.*;
import com.sun.javadoc.*;

/**
 * The interface for writing member summary output.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */

public interface MemberSummaryWriter {
    
    /**
     * Write the member summary header for the given class.
     *
     * @param classDoc the class the summary belongs to.
     */
    public void writeMemberSummaryHeader(ClassDoc classDoc);
    
    /**
     * Write the member summary for the given class and member.
     *
     * @param classDoc the class the summary belongs to.
     * @param member the member that I am summarizing.
     * @param firstSentenceTags the tags for the sentence being documented.
     * @param isFirst true if this is the first member in the list.
     * @param isLast true if this the last member being documented.
     */
    public void writeMemberSummary(ClassDoc classDoc, ProgramElementDoc member, 
        Tag[] firstSentenceTags, boolean isFirst, boolean isLast);
    
    /**
     * Write the member summary footer for the given class.
     *
     * @param classDoc the class the summary belongs to.
     */
    public void writeMemberSummaryFooter(ClassDoc classDoc);
    
    /**
     * Write the inherited member summary header for the given class.
     *
     * @param classDoc the class the summary belongs to.
     */
    public void writeInheritedMemberSummaryHeader(ClassDoc classDoc);
    
    /**
     * Write the inherited member summary for the given class and member.
     *
     * @param classDoc the class the inherited member belongs to.
     * @param member   the inherited member that I am summarizing.
     * @param isFirst  true if this is the first member in the list.
     * @param isLast   true if this is the last member in the list.
     */
    public void writeInheritedMemberSummary(ClassDoc classDoc, 
        ProgramElementDoc member, boolean isFirst, boolean isLast);
    
    /**
     * Write the inherited member summary footer for the given class.
     *
     * @param classDoc the class the summary belongs to.
     */
    public void writeInheritedMemberSummaryFooter(ClassDoc classDoc);
    
    /**
     * Close the writer.
     */
    public void close() throws IOException;
}

