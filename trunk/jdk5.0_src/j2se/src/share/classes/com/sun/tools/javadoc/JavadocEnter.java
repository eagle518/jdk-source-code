/**
 * @(#)JavadocEnter.java	1.12 04/05/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.List;
import com.sun.tools.javac.code.Kinds;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.comp.Enter;
import com.sun.tools.javac.tree.Tree;
import com.sun.tools.javac.tree.Tree.*;

/**
 *  Javadoc's own enter phase does a few things above and beyond that
 *  done by javac.
 *  @author Neal Gafter
 */
public class JavadocEnter extends Enter {
    public static JavadocEnter instance0(Context context) {
	Enter instance = context.get(enterKey);
	if (instance == null) 
	    instance = new JavadocEnter(context);
	return (JavadocEnter)instance;
    }

    public static void preRegister(final Context context) {
        context.put(enterKey, new Context.Factory<Enter>() {
	       public Enter make() {
		   return new JavadocEnter(context);
	       }
        });
    }

    protected JavadocEnter(Context context) {
	super(context);
	messager = Messager.instance0(context);
	docenv = DocEnv.instance(context);
    }

    final Messager messager;
    final DocEnv docenv;

    public void main(List<Tree> trees) {
	// count all Enter errors as warnings.
	int nerrors = messager.nerrors;
	super.main(trees);
	messager.nwarnings += (messager.nerrors - nerrors);
	messager.nerrors = nerrors;
    }

    public void visitTopLevel(TopLevel tree) {
	super.visitTopLevel(tree);
	if (endsWith("" + tree.sourcefile, "package-info.java")) {
	    String comment = tree.docComments.get(tree);
	    docenv.makePackageDoc(tree.packge, comment, tree);
	}
    }

    public void visitClassDef(ClassDef tree) {
	super.visitClassDef(tree);
	if (tree.sym != null && tree.sym.kind == Kinds.TYP) {
	    if (tree.sym == null) return;
	    String comment = env.toplevel.docComments.get(tree);
	    ClassSymbol c = (ClassSymbol)tree.sym;
	    docenv.makeClassDoc(c, comment, tree);
	}
    }

    /** Don't complain about a duplicate class. */
    protected void duplicateClass(int pos, ClassSymbol c) {}

}
