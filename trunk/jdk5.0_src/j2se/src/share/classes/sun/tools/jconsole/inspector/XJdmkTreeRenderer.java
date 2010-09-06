/*
 * @(#)XJdmkTreeRenderer.java	1.5 04/04/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

// java import
import java.awt.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;
import java.awt.event.*;
import java.io.*;
import java.util.Date;
//

// jmx import
import javax.management.*;
//

// swing import
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.tree.*;
//
import sun.tools.jconsole.MBeansTab;

public class XJdmkTreeRenderer extends DefaultTreeCellRenderer {
    public void superGetTreeCellRendererComponent(final JTree tree,
						  final Object value,
						  final boolean sel,
						  final boolean expanded,
						  final boolean leaf,
						  final int row,
						  final boolean hasFocus) {
	super.getTreeCellRendererComponent(tree, 
					   value,
					   sel,
					   expanded, 
					   leaf, 
					   row,
					   hasFocus);
    }

    public Component getTreeCellRendererComponent(final JTree tree,
						  final Object value,
						  final boolean sel,
						  final boolean expanded,
						  final boolean leaf,
						  final int row,
						  final boolean hasFocus) {
	super.getTreeCellRendererComponent(tree, 
					   value,
					   sel,
					   expanded, 
					   leaf, 
					   row,
					   hasFocus);
	final Object userObject = 
	    ((DefaultMutableTreeNode)value).getUserObject();
	Icon ic = null;
	if (userObject instanceof XMBean) {
	    final XMBean xmbean = (XMBean) userObject;
	    setIcon((ImageIcon)(xmbean.getIcon()));
	}
	if (value.toString().equals(((XMBeanTree)tree).getTitle())) {
	    try {
		setIcon(IconManager.MBEANTREE_ROOT);
	    }catch(Exception e) {
		System.out.println("Error setting Tree root icon :"+ 
				   e.getMessage());
	    }
	}
	return this;
    }
}
