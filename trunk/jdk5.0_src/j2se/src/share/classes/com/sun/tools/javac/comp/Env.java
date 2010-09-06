/**
 * @(#)Env.java	1.17 04/05/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.comp;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.tree.*;

/** A class for environments, instances of which are passed as
 *  arguments to tree visitors.  Environments refer to important ancestors
 *  of the subtree that's currently visited, such as the enclosing method,
 *  the enclosing class, or the enclosing toplevel node. They also contain
 *  a generic component, represented as a type parameter, to carry further
 *  information specific to individual passes.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Env<A> {

    /** The next enclosing environment.
     */
    public Env<A> next;

    /** The environment enclosing the current class.
     */
    public Env<A> outer;

    /** The tree with which this environment is associated.
     */
    public Tree tree;

    /** The enclosing toplevel tree.
     */
    public Tree.TopLevel toplevel;

    /** The next enclosing class definition.
     */
    public Tree.ClassDef enclClass;

    /** The next enclosing method definition.
     */
    public Tree.MethodDef enclMethod;

    /** A generic field for further information.
     */
    public A info;

    /** Is this an environment for evaluating a base clause?
     */
    public boolean baseClause = false;

    /** Create an outermost environment for a given (toplevel)tree,
     *  with a given info field.
     */
    public Env(Tree tree, A info) {
	this.next = null;
	this.outer = null;
	this.tree = tree;
	this.toplevel = null;
	this.enclClass = null;
	this.enclMethod = null;
	this.info = info;
    }

    /** Duplicate this environment, updating with given tree and info,
     *  and copying all other fields.
     */
    public Env<A> dup(Tree tree, A info) {
	return dupto(new Env<A>(tree, info));
    }

    /** Duplicate this environment into a given Environment,
     *  using its tree and info, and copying all other fields.
     */
    public Env<A> dupto(Env<A> that) {
	that.next = this;
	that.outer = this.outer;
	that.toplevel = this.toplevel;
 	that.enclClass = this.enclClass;
	that.enclMethod = this.enclMethod;
	return that;
    }

    /** Duplicate this environment, updating with given tree,
     *  and copying all other fields.
     */
    public Env<A> dup(Tree tree) {
	return dup(tree, this.info);
    }

    /** Return closest enclosing environment which points to a tree with given tag.
     */
    public Env<A> enclosing(int tag) {
	Env<A> env1 = this;
	while (env1 != null && env1.tree.tag != tag) env1 = env1.next;
	return env1;
    }
}

