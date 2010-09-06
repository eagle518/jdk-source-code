/*
 * @(#)FieldWriterImpl.java	1.42 04/07/27
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.tools.doclets.internal.toolkit.taglets.*;
import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.javadoc.*;

import java.io.*;

/**
 * Writes field documentation in HTML format.
 *
 * @author Robert Field
 * @author Atul M Dambalkar
 * @author Jamie Ho (rewrite)
 */
public class FieldWriterImpl extends AbstractMemberWriter 
    implements FieldWriter, MemberSummaryWriter {
    
    private boolean printedSummaryHeader = false;
    
    public FieldWriterImpl(SubWriterHolderWriter writer, ClassDoc classdoc) {
        super(writer, classdoc);
    }
    
    public FieldWriterImpl(SubWriterHolderWriter writer) {
        super(writer);
    }
    
    /**
     * Write the fields summary header for the given class.
     *
     * @param classDoc the class the summary belongs to.
     */
    public void writeMemberSummaryHeader(ClassDoc classDoc) {
        printedSummaryHeader = true;
        writer.println("<!-- =========== FIELD SUMMARY =========== -->"); 
        writer.println();
        writer.printSummaryHeader(this, classDoc);
    }
    
    /**
     * Write the fields summary footer for the given class.
     *
     * @param classDoc the class the summary belongs to.
     */
    public void writeMemberSummaryFooter(ClassDoc classDoc) {
        writer.tableEnd();
        writer.space();
    }
    
    /**
     * Write the inherited fields summary header for the given class.
     *
     * @param classDoc the class the summary belongs to.
     */
    public void writeInheritedMemberSummaryHeader(ClassDoc classDoc) {
        if(! printedSummaryHeader){
            //We don't want inherited summary to not be under heading.            
            writeMemberSummaryHeader(classDoc);
            writeMemberSummaryFooter(classDoc);            
            printedSummaryHeader = true;            
        }
        writer.printInheritedSummaryHeader(this, classDoc);
    }
    
    /**
     * {@inheritDoc}
     */
    public void writeInheritedMemberSummary(ClassDoc classDoc, 
        ProgramElementDoc field, boolean isFirst, boolean isLast) {
        writer.printInheritedSummaryMember(this, classDoc, field, isFirst);
    }
    
    /**
     * Write the inherited fields summary footer for the given class.
     *
     * @param classDoc the class the summary belongs to.
     */
    public void writeInheritedMemberSummaryFooter(ClassDoc classDoc) {
        writer.printInheritedSummaryFooter(this, classDoc);
    }
    
    /**
     * Write the header for the field documentation.
     *
     * @param classDoc the class that the fields belong to.
     */
    public void writeHeader(ClassDoc classDoc, String header) {
        writer.println();
        writer.println("<!-- ============ FIELD DETAIL =========== -->"); 
        writer.println();
        writer.anchor("field_detail");
        writer.printTableHeadingBackground(header);
        writer.println();
    }
    
    /**
     * Write the field header for the given field.
     *
     * @param field the field being documented.
     * @param isFirst the flag to indicate whether or not the field is the 
     *        first to be documented.
     */
    public void writeFieldHeader(FieldDoc field, boolean isFirst) {
        if (! isFirst) {
            writer.printMemberHeader();
            writer.println("");
        }
        writer.anchor(field.name());
        writer.h3();
        writer.print(field.name());
        writer.h3End();
    }
    
    /**
     * Write the signature for the given field.
     *
     * @param field the field being documented.
     */
    public void writeSignature(FieldDoc field) {
        writer.pre();
        writer.writeAnnotationInfo(field);
        printModifiers(field);
        writer.printLink(new LinkInfoImpl(LinkInfoImpl.CONTEXT_MEMBER, 
            field.type()));
        print(' ');
        if (configuration().linksource) {
            writer.printSrcLink(field, field.name());
        } else {
            bold(field.name());
        }
        writer.preEnd();
        writer.dl();
    }
    
    /**
     * Write the deprecated output for the given field.
     *
     * @param field the field being documented.
     */
    public void writeDeprecated(FieldDoc field) {
        print(((TagletOutputImpl)
            (new DeprecatedTaglet()).getTagletOutput(field, 
            writer.getTagletWriterInstance(false))).toString());
    }
    
    /**
     * Write the comments for the given field.
     *
     * @param field the field being documented.
     */
    public void writeComments(FieldDoc field) {
        ClassDoc holder = field.containingClass();
        if (field.inlineTags().length > 0) {
            if (holder.equals(classdoc) || 
                (! (holder.isPublic() || Util.isLinkable(holder, configuration())))) {
                writer.dd();
                writer.printInlineComment(field);
            } else {
                String classlink = writer.codeText(
                    writer.getDocLink(LinkInfoImpl.CONTEXT_FIELD_DOC_COPY,
                        holder, field, 
                        holder.isIncluded() ? 
                            holder.typeName() : holder.qualifiedTypeName(), 
                        false));
                writer.dd();
                writer.bold(configuration().getText(holder.isClass()?
                   "doclet.Description_From_Class" : 
                    "doclet.Description_From_Interface", classlink));
                writer.ddEnd();
                writer.dd();
                writer.printInlineComment(field);
            }
        }
    }
    
    /**
     * Write the tag output for the given field.
     *
     * @param field the field being documented.
     */
    public void writeTags(FieldDoc field) {
        writer.printTags(field);
    }
    
    /**
     * Write the field footer.
     */
    public void writeFieldFooter() {
        writer.dlEnd();
    }
    
    /**
     * Write the footer for the field documentation.
     *
     * @param classDoc the class that the fields belong to.
     */
    public void writeFooter(ClassDoc classDoc) {
        //No footer to write for field documentation
    }
    
    /**
     * Close the writer.
     */
    public void close() throws IOException {
        writer.close();
    }
    
    public int getMemberKind() {
        return VisibleMemberMap.FIELDS;
    }
    
    public void printSummaryLabel(ClassDoc cd) {
        writer.boldText("doclet.Field_Summary");
    }
    
    public void printSummaryAnchor(ClassDoc cd) {
        writer.anchor("field_summary");
    }
    
    public void printInheritedSummaryAnchor(ClassDoc cd) {
        writer.anchor("fields_inherited_from_class_" + configuration().getClassName(cd));
    }
    
    public void printInheritedSummaryLabel(ClassDoc cd) {
        String classlink = writer.getPreQualifiedClassLink(
            LinkInfoImpl.CONTEXT_MEMBER, cd, false);
        writer.bold();
        String key = cd.isClass()?
            "doclet.Fields_Inherited_From_Class" :
            "doclet.Fields_Inherited_From_Interface";
        writer.printText(key, classlink);
        writer.boldEnd();
    }
    
    protected void writeSummaryLink(int context, ClassDoc cd, ProgramElementDoc member) {
        writer.bold();
        writer.printDocLink(context, cd , (MemberDoc) member, member.name(), false);
        writer.boldEnd();
    }
    
    protected void writeInheritedSummaryLink(ClassDoc cd,
            ProgramElementDoc member) {
        writer.printDocLink(LinkInfoImpl.CONTEXT_MEMBER, cd, (MemberDoc)member, 
            member.name(), false);
    }
    
    protected void printSummaryType(ProgramElementDoc member) {
        FieldDoc field = (FieldDoc)member;
        printModifierAndType(field, field.type());
    }
    
    protected void writeDeprecatedLink(ProgramElementDoc member) {
        writer.printDocLink(LinkInfoImpl.CONTEXT_MEMBER, 
            (MemberDoc) member, ((FieldDoc)member).qualifiedName(), false);
    }
    
    protected void printNavSummaryLink(ClassDoc cd, boolean link) {
        if (link) {
            writer.printHyperLink("", (cd == null)?
                        "field_summary":
                        "fields_inherited_from_class_" +
                        configuration().getClassName(cd),
                    configuration().getText("doclet.navField"));        
        } else {
            writer.printText("doclet.navField");
        }
    }
    
    protected void printNavDetailLink(boolean link) {
        if (link) {
            writer.printHyperLink("", "field_detail",
                configuration().getText("doclet.navField"));
        } else {
            writer.printText("doclet.navField");
        }
    }
}


