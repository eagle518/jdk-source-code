/*
 * @(#)DirectoryTreeNode.java	1.5 03/12/23 16:38:39
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

import java.util.*;

public interface DirectoryTreeNode {
    public boolean isFile();
    public boolean isDirectory();
    public String getName();
    public String getParent();
    public Iterator getChildren() throws IllegalArgumentException;
    public int getNumChildren() throws IllegalArgumentException;
    public DirectoryTreeNode getChild(int i)
	throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
}
