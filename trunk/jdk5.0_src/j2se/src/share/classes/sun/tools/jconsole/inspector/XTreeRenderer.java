/*
 * @(#)XTreeRenderer.java	1.4 04/04/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

import javax.swing.Icon;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import java.awt.Color;
import java.awt.Component;

import sun.tools.jconsole.MBeansTab;

public class XTreeRenderer extends XJdmkTreeRenderer {

    private Color defaultForegroundColor;
    private final static Color markedColor = Color.red;

    public Component getTreeCellRendererComponent(JTree tree,
						  Object value,
						  boolean sel,
						  boolean expanded,
						  boolean leaf,
						  int row,
						  boolean hasFocus) {
	Component component = 
	    super.getTreeCellRendererComponent(tree, 
					       value,
					       sel,
					       expanded, 
					       leaf,
					       row,
					       hasFocus);
	if (defaultForegroundColor == null) {
	    defaultForegroundColor = getForeground();
	}
	XTree xtree = (XTree) tree;
	if (xtree.isMarkedRow(row)) {
	    setForeground(markedColor);
	}
	else {
	    if (defaultForegroundColor != null) {
		setForeground(defaultForegroundColor);
	    }
	}
	//tool tip in case 
	if (xtree.isTreeToolTip()) {
	    Object userObject = ((DefaultMutableTreeNode)value).
		getUserObject();
	    if (userObject instanceof XMBean) {
		XMBean xmbean = (XMBean)userObject;
		setToolTipText(xmbean.getObjectName().toString());
	    }
	    else {
		setToolTipText(null);
	    }
	}
	return component;
    }
}
