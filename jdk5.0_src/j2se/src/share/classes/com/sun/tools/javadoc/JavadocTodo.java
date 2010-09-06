/**
 * @(#)JavadocTodo.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import com.sun.tools.javac.comp.*;
import com.sun.tools.javac.util.*;

/**
 *  Javadoc's own todo queue doesn't queue its inputs, as javadoc
 *  doesn't perform attribution of method bodies or semantic checking.
 *  @author Neal Gafter
 */
public class JavadocTodo extends Todo {
    public static void preRegister(final Context context) {
        context.put(todoKey, new Context.Factory<Todo>() {
	       public Todo make() {
		   return new JavadocTodo(context);
	       }
        });
    }

    protected JavadocTodo(Context context) {
	super(context);
    }

    public ListBuffer<Env<AttrContext>> append(Env<AttrContext> e) {
	// do nothing; Javadoc doesn't perform attribution.
	return this;
    }
}
