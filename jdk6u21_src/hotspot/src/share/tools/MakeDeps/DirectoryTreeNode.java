/*
 * Copyright (c) 1999, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
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
