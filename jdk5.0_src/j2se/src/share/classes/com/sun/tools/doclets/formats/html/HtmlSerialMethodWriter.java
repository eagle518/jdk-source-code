/*
 * @(#)HtmlSerialMethodWriter.java	1.36 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.tools.doclets.internal.toolkit.taglets.*;
import com.sun.javadoc.*;

/**
 * Generate serialized form for Serializable/Externalizable methods.
 * Documentation denoted by the <code>serialData</code> tag is processed.
 *
 * @author Joe Fialli
 */
public class HtmlSerialMethodWriter extends MethodWriterImpl implements
        SerializedFormWriter.SerialMethodWriter{
    
    private boolean printedFirstMember = false;
    
    public HtmlSerialMethodWriter(SubWriterHolderWriter writer, 
            ClassDoc classdoc) {
        super(writer, classdoc);
    }
    
    public void writeHeader(String heading) {
        writer.anchor("serialized_methods");
        writer.printTableHeadingBackground(heading);
        writer.p();
    }
            
    public void writeNoCustomizationMsg(String msg) {
        writer.print(msg);
        writer.p();
    }
            
    public void writeMemberHeader(MethodDoc member) {
        if (printedFirstMember) {
            writer.printMemberHeader();
        }
        printedFirstMember = true;
        writer.anchor(member);
        printHead(member);
        writeSignature(member); 
    }
            
    public void writeMemberFooter(MethodDoc member) {       
        writer.dlEnd();
    }
    
    public void writeDeprecatedMemberInfo(MethodDoc member) {
        print(((TagletOutputImpl)
            (new DeprecatedTaglet()).getTagletOutput(member, 
            writer.getTagletWriterInstance(false))).toString());
    }
    
    public void writeMemberDescription(MethodDoc member) {
        printComment(member);
    }
    
    public void writeMemberTags(MethodDoc member) {
        writer.dd();
        writer.dl();
        TagletOutputImpl output = new TagletOutputImpl("");
        TagletManager tagletManager = 
            ConfigurationImpl.getInstance().tagletManager;
        TagletWriter.genTagOuput(tagletManager, member,
            tagletManager.getSerializedFormTags(),
            writer.getTagletWriterInstance(false), output);
        print(output.toString());
        MethodDoc method = (MethodDoc)member;
        if (method.name().compareTo("writeExternal") == 0
                && method.tags("serialData").length == 0) {
            serialWarning(member.position(), "doclet.MissingSerialDataTag",
                method.containingClass().qualifiedName(), method.name());
        }
        writer.ddEnd();
        writer.dlEnd();
    }
    
    protected void printTypeLinkNoDimension(Type type) {
        ClassDoc cd = type.asClassDoc();
        if (type.isPrimitive() || cd.isPackagePrivate()) {
            print(type.typeName());
        } else {
            writer.printLink(new LinkInfoImpl(
                LinkInfoImpl.CONTEXT_SERIAL_MEMBER,type));
        }
    }
}



