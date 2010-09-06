/**
 * @(#)Todo.java	1.8 04/02/06
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.comp;

import com.sun.tools.javac.util.*;

/** A queue of all as yet unattributed classes.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Todo extends ListBuffer<Env<AttrContext>> {
    /** The context key for the todo list. */
    protected static final Context.Key<Todo> todoKey =
	new Context.Key<Todo>();

    /** Get the Todo instance for this context. */
    public static Todo instance(Context context) {
	Todo instance = context.get(todoKey);
	if (instance == null)
	    instance = new Todo(context);
	return instance;
    }

    /** Create a new todo list. */
    protected Todo(Context context) {
	context.put(todoKey, this);
    }
}
