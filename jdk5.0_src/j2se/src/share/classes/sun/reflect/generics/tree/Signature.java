/*
 * @(#)Signature.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.tree;

/**
 * Common superinterface for generic signatures. These are the signatures
 * of complete class and method/constructor delcarations.
 */
public interface Signature extends Tree{
    FormalTypeParameter[] getFormalTypeParameters();
}
