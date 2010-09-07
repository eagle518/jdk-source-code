/*
 * @(#)ResourceVisitor.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;
import com.sun.deploy.xml.XMLable;

/**
 * A visitor interface for the various ResourceType objects
 */
public interface ResourceVisitor {
    public void visitJARDesc(JARDesc jad);
    public void visitPropertyDesc(PropertyDesc prd);
    public void visitPackageDesc(PackageDesc pad);
    public void visitExtensionDesc(ExtensionDesc ed);
    public void visitJREDesc(JREDesc jrd);
}

