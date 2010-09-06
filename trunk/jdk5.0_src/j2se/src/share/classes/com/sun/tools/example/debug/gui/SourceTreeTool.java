/*
 * @(#)SourceTreeTool.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */

package com.sun.tools.example.debug.gui;

import java.io.*;
import java.util.*;

import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;

import com.sun.jdi.*;
import com.sun.tools.example.debug.bdi.*;

public class SourceTreeTool extends JPanel {

    private Environment env;

    private ExecutionManager runtime;
    private SourceManager sourceManager;
    private ClassManager classManager;

    private JTree tree;
    private SourceTreeNode root;
    private SearchPath sourcePath;
    private CommandInterpreter interpreter;

    private static String HEADING = "SOURCES";

    public SourceTreeTool(Environment env) {

	super(new BorderLayout());

	this.env = env;
	this.runtime = env.getExecutionManager();
	this.sourceManager = env.getSourceManager();

	this.interpreter = new CommandInterpreter(env);

	sourcePath = sourceManager.getSourcePath();
	root = createDirectoryTree(HEADING);
	
        // Create a tree that allows one selection at a time.
        tree = new JTree(new DefaultTreeModel(root));
	tree.setSelectionModel(new SingleLeafTreeSelectionModel());

	/******
        // Listen for when the selection changes.
        tree.addTreeSelectionListener(new TreeSelectionListener() {
            public void valueChanged(TreeSelectionEvent e) {
                SourceTreeNode node = (SourceTreeNode)
		    (e.getPath().getLastPathComponent());
		interpreter.executeCommand("view " + node.getRelativePath());
	    }
	});
	******/

	MouseListener ml = new MouseAdapter() {
	    public void mouseClicked(MouseEvent e) {
		int selRow = tree.getRowForLocation(e.getX(), e.getY());
		TreePath selPath = tree.getPathForLocation(e.getX(), e.getY());
		if(selRow != -1) {
		    if(e.getClickCount() == 1) {
			SourceTreeNode node =
			    (SourceTreeNode)selPath.getLastPathComponent();
			// If user clicks on leaf, select it, and issue 'view' command.
			if (node.isLeaf()) {
			    tree.setSelectionPath(selPath);
			    interpreter.executeCommand("view " + node.getRelativePath());
			}
		    }
		}
	    }
	};
	tree.addMouseListener(ml);

	JScrollPane treeView = new JScrollPane(tree);
	add(treeView);

	// Create listener for source path changes.

	SourceTreeToolListener listener = new SourceTreeToolListener();
	sourceManager.addSourceListener(listener);

        //### remove listeners on exit!
    }

    private class SourceTreeToolListener implements SourceListener {

	public void sourcepathChanged(SourcepathChangedEvent e) {
            sourcePath = sourceManager.getSourcePath();
	    root = createDirectoryTree(HEADING);
	    tree.setModel(new DefaultTreeModel(root));
	}
	
    }

    private static class SourceOrDirectoryFilter implements FilenameFilter {
	public boolean accept(File dir, String name) {
	    return (name.endsWith(".java") ||
		    new File(dir, name).isDirectory());
	}
    }

    private static FilenameFilter filter = new SourceOrDirectoryFilter();

    SourceTreeNode createDirectoryTree(String label) {
	try {
	    return new SourceTreeNode(label, null, "", true);
	} catch (SecurityException e) {
	    env.failure("Cannot access source file or directory");
	    return null;
	}
    }


    class SourceTreeNode implements TreeNode {
    
	private String name;
	private boolean isDirectory;
	private SourceTreeNode parent;
	private SourceTreeNode[] children;
	private String relativePath;
	private boolean isExpanded;
	
	private SourceTreeNode(String label,
			       SourceTreeNode parent,
			       String relativePath,
			       boolean isDirectory) {
	    this.name = label;
	    this.relativePath = relativePath;
	    this.parent = parent;
	    this.isDirectory = isDirectory;
	}

	public String toString() {
	    return name;
	}
	
	public String getRelativePath() {
	    return relativePath;
	}
	
	private void expandIfNeeded() {
	    try {
		if (!isExpanded && isDirectory) {
		    String[] files = sourcePath.children(relativePath, filter);
		    children = new SourceTreeNode[files.length];
		    for (int i = 0; i < files.length; i++) {
			String childName =
			    (relativePath.equals(""))
			    ? files[i]
			    : relativePath + File.separator + files[i];
			File file = sourcePath.resolve(childName);
			boolean isDir = (file != null && file.isDirectory());
			children[i] = 
			    new SourceTreeNode(files[i], this, childName, isDir);
		    }
		}
		isExpanded = true;
	    } catch (SecurityException e) {
		children = null;
		env.failure("Cannot access source file or directory");
	    }
	}
	
	// -- interface TreeNode --
	
	/*
	 * Returns the child <code>TreeNode</code> at index 
	 * <code>childIndex</code>.
	 */
	public TreeNode getChildAt(int childIndex) {
	    expandIfNeeded();
	    return children[childIndex];
	}
	
	/**
	 * Returns the number of children <code>TreeNode</code>s the receiver
	 * contains.
	 */
	public int getChildCount() {
	    expandIfNeeded();
	    return children.length;
	}
	
	/**
	 * Returns the parent <code>TreeNode</code> of the receiver.
	 */
	public TreeNode getParent() {
	    return parent;
	}
	
	/**
	 * Returns the index of <code>node</code> in the receivers children.
	 * If the receiver does not contain <code>node</code>, -1 will be
	 * returned.
	 */
	public int getIndex(TreeNode node) {
	    expandIfNeeded();
	    for (int i = 0; i < children.length; i++) {
		if (children[i] == node)
		    return i;
	    }
	    return -1;
	}
	
	/**
	 * Returns true if the receiver allows children.
	 */
	public boolean getAllowsChildren() {
	    return isDirectory;
	}
	
	/**
	 * Returns true if the receiver is a leaf.
	 */
	public boolean isLeaf() {
	    expandIfNeeded();
	    return !isDirectory;
	}
	
	/**
	 * Returns the children of the receiver as an Enumeration.
	 */
	public Enumeration children() {
	    expandIfNeeded();
	    return new Enumeration() {
		int i = 0;
		public boolean hasMoreElements() {
		    return (i < children.length);
		}
		public Object nextElement() throws NoSuchElementException {
		    if (i >= children.length) {
			throw new NoSuchElementException();
		    }
		    return children[i++];
		}
	    };
	}

    }

}
