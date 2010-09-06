/*
 * @(#)WriterFactoryImpl.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.javadoc.*;

/**
 * The factory that returns HTML writers.
 *
 * @author Jamie Ho
 * @since 1.5
 */
public class WriterFactoryImpl implements WriterFactory {
    
    private static WriterFactoryImpl instance;
    
    private ConfigurationImpl configuration;
    
    private WriterFactoryImpl(ConfigurationImpl configuration) {
        this.configuration = configuration;
    }
    
    /**
     * Return an instance of this factory.
     *
     * @return an instance of this factory.
     */
    public static WriterFactoryImpl getInstance() {
        if (instance == null) {
            instance = new WriterFactoryImpl(ConfigurationImpl.getInstance());
        }
        return instance;
    }
    
    /**
     * {@inheritDoc}
     */
    public ConstantsSummaryWriter getConstantsSummaryWriter() throws Exception {
        return new ConstantsSummaryWriterImpl(configuration);
    }
    
    /**
     * {@inheritDoc}
     */
    public PackageSummaryWriter getPackageSummaryWriter(PackageDoc packageDoc,
        PackageDoc prevPkg, PackageDoc nextPkg) throws Exception {
        return new PackageWriterImpl(ConfigurationImpl.getInstance(), packageDoc,
            prevPkg, nextPkg);
    }
    
    /**
     * {@inheritDoc}
     */
    public ClassWriter getClassWriter(ClassDoc classDoc, ClassDoc prevClass, 
            ClassDoc nextClass, ClassTree classTree)
            throws Exception {
        return new ClassWriterImpl(classDoc, prevClass, nextClass, classTree);
    }
    
    /**
     * {@inheritDoc}
     */
    public AnnotationTypeWriter getAnnotationTypeWriter(
        AnnotationTypeDoc annotationType, Type prevType, Type nextType)
    throws Exception {
        return new AnnotationTypeWriterImpl(annotationType, prevType, nextType);
    }
    
    /**
     * {@inheritDoc}
     */
    public AnnotationTypeOptionalMemberWriter 
            getAnnotationTypeOptionalMemberWriter(
        AnnotationTypeWriter annotationTypeWriter) throws Exception {
        return new AnnotationTypeOptionalMemberWriterImpl(
            (SubWriterHolderWriter) annotationTypeWriter, 
            annotationTypeWriter.getAnnotationTypeDoc());
    }
    
    /**
     * {@inheritDoc}
     */
    public AnnotationTypeRequiredMemberWriter 
            getAnnotationTypeRequiredMemberWriter(AnnotationTypeWriter annotationTypeWriter) throws Exception {
        return new AnnotationTypeRequiredMemberWriterImpl(
            (SubWriterHolderWriter) annotationTypeWriter, 
            annotationTypeWriter.getAnnotationTypeDoc());
    }
    
    /**
     * {@inheritDoc}
     */
    public EnumConstantWriter getEnumConstantWriter(ClassWriter classWriter)
            throws Exception {
        return new EnumConstantWriterImpl((SubWriterHolderWriter) classWriter, 
            classWriter.getClassDoc());
    }
    
    /**
     * {@inheritDoc}
     */
    public FieldWriter getFieldWriter(ClassWriter classWriter)
            throws Exception {
        return new FieldWriterImpl((SubWriterHolderWriter) classWriter, 
            classWriter.getClassDoc());
    }
    
    /**
     * {@inheritDoc}
     */
    public  MethodWriter getMethodWriter(ClassWriter classWriter)
            throws Exception {
        return new MethodWriterImpl((SubWriterHolderWriter) classWriter, 
            classWriter.getClassDoc());
    }
    
    /**
     * {@inheritDoc}
     */
    public ConstructorWriter getConstructorWriter(ClassWriter classWriter)
            throws Exception {
        return new ConstructorWriterImpl((SubWriterHolderWriter) classWriter, 
            classWriter.getClassDoc());
    }    
    
    /**
     * {@inheritDoc}
     */
    public MemberSummaryWriter getMemberSummaryWriter(
        ClassWriter classWriter, int memberType)
    throws Exception {
        switch (memberType) {
            case VisibleMemberMap.CONSTRUCTORS:
                return (ConstructorWriterImpl) getConstructorWriter(classWriter);
            case VisibleMemberMap.ENUM_CONSTANTS:
                return (EnumConstantWriterImpl) getEnumConstantWriter(classWriter);
            case VisibleMemberMap.FIELDS:
                return (FieldWriterImpl) getFieldWriter(classWriter);
            case VisibleMemberMap.INNERCLASSES:
                return new NestedClassWriterImpl((SubWriterHolderWriter) 
                    classWriter, classWriter.getClassDoc());
            case VisibleMemberMap.METHODS:
                return (MethodWriterImpl) getMethodWriter(classWriter);
            default:
                return null;
        }
    }
    
    /**
     * {@inheritDoc}
     */
    public MemberSummaryWriter getMemberSummaryWriter(
        AnnotationTypeWriter annotationTypeWriter, int memberType)
    throws Exception {
        switch (memberType) {
            case VisibleMemberMap.ANNOTATION_TYPE_MEMBER_OPTIONAL:
                return (AnnotationTypeOptionalMemberWriterImpl) 
                    getAnnotationTypeOptionalMemberWriter(annotationTypeWriter);
            case VisibleMemberMap.ANNOTATION_TYPE_MEMBER_REQUIRED:
                return (AnnotationTypeRequiredMemberWriterImpl) 
                    getAnnotationTypeRequiredMemberWriter(annotationTypeWriter);
            default:
                return null;
        }
    }
    
    /**
     * {@inheritDoc}
     */
    public SerializedFormWriter getSerializedFormWriter() throws Exception {
        return new SerializedFormWriterImpl();
    }
}
