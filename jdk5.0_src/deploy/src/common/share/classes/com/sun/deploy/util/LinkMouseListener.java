/*
 * LinkMouseListener.java
 *
 * Created on May 21, 2003, 4:10 PM
 */

package com.sun.deploy.util;


import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import java.awt.Cursor;
import java.awt.Color;

import javax.swing.JLabel;

/**
 *
 * @author  mfisher
 * @version 
 */
public class LinkMouseListener implements MouseListener {

    private native void launchLink(String link);

    private Cursor handCursor = new Cursor(Cursor.HAND_CURSOR); 
    private Color	txtColor =null;
    private Cursor	lblCursor = null;
    private JLabel	label;
	
    public LinkMouseListener(JLabel label) {
        this.label = label;
    }    

    public void mouseClicked(MouseEvent e) {
        final String url = label.getText();
        Thread t = new Thread(new Runnable() {
            public void run() {
                launchLink(url);
            }
	});
	t.start();
    }
	 
    public void mouseEntered(MouseEvent e) {
        txtColor = label.getForeground();
        lblCursor = label.getCursor();

	label.setForeground(Color.RED);
	label.setCursor(handCursor);
    }

    public void mouseExited(MouseEvent e) {
        if(txtColor != null)
            label.setForeground(txtColor);

        if(lblCursor != null)
            label.setCursor(lblCursor);
    }
	
    public void mousePressed(MouseEvent e) {	
    } 
    public void mouseReleased(MouseEvent e) {
    }

}
