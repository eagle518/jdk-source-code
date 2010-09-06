/*
 * @(#)TreeBuilder.java	1.7 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

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
        
        /*
         * Add mouse listener to track mouse clicks and repaint the
         * tree when user clicks with mouse anywhere on the tree.
         * This ensures that no two radio buttons will be checked at
         * the same time.
         */
        tree.addMouseListener(new TreeMouseListener());
    
        return tree;
    }
    
    static final class TreeMouseListener extends MouseAdapter{
        
        public TreeMouseListener(){
            super();
        }
        
        public void mouseClicked(MouseEvent e){
            if (e.getSource() instanceof JTree){
                /*
                 * This is in order to repaint the tree and update current
                 * selection in radio button group.
                 */
                ((JTree)e.getSource()).repaint();
            }
        }        
    }
}
