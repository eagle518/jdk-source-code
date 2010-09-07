/*
 * @(#)BeanCustomizer.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.beans.*;
import java.awt.*;
import java.awt.event.*;
import sun.awt.windows.WEmbeddedFrame;
import sun.plugin.viewer.AxBridgeObject;

public class BeanCustomizer{

    /*
     * Constructor
     * Build a new interface object for a customizer
     * @param customizerClass JavaBean customizer class
     */
    public BeanCustomizer(BeanInfo bInfo) {
	this.bInfo = bInfo;
    }

    /*
     * Open the customizer inside the Embedded Frame specified 
     * @param frame Frame object hosting the customizer component
     */
    public boolean open(int handle, int r, int g, int b) {
	boolean result = false;

	if (comp != null) {
	    frame = new WEmbeddedFrame((long)handle);
	    if(frame != null) {
		frame.setLayout(new BorderLayout());
		Color bkColor = new Color(r, g, b);
		frame.setBackground(bkColor);
		comp.setBackground(bkColor);
		comp.setSize(comp.getPreferredSize());
		frame.setSize(comp.getPreferredSize());
		frame.add(comp);
		frame.validate();
		result = true;
	    }
	}

	return result;
    }

    /*
     * Show customizer frame
     */
    public void show(boolean enable) {
	//System.out.println("Enable: " + enable);
	frame.setVisible(enable);
	comp.repaint();
    }

    /*
     * Move customizer frame
     */
    public void move(int x, int y, int width, int height) {
	frame.setBounds(x, y, width, height);
    }

    /*
     * setObjects specifies which JavaBean is going to be 
     * attached to the customizer
     * @param bean JavaBean to be customized
     */
    public void setObject(Object obj) {
	Object bean = ((AxBridgeObject)obj).getJavaObject();
	try {
	    Class cls = bInfo.getBeanDescriptor().getCustomizerClass();
	    if(cls != null) {
		customizer = (Customizer)cls.newInstance();
		comp = (Component)customizer;
		customizer.setObject(bean);
	    }
	    
	    //If no customizer, try property editors
	    if(comp == null) {
		PropertyDescriptor[] pds = bInfo.getPropertyDescriptors();
		for(int i=0;i<pds.length;i++) {
		    cls = pds[i].getPropertyEditorClass();
		    if(cls != null) {
			//System.out.println(cls.toString() + i);
			propEditor = (PropertyEditor)cls.newInstance();
			if(propEditor.supportsCustomEditor()) {
			    comp = propEditor.getCustomEditor();
			    propEditor.setValue(bean);
			    break;
			}
		    }
		}
	    }
	} catch(Throwable e) {
	    e.printStackTrace();
	}
    }
  
    private boolean dirty = false;	    
    Customizer customizer = null;
    PropertyEditor propEditor  = null;
    Component comp;
    WEmbeddedFrame frame;
    BeanInfo bInfo;
}

