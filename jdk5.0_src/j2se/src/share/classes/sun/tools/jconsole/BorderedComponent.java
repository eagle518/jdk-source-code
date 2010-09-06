/*
 * @(#)BorderedComponent.java	1.9 04/05/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;
import javax.swing.border.*;

public class BorderedComponent extends JPanel implements ActionListener {
    JButton moreOrLessButton;
    String labelStr, valueLabelStr;
    TitledBorder border;
    JLabel label;
    JComponent comp;
    boolean collapsed = false;

    static ImageIcon collapseIcon;
    static ImageIcon expandIcon;

    public BorderedComponent(String text, JComponent comp) {
	this(text, comp, false);
    }
    
    public void setTitle(String title) {
	border.setTitle(title);
    }

    public BorderedComponent(String text, JComponent comp, boolean collapsible) {
	super(null);

	this.labelStr = text;
	this.border = new TitledBorder(text);
	this.comp = comp;

	setBorder(new CompoundBorder(border, new EmptyBorder(0, 10, 0, 0)));

	if (collapseIcon == null || expandIcon == null) {
	    Class cl = getClass();
	    collapseIcon   = new ImageIcon(cl.getResource("resources/collapse.png"));
	    expandIcon = new ImageIcon(cl.getResource("resources/expand.png"));
	}

	if (collapsible) {
	    moreOrLessButton = new JButton(collapseIcon);
	    moreOrLessButton.addActionListener(this);
	    add(moreOrLessButton);
	}
	add(comp);
    }

    public void setValueLabel(String str) {
	this.valueLabelStr = str;
	if (label != null) {
	    label.setText(Resources.getText("Current value",valueLabelStr));
	}
    }

    public void actionPerformed(ActionEvent ev) {
	if (collapsed) {
	    if (label != null) {
		remove(label);
	    }
	    add(comp);
	    moreOrLessButton.setIcon(collapseIcon);
	} else {
	    remove(comp);
	    if (valueLabelStr != null) {
		if (label == null) {
		    label = new JLabel(Resources.getText("Current value",
                                                         valueLabelStr));
		}
		add(label);
	    }
	    moreOrLessButton.setIcon(expandIcon);
	}
	collapsed = !collapsed;

	JComponent container = (JComponent)getParent();
	if (container != null &&
	    container.getLayout() instanceof VariableGridLayout) {

	    ((VariableGridLayout)container.getLayout()).setFillRow(this, !collapsed);
	    container.revalidate();
	}
    }

    public Dimension getMinimumSize() {
	if (moreOrLessButton != null) {
	    Dimension d = moreOrLessButton.getMinimumSize();
	    Insets i = getInsets();
	    d.width  += i.left + i.right;
	    d.height += i.top + i.bottom;
	    return d;
	} else {
	    return super.getMinimumSize();
	}
    }

    public void layout() {
	Dimension d = getSize();
	Insets i = getInsets();

	if (collapsed) {
	    if (label != null) {
		Dimension p = label.getPreferredSize();
		label.setBounds(i.left,
				i.top + (d.height - i.top - i.bottom - p.height) / 2,
				p.width,
				p.height);
	    }
	} else {
	    comp.setBounds(i.left,
			   i.top,
			   d.width - i.left - i.right,
			   d.height - i.top - i.bottom);
	}
	if (moreOrLessButton != null) {
	    Dimension p = moreOrLessButton.getPreferredSize();
	    moreOrLessButton.setBounds(d.width  - i.right  - p.width,
				       d.height - i.bottom - p.height,
				       p.width,
				       p.height);
	}
    }
}
