/*
 * @(#)HtmlSerialFieldWriter.java	1.30 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.tools.doclets.internal.toolkit.taglets.*;
import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.javadoc.*;
import java.util.*;

/**
 * Generate serialized form for serializable fields.
 * Documentation denoted by the tags <code>serial</code> and
 * <code>serialField<\code> is processed.
 *
 * @author Joe Fialli
 */
public class HtmlSerialFieldWriter extends FieldWriterImpl
    implements SerializedFormWriter.SerialFieldWriter {
    ProgramElementDoc[] members = null;
    
    private boolean printedOverallAnchor = false;
    
    private boolean printedFirstMember = false;
    
    public HtmlSerialFieldWriter(SubWriterHolderWriter writer,
                                    ClassDoc classdoc) {
        super(writer, classdoc);
    }
    
    public List members(ClassDoc cd) {
        return Util.asList(cd.serializableFields());
    }
    
    protected void printTypeLinkNoDimension(Type type) {
        ClassDoc cd = type.asClassDoc();
        //Linking to package private classes in serialized for causes
        //broken links.  Don't link to them.
        if (type.isPrimitive() || cd.isPackagePrivate()) {
            print(type.typeName());
        } else {
            writer.printLink(new LinkInfoImpl(
                LinkInfoImpl.CONTEXT_SERIAL_MEMBER, type));
        }
    }
    
    public void writeHeader(String heading) {
        if (! printedOverallAnchor) {
            writer.anchor("serializedForm");
            printedOverallAnchor = true;
            writer.printTableHeadingBackground(heading);
            writer.println();
            if (heading.equals(
                   configuration().getText("doclet.Serialized_Form_class"))) {
                writer.dl();
            }
        } else {
            writer.printTableHeadingBackground(heading);
            writer.println();
        }
    }
    
    public void writeMemberHeader(ClassDoc fieldType, String fieldTypeStr,
            String fieldDimensions, String fieldName) {
        if (printedFirstMember) {
            writer.printMemberHeader();
        }
        printedFirstMember = true;
        writer.h3();
        writer.print(fieldName);
        writer.h3End();
        writer.pre();
        if (fieldType == null) {
            writer.print(fieldTypeStr);
        } else {
            writer.printLink(new LinkInfoImpl(LinkInfoImpl.CONTEXT_SERIAL_MEMBER, 
                fieldType));
        }
        print(fieldDimensions + ' ');
        bold(fieldName);
        writer.preEnd();
        writer.dl();
    }
    
    /**
     * Write the deprecated information for this member.
     *
     * @param field the field to document.
     */
    public void writeMemberDeprecatedInfo(FieldDoc field) {
        print(((TagletOutputImpl)
            (new DeprecatedTaglet()).getTagletOutput(field, 
            writer.getTagletWriterInstance(false))).toString());
    }
    
    /**
     * Write the description text for this member.
     *
     * @param field the field to document.
     */
    public void writeMemberDescription(FieldDoc field) {
        if (field.inlineTags().length > 0) {
            writer.dd();
            writer.printInlineComment(field);
        }
        Tag[] tags = field.tags("serial");
        if (tags.length > 0) {
            writer.dt();
            writer.dd();
            writer.printInlineComment(field, tags[0]);            
        }        
    }
    
    /**
     * Write the description text for this member represented by the tag.
     *
     * @param serialFieldTag the field to document (represented by tag).
     */
    public void writeMemberDescription(SerialFieldTag serialFieldTag) {        
        writer.dd();
        writer.print(serialFieldTag.description());
        writer.dlEnd();
    }
    
    /**
     * Write the tag information for this member.
     *
     * @param field the field to document.
     */
    public void writeMemberTags(FieldDoc field) {
        writer.dl();
        TagletOutputImpl output = new TagletOutputImpl("");
        TagletWriter.genTagOuput(configuration().tagletManager, field,
            configuration().tagletManager.getCustomTags(field),
                writer.getTagletWriterInstance(false), output);
        if (output.toString().length() > 0) {
            print(output.toString());
        }
        writer.dlEnd();
    }    
    public void writeMemberFooter(FieldDoc member) {
        writer.dlEnd();
    }
}


