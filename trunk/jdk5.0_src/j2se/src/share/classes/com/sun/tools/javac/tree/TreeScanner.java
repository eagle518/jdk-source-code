/**
 * @(#)TreeScanner.java	1.17 04/01/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.tree;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.tree.Tree.*;

/** A subclass of Tree.Visitor, this class defines
 *  a general tree scanner pattern. Translation proceeds recursively in
 *  left-to-right order down a tree. There is one visitor method in this class
 *  for every possible kind of tree node.  To obtain a specific
 *  scanner, it suffices to override those visitor methods which
 *  do some interesting work. The scanner class itself takes care of all
 *  navigational aspects.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class TreeScanner extends Visitor {

    /** Visitor method: Scan a single node.
     */
    public void scan(Tree tree) {
	if(tree!=null) tree.accept(this);
    }

    /** Visitor method: scan a list of nodes.
     */
    public void scan(List<? extends Tree> trees) {
	if (trees != null)
	for (List<? extends Tree> l = trees; l.nonEmpty(); l = l.tail)
	    scan(l.head);
    }


/* ***************************************************************************
 * Visitor methods
 ****************************************************************************/

    public void visitTopLevel(TopLevel tree) {
	scan(tree.pid);
	scan(tree.defs);
    }

    public void visitImport(Import tree) {
	scan(tree.qualid);
    }

    public void visitClassDef(ClassDef tree) {
	scan(tree.mods);
	scan(tree.typarams);
	scan(tree.extending);
	scan(tree.implementing);
	scan(tree.defs);
    }

    public void visitMethodDef(MethodDef tree) {
	scan(tree.mods);
	scan(tree.restype);
	scan(tree.typarams);
	scan(tree.params);
	scan(tree.thrown);
	scan(tree.body);
    }
	
    public void visitVarDef(VarDef tree) {
	scan(tree.mods);
	scan(tree.vartype);
	scan(tree.init);
    }
	
    public void visitSkip(Skip tree) {
    }

    public void visitBlock(Block tree) {
	scan(tree.stats);
    }

    public void visitDoLoop(DoLoop tree) {
	scan(tree.body);
	scan(tree.cond);
    }

    public void visitWhileLoop(WhileLoop tree) {
	scan(tree.cond);
	scan(tree.body);
    }

    public void visitForLoop(ForLoop tree) {
	scan(tree.init);
	scan(tree.cond);
	scan(tree.step);
	scan(tree.body);
    }

    public void visitForeachLoop(ForeachLoop tree) {
	scan(tree.var);
	scan(tree.expr);
	scan(tree.body);
    }

    public void visitLabelled(Labelled tree) {
	scan(tree.body);
    }

    public void visitSwitch(Switch tree) {
	scan(tree.selector);
	scan(tree.cases);
    }

    public void visitCase(Case tree) {
	scan(tree.pat);
	scan(tree.stats);
    }

    public void visitSynchronized(Synchronized tree) {
	scan(tree.lock);
	scan(tree.body);
    }

    public void visitTry(Try tree) {
	scan(tree.body);
	scan(tree.catchers);
	scan(tree.finalizer);
    }

    public void visitCatch(Catch tree) {
	scan(tree.param);
	scan(tree.body);
    }

    public void visitConditional(Conditional tree) {
	scan(tree.cond);
	scan(tree.truepart);
	scan(tree.falsepart);
    }

    public void visitIf(If tree) {
	scan(tree.cond);
	scan(tree.thenpart);
	scan(tree.elsepart);
    }

    public void visitExec(Exec tree) {
	scan(tree.expr);
    }

    public void visitBreak(Break tree) {
    }

    public void visitContinue(Continue tree) {
    }

    public void visitReturn(Return tree) {
	scan(tree.expr);
    }

    public void visitThrow(Throw tree) {
	scan(tree.expr);
    }

    public void visitAssert(Assert tree) {
	scan(tree.cond);
	scan(tree.detail);
    }

    public void visitApply(Apply tree) {
	scan(tree.meth);
	scan(tree.args);
    }

    public void visitNewClass(NewClass tree) {
	scan(tree.encl);
	scan(tree.clazz);
	scan(tree.args);
	scan(tree.def);
    }

    public void visitNewArray(NewArray tree) {
	scan(tree.elemtype);
	scan(tree.dims);
	scan(tree.elems);
    }

    public void visitParens(Parens tree) {
	scan(tree.expr);
    }

    public void visitAssign(Assign tree) {
	scan(tree.lhs);
	scan(tree.rhs);
    }

    public void visitAssignop(Assignop tree) {
	scan(tree.lhs);
	scan(tree.rhs);
    }

    public void visitUnary(Unary tree) {
	scan(tree.arg);
    }

    public void visitBinary(Binary tree) {
	scan(tree.lhs);
	scan(tree.rhs);
    }

    public void visitTypeCast(TypeCast tree) {
	scan(tree.clazz);
	scan(tree.expr);
    }

    public void visitTypeTest(TypeTest tree) {
	scan(tree.expr);
	scan(tree.clazz);
    }

    public void visitIndexed(Indexed tree) {
	scan(tree.indexed);
	scan(tree.index);
    }

    public void visitSelect(Select tree) {
	scan(tree.selected);
    }

    public void visitIdent(Ident tree) {
    }

    public void visitLiteral(Literal tree) {
    }

    public void visitTypeIdent(TypeIdent tree) {
    }

    public void visitTypeArray(TypeArray tree) {
	scan(tree.elemtype);
    }

    public void visitTypeApply(TypeApply tree) {
	scan(tree.clazz);
	scan(tree.arguments);
    }

    public void visitTypeParameter(TypeParameter tree) {
	scan(tree.bounds);
    }

    public void visitTypeArgument(TypeArgument tree) {
	if (tree.inner != null)
	    scan(tree.inner);
    }

    public void visitModifiers(Modifiers tree) {
	scan(tree.annotations);
    }

    public void visitAnnotation(Annotation tree) {
	scan(tree.annotationType);
	scan(tree.args);
    }

    public void visitErroneous(Erroneous tree) {
    }

    public void visitLetExpr(LetExpr tree) {
	scan(tree.defs);
	scan(tree.expr);
    }

    public void visitTree(Tree tree) {
	assert false;
    }
}
