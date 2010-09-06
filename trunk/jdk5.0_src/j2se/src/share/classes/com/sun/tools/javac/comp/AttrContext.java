/**
 * @(#)AttrContext.java	1.19 04/04/15
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.comp;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.tree.*;

import com.sun.tools.javac.code.Symbol.*;

/** Contains information specific to the attribute and enter
 *  passes, to be used in place of the generic field in environments.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class AttrContext {

    /** The scope of local symbols.
     */
    Scope scope = null;

    /** The number of enclosing `static' modifiers.
     */
    int staticLevel = 0;

    /** Is this an environment for a this(...) or super(...) call?
     */
    boolean isSelfCall = false;

    /** Are we evaluating the selector of a `super' or type name?
     */
    boolean selectSuper = false;

    /** Are arguments to current function applications boxed into an array for varargs?
     */
    boolean varArgs = false;

    /** A list of type variables that are all-quantifed in current context.
     */
    List<Type> tvars = Type.emptyList;

    /** Duplicate this context, replacing scope field and copying all others.
     */
    AttrContext dup(Scope scope) {
	AttrContext info = new AttrContext();
	info.scope = scope;
	info.staticLevel = staticLevel;
	info.isSelfCall = isSelfCall;
	info.selectSuper = selectSuper;
	info.varArgs = varArgs;
	info.tvars = tvars;
	return info;
    }

    /** Duplicate this context, copying all fields.
     */
    AttrContext dup() {
	return dup(scope);
    }
}

