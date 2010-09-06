/**
 * @(#)TreeTranslator.java	1.30 04/01/09
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
 *  a general tree translator pattern. Translation proceeds recursively in
 *  left-to-right order down a tree, constructing translated nodes by
 *  overwriting existing ones. There is one visitor method in this class
 *  for every possible kind of tree node.  To obtain a specific
 *  translator, it suffices to override those visitor methods which
 *  do some interesting work. The translator class itself takes care of all
 *  navigational aspects.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class TreeTranslator extends Tree.Visitor {

    /** Visitor result field: a tree
     */
    protected Tree result;

    /** Visitor method: Translate a single node.
     */
    public Tree translate(Tree tree) {
	if (tree == null) {
	    return null;
	} else {
	    tree.accept(this);
	    Tree result = this.result;
	    this.result = null;
	    return result;
	}
    }

    /** Visitor method: translate a list of nodes.
     */
    public List<Tree> translate(List<Tree> trees) {
	if (trees == null) return null;
	for (List<Tree> l = trees; l.nonEmpty(); l = l.tail)
	    l.head = translate(l.head);
	return trees;
    }


    /**  Visitor method: translate a list of variable definitions.
     */
    public List<VarDef> translateVarDefs(List<VarDef> trees) {
	for (List<VarDef> l = trees; l.nonEmpty(); l = l.tail)
	    l.head = (VarDef)translate(l.head);
	return trees;
    }

    /**  Visitor method: translate a list of type parameters.
     */
    public List<TypeParameter> translateTypeParams(List<TypeParameter> trees) {
	for (List<TypeParameter> l = trees; l.nonEmpty(); l = l.tail)
	    l.head = (TypeParameter)translate(l.head);
	return trees;
    }

    /**  Visitor method: translate a list of case parts of switch statements.
     */
    public List<Case> translateCases(List<Case> trees) {
	for (List<Case> l = trees; l.nonEmpty(); l = l.tail)
	    l.head = (Case)translate(l.head);
	return trees;
    }

    /**  Visitor method: translate a list of catch clauses in try statements.
     */
    public List<Catch> translateCatchers(List<Catch> trees) {
	for (List<Catch> l = trees; l.nonEmpty(); l = l.tail)
	    l.head = (Catch)translate(l.head);
	return trees;
    }

    /**  Visitor method: translate a list of catch clauses in try statements.
     */
    public List<Annotation> translateAnnotations(List<Annotation> trees) {
	for (List<Annotation> l = trees; l.nonEmpty(); l = l.tail)
	    l.head = (Annotation)translate(l.head);
	return trees;
    }

/* ***************************************************************************
 * Visitor methods
 ****************************************************************************/

    public void visitTopLevel(TopLevel tree) {
	tree.pid = translate(tree.pid);
	tree.defs = translate(tree.defs);
	result = tree;
    }

    public void visitImport(Import tree) {
	tree.qualid = translate(tree.qualid);
	result = tree;
    }

    public void visitClassDef(ClassDef tree) {
	tree.mods = (Modifiers)translate(tree.mods);
	tree.typarams = translateTypeParams(tree.typarams);
	tree.extending = translate(tree.extending);
	tree.implementing = translate(tree.implementing);
	tree.defs = translate(tree.defs);
	result = tree;
    }

    public void visitMethodDef(MethodDef tree) {
	tree.mods = (Modifiers)translate(tree.mods);
	tree.restype = translate(tree.restype);
	tree.typarams = translateTypeParams(tree.typarams);
	tree.params = translateVarDefs(tree.params);
	tree.thrown = translate(tree.thrown);
	tree.body = (Block)translate(tree.body);
	result = tree;
    }
	
    public void visitVarDef(VarDef tree) {
	tree.mods = (Modifiers)translate(tree.mods);
	tree.vartype = translate(tree.vartype);
	tree.init = translate(tree.init);
	result = tree;
    }
	
    public void visitSkip(Skip tree) {
	result = tree;
    }

    public void visitBlock(Block tree) {
	tree.stats = translate(tree.stats);
	result = tree;
    }

    public void visitDoLoop(DoLoop tree) {
	tree.body = translate(tree.body);
	tree.cond = translate(tree.cond);
	result = tree;
    }

    public void visitWhileLoop(WhileLoop tree) {
	tree.cond = translate(tree.cond);
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitForLoop(ForLoop tree) {
	tree.init = translate(tree.init);
	tree.cond = translate(tree.cond);
	tree.step = translate(tree.step);
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitForeachLoop(ForeachLoop tree) {
	tree.var = (VarDef)translate(tree.var);
	tree.expr = translate(tree.expr);
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitLabelled(Labelled tree) {
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitSwitch(Switch tree) {
	tree.selector = translate(tree.selector);
	tree.cases = translateCases(tree.cases);
	result = tree;
    }

    public void visitCase(Case tree) {
	tree.pat = translate(tree.pat);
	tree.stats = translate(tree.stats);
	result = tree;
    }

    public void visitSynchronized(Synchronized tree) {
	tree.lock = translate(tree.lock);
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitTry(Try tree) {
	tree.body = translate(tree.body);
	tree.catchers = translateCatchers(tree.catchers);
	tree.finalizer = translate(tree.finalizer);
	result = tree;
    }

    public void visitCatch(Catch tree) {
	tree.param = (VarDef)translate(tree.param);
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitConditional(Conditional tree) {
	tree.cond = translate(tree.cond);
	tree.truepart = translate(tree.truepart);
	tree.falsepart = translate(tree.falsepart);
	result = tree;
    }

    public void visitIf(If tree) {
	tree.cond = translate(tree.cond);
	tree.thenpart = translate(tree.thenpart);
	tree.elsepart = translate(tree.elsepart);
	result = tree;
    }

    public void visitExec(Exec tree) {
	tree.expr = translate(tree.expr);
	result = tree;
    }

    public void visitBreak(Break tree) {
	result = tree;
    }

    public void visitContinue(Continue tree) {
	result = tree;
    }

    public void visitReturn(Return tree) {
	tree.expr = translate(tree.expr);
	result = tree;
    }

    public void visitThrow(Throw tree) {
	tree.expr = translate(tree.expr);
	result = tree;
    }

    public void visitAssert(Assert tree) {
	tree.cond = translate(tree.cond);
	tree.detail = translate(tree.detail);
	result = tree;
    }

    public void visitApply(Apply tree) {
	tree.meth = translate(tree.meth);
	tree.args = translate(tree.args);
	result = tree;
    }

    public void visitNewClass(NewClass tree) {
	tree.encl = translate(tree.encl);
	tree.clazz = translate(tree.clazz);
	tree.args = translate(tree.args);
	tree.def = (ClassDef)translate(tree.def);
	result = tree;
    }

    public void visitNewArray(NewArray tree) {
	tree.elemtype = translate(tree.elemtype);
	tree.dims = translate(tree.dims);
	tree.elems = translate(tree.elems);
	result = tree;
    }

    public void visitParens(Parens tree) {
	tree.expr = translate(tree.expr);
	result = tree;
    }

    public void visitAssign(Assign tree) {
	tree.lhs = translate(tree.lhs);
	tree.rhs = translate(tree.rhs);
	result = tree;
    }

    public void visitAssignop(Assignop tree) {
	tree.lhs = translate(tree.lhs);
	tree.rhs = translate(tree.rhs);
	result = tree;
    }

    public void visitUnary(Unary tree) {
	tree.arg = translate(tree.arg);
	result = tree;
    }

    public void visitBinary(Binary tree) {
	tree.lhs = translate(tree.lhs);
	tree.rhs = translate(tree.rhs);
	result = tree;
    }

    public void visitTypeCast(TypeCast tree) {
	tree.clazz = translate(tree.clazz);
	tree.expr = translate(tree.expr);
	result = tree;
    }

    public void visitTypeTest(TypeTest tree) {
	tree.expr = translate(tree.expr);
	tree.clazz = translate(tree.clazz);
	result = tree;
    }

    public void visitIndexed(Indexed tree) {
	tree.indexed = translate(tree.indexed);
	tree.index = translate(tree.index);
	result = tree;
    }

    public void visitSelect(Select tree) {
	tree.selected = translate(tree.selected);
	result = tree;
    }

    public void visitIdent(Ident tree) {
	result = tree;
    }

    public void visitLiteral(Literal tree) {
	result = tree;
    }

    public void visitTypeIdent(TypeIdent tree) {
	result = tree;
    }

    public void visitTypeArray(TypeArray tree) {
	tree.elemtype = translate(tree.elemtype);
	result = tree;
    }

    public void visitTypeApply(TypeApply tree) {
	tree.clazz = translate(tree.clazz);
	tree.arguments = translate(tree.arguments);
	result = tree;
    }

    public void visitTypeParameter(TypeParameter tree) {
	tree.bounds = translate(tree.bounds);
	result = tree;
    }

    public void visitTypeArgument(TypeArgument tree) {
	tree.inner = translate(tree.inner);
	result = tree;
    }

    public void visitErroneous(Erroneous tree) {
	result = tree;
    }

    public void visitLetExpr(LetExpr tree) {
	tree.defs = translateVarDefs(tree.defs);
	tree.expr = translate(tree.expr);
	result = tree;
    }

    public void visitModifiers(Modifiers tree) {
	tree.annotations = translateAnnotations(tree.annotations);
	result = tree;
    }

    public void visitAnnotation(Annotation tree) {
	tree.annotationType = translate(tree.annotationType);
	tree.args = translate(tree.args);
	result = tree;
    }

    public void visitTree(Tree tree) {
	throw new AssertionError(tree);
    }
}
