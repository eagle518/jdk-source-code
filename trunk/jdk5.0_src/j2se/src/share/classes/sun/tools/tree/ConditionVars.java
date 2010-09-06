/*
 * @(#)ConditionVars.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

/**
 * This class is used to hold two sets of variables,
 * one for the true branch, one for the false branch.
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
class ConditionVars {
    Vset vsTrue;
    Vset vsFalse;
}
