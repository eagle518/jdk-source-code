/*
 * @(#)ResourceVisitor.java	1.8 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

