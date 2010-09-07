/*
 * @(#)ITreeNode.java	1.5 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.util.Properties;

public interface ITreeNode {
    public String getDescription();

    public int getChildNodeCount();
    public ITreeNode getChildNode( int i );
    public void addChildNode( ITreeNode path );

    public int getPropertyCount();
    public IProperty getProperty( int i );
    public void addProperty( IProperty prop );

}
