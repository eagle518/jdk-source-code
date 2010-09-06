/*
 * @(#)DirectoryTree.java	1.8 03/12/23 16:38:39
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/** Encapsulates a notion of a directory tree. Designed to allow fast
    querying of full paths for unique filenames in the hierarchy. */

import java.io.*;
import java.util.*;

public class DirectoryTree {

    /** The root of the read directoryTree */
    private Node rootNode;

    /** Subdirs to ignore; Vector of Strings */
    private Vector subdirsToIgnore;

    /** This maps file names to Lists of nodes. */
    private Hashtable nameToNodeListTable;

    /** Output "."'s as directories are read. Defaults to false. */
    private boolean verbose;

    public DirectoryTree() {
	subdirsToIgnore = new Vector();
	verbose = false;
    }

    /** Takes an absolute path to the root directory of this
        DirectoryTree. Throws IllegalArgumentException if the given
        string represents a plain file or nonexistent directory. */

    public DirectoryTree(String baseDirectory) {
	this();
	readDirectory(baseDirectory);
    }

    public void addSubdirToIgnore(String subdir) {
	subdirsToIgnore.add(subdir);
    }

    /** Output "."'s to System.out as directories are read. Defaults
        to false. */
    public void setVerbose(boolean newValue) {
	verbose = newValue;
    }

    public boolean getVerbose() {
	return verbose;
    }

    /** Takes an absolute path to the root directory of this
        DirectoryTree. Throws IllegalArgumentException if the given
        string represents a plain file or nonexistent directory. */

    public void readDirectory(String baseDirectory)
	throws IllegalArgumentException {
	File root = new File(baseDirectory);
	if (!root.isDirectory()) {
	    throw new IllegalArgumentException("baseDirectory \"" +
					       baseDirectory +
					       "\" does not exist or " +
					       "is not a directory");
	}
	try {
	    root = root.getCanonicalFile();
	}
	catch (IOException e) {
	    throw new RuntimeException(e.toString());
	}
	rootNode = new Node(root);
	readDirectory(rootNode, root);
    }

    /** Queries the DirectoryTree for a file or directory name. Takes
	only the name of the file or directory itself (i.e., no parent
	directory information should be in the passed name). Returns a
	List of DirectoryTreeNodes specifying the full paths of all of
	the files or directories of this name in the DirectoryTree.
	Returns null if the directory tree has not been read from disk
	yet or if the file was not found in the tree. */
    public List findFile(String name) {
	if (rootNode == null) {
	    return null;
	}

	if (nameToNodeListTable == null) {
	    nameToNodeListTable = new Hashtable();
	    try {
		buildNameToNodeListTable(rootNode);
	    } catch (IOException e) {
		e.printStackTrace();
		return null;
	    }
	}

	return (List) nameToNodeListTable.get(name);
    }
    
    private void buildNameToNodeListTable(Node curNode)
      throws IOException {
	String fullName = curNode.getName();
	String parent = curNode.getParent();
	String separator = System.getProperty("file.separator");

        if (parent != null) {
          if (!fullName.startsWith(parent)) {
	    throw new RuntimeException(
	        "Internal error: parent of file name \"" + fullName +
		"\" does not match file name \"" + parent + "\""
	    );
          }

          int len = parent.length();
          if (!parent.endsWith(separator)) {
	    len += separator.length();
          }
	
          String fileName = fullName.substring(len);

          if (fileName == null) {
	    throw new RuntimeException(
	        "Internal error: file name was empty"
	    );
          }

          List nodeList = (List) nameToNodeListTable.get(fileName);
          if (nodeList == null) {
	    nodeList = new Vector();
	    nameToNodeListTable.put(fileName, nodeList);
          }
	
          nodeList.add(curNode);
        } else {
          if (curNode != rootNode) {
            throw new RuntimeException(
                "Internal error: parent of file + \"" + fullName + "\"" +
                " was null"
            );
          }
        }

	if (curNode.isDirectory()) {
          Iterator iter = curNode.getChildren();
          if (iter != null) {
            while (iter.hasNext()) {
              buildNameToNodeListTable((Node) iter.next());
            }
          }
	}
    }

    /** Reads all of the files in the given directory and adds them as
        children of the directory tree node. Requires that the passed
        node represents a directory. */

    private void readDirectory(Node parentNode, File parentDir) {
	File[] children = parentDir.listFiles();
	if (children == null)
	    return;
	if (verbose) {
	    System.out.print(".");
	    System.out.flush();
	}
	for (int i = 0; i < children.length; i++) {
	    File child = children[i];
	    children[i] = null;
	    boolean isDir = child.isDirectory();
	    boolean mustSkip = false;
	    if (isDir) {
		for (Iterator iter = subdirsToIgnore.iterator();
		     iter.hasNext(); ) {
		    if (child.getName().equals((String) iter.next())) {
			mustSkip = true;
			break;
		    }
		}
	    }
	    if (!mustSkip) {
		Node childNode = new Node(child);
		parentNode.addChild(childNode);
		if (isDir) {
		    readDirectory(childNode, child);
		}
	    }
	}
    }

    private class Node implements DirectoryTreeNode {
	private File file;
	private Vector children;
	
	/** file must be a canonical file */
	Node(File file) {
	    this.file = file;
	    children = new Vector();
	}

	public boolean isFile() {
	    return file.isFile();
	}

	public boolean isDirectory() {
	    return file.isDirectory();
	}

	public String getName() {
	    return file.getPath();
	}

	public String getParent() {
	    return file.getParent();
	}

	public void addChild(Node n) {
	    children.add(n);
	}

	public Iterator getChildren() throws IllegalArgumentException {
	    return children.iterator();
	}

	public int getNumChildren() throws IllegalArgumentException {
	    return children.size();
	}
	
	public DirectoryTreeNode getChild(int i)
	    throws IllegalArgumentException, ArrayIndexOutOfBoundsException {
	    return (DirectoryTreeNode) children.get(i);
	}
    }
}
