/*
 * @(#)Node.java	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package j2dbench;

import java.io.PrintWriter;
import javax.swing.JLabel;
import javax.swing.JComponent;

public abstract class Node {
    private String nodeName;
    private String description;
    private Group parent;
    private Node next;

    protected Node() {
    }

    public Node(Group parent, String nodeName, String description) {
	this.parent = parent;
	this.nodeName = nodeName;
	this.description = description;
	parent.addChild(this);
    }

    public Group getParent() {
	return parent;
    }

    public String getNodeName() {
	return nodeName;
    }

    public String getTreeName() {
	String name = nodeName;
	if (parent != null) {
	    String pname = parent.getTreeName();
	    if (pname != null) {
		name = pname + "." + name;
	    }
	}
	return name;
    }

    public String getDescription() {
	return description;
    }

    public JComponent getJComponent() {
	return (nodeName != null) ? new JLabel(description) : null;
    }

    public Node getNext() {
	return next;
    }

    public void setNext(Node node) {
	this.next = node;
    }

    public void traverse(Visitor v) {
	v.visit(this);
    }

    public abstract void restoreDefault();

    public abstract void write(PrintWriter pw);

    public abstract String setOption(String key, String value);

    public static interface Visitor {
	public void visit(Node node);
    }

    public static interface Iterator {
	public boolean hasNext();
	public Node next();
    }
}
