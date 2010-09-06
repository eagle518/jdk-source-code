/**
 * @(#)JavadocMemberEnter.java	1.5 04/05/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.code.Flags;
import com.sun.tools.javac.code.Kinds;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.comp.MemberEnter;
import com.sun.tools.javac.tree.Tree.*;

/**
 *  Javadoc's own memberEnter phase does a few things above and beyond that
 *  done by javac.
 *  @author Neal Gafter
 */
class JavadocMemberEnter extends MemberEnter {
    public static JavadocMemberEnter instance0(Context context) {
	MemberEnter instance = context.get(memberEnterKey);
	if (instance == null) 
	    instance = new JavadocMemberEnter(context);
	return (JavadocMemberEnter)instance;
    }

    public static void preRegister(final Context context) {
        context.put(memberEnterKey, new Context.Factory<MemberEnter>() {
	       public MemberEnter make() {
		   return new JavadocMemberEnter(context);
	       }
        });
    }

    final DocEnv docenv;

    protected JavadocMemberEnter(Context context) {
	super(context);
	docenv = DocEnv.instance(context);
    }

    public void visitMethodDef(MethodDef tree) {
	super.visitMethodDef(tree);
	MethodSymbol meth = (MethodSymbol)tree.sym;
	if (meth == null || meth.kind != Kinds.MTH) return;
	String docComment = env.toplevel.docComments.get(tree);
	if (meth.isConstructor())
	    docenv.makeConstructorDoc(meth, docComment, tree);
	else if (isAnnotationTypeElement(meth))
	    docenv.makeAnnotationTypeElementDoc(meth, docComment, tree);
	else
	    docenv.makeMethodDoc(meth, docComment, tree);
    }

    public void visitVarDef(VarDef tree) {
	super.visitVarDef(tree);
	if (tree.sym != null &&
		tree.sym.kind == Kinds.VAR &&
		!isParameter(tree.sym)) {
	    String docComment = env.toplevel.docComments.get(tree);
	    docenv.makeFieldDoc((VarSymbol)tree.sym, docComment, tree);
	}
    }

    private static boolean isAnnotationTypeElement(MethodSymbol meth) {
	return ClassDocImpl.isAnnotationType(meth.enclClass());
    }

    private static boolean isParameter(VarSymbol var) {
	return (var.flags() & Flags.PARAMETER) != 0;
    }
}
