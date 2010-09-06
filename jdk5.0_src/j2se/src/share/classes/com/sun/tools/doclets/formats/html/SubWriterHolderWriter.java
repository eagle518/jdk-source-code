/*
 * @(#)SubWriterHolderWriter.java	1.32 04/05/25
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.javadoc.*;
import com.sun.tools.doclets.internal.toolkit.util.*;

import java.io.*;

/**
 * This abstract class exists to provide functionality needed in the
 * the formatting of member information.  Since AbstractSubWriter and its
 * subclasses control this, they would be the logical place to put this.
 * However, because each member type has its own subclass, subclassing
 * can not be used effectively to change formatting.  The concrete
 * class subclass of this class can be subclassed to change formatting.
 *
 * @see AbstractMemberWriter
 * @see ClassWriterImpl
 *
 * @author Robert Field
 * @author Atul M Dambalkar
 */
public abstract class SubWriterHolderWriter extends HtmlDocletWriter {
    
    public SubWriterHolderWriter(ConfigurationImpl configuration,
                                 String filename) throws IOException {
        super(configuration, filename);
    }


    public SubWriterHolderWriter(ConfigurationImpl configuration,
                                 String path, String filename, String relpath)
                                 throws IOException {
        super(configuration, path, filename, relpath);
    }

    public void printTypeSummaryHeader() {
        tdIndex();
        font("-1");
        code();
    }

    public void printTypeSummaryFooter() {
        codeEnd();
        fontEnd();
        tdEnd();
    }

    public void printSummaryHeader(AbstractMemberWriter mw, ClassDoc cd) {
        mw.printSummaryAnchor(cd);
        tableIndexSummary();
        tableHeaderStart("#CCCCFF");
        mw.printSummaryLabel(cd);
        tableHeaderEnd();
    }

    public void printTableHeadingBackground(String str) {
        tableIndexDetail();
        tableHeaderStart("#CCCCFF", 1);
        bold(str);
        tableHeaderEnd();
        tableEnd();
    }
 
    public void printInheritedSummaryHeader(AbstractMemberWriter mw, ClassDoc cd) {
        mw.printInheritedSummaryAnchor(cd);
        tableIndexSummary();
        tableInheritedHeaderStart("#EEEEFF");
        mw.printInheritedSummaryLabel(cd);
        tableInheritedHeaderEnd();
        trBgcolorStyle("white", "TableRowColor");
        summaryRow(0);
        code();
    }

    public void printSummaryFooter(AbstractMemberWriter mw, ClassDoc cd) {
        tableEnd();
        space();
    }

    public void printInheritedSummaryFooter(AbstractMemberWriter mw, ClassDoc cd) {
        codeEnd();
        summaryRowEnd();
        trEnd();
        tableEnd();
        space();
    }
    
    protected void printIndexComment(Doc member) {
        printIndexComment(member, member.firstSentenceTags());
    }

    protected void printIndexComment(Doc member, Tag[] firstSentenceTags) {
        Tag[] deprs = member.tags("deprecated");
        if (Util.isDeprecated((ProgramElementDoc) member)) {            
            boldText("doclet.Deprecated");
            space();
            if (deprs.length > 0) {            	
            	printInlineDeprecatedComment(member, deprs[0]);
            }
            return;
        } else {
            ClassDoc cd = ((ProgramElementDoc)member).containingClass();
            if (cd != null && Util.isDeprecated(cd)) {
                boldText("doclet.Deprecated"); space();
            }
        }
        printSummaryComment(member, firstSentenceTags);
    }

    public void printSummaryLinkType(AbstractMemberWriter mw,
                                     ProgramElementDoc member) {
        trBgcolorStyle("white", "TableRowColor");        
        mw.printSummaryType(member);
        summaryRow(0);
        code();
    }
    
    public void printSummaryLinkComment(AbstractMemberWriter mw,
                                        ProgramElementDoc member) {
        printSummaryLinkComment(mw, member, member.firstSentenceTags());
    }
    
    public void printSummaryLinkComment(AbstractMemberWriter mw,
                                        ProgramElementDoc member,
                                        Tag[] firstSentenceTags) {
        codeEnd();
        println(); 
        br();
        printNbsps();
        printIndexComment(member, firstSentenceTags);
        summaryRowEnd();
        trEnd();
    }

    public void printInheritedSummaryMember(AbstractMemberWriter mw, ClassDoc cd,
            ProgramElementDoc member, boolean isFirst) {
        if (! isFirst) {
            mw.print(", ");
        }
        mw.writeInheritedSummaryLink(cd, member);       
    }

    public void printMemberHeader() {
        hr();
    }

    public void printMemberFooter() {
    }

}




