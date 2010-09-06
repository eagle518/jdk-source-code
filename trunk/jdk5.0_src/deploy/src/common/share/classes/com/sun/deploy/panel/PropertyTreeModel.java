/*
 * @(#)PropertyTreeModel.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.event.TreeModelListener;

public class PropertyTreeModel implements TreeModel {

    public PropertyTreeModel( ITreeNode root ) {
        this.root = root;
    }

    public Object getRoot() {
        return root;
    }

    public int getChildCount(Object parent) {
         if ( parent instanceof ITreeNode ) {
             ITreeNode path = (ITreeNode)parent;
             return path.getChildNodeCount() + path.getPropertyCount();
         }

         return 0;
     }

    public Object getChild(Object parent, int index) {
        ITreeNode path = (ITreeNode)parent;
        int iNodeCount = path.getChildNodeCount();
        return ( index < iNodeCount ) ?
                (Object)path.getChildNode( index ) : path.getProperty( index - iNodeCount );
    }

    public boolean isLeaf(Object node) {
        return ( node instanceof IProperty );
    }

    public void valueForPathChanged( TreePath path, Object newValue ) {
        Object oLast = path.getLastPathComponent();
        if ( oLast instanceof IProperty ) {
            ((IProperty)oLast).setValue( (String)newValue );  
        }           
    }

    public int getIndexOfChild(Object parent, Object child) {
        for ( int i = 0; i < getChildCount( parent ); i++ ) {
            if ( getChild( parent, i ) == child ) {
                return i;
            }
        }
        return -1;
    }

    // Our tree does not change, so we don't need to worry...
    public void addTreeModelListener(TreeModelListener l) {}
    public void removeTreeModelListener(TreeModelListener l) {}

    private ITreeNode root;

}
