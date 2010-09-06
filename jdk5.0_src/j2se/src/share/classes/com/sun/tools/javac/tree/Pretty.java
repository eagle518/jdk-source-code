/**
 * @(#)Pretty.java	1.56 04/06/06
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.tree;

import java.io.*;
import java.util.*;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.util.List;
import com.sun.tools.javac.code.*;

import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.tree.Tree.*;

import static com.sun.tools.javac.code.Flags.*;

/** Prints out a tree as an indented Java source program.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Pretty extends Tree.Visitor {

    public Pretty(PrintWriter out, boolean sourceOutput) {
	this.out = out;
	this.sourceOutput = sourceOutput;
    }

    /** Set when we are producing source output.  If we're not
     *  producing source output, we can sometimes give more detail in
     *  the output even though that detail would not be valid java
     *  soruce.
     */
    private final boolean sourceOutput;

    /** The output stream on which trees are printed.
     */
    PrintWriter out;

    /** Indentation width (can be reassigned from outside).
     */
    public int width = 4;

    /** The current left margin.
     */
    int lmargin = 0;

    /** The enclosing class name.
     */
    Name enclClassName;

    /** A hashtable mapping trees to their documentation comments
     *  (can be null)
     */
    Map<Tree,String> docComments = null;

    /** Align code to be indented to left margin.
     */
    void align() {
        for (int i = 0; i < lmargin; i++) out.print(" ");
    }

    /** Increase left margin by indentation width.
     */
    void indent() {
        lmargin = lmargin + width;
    }

    /** Decrease left margin by indentation width.
     */
    void undent() {
        lmargin = lmargin - width;
    }

    /** Enter a new precedence level. Emit a `(' if new precedence level
     *  is less than precedence level so far.
     *  @param contextPrec    The precedence level in force so far.
     *  @param ownPrec        The new precedence level.
     */
    void open(int contextPrec, int ownPrec) {
        if (ownPrec < contextPrec) out.print("(");
    }

    /** Leave precedence level. Emit a `(' if inner precedence level
     *  is less than precedence level we revert to.
     *  @param contextPrec    The precedence level we revert to.
     *  @param ownPrec        The inner precedence level.
     */
    void close(int contextPrec, int ownPrec) {
        if (ownPrec < contextPrec) out.print(")");
    }

    /** Print string, replacing all non-ascii character with unicode escapes.
     */
    public void print(Object s) {
	out.print(Convert.escapeUnicode(s.toString()));
    }

    /** Print new line.
     */
    public void println() {
	out.println();
    }

/**************************************************************************
 * Traversal methods
 *************************************************************************/

    /** Visitor argument: the current precedence level.
     */
    int prec;

    /** Visitor method: print expression tree.
     *  @param prec  The current precedence level.
     */
    public void printExpr(Tree tree, int prec) {
	int prevPrec = this.prec;
	try {
	    this.prec = prec;
	    if (tree == null) print("/*missing*/");
	    else tree.accept(this);
	} finally {
	    this.prec = prevPrec;
	}
    }
	
    /** Derived visitor method: print expression tree at minimum precedence level
     *  for expression.
     */
    public void printExpr(Tree tree) {
	printExpr(tree, TreeInfo.noPrec);
    }

    /** Derived visitor method: print statement tree.
     */
    public void printStat(Tree tree) {
	printExpr(tree, TreeInfo.notExpression);
    }

    /** Derived visitor method: print list of expression trees, separated by given string.
     *  @param sep the separator string
     */
    public <T extends Tree> void printExprs(List<T> trees, String sep) {
	if (trees.nonEmpty()) {
	    printExpr(trees.head);
	    for (List<T> l = trees.tail; l.nonEmpty(); l = l.tail) {
		print(sep);
		printExpr(l.head);
	    }
	}
    }

    /** Derived visitor method: print list of expression trees, separated by commas.
     */
    public <T extends Tree> void printExprs(List<T> trees) {
	printExprs(trees, ", ");
    }

    /** Derived visitor method: print list of statements, each on a separate line.
     */
    public <T extends Tree> void printStats(List<T> trees) {
	for (List<T> l = trees; l.nonEmpty(); l = l.tail) {
            align();
            printStat(l.head);
	    println();
        }
    }

    /** Print a set of modifiers.
     */
    public void printFlags(long flags) {
	if ((flags & SYNTHETIC) != 0) print("/*synthetic*/ ");
	print(TreeInfo.flagNames(flags));
	if ((flags & StandardFlags) != 0) print(" ");
	if ((flags & ANNOTATION) != 0) print("@");
    }

    public void printAnnotations(List<Annotation> trees) {
	for (List<Annotation> l = trees; l.nonEmpty(); l = l.tail) {
            printStat(l.head);
	    println();
            align();
        }
    }

    /** Print documentation comment, if it exists
     *  @param tree    The tree for which a documentation comment should be printed.
     */
    public void printDocComment(Tree tree) {
        if (docComments != null) {
	  String dc = docComments.get(tree);
	  if (dc != null) {
	    print("/**"); println();
	    int pos = 0;
	    int endpos = lineEndPos(dc, pos);
	    while (pos < dc.length()) {
	        align();
		print(" *");
		if (pos < dc.length() && dc.charAt(pos) > ' ') print(" ");
		print(dc.substring(pos, endpos)); println();
		pos = endpos + 1;
		endpos = lineEndPos(dc, pos);
	    }
	    align(); print(" */"); println();
	    align();
	  }
	}
    }
//where
        static int lineEndPos(String s, int start) {
	    int pos = s.indexOf('\n', start);
	    if (pos < 0) pos = s.length();
	    return pos;
	}

    /** If type parameter list is non-empty, print it enclosed in "<...>" brackets.
     */
    public void printTypeParameters(List<TypeParameter> trees) {
	if (trees.nonEmpty()) {
	    print("<");
	    printExprs(trees);
	    print(">");
	}
    }

    /** Print a block.
     */
    public void printBlock(List<Tree> stats) {
	print("{");
	println();
	indent();
	printStats(stats);
	undent();
	align();
	print("}");
    }

    /** Print a block.
     */
    public void printEnumBody(List<Tree> stats) {
	print("{");
	println();
	indent();
	boolean first = true;
	for (List<Tree> l = stats; l.nonEmpty(); l = l.tail) {
	    if (isEnumerator(l.head)) {
		if (!first) {
		    print(",");
		    println();
		}
		align();
		printStat(l.head);
		first = false;
	    }
        }
	print(";");
	println();
	for (List<Tree> l = stats; l.nonEmpty(); l = l.tail) {
	    if (!isEnumerator(l.head)) {
		align();
		printStat(l.head);
		println();
	    }
        }
	undent();
	align();
	print("}");
    }

    /** Is the given tree an enumerator definition? */
    boolean isEnumerator(Tree t) {
	return t.tag == Tree.VARDEF && (((VarDef)t).mods.flags & ENUM) != 0;
    }

    /** Print unit consisting of package clause and import statements in toplevel,
     *  followed by class definition. if class definition == null,
     *  print all definitions in toplevel.
     *  @param tree     The toplevel tree
     *  @param cdef     The class definition, which is assumed to be part of the
     *                  toplevel tree.
     */
    public void printUnit(TopLevel tree, ClassDef cdef) {
        docComments = tree.docComments;
	printDocComment(tree);
	if (tree.pid != null) {
	    print("package ");
	    printExpr(tree.pid);
	    print(";");
	    println();
	}
	boolean firstImport = true;
	for (List<Tree> l = tree.defs;
	     l.nonEmpty() && (cdef == null || l.head.tag == Tree.IMPORT);
	     l = l.tail) {
	    if (l.head.tag == Tree.IMPORT) {
		Import imp = (Import)l.head;
		Name name = TreeInfo.name(imp.qualid);
		if (name == name.table.asterisk ||
                    cdef == null ||
		    isUsed(TreeInfo.symbol(imp.qualid), cdef)) {
		    if (firstImport) {
			firstImport = false;
			println();
		    }
		    printStat(imp);
		}
	    } else {
		printStat(l.head);
	    }
	}
	if (cdef != null) {
	    printStat(cdef);
	    println();
	}
    }
        // where
	boolean isUsed(final Symbol t, Tree cdef) {
	    class UsedVisitor extends TreeScanner {
		public void scan(Tree tree) {
		    if (tree!=null && !result) tree.accept(this);
		}
		boolean result = false;
		public void visitIdent(Ident tree) {
		    if (tree.sym == t) result = true;
		}
	    }
	    UsedVisitor v = new UsedVisitor();
	    v.scan(cdef);
	    return v.result;
	}

/**************************************************************************
 * Visitor methods
 *************************************************************************/

    public void visitTopLevel(TopLevel tree) {
	printUnit(tree, null);
    }

    public void visitImport(Import tree) {
	print("import ");
        if (tree.staticImport) print("static ");
	printExpr(tree.qualid);
	print(";");
	println();
    }

    public void visitClassDef(ClassDef tree) {
        println(); align();
        printDocComment(tree);
	printAnnotations(tree.mods.annotations);
	printFlags(tree.mods.flags & ~INTERFACE);
	Name enclClassNamePrev = enclClassName;
	enclClassName = tree.name;
	if ((tree.mods.flags & INTERFACE) != 0) {
	    print("interface " + tree.name);
	    printTypeParameters(tree.typarams);
	    if (tree.implementing.nonEmpty()) {
		print(" extends ");
		printExprs(tree.implementing);
	    }
	} else {
	    if ((tree.mods.flags & ENUM) != 0)
		print("enum " + tree.name);
	    else
		print("class " + tree.name);
	    printTypeParameters(tree.typarams);
	    if (tree.extending != null) {
		print(" extends ");
		printExpr(tree.extending);
	    }
	    if (tree.implementing.nonEmpty()) {
		print(" implements ");
		printExprs(tree.implementing);
	    }
	}
	print(" ");
	if ((tree.mods.flags & ENUM) != 0) {
	    printEnumBody(tree.defs);
	} else {
	    printBlock(tree.defs);
	}
	enclClassName = enclClassNamePrev;
    }

    public void visitMethodDef(MethodDef tree) {
	// when producing source output, omit anonymous constructors
	if (tree.name == tree.name.table.init &&
	    enclClassName == null &&
	    sourceOutput) return;
	println(); align();
	printDocComment(tree);
	printExpr(tree.mods);
	printTypeParameters(tree.typarams);
	if (tree.name == tree.name.table.init) {
	    print(enclClassName != null ? enclClassName : tree.name);
	} else {
	    printExpr(tree.restype);
	    print(" " + tree.name);
	}
	print("(");
	printExprs(tree.params);
	print(")");
	if (tree.thrown.nonEmpty()) {
	    print(" throws ");
	    printExprs(tree.thrown);
	}
	if (tree.body != null) {
	    print(" ");
	    printStat(tree.body);
	} else {
	    print(";");
	}
    }

    public void visitVarDef(VarDef tree) {
        if (docComments != null && docComments.get(tree) != null) {
	    println(); align();
	}
        printDocComment(tree);
	if ((tree.mods.flags & ENUM) != 0) {
	    print("/*public static final*/ ");
	    print(tree.name);
	    if (tree.init != null) {
                print(" /* = ");
                printExpr(tree.init);
                print(" */");
            }
	} else {
	    printExpr(tree.mods);
	    if ((tree.mods.flags & VARARGS) != 0) {
		printExpr(((TypeArray)tree.vartype).elemtype);
		print("... " + tree.name);
	    } else {
		printExpr(tree.vartype);
		print(" " + tree.name);
	    }
	    if (tree.init != null) {
		print(" = ");
		printExpr(tree.init);
	    }
	    if (prec == TreeInfo.notExpression) print(";");
	}
    }

    public void visitSkip(Skip tree) {
	print(";");
    }	

    public void visitBlock(Block tree) {
        printFlags(tree.flags);
	printBlock(tree.stats);
    }	

    public void visitDoLoop(DoLoop tree) {
	print("do ");
	printStat(tree.body);
	align();
	print(" while ");
	if (tree.cond.tag == Tree.PARENS) {
	    printExpr(tree.cond);
	} else {
	    print("(");
	    printExpr(tree.cond);
	    print(")");
	}
	print(";");
    }

    public void visitWhileLoop(WhileLoop tree) {
	print("while ");
	if (tree.cond.tag == Tree.PARENS) {
	    printExpr(tree.cond);
	} else {
	    print("(");
	    printExpr(tree.cond);
	    print(")");
	}
	print(" ");
	printStat(tree.body);
    }

    public void visitForLoop(ForLoop tree) {
	print("for (");
	if (tree.init.nonEmpty()) {
	    if (tree.init.head.tag == Tree.VARDEF) {
		printExpr(tree.init.head);
		for (List<Tree> l = tree.init.tail; l.nonEmpty(); l = l.tail) {
		    VarDef vdef = (VarDef)l.head;
		    print(", " + vdef.name + " = ");
		    printExpr(vdef.init);
		}
	    } else {
		printExprs(tree.init);
	    }
	}
	print("; ");
	if (tree.cond != null) printExpr(tree.cond);
	print("; ");
	printExprs(tree.step);
	print(") ");
	printStat(tree.body);
    }

    public void visitForeachLoop(ForeachLoop tree) {
	print("for (");
	printExpr(tree.var);
	print(" : ");
	printExpr(tree.expr);
	print(") ");
	printStat(tree.body);
    }

    public void visitLabelled(Labelled tree) {
	print(tree.label + ": ");
	printStat(tree.body);
    }

    public void visitSwitch(Switch tree) {
	print("switch ");
	if (tree.selector.tag == Tree.PARENS) {
	    printExpr(tree.selector);
	} else {
	    print("(");
	    printExpr(tree.selector);
	    print(")");
	}
	print(" {");
	println();
	printStats(tree.cases);
	align();
	print("}");
    }

    public void visitCase(Case tree) {
	if (tree.pat == null) {
	    print("default");
	} else {
	    print("case ");
	    printExpr(tree.pat);
	}
	print(": ");
	println();
	indent();
	printStats(tree.stats);
	undent();
	align();
    }

    public void visitSynchronized(Synchronized tree) {
	print("synchronized ");
	if (tree.lock.tag == Tree.PARENS) {
	    printExpr(tree.lock);
	} else {
	    print("(");
	    printExpr(tree.lock);
	    print(")");
	}
	print(" ");
	printStat(tree.body);
    }

    public void visitTry(Try tree) {
	print("try ");
	printStat(tree.body);
	for (List<Catch> l = tree.catchers; l.nonEmpty(); l = l.tail) {
	    printStat(l.head);
	}
	if (tree.finalizer != null) {
	    print(" finally ");
	    printStat(tree.finalizer);
	}
    }

    public void visitCatch(Catch tree) {
	print(" catch (");
	printExpr(tree.param);
	print(") ");
	printStat(tree.body);
    }

    public void visitConditional(Conditional tree) {
	open(prec, TreeInfo.condPrec);
	printExpr(tree.cond, TreeInfo.condPrec);
	print(" ? ");
	printExpr(tree.truepart, TreeInfo.condPrec);
	print(" : ");
	printExpr(tree.falsepart, TreeInfo.condPrec);
	close(prec, TreeInfo.condPrec);
    }

    public void visitIf(If tree) {
	print("if ");
	if (tree.cond.tag == Tree.PARENS) {
	    printExpr(tree.cond);
	} else {
	    print("(");
	    printExpr(tree.cond);
	    print(")");
	}
	print(" ");
	printStat(tree.thenpart);
	if (tree.elsepart != null) {
	    print(" else ");
	    printStat(tree.elsepart);
	}
    }

    public void visitExec(Exec tree) {
	printExpr(tree.expr);
	if (prec == TreeInfo.notExpression) print(";");
    }

    public void visitBreak(Break tree) {
	print("break");
	if (tree.label != null) print(" " + tree.label);
	print(";");
    }

    public void visitContinue(Continue tree) {
	print("continue");
	if (tree.label != null) print(" " + tree.label);
	print(";");
    }

    public void visitReturn(Return tree) {
	print("return");
	if (tree.expr != null) {
	    print(" ");
	    printExpr(tree.expr);
	}
	print(";");
    }

    public void visitThrow(Throw tree) {
	print("throw ");
	printExpr(tree.expr);
	print(";");
    }

    public void visitAssert(Assert tree) {
	print("assert ");
	    printExpr(tree.cond);
	if (tree.detail != null) {
	    print(" : ");
	    printExpr(tree.detail);
	}
	print(";");
    }

    public void visitApply(Apply tree) {
	if (tree.typeargs != null) {
	    if (tree.meth.tag == Tree.SELECT) {
		Select left = (Select)tree.meth;
		printExpr(left.selected);
		print(".<");
		printExprs(tree.typeargs);
		print(">" + left.name);
	    } else {
		print("<");
		printExprs(tree.typeargs);
		print(">");
		printExpr(tree.meth);
	    }
	} else {
	    printExpr(tree.meth);
	}
	print("(");
	printExprs(tree.args);
	print(")");
    }

    public void visitNewClass(NewClass tree) {
	if (tree.encl != null) {
	    printExpr(tree.encl);
	    print(".");
	}
	print("new ");
	if (tree.typeargs != null) {
	    print("<");
	    printExprs(tree.typeargs);
	    print(">");
	}
	printExpr(tree.clazz);
	print("(");
	printExprs(tree.args);
	print(")");
	if (tree.def != null) {
	    Name enclClassNamePrev = enclClassName;
	    enclClassName =
		tree.def.name != null ? tree.def.name :
		tree.type != null && tree.type.tsym.name != tree.type.tsym.name.table.empty ? tree.type.tsym.name :
		null;
	    if ((tree.def.mods.flags & Flags.ENUM) != 0) print("/*enum*/");
	    printBlock(((ClassDef)tree.def).defs);
	    enclClassName = enclClassNamePrev;
	}
    }

    public void visitNewArray(NewArray tree) {
	if (tree.elemtype != null) {
	    print("new ");
	    Tree elem = tree.elemtype;
	    if (elem instanceof TypeArray)
		printBaseElementType((TypeArray) elem);
	    else
		printExpr(elem);
	    for (List<Tree> l = tree.dims; l.nonEmpty(); l = l.tail) {
		print("[");
		printExpr(l.head);
		print("]");
	    }
	    if (elem instanceof TypeArray)
		printBrackets((TypeArray) elem);
	}
	if (tree.elems != null) {
	    if (tree.elemtype != null) print("[]");
	    print("{");
	    printExprs(tree.elems);
	    print("}");
	}
    }

    public void visitParens(Parens tree) {
	print("(");
	printExpr(tree.expr);
	print(")");
    }

    public void visitAssign(Assign tree) {
	open(prec, TreeInfo.assignPrec);
	printExpr(tree.lhs, TreeInfo.assignPrec + 1);
	print(" = ");
	printExpr(tree.rhs, TreeInfo.assignPrec);
	close(prec, TreeInfo.assignPrec);
    }

    public String operatorName(int tag) {
	switch(tag) {
	case Tree.POS:     return "+";
	case Tree.NEG:     return "-";
	case Tree.NOT:     return "!";
	case Tree.COMPL:   return "~";
	case Tree.PREINC:  return "++";
	case Tree.PREDEC:  return "--";
	case Tree.POSTINC: return "++";
	case Tree.POSTDEC: return "--";
	case Tree.NULLCHK: return "<*nullchk*>";
	case Tree.OR:      return "||";
	case Tree.AND:     return "&&";
	case Tree.EQ:      return "==";
	case Tree.NE:      return "!=";
	case Tree.LT:      return "<";
	case Tree.GT:      return ">";
	case Tree.LE:      return "<=";
	case Tree.GE:      return ">=";
	case Tree.BITOR:   return "|";
	case Tree.BITXOR:  return "^";
	case Tree.BITAND:  return "&";
	case Tree.SL:      return "<<";
	case Tree.SR:      return ">>";
	case Tree.USR:     return ">>>";
	case Tree.PLUS:    return "+";
	case Tree.MINUS:   return "-";
	case Tree.MUL:     return "*";
	case Tree.DIV:     return "/";
	case Tree.MOD:     return "%";
	default: throw new Error();
	}
    }

    public void visitAssignop(Assignop tree) {
	open(prec, TreeInfo.assignopPrec);
	printExpr(tree.lhs, TreeInfo.assignopPrec + 1);
	print(" " + operatorName(tree.tag - Tree.ASGOffset) + "= ");
	printExpr(tree.rhs, TreeInfo.assignopPrec);
	close(prec, TreeInfo.assignopPrec);
    }

    public void visitUnary(Unary tree) {
	int ownprec = TreeInfo.opPrec(tree.tag);
	String opname = operatorName(tree.tag).toString();
	open(prec, ownprec);
	if (tree.tag <= Tree.PREDEC) {
	    print(opname);
	    printExpr(tree.arg, ownprec);
	} else {
	    printExpr(tree.arg, ownprec);
	    print(opname);
	}
	close(prec, ownprec);
    }

    public void visitBinary(Binary tree) {
	int ownprec = TreeInfo.opPrec(tree.tag);
	String opname = operatorName(tree.tag).toString();
	open(prec, ownprec);
	printExpr(tree.lhs, ownprec);
	print(" " + opname + " ");
	printExpr(tree.rhs, ownprec + 1);
	close(prec, ownprec);
    }

    public void visitTypeCast(TypeCast tree) {
	open(prec, TreeInfo.prefixPrec);
	print("(");
	printExpr(tree.clazz);
	print(")");
	printExpr(tree.expr, TreeInfo.prefixPrec);
	close(prec, TreeInfo.prefixPrec);
    }

    public void visitTypeTest(TypeTest tree) {
	open(prec, TreeInfo.ordPrec);
	printExpr(tree.expr, TreeInfo.ordPrec);
	print(" instanceof ");
	printExpr(tree.clazz, TreeInfo.ordPrec + 1);
	close(prec, TreeInfo.ordPrec);
    }

    public void visitIndexed(Indexed tree) {
	printExpr(tree.indexed, TreeInfo.postfixPrec);
	print("[");
	printExpr(tree.index);
	print("]");
    }

    public void visitSelect(Select tree) {
	printExpr(tree.selected, TreeInfo.postfixPrec);
	print("." + tree.name);
    }

    public void visitIdent(Ident tree) {
	print(tree.name);
    }

    public void visitLiteral(Literal tree) {
	switch (tree.typetag) {
	case TypeTags.INT:
	    print(tree.value.toString());
	    break;
	case TypeTags.LONG:
	    print(tree.value + "L");
	    break;
	case TypeTags.FLOAT:
	    print(tree.value + "F");
	    break;
	case TypeTags.DOUBLE:
	    print(tree.value.toString());
	    break;
	case TypeTags.CHAR:
	    print("\'" +
		  Convert.quote(
		      String.valueOf((char)((Number)tree.value).intValue())) +
		  "\'");
	    break;
	default:	
	    print("\"" + Convert.quote(tree.value.toString()) + "\"");
	    break;
	}
    }

    public void visitTypeIdent(TypeIdent tree) {
	switch(tree.typetag) {
	case TypeTags.BYTE:
	    print("byte");
	    break;
	case TypeTags.CHAR:
	    print("char");
	    break;
	case TypeTags.SHORT:
	    print("short");
	    break;
	case TypeTags.INT:
	    print("int");
	    break;
	case TypeTags.LONG:
	    print("long");
	    break;
	case TypeTags.FLOAT:
	    print("float");
	    break;
	case TypeTags.DOUBLE:
	    print("double");
	    break;
	case TypeTags.BOOLEAN:
	    print("boolean");
	    break;
	case TypeTags.VOID:
	    print("void");
	    break;
	default:
	    print("error");
	    break;
	}
    }

    public void visitTypeArray(TypeArray tree) {
	printBaseElementType(tree);
	printBrackets(tree);
    }

    // Prints the inner element type of a nested array
    private void printBaseElementType(TypeArray tree) {
	Tree elem = tree.elemtype;
	while (elem instanceof TypeArgument)
	    elem = ((TypeArgument) elem).inner;
	if (elem instanceof TypeArray)
	    printBaseElementType((TypeArray) elem);
	else
	    printExpr(elem);
    }

    // prints the brackets of a nested array in reverse order
    private void printBrackets(TypeArray tree) {
	Tree elem;
	while (true) {
	    elem = tree.elemtype;
	    print("[]");
	    if (!(elem instanceof TypeArray)) break;
	    tree = (TypeArray) elem;
	}
    }	

    public void visitTypeApply(TypeApply tree) {
	printExpr(tree.clazz);
	print("<");
	printExprs(tree.arguments);
	print(">");
    }

    public void visitTypeParameter(TypeParameter tree) {
	print(tree.name);
	if (tree.bounds.nonEmpty()) {
	    print(" extends ");
	    printExprs(tree.bounds, " & ");
	}
    }

    public void visitTypeArgument(TypeArgument tree) {
	print("" + tree.kind);
	if (tree.kind != BoundKind.UNBOUND)
	    printExpr(tree.inner);
    }


    public void visitErroneous(Erroneous tree) {
	print("(ERROR)");
    }

    public void visitLetExpr(LetExpr tree) {
	print("(let " + tree.defs + " in " + tree.expr + ")");
    }

    public void visitModifiers(Modifiers mods) {
	printAnnotations(mods.annotations);
	printFlags(mods.flags);
    }

    public void visitAnnotation(Annotation tree) {
	print("@");
	printExpr(tree.annotationType);
	print("(");
	printExprs(tree.args);
	print(")");
    }

    public void visitTree(Tree tree) {
	print("(UNKNOWN: " + tree + ")");
	println();
    }
}
