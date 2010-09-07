/*
 * @(#)SimpleTreeNode.java	1.6 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

class SimpleTreeNode implements ITreeNode {

    SimpleTreeNode( String desc ) {
        String text = com.sun.deploy.resources.ResourceManager.getMessage(desc);
        this.desc = text;
       //this.desc = desc;
    }

    public String getDescription() {
        return desc;
    }

    public int getChildNodeCount() {
        return lChildNodes.size();
    }

    public ITreeNode getChildNode( int i ) {
        return (ITreeNode)lChildNodes.get( i );
    }

    public void addChildNode( ITreeNode node ) {
        lChildNodes.add( node );
    }

    public int getPropertyCount() {
        return lProperties.size();
    }

    public IProperty getProperty( int i ) {
        return (IProperty)lProperties.get( i );
    }

    public void addProperty( IProperty prop ) {
        lProperties.add( prop );
    }

    public String toString() {
        return desc;
    }

    private String desc;
    private List lChildNodes = new ArrayList();
    private List lProperties = new ArrayList();
}
