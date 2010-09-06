/*
 * @(#)AnnotationTypeRequiredMemberWriterImpl.java	1.9 04/05/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.tools.doclets.internal.toolkit.taglets.*;
import com.sun.javadoc.*;

import java.io.*;

/**
 * Writes annotation type required member documentation in HTML format.
 *
 * @author Jamie Ho
 */
public class AnnotationTypeRequiredMemberWriterImpl extends AbstractMemberWriter 
    implements AnnotationTypeRequiredMemberWriter, MemberSummaryWriter {
    
    /**
     * Construct a new AnnotationTypeRequiredMemberWriterImpl.
     * 
     * @param writer         the writer that will write the output.
     * @param annotationType the AnnotationType that holds this member.
     */    
    public AnnotationTypeRequiredMemberWriterImpl(SubWriterHolderWriter writer, 
        AnnotationTypeDoc annotationType) {
        super(writer, annotationType);
    }
    
    /**
     * Write the annotation type member summary header for the given class.
     *
     * @param classDoc the class the summary belongs to.
     */
    public void writeMemberSummaryHeader(ClassDoc classDoc) {
        writer.println("<!-- =========== ANNOTATION TYPE REQUIRED MEMBER SUMMARY =========== -->"); 
        writer.println();
        writer.printSummaryHeader(this, classDoc);
    }
    
    /**
     * Write the annotation type member summary footer for the given class.
     *
     * @param classDoc the class the summary belongs to.
     */
    public void writeMemberSummaryFooter(ClassDoc classDoc) {
        writer.printSummaryFooter(this, classDoc);
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeInheritedMemberSummaryHeader(ClassDoc classDoc) {
        //Not appliable.
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeInheritedMemberSummary(ClassDoc classDoc, 
        ProgramElementDoc member, boolean isFirst, boolean isLast) {
        //Not appliable.
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeInheritedMemberSummaryFooter(ClassDoc classDoc) {
        //Not appliable.
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeHeader(ClassDoc classDoc, String header) {
        writer.println();
        writer.println("<!-- ============ ANNOTATION TYPE MEMBER DETAIL =========== -->"); 
        writer.println();
        writer.anchor("annotation_type_element_detail");
        writer.printTableHeadingBackground(header);
        writer.println();
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeMemberHeader(MemberDoc member, boolean isFirst) {
        if (! isFirst) {
            writer.printMemberHeader();
            writer.println("");
        }
        writer.anchor(member.name() + ((ExecutableMemberDoc) member).signature());
        writer.h3();
        writer.print(member.name());
        writer.h3End();
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeSignature(MemberDoc member) {
        writer.pre();
        writer.writeAnnotationInfo(member);
        printModifiers(member);
        writer.printLink(new LinkInfoImpl(LinkInfoImpl.CONTEXT_MEMBER, 
            getType(member)));
        print(' ');
        if (configuration().linksource) {
            writer.printSrcLink(member, member.name());
        } else {
            bold(member.name());
        }
        writer.preEnd();
        writer.dl();
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeComments(MemberDoc member) {
        if (member.inlineTags().length > 0) {
            writer.dd();
            writer.printInlineComment(member);
        }
    }
    
    /**
     * Write the tag output for the given member.
     *
     * @param member the member being documented.
     */
    public void writeTags(MemberDoc member) {
        writer.printTags(member);
    }
    
    /**
     * Write the annotation type member footer.
     */
    public void writeMemberFooter() {
        writer.dlEnd();
    }
    
    /**
     * Write the footer for the annotation type member documentation.
     *
     * @param classDoc the class that the annotation type member belong to.
     */
    public void writeFooter(ClassDoc classDoc) {
        //No footer to write for annotation type member documentation
    }
    
    /**
     * Close the writer.
     */
    public void close() throws IOException {
        writer.close();
    }
    
    /**
     * {@inheritDoc}
     */
    public void printSummaryLabel(ClassDoc cd) {
        writer.boldText("doclet.Annotation_Type_Required_Member_Summary");
    }
    
    /**
     * {@inheritDoc}
     */
    public void printSummaryAnchor(ClassDoc cd) {
        writer.anchor("annotation_type_required_element_summary");
    }
    
    /**
     * {@inheritDoc}
     */
    public void printInheritedSummaryAnchor(ClassDoc cd) {
    }   // no such
    
    /**
     * {@inheritDoc}
     */
    public void printInheritedSummaryLabel(ClassDoc cd) {
        // no such
    }
    
    /**
     * {@inheritDoc}
     */
    protected void writeSummaryLink(int context, ClassDoc cd, ProgramElementDoc member) {
        writer.bold();
        writer.printDocLink(context, (MemberDoc) member, member.name(), false);
        writer.boldEnd();
    }
    
    /**
     * {@inheritDoc}
     */
    protected void writeInheritedSummaryLink(ClassDoc cd,
            ProgramElementDoc member) {
        //Not applicable.
    }
    
    /**
     * {@inheritDoc}
     */
    protected void printSummaryType(ProgramElementDoc member) {
        MemberDoc m = (MemberDoc)member;
        printModifierAndType(m, getType(m));
    }
    
    /**
     * {@inheritDoc}
     */
    protected void writeDeprecatedLink(ProgramElementDoc member) {
        writer.printDocLink(LinkInfoImpl.CONTEXT_MEMBER, 
            (MemberDoc) member, ((MemberDoc)member).qualifiedName(), false);
    }
    
    /**
     * {@inheritDoc}
     */
    protected void printNavSummaryLink(ClassDoc cd, boolean link) {
        if (link) {
            writer.printHyperLink("", "annotation_type_required_element_summary",
                    configuration().getText("doclet.navAnnotationTypeRequiredMember"));        
        } else {
            writer.printText("doclet.navAnnotationTypeRequiredMember");
        }
    }
    
    /**
     * {@inheritDoc}
     */
    protected void printNavDetailLink(boolean link) {
        if (link) {
            writer.printHyperLink("", "annotation_type_element_detail",
                configuration().getText("doclet.navAnnotationTypeMember"));
        } else {
            writer.printText("doclet.navAnnotationTypeMember");
        }
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeDeprecated(MemberDoc member) {
        print(((TagletOutputImpl)
            (new DeprecatedTaglet()).getTagletOutput(member, 
            writer.getTagletWriterInstance(false))).toString());
    }
    
    private Type getType(MemberDoc member) {
        if (member instanceof FieldDoc) {
            return ((FieldDoc) member).type();
        } else {
            return ((MethodDoc) member).returnType();
        }
    }
}


