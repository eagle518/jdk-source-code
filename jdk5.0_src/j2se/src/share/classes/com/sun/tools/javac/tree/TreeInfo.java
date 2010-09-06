/**
 * @(#)TreeInfo.java	1.38 04/07/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.tree;

import java.io.File;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.code.*;

import com.sun.tools.javac.code.Type.*;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.tree.Tree.*;

import static com.sun.tools.javac.code.Flags.*;

/** Utility class containing inspector methods for trees.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class TreeInfo {
    protected static final Context.Key<TreeInfo> treeInfoKey =
	new Context.Key<TreeInfo>();

    public static TreeInfo instance(Context context) {
	TreeInfo instance = context.get(treeInfoKey);
	if (instance == null)
	    instance = new TreeInfo(context);
	return instance;
    }

    /** The names of all operators.
     */
    private Name[] opname = new Name[Tree.MOD - Tree.POS + 1];

    private TreeInfo(Context context) {
	context.put(treeInfoKey, this);

	Name.Table names = Name.Table.instance(context);
	opname[Tree.POS     - Tree.POS] = names.fromString("+");
	opname[Tree.NEG     - Tree.POS] = names.hyphen;
	opname[Tree.NOT     - Tree.POS] = names.fromString("!");
	opname[Tree.COMPL   - Tree.POS] = names.fromString("~");
	opname[Tree.PREINC  - Tree.POS] = names.fromString("++");
	opname[Tree.PREDEC  - Tree.POS] = names.fromString("--");
	opname[Tree.POSTINC - Tree.POS] = names.fromString("++");
	opname[Tree.POSTDEC - Tree.POS] = names.fromString("--");
	opname[Tree.NULLCHK - Tree.POS] = names.fromString("<*nullchk*>");
	opname[Tree.OR      - Tree.POS] = names.fromString("||");
	opname[Tree.AND     - Tree.POS] = names.fromString("&&");
	opname[Tree.EQ      - Tree.POS] = names.fromString("==");
	opname[Tree.NE      - Tree.POS] = names.fromString("!=");
	opname[Tree.LT      - Tree.POS] = names.fromString("<");
	opname[Tree.GT      - Tree.POS] = names.fromString(">");
	opname[Tree.LE      - Tree.POS] = names.fromString("<=");
	opname[Tree.GE      - Tree.POS] = names.fromString(">=");
	opname[Tree.BITOR   - Tree.POS] = names.fromString("|");
	opname[Tree.BITXOR  - Tree.POS] = names.fromString("^");
	opname[Tree.BITAND  - Tree.POS] = names.fromString("&");
	opname[Tree.SL      - Tree.POS] = names.fromString("<<");
	opname[Tree.SR      - Tree.POS] = names.fromString(">>");
	opname[Tree.USR     - Tree.POS] = names.fromString(">>>");
	opname[Tree.PLUS    - Tree.POS] = names.fromString("+");
	opname[Tree.MINUS   - Tree.POS] = names.hyphen;
	opname[Tree.MUL     - Tree.POS] = names.asterisk;
	opname[Tree.DIV     - Tree.POS] = names.slash;
	opname[Tree.MOD     - Tree.POS] = names.fromString("%");
    }


    /** Return name of operator with given tree tag.
     */
    public Name operatorName(int tag) {
	return opname[tag - Tree.POS];
    }

    /** Is tree a constructor declaration?
     */
    public static boolean isConstructor(Tree tree) {
	if (tree.tag == Tree.METHODDEF) {
	    Name name = ((MethodDef)tree).name;
	    return name == name.table.init;
	} else {
	    return false;
	}
    }

    /** Is there a constructor declaration in the given list of trees?
     */
    public static boolean hasConstructors(List<Tree> trees) {
	for (List<Tree> l = trees; l.nonEmpty(); l = l.tail)
	    if (isConstructor(l.head)) return true;
	return false;
    }

    /** Is statement an initializer for a synthetic field?
     */
    public static boolean isSyntheticInit(Tree stat) {
	if (stat.tag == Tree.EXEC) {
	    Exec exec = (Exec)stat;
	    if (exec.expr.tag == Tree.ASSIGN) {
		Assign assign = (Assign)exec.expr;
		if (assign.lhs.tag == Tree.SELECT) {
		    Select select = (Select)assign.lhs;
		    if (select.sym != null &&
			(select.sym.flags() & SYNTHETIC) != 0) {
			Name selected = name(select.selected);
			if (selected != null && selected == selected.table._this)
			    return true;
		    }
		}
	    }
	}
	return false;
    }

    /** If the expression is a method call, return the method name, null
     *  otherwise. */
    public static Name calledMethodName(Tree tree) {
	if (tree.tag == Tree.EXEC) {
	    Exec exec = (Exec)tree;
	    if (exec.expr.tag == Tree.APPLY) {
		Name mname = TreeInfo.name(((Apply)exec.expr).meth);
		return mname;
	    }
	}
	return null;
    }

    /** Is this a call to this or super?
     */
    public static boolean isSelfCall(Tree tree) {
	Name name = calledMethodName(tree);
	if (name != null) {
	    Name.Table names = name.table;
	    return name==names._this || name==names._super;
	} else {
	    return false;
	}
    }

    /** Is this a call to super?
     */
    public static boolean isSuperCall(Tree tree) {
	Name name = calledMethodName(tree);
	if (name != null) {
	    Name.Table names = name.table;
	    return name==names._super;
	} else {
	    return false;
	}
    }

    /** Is this a constructor whose first (non-synthetic) statement is not
     *  of the form this(...)?
     */
    public static boolean isInitialConstructor(Tree tree) {
	Apply app = firstConstructorCall(tree);
	if (app == null) return false;
	Name meth = name(app.meth);
	return meth == null || meth != meth.table._this;
    }

    /** Return the first call in a constructor definition. */
    public static Apply firstConstructorCall(Tree tree) {
	if (tree.tag != Tree.METHODDEF) return null;
	MethodDef md = (MethodDef) tree;
	Name.Table names = md.name.table;
	if (md.name != names.init) return null;
	if (md.body == null) return null;
	List<Tree> stats = md.body.stats;
	// Synthetic initializations can appear before the super call.
	while (stats.nonEmpty() && isSyntheticInit(stats.head))
	    stats = stats.tail;
	if (stats.isEmpty()) return null;
	if (stats.head.tag != Tree.EXEC) return null;
	Exec exec = (Exec) stats.head;
	if (exec.expr.tag != Tree.APPLY) return null;
	return (Apply)exec.expr;
    }

    /** The position of the first statement in a block, or the position of
     *  the block itself if it is empty.
     */
    public static int firstStatPos(Tree tree) {
	if (tree.tag == Tree.BLOCK && ((Block) tree).stats.nonEmpty())
	    return ((Block) tree).stats.head.pos;
	else
	    return tree.pos;
    }

    /** The end position of given tree, if it is a block with
     *  defined endpos.
     */
    public static int endPos(Tree tree) {
	if (tree.tag == Tree.BLOCK && ((Block) tree).endpos != Position.NOPOS)
	    return ((Block) tree).endpos;
	else if (tree.tag == Tree.SYNCHRONIZED)
	    return endPos(((Synchronized) tree).body);
	else if (tree.tag == Tree.TRY) {
	    Try t = (Try) tree;
	    return endPos((t.finalizer != null)
			  ? t.finalizer
			  : t.catchers.last().body);
	} else
	    return tree.pos;
    }

    /** The position of the finalizer of given try/synchronized statement.
     */
    public static int finalizerPos(Tree tree) {
	if (tree.tag == Tree.TRY) {
	    Try t = (Try) tree;
	    assert t.finalizer != null;
	    return firstStatPos(t.finalizer);
	} else if (tree.tag == Tree.SYNCHRONIZED) {
	    return endPos(((Synchronized) tree).body);
	} else {
	    throw new AssertionError();
	}
    }

    /** Find the position for reporting an error about a symbol, where
     *  that symbol is defined somewhere in the given tree. */
    public static int positionFor(final Symbol sym, final Tree tree) {
        Tree decl = declarationFor(sym, tree);
        return ((decl != null) ? decl : tree).pos;
    }

    /** Find the declaration for a symbol, where
     *  that symbol is defined somewhere in the given tree. */
    public static Tree declarationFor(final Symbol sym, final Tree tree) {
	class DeclScanner extends TreeScanner {
            Tree result = null;
            public void scan(Tree tree) {
                if (tree!=null && result==null)
                    tree.accept(this);
            }
	    public void visitTopLevel(TopLevel that) {
		if (that.packge == sym) result = that;
		else super.visitTopLevel(that);
	    }
	    public void visitClassDef(ClassDef that) {
		if (that.sym == sym) result = that;
		else super.visitClassDef(that);
	    }
	    public void visitMethodDef(MethodDef that) {
		if (that.sym == sym) result = that;
		else super.visitMethodDef(that);
	    }
	    public void visitVarDef(VarDef that) {
		if (that.sym == sym) result = that;
		else super.visitVarDef(that);
	    }
	}
	DeclScanner s = new DeclScanner();
	tree.accept(s);
	return s.result;
    }

    /** Return the statement referenced by a label.
     *  If the label refers to a loop or switch, return that switch
     *  otherwise return the labelled statement itself
     */
    public static Tree referencedStatement(Labelled tree) {
	Tree t = tree;
	do t = ((Labelled) t).body;
	while (t.tag == Tree.LABELLED);
	switch (t.tag) {
	case Tree.DOLOOP: case Tree.WHILELOOP: case Tree.FORLOOP: case Tree.SWITCH:
	    return t;
	default:
	    return tree;
	}
    }

    /** Skip parens and return the enclosed expression
     */
    public static Tree skipParens(Tree tree) {
	while (tree.tag == Tree.PARENS) {
	    tree = ((Parens)tree).expr;
	}
	return tree;
    }

    /** Return the types of a list of trees.
     */
    public static List<Type> types(List<Tree> trees) {
	ListBuffer<Type> ts = new ListBuffer<Type>();
	for (List<Tree> l = trees; l.nonEmpty(); l = l.tail)
	    ts.append(l.head.type);
	return ts.toList();
    }

    /** If this tree is an identifier or a field or a parameterized type,
     *  return its name, otherwise return null.
     */
    public static Name name(Tree tree) {
	switch (tree.tag) {
	case Tree.IDENT:
	    return ((Ident)tree).name;
	case Tree.SELECT:
	    return ((Select)tree).name;
	case Tree.TYPEAPPLY:
	    return name(((TypeApply)tree).clazz);
	default:
	    return null;
	}
    }

    /** If this tree is a qualified identifier, its return fully qualified name,
     *  otherwise return null.
     */
    public static Name fullName(Tree tree) {
	tree = skipParens(tree);
	switch (tree.tag) {
	case Tree.IDENT:
	    return ((Ident)tree).name;
	case Tree.SELECT:
	    Name sname = fullName(((Select)tree).selected);
	    return sname == null ? null : sname.append('.', name(tree));
	default:
	    return null;
	}
    }

    /** If this tree is an identifier or a field, return its symbol,
     *  otherwise return null.
     */
    public static Symbol symbol(Tree tree) {
	tree = skipParens(tree);
	switch (tree.tag) {
	case Tree.IDENT:
	    return ((Ident)tree).sym;
	case Tree.SELECT:
	    return ((Select)tree).sym;
	case Tree.TYPEAPPLY:
	    return symbol(((TypeApply)tree).clazz);
	default:
	    return null;
	}
    }

    /** Return true if this is a nonstatic selection. */
    public static boolean nonstaticSelect(Tree tree) {
	tree = skipParens(tree);
	if (tree.tag != Tree.SELECT) return false;
	Select s = (Select) tree;
	Symbol e = symbol(s.selected);
	return e == null || (e.kind != Kinds.PCK && e.kind != Kinds.TYP);
    }

    /** If this tree is an identifier or a field, set its symbol, otherwise skip.
     */
    public static void setSymbol(Tree tree, Symbol sym) {
	tree = skipParens(tree);
	switch (tree.tag) {
	case Tree.IDENT:
	    ((Ident)tree).sym = sym; break;
	case Tree.SELECT:
	    ((Select)tree).sym = sym; break;
	default:
	}
    }

    /** If this tree is a declaration or a block, return its flags field,
     *  otherwise return 0.
     */
    public static long flags(Tree tree) {
	switch (tree.tag) {
	case Tree.VARDEF:
	    return ((VarDef)tree).mods.flags;
	case Tree.METHODDEF:
	    return ((MethodDef)tree).mods.flags;
	case Tree.CLASSDEF:
	    return ((ClassDef)tree).mods.flags;
	case Tree.BLOCK:
	    return ((Block)tree).flags;
	default:
	    return 0;
	}
    }

    /** Return first (smallest) flag in `flags':
     *  pre: flags != 0
     */
    public static long firstFlag(long flags) {
	int flag = 1;
        while ((flag & StandardFlags) != 0 && (flag & flags) == 0)
	    flag = flag << 1;
        return flag;
    }

    /** Return flags as a string, separated by " ".
     */
    public static String flagNames(long flags) {
        return Flags.toString(flags & StandardFlags).trim();
    }

    /** Operator precedences values.
     */
    public static final int
        notExpression = -1,   // not an expression
        noPrec = 0,           // no enclosing expression
        assignPrec = 1,
	assignopPrec = 2,
	condPrec = 3,
	orPrec = 4,
	andPrec = 5,
	bitorPrec = 6,
	bitxorPrec = 7,
	bitandPrec = 8,
	eqPrec = 9,
	ordPrec = 10,
	shiftPrec = 11,
	addPrec = 12,
	mulPrec = 13,
	prefixPrec = 14,
	postfixPrec = 15,
	precCount = 16;


    /** Map operators to their precedence levels.
     */
    public static int opPrec(int op) {
	switch(op) {
	case Tree.POS:
	case Tree.NEG:
	case Tree.NOT:
	case Tree.COMPL:
	case Tree.PREINC:
	case Tree.PREDEC: return prefixPrec;
	case Tree.POSTINC:
	case Tree.POSTDEC:
	case Tree.NULLCHK: return postfixPrec;
	case Tree.ASSIGN: return assignPrec;
	case Tree.BITOR_ASG:
	case Tree.BITXOR_ASG:
	case Tree.BITAND_ASG:
	case Tree.SL_ASG:
	case Tree.SR_ASG:
	case Tree.USR_ASG:
	case Tree.PLUS_ASG:
	case Tree.MINUS_ASG:
	case Tree.MUL_ASG:
	case Tree.DIV_ASG:
	case Tree.MOD_ASG: return assignopPrec;
        case Tree.OR: return orPrec;
        case Tree.AND: return andPrec;
        case Tree.EQ:
        case Tree.NE: return eqPrec;
        case Tree.LT:
        case Tree.GT:
        case Tree.LE:
        case Tree.GE: return ordPrec;
        case Tree.BITOR: return bitorPrec;
        case Tree.BITXOR: return bitxorPrec;
        case Tree.BITAND: return bitandPrec;
        case Tree.SL:
        case Tree.SR:
        case Tree.USR: return shiftPrec;
        case Tree.PLUS:
        case Tree.MINUS: return addPrec;
        case Tree.MUL:
        case Tree.DIV:
        case Tree.MOD: return mulPrec;
	case Tree.TYPETEST: return ordPrec;
	default: throw new AssertionError();
	}
    }
}



