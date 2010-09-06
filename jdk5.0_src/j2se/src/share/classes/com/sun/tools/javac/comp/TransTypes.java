/**
 * @(#)TransTypes.java	1.75 04/06/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.comp;

import java.util.*;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.util.List;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.tree.*;

import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.tree.Tree.*;
import com.sun.tools.javac.code.Type.*;

import static com.sun.tools.javac.code.Flags.*;
import static com.sun.tools.javac.code.Kinds.*;
import static com.sun.tools.javac.code.TypeTags.*;

/** This pass translates Generic Java to conventional Java.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class TransTypes extends TreeTranslator {
    /** The context key for the TransTypes phase. */
    protected static final Context.Key<TransTypes> transTypesKey =
	new Context.Key<TransTypes>();

    /** Get the instance for this context. */
    public static TransTypes instance(Context context) {
	TransTypes instance = context.get(transTypesKey);
	if (instance == null)
	    instance = new TransTypes(context);
	return instance;
    }

    private Name.Table names;
    private Log log;
    private Symtab syms;
    private TreeMaker make;
    private Enter enter;
    private boolean allowEnums;
    private Types types;

    protected TransTypes(Context context) {
	context.put(transTypesKey, this);
	names = Name.Table.instance(context);
	log = Log.instance(context);
	syms = Symtab.instance(context);
	enter = Enter.instance(context);
	overridden = new HashMap<MethodSymbol,MethodSymbol>();
	allowEnums = Source.instance(context).allowEnums();
	types = Types.instance(context);
        make = TreeMaker.instance(context);
    }

    /** A hashtable mapping bridge methods to the methods they override after
     *  type erasure.
     */
    Map<MethodSymbol,MethodSymbol> overridden;

    /** Construct an attributed tree for a cast of expression to target type,
     *  unless it already has precisely that type.
     *  @param tree    The expression tree.
     *  @param target  The target type.
     */
    Tree cast(Tree tree, Type target) {
	int oldpos = make.pos;
	make.at(tree.pos);
	if (!types.isSameType(tree.type, target)) {
	    tree = make.TypeCast(make.Type(target), tree).setType(target);
	}
	make.pos = oldpos;
	return tree;
    }

    /** Construct an attributed tree to coerce an expression to some erased
     *  target type, unless the expression is already assignable to that type.
     *  If target type is a constant type, use its base type instead.
     *  @param tree    The expression tree.
     *  @param target  The target type.
     */
    Tree coerce(Tree tree, Type target) {
	Type btarget = target.baseType();
	if (tree.type.isPrimitive() == target.isPrimitive()) {
	    return types.isAssignable(tree.type, btarget, Warner.noWarnings)
		? tree
		: cast(tree, btarget);
	}
	return tree;
    }

    /** Given an erased reference type, assume this type as the tree's type.
     *  Then, coerce to some given target type unless target type is null.
     *  This operation is used in situations like the following:
     *
     *  class Cell<A> { A value; }
     *  ...
     *  Cell<Integer> cell;
     *  Integer x = cell.value;
     *
     *  Since the erasure of Cell.value is Object, but the type
     *  of cell.value in the assignment is Integer, we need to
     *  adjust the original type of cell.value to Object, and insert
     *  a cast to Integer. That is, the last assignment becomes:
     *
     *  Integer x = (Integer)cell.value;
     *
     *  @param tree       The expression tree whose type might need adjustment.
     *  @param erasedType The expression's type after erasure.
     *  @param target     The target type, which is usually the erasure of the
     *                    expression's original type.
     */
    Tree retype(Tree tree, Type erasedType, Type target) {
//	System.err.println("retype " + tree + " to " + erasedType);//DEBUG
	if (erasedType.tag > lastBaseTag) {
	    if (target != null && target.isPrimitive())
		target = tree.type;
	    tree.type = erasedType;
	    if (target != null) return coerce(tree, target);
	}
	return tree;
    }

    /** Translate method argument list, casting each argument
     *  to its corresponding type in a list of target types.
     *  @param trees     The method argument list.
     *  @param targets   The list of target types.
     *  @param varargsElement The erasure of the varargs element type,
     *          or null if translating a non-varargs invocation
     */
    List<Tree> translateArgs(List<Tree> _args, List<Type> parameters, Type varargsElement) {
	if (parameters.isEmpty()) return _args;
	List<Tree> args = _args;
	while (parameters.tail.nonEmpty()) {
	    args.head = translate(args.head, parameters.head);
	    args = args.tail;
	    parameters = parameters.tail;
	}
	Type parameter = parameters.head;
	assert varargsElement != null || args.length() == 1;
	if (varargsElement != null) {
	    while (args.nonEmpty()) {
		args.head = translate(args.head, varargsElement);
		args = args.tail;
	    }
	} else {
	    args.head = translate(args.head, parameter);
	}
	return _args;
    }

    /** Add a bridge definition and enter corresponding method symbol in
     *  local scope of origin.
     *
     *  @param pos     The source code position to be used for the definition.
     *  @param meth    The method for which a bridge needs to be added
     *  @param impl    That method's implementation (possibly the method itself)
     *  @param origin  The class to which the bridge will be added
     *  @param hypothetical
     *                 True if the bridge method is not strictly necessary in the
     *                 binary, but is represented in the symbol table to detect
     *                 erasure clashes.
     *  @param bridges The list buffer to which the bridge will be added
     */
    void addBridge(int pos,
		   MethodSymbol meth,
		   MethodSymbol impl,
		   ClassSymbol origin,
                   boolean hypothetical,
		   ListBuffer<Tree> bridges) {
	make.at(pos);
	Type origType = types.memberType(origin.type, meth);
	Type origErasure = erasure(origType);

	// Create a bridge method symbol and a bridge definition without a body.
	Type bridgeType = meth.erasure(types);
        long flags = impl.flags() & AccessFlags | SYNTHETIC | BRIDGE;
        if (hypothetical) flags |= HYPOTHETICAL;
	MethodSymbol bridge = new MethodSymbol(flags,
                                               meth.name,
                                               bridgeType,
                                               origin);
        if (!hypothetical) {
            MethodDef md = make.MethodDef(bridge, null);

            // The bridge calls this.impl(..), if we have an implementation
            // in the current class, super.impl(...) otherwise.
            Tree receiver = (impl.owner == origin)
                ? make.This(origin.erasure(types))
                : make.Super(types.supertype(origin.type).tsym.erasure(types), origin);

            // The type returned from the original method.
            Type calltype = erasure(impl.type.restype());

            // Construct a call of  this.impl(params), or super.impl(params),
            // casting params and possibly results as needed.
            Tree call =
                make.Apply(
                           null,
                           make.Select(receiver, impl).setType(calltype),
                           translateArgs(make.Idents(md.params), origErasure.argtypes(), null))
                .setType(calltype);
            Tree stat = (origErasure.restype().tag == VOID)
                ? (Tree)make.Exec(call)
                : (Tree)make.Return(coerce(call, bridgeType.restype()));
            md.body = make.Block(0, List.make(stat));

            // Add bridge to `bridges' buffer
            bridges.append(md);
        }

	// Add bridge to scope of enclosing class and `overridden' table.
	origin.members().enter(bridge);
	overridden.put(bridge, meth);
    }

    /** Add bridge if given symbol is a non-private, non-static member
     *  of the given class, which is either defined in the class or non-final
     *  inherited, and one of the two following conditions holds:
     *  1. The method's type changes in the given class, as compared to the
     *     class where the symbol was defined, (in this case
     *     we have extended a parameterized class with non-trivial parameters).
     *  2. The method has an implementation with a different erased return type.
     *     (in this case we have used co-variant returns).
     *  If a bridge already exists in some other class, no new bridge is added.
     *  Instead, it is checked that the bridge symbol overrides the method symbol.
     *  (Spec ???).
     *  todo: what about bridges for privates???
     *
     *  @param pos     The source code position to be used for the definition.
     *  @param sym     The symbol for which a bridge might have to be added.
     *  @param origin  The class in which the bridge would go.
     *  @param bridges The list buffer to which the bridge would be added.
     */
    void addBridgeIfNeeded(int pos,
			   Symbol sym,
			   ClassSymbol origin,
			   ListBuffer<Tree> bridges) {
	if (sym.kind == MTH &&
	    sym.name != names.init &&
	    (sym.flags() & (PRIVATE | SYNTHETIC | STATIC)) == 0 &&
	    sym.isMemberOf(origin, types))
	{
	    MethodSymbol meth = (MethodSymbol)sym;
	    MethodSymbol bridge = meth.binaryImplementation(origin, types);
	    MethodSymbol impl = meth.implementation(origin, types, true);
	    if (bridge == null || bridge == meth || impl != null && !bridge.owner.isSubClass(impl.owner, types)) {
		// No bridge was added yet.
		if (impl != null &&
		    ((impl.flags() & FINAL) == 0 || impl.owner == origin) &&
		    ((impl != meth || (meth.flags() & ABSTRACT) == 0) &&
		     (!types.isSameType(erasure(types.memberType(origin.type, meth)), meth.erasure(types)) ||
                      !types.isSameType(erasure(types.memberType(origin.type, impl)), impl.erasure(types)))
		     ||
		     impl != meth &&
		     !types.isSameType(impl.erasure(types).restype(), meth.erasure(types).restype())))
		{
		    addBridge(pos, meth, impl, origin, bridge==impl, bridges);
		}
	    } else if ((bridge.flags() & SYNTHETIC) != 0) {
		MethodSymbol other = overridden.get(bridge);
		if (other != null && other != meth) {
		    if (impl == null || !impl.overrides(other, origin, types, true)) {
			// Bridge for other symbol pair was added
			log.error(pos, "name.clash.same.erasure.no.override",
				  other, other.location(origin.type, types),
				  meth,  meth.location(origin.type, types));
		    }
		}
	    } else if (!bridge.overrides(meth, origin, types, true)) {
		// Accidental binary override without source override.
                if (bridge.owner == origin ||
                    types.asSuper(bridge.owner.type, meth.owner) == null)
                    // Don't diagnose the problem if it would already
                    // have been reported in the superclass
                    log.error(pos, "name.clash.same.erasure.no.override",
                              bridge, bridge.location(origin.type, types),
                              meth,  meth.location(origin.type, types));
	    }
	}
    }

    void addBridges(int pos,
		    TypeSymbol i,
		    ClassSymbol origin,
		    ListBuffer<Tree> bridges) {
	for (Scope.Entry e = i.members().elems; e != null; e = e.sibling)
	    addBridgeIfNeeded(pos, e.sym, origin, bridges);
	for (List<Type> l = types.interfaces(i.type); l.nonEmpty(); l = l.tail)
	    addBridges(pos, l.head.tsym, origin, bridges);
    }

    /** Add all necessary bridges to some class appending them to list buffer.
     *  @param pos     The source code position to be used for the bridges.
     *  @param origin  The class in which the bridges go.
     *  @param bridges The list buffer to which the bridges are added.
     */
    void addBridges(int pos, ClassSymbol origin, ListBuffer<Tree> bridges) {
	Type st = types.supertype(origin.type);
	while (st.tag == CLASS) {
//	    if (isSpecialization(st))
	    addBridges(pos, st.tsym, origin, bridges);
	    st = types.supertype(st);
	}
	for (List<Type> l = types.interfaces(origin.type); l.nonEmpty(); l = l.tail)
//	    if (isSpecialization(l.head))
	    addBridges(pos, l.head.tsym, origin, bridges);
    }

/* ************************************************************************
 * Visitor methods
 *************************************************************************/

    /** Visitor argument: proto-type.
     */
    private Type pt;

    /** Visitor method: perform a type translation on tree.
     */
    public Tree translate(Tree tree, Type pt) {
	Type prevPt = this.pt;
	try {
	    this.pt = pt;
	    if (tree == null) result = null;
	    else tree.accept(this);
	} finally {
	    this.pt = prevPt;
	}
	return result;
    }

    /** Visitor method: perform a type translation on list of trees.
     */
    public List<Tree> translate(List<Tree> trees, Type pt) {
	Type prevPt = this.pt;
	List<Tree> res;
	try {
	    this.pt = pt;
	    res = translate(trees);
	} finally {
	    this.pt = prevPt;
	}
	return res;
    }

    public void visitClassDef(ClassDef tree) {
	translateClass(tree.sym);
	result = tree;
    }

    MethodDef currentMethod = null;
    public void visitMethodDef(MethodDef tree) {
	MethodDef previousMethod = currentMethod;
	try {
	    currentMethod = tree;
	    tree.restype = translate(tree.restype, null);
	    tree.typarams = TypeParameter.emptyList;
	    tree.params = translateVarDefs(tree.params);
	    tree.thrown = translate(tree.thrown, null);
	    tree.body = (Block)translate(tree.body, tree.sym.erasure(types).restype());
	    tree.type = erasure(tree.type);
	    result = tree;
	} finally {
	    currentMethod = previousMethod;
	}

	// Check that we do not introduce a name clash by erasing types.
	for (Scope.Entry e = tree.sym.owner.members().lookup(tree.name);
	     e.sym != null;
	     e = e.next()) {
	    if (e.sym != tree.sym &&
		types.isSameType(erasure(e.sym.type), tree.type)) {
		log.error(tree.pos,
			  "name.clash.same.erasure", tree.sym,
			  e.sym);
		return;
	    }
	}
    }

    public void visitVarDef(VarDef tree) {
	tree.vartype = translate(tree.vartype, null);
	tree.init = translate(tree.init, tree.sym.erasure(types));
	tree.type = erasure(tree.type);
	result = tree;
    }
	
    public void visitDoLoop(DoLoop tree) {
	tree.body = translate(tree.body);
	tree.cond = translate(tree.cond, syms.booleanType);
	result = tree;
    }

    public void visitWhileLoop(WhileLoop tree) {
	tree.cond = translate(tree.cond, syms.booleanType);
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitForLoop(ForLoop tree) {
	tree.init = translate(tree.init, null);
	if (tree.cond != null)
	    tree.cond = translate(tree.cond, syms.booleanType);
	tree.step = translate(tree.step, null);
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitForeachLoop(ForeachLoop tree) {
	tree.var = (VarDef)translate(tree.var, null);
	Type iterableType = tree.expr.type;
	tree.expr = translate(tree.expr, erasure(tree.expr.type));
	tree.expr.type = iterableType; // preserve type for Lower
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitSwitch(Switch tree) {
	Type selsuper = types.supertype(tree.selector.type);
	boolean enumSwitch = selsuper != null &&
	    selsuper.tsym == syms.enumSym;
	Type target = enumSwitch ? erasure(tree.selector.type) : syms.intType;
	tree.selector = translate(tree.selector, target);
	tree.cases = translateCases(tree.cases);
	result = tree;
    }

    public void visitCase(Case tree) {
	tree.pat = translate(tree.pat, null);
	tree.stats = translate(tree.stats);
	result = tree;
    }

    public void visitSynchronized(Synchronized tree) {
	tree.lock = translate(tree.lock, erasure(tree.lock.type));
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitConditional(Conditional tree) {
	tree.cond = translate(tree.cond, syms.booleanType);
	tree.truepart = translate(tree.truepart, erasure(tree.type));
	tree.falsepart = translate(tree.falsepart, erasure(tree.type));
	tree.type = erasure(tree.type);
	result = tree;
    }

   public void visitIf(If tree) {
	tree.cond = translate(tree.cond, syms.booleanType);
	tree.thenpart = translate(tree.thenpart);
	tree.elsepart = translate(tree.elsepart);
	result = tree;
    }

    public void visitExec(Exec tree) {
	tree.expr = translate(tree.expr, null);
	result = tree;
    }

    public void visitReturn(Return tree) {
	tree.expr = translate(tree.expr, currentMethod.sym.erasure(types).restype());
	result = tree;
    }

    public void visitThrow(Throw tree) {
	tree.expr = translate(tree.expr, erasure(tree.expr.type));
	result = tree;
    }

    public void visitAssert(Assert tree) {
	tree.cond = translate(tree.cond, syms.booleanType);
	if (tree.detail != null)
	    tree.detail = translate(tree.detail, erasure(tree.detail.type));
	result = tree;
    }

    public void visitApply(Apply tree) {
	tree.meth = translate(tree.meth, null);
	Symbol meth = TreeInfo.symbol(tree.meth);
	Type mt = meth.erasure(types);
	List<Type> argtypes = mt.argtypes();
        if (allowEnums &&
            meth.name==names.init &&
            meth.owner == syms.enumSym)
            argtypes = argtypes.tail.tail;
        if (tree.varargsElement != null)
            tree.varargsElement = types.erasure(tree.varargsElement);
        else
            assert tree.args.length() == argtypes.length();
	tree.args = translateArgs(tree.args, argtypes, tree.varargsElement);

	// Insert casts of method invocation results as needed.
	result = retype(tree, mt.restype(), pt);
    }

    public void visitNewClass(NewClass tree) {
	if (tree.encl != null)
	    tree.encl = translate(tree.encl, erasure(tree.encl.type));
	tree.clazz = translate(tree.clazz, null);
        if (tree.varargsElement != null)
            tree.varargsElement = types.erasure(tree.varargsElement);
	tree.args = translateArgs(
	    tree.args, tree.constructor.erasure(types).argtypes(), tree.varargsElement);
	tree.def = (ClassDef)translate(tree.def, null);
	tree.type = erasure(tree.type);
	result = tree;
    }

    public void visitNewArray(NewArray tree) {
	tree.elemtype = translate(tree.elemtype, null);
	translate(tree.dims, syms.intType);
	tree.elems = translate(tree.elems,
			       (tree.type == null) ? null
			       : erasure(types.elemtype(tree.type)));
	tree.type = erasure(tree.type);

	result = tree;
    }

    public void visitParens(Parens tree) {
	tree.expr = translate(tree.expr, pt);
	tree.type = erasure(tree.type);
	result = tree;
    }

    public void visitAssign(Assign tree) {
	tree.lhs = translate(tree.lhs, null);
	tree.rhs = translate(tree.rhs, erasure(tree.lhs.type));
	tree.type = erasure(tree.type);
	result = tree;
    }

    public void visitAssignop(Assignop tree) {
	tree.lhs = translate(tree.lhs, null);
	tree.rhs = translate(tree.rhs, erasure(tree.rhs.type));
	tree.type = erasure(tree.type);
	result = tree;
    }

    public void visitUnary(Unary tree) {
	tree.arg = translate(tree.arg, tree.operator.type.argtypes().head);
	result = tree;
    }

    public void visitBinary(Binary tree) {
	tree.lhs = translate(tree.lhs, tree.operator.type.argtypes().head);
	tree.rhs = translate(tree.rhs, tree.operator.type.argtypes().tail.head);
	result = tree;
    }

    public void visitTypeCast(TypeCast tree) {
	tree.clazz = translate(tree.clazz, null);
	tree.type = erasure(tree.type);
	tree.expr = translate(tree.expr, tree.type);
	result = tree;
    }

    public void visitTypeTest(TypeTest tree) {
	tree.expr = translate(tree.expr, null);
	tree.clazz = translate(tree.clazz, null);
	result = tree;
    }

    public void visitIndexed(Indexed tree) {
	tree.indexed = translate(tree.indexed, erasure(tree.indexed.type));
	tree.index = translate(tree.index, syms.intType);

	// Insert casts of indexed expressions as needed.
	result = retype(tree, types.elemtype(tree.indexed.type), pt);
    }

    // There ought to be nothing to rewrite here;
    // we don't generate code.
    public void visitAnnotation(Annotation tree) {
	result = tree;
    }

    public void visitIdent(Ident tree) {
	Type et = tree.sym.erasure(types);

	// Map type variables to their bounds.
	if (tree.sym.kind == TYP && tree.sym.type.tag == TYPEVAR) {
	    result = make.at(tree.pos).Type(et);
	} else
	// Map constants expressions to themselves.
	if (tree.type.constValue != null) {
	    result = tree;
	}
	// Insert casts of variable uses as needed.
	else if (tree.sym.kind == VAR) {
	    result = retype(tree, et, pt);
	}
	else {
	    tree.type = erasure(tree.type);
	    result = tree;
	}
    }

    public void visitSelect(Select tree) {
	Type t = tree.selected.type;
	if (t.tag == TYPEVAR && t.bound().isCompound()) {
	    if ((tree.sym.flags() & IPROXY) != 0) {
		tree.sym = ((MethodSymbol)tree.sym).
		    implemented((TypeSymbol)tree.sym.owner, types);
	    }
	    tree.selected = cast(
                translate(tree.selected, erasure(t)),
		erasure(tree.sym.owner.type));
	} else
	    tree.selected = translate(tree.selected, erasure(t));
	
	// Map constants expressions to themselves.
	if (tree.type.constValue != null) {
	    result = tree;
	}
	// Insert casts of variable uses as needed.
	else if (tree.sym.kind == VAR) {
	    result = retype(tree, tree.sym.erasure(types), pt);
	}
	else {
	    tree.type = erasure(tree.type);
	    result = tree;
	}
    }

    public void visitTypeArray(TypeArray tree) {
	tree.elemtype = translate(tree.elemtype, null);
	tree.type = erasure(tree.type);
	result = tree;
    }

    /** Visitor method for parameterized types.
     */
    public void visitTypeApply(TypeApply tree) {
	// Delete all type parameters.
	result = translate(tree.clazz, null);
    }

/**************************************************************************
 * utility methods
 *************************************************************************/

    private Type erasure(Type t) {
	return types.erasure(t);
    }

/**************************************************************************
 * main method
 *************************************************************************/

    void translateClass(ClassSymbol c) {
	Type st = types.supertype(c.type);

	// process superclass before derived
	if (st.tag == CLASS)
	    translateClass((ClassSymbol)st.tsym);

	Env<AttrContext> env = enter.typeEnvs.remove(c);
	if (env != null) {
	    // class has not been translated yet

	    TreeMaker savedMake = make;
	    Type savedPt = pt;
	    make = make.forToplevel(env.toplevel);
	    pt = null;
	    try {
		ClassDef tree = (ClassDef) env.tree;
		tree.typarams = TypeParameter.emptyList;
		super.visitClassDef(tree);
		make.at(tree.pos);

		ListBuffer<Tree> bridges = new ListBuffer<Tree>();
		if ((tree.sym.flags() & INTERFACE) == 0)
		    addBridges(tree.pos, tree.sym, bridges);
		tree.defs = bridges.toList().prependList(tree.defs);
		tree.type = erasure(tree.type);
	    } finally {
		make = savedMake;
		pt = savedPt;
	    }
	}
    }

    /** Translate a toplevel class definition.
     *  @param cdef    The definition to be translated.
     */
    public Tree translateTopLevelClass(Tree cdef, TreeMaker make) {
	// note that this method does NOT support recursion.
	this.make = make;
	pt = null;
	return translate(cdef, null);
    }
}

