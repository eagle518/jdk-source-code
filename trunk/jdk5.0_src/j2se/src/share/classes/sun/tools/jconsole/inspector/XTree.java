/*
 * @(#)XTree.java	1.6 04/04/13
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole.inspector;

// java import 
import java.awt.*;
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

import sun.tools.jconsole.Resources;
import sun.tools.jconsole.MBeansTab;

public class XTree extends XMBeanTree {
    private HashMap<String, DefaultMutableTreeNode> nodes;

    public XTree() {
        this(new DefaultMutableTreeNode(Resources.getText("Tree")));
    }
    
    public XTree(TreeNode root) {
	super(root);
	nodes = new HashMap<String, DefaultMutableTreeNode>();
	if (isTreeToolTip()) {
	    //Enable Tool Tip
	    ToolTipManager.sharedInstance().registerComponent(this);
	}
    }
    
    public void removeAll() {	
	super.removeAll();
	nodes.clear();
    }
    
    public synchronized void delMBeanFromView(ObjectName mbean) {
	
	//we suppose here that mbeans are removed one by one (notification)
	
	//deletes the tree node and recursively all non XMBean parent which 
	//becomes leaf
	
	DefaultMutableTreeNode node = null;
	Dn dn = buildDn(mbean);
	if (dn.size() > 0) {
	    DefaultTreeModel treeModel = (DefaultTreeModel)getModel();
	    String hashKey = dn.getHashKey(dn.getToken(0));
	    node = nodes.get(hashKey);
	    if ((node != null) && (!node.isRoot())) {
		DefaultMutableTreeNode parent = 
		    (DefaultMutableTreeNode) node.getParent();
		treeModel.removeNodeFromParent(node);
		nodes.remove(hashKey);
		DefaultMutableTreeNode lastNode = 
		    delParentFromView(dn,1,parent);
		if (lastNode != null) {
		    // treeModel.nodeStructureChanged(lastNode);
		}
	    }
	}
    }	

    // removes only parents with are non XMBean and leaf  (suppose childs 
    // have been removed before)
    protected DefaultMutableTreeNode 
	delParentFromView(Dn dn,
			  int index,
			  DefaultMutableTreeNode node) {
	if ((!node.isRoot()) && node.isLeaf() && 
	    (!(node.getUserObject() instanceof XMBean))) {
	    DefaultMutableTreeNode parent = 
		(DefaultMutableTreeNode) node.getParent();
	    removeChildNode(node);
	    String hashKey = dn.getHashKey(dn.getToken(index));
	    nodes.remove(hashKey);
	    delParentFromView(dn,index+1,parent);
	}
	return node;
    }

    //returns the node added containing the XMBean
    public  synchronized void addMBeanToView(MBeansTab mbeansTab, 
					     ObjectName mbean) 
	throws javax.management.InstanceNotFoundException {

	 //from leaf to root

	 Dn dn = buildDn(mbean);
	 if (dn.size() == 0) return;

	 Token token = dn.getToken(0);

	 DefaultMutableTreeNode node = null;
	 boolean nodeCreated = true;

	 //
	 //Add the node or replace it's user object if already added
	 //

	 String hashKey = dn.getHashKey(token);
	 
	 if (nodes.containsKey(hashKey)) {		
	     //already in the tree, means it has been created previously 
	     //when adding another node
	     node = nodes.get(hashKey);
	     //sets the user object
	     changeNodeValue(node,createNodeValue(mbean,mbeansTab,token));
	     nodeCreated = false;
	 }
	 else {
	     //create a new node
	     node = createDnNode(dn,token,mbeansTab,mbean);
	     if (node != null) {
		 nodes.put(hashKey, node);
		 nodeCreated = true;
	     }
	     else {
		 return;
	     }
	 }
	  
	 
	 //
	 //Add (vitual) nodes without user object if necessary
	 //

	 for (int i=1;i<dn.size();i++) {
	     DefaultMutableTreeNode currentNode = null;
	     token = dn.getToken(i);
	     hashKey = dn.getHashKey(token);
   
	     if (nodes.containsKey(hashKey)) {
		 //node already present
		 if (nodeCreated) {
		     //previous node created, link to do 
		      currentNode = nodes.get(hashKey);
		      addChildNode(currentNode,node);
		      return;
		 }
		 else {
		     //both nodes already presents
		     return;
		 }
	     }
	     else {
		 //creates the node that can be a vitual one		 
		 if (token.getKeyDn().equals("domain")) { 
		     //better match on keyDn that on Dn 
		     currentNode =  createDomainNode(dn,token);
		     if (currentNode != null) {
			 addChildNode(getRoot(),currentNode);
		     }
		 }
		 else {
		     currentNode = createSubDnNode(dn,token);
		     if (currentNode == null) {
			 //skip
			 continue;
		     }
		 }
		 
		 nodes.put(hashKey, currentNode);
		 addChildNode(currentNode, node);
		 nodeCreated = true;
	     }

	     node = currentNode;

	 }
     }


    //re-implementation to take into account the order

    public synchronized void changeNodeValue(DefaultMutableTreeNode node,
					     Object nodeValue) {
	//delete the node and re-orders it in case the node value modify 
	//the order in the tree
	if (node instanceof Comparable) {
	    //does it stays at the same place ?
	    DefaultMutableTreeNode clone = 
		(DefaultMutableTreeNode) node.clone();
	    clone.setUserObject(nodeValue);
	    if (((Comparable<DefaultMutableTreeNode>) node).compareTo(clone) 
		== 0) {
		 super.changeNodeValue(node,nodeValue);
	    }
	    else {
		// delete the node and re-orders it in case the node 
		// value modify the order in the tree
		DefaultMutableTreeNode parent = 
		    (DefaultMutableTreeNode) node.getParent();
		removeChildNode(node);
		node.setUserObject(nodeValue);
		addChildNode(parent,node);
	    }
	}
	else {
	    //not comparable stays at the same place
	    super.changeNodeValue(node,nodeValue);
	}
    }
	    
	

    public synchronized void addChildNode(DefaultMutableTreeNode parent, 
					  DefaultMutableTreeNode child) {
	int childCount = parent.getChildCount();
	if (childCount == 0) {
	    super.addChildNode(parent,child);
	}
	else if (child instanceof Comparable) {
	    int i=0;
	    for (i=0;i<childCount;i++) {
		DefaultMutableTreeNode brother = 
		    (DefaultMutableTreeNode) parent.getChildAt(i);
		//child < brother
		if (((Comparable<DefaultMutableTreeNode>) child).
		    compareTo(brother) < 0) {
		    super.addChildNode(parent,child,i);
		    break;
		}
		//child = brother
		else if (((Comparable<DefaultMutableTreeNode>) child).
			 compareTo(brother) == 0) {
		    super.addChildNode(parent,child,i);
		    break;
		}
	    }
	    //child < all brothers
	    if (i==childCount) {
		super.addChildNode(parent,child,childCount);
	    }
	}
	else {
	    //not comparable, add at the end
	    super.addChildNode(parent,child,childCount);
	}	    
    }

    //creates the domain node, called on a domain token
    public DefaultMutableTreeNode createDomainNode(Dn dn,Token token) {
	 DefaultMutableTreeNode currentNode = 
	     new OrderedDefaultMutableTreeNode(dn.getDomain());
	 return currentNode;
	
    }
    
    //creates the node corresponding to the whole Dn
    public DefaultMutableTreeNode createDnNode(Dn dn,
					       Token token,
					       MBeansTab mbeansTab, 
					       ObjectName mbean) 
	throws javax.management.InstanceNotFoundException {
	return new 
	    OrderedDefaultMutableTreeNode(createNodeValue(mbean,
							  mbeansTab,
							  token));
    }
    
    //creates a node with the token value, call for each non domain sub 
    //dn token
    public DefaultMutableTreeNode createSubDnNode(Dn dn,Token token) {
	return  new OrderedDefaultMutableTreeNode(token.getValue());
    }

    public Object createNodeValue(ObjectName mbean,
				  MBeansTab mbeansTab,
				  Token token) 
	throws javax.management.InstanceNotFoundException {
	String text = token.getValue();
	XMBean xmbean = new XMBean(mbean,mbeansTab);
	 if (text != null) {
	     xmbean.setText(text);
	 }
	 return xmbean;
     }


    public final Dn buildDn(ObjectName mbean) {

	String domain = mbean.getDomain();
	String globalDn = mbean.getKeyPropertyListString();

	Dn dn = buildDn(domain,globalDn,mbean);
	    
	//update the dn tokens
	updateDn(dn,mbean);

	//reverse the Dn (from leaf to root)
	dn.reverseOrder();
	
	
	//compute the hashDn
	dn.computeHashDn();
	
	return dn;
	
    }

    public Dn buildDn(String domain, String globalDn, ObjectName mbean) {

	Dn dn = new Dn(domain,globalDn);
	String keyDn = "no_key";
	if (isTreeView()) {
	    StringTokenizer tokenizer = new StringTokenizer(globalDn,",");
	    while (tokenizer.hasMoreTokens()) {
		dn.addToken(new Token(keyDn,tokenizer.nextToken()));
	    }
	}
	else {
	    //flat view 
	    dn.addToken(new Token(keyDn,"properties="+globalDn));
	}
	return dn;
    }

    //adds the domain as the first token
    public void updateDn(Dn dn, ObjectName mbean) {
	//add the domain
	dn.addToken(0,new Token("domain","domain="+dn.getDomain()));
    }


    //Node Expansion in case of rebooting (tree is removed and then rebuilt)
    //current policy is to re-expand previously expanded nodes base on node 
    //displayed paths
    //One can re-implement this methods for customizing the paths

    
     /** 
     * This method can be re-implemented so as to assign a new key to the 
     * expanded paths that will be used when nodes 
     * will re-appear after a re-connection (in case some node values have 
     * changed)
     * @return the new path.toString() as key
     */
    protected Object getExpandedPathKey(TreePath path) {
	return super.getExpandedPathKey(path); 
    }
    
    

    //called to re-expand a node that was perhaps previoulsy expanded
    protected void expandNodePath(TreePath path) {
	super.expandNodePath(path);
    }

    


    //
    //utility methods
    //
    
    public String buildHashKey(Dn dn,Token token) {
	    int begin = dn.getHashDn().indexOf(token.getHashToken());
	    return  dn.getHashDn().substring(begin,dn.getHashDn().length());
    }    


  
    //
    //utility objects
    //

    public static class OrderedDefaultMutableTreeNode 
	extends DefaultMutableTreeNode 
	implements Comparable {
	
	public OrderedDefaultMutableTreeNode(Object userObject) {
	    super(userObject);
	}

	public int compareTo(Object o) {
	    return (this.toString().compareTo(o.toString()));
	}

    }

    //
    //tree parameters
    //

    private boolean treeView;
    private boolean treeViewInit = false;
    public boolean isTreeView() {
	if (!treeViewInit) {
	    treeView = getTreeViewValue();
	    treeViewInit = true;
	}   
	return treeView;
    }

    private boolean treeToolTip;
    private boolean treeToolTipInit = false;
    public boolean isTreeToolTip() {
	if (!treeToolTipInit) {
	    treeToolTip = getTreeToolTipValue();
	    treeToolTipInit = true;
	}
	return treeToolTip;
    }

    private boolean getTreeViewValue() {
	String treeView = System.getProperties().getProperty("treeView");
	return ((treeView == null) ? true : !(treeView.equals("false")));
    }

    private boolean getTreeToolTipValue() {
	String treeToolTip = System.getProperties().getProperty("treeToolTip");
	return ((treeToolTip == null) ? false : treeToolTip.equals("true"));
    }
	    	

    public static class Dn {

	private ArrayList<Token> tokens;
	private String domain;
	private String hashDn;
	private String dn;
	
	public Dn(String domain,String dn) {
	    tokens = new ArrayList<Token>(8);
	    this.domain = domain;
	    this.dn = dn;
	}
	    
	public void clearTokens() {
	    tokens.clear();
	}

	public void addToken(Token token) {
	    tokens.add(token);
	}

	public void addToken(int index,Token token) {
	    tokens.add(index,token);
	}

	public void setToken(int index,Token token) {
	    tokens.set(index,token);
	}

	public void removeToken(int index) {
	    tokens.remove(index);
	}

	public Token getToken(int index) {
	    return ((Token) tokens.get(index));
	}

	public void reverseOrder() {
	    ArrayList<Token> newOrder = new ArrayList<Token>(tokens.size());
	    for (int i=tokens.size()-1;i>=0;i--) {
		newOrder.add(tokens.get(i));
	    }
	    tokens = newOrder;
	}

	public int size() {
	    return tokens.size();
	}

	public String getDomain() {
	    return domain;
	}

	public String getDn() {
	    return dn;
	}

	public String getHashDn() {
	    return hashDn;
	}    

	public String getHashKey(Token token) {
	    int begin = getHashDn().indexOf(token.getHashToken());
	    return  getHashDn().substring(begin,getHashDn().length());
	}

	//hashDn
	public void computeHashDn() {
	    StringBuffer hashDn = new StringBuffer();
	    for (int i=0;i<tokens.size();i++) {
		Token token = (Token) tokens.get(i);
		String hashToken = token.getHashToken();
		if (hashToken == null) {
		    hashToken = token.getToken()+""+(tokens.size()-i);
		    token.setHashToken(hashToken);
		}
		hashDn.append(hashToken);
		hashDn.append(",");
	    }
	    if (tokens.size() > 0) {
		this.hashDn = hashDn.substring(0,hashDn.length()-1);
	    }
	    else {
		this.hashDn = "";
	    }
	}
	    

	public String toString() {
	    return tokens.toString();
	}

    }
	    
	    
	

    public static class Token {

	private String keyDn;
	private String token;
	private String hashToken;
	private String key;
	private String value;

	public Token(String keyDn,String token) {
	    this.keyDn = keyDn;
	    this.token = token;
	    buildKeyValue();
	}   

	public Token(String keyDn,String token,String hashToken) {
	    this.keyDn = keyDn;
	    this.token = token;
	    this.hashToken = hashToken;
	    buildKeyValue();
	}

	public String getKeyDn() {
	    return keyDn;
	}

	public String getToken() {
	    return token;
	}

	public void setValue(String value) {
	    this.value = value;
	    this.token = key+"="+value;
	}
	
	public void setKey(String key) {
	    this.key = key;
	    this.token = key+":"+value;
	}

	public void setKeyDn(String keyDn) {
	    this.keyDn = keyDn;
	}

	public  void setHashToken(String hashToken) {
	    this.hashToken = hashToken;
	}

	public String getHashToken() {
	    return hashToken;
	}

	public String getKey() {
	    return key;
	}

	public String getValue() {
	    return value;
	}
		

	public String toString(){
	    return getToken();
	}

	public boolean equals(Object object) {
	    if (object instanceof Token) {
		return token.equals(((Token) object));
	    }
	    else {
		return false;
	    }
	}
				    
		

	private void buildKeyValue() {
	    int index = token.indexOf("=");
	    if (index < 0) {
		key = token;
		value = token;
	    }
	    else {
		key = token.substring(0,index);
		value = token.substring(index+1,token.length());
	    }
	}
    }
}

    
    
	

    
