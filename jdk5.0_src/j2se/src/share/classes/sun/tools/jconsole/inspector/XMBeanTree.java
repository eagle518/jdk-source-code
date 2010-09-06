/*
 * @(#)XMBeanTree.java	1.4 04/04/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

// java import
//import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import java.io.*;
import java.io.*;
import java.util.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;
//

// swing import
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import javax.swing.*;
//

// jmx import
import javax.management.*;
//

import sun.tools.jconsole.MBeansTab;

public abstract class XMBeanTree extends JTree 
    implements TreeSelectionListener, TreeExpansionListener {
    
    public static final String SELECTION_CHANGED_EVENT = 
	"jam.xmbean.selection.changed";
    public static final String VALUE_UPDATE_EVENT = "jam.xmbean.value.update";
    
    private java.util.List<NotificationListener> notificationListenersList;
    
    private boolean expanded;
    protected HashMap<Object, String> expandedPaths;
    
    public XMBeanTree(TreeNode root) {
	super(root);
	setRootVisible(true);
	addTreeSelectionListener(this);
	addTreeExpansionListener(this);
	expandedPaths = new HashMap<Object, String>();
	List<NotificationListener> l = new ArrayList<NotificationListener>(1);
	notificationListenersList = 
	    Collections.synchronizedList(l);
    }
	
    public String getTitle() {
	return ((DefaultMutableTreeNode)getModel().getRoot()).
	    getUserObject().toString();
    }
    
    public void setTitle(String title) {
	((DefaultMutableTreeNode)getModel().getRoot()).setUserObject(title);
    }
	
    public DefaultMutableTreeNode getRoot() {
	return (DefaultMutableTreeNode)getModel().getRoot();
    }
	
    /**
     * This methods expands all tree nodes
     */
    public synchronized void expandAllNodes() {

	DefaultMutableTreeNode root = getRoot();
	Enumeration nodes = root.depthFirstEnumeration();
	while (nodes.hasMoreElements()) {
	    DefaultMutableTreeNode node = 
		(DefaultMutableTreeNode) nodes.nextElement();
	    TreePath path = new TreePath(node.getPath());
	    expandPath(path);
	}
	expanded = true;
	unselectNode(getRoot());
    }

 
	
	    
	
	
    /**
     * This change the node value
     */
    public synchronized void changeNodeValue(DefaultMutableTreeNode node,
					     Object nodeValue) {
	node.setUserObject(nodeValue);
	DefaultTreeModel model = (DefaultTreeModel)getModel();
	model.nodeChanged(node);
	//was perhaps expanded previously, re - expand it if it was
	TreePath path = new TreePath(node.getPath());
	expandNodePath(path);
    }
	
	

    /**
     * This method removes the node from its parent
     */
    public synchronized void removeChildNode(DefaultMutableTreeNode child) {
	DefaultTreeModel model = (DefaultTreeModel)getModel();
	model.removeNodeFromParent(child);

    }		    
    


    /**
     * This method adds the child to the specified parent node 
     * at specific index
     */
    public synchronized void addChildNode(DefaultMutableTreeNode parent, 
					  DefaultMutableTreeNode child,
					  int index) {

	//Tree does not show up when there is only the root node
	    
	boolean rootLeaf = false;
	if (getRoot().isLeaf()) {
	    rootLeaf = true;
	}

	DefaultTreeModel model = (DefaultTreeModel)getModel();
	model.insertNodeInto(child, parent,index);

	if (rootLeaf) {
	    model.nodeStructureChanged(getRoot());
	}

	TreePath path = new TreePath(parent.getPath());
	expandNodePath(path);	    
    }

    /**
     * This method adds the child to the specified parent node.
     */
    public synchronized void addChildNode(DefaultMutableTreeNode parent, 
					  DefaultMutableTreeNode child) {
	addChildNode(parent,child,parent.getChildCount());
    }
	
    /**
     * This method adds the XMBean to the tree.
     */
    public void addMBeanToView(MBeansTab mbeansTab, ObjectName mbean) 
	throws javax.management.InstanceNotFoundException {
	XMBean xmbean = new XMBean(mbean,mbeansTab);
	DefaultMutableTreeNode childNode = new DefaultMutableTreeNode(xmbean);
	DefaultTreeModel treeModel = (DefaultTreeModel)getModel();
		
	treeModel.insertNodeInto(childNode, 
				 getRoot(),
				 getRoot().getChildCount());	
	treeModel.nodeStructureChanged(getRoot());
    }
	
    /**
     * This method deletes the XMBean from the tree.
     */
    public void delMBeanFromView(ObjectName mbean) {
	XMBean currXMBean;				
		
	// tree view
	DefaultTreeModel treeModel = (DefaultTreeModel)getModel();
	List<DefaultMutableTreeNode> nodes = getAllNodes();
	
	for(DefaultMutableTreeNode n : nodes) {
	    if(n.getUserObject() instanceof XMBean) {
		XMBean xmbean = (XMBean) n.getUserObject();
		if (xmbean.getObjectName().equals(mbean)) {
		    treeModel.removeNodeFromParent(n);
		    treeModel.nodeStructureChanged(n.getParent());
		}
	    }
	}
    }
	

    /**
     * This method returns an array of all selected XMBean nodes.
     * If a sub-directory is selected, any tests contained within 
     * that directory are considered as selected.
     * IF Selection model is SINGLE_TREE_SELECTION then returns 
     * only THE Selected MBean
     * 
     */
    public List<XMBean> getSelectedXMBeans() {
	ArrayList<XMBean> selectedNodes = new ArrayList<XMBean>(5);
	DefaultMutableTreeNode node;
	TreePath[] selectedPaths = getSelectionPaths();
	// check if ANY tests are selected
	if (selectedPaths==null)
	    return selectedNodes;
	for(TreePath path : selectedPaths) {
	    node = getNode(path);
	    if (node.getUserObject() instanceof XMBean) {
		selectedNodes.add((XMBean)node.getUserObject());
	    }
	    else if (getSelectionModel().getSelectionMode() != 
		     TreeSelectionModel.SINGLE_TREE_SELECTION ) {
		// if a directory is selected, and not expanded, 
		//add all subnodes which are xmbeans
		if (!isExpanded(path)) {
		    List<XMBean> t = getAllXMBeans(node);
		    for(XMBean m : t)
			selectedNodes.add(m);
		}
	    }
	}
	return selectedNodes;
    }

    public List<XMBean> getSelectedXMBeans(TreeSelectionEvent e) {
	TreePath[] allpaths = e.getPaths();
	ArrayList<XMBean> xmbeans = new ArrayList<XMBean>(5);
	DefaultMutableTreeNode node;
	for(TreePath path : allpaths) {
	    if (e.isAddedPath(path)) {
		node = getNode(path);
		if (node.getUserObject() instanceof XMBean) {
		    xmbeans.add((XMBean)node.getUserObject());
		}
		else if (getSelectionModel().getSelectionMode() != 
			 TreeSelectionModel.SINGLE_TREE_SELECTION ) {
		    // if a directory is selected, and not expanded, 
		    // add all subnodes which are xmbeans
		    if (!isExpanded(path)) {
			//to do
		    }
		}
	    }
	}
	return xmbeans;
    }

    //returns selected xmbean paths
    public List<TreePath> getSelectedXMBeanPaths() {
	ArrayList<TreePath> paths = new ArrayList<TreePath>(5);
	DefaultMutableTreeNode node;
	TreePath[] selectedPaths = getSelectionPaths();
	if (selectedPaths==null)
	    return paths;

	for(TreePath path : paths) {
	    node = getNode(path);
	    if (node.getUserObject() instanceof XMBean) {
		paths.add(path);
	    }
	    else if (getSelectionModel().getSelectionMode() != 
		     TreeSelectionModel.SINGLE_TREE_SELECTION ) {
		// if a directory is selected, and not expanded, 
		// add all subnodes which are xmbeans
		if (!isExpanded(path)) {
		    //to do
		}
	    }
	}
	return paths;
    }

    //returns selected xmbean paths from the received tree selection event
    public List<TreePath> getSelectedXMBeanPaths(TreeSelectionEvent e) {
	TreePath[] allpaths = e.getPaths();
	ArrayList<TreePath> paths = new ArrayList<TreePath>(5);
	DefaultMutableTreeNode node;
	for(TreePath path : allpaths) {
	    if (e.isAddedPath(path)) {
		node = getNode(path);
		if (node.getUserObject() instanceof XMBean) {
		    paths.add(path);
		}
		else if (getSelectionModel().getSelectionMode() != 
			 TreeSelectionModel.SINGLE_TREE_SELECTION ) {
		    // if a directory is selected, and not expanded, 
		    // add all subnodes which are xmbeans
		    if (!isExpanded(path)) {
			//to do
		    }
		}
	    }
	}
	return paths;
    }
	
    public void selectXMBean(XMBean mb) {
	List<DefaultMutableTreeNode> all = getAllNodes(getRoot());
	XMBean curr;
	for(DefaultMutableTreeNode n : all) {
	    curr = (XMBean) n.getUserObject();
	    if(mb.equals(curr))
		selectNode(n);
	    //Should you call return?
	}
    }
	
    public void unSelectXMBean(XMBean mb) {
	List<DefaultMutableTreeNode> all = getAllNodes(getRoot());
	XMBean curr;
	for(DefaultMutableTreeNode n : all) {
	    curr = (XMBean) n.getUserObject();
	    if(mb.equals(curr))
		unselectNode(n);
	    //Should you call return?
	}
    }
	
    public void unSelectMBean(ObjectName on) {
	List<DefaultMutableTreeNode> all = getAllNodes(getRoot());
	ObjectName curr;
	for(DefaultMutableTreeNode n : all) {
	    curr = ((XMBean) n.getUserObject()).getObjectName();
	    if(on.equals(curr))
		unselectNode(n);
	    return;
	}
    }
	
    /**
     * This method returns the DefaultMutableTreeNode matching the
     * TreePath parameter.
     */
    public DefaultMutableTreeNode getNode(TreePath tree) {
	return (DefaultMutableTreeNode)tree.getLastPathComponent();
    }
	
    public void selectNode(DefaultMutableTreeNode node) {
	addSelectionPath(getTreePath(node));		
    }
	
    public void unselectNode(DefaultMutableTreeNode node) {
	removeSelectionPath(getTreePath(node));		
    }

    private TreePath markedPath = null;

    public void markNode(DefaultMutableTreeNode markedNode) {
	this.markedPath = new TreePath(markedNode.getPath());
	((DefaultTreeModel)getModel()).nodeChanged(markedNode);
    }

    public void unmarkNodes() {
	if (markedPath != null) {
	    TreeNode markedNode = (TreeNode) markedPath.getLastPathComponent();
	    ((DefaultTreeModel)getModel()).nodeChanged(markedNode);
	    markedPath = null;
	}
    }

    public boolean isMarkedRow(int row) {
	return (row == getRowForPath(markedPath));
    }
	
	
    /**
     * This method returns all the XMBean nodes in the tree.
     * @return a List containing all the XMBean objects in this tree.
     */
    public List<DefaultMutableTreeNode> getAllNodes() {
	return getAllNodes(getRoot());
    }
	
    /**
     * This method returns all the TestTreeNode starting at the node parameter.
     * @param node - the root to start building the test list from.
     * @return a List containing all the TestTreeNode objects in this tree.
     */
    public List<DefaultMutableTreeNode> 
	getAllNodes(DefaultMutableTreeNode node) {
	ArrayList<DefaultMutableTreeNode> result = 
	    new ArrayList<DefaultMutableTreeNode>(node.getChildCount());
	
	for (Enumeration e = node.depthFirstEnumeration(); 
	     e.hasMoreElements();) {
	    node = (DefaultMutableTreeNode)e.nextElement();
	    result.add(node);
	}
	
	return result;
    }
    
    /**
     * This method returns all the XMBeans starting at the root.
     * @return a List containing all the TestTreeNode objects in this tree.
     */
    public List<XMBean> getAllXMBeans() {
	return getAllXMBeans(getRoot());
    }
	
    /**
     * This method returns all the XMBean starting at the node parameter.
     * @param node - the root to start building the test list from.
     * @return a List containing all the TestTreeNode objects in this tree.
     */
    public List<XMBean> getAllXMBeans(DefaultMutableTreeNode node) {
	ArrayList<XMBean> result = new ArrayList<XMBean>(node.getChildCount());
	for (Enumeration e = node.depthFirstEnumeration(); 
	     e.hasMoreElements();) {
	    node = (DefaultMutableTreeNode)e.nextElement();
	    if (node.getUserObject() instanceof XMBean)
		result.add((XMBean) node.getUserObject());
	}
	return result;
    }
	
    /**
     * This method returns all the DefaultMutableTreeNode which contain 
     * XMBeans.
     * @return a List containing all the DefaultMutableTreeNode objects 
     * in this tree.
     */
    public List<DefaultMutableTreeNode> getAllXMBeanNodes() {
	return getAllXMBeanNodes(getRoot());
    }
	
    /**
     * This method returns all the DefaultMutableTreeNode which contain 
     * XMBeans starting at the node parameter.
     * @param node - the root to start building the test list from.
     * @return a List containing all the DefaultMutableTreeNode objects 
     * in this tree.
     */
    public List<DefaultMutableTreeNode> 
	getAllXMBeanNodes(DefaultMutableTreeNode node) {
	ArrayList<DefaultMutableTreeNode> result = 
	    new ArrayList<DefaultMutableTreeNode>(node.getChildCount());
	for (Enumeration e = node.depthFirstEnumeration(); 
	     e.hasMoreElements();) {
	    node = (DefaultMutableTreeNode)e.nextElement();
	    if (node.getUserObject() instanceof XMBean)
		result.add(node);
	}
	return result;
    }
	
    /**
     * This method removes all the displayed nodes from the tree,
     * but does not affect actual MBS contents.
     */
    public void removeAll() {	
	getRoot().removeAllChildren();
	DefaultTreeModel treeModel = (DefaultTreeModel)getModel();
	treeModel.nodeStructureChanged(getRoot());
    } 
    
    /*
     * Returns the tree path corresponding to the node.
     */
    public TreePath getTreePath(DefaultMutableTreeNode node) {
	return new TreePath(node.getPath());
    }		
	
    /**
     * Sets all of the XMBeans in this MBeanServer to selected.
     */
    public void selectAll() {
	setSelectionInterval(0,getRowCount());
    }
	
    /**
     * Sets all of the XMBeans in this MBeanServer to selected.
     */
    public void unSelectAll() {
	if (getSelectedCount()>0) {
	    super.clearSelection();
	}
    }
	
    /**
     * Inverts the currently selected MBeans (selected become unselected, 
     * and unselected become
     * selected.
     */
    public void invertSelection() {
	//todo
    }
	
    /**
     * This method returns the list of selected XMBeans in this mbean server.
     */
    public java.util.List getSelected() {
	return getSelectedXMBeans();
    }
	
    public int getSelectedCount() {
	return getSelected().size();
    }
	
    /**
     * Returns true if more than one MBean is selected simultaneously.
     */
    public boolean isMultiSelect() {
	return getSelectedCount()>1;
    }
	
    /**
     * This method will ensure the node is expanded, and visible in the 
     * displayed tree.
     */
    public void setNodeVisible(DefaultMutableTreeNode node) {
	// ensure it's visible
	scrollPathToVisible(new TreePath(node.getPath()));
    }
	
    /**
     * This method returns the DefaultMutableTreeNode containing the XMBean 
     * for the specified ObjectName, or null if it was not found in the tree.
     */
    public DefaultMutableTreeNode getXMBean(ObjectName objectName) {
	List<DefaultMutableTreeNode> all = getAllXMBeanNodes();
	ObjectName currObjectName;
	for(DefaultMutableTreeNode currNode : all) {
	    currObjectName =((XMBean)currNode.getUserObject()).getObjectName();
	    if (objectName.equals(currObjectName))
		return currNode;
	}
	return null;
    }
	
    public void addXMBeanListener(NotificationListener nl) {
	notificationListenersList.add(nl);
    }
	
    public void removeXMBeanListener(NotificationListener nl) {
	notificationListenersList.remove(nl);
    }
	
    public void fireChangedNotification(String type, Object handback) {
	Notification e = new Notification(type,this,0);
	for(NotificationListener nl : notificationListenersList)
	    nl.handleNotification(e,handback);
    }

    /* to be called when an XMBean value has changed */
    public void notifyXMBeanValueUpdate(ObjectName objectName, XMBean mbean) {
	//refresh the tree node
	DefaultMutableTreeNode node = getXMBean(objectName);
	if (node != null) {
	    ((DefaultTreeModel)getModel()).nodeChanged(node);
	    fireChangedNotification(XMBeanTree.VALUE_UPDATE_EVENT,mbean);
	}
    }
	
    public void valueChanged(TreeSelectionEvent e) {
	unmarkNodes();
	List paths =  getSelectedXMBeanPaths(e);
	fireChangedNotification(XMBeanTree.SELECTION_CHANGED_EVENT,paths);
    }
    
    /* tree expansion listener implementation */
    
    public void treeCollapsed(TreeExpansionEvent event) {
	TreePath path = event.getPath();
	deleteExpandedPath(path);
    }

    public void treeExpanded(TreeExpansionEvent event) {
	TreePath path = event.getPath();
	saveExpandedPath(path);
    }

    //call getExpandedPathKey
    private void deleteExpandedPath(TreePath path) {
	Object pathKey = getExpandedPathKey(path);
	expandedPaths.remove(pathKey);
    }

    //call getExpandedPathKey
    private void saveExpandedPath(TreePath path) {
	Object pathKey = getExpandedPathKey(path);
	expandedPaths.put(pathKey,"");
    }

    //call getExpandedPathKey
    private boolean isExpandedPath(TreePath path) {
	Object pathKey = getExpandedPathKey(path);
	return (expandedPaths.containsKey(pathKey));
    }

    /** 
     * This method can be re-implemented so as to assign a new key to the 
     * expanded paths that will be used when nodes 
     * will re-appear after a re-connection (in case some node values have 
     * changed)
     * @return the path.toString() as key
     */
    protected Object getExpandedPathKey(TreePath path) {
	return path.toString();
    }	
	
    /**
     * This method is called by addChildNode or changeNodeValue to 
     * re-expand a path 
     * that was previously expanded
     */
    protected void expandNodePath(TreePath path) {
	if (isExpandedPath(path)) {
	    TreePath parentPath = path;
	    boolean toExpand = true;
	    while (toExpand && (parentPath.getPathCount() > 2)) {
		parentPath = parentPath.getParentPath();
		if (!(isExpandedPath(parentPath))) {
		    toExpand = false;
		}
	    }
	    if (toExpand) {
		expandPath(path);
	    }
	}
    }
}
