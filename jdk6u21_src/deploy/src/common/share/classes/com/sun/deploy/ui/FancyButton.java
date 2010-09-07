/*
 * @(#)FancyButton.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * ActionLabel.java
 *
 * Created on April 12, 2005, 3:21 PM
 *
 * @author mfisher
 */

package com.sun.deploy.ui;

import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.BorderFactory;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Graphics;
import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import javax.swing.InputMap;
import javax.swing.KeyStroke;
import java.awt.event.KeyEvent;
import javax.swing.AbstractAction;
import java.awt.event.ActionEvent;
import com.sun.deploy.resources.ResourceManager;


/**
 * This is a label with specific text color <code>originalColor</code>.
 * On <code>mouseEntered</code> event the color of the label changes
 * to <code>activeColor</code>, which is by default red. 
 * On <code>mouseExited</code> event the color of the label changes back
 * to the <code>originalColor</code>.  The cursor for this component is set 
 * to <code>Cursor.HAND_CURSOR</code>.  
 *
 * This class is used in several places, and since each use case requires
 * different action on <code>mouseClicked</code> event, we do not specify
 * an action for it here.  You should add mouse listener to the instance
 * of this label and implement specific action on <code>mouseClicked</code>
 * event.
 * 
 */
class FancyButton extends JButton implements MouseListener{
    private static Color originalColor = new Color(53, 85, 107);
    private Color activeColor = new Color(192, 102, 0);
    private Cursor handCursor = new Cursor(Cursor.HAND_CURSOR);
    
    /*
     * Use default color - Sun dark gray
     */
    FancyButton(String text){
        this(text, originalColor);
    }
    
    FancyButton(String text, Color color){
        /* 
         * Since this is fancy button now, it needs to get resource string 
         * from the resource bundle.
         */
        super(ResourceManager.getMessage(text));
        setMnemonic(ResourceManager.getAcceleratorKey(text));
        originalColor = color;
        setForeground(originalColor);
        setCursor(handCursor);
        setBorderPainted(false);
        setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        addMouseListener(this);
        
        // Disable rollover to have no background/foreground color 
        // changes on mouse rollover based on l&f.
        setRolloverEnabled(false);
        
        // For metal l&f do not draw metal/ocean button color
        this.setContentAreaFilled(false);

        // This fancy button should ignore <enter> key to allow dialog's
        // default key to be pressed on <enter> event.
        KeyStroke ks;
        ks = KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0);
        getInputMap().put(ks, "none");
    }
    
    public void mouseClicked(MouseEvent e) {}
    
    public void mouseEntered(MouseEvent e) {        
        setForeground(activeColor);
    }
    
    public void mouseExited(MouseEvent e) {
        if(originalColor != null){
            setForeground(originalColor);
        }
    }  
    
    public void mouseReleased(MouseEvent e){}
    public void mousePressed(MouseEvent e){}    

}

