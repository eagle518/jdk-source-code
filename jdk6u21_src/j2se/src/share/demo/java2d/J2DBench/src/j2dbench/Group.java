/*
 * @(#)Group.java	1.11 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package j2dbench;

import java.io.PrintWriter;
import javax.swing.BoxLayout;
import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;
import javax.swing.border.TitledBorder;
import java.util.NoSuchElementException;

import j2dbench.ui.CompactLayout;
import j2dbench.ui.EnableButton;

public class Group extends Node {
    public static Group root = new Group();

    private Node children;
    private boolean tabbed;
    private boolean hidden;
    private boolean horizontal;
    private Boolean bordered;
    private int tabPlacement;

    private Group() {
	setTabbed(JTabbedPane.LEFT);
    }

    public Group(String nodeName, String description) {
	this(root, nodeName, description);
    }

    public Group(Group parent, String nodeName, String description) {
	super(parent, nodeName, description);
    }

    public void addChild(Node child) {
	Node prev = null;
	for (Node node = children; node != null; node = node.getNext()) {
	    if (node.getNodeName().equalsIgnoreCase(child.getNodeName())) {
		throw new RuntimeException("duplicate child added");
	    }
	    prev = node;
	}
	if (prev == null) {
	    children = child;
	} else {
	    prev.setNext(child);
	}
    }

    public Node.Iterator getChildIterator() {
	return new ChildIterator();
    }

    public Node.Iterator getRecursiveChildIterator() {
	return new RecursiveChildIterator();
    }

    public Node getFirstChild() {
	return children;
    }

    public boolean isBordered() {
        if (bordered == null) {
            return (getParent() == null || !getParent().isTabbed());
        }
        return bordered.booleanValue();
    }

    public boolean isTabbed() {
	return tabbed;
    }

    public boolean isHidden() {
	return hidden;
    }

    public boolean isHorizontal() {
	return horizontal;
    }

    public void setBordered(boolean b) {
        bordered = b ? Boolean.TRUE : Boolean.FALSE;
    }

    public void setTabbed() {
        setTabbed(JTabbedPane.TOP);
    }

    public void setTabbed(int tabPlacement) {
        this.tabbed = true;
        this.tabPlacement = tabPlacement;
    }

    public void setHidden() {
	hidden = true;
    }

    public void setHorizontal() {
	horizontal = true;
    }

    public void traverse(Visitor v) {
	super.traverse(v);
	for (Node node = children; node != null; node = node.getNext()) {
	    node.traverse(v);
	}
    }

    public void restoreDefault() {
    }

    public String setOption(String key, String value) {
	int index = key.indexOf('.');
	String subkey;
	if (index < 0) {
	    subkey = "";
	} else {
	    subkey = key.substring(index+1);
	    key = key.substring(0, index);
	}
	for (Node node = children; node != null; node = node.getNext()) {
	    if (node.getNodeName().equalsIgnoreCase(key)) {
		return node.setOption(subkey, value);
	    }
	}
	return "Key failed to match an existing option";
    }

    public void write(PrintWriter pw) {
    }

    public JComponent getJComponent() {
	if (isHidden()) {
	    return null;
	} else if (isTabbed()) {
	    JTabbedPane jtp = new JTabbedPane(tabPlacement);
	    for (Node node = children; node != null; node = node.getNext()) {
		JComponent comp = node.getJComponent();
		if (comp != null) {
		    jtp.addTab(node.getDescription(), comp);
		}
	    }
	    return jtp;
	} else {
	    JPanel p = new JPanel();
	    p.setLayout(new BoxLayout(p,
				      horizontal
				      ? BoxLayout.X_AXIS
				      : BoxLayout.Y_AXIS));
	    p.setLayout(new CompactLayout(horizontal));
            if (getDescription() != null && isBordered()) {
		p.setBorder(new TitledBorder(getDescription()));
		addEnableButtons(p);
	    }
	    for (Node node = children; node != null; node = node.getNext()) {
		JComponent comp = node.getJComponent();
		if (comp != null) {
		    p.add(comp);
		}
	    }
	    return p;
	}
    }

    public void addEnableButtons(JPanel p) {
	p.add(new EnableButton(this, EnableButton.DEFAULT));
	p.add(new EnableButton(this, EnableButton.CLEAR));
	p.add(new EnableButton(this, EnableButton.INVERT));
	p.add(new EnableButton(this, EnableButton.SET));
    }

    public static void restoreAllDefaults() {
	root.traverse(new Visitor() {
	    public void visit(Node node) {
		node.restoreDefault();
	    }
	});
    }

    public static void writeAll(final PrintWriter pw) {
	root.traverse(new Visitor() {
	    public void visit(Node node) {
		node.write(pw);
	    }
	});
	pw.flush();
    }

    public static String setOption(String s) {
	int index = s.indexOf('=');
	if (index < 0) {
	    return "No value specified";
	}
	String key = s.substring(0, index);
	String value = s.substring(index+1);
	return root.setOption(key, value);
    }

    public String toString() {
	return "Group("+getTreeName()+")";
    }

    public class ChildIterator implements Node.Iterator {
	protected Node cur = getFirstChild();

	public boolean hasNext() {
	    return (cur != null);
	}

	public Node next() {
	    Node ret = cur;
	    if (ret == null) {
		throw new NoSuchElementException();
	    }
	    cur = cur.getNext();
	    return ret;
	}
    }

    public class RecursiveChildIterator extends ChildIterator {
	Node.Iterator subiterator;

	public boolean hasNext() {
	    while (true) {
		if (subiterator != null && subiterator.hasNext()) {
		    return true;
		}
		if (cur instanceof Group) {
		    subiterator = ((Group) cur).getRecursiveChildIterator();
		    cur = cur.getNext();
		} else {
		    subiterator = null;
		    return super.hasNext();
		}
	    }
	}

	public Node next() {
	    if (subiterator != null) {
		return subiterator.next();
	    } else {
		return super.next();
	    }
	}
    }

    public static class EnableSet extends Group implements Modifier {
	public EnableSet() {
	    super();
	}

	public EnableSet(Group parent, String nodeName, String description) {
	    super(parent, nodeName, description);
	}

	public Modifier.Iterator getIterator(TestEnvironment env) {
	    return new EnableIterator();
	}

	public void modifyTest(TestEnvironment env, Object val) {
	    ((Option.Enable) val).modifyTest(env);
	    env.setModifier(this, val);
	}

	public void restoreTest(TestEnvironment env, Object val) {
	    ((Option.Enable) val).restoreTest(env);
	    env.removeModifier(this);
	}

	public String getAbbreviatedModifierDescription(Object val) {
	    Option.Enable oe = (Option.Enable) val;
	    return oe.getAbbreviatedModifierDescription(Boolean.TRUE);
	}

	public String getModifierValueName(Object val) {
	    Option.Enable oe = (Option.Enable) val;
	    return oe.getModifierValueName(Boolean.TRUE);
	}

	public class EnableIterator implements Modifier.Iterator {
	    Node.Iterator childiterator = getRecursiveChildIterator();
	    Option.Enable curval;

	    public boolean hasNext() {
		if (curval != null) {
		    return true;
		}
		while (childiterator.hasNext()) {
		    Node node = childiterator.next();
		    if (node instanceof Option.Enable) {
			curval = (Option.Enable) node;
			if (curval.isEnabled()) {
			    return true;
			}
			curval = null;
		    }
		}
		return false;
	    }

	    public Object next() {
		if (curval == null) {
		    if (!hasNext()) {
			throw new NoSuchElementException();
		    }
		}
		Object ret = curval;
		curval = null;
		return ret;
	    }
	}
    }
}
