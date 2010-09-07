/*
 * @(#)ResourceType.java	1.11 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;
import com.sun.deploy.xml.XMLable;

/*
 * Public super class for all resource entries
 */
interface ResourceType extends XMLable {
    /** Visit this specific type */
    void visit(ResourceVisitor visitor);
}
