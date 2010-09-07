/*
 * @(#)SpecialTreeListener.java	1.4 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import javax.swing.JTree;
import javax.swing.tree.TreePath;


/**
 * This class handles key events on JTree.  
 * There are two special cases:
 *
 * 1.  When "space" is pressed, if we are currently on one of the radio
 * button nodes, or checkbox nodes, we need to select that node and update
 * the tree.
 *
 * 2.  When "Down" errow key is pressed, and we have moved to the text
 * field node, we want to start editing the node, so that the first 
 * component of the node would be in focus.
 * 
 */
public class SpecialTreeListener extends KeyAdapter {

    /** Creates new SpecialTreeListener */
    public SpecialTreeListener() {
        super();
    }
    
    /*
     * This handles the "space" key event.
     */
    public void keyPressed(KeyEvent keyEvent) {  
        if (keyEvent.getSource() instanceof JTree){
            JTree tree = (JTree) keyEvent.getSource();
            
            TreePath path = tree.getSelectionPath();
            switch(keyEvent.getKeyCode()){
                case KeyEvent.VK_SPACE:
                // Toggle checkbox or radiobutton
                if (path != null && path.getLastPathComponent() instanceof IProperty){
                    IProperty prop = (IProperty)path.getLastPathComponent();
                    if ( prop instanceof ToggleProperty){
                        ((ToggleProperty)prop).setValue( ("true".equalsIgnoreCase( prop.getValue() ))? "false" : "true" );
                    }
                    if ( prop instanceof RadioProperty){
                        ((RadioProperty)prop).setValue(prop.getValue());
                    }
                    tree.repaint();
                }
                break;
            default:
                break;
            }
        }
    }
    
    /*
     * This handles the "DOWN" and "RIGHT" errow keys.
     */
    public void keyReleased(KeyEvent keyEvent) {
        if (keyEvent.getSource() instanceof JTree){
            JTree tree = (JTree) keyEvent.getSource();
            TreePath path = tree.getSelectionPath();
        
            switch(keyEvent.getKeyCode()){
                case KeyEvent.VK_DOWN:
                    if (path != null && path.getLastPathComponent() instanceof TextFieldProperty){
                        tree.startEditingAtPath(path);
                    }
                    break;
                case KeyEvent.VK_RIGHT:
                    if (path != null && path.getLastPathComponent() instanceof TextFieldProperty){
                        tree.startEditingAtPath(path);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
