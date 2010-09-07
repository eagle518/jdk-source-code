/*
 * @(#)PropertyTreeModel.java	1.6 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeNode;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.EventListenerList;

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
            
            fireTreeNodesChanged(path);
        }           
    }
    
    protected void fireTreeNodesChanged( TreePath path ){
        // Guaranteed to return a non-null array
        Object[] listeners = listenersList.getListenerList();
        TreeModelEvent tme = null;
        
        // Process the listeners last to first, notifying
        // those that are interested in this event
        for ( int i = listeners.length-2; i >= 0; i = i - 2 ){
            if ( listeners[i] == TreeModelListener.class ){
                // Lazily create the event:
                if ( tme == null ){
                    tme = new TreeModelEvent(this, path);
                }
                
                ((TreeModelListener)listeners[i + 1]).treeNodesChanged(tme);
            }
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

    /*
     * The tree model listener is needed in order to reflect the change
     * in radio button selection.  We don't need any special tree model
     * listener, the default will be provided by JTree.
     */
    public void addTreeModelListener(TreeModelListener l) {
        listenersList.add(TreeModelListener.class, l);        
    }
    public void removeTreeModelListener(TreeModelListener l) {
        listenersList.remove(TreeModelListener.class, l);
    }

    private ITreeNode root;
    private EventListenerList listenersList = new EventListenerList();

}

