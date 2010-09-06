/*
 * @(#)WindowsPlacesBar.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.swing;

import java.awt.*;
import java.awt.event.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.*;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.filechooser.*;

import sun.awt.shell.*;

/**
 * <b>WARNING:</b> This class is an implementation detail and is only
 * public so that it can be used by two packages. You should NOT consider
 * this public API.
 * <p>
 * 
 * @version 1.4, 12/19/03
 * @author Leif Samuelsson
 */
public class WindowsPlacesBar extends JToolBar
			      implements ActionListener, PropertyChangeListener {
    JFileChooser fc;
    JToggleButton[] buttons;
    ButtonGroup buttonGroup;
    File[] files;
    final Dimension buttonSize;

    public WindowsPlacesBar(JFileChooser fc, boolean isXPStyle) {
	super(JToolBar.VERTICAL);
	this.fc = fc;
	setFloatable(false);
	putClientProperty("JToolBar.isRollover", Boolean.TRUE);

	boolean isXPPlatform = (System.getProperty("os.name").startsWith("Windows") &&
				System.getProperty("os.version").compareTo("5.1") >= 0);

	if (isXPStyle) {
	    buttonSize = new Dimension(83, 69);
	    putClientProperty("XPStyle.subClass", "placesbar");
	    setBorder(new EmptyBorder(1, 1, 1, 1));
	} else {
	    // The button size almost matches the XP style when in Classic style on XP
	    buttonSize = new Dimension(83, isXPPlatform ? 65 : 54);
	    setBorder(new BevelBorder(BevelBorder.LOWERED,
				      UIManager.getColor("ToolBar.highlight"),
				      UIManager.getColor("ToolBar.background"),
				      UIManager.getColor("ToolBar.darkShadow"),
				      UIManager.getColor("ToolBar.shadow")));
	}
	Color bgColor = new Color(UIManager.getColor("ToolBar.shadow").getRGB());
	setBackground(bgColor);
	FileSystemView fsv = fc.getFileSystemView();

	files = (File[])ShellFolder.get("fileChooserShortcutPanelFolders");
	buttons = new JToggleButton[files.length];
	buttonGroup = new ButtonGroup();
	for (int i = 0; i < files.length; i++) {
	    if (fsv.isFileSystemRoot(files[i])) {
		// Create special File wrapper for drive path
		files[i] = fsv.createFileObject(files[i].getAbsolutePath());
	    }

	    String folderName = fsv.getSystemDisplayName(files[i]);
	    int index = folderName.lastIndexOf(File.separatorChar);
	    if (index >= 0 && index < folderName.length() - 1) {
		folderName = folderName.substring(index + 1);
	    }
	    Icon icon = null;
	    if (files[i] instanceof ShellFolder) {
		// We want a large icon, fsv only gives us a small.
		ShellFolder sf = (ShellFolder)files[i];
		icon = new ImageIcon(sf.getIcon(true), sf.getFolderType());
	    } else {
		icon = fsv.getSystemIcon(files[i]);
	    }
	    buttons[i] = new JToggleButton(folderName, icon);
	    if (isXPPlatform) {
		buttons[i].setIconTextGap(2);
		buttons[i].setMargin(new Insets(2, 2, 2, 2));
		buttons[i].setText("<html><center>"+folderName+"</center></html>");
	    }
	    if (!isXPStyle) {
		Color fgColor = new Color(UIManager.getColor("List.selectionForeground").getRGB());
		buttons[i].setBackground(bgColor);
		buttons[i].setForeground(fgColor);
	    }
	    buttons[i].setHorizontalTextPosition(JToggleButton.CENTER);
	    buttons[i].setVerticalTextPosition(JToggleButton.BOTTOM);
	    buttons[i].setAlignmentX(JComponent.CENTER_ALIGNMENT);
	    buttons[i].setPreferredSize(buttonSize);
	    buttons[i].setMaximumSize(buttonSize);
	    buttons[i].addActionListener(this);
	    add(buttons[i]);
	    if (i < files.length-1 && isXPStyle) {
		add(Box.createRigidArea(new Dimension(1, 1)));
	    }
	    buttonGroup.add(buttons[i]);
	}
	doDirectoryChanged(fc.getCurrentDirectory());
    }

    protected void doDirectoryChanged(File f) {
	for (int i=0; i<buttons.length; i++) {
	    JToggleButton b = buttons[i];
	    if (files[i].equals(f)) {
		b.setSelected(true);
		break;
	    } else if (b.isSelected()) {
		// Remove temporarily from group because it doesn't
		// allow for no button to be selected.
		buttonGroup.remove(b);
		b.setSelected(false);
		buttonGroup.add(b);
	    }
	}
    }

    public void propertyChange(PropertyChangeEvent e) {
	String prop = e.getPropertyName();
	if (prop == JFileChooser.DIRECTORY_CHANGED_PROPERTY) {
	    doDirectoryChanged(fc.getCurrentDirectory());
	}
    }

    public void actionPerformed(ActionEvent e) {
	JToggleButton b = (JToggleButton)e.getSource();
	for (int i=0; i<buttons.length; i++) {
	    if (b == buttons[i]) {
		fc.setCurrentDirectory(files[i]);
		break;
	    }
	}
    }

    public Dimension getPreferredSize() {
	Dimension min  = super.getMinimumSize();
	Dimension pref = super.getPreferredSize();
	if (min.height > pref.height) {
	    pref = new Dimension(pref.width, min.height);
	}
	return pref;
    }
}

