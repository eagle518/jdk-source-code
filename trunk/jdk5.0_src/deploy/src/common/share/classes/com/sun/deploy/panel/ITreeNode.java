/*
 * @(#)ITreeNode.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
