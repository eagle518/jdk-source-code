/*
 * @(#)TreeBuilder.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.*;
import java.awt.Font;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Component;
import java.awt.event.MouseEvent;
import javax.swing.tree.TreeSelectionModel;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.ToolTipManager;

/*
 * Build the JTree and set all the properties for it.
 */
public class TreeBuilder {

    public static final JTree createTree( PropertyTreeModel model ) {
        JTree tree = new JTree();
        
        tree.setModel( model );
        tree.setCellRenderer( TreeRenderers.getRenderer() );
        TreeEditors.DelegateEditor editor = new TreeEditors.DelegateEditor( tree );
        tree.setEditable( true );
	tree.setOpaque( true );
        tree.setCellEditor( editor );
        tree.setDoubleBuffered(true); 
        tree.putClientProperty("JTree.lineStyle", "None");
        
        tree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);       

        // To work around look and feel appearance:
        tree.setRowHeight(0);     
        
        /*
         * This is needed to show the tooltips on the nodes.
         */
        ToolTipManager.sharedInstance().registerComponent(tree);
        
        /*
         * Add key listener to track "Space" key events and toggle
         * checkbox and radiobutton properties in the tree.
         */
        tree.addKeyListener(new SpecialTreeListener());
           
        return tree;
    }
}
