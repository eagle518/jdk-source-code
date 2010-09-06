/**
 * @(#)Attr.java	1.176 04/07/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.comp;

import java.util.*;
import java.util.Set;

import com.sun.tools.javac.code.*;
import com.sun.tools.javac.jvm.*;
import com.sun.tools.javac.tree.*;
import com.sun.tools.javac.util.*;
import com.sun.tools.javac.util.List;

import com.sun.tools.javac.jvm.Target;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.tree.Tree.*;
import com.sun.tools.javac.code.Type.*;

import static com.sun.tools.javac.code.Flags.*;
import static com.sun.tools.javac.code.Kinds.*;
import static com.sun.tools.javac.code.TypeTags.*;

/** This is the main context-dependent analysis phase in GJC. It
 *  encompasses name resolution, type checking and constant folding as
 *  subtasks. Some subtasks involve auxiliary classes.
 *  @see Check
 *  @see Resolve
 *  @see ConstFold
 *  @see Infer
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Attr extends Tree.Visitor {
    protected static final Context.Key<Attr> attrKey =
	new Context.Key<Attr>();

    final Name.Table names;
    final Log log;
    final Symtab syms;
    final Resolve rs;
    final Check chk;
    final MemberEnter memberEnter;
    final TreeMaker make;
    final ConstFold cfolder;
    final Enter enter;
    final TreeInfo treeinfo;
    final Target target;
    final Types types;
    final Infer infer;
    final Annotate annotate;

    public static Attr instance(Context context) {
	Attr instance = context.get(attrKey);
	if (instance == null)
	    instance = new Attr(context);
	return instance;
    }

    protected Attr(Context context) {
	context.put(attrKey, this);

	names = Name.Table.instance(context);
	log = Log.instance(context);
	syms = Symtab.instance(context);
	rs = Resolve.instance(context);
	chk = Check.instance(context);
	memberEnter = MemberEnter.instance(context);
	make = TreeMaker.instance(context);
	enter = Enter.instance(context);
	cfolder = ConstFold.instance(context);
	treeinfo = TreeInfo.instance(context);
	target = Target.instance(context);
	types = Types.instance(context);
	infer = Infer.instance(context);
	annotate = Annotate.instance(context);

	Options options = Options.instance(context);

	Source source = Source.instance(context);
	allowGenerics = source.allowGenerics();
	allowVarargs = source.allowVarargs();
	allowEnums = source.allowEnums();
	allowBoxing = source.allowBoxing();
	relax = (options.get("-retrofit") != null ||
		 options.get("-relax") != null);
	lintSerial = options.lint("serial");
    }

    /** Switch: relax some constraints for retrofit mode.
     */
    boolean relax;

    /** Switch: support generics?
     */
    boolean allowGenerics;

    /** Switch: allow variable-arity methods.
     */
    boolean allowVarargs;

    /** Switch: support enums?
     */
    boolean allowEnums;

    /** Switch: support boxing and unboxing?
     */
    boolean allowBoxing;

    /** Switch: warn about serialVersionUID problems.
     */
    private boolean lintSerial;

    /** Check kind and type of given tree against protokind and prototype.
     *  If check succeeds, store type in tree and return it.
     *  If check fails, store errType in tree and return it.
     *  No checks are performed if the prototype is a method type.
     *  Its not necessary in this case since we know that kind and type
     *  are correct.
     *
     *  @param tree     The tree whose kind and type is checked
     *  @param owntype  The computed type of the tree
     *  @param ownkind  The computed kind of the tree
     *  @param pkind    The expected kind (or: protokind) of the tree
     *  @param pt       The expected type (or: prototype) of the tree
     */
    Type check(Tree tree, Type owntype, int ownkind, int pkind, Type pt) {
	if (owntype.tag != ERROR && pt.tag != METHOD && pt.tag != FORALL) {
	    if ((ownkind & ~pkind) == 0) {
		owntype = chk.checkType(tree.pos, owntype, pt);
	    } else {
		log.error(tree.pos, "unexpected.type",
			  Resolve.kindNames(pkind),
			  Resolve.kindName(ownkind));
		owntype = syms.errType;
	    }
	}
	tree.type = owntype;
	return owntype;
    }

    /** Is given blank final variable assignable, i.e. in a scope where it
     *  may be assigned to even though it is final?
     *  @param v      The blank final variable.
     *  @param env    The current environment.
     */
    boolean isAssignableAsBlankFinal(VarSymbol v, Env<AttrContext> env) {
	Symbol owner = env.info.scope.owner;
	   // owner refers to the innermost variable, method or
           // initializer block declaration at this point.
	return
	    v.owner == owner
	    ||
	    ((owner.name == names.init ||    // i.e. we are in a constructor
	      owner.kind == VAR ||           // i.e. we are in a variable initializer
	      (owner.flags() & BLOCK) != 0)  // i.e. we are in an initializer block
	     &&
	     v.owner == owner.owner
	     &&
	     ((v.flags() & STATIC) != 0) == Resolve.isStatic(env));
    }

    /** Check that variable can be assigned to.
     *  @param pos    The current source code position.
     *  @param v      The assigned varaible
     *  @param base   If the variable is referred to in a Select, the part
     *                to the left of the `.', null otherwise.
     *  @env          The current environment.
     */
    void checkAssignable(int pos, VarSymbol v, Tree base, Env<AttrContext> env) {
	if ((v.flags() & FINAL) != 0 &&
	    ((v.flags() & HASINIT) != 0
	     ||
	     !((base == null ||
	       (base.tag == Tree.IDENT && TreeInfo.name(base) == names._this)) &&
	       isAssignableAsBlankFinal(v, env)))) {
	    log.error(pos, "cant.assign.val.to.final.var", v);
	}
    }

    /** Does tree represent a static reference to an identifier?
     *  It is assumed that tree is either a SELECT or an IDENT.
     *  We have to weed out selects from non-type names here.
     *  @param tree    The candidate tree.
     */
    boolean isStaticReference(Tree tree) {
	if (tree.tag == Tree.SELECT) {
	    Symbol lsym = TreeInfo.symbol(((Select) tree).selected);
	    if (lsym == null || lsym.kind != TYP) {
		return false;
	    }
	}
	return true;
    }

    /** Is this symbol a type?
     */
    static boolean isType(Symbol sym) {
	return sym != null && sym.kind == TYP;
    }

    /** The current `this' symbol.
     *  @param env    The current environment.
     */
    Symbol thisSym(Env<AttrContext> env) {
	return rs.resolveSelf(
	    Position.NOPOS, env, env.enclClass.sym, names._this);
    }

/* ************************************************************************
 * Visitor methods
 *************************************************************************/

    /** Visitor argument: the current environment.
     */
    Env<AttrContext> env;

    /** Visitor argument: the currently expected proto-kind.
     */
    int pkind;

    /** Visitor argument: the currently expected proto-type.
     */
    Type pt;

    /** Visitor result: the computed type.
     */
    Type result;

    /** Visitor method: attribute a tree, catching any completion failure
     *  exceptions. Return the tree's type.
     *
     *  @param tree    The tree to be visited.
     *  @param env     The environment visitor argument.
     *  @param pkind   The protokind visitor argument.
     *  @param pt      The prototype visitor argument.
     */
    Type attribTree(Tree tree, Env<AttrContext> env, int pkind, Type pt) {
	Env<AttrContext> prevEnv = this.env;
	int prevPkind = this.pkind;
	Type prevPt = this.pt;
	try {
	    this.env = env;
	    this.pkind = pkind;
	    this.pt = pt;
	    tree.accept(this);
	    return result;
	} catch (CompletionFailure ex) {
	    tree.type = syms.errType;
	    return chk.completionError(tree.pos, ex);
	} finally {
	    this.env = prevEnv;
	    this.pkind = prevPkind;
	    this.pt = prevPt;
	}
    }

    /** Derived visitor method: attribute an expression tree.
     */
    Type attribExpr(Tree tree, Env<AttrContext> env, Type pt) {
	return attribTree(tree, env, VAL, pt);
    }

    /** Derived visitor method: attribute an expression tree with
     *  no constraints on the computed type.
     */
    Type attribExpr(Tree tree, Env<AttrContext> env) {
	return attribTree(tree, env, VAL, Type.noType);
    }

    /** Derived visitor method: attribute a type tree.
     */
    Type attribType(Tree tree, Env<AttrContext> env) {
	Type result = attribTree(tree, env, TYP, Type.noType);
	return result;
    }

    /** Derived visitor method: attribute a statement or definition tree.
     */
    Type attribStat(Tree tree, Env<AttrContext> env) {
	return attribTree(tree, env, NIL, Type.noType);
    }

    /** Attribute a list of expressions, returning a list of types.
     */
    List<Type> attribExprs(List<Tree> trees, Env<AttrContext> env, Type pt) {
	ListBuffer<Type> ts = new ListBuffer<Type>();
	for (List<Tree> l = trees; l.nonEmpty(); l = l.tail)
	    ts.append(attribExpr(l.head, env, pt));
	return ts.toList();
    }

    /** Attribute a list of statements, returning nothing.
     */
    <T extends Tree> void attribStats(List<T> trees, Env<AttrContext> env) {
	for (List<T> l = trees; l.nonEmpty(); l = l.tail)
	    attribStat(l.head, env);
    }

    /** Attribute the arguments in a method call, returning a list of types.
     */
    List<Type> attribArgs(List<Tree> trees, Env<AttrContext> env) {
	ListBuffer<Type> argtypes = new ListBuffer<Type>();
	for (List<Tree> l = trees; l.nonEmpty(); l = l.tail)
	    argtypes.append(chk.checkNonVoid(
		l.head.pos, attribTree(l.head, env, VAL, Infer.anyPoly)));
	return argtypes.toList();
    }

    /** Attribute a type argument list, returning a list of types.
     */
    List<Type> attribTypes(List<Tree> trees, Env<AttrContext> env) {
	if (trees == null)
	    return null;
	ListBuffer<Type> argtypes = new ListBuffer<Type>();
	for (List<Tree> l = trees; l.nonEmpty(); l = l.tail)
	    argtypes.append(chk.checkRefType(l.head.pos, attribType(l.head, env)));
	return argtypes.toList();
    }

    /** Attribute type reference in an `extends' or `implements' clause.
     *
     *  @param tree              The tree making up the type reference.
     *  @param env               The environment current at the reference.
     *  @param classExpected     true if only a class is expected here.
     *  @param interfaceExpected true if only an interface is expected here.
     */
    Type attribBase(Tree tree,
		    Env<AttrContext> env,
		    boolean classExpected,
		    boolean interfaceExpected,
		    boolean checkExtensible) {
	Type t = attribType(tree, env);
	if (t.tag == TYPEVAR && !classExpected && !interfaceExpected) {
	    // check that type variable is already visible
	    if (t.bound() == null) {
		log.error(tree.pos, "illegal.forward.ref");
		return syms.errType;
	    }
	} else {
	    t = chk.checkClassType(tree.pos, t, checkExtensible|!allowGenerics);
	}
	if (interfaceExpected & (t.tsym.flags() & INTERFACE) == 0) {
	    log.error(tree.pos, "intf.expected.here");
	    // return errType is necessary since otherwise there might
	    // be undetected cycles which cause attribution to loop
	    return syms.errType;
	} else if (checkExtensible &&
		   classExpected &&
		   (t.tsym.flags() & INTERFACE) != 0) {
	    log.error(tree.pos, "no.intf.expected.here");
	    return syms.errType;
	}
	if (checkExtensible &&
	    ((t.tsym.flags() & FINAL) != 0)) {
	    log.error(tree.pos,
		      "cant.inherit.from.final", t.tsym);
	}
	chk.checkNonCyclic(tree.pos, t);
	return t;
    }

    public void visitClassDef(ClassDef tree) {
	// Local classes have not been entered yet, so we need to do it now:
	if ((env.info.scope.owner.kind & (VAR | MTH)) != 0)
	    enter.classEnter(tree, env);

	ClassSymbol c = tree.sym;
	if (c == null) {
	    // exit in case something drastic went wrong during enter.
	    result = null;
	} else {
	    // make sure class has been completed:
	    c.complete();

	    // If this class appears as an anonymous class
	    // in a superclass constructor call where
	    // no explicit outer instance is given,
	    // disable implicit outer instance from being passed.
	    // (This would be an illegal access to "this before super").
	    if (env.info.isSelfCall &&
		env.tree.tag == Tree.NEWCLASS &&
		((NewClass)env.tree).encl == null)
	    {
		c.flags_field |= NOOUTERTHIS;
	    }
	    attribClass(tree.pos, c);
	    result = tree.type = c.type;
	}
    }

    public void visitMethodDef(MethodDef tree) {
	MethodSymbol m = tree.sym;
	chk.checkDeprecatedAnnotation(tree.pos, m);

	// If we override any other methods, check that we do so properly.
        // JLS ???
	chk.checkOverride(tree, m);

	// Create a new environment with local scope
        // for attributing the method.
	Env<AttrContext> localEnv = memberEnter.methodEnv((MethodDef)tree, env);

	// Enter all type parameters into the local method scope.
	for (List<TypeParameter> l = tree.typarams; l.nonEmpty(); l = l.tail)
	    localEnv.info.scope.enterIfAbsent(l.head.type.tsym);

	ClassSymbol owner = env.enclClass.sym;
	if ((owner.flags() & ANNOTATION) != 0 &&
	    tree.params.nonEmpty())
	    log.error(tree.params.head.pos, 
                      "intf.annotation.members.cant.have.params");

	// Attribute all value parameters.
	for (List<VarDef> l = tree.params; l.nonEmpty(); l = l.tail) {
	    attribStat(l.head, localEnv);
	}

	// Check that type parameters are well-formed.
	chk.validateTypeParams(tree.typarams);
	if ((owner.flags() & ANNOTATION) != 0 &&
	    tree.typarams.nonEmpty())
	    log.error(tree.typarams.head.pos, 
                      "intf.annotation.members.cant.have.type.params");

	// Check that result type is well-formed.
	chk.validate(tree.restype);
	if ((owner.flags() & ANNOTATION) != 0)
	    chk.validateAnnotationType(tree.restype);

	// Check that all exceptions mentioned in the throws clause extend
	// java.lang.Throwable.
	if ((owner.flags() & ANNOTATION) != 0 && tree.thrown.nonEmpty())
	    log.error(tree.thrown.head.pos, 
                      "throws.not.allowed.in.intf.annotation");
	for (List<Tree> l = tree.thrown; l.nonEmpty(); l = l.tail)
	    chk.checkType(l.head.pos, l.head.type, syms.throwableType);

	if (tree.body == null) {
	    // Empty bodies are only allowed for
            // abstract, native, or interface methods, or for methods
	    // in a retrofit signature class.
	    if ((owner.flags() & INTERFACE) == 0 &&
		(tree.mods.flags & (ABSTRACT | NATIVE)) == 0 &&
		!relax)
		log.error(tree.pos, "missing.meth.body.or.decl.abstract");
	    if (tree.defaultValue != null) {
		if ((owner.flags() & ANNOTATION) == 0)
		    log.error(tree.pos, 
                              "default.allowed.in.intf.annotation.member");
	    }
	} else if ((owner.flags() & INTERFACE) != 0) {
	    log.error(tree.body.pos, "intf.meth.cant.have.body");
	} else if ((tree.mods.flags & ABSTRACT) != 0) {
	    log.error(tree.pos, "abstract.meth.cant.have.body");
	} else if ((tree.mods.flags & NATIVE) != 0) {
	    log.error(tree.pos, "native.meth.cant.have.body");
	} else {
	    // Add an implicit super() call unless an explicit call to
	    // super(...) or this(...) is given
	    // or we are compiling class java.lang.Object.
	    if (tree.name == names.init && owner.type != syms.objectType) {
		Block body = tree.body;
		if (body.stats.isEmpty() ||
		    !TreeInfo.isSelfCall(body.stats.head)) {
		    body.stats = body.stats.
			prepend(memberEnter.SuperCall(make.at(body.pos),
						      Type.emptyList,
						      VarDef.emptyList,
						      false));
		} else if ((env.enclClass.sym.flags() & ENUM) != 0 &&
			   (tree.mods.flags & GENERATEDCONSTR) == 0 &&
			   TreeInfo.isSuperCall(body.stats.head)) {
		    // enum constructors are not allowed to call super
		    // directly, so make sure there aren't any super calls
		    // in enum constructors, except in the compiler
		    // generated one.
		    log.error(tree.body.stats.head.pos,"call.to.super.not.allowed.in.enum.ctor",
			      env.enclClass.sym);
		}
	    }

	    // Attribute method body.
	    attribStat(tree.body, localEnv);
	}
	localEnv.info.scope.leave();
	result = tree.type = m.type;
	chk.validateAnnotations(tree.mods.annotations, m);
    }

    public void visitVarDef(VarDef tree) {
	// Local variables have not been entered yet, so we need to do it now:
	if (env.info.scope.owner.kind == MTH) {
            if (tree.sym != null) {
                // parameters have already been entered
                env.info.scope.enter(tree.sym);
            } else {
                memberEnter.memberEnter(tree, env);
                annotate.flush();
            }
	}

	// Check that the variable's declared type is well-formed.
	chk.validate(tree.vartype);

	VarSymbol v = tree.sym;
	chk.checkDeprecatedAnnotation(tree.pos, v);

	if (tree.init != null) {
            // In order to catch self-references, we set the variable's
            // declaration position to maximal possible value, effectively
	    // marking the variable as undefined.
	    v.pos = Position.MAXPOS;

	    if ((v.flags_field & FINAL) != 0 && tree.init.tag != Tree.NEWCLASS) {
	        evalInit(v);
	    } else {
	        // Attribute initializer in a new environment
	        // with the declared variable as owner.
	        // Check that initializer conforms to variable's declared type.
	        Type itype = attribExpr(
		    tree.init, memberEnter.initEnv((VarDef)tree, env), v.type);
	    }
	    // Assign proper declared position.
	    v.pos = tree.pos;
	}
	result = tree.type = v.type;
	chk.validateAnnotations(tree.mods.annotations, v);
    }

    public void visitSkip(Skip tree) {
	result = null;
    }

    public void visitBlock(Block tree) {
	if (env.info.scope.owner.kind == TYP) {
	    // Block is a static or instance initializer;
	    // let the owner of the environment be a freshly
	    // created BLOCK-method.
	    Env<AttrContext> localEnv =
		env.dup(tree, env.info.dup(env.info.scope.dupUnshared()));
	    localEnv.info.scope.owner =
		new MethodSymbol(tree.flags | BLOCK, names.empty, null,
				 env.info.scope.owner);
	    if ((tree.flags & STATIC) != 0) localEnv.info.staticLevel++;
	    attribStats(tree.stats, localEnv);
	} else {
	    // Create a new local environment with a local scope.
	    Env<AttrContext> localEnv =
		env.dup(tree, env.info.dup(env.info.scope.dup()));
	    attribStats(tree.stats, localEnv);
	    localEnv.info.scope.leave();
	}
	result = null;
    }

    public void visitDoLoop(DoLoop tree) {
	attribStat(tree.body, env.dup(tree));
	attribExpr(tree.cond, env, syms.booleanType);
	result = null;
    }

    public void visitWhileLoop(WhileLoop tree) {
	attribExpr(tree.cond, env, syms.booleanType);
	attribStat(tree.body, env.dup(tree));
	result = null;
    }

    public void visitForLoop(ForLoop tree) {
	Env<AttrContext> loopEnv =
	    env.dup(env.tree, env.info.dup(env.info.scope.dup()));
	attribStats(tree.init, loopEnv);
	if (tree.cond != null) attribExpr(tree.cond, loopEnv, syms.booleanType);
	loopEnv.tree = tree; // before, we were not in loop!
	attribStats(tree.step, loopEnv);
	attribStat(tree.body, loopEnv);
	loopEnv.info.scope.leave();
	result = null;
    }

    public void visitForeachLoop(ForeachLoop tree) {
	Env<AttrContext> loopEnv =
	    env.dup(env.tree, env.info.dup(env.info.scope.dup()));
	attribStat(tree.var, loopEnv);
	Type exprType = types.upperBound(attribExpr(tree.expr, loopEnv));
	chk.checkNonVoid(tree.pos, exprType);
	Type elemtype = types.elemtype(exprType); // perhaps expr is an array?
	if (elemtype == null) {
	    // or perhaps expr implements Iterable<T>?
	    Type base = types.asSuper(exprType, syms.iterableType.tsym);
	    if (base == null) {
		log.error(tree.expr.pos, "foreach.not.applicable.to.type");
		elemtype = syms.errType;
	    } else {
		List<Type> iterableParams = base.allparams();
		elemtype = iterableParams.isEmpty()
		    ? syms.objectType
		    : types.upperBound(iterableParams.head);
	    }
	}
	chk.checkType(tree.expr.pos, elemtype, tree.var.sym.type);
	loopEnv.tree = tree; // before, we were not in loop!
	attribStat(tree.body, loopEnv);
	loopEnv.info.scope.leave();
	result = null;
    }

    public void visitLabelled(Labelled tree) {
	// Check that label is not used in an enclosing statement
	Env<AttrContext> env1 = env;
	while (env1 != null && env1.tree.tag != Tree.CLASSDEF) {
	    if (env1.tree.tag == Tree.LABELLED &&
		((Labelled) env1.tree).label == tree.label) {
		log.error(tree.pos, "label.already.in.use",
			  tree.label);
		break;
	    }
	    env1 = env1.next;
	}

	attribStat(tree.body, env.dup(tree));
	result = null;
    }

    public void visitSwitch(Switch tree) {
	Type seltype = attribExpr(tree.selector, env);

	Env<AttrContext> switchEnv =
	    env.dup(tree, env.info.dup(env.info.scope.dup()));

	boolean enumSwitch =
            allowEnums &&
            (seltype.tsym.flags() & Flags.ENUM) != 0;
	if (!enumSwitch)
	    seltype = chk.checkType(tree.selector.pos, seltype, syms.intType);

	// Attribute all cases and
	// check that there are no duplicate case labels or default clauses.
	Set<Object> labels = new HashSet<Object>(); // The set of case labels.
	boolean hasDefault = false;      // Is there a default label?
	for (List<Case> l = tree.cases; l.nonEmpty(); l = l.tail) {
	    Case c = l.head;
	    Env<AttrContext> caseEnv =
		switchEnv.dup(c, env.info.dup(switchEnv.info.scope.dup()));
	    if (c.pat != null) {
		if (enumSwitch) {
		    Symbol sym = enumConstant(c.pat, seltype);
		    if (sym == null) {
			log.error(c.pat.pos, "enum.const.req");
		    } else if (!labels.add(sym)) {
			log.error(c.pos, "duplicate.case.label");
		    }
		} else {
		    Type pattype = attribExpr(c.pat, switchEnv, seltype);
		    if (pattype.tag != ERROR) {
			if (pattype.constValue == null) {
			    log.error(c.pat.pos, "const.expr.req");
			} else if (labels.contains(pattype.constValue)) {
			    log.error(c.pos, "duplicate.case.label");
			} else {
			    labels.add(pattype.constValue);
			}
		    }
		}
	    } else if (hasDefault) {
		log.error(c.pos, "duplicate.default.label");
	    } else {
		hasDefault = true;
	    }
	    attribStats(c.stats, caseEnv);
	    caseEnv.info.scope.leave();
	    addVars(c.stats, switchEnv.info.scope);
	}

	switchEnv.info.scope.leave();
	result = null;
    }
    // where
        /** Add any variables defined in stats to the switch scope. */
        private static void addVars(List<Tree> stats, Scope switchScope) {
	    for (;stats.nonEmpty(); stats = stats.tail) {
		Tree stat = stats.head;
		if (stat.tag == Tree.VARDEF)
		    switchScope.enter(((VarDef)stat).sym);
	    }
	}
    // where
    /** Return the selected enumeration constant symbol, or null. */
    private Symbol enumConstant(Tree tree, Type enumType) {
	if (tree.tag != Tree.IDENT) {
	    log.error(tree.pos, "enum.label.must.be.unqualified.enum");
	    return syms.errSymbol;
	}
	Ident ident = (Ident)tree;
	Name name = ident.name;
	for (Scope.Entry e = enumType.tsym.members().lookup(name);
	     e.scope != null; e = e.next()) {
	    if (e.sym.kind == VAR) {
		Symbol s = ident.sym = e.sym;
                evalInit((VarSymbol)s);
		ident.type = s.type;
		return ((s.flags_field & Flags.ENUM) == 0)
		    ? null : s;
	    }
	}
	return null;
    }

    public void visitSynchronized(Synchronized tree) {
	chk.checkRefType(tree.pos, attribExpr(tree.lock, env));
	attribStat(tree.body, env);
	result = null;
    }

    public void visitTry(Try tree) {
	// Attribute body
	attribStat(tree.body, env.dup(tree, env.info.dup()));

	// Attribute catch clauses
	for (List<Catch> l = tree.catchers; l.nonEmpty(); l = l.tail) {
	    Catch c = l.head;
	    Env<AttrContext> catchEnv =
		env.dup(c, env.info.dup(env.info.scope.dup()));
	    Type ctype = attribStat(c.param, catchEnv);
	    chk.checkType(c.param.vartype.pos,
                          chk.checkClassType(c.param.vartype.pos, ctype),
                          syms.throwableType);
	    attribStat(c.body, catchEnv);
	    catchEnv.info.scope.leave();
	}

	// Attribute finalizer
	if (tree.finalizer != null) attribStat(tree.finalizer, env);
	result = null;
    }

    public void visitConditional(Conditional tree) {
	attribExpr(tree.cond, env, syms.booleanType);
	attribExpr(tree.truepart, env);
	attribExpr(tree.falsepart, env);
	result = check(tree,
		       capture(condType(tree.pos, tree.cond.type,
					tree.truepart.type, tree.falsepart.type)),
		       VAL, pkind, pt);
    }
    //where
        /** Compute the type of a conditional expression, after
	 *  checking that it exists. See Spec 15.25.
	 *
	 *  @param pos      The source position to be used for
	 *                  error diagnostics.
	 *  @param condtype The type of the expression's condition.
	 *  @param thentype The type of the expression's then-part.
	 *  @param elsetype The type of the expression's else-part.
	 */
        private Type condType(int pos,
			      Type condtype,
			      Type thentype,
			      Type elsetype) {
	    Type ctype = condType1(pos, condtype, thentype, elsetype);

  	    // If condition and both arms are numeric constants,
	    // evaluate at compile-time.
	    return ((condtype.constValue != null) &&
		    (thentype.constValue != null) &&
		    (elsetype.constValue != null))
		? cfolder.coerce(condtype.isTrue()?thentype:elsetype, ctype)
		: ctype;
	}
        /** Compute the type of a conditional expression, after
	 *  checking that it exists.  Does not take into
	 *  account the special case where condition and both arms
	 *  are constants.
	 *
	 *  @param pos      The source position to be used for error
	 *                  diagnostics.
	 *  @param condtype The type of the expression's condition.
	 *  @param thentype The type of the expression's then-part.
	 *  @param elsetype The type of the expression's else-part.
	 */
        private Type condType1(int pos, Type condtype,
			       Type thentype, Type elsetype) {
	    // If same type, that is the result
	    if (types.isSameType(thentype, elsetype))
		return thentype.baseType();

	    Type thenUnboxed = (!allowBoxing || thentype.isPrimitive())
		? thentype : types.unboxedType(thentype);
	    Type elseUnboxed = (!allowBoxing || elsetype.isPrimitive())
		? elsetype : types.unboxedType(elsetype);

	    // Otherwise, if both arms can be converted to a numeric
	    // type, return the least numeric type that fits both arms
	    // (i.e. return larger of the two, or return int if one
	    // arm is short, the other is char).
	    if (thenUnboxed.isPrimitive() && elseUnboxed.isPrimitive()) {
		// If one arm has an integer subrange type (i.e., byte,
		// short, or char), and the other is an integer constant
		// that fits into the subrange, return the subrange type.
		if (thenUnboxed.tag < INT && elseUnboxed.tag == INT &&
		    types.isAssignable(elseUnboxed, thenUnboxed))
		    return thenUnboxed.baseType();
		if (elseUnboxed.tag < INT && thenUnboxed.tag == INT &&
		    types.isAssignable(thenUnboxed, elseUnboxed))
		    return elseUnboxed.baseType();

		for (int i = BYTE; i < VOID; i++) {
                    Type candidate = syms.typeOfTag[i];
		    if (types.isSubType(thenUnboxed, candidate) &&
			types.isSubType(elseUnboxed, candidate))
			return candidate;
                }
	    }

	    // Those were all the cases that could result in a primitive
	    if (allowBoxing) {
		if (thentype.isPrimitive())
		    thentype = types.boxedClass(thentype).type;
		if (elsetype.isPrimitive())
		    elsetype = types.boxedClass(elsetype).type;
	    }

	    if (types.isSubType(thentype, elsetype)) return elsetype;
	    if (types.isSubType(elsetype, thentype)) return thentype;

	    if (!allowBoxing || thentype.tag == VOID || elsetype.tag == VOID) {
		log.error(pos, "neither.conditional.subtype",
			  thentype, elsetype);
		return thentype.baseType();
	    }

	    // both are known to be reference types.  The result is
	    // lub(thentype,elsetype). This cannot fail, as it will
	    // always be possible to infer "Object" if nothing better.
	    return types.lub(thentype, elsetype);
	}

    public void visitIf(If tree) {
	attribExpr(tree.cond, env, syms.booleanType);
	attribExpr(tree.thenpart, env, pt);
	if (tree.elsepart != null) attribExpr(tree.elsepart, env, pt);
	result = null;
    }

    public void visitExec(Exec tree) {
	attribExpr(tree.expr, env);
	result = null;
    }

    public void visitBreak(Break tree) {
	tree.target = findJumpTarget(tree.pos, tree.tag, tree.label, env);
	result = null;
    }

    public void visitContinue(Continue tree) {
	tree.target = findJumpTarget(tree.pos, tree.tag, tree.label, env);
	result = null;
    }
    //where
        /** Return the target of a break or continue statement, if it exists,
	 *  report an error if not.
	 *  Note: The target of a labelled break or continue is the
	 *  (non-labelled) statement tree referred to by the label,
	 *  not the tree representing the labelled statement itself.
	 *
	 *  @param pos     The position to be used for error diagnostics
	 *  @param tag     The tag of the jump statement. This is either
	 *                 Tree.BREAK or Tree.CONTINUE.
	 *  @param label   The label of the jump statement, or null if no
	 *                 label is given.
	 *  @param env     The environment current at the jump statement.
	 */
        private Tree findJumpTarget(int pos,
				    int tag,
				    Name label,
				    Env<AttrContext> env) {
	    // Search environments outwards from the point of jump.
	    Env<AttrContext> env1 = env;
	    LOOP:
	    while (env1 != null) {
		switch (env1.tree.tag) {
		case Tree.LABELLED:
		    Labelled labelled = (Labelled)env1.tree;
		    if (label == labelled.label) {
			// If jump is a continue, check that target is a loop.
			if (tag == Tree.CONTINUE) {
			    if (labelled.body.tag != Tree.DOLOOP &&
				labelled.body.tag != Tree.WHILELOOP &&
				labelled.body.tag != Tree.FORLOOP &&
				labelled.body.tag != Tree.FOREACHLOOP)
				log.error(pos, "not.loop.label", label);
			    // Found labelled statement target, now go inwards
			    // to next non-labelled tree.
			    return TreeInfo.referencedStatement(labelled);
			} else {
			    return labelled;
			}
		    }
		    break;
		case Tree.DOLOOP:
		case Tree.WHILELOOP:
		case Tree.FORLOOP:
		case Tree.FOREACHLOOP:
		    if (label == null) return env1.tree;
		    break;
		case Tree.SWITCH:
		    if (label == null && tag == Tree.BREAK) return env1.tree;
		    break;
		case Tree.METHODDEF:
		case Tree.CLASSDEF:
		    break LOOP;
		default:
		}
		env1 = env1.next;
	    }
	    if (label != null)
		log.error(pos, "undef.label", label);
	    else if (tag == Tree.CONTINUE)
		log.error(pos, "cont.outside.loop");
	    else
		log.error(pos, "break.outside.switch.loop");
	    return null;
	}

    public void visitReturn(Return tree) {
	// Check that there is an enclosing method which is
	// nested within than the enclosing class.
	if (env.enclMethod == null ||
	    env.enclMethod.sym.owner != env.enclClass.sym) {
	    log.error(tree.pos, "ret.outside.meth");

	} else {
	    // Attribute return expression, if it exists, and check that
	    // it conforms to result type of enclosing method.
	    Symbol m = env.enclMethod.sym;
	    if (m.type.restype().tag == VOID) {
		if (tree.expr != null)
		    log.error(tree.expr.pos,
			      "cant.ret.val.from.meth.decl.void");
	    } else if (tree.expr == null) {
		log.error(tree.pos, "missing.ret.val");
	    } else {
		attribExpr(tree.expr, env, m.type.restype());
	    }
	}
	result = null;
    }

    public void visitThrow(Throw tree) {
	Type t = attribExpr(tree.expr, env, syms.throwableType);
	result = null;
    }

    public void visitAssert(Assert tree) {
	Type ct = attribExpr(tree.cond, env, syms.booleanType);
	if (tree.detail != null) {
	    chk.checkNonVoid(tree.detail.pos, attribExpr(tree.detail, env));
	}
	result = null;
    }

     /** Visitor method for method invocations.
     *  NOTE: The method part of an application will have in its type field
     *        the return type of the method, not the method's type itself!
     */
    public void visitApply(Apply tree) {
	// The local environment of a method application is
	// a new environment nested in the current one.
	Env<AttrContext> localEnv = env.dup(tree, env.info.dup());

	// The types of the actual method arguments.
	List<Type> argtypes;

	// The types of the actual method type arguments.
	List<Type> typeargtypes = null;

	Name methName = TreeInfo.name(tree.meth);

	boolean isConstructorCall =
	    methName == names._this || methName == names._super;

	if (isConstructorCall) {
            // We are seeing a ...this(...) or ...super(...) call.
	    // Check that this is the first statement in a constructor.
	    if (checkFirstConstructorStat(tree, env)) {

		// Record the fact
		// that this is a constructor call (using isSelfCall).
		localEnv.info.isSelfCall = true;

		// Attribute arguments, yielding list of argument types.
		argtypes = attribArgs(tree.args, localEnv);
		typeargtypes = attribTypes(tree.typeargs, localEnv);

		// Variable `site' points to the class in which the called
		// constructor is defined.
		Type site = env.enclClass.sym.type;
		if (methName == names._super) site = types.supertype(site);

		if (site.tag == CLASS) {
		    if (site.outer().tag == CLASS) {
			// we are calling a nested class

			if (tree.meth.tag == Tree.SELECT) {
			    Tree qualifier = ((Select)tree.meth).selected;

			    // We are seeing a prefixed call, of the form
			    //     <expr>.super(...).
			    // Check that the prefix expression conforms
			    // to the outer instance type of the class.
			    chk.checkRefType(qualifier.pos,
					     attribExpr(qualifier, localEnv,
							site.outer()));
			} else if (methName == names._super) {
			    // qualifier omitted; check for existence
			    // of an appropriate implicit qualifier.
			    rs.resolveImplicitThis(tree.meth.pos,
						   localEnv, site);
			}
		    } else if (tree.meth.tag == Tree.SELECT) {
			log.error(tree.meth.pos, "illegal.qual.not.icls",
				  site.tsym);
		    }

		    // if we're calling a java.lang.Enum constructor,
		    // prefix the implicit String and int parameters
		    if (site.tsym == syms.enumSym && allowEnums)
			argtypes = argtypes.prepend(syms.intType).prepend(syms.stringType);

		    // Resolve the called constructor under the assumption
		    // that we are referring to a superclass instance of the
		    // current instance (JLS ???).
		    boolean selectSuperPrev = localEnv.info.selectSuper;
		    localEnv.info.selectSuper = true;
		    localEnv.info.varArgs = false;
		    Symbol sym = rs.resolveConstructor(
			tree.meth.pos, localEnv, site, argtypes, typeargtypes);
		    localEnv.info.selectSuper = selectSuperPrev;

		    // Set method symbol to resolved constructor...
 		    TreeInfo.setSymbol(tree.meth, sym);

		    // ...and check that it is legal in the current context.
		    // (this will also set the tree's type)
		    Type mpt = newMethTemplate(argtypes, typeargtypes);
		    checkId(tree.meth, site, sym, localEnv, MTH,
                            mpt, tree.varargsElement != null);
		}
		// Otherwise, `site' is an error type and we do nothing
	    }
	    result = tree.type = syms.voidType;
	} else {
	    // Otherwise, we are seeing a regular method call.
	    // Attribute the arguments, yielding list of argument types, ...
	    argtypes = attribArgs(tree.args, localEnv);
	    typeargtypes = attribTypes(tree.typeargs, localEnv);

	    // ... and attribute the method using as a prototype a methodtype
	    // whose formal argument types is exactly the list of actual
	    // arguments (this will also set the method symbol).
	    Type mpt = newMethTemplate(argtypes, typeargtypes);
	    localEnv.info.varArgs = false;
	    Type mtype = attribExpr(tree.meth, localEnv, mpt);
	    if (localEnv.info.varArgs)
                assert mtype.isErroneous() || tree.varargsElement != null;

	    // Compute the result type.
	    Type restype = mtype.restype();

            // The result encounters an rvalue-conversion, which turns
            // every "? extends T" into a "T".  This will be
            // unnecessary once we integrate more sane substitution
            // rules (4916563).
            restype = types.upperBound(restype);

	    // as a special case, array.clone() has a result that is
	    // the same as static type of the array being cloned
	    if (tree.meth.tag == Tree.SELECT &&
		methName == names.clone &&
		types.isArray(((Select)tree.meth).selected.type))
		restype = ((Select)tree.meth).selected.type;

	    // as a special case, x.getClass() has type Class<? extends |X|>
	    if (allowGenerics &&
		methName == names.getClass && tree.args.isEmpty()) {
		Type qualifier = (tree.meth.tag == Tree.SELECT)
		    ? ((Select)tree.meth).selected.type
		    : env.enclClass.sym.type;
		restype = new
		    ClassType(restype.outer(),
			      Type.emptyList.prepend
			      (new ArgumentType(types.erasure(qualifier),
						BoundKind.EXTENDS,
						syms.boundClass)),
			      restype.tsym);
	    }

	    // Check that value of resulting type is admissible in the
	    // current context.  Also, capture the return type
	    result = check(tree, capture(restype), VAL, pkind, pt);
	}
	if (tree.typeargs != null) chk.validate(tree.typeargs);
    }
    //where
        /** Check that given application node appears as first statement
	 *  in a constructor call.
	 *  @param tree   The application node
	 *  @param env    The environment current at the application.
	 */
        boolean checkFirstConstructorStat(Apply tree, Env<AttrContext> env) {
	    MethodDef enclMethod = env.enclMethod;
	    if (enclMethod != null && enclMethod.name == names.init) {
		Block body = (Block)enclMethod.body;
		if (body.stats.head.tag == Tree.EXEC &&
		    ((Exec)body.stats.head).expr == tree)
		    return true;
	    }
	    log.error(tree.pos,"call.must.be.first.stmt.in.ctor",
		      TreeInfo.name(tree.meth));
	    return false;
	}

        /** Obtain a method type with given argument types.
	 */
        Type newMethTemplate(List<Type> argtypes, List<Type> typeargtypes) {
            MethodType mt = new MethodType(argtypes, null, null, syms.methodClass);
	    return (typeargtypes == null) ? mt : (Type)new ForAll(typeargtypes, mt);
	}

    public void visitNewClass(NewClass tree) {
	Type owntype = syms.errType;

	// The local environment of a class creation is
	// a new environment nested in the current one.
	Env<AttrContext> localEnv = env.dup(tree, env.info.dup());

	// The anonymous inner class definition of the new expression,
	// if one is defined by it.
	ClassDef cdef = tree.def;

	// Tf enclosing class is given, attribute it, and
	// complete class name to be fully qualified
	Tree clazz = tree.clazz; // Class field following new
	Tree clazzid =		// Identifier in class field
	    (clazz.tag == Tree.TYPEAPPLY)
	    ? ((TypeApply)clazz).clazz
	    : clazz;

	Tree clazzid1 = clazzid; // The same in fully qualified form

	if (tree.encl != null) {
	    // We are seeing a qualified new, of the form
	    //    <expr>.new C <...> (...) ...
	    // In this case, we we let clazz stand for the name of the
	    // allocated class C prefixed with the type of the qualifier
	    // expression, so that we can
	    // resolve it with standard techniques later. I.e., if
	    // <expr> has type T, then <expr>.new C <...> (...)
	    // yields a clazz T.C.
	    Type encltype = chk.checkRefType(tree.encl.pos,
					     attribExpr(tree.encl, env));
	    clazzid1 = make.at(clazz.pos).Select(make.Type(encltype),
						 ((Ident)clazzid).name);
	    if (clazz.tag == Tree.TYPEAPPLY)
		clazz = make.at(tree.pos).
		    TypeApply(clazzid1,
			      ((TypeApply)clazz).arguments);
	    else
		clazz = clazzid1;
//	    System.out.println(clazz + " generated.");//DEBUG
	}

	// Attribute clazz expression and store
        // symbol + type back into the attributed tree.
	Type clazztype = chk.checkClassType(
	    tree.clazz.pos, attribType(clazz, env), true);
	chk.validate(clazz);
	if (tree.encl != null) {
	    // We have to work in this case to store
	    // symbol + type back into the attributed tree.
	    tree.clazz.type = clazztype;
	    TreeInfo.setSymbol(clazzid, TreeInfo.symbol(clazzid1));
	    clazzid.type = ((Ident)clazzid).sym.type;
	    if ((clazztype.tsym.flags() & STATIC) != 0 &&
		!clazztype.isErroneous()) {
		log.error(tree.pos, "qualified.new.of.static.class",
			  clazztype.tsym);
	    }
	} else if ((clazztype.tsym.flags() & INTERFACE) == 0 &&
		   clazztype.outer().tag == CLASS) {
	    // Check for the existence of an apropos outer instance
	    rs.resolveImplicitThis(tree.pos, env, clazztype);
	}

	// Attribute constructor arguments.
	List<Type> argtypes = attribArgs(tree.args, localEnv);
	List<Type> typeargtypes = attribTypes(tree.typeargs, localEnv);

	// If we have made no mistakes in the class type...
	if (clazztype.tag == CLASS) {
	    // Enums may not be instantiated except implicitly
	    if (allowEnums &&
		(clazztype.tsym.flags_field&Flags.ENUM) != 0 &&
		(env.tree.tag != Tree.VARDEF ||
		 (((VarDef)env.tree).mods.flags&Flags.ENUM) == 0 ||
		 ((VarDef)env.tree).init != tree))
		log.error(tree.pos, "enum.cant.be.instantiated");
	    // Check that class is not abstract
	    if (cdef == null &&
		(clazztype.tsym.flags() & (ABSTRACT | INTERFACE)) != 0) {
		log.error(tree.pos, "abstract.cant.be.instantiated",
			  clazztype.tsym);
	    }
	    // If we are seeing an anonymous class that implements an interface,
	    // check that no outer instance or constructor arguments are given.
	    else if (cdef != null && (clazztype.tsym.flags() & INTERFACE) != 0) {
		if (argtypes.nonEmpty()) {
		    log.error(tree.pos, "anon.class.impl.intf.no.args");
		    argtypes = Type.emptyList;
		} else if (tree.encl != null) {
		    log.error(tree.pos, "anon.class.impl.intf.no.qual.for.new");
		}
	    }

	    // Resolve the called constructor under the assumption
	    // that we are referring to a superclass instance of the
	    // current instance (JLS ???).
	    else {
		localEnv.info.selectSuper = cdef != null;
		localEnv.info.varArgs = false;
		tree.constructor = rs.resolveConstructor(
		    tree.pos, localEnv, clazztype, argtypes, typeargtypes);
                Type ctorType = checkMethod(clazztype,
                                            tree.constructor,
                                            localEnv,
                                            tree.args,
                                            argtypes,
                                            typeargtypes,
                                            localEnv.info.varArgs);
		if (localEnv.info.varArgs)
                    assert ctorType.isErroneous() || tree.varargsElement != null;
	    }

	    if (cdef != null) {
		// We are seeing an anonymous class instance creation.
		// In this case, the class instance creation
		// expression
		//
		//    E.new <typeargs1>C<typargs2>(args) { ... }
		//
		// is represented internally as
		//
		//    E . new <typeargs1>C<typargs2>(args) ( class <empty-name> { ... } )  .
		//
		// This expression is then *transformed* as follows:
		//
		// (1) add a STATIC flag to the class definition
		//     if the current environment is static
		// (2) add an extends or implements clause
		// (3) add a constructor.
		//
		// For instance, if C is a class, and ET is the type of E,
		// the expression
		//
		//    E.new <typeargs1>C<typargs2>(args) { ... }
		//
		// is translated to (where X is a fresh name and typarams is the
		// parameter list of the super constructor):
		//
		//   new <typeargs1>X(<*nullchk*>E, args) where
		//     X extends C<typargs2> {
		//       <typarams> X(ET e, args) {
		//         e.<typeargs1>super(args)
		//       }
		//       ...
		//     }
		if (Resolve.isStatic(env)) cdef.mods.flags |= STATIC;

		if ((clazztype.tsym.flags() & INTERFACE) != 0) {
		    cdef.implementing = List.make(clazz);
		} else {
		    cdef.extending = clazz;
		}

		attribStat(cdef, localEnv);

		// If an outer instance is given,
		// prefix it to the constructor arguments
		// and delete it from the new expression
		if (tree.encl != null) {
		    tree.args = tree.args.prepend(makeNullCheck(tree.encl));
		    argtypes = argtypes.prepend(tree.encl.type);
		    tree.encl = null;
		}

		// Reassign clazztype and recompute constructor.
		clazztype = cdef.sym.type;
		Symbol sym = rs.resolveConstructor(
		    tree.pos, localEnv, clazztype, argtypes,
                    typeargtypes, true, tree.varargsElement != null);
		assert sym.kind < AMBIGUOUS || tree.constructor.type.isErroneous();
		tree.constructor = sym;
	    }

	    if (tree.constructor != null && tree.constructor.kind == MTH)
		owntype = clazztype;
	}
	result = check(tree, owntype, VAL, pkind, pt);
	if (tree.typeargs != null) chk.validate(tree.typeargs);
    }

    /** Make an attributed null check tree.
     */
    public Tree makeNullCheck(Tree arg) {
	// optimization: X.this is never null; skip null check
	Name name = TreeInfo.name(arg);
	if (name == names._this || name == names._super) return arg;

	int optag = Tree.NULLCHK;
	Unary tree = make.at(arg.pos).Unary(optag, arg);
	tree.operator = syms.nullcheck;
	tree.type = arg.type;
	return tree;
    }

    public void visitNewArray(NewArray tree) {
	Type owntype = syms.errType;
	Type elemtype;
	if (tree.elemtype != null) {
	    elemtype = attribType(tree.elemtype, env);
	    chk.validate(tree.elemtype);
	    owntype = elemtype;
	    for (List<Tree> l = tree.dims; l.nonEmpty(); l = l.tail) {
		attribExpr(l.head, env, syms.intType);
		owntype = new ArrayType(owntype, syms.arrayClass);
	    }
	} else {
	    // we are seeing an untyped aggregate { ... }
	    // this is allowed only if the prototype is an array
	    if (pt.tag == ARRAY) {
		elemtype = types.elemtype(pt);
	    } else {
		if (pt.tag != ERROR) {
		    log.error(tree.pos, "illegal.initializer.for.type",
			      pt);
		}
		elemtype = syms.errType;
	    }
	}
	if (tree.elems != null) {
	    // VGJ: Checking of the individual elements of an
	    // initializer list is treated as a sequence of array
	    // index assignments. That means that the initializer of a
	    // T[+] array will not typecheck if we don't use the upper
	    // bound instead of the real type.
	    attribExprs(tree.elems, env, types.upperBound(elemtype));
	    owntype = new ArrayType(elemtype, syms.arrayClass);
	}
        if (!types.isReifiable(elemtype))
            log.error(tree.pos, "generic.array.creation");
	result = check(tree, owntype, VAL, pkind, pt);
    }

    public void visitParens(Parens tree) {
	Type owntype = attribTree(tree.expr, env, pkind, pt);
	result = check(tree, owntype, pkind, pkind, pt);
	Symbol sym = TreeInfo.symbol(tree);
	if (sym != null && (sym.kind&(TYP|PCK)) != 0)
	    log.error(tree.pos, "illegal.start.of.type");
    }

    public void visitAssign(Assign tree) {
	Type owntype = attribTree(tree.lhs, env.dup(tree), VAR, Type.noType);
	Type capturedType = capture(owntype);
	attribExpr(tree.rhs, env, owntype);
	result = check(tree, capturedType, VAL, pkind, pt);
    }

    public void visitAssignop(Assignop tree) {
	// Attribute arguments.
	Type owntype = attribTree(tree.lhs, env, VAR, Type.noType);

	// Find operator.
	Symbol operator = tree.operator = rs.resolveBinaryOperator(
	    tree.pos, tree.tag - Tree.ASGOffset, env,
	    owntype, attribExpr(tree.rhs, env));

	if (operator.kind == MTH) {
	    if (types.isSameType(operator.type.restype(), syms.stringType)) {
		// String assignment; make sure the lhs is a string
		chk.checkType(tree.lhs.pos,
			      owntype,
			      syms.stringType);
	    } else {
		chk.checkCastable(tree.rhs.pos,
				  operator.type.restype(),
				  owntype);
	    }
	}
	result = check(tree, owntype, VAL, pkind, pt);
    }

    public void visitUnary(Unary tree) {
	// Attribute arguments.
	Type argtype = (Tree.PREINC <= tree.tag && tree.tag <= Tree.POSTDEC)
	    ? attribTree(tree.arg, env, VAR, Type.noType)
	    : chk.checkNonVoid(tree.arg.pos, attribExpr(tree.arg, env));

	// Find operator.
	Symbol operator = tree.operator =
	    rs.resolveUnaryOperator(tree.pos, tree.tag, env, argtype);

	Type owntype = syms.errType;
	if (operator.kind == MTH) {
	    owntype = (Tree.PREINC <= tree.tag && tree.tag <= Tree.POSTDEC)
		? tree.arg.type
		: operator.type.restype();
	    int opc = ((OperatorSymbol)operator).opcode;

	    // If the argument is constant, fold it.
	    if (argtype.constValue != null) {
		Type ctype = cfolder.fold1(opc, argtype);
		if (ctype != null) {
		    owntype = cfolder.coerce(ctype, owntype);

		    // Remove constant types from arguments to
		    // conserve space. The parser will fold concatenations
		    // of string literals; the code here also
		    // gets rid of intermediate results when some of the
		    // operands are constant identifiers.
		    if (tree.arg.type.tsym == syms.stringType.tsym) {
			tree.arg.type = syms.stringType;
		    }
		}
	    }
	}
	result = check(tree, owntype, VAL, pkind, pt);
    }

    public void visitBinary(Binary tree) {
	// Attribute arguments.
	Type left = chk.checkNonVoid(tree.lhs.pos, attribExpr(tree.lhs, env));
	Type right = chk.checkNonVoid(tree.lhs.pos, attribExpr(tree.rhs, env));

	// Find operator.
	Symbol operator = tree.operator =
	    rs.resolveBinaryOperator(tree.pos, tree.tag, env, left, right);

	Type owntype = syms.errType;
	if (operator.kind == MTH) {
	    owntype = operator.type.restype();
	    int opc = ((OperatorSymbol)operator).opcode;

	    if (opc == ByteCodes.error) {
		log.error(tree.lhs.pos,
			  "operator.cant.be.applied",
			  treeinfo.operatorName(tree.tag),
			  left + "," + right);
	    }

	    // If both arguments are constants, fold them.
	    if (left.constValue != null && right.constValue != null) {
		Type ctype = cfolder.fold2(opc, left, right);
		if (ctype != null) {
		    owntype = cfolder.coerce(ctype, owntype);

		    // Remove constant types from arguments to
		    // conserve space. The parser will fold concatenations
		    // of string literals; the code here also
		    // gets rid of intermediate results when some of the
		    // operands are constant identifiers.
		    if (tree.lhs.type.tsym == syms.stringType.tsym) {
			tree.lhs.type = syms.stringType;
		    }
		    if (tree.rhs.type.tsym == syms.stringType.tsym) {
			tree.rhs.type = syms.stringType;
		    }
		}
	    }

	    // Check that argument types of a reference ==, != are
	    // castable to each other, (JLS???).
	    if ((opc == ByteCodes.if_acmpeq || opc == ByteCodes.if_acmpne)) {
		if (!types.isCastable(left, right, new Warner(tree.pos))) {
		    log.error(tree.pos, "incomparable.types", left, right);
		}
	    }
	}
	result = check(tree, owntype, VAL, pkind, pt);
    }

    public void visitTypeCast(TypeCast tree) {
	Type clazztype = attribType(tree.clazz, env);
	Type exprtype = attribExpr(tree.expr, env, Infer.anyPoly);
	Type owntype = chk.checkCastable(tree.expr.pos, exprtype, clazztype);
	if (exprtype.constValue != null)
	    owntype = cfolder.coerce(exprtype, owntype);
	result = check(tree, capture(owntype), VAL, pkind, pt);
    }

    public void visitTypeTest(TypeTest tree) {
	Type exprtype = chk.checkNullOrRefType(
            tree.expr.pos, attribExpr(tree.expr, env));
	Type clazztype = chk.checkReifiableReferenceType(
	    tree.clazz.pos, attribType(tree.clazz, env));
	chk.checkCastable(tree.expr.pos, exprtype, clazztype);
	result = check(tree, syms.booleanType, VAL, pkind, pt);
    }

    public void visitIndexed(Indexed tree) {
	Type owntype = syms.errType;
	Type atype = attribExpr(tree.indexed, env);
	attribExpr(tree.index, env, syms.intType);
	if (types.isArray(atype))
	    owntype = types.elemtype(atype);
	else if (atype.tag != ERROR)
	    log.error(tree.pos, "array.req.but.found", atype);
        if ((pkind & VAR) == 0) owntype = capture(owntype);
	result = check(tree, owntype, VAR, pkind, pt);
    }

    public void visitIdent(Ident tree) {
	Symbol sym;
	boolean varArgs = false;

	// Find symbol
	if (pt.tag == METHOD || pt.tag == FORALL) {
	    // If we are looking for a method, the prototype `pt' will be a
	    // method type with the type of the call's arguments as parameters.
	    env.info.varArgs = false;
	    sym = rs.resolveMethod(tree.pos, env, tree.name, pt.argtypes(), pt.typarams());
	    varArgs = env.info.varArgs;
	} else if (tree.sym != null && tree.sym.kind != VAR) {
	    sym = tree.sym;
	} else {
	    sym = rs.resolveIdent(tree.pos, env, tree.name, pkind);
	}
	tree.sym = sym;

	// (1) Also find the environment current for the class where
        //     sym is defined (`symEnv').
	Env<AttrContext> symEnv = env;
	if (env.enclClass.sym.owner.kind != PCK && // we are in an inner class
	    (sym.kind & (VAR | MTH | TYP)) != 0 &&
	    sym.owner.kind == TYP &&
	    tree.name != names._this && tree.name != names._super) {

	    // Find environment in which identifier is defined.
	    while (symEnv.outer != null &&
		   !sym.isMemberOf(symEnv.enclClass.sym, types))
		symEnv = symEnv.outer;
	}

	// If symbol is a variable, ...
	if (sym.kind == VAR) {
	    VarSymbol v = (VarSymbol)sym;

	    // ..., evaluate its initializer, if it has one, and check for
	    // illegal forward reference.
	    checkInit(tree, env, v);

	    // If symbol is a local variable accessed from an embedded
	    // inner class check that it is final.
	    if (v.owner.kind == MTH &&
		v.owner != env.info.scope.owner &&
		(v.flags_field & FINAL) == 0) {
		log.error(tree.pos,
			  "local.var.accessed.from.icls.needs.final",
			  v);
	    }

	    // If we are expecting a variable (as opposed to a value), check
	    // that the variable is assignable in the current environment.
	    if (pkind == VAR)
		checkAssignable(tree.pos, v, null, env);
	}

	// In a constructor body,
        // if symbol is a field or instance method, check that it is
	// not accessed before the supertype constructor is called.
	if (symEnv.info.isSelfCall &&
	    (sym.kind & (VAR | MTH)) != 0 &&
	    sym.owner.kind == TYP &&
	    (sym.flags() & STATIC) == 0) {
	    chk.earlyRefError(tree.pos, sym.kind == VAR ? sym : thisSym(env));
	}
	result = checkId(tree, env.enclClass.sym.type, sym, env, pkind, pt, varArgs);
    }

    public void visitSelect(Select tree) {
	// Determine the expected kind of the qualifier expression.
	int skind = 0;
	if (tree.name == names._this || tree.name == names._super ||
	    tree.name == names._class)
	{
	    skind = TYP;
	} else {
	    if ((pkind & PCK) != 0) skind = skind | PCK;
	    if ((pkind & TYP) != 0) skind = skind | TYP | PCK;
	    if ((pkind & (VAL | MTH)) != 0) skind = skind | VAL | TYP;
	}

	// Attribute the qualifier expression, and determine its symbol (if any).
	Type site = attribTree(tree.selected, env, skind, Infer.anyPoly);
	site = capture(site); // Capture field access

	// don't allow T.class T[].class, etc
        if (skind == TYP) {
            Type elt = site;
            while (elt.tag == ARRAY)
                elt = ((ArrayType)elt).elemtype;
            if (elt.tag == TYPEVAR) {
                log.error(tree.pos, "type.var.cant.be.deref");
                result = syms.errType;
                return;
            }
        }

	// If qualifier symbol is a type or `super', assert `selectSuper'
	// for the selection. This is relevant for determining whether
	// protected symbols are accessible.
	Symbol sitesym = TreeInfo.symbol(tree.selected);
	boolean selectSuperPrev = env.info.selectSuper;
	env.info.selectSuper =
	    sitesym != null &&
	    sitesym.name == names._super;

	// If selected expression is polymorphic, strip
	// type parameters and remember in env.info.tvars, so that
	// they can be added later (in Attr.checkId and Infer.instantiateMethod).
	if (tree.selected.type.tag == FORALL) {
	    ForAll pstype = (ForAll)tree.selected.type;
	    env.info.tvars = pstype.tvars;
	    site = tree.selected.type = pstype.qtype;
	}

	// Determine the symbol represented by the selection.
	env.info.varArgs = false;
	Symbol sym = selectSym(tree, site, env, pt, pkind);
	boolean varArgs = env.info.varArgs;
	tree.sym = sym;

	// If that symbol is a variable, ...
	if (sym.kind == VAR) {
	    // ..., evaluate its initializer, if it has one
	    VarSymbol v = (VarSymbol)sym;
	    evalInit(v);

	    // If we are expecting a variable (as opposed to a value), check
	    // that the variable is assignable in the current environment.
	    if (pkind == VAR)
		checkAssignable(tree.pos, v, tree.selected, env);
	}

	// Disallow selecting a type from an expression
	if (isType(sym) && (sitesym==null || (sitesym.kind&(TYP|PCK)) == 0)) {
	    tree.type = check(tree.selected, pt,
			      sitesym == null ? VAL : sitesym.kind, TYP|PCK, pt);
	}

	if (isType(sitesym)) {
	    if (sym.name == names._this) {
		// If `C' is the currently compiled class, check that
		// C.this' does not appear in a call to a super(...)
		if (env.info.isSelfCall &&
		    site.tsym == env.enclClass.sym) {
		    chk.earlyRefError(tree.pos, sym);
		}
	    } else {
		// Check if type-qualified fields or methods are static (JLS)
		if ((sym.flags() & STATIC) == 0 &&
		    sym.name != names._super &&
		    (sym.kind == VAR || sym.kind == MTH)) {
		    rs.access(rs.new StaticError(sym),
			      tree.pos, site, sym.name, true);
		}
	    }
	}

	// If we are selecting an instance member via a `super', ...
	if (env.info.selectSuper && (sym.flags() & STATIC) == 0) {

	    // Check that super-qualified symbols are not abstract (JLS)
	    rs.checkNonAbstract(tree.pos, sym);

	    if (site.isRaw()) {
		// Determine argument types for site.
		Type site1 = types.asSuper(env.enclClass.sym.type, site.tsym);
		if (site1 != null) site = site1;
	    }
	}

	env.info.selectSuper = selectSuperPrev;
	result = checkId(tree, site, sym, env, pkind, pt, varArgs);
	env.info.tvars = Type.emptyList;
    }
    //where
        /** Determine symbol referenced by a Select expression,
	 *
	 *  @param tree   The select tree.
	 *  @param site   The type of the selected expression,
	 *  @param env    The current environment.
	 *  @param pt     The current prototype.
	 *  @param pkind  The expected kind(s) of the Select expression.
	 */
        private Symbol selectSym(Select tree,
				 Type site,
				 Env<AttrContext> env,
				 Type pt,
				 int pkind) {
	    int pos = tree.pos;
	    Name name = tree.name;

	    switch (site.tag) {
	    case PACKAGE:
		return rs.access(
		    rs.findIdentInPackage(env, site.tsym, name, pkind),
		    pos, site, name, true);
	    case ARRAY:
	    case CLASS:
		if (pt.tag == METHOD || pt.tag == FORALL) {
		    return rs.resolveQualifiedMethod(
			pos, env, site, name, pt.argtypes(), pt.typarams());
		} else if (name == names._this || name == names._super) {
		    return rs.resolveSelf(pos, env, site.tsym, name);
		} else if (name == names._class) {
		    // In this case, we have already made sure in
		    // visitSelect that qualifier expression is a type.
		    Type t = syms.classType;
		    List<Type> typeargs = allowGenerics
			? Type.emptyList.prepend(types.erasure(site))
			: Type.emptyList;
		    t = new ClassType(t.outer(), typeargs, t.tsym);
		    return new VarSymbol(
			STATIC | PUBLIC | FINAL, names._class, t, site.tsym);
		} else {
		    // We are seeing a plain identifier as selector.
		    Symbol sym = rs.findIdentInType(env, site, name, pkind);
		    if ((pkind & ERRONEOUS) == 0)
			sym = rs.access(sym, pos, site, name, true);
		    return sym;
		}
	    case TYPEARG:
		// Handle foo[0].x (site is the type of foo[0])
		return selectSym(tree, types.upperBound(site), env, pt, pkind);
	    case TYPEVAR:
		Symbol sym = TreeInfo.symbol(tree);
		if (isType(sym)) {
		    log.error(pos, "type.var.cant.be.deref");
		    return syms.errSymbol;
		} else {
		    return selectSym(tree, site.bound(), env, pt, pkind);
		}
	    case ERROR:
		// preserve identifier names through errors
		return new ErrorType(name, site.tsym).tsym;
	    default:
		// The qualifier expression is of a primitive type -- only
		// .class is allowed for these.
		if (name == names._class) {
		    // In this case, we have already made sure in Select that
		    // qualifier expression is a type.
		    Type t = syms.classType;
                    Type arg = types.boxedClass(site).type;
		    t = new ClassType(t.outer(), Type.emptyList.prepend(arg), t.tsym);
		    return new VarSymbol(
			STATIC | PUBLIC | FINAL, names._class, t, site.tsym);
		} else {
		    log.error(pos, "cant.deref", site);
		    return syms.errSymbol;
		}
	    }
	}

        /** Determine type of identifier or select expression and check that
	 *  (1) the referenced symbol is not deprecated
	 *  (2) the symbol's type is safe (@see checkSafe)
	 *  (3) if symbol is a variable, check that its type and kind are
	 *      compatible with the prototype and protokind.
	 *  (4) if symbol is an instance field of a raw type,
	 *      which is being assigned to, issue an unchecked warning if its
	 *      type changes under erasure.
	 *  (5) if symbol is an instance method of a raw type, issue an
	 *      unchecked warning if its argument types change under erasure.
	 *  If checks succeed:
	 *    If symbol is a constant, return its constant type
	 *    else if symbol is a method, return its result type
	 *    otherwise return its type.
	 *  Otherwise return errType.
	 *
	 *  @param tree       The syntax tree representing the identifier
	 *  @param site       If this is a select, the type of the selected
	 *                    expression, otherwise the type of the current class.
	 *  @param sym        The symbol representing the identifier.
	 *  @param env        The current environment.
	 *  @param pkind      The set of expected kinds.
	 *  @param pt         The expected type.
	 */
        Type checkId(Tree tree,
                     Type site,
                     Symbol sym,
		     Env<AttrContext> env,
                     int pkind,
		     Type pt,
                     boolean useVarargs) {
	    if (pt.isErroneous()) return syms.errType;
	    Type owntype; // The computed type of this identifier occurrence.
	    switch (sym.kind) {
	    case TYP:
		// For types, the computed type equals the symbol's type,
		// except for two situations:
		owntype = sym.type;
		if (owntype.tag == CLASS) {
                    Type ownOuter = owntype.outer();

		    // (a) If the symbol's type is parameterized, erase it
                    // because no type parameters were given.
                    // We recover generic outer type later in visitTypeApply.
		    if (owntype.tsym.type.typarams().nonEmpty()) {
			owntype = types.erasure(owntype);
		    }

		    // (b) If the symbol's type is an inner class, then
		    // we have to interpret its outer type as a superclass
		    // of the site type. Example:
		    //
		    // class Tree<A> { class Visitor { ... } }
		    // class PointTree extends Tree<Point> { ... }
		    // ...PointTree.Visitor...
		    //
		    // Then the type of the last expression above is
		    // Tree<Point>.Visitor.
		    else if (ownOuter.tag == CLASS && site != ownOuter) {
			Type normOuter = site;
			if (normOuter.tag == CLASS)
                            normOuter = types.asEnclosingSuper(site, ownOuter.tsym);
                        if (normOuter == null) // perhaps from an import
                            normOuter = types.erasure(ownOuter);
			if (normOuter != ownOuter)
			    owntype = new ClassType(
				normOuter, Type.emptyList, owntype.tsym);
		    }
                }
		break;
	    case VAR:
		VarSymbol v = (VarSymbol)sym;
		// Test (4): if symbol is an instance field of a raw type,
		// which is being assigned to, issue an unchecked warning if
		// its type changes under erasure.
		if (allowGenerics &&
		    pkind == VAR &&
		    v.owner.kind == TYP &&
		    (v.flags() & STATIC) == 0 &&
		    site.tag == CLASS) {
		    Type s = types.asOuterSuper(site, v.owner);
		    if (s != null &&
			s.isRaw() &&
			!types.isSameType(v.type, v.erasure(types))) {
			chk.warnUnchecked(tree.pos,
					  "unchecked.assign.to.var",
					  v, site);
		    }
		}
		// The computed type of a variable is the type of the
		// variable symbol, taken as a member of the site type.
		owntype = (sym.owner.kind == TYP &&
			   sym.name != names._this && sym.name != names._super)
		    ? types.memberType(site, sym)
		    : sym.type;

		if (env.info.tvars.nonEmpty()) {
		    Type owntype1 = new ForAll(env.info.tvars, owntype);
		    for (List<Type> l = env.info.tvars; l.nonEmpty(); l = l.tail)
			if (!owntype.contains(l.head)) {
			    log.error(tree.pos, "undetermined.type", owntype1);
			    owntype1 = syms.errType;
			}
		    owntype = owntype1;
		}

		// If the variable is a constant, record constant value in
		// computed type.
		if (v.constValue != null && isStaticReference(tree))
		    owntype = owntype.constType(v.constValue);

		if (pkind == VAL) {
		    owntype = capture(owntype); // capture "names as expressions"
		}
		break;
	    case MTH: {
                Apply app = (Apply)env.tree;
                owntype = checkMethod(site, sym, env, app.args,
                                      pt.argtypes(), pt.typarams(),
                                      env.info.varArgs);
		break;
            }
	    case PCK: case ERR:
		owntype = sym.type;
		break;
	    default:
		throw new AssertionError("unexpected kind: " + sym.kind +
					 " in tree " + tree);
	    }

	    // Test (1): emit a `deprecation' warning if symbol is deprecated.
	    // (for constructors, the error was given when the constructor was
	    // resolved)
	    if (sym.name != names.init &&
		(sym.flags() & DEPRECATED) != 0 &&
		(env.info.scope.owner.flags() & DEPRECATED) == 0 &&
		sym.outermostClass() != env.info.scope.owner.outermostClass())
		chk.warnDeprecated(tree.pos, sym);

	    // Test (3): if symbol is a variable, check that its type and
	    // kind are compatible with the prototype and protokind.
	    return check(tree, owntype, sym.kind, pkind, pt);
        }

        /** Check that variable is initialized and evaluate the variable's
	 *  initializer, if not yet done. Also check that variable is not
	 *  referenced before it is defined.
	 *  @param tree    The tree making up the variable reference.
	 *  @param env     The current environment.
	 *  @param v       The variable's symbol.
	 */
	private void checkInit(Ident tree, Env<AttrContext> env, VarSymbol v) {
//	    System.err.println(v + " " + ((v.flags() & STATIC) != 0) + " " +
//			       tree.pos + " " + v.pos + " " +
//			       Resolve.isStatic(env));//DEBUG

	    // A forward reference is diagnosed if the declaration position
	    // of the variable is greater than the current tree position
	    // and the tree and variable definition occur in the same class
	    // definition.  Note that writes don't count as references.
	    // This check applies only to class and instance
	    // variables.  Local variables follow different scope rules,
	    // and are subject to definite assignment checking.
	    if (v.pos > tree.pos &&
		v.owner.kind == TYP &&
		canOwnInitializer(env.info.scope.owner) &&
		v.owner == env.info.scope.owner.enclClass() &&
		((v.flags() & STATIC) != 0) == Resolve.isStatic(env) &&
		(env.tree.tag != Tree.ASSIGN ||
		 TreeInfo.skipParens(((Assign)env.tree).lhs) != tree))
	      log.error(tree.pos, "illegal.forward.ref");

	    evalInit(v);

	    // In an enum type, constructors and instance initializers
	    // may not reference its static members unless they are constant.
	    if (((v.flags() & STATIC) != 0) &&
		((v.owner.flags() & ENUM) != 0) &&
                v.constValue == null &&
		v.owner == env.info.scope.owner.enclClass() &&
		Resolve.isInitializer(env))
		log.error(tree.pos, "illegal.enum.static.ref");
	}

        /** Can the given symbol be the owner of code which forms part
	 *  if class initialization? This is the case if the symbol is
	 *  a type or field, or if the symbol is the synthetic method.
	 *  owning a block.
	 */
        private boolean canOwnInitializer(Symbol sym) {
	    return
		(sym.kind & (VAR | TYP)) != 0 ||
		(sym.kind == MTH && (sym.flags() & BLOCK) != 0);
	}

    Warner noteWarner = new Warner();

    /**
     * Check that method arguments conform to its instantation.
     **/
    public Type checkMethod(Type site,
                            Symbol sym,
                            Env<AttrContext> env,
                            final List<Tree> argtrees,
                            List<Type> argtypes,
                            List<Type> typeargtypes,
                            boolean useVarargs) {
        // Test (5): if symbol is an instance method of a raw type, issue
        // an unchecked warning if its argument types change under erasure.
        if (allowGenerics &&
            (sym.flags() & STATIC) == 0 &&
            site.tag == CLASS) {
            Type s = types.asOuterSuper(site, sym.owner);
            if (s != null && s.isRaw() &&
                !types.isSameTypes(sym.type.argtypes(),
                                   sym.erasure(types).argtypes())) {
                chk.warnUnchecked(env.tree.pos,
                                  "unchecked.call.mbr.of.raw.type",
                                  sym, site);
            }
        }

        // Compute the identifier's instantiated type.
        // For methods, we need to compute the instance type by
        // Resolve.instantiate from the symbol's type as well as
        // any type arguments and value arguments.
        noteWarner.warned = false;
        Type owntype = rs.instantiate(env,
                                      site,
                                      sym,
                                      argtypes,
                                      typeargtypes,
                                      true,
                                      useVarargs,
                                      noteWarner);
        boolean warned = noteWarner.warned;

        // If this fails, something went wrong; we should not have
        // found the identifier in the first place.
        if (owntype == null) {
            if (!pt.isErroneous())
                log.error(env.tree.pos,
                          "internal.error.cant.instantiate",
                          sym, site,
                          Type.toString(pt.argtypes()));
            owntype = syms.errType;
        } else {
            // System.out.println("call   : " + env.tree);
            // System.out.println("method : " + owntype);
            // System.out.println("actuals: " + argtypes);
            List<Type> formals = owntype.argtypes();
            Type last = useVarargs ? formals.last() : null;
            if (sym.name==names.init &&
                sym.owner == syms.enumSym)
                formals = formals.tail.tail;
            List<Tree> args = argtrees;
            while (formals.head != last) {
                Tree arg = args.head;
                Warner warn = chk.convertWarner(arg.pos, arg.type, formals.head);
                if (!types.isConvertible(arg.type, formals.head, warn))
                    ;//throw new AssertionError(env.tree); // temporarily disabled, pending snapshots
                warned |= warn.warned;
                args = args.tail;
                formals = formals.tail;
            }
            if (useVarargs) {
                Type varArg = types.elemtype(last);
                while (args.tail != null) {
                    Tree arg = args.head;
                    Warner warn = chk.convertWarner(arg.pos, arg.type, varArg);
                    if (!types.isConvertible(arg.type, varArg, warn))
                        ;//throw new AssertionError(env.tree); // temporarily disabled, pending snapshots
                    warned |= warn.warned;
                    args = args.tail;
                }
            } else if ((sym.flags() & VARARGS) != 0 && allowVarargs) {
                // non-varargs call to varargs method
                Type varParam = owntype.argtypes().last();
                Type lastArg = argtypes.last();
                if (types.isSubTypeUnchecked(lastArg, types.elemtype(varParam)) &&
                    !types.isSameType(types.erasure(varParam), types.erasure(lastArg)))
                    log.warning(argtrees.last().pos, "inexact.non-varargs.call",
                                types.elemtype(varParam),
                                varParam);
            }

            if (warned && sym.type.tag == FORALL) {
                String typeargs = "";
                if (typeargtypes != null && typeargtypes.nonEmpty()) {
                    typeargs = "<" + Type.toString(typeargtypes) + ">";
                }
                chk.warnUnchecked(env.tree.pos,
                                  "unchecked.meth.invocation.applied",
                                  sym,
                                  sym.location(),
                                  typeargs,
                                  Type.toString(argtypes));
                owntype = new MethodType(owntype.argtypes(),
                                         types.erasure(owntype.restype()),
                                         owntype.thrown(),
                                         syms.methodClass);
            }
            if (useVarargs) {
                Tree tree = env.tree;
                Type argtype = owntype.argtypes().last();
                if (!types.isReifiable(argtype))
                    chk.warnUnchecked(env.tree.pos,
                                      "unchecked.generic.array.creation",
                                      argtype);
                Type elemtype = types.elemtype(argtype);
                switch (tree.tag) {
                case Tree.APPLY:
                    ((Apply)tree).varargsElement = elemtype;
                    break;
                case Tree.NEWCLASS:
                    ((NewClass)tree).varargsElement = elemtype;
                    break;
                default:
                    throw new AssertionError(""+tree);
                }
            }
        }
        return owntype;
    }

    /** Evaluate a final variable's initializer, unless this has already been
     *  done, and set variable's constant value, if the initializer is
     *  constant.
     */
    public void evalInit(VarSymbol v) {
	if (v.constValue instanceof AttrContextEnv) {
	    // In this case, `v' is final, with an as yet unevaluated
	    // initializer. The constValue points to the environment
	    // current at the point of the variable definition
	    // (its initEnv).
	    AttrContextEnv evalEnv = (AttrContextEnv) v.constValue;
	    Name prev = log.useSource(evalEnv.toplevel.sourcefile);
	    v.constValue = null; // to make sure we don't evaluate this twice.
	    Type itype = attribExpr(((VarDef)evalEnv.tree).init, evalEnv, v.type);
	    if (itype.constValue != null)
		v.constValue = cfolder.coerce(itype, v.type).constValue;
	    log.useSource(prev);
	}
    }

    public void visitLiteral(Literal tree) {
	result = check(
	    tree, litType(tree.typetag).constType(tree.value), VAL, pkind, pt);
    }
    //where
    /** Return the type of a literal with given type tag.
     */
    Type litType(int tag) {
	return (tag == TypeTags.CLASS) ? syms.stringType : syms.typeOfTag[tag];
    }

    public void visitTypeIdent(TypeIdent tree) {
	result = check(tree, syms.typeOfTag[tree.typetag], TYP, pkind, pt);
    }

    public void visitTypeArray(TypeArray tree) {
	Type etype = attribType(tree.elemtype, env);
	Type type = new ArrayType(etype, syms.arrayClass);
	result = check(tree, type, TYP, pkind, pt);
    }

    /** Visitor method for parameterized types.
     *  Bound checking is left until later, since types are attributed
     *  before supertype structure is completely known
     */
    public void visitTypeApply(TypeApply tree) {
	Type owntype = syms.errType;

	// Attribute functor part of application and make sure it's a class.
	Type clazztype = chk.checkClassType(tree.clazz.pos, attribType(tree.clazz, env));

	// Attribute type parameters
	List<Type> actuals = attribTypes(tree.arguments, env);

        if (clazztype.tag == CLASS) {
	    List<Type> formals = clazztype.tsym.type.typarams();

	    if (actuals.length() == formals.length()) {
		List<Type> a = actuals;
		List<Type> f = formals;
		while (a.nonEmpty()) {
		    a.head = a.head.withTypeVar(f.head);
		    a = a.tail;
		    f = f.tail;
		}
                // Compute the proper generic outer
                Type clazzOuter = clazztype.outer();
                if (clazzOuter.tag == CLASS) {
                    Type site;
                    if (tree.clazz.tag == Tree.IDENT) {
                        site = env.enclClass.sym.type;
                    } else if (tree.clazz.tag == Tree.SELECT) {
                        site = ((Select)tree.clazz).selected.type;
                    } else throw new AssertionError(""+tree);
                    if (clazzOuter.tag == CLASS && site != clazzOuter) {
                        if (site.tag == CLASS)
                            site = types.asOuterSuper(site, clazzOuter.tsym);
                        if (site == null)
                            site = types.erasure(clazzOuter);
                        clazzOuter = site;
                    }
                }
		owntype = new ClassType(clazzOuter, actuals, clazztype.tsym);
	    } else {
		if (formals.length() != 0) {
		    log.error(tree.pos, "wrong.number.type.args",
			      Integer.toString(formals.length()));
		} else {
		    log.error(tree.pos, "type.doesnt.take.params", clazztype.tsym);
		}
		owntype = syms.errType;
	    }
	}
	result = check(tree, owntype, TYP, pkind, pt);
    }

    public void visitTypeParameter(TypeParameter tree) {
	TypeVar a = (TypeVar)tree.type;
	Set<Type> boundSet = new HashSet<Type>();
	ListBuffer<Type> boundBuf = new ListBuffer<Type>();
	if (tree.bounds.nonEmpty()) {
	    // accept class or interface or typevar as first bound.
	    Type b = attribBase(tree.bounds.head, env, false, false, false);
	    boundBuf.append(b);
	    boundSet.add(b);
	    if (b.tag == TYPEVAR) {
		// if first bound was a typevar, do not accept further bounds.
		if (tree.bounds.tail.nonEmpty()) {
		    log.error(tree.bounds.tail.head.pos,
			      "type.var.may.not.be.followed.by.other.bounds");
		    tree.bounds.tail = List.make();
		}
	    } else {
		// if first bound was a class or interface, accept only interfaces
		// as further bounds.
		for (List<Tree> l = tree.bounds.tail; l.nonEmpty(); l = l.tail) {
		    Type i = attribBase(l.head, env, false, true, false);
		    if (i.tag == CLASS) {
			boundBuf.append(i);
			chk.checkNotRepeated(l.head.pos, i, boundSet);
		    }
		}
	    }
	}
	List<Type> bs = boundBuf.toList();

	// if no bounds are given, assume a single bound of java.lang.Object.
	types.setBounds(a, bs.nonEmpty() ? bs : List.make(syms.objectType));

	// in case of multiple bounds ...
	if (bs.length() > 1) {
	    // ... the variable's bound is a class type flagged COMPOUND
	    // (see comment for TypeVar.bound).
	    // In this case, generate a class tree that represents the
	    // bound class, ...
	    Tree extending;
	    List<Tree> implementing;
	    if ((bs.head.tsym.flags() & INTERFACE) == 0) {
		extending = tree.bounds.head;
		implementing = tree.bounds.tail;
	    } else {
		extending = null;
		implementing = tree.bounds;
	    }
	    ClassDef cd = make.at(tree.pos).ClassDef(
		make.Modifiers(PUBLIC | ABSTRACT),
		tree.name, TypeParameter.emptyList,
		extending, implementing, Tree.emptyList);

	    ClassSymbol c = (ClassSymbol)a.bound().tsym;
	    assert (c.flags() & COMPOUND) != 0;
	    cd.sym = c;
	    c.sourcefile = env.toplevel.sourcefile;

	    // ... and attribute the bound class
	    c.flags_field |= UNATTRIBUTED;
	    Env<AttrContext> cenv = enter.classEnv(cd, env);
	    enter.typeEnvs.put(c, cenv);
	    attribClass(tree.pos, c);
	}
    }


    public void visitTypeArgument(TypeArgument tree) {
	//- System.err.println("visitTypeArgument("+tree+");");//DEBUG
	Type type = (tree.kind == BoundKind.UNBOUND) ? syms.objectType : attribType(tree.inner, env);
	result = check(tree, new ArgumentType(chk.checkRefType(tree.pos, type),
					      tree.kind,
					      syms.boundClass),
		       TYP, pkind, pt);
    }

    public void visitAnnotation(Annotation tree) {
        log.error(tree.pos, "annotation.not.valid.for.type", pt);
	result = tree.type = syms.errType;
    }

    public void visitErroneous(Erroneous tree) {
	result = tree.type = syms.errType;
    }

    /** Default visitor method for all other trees.
     */
    public void visitTree(Tree tree) {
	throw new AssertionError();
    }

    /** Main method: attribute class definition associated with given class symbol.
     *  reporting completion failures at the given position.
     *  @param pos The source position at which completion errors are to be
     *             reported.
     *  @param c   The class symbol whose definition will be attributed.
     */
    public void attribClass(int pos, ClassSymbol c) {
	try {
            annotate.flush();
	    attribClass(c);
	} catch (CompletionFailure ex) {
	    chk.completionError(pos, ex);
	}
    }

    /** Attribute class definition associated with given class symbol.
     *  @param c   The class symbol whose definition will be attributed.
     */
    void attribClass(ClassSymbol c) throws CompletionFailure {
	if (c.type.tag == ERROR) return;

	// Check for cycles in the inheritance graph, which can arise from
	// ill-formed class files.
	chk.checkNonCyclic(Position.NOPOS, c.type);

	Type st = types.supertype(c.type);
	if ((c.flags_field & Flags.COMPOUND) == 0) {
	    // First, attribute superclass.
	    if (st.tag == CLASS)
		attribClass((ClassSymbol)st.tsym);

	    // Next attribute owner, if it is a class.
	    if (c.owner.kind == TYP && c.owner.type.tag == CLASS)
		attribClass((ClassSymbol)c.owner);
	}

	// The previous operations might have attributed the current class
	// if there was a cycle. So we test first whether the class is still
	// UNATTRIBUTED.
	if ((c.flags_field & UNATTRIBUTED) != 0) {
	    c.flags_field &= ~UNATTRIBUTED;

	    // Get environment current at the point of class definition.
	    Env<AttrContext> env = enter.typeEnvs.get(c);

	    Name prev = log.useSource(c.sourcefile);

	    try {
		// java.lang.Enum may not be subclassed by a non-enum
		if (st.tsym == syms.enumSym &&
		    ((c.flags_field & Flags.ENUM) == 0) ) 
		    log.error(env.tree.pos, "enum.no.subclassing");

		// Enums may not be extended by source-level classes
		if (st.tsym != null &&
		    ((st.tsym.flags_field & Flags.ENUM) != 0) &&
		    ((c.flags_field & Flags.ENUM) == 0) &&
		    !target.compilerBootstrap(c)) {
		    log.error(env.tree.pos, "enum.types.not.extensible");
		}
		attribClassBody(env, c);
	    } finally {
		log.useSource(prev);
	    }

	    // check for missing @Deprecated annotations
	    chk.checkDeprecatedAnnotation(env.tree.pos, c);
	}
    }

    public void visitImport(Import tree) {
	// nothing to do
    }

    /** Finish the attribution of a class. */
    private void attribClassBody(Env<AttrContext> env, ClassSymbol c) {
	ClassDef tree = (ClassDef)env.tree;
	assert c == tree.sym;

	// Validate annotations
	chk.validateAnnotations(tree.mods.annotations, c);

	// Validate type parameters, supertype and interfaces.
	chk.validateTypeParams(tree.typarams);
	chk.validate(tree.extending);
	chk.validate(tree.implementing);

	// If this is a non-abstract class, check that it has no abstract
	// methods or unimplemented methods of an implemented interface.
	if ((c.flags() & (ABSTRACT | INTERFACE)) == 0)
	    { if (!relax) chk.checkAllDefined(tree.pos, c); }

	if ((c.flags() & ANNOTATION) != 0) {
	    if (tree.implementing.nonEmpty())
		log.error(tree.implementing.head.pos, 
                          "cant.extend.intf.annotation");
	    if (tree.typarams.nonEmpty())
		log.error(tree.typarams.head.pos, 
                          "intf.annotation.cant.have.type.params");
	}

	// If abstract, check that all extended classes and interfaces
	// are compatible (i.e. no two define methods with same arguments
	// yet different return types).  (JLS 8.4.6.3)
	else
	    chk.checkCompatibleSupertypes(tree.pos, c.type);

	// Check that class does not import the same parameterized interface
	// with two different argument lists.
	chk.checkClassBounds(tree.pos, c.type);

	tree.type = c.type;

	boolean assertsEnabled = false;
	assert assertsEnabled = true;
	if (assertsEnabled) {
	    for (List<TypeParameter> l = tree.typarams;
		 l.nonEmpty(); l = l.tail)
		assert env.info.scope.lookup(l.head.name).scope != null;
	}

	// Check that all methods which implement some
	// method conform to the method they implement.
	chk.checkImplementations(tree);

	for (List<Tree> l = tree.defs; l.nonEmpty(); l = l.tail) {
	    // Attribute declaration
	    attribStat(l.head, env);
	    // Check that declarations in inner classes are not static (JLS 8.1.2)
	    // Make an exception for static constants.
	    if (c.owner.kind != PCK &&
		((c.flags() & STATIC) == 0 || c.name == names.empty) &&
		(TreeInfo.flags(l.head) & (STATIC | INTERFACE)) != 0) {
		Symbol sym = null;
		if (l.head.tag == Tree.VARDEF) sym = ((VarDef) l.head).sym;
		if (sym == null ||
		    sym.kind != VAR ||
		    ((VarSymbol) sym).constValue == null)
		    log.error(l.head.pos, "icls.cant.have.static.decl");
	    }
	}

	// Check for cycles among non-initial constructors.
	chk.checkCyclicConstructors(tree);

        // Check for cycles among annotation elements.
        chk.checkNonCyclicElements(tree);

	// Check for proper use of serialVersionUID
	if (lintSerial &&
	    types.isSubType(c.type, syms.serializableType) &&
            (c.flags() & Flags.ENUM) == 0 &&
	    (c.flags() & ABSTRACT) == 0) {
	    checkSerialVersionUID(tree, c);
	}
    }
	// where
	/** Check that an appropriate serialVersionUID member is defined. */
	private void checkSerialVersionUID(ClassDef tree, ClassSymbol c) {

	    // check for presence of serialVersionUID
	    Scope.Entry e = c.members().lookup(names.serialVersionUID);
	    while (e.scope != null && e.sym.kind != VAR) e = e.next();
	    if (e.scope == null) {
		log.warning(tree.pos, "missing.SVUID", c);
		return;
	    }

	    // check that it is static final
	    VarSymbol svuid = (VarSymbol)e.sym;
	    if ((svuid.flags() & (STATIC | FINAL)) !=
		(STATIC | FINAL))
		log.warning(TreeInfo.positionFor(svuid, tree), "improper.SVUID", c);

	    // check that it is long
	    else if (svuid.type.tag != TypeTags.LONG)
		log.warning(TreeInfo.positionFor(svuid, tree), "long.SVUID", c);

	    // check constant
	    else if (svuid.constValue == null)
		log.warning(TreeInfo.positionFor(svuid, tree), "constant.SVUID", c);
	}

    private Type capture(Type type) {
	return types.capture(type);
    }
}
