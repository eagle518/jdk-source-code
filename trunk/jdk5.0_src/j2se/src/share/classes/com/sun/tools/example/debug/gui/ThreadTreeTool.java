/*
 * @(#)ThreadTreeTool.java	1.11 03/12/19
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
import java.util.List;  // Must import explicitly due to conflict with javax.awt.List

import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;

import com.sun.jdi.*;
import com.sun.tools.example.debug.event.*;
import com.sun.tools.example.debug.bdi.*;

//### Bug: If the name of a thread is changed via Thread.setName(), the
//### thread tree view does not reflect this.  The name of the thread at
//### the time it is created is used throughout its lifetime.

public class ThreadTreeTool extends JPanel {

    private Environment env;

    private ExecutionManager runtime;
    private SourceManager sourceManager;
    private ClassManager classManager;

    private JTree tree;
    private DefaultTreeModel treeModel;
    private ThreadTreeNode root;
    private SearchPath sourcePath;

    private CommandInterpreter interpreter;

    private static String HEADING = "THREADS";

    public ThreadTreeTool(Environment env) {

	super(new BorderLayout());

	this.env = env;
	this.runtime = env.getExecutionManager();
	this.sourceManager = env.getSourceManager();

	this.interpreter = new CommandInterpreter(env);

	root = createThreadTree(HEADING);
	treeModel = new DefaultTreeModel(root);
	
        // Create a tree that allows one selection at a time.
	
        tree = new JTree(treeModel);
	tree.setSelectionModel(new SingleLeafTreeSelectionModel());

	MouseListener ml = new MouseAdapter() {
	    public void mouseClicked(MouseEvent e) {
		int selRow = tree.getRowForLocation(e.getX(), e.getY());
		TreePath selPath = tree.getPathForLocation(e.getX(), e.getY());
		if(selRow != -1) {
		    if(e.getClickCount() == 1) {
			ThreadTreeNode node =
			    (ThreadTreeNode)selPath.getLastPathComponent();
			// If user clicks on leaf, select it, and issue 'thread' command.
			if (node.isLeaf()) {
			    tree.setSelectionPath(selPath);
			    interpreter.executeCommand("thread " +
						       node.getThreadId() +
						       "  (\"" +
						       node.getName() + "\")");
			}
		    }
		}
	    }
	};

	tree.addMouseListener(ml);

	JScrollPane treeView = new JScrollPane(tree);
	add(treeView);

	// Create listener.
	ThreadTreeToolListener listener = new ThreadTreeToolListener();
	runtime.addJDIListener(listener);
	runtime.addSessionListener(listener);

        //### remove listeners on exit!
    }

    HashMap threadTable = new HashMap();

    private List threadPath(ThreadReference thread) {
	// May exit abnormally if VM disconnects.
	List l = new ArrayList();
	l.add(0, thread.name());
	ThreadGroupReference group = thread.threadGroup();
	while (group != null) {
	    l.add(0, group.name());
	    group = group.parent();
	}
	return l;
    }

    private class ThreadTreeToolListener extends JDIAdapter 
                              implements JDIListener, SessionListener {

	// SessionListener

	public void sessionStart(EventObject e) {
	    try {
		Iterator iter = runtime.allThreads().iterator();
		while (iter.hasNext()) {
		    ThreadReference thread = ((ThreadReference)iter.next());
		    root.addThread(thread);
		}
	    } catch (VMDisconnectedException ee) {
		// VM went away unexpectedly.
	    } catch (NoSessionException ee) {
		// Ignore.  Should not happen.
	    }
	}

	public void sessionInterrupt(EventObject e) {}
	public void sessionContinue(EventObject e) {}


	// JDIListener

        public void threadStart(ThreadStartEventSet e) {
	    root.addThread(e.getThread());
	}

	public void threadDeath(ThreadDeathEventSet e) {
	    root.removeThread(e.getThread());
	}

        public void vmDisconnect(VMDisconnectEventSet e) {
	    // Clear the contents of this view.
	    root = createThreadTree(HEADING);
	    treeModel = new DefaultTreeModel(root);
	    tree.setModel(treeModel);
	    threadTable = new HashMap();
	}

    }

    ThreadTreeNode createThreadTree(String label) {
	return new ThreadTreeNode(label, null);
    }

    class ThreadTreeNode extends DefaultMutableTreeNode {

	String name;
	ThreadReference thread;	// null if thread group
	long uid;
	String description;

	ThreadTreeNode(String name, ThreadReference thread) {
	    if (name == null) {
		name = "<unnamed>";
	    }
	    this.name = name;
	    this.thread = thread;
	    if (thread == null) {
		this.uid = -1;
		this.description = name;
	    } else {
		this.uid = thread.uniqueID();
		this.description = name + " (t@" + Long.toHexString(uid) + ")";
	    }
	}

	public String toString() {
	    return description;
	}

	public String getName() {
	    return name;
	}

	public ThreadReference getThread() {
	    return thread;
	}

	public String getThreadId() {
	    return "t@" + Long.toHexString(uid);
	}

	private boolean isThreadGroup() {
	    return (thread == null);
	}

	public boolean isLeaf() {
	    return !isThreadGroup();
	}

	public void addThread(ThreadReference thread) {
	    // This can fail if the VM disconnects.
	    // It is important to do all necessary JDI calls
	    // before modifying the tree, so we don't abort
	    // midway through!
	    if (threadTable.get(thread) == null) {
		// Add thread only if not already present.
		try {
		    List path = threadPath(thread);
		    // May not get here due to exception.
		    // If we get here, we are committed.
		    // We must not leave the tree partially updated.
		    try {
			threadTable.put(thread, path);
			addThread(path, thread);
		    } catch (Throwable tt) {
			//### Assertion failure.
			throw new RuntimeException("ThreadTree corrupted");
		    }
		} catch (VMDisconnectedException ee) {
		    // Ignore.  Thread will not be added.
		}
	    }
	}

	private void addThread(List threadPath, ThreadReference thread) {
	    int size = threadPath.size();
	    if (size == 0) {
		return;
	    } else if (size == 1) {
		String name = (String)threadPath.get(0);
		insertNode(name, thread);
	    } else {
		String head = (String)threadPath.get(0);
		List tail = threadPath.subList(1, size);
		ThreadTreeNode child = insertNode(head, null);
		child.addThread(tail, thread);
	    }
	}

	private ThreadTreeNode insertNode(String name, ThreadReference thread) {
	    for (int i = 0; i < getChildCount(); i++) {
		ThreadTreeNode child = (ThreadTreeNode)getChildAt(i);
		int cmp = name.compareTo(child.getName());
		if (cmp == 0 && thread == null) {
		    // A like-named interior node already exists.
		    return child;
		} else if (cmp < 0) {
		    // Insert new node before the child.
		    ThreadTreeNode newChild = new ThreadTreeNode(name, thread);
		    treeModel.insertNodeInto(newChild, this, i);
		    return newChild;
		}
	    }
	    // Insert new node after last child.
	    ThreadTreeNode newChild = new ThreadTreeNode(name, thread);
	    treeModel.insertNodeInto(newChild, this, getChildCount());
	    return newChild;
	}

	public void removeThread(ThreadReference thread) {
	    List threadPath = (List)threadTable.get(thread);
	    // Only remove thread if we recorded it in table.
	    // Original add may have failed due to VM disconnect.
	    if (threadPath != null) {
		removeThread(threadPath, thread);
	    }
	}

	private void removeThread(List threadPath, ThreadReference thread) {
	    int size = threadPath.size();
	    if (size == 0) {
		return;
	    } else if (size == 1) {
		String name = (String)threadPath.get(0);
		ThreadTreeNode child = findLeafNode(thread, name);
                treeModel.removeNodeFromParent(child);
	    } else {
		String head = (String)threadPath.get(0);
		List tail = threadPath.subList(1, size);
		ThreadTreeNode child = findInternalNode(head);
		child.removeThread(tail, thread);
		if (child.isThreadGroup() && child.getChildCount() < 1) {
		    // Prune non-leaf nodes with no children.
		    treeModel.removeNodeFromParent(child);
		}
	    }
	}
	
	private ThreadTreeNode findLeafNode(ThreadReference thread, String name) {
	    for (int i = 0; i < getChildCount(); i++) {
		ThreadTreeNode child = (ThreadTreeNode)getChildAt(i);
		if (child.getThread() == thread) {
		    if (!name.equals(child.getName())) {
			//### Assertion failure.
			throw new RuntimeException("name mismatch");
		    }
		    return child;
		}
	    }
	    //### Assertion failure.
	    throw new RuntimeException("not found");
	}
	
	private ThreadTreeNode findInternalNode(String name) {
	    for (int i = 0; i < getChildCount(); i++) {
		ThreadTreeNode child = (ThreadTreeNode)getChildAt(i);
		if (name.equals(child.getName())) {
		    return child;
		}
	    }
	    //### Assertion failure.
	    throw new RuntimeException("not found");
	}

    }

}
