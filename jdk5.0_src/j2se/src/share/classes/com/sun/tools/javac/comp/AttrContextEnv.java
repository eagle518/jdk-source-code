/**
 * @(#)AttrContextEnv.java	1.8 04/01/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.comp;

import com.sun.tools.javac.tree.Tree;


/** Env<A> specialized as Env<AttrContext>
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class AttrContextEnv extends Env<AttrContext> {

    /** Create an outermost environment for a given (toplevel)tree,
     *  with a given info field.
     */
    public AttrContextEnv(Tree tree, AttrContext info) {
	super(tree, info);
    }
}
