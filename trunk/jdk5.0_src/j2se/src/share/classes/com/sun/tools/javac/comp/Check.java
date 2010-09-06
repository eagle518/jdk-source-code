/**
 * @(#)Check.java	1.137 04/07/16
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

import com.sun.tools.javac.tree.Tree.*;
import com.sun.tools.javac.code.Type.*;
import com.sun.tools.javac.code.Symbol.*;

import static com.sun.tools.javac.code.Flags.*;
import static com.sun.tools.javac.code.Kinds.*;
import static com.sun.tools.javac.code.TypeTags.*;

/** Type checking helper class for the attribution phase.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Check {
    protected static final Context.Key<Check> checkKey =
	new Context.Key<Check>();

    private final Name.Table names;
    private final Log log;
    private final Symtab syms;
    private final Infer infer;
    private final Target target;
    private final Source source;
    private final Name classDollar;
    private final Name thisDollar;
    private final Types types;
    boolean skipAnnotations;

    public static Check instance(Context context) {
	Check instance = context.get(checkKey);
	if (instance == null)
	    instance = new Check(context);
	return instance;
    }

    protected Check(Context context) {
	context.put(checkKey, this);

	names = Name.Table.instance(context);
	log = Log.instance(context);
	syms = Symtab.instance(context);
	infer = Infer.instance(context);
	this.types = Types.instance(context);
	Options options = Options.instance(context);
	target = Target.instance(context);
        source = Source.instance(context);

	classDollar = names.
	    fromString("class" + target.syntheticNameChar());
	thisDollar = names.
	    fromString("this" + target.syntheticNameChar());

	Source source = Source.instance(context);
	allowGenerics = source.allowGenerics();
	allowAnnotations = source.allowAnnotations();
	warnunchecked = options.lint("unchecked");
	deprecation = options.lint("deprecation");
	checkDeprecatedAnnotation = options.lint("dep-ann");
	complexInference = options.get("-complexinference") != null;
	skipAnnotations = options.get("skipAnnotations") != null;
    }

    /** Switch: generics enabled?
     */
    boolean allowGenerics;

    /** Switch: variant arrays enabled?
     */
    boolean allowVariance;

    /** Switch: annotations enabled?
     */
    boolean allowAnnotations;

    /** Switch: -Xlint:unchecked option set?
     */
    boolean warnunchecked;

    /** Switch: -Xlint:deprecation option set?
     */
    boolean deprecation;

    /** Switch: -Xlint:dep-ann option set?
     */
    boolean checkDeprecatedAnnotation;

    /** Switch: -complexinference option set?
     */
    boolean complexInference;

    /** A table mapping flat names of all compiled classes in this run to their
     *  symbols; maintained from outside.
     */
    public Map<Name,ClassSymbol> compiled = new HashMap<Name, ClassSymbol>();

    /** Output variable: the source file where deprecated symbols were
     *  encountered. If deprecated symbols were encountered in more than one
     *  input file, the variable is set to "*".
     */
    public Name deprecatedSource;

    /** Output variable: the source file where deprecated symbols were
     *  encountered. If deprecated symbols were encountered in more than one
     *  input file, the variable is set to "*".
     */
    public Name uncheckedSource;

/* *************************************************************************
 * Errors and Warnings
 **************************************************************************/

    /** Warn about deprecated symbol.
     *  @param pos        Position to be used for error reporting.
     *  @param sym        The deprecated symbol.
     */
    void warnDeprecated(int pos, Symbol sym) {
	if (deprecatedSource == null)
	    deprecatedSource = log.currentSource();
	else if (deprecatedSource != log.currentSource())
	    deprecatedSource = names.asterisk;

	if (deprecation)
	    log.warning(pos, "has.been.deprecated",
			sym, sym.location());
    }

    /** Warn about unchecked operation.
     *  @param pos        Position to be used for error reporting.
     *  @param msg        A string describing the problem.
     */
    public void warnUnchecked(int pos, String msg, Object... args) {
	if (uncheckedSource == null)
	    uncheckedSource = log.currentSource();
	else if (uncheckedSource != log.currentSource())
	    uncheckedSource = names.asterisk;
	if (warnunchecked) log.warning(pos, msg, args);
    }


    /** Report a failure to complete a class.
     *  @param pos        Position to be used for error reporting.
     *  @param ex         The failure to report.
     */
    public Type completionError(int pos, CompletionFailure ex) {
	log.error(pos, "cant.access", ex.sym, ex.errmsg);
	if (ex instanceof ClassReader.BadClassFile) throw new Abort();
	else return syms.errType;
    }

    /** Report a type error.
     *  @param pos        Position to be used for error reporting.
     *  @param problem    A string describing the error.
     *  @param found      The type that was found.
     *  @param req        The type that was required.
     */
    Type typeError(int pos, Object problem, Type found, Type req) {
	log.error(pos, "prob.found.req",
		  problem, found, req);
	return syms.errType;
    }

    Type typeError(int pos, String problem, Type found, Type req, Object explanation) {
	log.error(pos, "prob.found.req.1", problem, found, req, explanation);
	return syms.errType;
    }

    /** Report an error that wrong type tag was found.
     *  @param pos        Position to be used for error reporting.
     *  @param required   An internationalized string describing the type tag
     *                    required.
     *  @param found      The type that was found.
     */
    Type typeTagError(int pos, Object required, Object found) {
	log.error(pos, "type.found.req", found, required);
	return syms.errType;
    }

    /** Report an error that symbol cannot be referenced before super
     *  has been called.
     *  @param pos        Position to be used for error reporting.
     *  @param sym        The referenced symbol.
     */
    void earlyRefError(int pos, Symbol sym) {
	log.error(pos, "cant.ref.before.ctor.called", sym);
    }

    /** Report duplicate declaration error.
     */
    void duplicateError(int pos, Symbol sym) {
	if (!sym.type.isErroneous()) {
	    log.error(pos, "already.defined", sym, sym.location());
	}
    }

/* ************************************************************************
 * duplicate declaration checking
 *************************************************************************/

    /** Check that variable does not hide variable with same name in
     *	immediately enclosing local scope.
     *	@param pos	     Position for error reporting.
     *	@param sym	     The symbol.
     *	@param s	     The scope.
     */
    void checkTransparentVar(int pos, VarSymbol v, Scope s) {
	if (s.next != null) {
	    for (Scope.Entry e = s.next.lookup(v.name);
		 e.scope != null && e.sym.owner == v.owner;
		 e = e.next()) {
		if (e.sym.kind == VAR &&
		    (e.sym.owner.kind & (VAR | MTH)) != 0 &&
		    v.name != names.error) {
		    duplicateError(pos, e.sym);
		    return;
		}
	    }
	}
    }

    /** Check that a class or interface does not hide a class or
     *	interface with same name in immediately enclosing local scope.
     *	@param pos	     Position for error reporting.
     *	@param sym	     The symbol.
     *	@param s	     The scope.
     */
    void checkTransparentClass(int pos, ClassSymbol c, Scope s) {
	if (s.next != null) {
	    for (Scope.Entry e = s.next.lookup(c.name);
		 e.scope != null && e.sym.owner == c.owner;
		 e = e.next()) {
		if (e.sym.kind == TYP &&
		    (e.sym.owner.kind & (VAR | MTH)) != 0 &&
		    c.name != names.error) {
		    duplicateError(pos, e.sym);
		    return;
		}
	    }
	}
    }

    /** Check that class does not have the same name as one of
     *	its enclosing classes, or as a class defined in its enclosing scope.
     *	return true if class is unique in its enclosing scope.
     *	@param pos	     Position for error reporting.
     *	@param name	     The class name.
     *	@param s	     The enclosing scope.
     */
    boolean checkUniqueClassName(int pos, Name name, Scope s) {
	for (Scope.Entry e = s.lookup(name); e.scope == s; e = e.next()) {
	    if (e.sym.kind == TYP && e.sym.name != names.error) {
		duplicateError(pos, e.sym);
		return false;
	    }
	}
	for (Symbol sym = s.owner; sym != null; sym = sym.owner) {
	    if (sym.kind == TYP && sym.name == name && sym.name != names.error) {
		duplicateError(pos, sym);
		return true;
	    }
	}
	return true;
    }

/* *************************************************************************
 * Class name generation
 **************************************************************************/

    /** Return name of local class.
     *  This is of the form    <enclClass> $ n <classname>
     *  where
     *    enclClass is the flat name of the enclosing class,
     *    classname is the simple name of the local class
     */
    Name localClassName(ClassSymbol c) {
	ClassSymbol topClass = c.outermostClass();
	for (int i=1; ; i++) {
	    Name flatname = names.
		fromString("" + c.owner.enclClass().flatname +
                           target.syntheticNameChar() + i +
                           c.name);
	    if (compiled.get(flatname) == null) return flatname;
	}
    }

/* *************************************************************************
 * Type Checking
 **************************************************************************/

    /** Check that a given type is assignable to a given proto-type.
     *  If it is, return the type, otherwise return errType.
     *  @param pos        Position to be used for error reporting.
     *  @param found      The type that was found.
     *  @param req        The type that was required.
     */
    Type checkType(int pos, Type found, Type req) {
	if (req.tag == ERROR)
	    return req;
	if (found.tag == FORALL)
	    return instantiatePoly(pos, (ForAll)found, req, convertWarner(pos, found, req));
	if (req.tag == NONE)
	    return found;
	if (types.isAssignable(found, req, convertWarner(pos, found, req)))
	    return found;
	if (found.tag <= DOUBLE && req.tag <= DOUBLE)
	    return typeError(pos, new Diagnostic("possible.loss.of.precision"), found, req);
	if (found.isSuperBound()) {
	    log.error(pos, "assignment.from.super-bound", found);
	    return syms.errType;
	}
	if (req.isExtendsBound()) {
	    log.error(pos, "assignment.to.extends-bound", req);
	    return syms.errType;
	}
	return typeError(pos, new Diagnostic("incompatible.types"), found, req);
    }

    /** Instantiate polymorphic type to some prototype, unless
     *  prototype is `anyPoly' in which case polymorphic type
     *  is returned unchanged.
     */
    Type instantiatePoly(int pos, ForAll t, Type pt, Warner warn) {
	if (pt == Infer.anyPoly && complexInference) {
	    return t;
	} else if (pt == Infer.anyPoly || pt.tag == NONE) {
	    Type newpt = t.qtype.tag <= VOID ? t.qtype : syms.objectType;
	    return instantiatePoly(pos, t, newpt, warn);
	} else if (pt.tag == ERROR) {
	    return pt;
	} else {
	    try {
		return infer.instantiateExpr(t, pt, warn);
	    } catch (Infer.NoInstanceException ex) {
		if (ex.isAmbiguous) {
		    Diagnostic d = ex.getDiagnostic();
		    log.error(pos,
			      "undetermined.type" + (d!=null ? ".1" : ""),
			      t, d);
		    return syms.errType;
		} else {
		    Diagnostic d = ex.getDiagnostic();
		    return typeError(pos,
                                     new Diagnostic("incompatible.types" + (d!=null ? ".1" : ""), d),
                                     t, pt);
		}
	    }
	}
    }

    /** Check that a given type can be cast to a given target type.
     *  Return the result of the cast.
     *  @param pos        Position to be used for error reporting.
     *  @param found      The type that is being cast.
     *  @param req        The target type of the cast.
     */
    Type checkCastable(int pos, Type found, Type req) {
	if (found.tag == FORALL) {
	    instantiatePoly(pos, (ForAll) found, req, castWarner(pos, found, req));
	    return req;
	} else if (types.isCastable(found, req, castWarner(pos, found, req))) {
	    return req;
	} else {
	    return typeError(pos,
			     new Diagnostic("inconvertible.types"),
			     found, req);
	}
    }
//where
        /** Is type a type variable, or a (possibly multi-dimensional) array of
	 *  type variables?
	 */
	boolean isTypeVar(Type t) {
	    return t.tag == TYPEVAR || t.tag == ARRAY && isTypeVar(types.elemtype(t));
	}

    /** Check that a type is within some bounds.
     *
     *  Used in TypeApply to verify that, e.g., X in V<X> is a valid
     *  type argument.
     *  @param pos           Position to be used for error reporting.
     *  @param a             The type that should be bounded by bs.
     *  @param bs            The bound.
     */
    private void checkExtends(int pos, Type a, TypeVar bs) {
	if (a.isUnbound()) {
	    return;
	} else if (a.tag != TYPEARG) {
	    a = types.upperBound(a);
	    for (List<Type> l = types.getBounds(bs); l.nonEmpty(); l = l.tail) {
		if (!types.isSubType(a, l.head)) {
		    log.error(pos, "not.within.bounds", a);
		    return;
		}
	    }
	} else if (a.isExtendsBound()) {
	    if (!types.isCastable(bs.bound(), types.upperBound(a), Warner.noWarnings))
		log.error(pos, "not.within.bounds", a);
	} else if (a.isSuperBound()) {
	    if (types.notSoftSubtype(types.lowerBound(a), bs.bound()))
		log.error(pos, "not.within.bounds", a);
	}
    }

    /** Check that type is different from 'void'.
     *  @param pos           Position to be used for error reporting.
     *  @param t             The type to be checked.
     */
    Type checkNonVoid(int pos, Type t) {
	if (t.tag == VOID) {
	    log.error(pos, "void.not.allowed.here");
	    return syms.errType;
	} else {
	    return t;
	}
    }

    /** Check that type is a class or interface type.
     *  @param pos           Position to be used for error reporting.
     *  @param t             The type to be checked.
     */
    Type checkClassType(int pos, Type t) {
	if (t.tag != CLASS && t.tag != ERROR)
            return typeTagError(pos,
                                new Diagnostic("type.req.class"),
                                (t.tag == TYPEVAR)
                                ? new Diagnostic("type.parameter", t)
                                : t); 
	else
	    return t;
    }

    /** Check that type is a class or interface type.
     *  @param pos           Position to be used for error reporting.
     *  @param t             The type to be checked.
     *  @param noBounds    True if type bounds are illegal here.
     */
    Type checkClassType(int pos, Type t, boolean noBounds) {
	t = checkClassType(pos, t);
	if (noBounds && t.isParameterized()) {
	    List<Type> args = t.typarams();
	    while (args.nonEmpty()) {
		if (args.head.tag == TYPEARG)
		    return typeTagError(pos,
					log.getLocalizedString("type.req.exact"),
					args.head);
		args = args.tail;
	    }
	}
	return t;
    }

    /** Check that type is a reifiable class, interface or array type.
     *  @param pos           Position to be used for error reporting.
     *  @param t             The type to be checked.
     */
    Type checkReifiableReferenceType(int pos, Type t) {
	if (t.tag != CLASS && t.tag != ARRAY && t.tag != ERROR) {
	    return typeTagError(pos,
				new Diagnostic("type.req.class.array"),
				t);
	} else if (!types.isReifiable(t)) {
	    log.error(pos, "illegal.generic.type.for.instof");
	    return syms.errType;
	} else {
	    return t;
	}
    }

    /** Check that type is a reference type, i.e. a class, interface or array type
     *  or a type variable.
     *  @param pos           Position to be used for error reporting.
     *  @param t             The type to be checked.
     */
    Type checkRefType(int pos, Type t) {
	switch (t.tag) {
	case CLASS:
	case ARRAY:
	case TYPEVAR:
	case TYPEARG:
	case ERROR:
	    return t;
	default:
	    return typeTagError(pos,
				new Diagnostic("type.req.ref"),
				t);
	}
    }

    /** Check that type is a null or reference type.
     *  @param pos           Position to be used for error reporting.
     *  @param t             The type to be checked.
     */
    Type checkNullOrRefType(int pos, Type t) {
	switch (t.tag) {
	case CLASS:
	case ARRAY:
	case TYPEVAR:
	case TYPEARG:
	case BOT:
	case ERROR:
	    return t;
	default:
	    return typeTagError(pos,
				new Diagnostic("type.req.ref"),
				t);
	}
    }

    /** Check that flag set does not contain elements of two conflicting sets. s
     *  Return true if it doesn't.
     *  @param pos           Position to be used for error reporting.
     *  @param flags         The set of flags to be checked.
     *  @param set1          Conflicting flags set #1.
     *  @param set2          Conflicting flags set #2.
     */
    boolean checkDisjoint(int pos, long flags, long set1, long set2) {
        if ((flags & set1) != 0 && (flags & set2) != 0) {
            log.error(pos,
		      "illegal.combination.of.modifiers",
		      TreeInfo.flagNames(TreeInfo.firstFlag(flags & set1)),
		      TreeInfo.flagNames(TreeInfo.firstFlag(flags & set2)));
            return false;
        } else
            return true;
    }

    /** Check that given modifiers are legal for given symbol and
     *  return modifiers together with any implicit modififiers for that symbol.
     *  Warning: we can't use flags() here since this method
     *  is called during class enter, when flags() would cause a premature
     *  completion.
     *  @param pos           Position to be used for error reporting.
     *  @param flags         The set of modifiers given in a definition.
     *  @param sym           The defined symbol.
     */
    long checkFlags(int pos, long flags, Symbol sym, Tree tree) {
	long mask;
	long implicit = 0;
	switch (sym.kind) {
	case VAR:
	    if (sym.owner.kind != TYP)
		mask = LocalVarFlags;
	    else if ((sym.owner.flags_field & INTERFACE) != 0)
		mask = implicit = InterfaceVarFlags;
	    else
		mask = VarFlags;
	    break;
	case MTH:
	    if (sym.name == names.init) {
		if ((sym.owner.flags_field & ENUM) != 0) { 
		    // enum constructors cannot be declared public or
		    // protected and must be implicitly or explicitly
		    // private
		    implicit = PRIVATE;
		    mask = PRIVATE;
		} else
		    mask = ConstructorFlags;
	    }  else if ((sym.owner.flags_field & INTERFACE) != 0)
		mask = implicit = InterfaceMethodFlags;
	    else {
		mask = MethodFlags;
	    }
	    // Imply STRICTFP if owner has STRICTFP set.
	    if (((flags|implicit) & Flags.ABSTRACT) == 0)
	      implicit |= sym.owner.flags_field & STRICTFP;
	    break;
	case TYP:
	    if (sym.isLocal()) {
		mask = LocalClassFlags;
		// Anonymous classes in static methods are themselves static;
		// that's why we admit STATIC here.
		if (sym.name.len == 0) mask |= STATIC;
	    } else if (sym.owner.kind == TYP) {
		mask = MemberClassFlags;
		if (sym.owner.owner.kind == PCK ||
                    (sym.owner.flags_field & STATIC) != 0)
                    mask |= STATIC;
                else if ((flags & ENUM) != 0)
                    log.error(pos, "mod.not.allowed.here", TreeInfo.flagNames(ENUM));
		// Nested interfaces and enums are always STATIC (Spec ???)
		if ((flags & (INTERFACE | ENUM)) != 0 ) implicit = STATIC;
	    } else {
		mask = ClassFlags;
	    }
	    // Interfaces are always ABSTRACT
	    if ((flags & INTERFACE) != 0) implicit |= ABSTRACT;

	    if ((flags & ENUM) != 0) {
		// enums can't be declared abstract or final
		mask &= ~(ABSTRACT | FINAL);
		implicit |= implicitEnumFinalFlag(tree);
	    }
	    // Imply STRICTFP if owner has STRICTFP set.
	    implicit |= sym.owner.flags_field & STRICTFP;
	    break;
	default:
	    throw new AssertionError();
	}
	long illegal = flags & StandardFlags & ~mask;
        if (illegal != 0)
            log.error(pos,
		      "mod.not.allowed.here", TreeInfo.flagNames(illegal));
        else if ((sym.kind == TYP ||
		  // ISSUE: Disallowing abstract&private is no longer appropriate
		  // in the presence of inner classes. Should it be deleted here?
		  checkDisjoint(pos, flags,
				ABSTRACT,
				PRIVATE | STATIC))
		 &&
		 checkDisjoint(pos, flags,
			       ABSTRACT | INTERFACE,
			       FINAL | NATIVE | SYNCHRONIZED)
		 &&
                 checkDisjoint(pos, flags,
                               PUBLIC,
                               PRIVATE | PROTECTED)
		 &&
                 checkDisjoint(pos, flags,
                               PRIVATE,
                               PUBLIC | PROTECTED)
		 &&
		 checkDisjoint(pos, flags,
			       FINAL,
			       VOLATILE)
		 &&
		 (sym.kind == TYP ||
		  checkDisjoint(pos, flags,
				ABSTRACT | NATIVE,
				STRICTFP))) {
	    // skip
        }
        return flags & (mask | ~StandardFlags) | implicit;
    }


    /** Determine if this enum should be implicitly final.
     *
     *  If the enum has no specialized enum contants, it is final.
     *
     *  If the enum does have specialized enum contants, it is
     *  <i>not</i> final.
     */
    private long implicitEnumFinalFlag(Tree tree) {
	if (tree.tag != Tree.CLASSDEF) return 0;
        class SpecialTreeVisitor extends Tree.Visitor {
            boolean specialized;
            SpecialTreeVisitor() {
                this.specialized = false;
            };
		
            public void visitTree(Tree tree) { /* no-op */ }
		
            public void visitVarDef(VarDef tree) {
                if ((tree.mods.flags & ENUM) != 0) {
                    if (tree.init instanceof NewClass &&
                        ((NewClass)tree.init).def != null) {
                        specialized = true;
                    }
                }
            }
        }

        SpecialTreeVisitor sts = new SpecialTreeVisitor();
        ClassDef cdef = (ClassDef) tree;
        for (Tree defs: cdef.defs) {
            defs.accept(sts);
            if (sts.specialized) return 0;
        }
        return FINAL;
    }

/* *************************************************************************
 * Type Validation
 **************************************************************************/

    /** Validate a type expression. That is,
     *  check that all type arguments of a parametric type are within
     *  their bounds. This must be done in a second phase after type attributon
     *  since a class might have a subclass as type parameter bound. E.g:
     *
     *  class B<A extends C> { ... }
     *  class C extends B<C> { ... }
     *
     *  and we can't make sure that the bound is already attributed because
     *  of possible cycles.
     */
    private Validator validator = new Validator();

    /** Visitor method: Validate a type expression, if it is not null, catching
     *  and reporting any completion failures.
     */
    void validate(Tree tree) {
	try {
	    if (tree != null) tree.accept(validator);
	} catch (CompletionFailure ex) {
	    completionError(tree.pos, ex);
	}
    }

    /** Visitor method: Validate a list of type expressions.
     */
    void validate(List<Tree> trees) {
	for (List<Tree> l = trees; l.nonEmpty(); l = l.tail)
	    validate(l.head);
    }

    /** Visitor method: Validate a list of type parameters.
     */
    void validateTypeParams(List<TypeParameter> trees) {
	for (List<TypeParameter> l = trees; l.nonEmpty(); l = l.tail)
	    validate(l.head);
    }

    /** A visitor class for type validation.
     */
    class Validator extends Tree.Visitor {

        public void visitTypeArray(TypeArray tree) {
	    validate(tree.elemtype);
	}

        public void visitTypeApply(TypeApply tree) {
	    if (tree.type.tag == CLASS) {
		List<Type> formals = tree.type.tsym.type.typarams();
		List<Type> actuals = tree.type.typarams();
		List<Tree> args = tree.arguments;
		List<Type> forms = formals;
		ListBuffer<TypeVar> tvars_buf = new ListBuffer<TypeVar>();

		// For matching pairs of actual argument types `a' and
		// formal type parameters with declared bound `b' ...
		while (args.nonEmpty() && forms.nonEmpty()) {
		    validate(args.head);

		    // exact type arguments needs to know their
		    // bounds (for upper and lower bound
		    // calculations).  So we create new TypeVars with
		    // bounds substed with actuals.
		    tvars_buf.append(types.substBound(((TypeVar)forms.head),
						      formals,
						      Type.removeBounds(actuals)));

		    args = args.tail;
		    forms = forms.tail;
		}

		args = tree.arguments;
		List<TypeVar> tvars = tvars_buf.toList();
		while (args.nonEmpty() && tvars.nonEmpty()) {
		    // Let the actual arguments know their bound
		    args.head.type.withTypeVar(tvars.head);
		    args = args.tail;
		    tvars = tvars.tail;
		}

		args = tree.arguments;
		tvars = tvars_buf.toList();
		while (args.nonEmpty() && tvars.nonEmpty()) {
		    checkExtends(args.head.pos,
				 args.head.type,
				 tvars.head);
		    args = args.tail;
		    tvars = tvars.tail;
		}

                // Check that this type is either fully parameterized, or
                // not parameterized at all.
                if (tree.type.outer().isRaw())
                    log.error(tree.pos, "improperly.formed.type.inner.raw.param");
                if (tree.clazz.tag == Tree.SELECT)
                    visitSelectInternal((Select)tree.clazz);
	    }
	}

        public void visitTypeParameter(TypeParameter tree) {
	    validate(tree.bounds);
	    checkClassBounds(tree.pos, tree.type);
	}

        public void visitSelect(Select tree) {
	    if (tree.type.tag == CLASS) {
                visitSelectInternal(tree);

                // Check that this type is either fully parameterized, or
                // not parameterized at all.
                if (tree.selected.type.isParameterized() && tree.type.tsym.type.typarams().nonEmpty())
                    log.error(tree.pos, "improperly.formed.type.param.missing");
            }
	}
        public void visitSelectInternal(Select tree) {
            // If this is a nested class, validate qualifying type.
            if (tree.type.outer().tag == CLASS)
                validate(tree.selected);

            // Otherwise, check that the qualifying type is not parameterized
            else if (tree.selected.type.isParameterized())
                log.error(tree.pos,
                          "cant.select.static.class.from.param.type");
        }

	/** Default visitor method: do nothing.
	 */
	public void visitTree(Tree tree) {
	}
    }

/* *************************************************************************
 * Exception checking
 **************************************************************************/

    /* The following methods treat classes as sets that contain
     * the class itself and all their subclasses
     */

    /** Is given type a subtype of some of the types in given list?
     */
    boolean subset(Type t, List<Type> ts) {
	for (List<Type> l = ts; l.nonEmpty(); l = l.tail)
	    if (types.isSubType(t, l.head)) return true;
	return false;
    }

    /** Is given type a subtype or supertype of
     *  some of the types in given list?
     */
    boolean intersects(Type t, List<Type> ts) {
	for (List<Type> l = ts; l.nonEmpty(); l = l.tail)
	    if (types.isSubType(t, l.head) || types.isSubType(l.head, t)) return true;
	return false;
    }

    /** Add type set to given type list, unless it is a subclass of some class
     *  in the list.
     */
    List<Type> incl(Type t, List<Type> ts) {
	return subset(t, ts) ? ts : excl(t, ts).prepend(t);
    }

    /** Remove type set from type set list.
     */
    List<Type> excl(Type t, List<Type> ts) {
	if (ts.isEmpty()) {
	    return ts;
	} else {
	    List<Type> ts1 = excl(t, ts.tail);
	    if (types.isSubType(ts.head, t)) return ts1;
	    else if (ts1 == ts.tail) return ts;
	    else return ts1.prepend(ts.head);
	}
    }

    /** Form the union of two type set lists.
     */
    List<Type> union(List<Type> ts1, List<Type> ts2) {
	List<Type> ts = ts1;
	for (List<Type> l = ts2; l.nonEmpty(); l = l.tail)
	    ts = incl(l.head, ts);
	return ts;
    }

    /** Form the difference of two type lists.
     */
    List<Type> diff(List<Type> ts1, List<Type> ts2) {
	List<Type> ts = ts1;
	for (List<Type> l = ts2; l.nonEmpty(); l = l.tail)
	    ts = excl(l.head, ts);
	return ts;
    }

    /** Form the intersection of two type lists.
     */
    public List<Type> intersect(List<Type> ts1, List<Type> ts2) {
	List<Type> ts = Type.emptyList;
	for (List<Type> l = ts1; l.nonEmpty(); l = l.tail)
	    if (subset(l.head, ts2)) ts = incl(l.head, ts);
	for (List<Type> l = ts2; l.nonEmpty(); l = l.tail)
	    if (subset(l.head, ts1)) ts = incl(l.head, ts);
	return ts;
    }

    /** Is exc an exception symbol that need not be declared?
     */
    boolean isUnchecked(ClassSymbol exc) {
	return
	    exc.kind == ERR ||
	    exc.isSubClass(syms.errorType.tsym, types) ||
	    exc.isSubClass(syms.runtimeExceptionType.tsym, types);
    }

    /** Is exc an exception type that need not be declared?
     */
    boolean isUnchecked(Type exc) {
	return
	    (exc.tag == TYPEVAR) ? isUnchecked(types.supertype(exc)) :
	    (exc.tag == CLASS) ? isUnchecked((ClassSymbol)exc.tsym) :
	    exc.tag == BOT;
    }

    /** Same, but handling completion failures.
     */
    boolean isUnchecked(int pos, Type exc) {
	try {
	    return isUnchecked(exc);
	} catch (CompletionFailure ex) {
	    completionError(pos, ex);
	    return true;
	}
    }

    /** Is exc handled by given exception list?
     */
    boolean isHandled(Type exc, List<Type> handled) {
	return isUnchecked(exc) || subset(exc, handled);
    }

    /** Return all exceptions in thrown list that are not in handled list.
     *  @param thrown     The list of thrown exceptions.
     *  @param handled    The list of handled exceptions.
     */
    List<Type> unHandled(List<Type> thrown, List<Type> handled) {
	List<Type> unhandled = Type.emptyList;
	for (List<Type> l = thrown; l.nonEmpty(); l = l.tail)
	    if (!isHandled(l.head, handled)) unhandled = unhandled.prepend(l.head);
	return unhandled;
    }

/* *************************************************************************
 * Overriding/Implementation checking
 **************************************************************************/

    /** The level of access protection given by a flag set,
     *  where PRIVATE is highest and PUBLIC is lowest.
     */
    static int protection(long flags) {
        switch ((short)(flags & AccessFlags)) {
        case PRIVATE: return 3;
        case PROTECTED: return 1;
        default:
        case PUBLIC: return 0;
        case 0: return 2;
        }
    }

    /** A string describing the access permission given by a flag set.
     *  This always returns a space-separated list of Java Keywords.
     */
    private static String protectionString(long flags) {
	long flags1 = flags & AccessFlags;
	return (flags1 == 0) ? "package" : TreeInfo.flagNames(flags1);
    }

    /** A customized "cannot override" error message.
     *  @param m      The overriding method.
     *  @param other  The overridden method.
     *  @return       An internationalized string.
     */
    static Object cannotOverride(MethodSymbol m, MethodSymbol other) {
	String key =
	    ((other.owner.flags() & INTERFACE) == 0) ? "cant.override" :
	    ((m.owner.flags() & INTERFACE) == 0) ? "cant.implement" :
	    "clashes.with";
	return new Diagnostic(key, m, m.location(), other, other.location());
    }

    /** A customized "override" warning message.
     *  @param m      The overriding method.
     *  @param other  The overridden method.
     *  @return       An internationalized string.
     */
    static Object overrides(MethodSymbol m, MethodSymbol other) {
	String key =
	    ((other.owner.flags() & INTERFACE) == 0) ? "unchecked.override" :
	    ((m.owner.flags() & INTERFACE) == 0) ? "unchecked.implement" :
	    "unchecked.clash.with";
	return new Diagnostic(key, m, m.location(), other, other.location());
    }

    /** Check that this method conforms with overridden method 'other'.
     *  where `origin' is the class where checking started.
     *  Complications:
     *  (1) Do not check overriding of synthetic methods
     *      (reason: they might be final).
     *      todo: check whether this is still necessary.
     *  (2) Admit the case where an interface proxy throws fewer exceptions
     *      than the method it implements. Augment the proxy methods with the
     *      undeclared exceptions in this case.
     *  (3) When generics are enabled, admit the case where an interface proxy
     *	    has a result type
     *      extended by the result type of the method it implements.
     *      Change the proxies result type to the smaller type in this case.
     *
     *  @param tree         The tree from which positions
     *			    are extracted for errors.
     *  @param m            The overriding method.
     *  @param other        The overridden method.
     *  @param origin       The class of which the overriding method
     *			    is a member.
     */
    void checkOverride(Tree tree,
		       MethodSymbol m,
		       MethodSymbol other,
		       ClassSymbol origin) {
	// Don't check overiding of synthetic methods or by bridge methods.
	if ((m.flags() & (SYNTHETIC|BRIDGE)) != 0 || (other.flags() & SYNTHETIC) != 0) {
	}

	// Error if static method overrides instance method (JLS 8.4.6.2).
	else if ((m.flags() & STATIC) != 0 &&
		   (other.flags() & STATIC) == 0) {
	    log.error(TreeInfo.positionFor(m, tree), "override.static",
		      cannotOverride(m, other));
	}

	// Error if instance method overrides static or final
	// method (JLS 8.4.6.1).
	else if ((other.flags() & FINAL) != 0 ||
		 (m.flags() & STATIC) == 0 &&
		 (other.flags() & STATIC) != 0) {
	    log.error(TreeInfo.positionFor(m, tree), "override.meth",
		      cannotOverride(m, other),
		      TreeInfo.flagNames(other.flags() & (FINAL | STATIC)));
	}

        else if ((m.owner.flags() & ANNOTATION) != 0) {
            log.error(TreeInfo.positionFor(m, tree),
                      "annotation.override", other, other.owner);
        }

	// Emphatic warning if varargs don't agree (JLS: proposed).
	else if (((m.flags() ^ other.flags()) & Flags.VARARGS) != 0) {
	    log.warning(TreeInfo.positionFor(m, tree),
			((m.flags() & Flags.VARARGS) != 0)
			? "override.varargs.missing"
			: "override.varargs.extra",
			cannotOverride(m, other));
	}

	// Error if overriding method has weaker access (JLS 8.4.6.3).
	else if ((origin.flags() & INTERFACE) == 0 &&
		 protection(m.flags()) > protection(other.flags())) {
	    log.error(TreeInfo.positionFor(m, tree), "override.weaker.access",
		      cannotOverride(m, other),
		      protectionString(other.flags()));

	} else {
            // Warn if instance method overrides bridge method (compiler spec ??)
            if ((other.flags() & BRIDGE) != 0) {
                log.warning(TreeInfo.positionFor(m, tree), "override.bridge",
                            overrides(m, other));
            }

	    Type mt = types.memberType(origin.type, m);
	    Type ot = types.memberType(origin.type, other);
	    // Error if overriding result type is different
	    // (or, in the case of generics mode, not a subtype) of
	    // overridden result type. We have to rename any type parameters
	    // before comparing types.
	    List<Type> mtvars = mt.typarams();
	    List<Type> otvars = ot.typarams();
	    Type mtres = mt.restype();
	    Type otres = types.subst(ot.restype(), otvars, mtvars);

	    overrideWarner.warned = false;
	    boolean resultTypesOK =
		types.returnTypeSubstitutable(mt, ot, otres, overrideWarner);
	    if (!resultTypesOK) {
                if (!source.allowCovariantReturns() &&
                    m.owner != origin &&
                    m.owner.isSubClass(other.owner, types)) {
                    // allow limited interoperability with covariant returns
                } else {
                    typeError(TreeInfo.positionFor(m, tree),
                              new Diagnostic("override.incompatible.ret",
                                             cannotOverride(m, other)),
                              mtres, otres);
                }
	    } else if (overrideWarner.warned) {
		warnUnchecked(TreeInfo.positionFor(m, tree),
			      "prob.found.req",
			      new Diagnostic("override.unchecked.ret",
					     overrides(m, other)),
			      mtres, otres);
	    } else {
		// Error if overriding method throws an exception not reported
		// by overridden method.
		List<Type> otthrown = types.subst(ot.thrown(), otvars, mtvars);
		List<Type> unhandled = unHandled(mt.thrown(), otthrown);
		if (unhandled.nonEmpty()) {
		    log.error(TreeInfo.positionFor(m, tree),
			      "override.meth.doesnt.throw",
			      cannotOverride(m, other),
			      unhandled.head);
		}
	    }
	    // Warn if a deprecated method overridden by a non-deprecated one.
	    if ((other.flags() & DEPRECATED) != 0 &&
		(m.flags() & DEPRECATED) == 0 &&
		m.outermostClass() != other.outermostClass())
		warnDeprecated(TreeInfo.positionFor(m, tree), other);
	}
    }

    // used to check if there were any unchecked conversions
    Warner overrideWarner = new Warner();

    /** Check that a class does not inherit two concrete methods
     *  with the same signature.
     *  @param pos          Position to be used for error reporting.
     *  @param site         The class type to be checked.
     */
    public void checkCompatibleConcretes(int pos, Type site) {
	Type sup = types.supertype(site);
	if (sup.tag != CLASS) return;

	for (Type t1 = sup;
	     t1.tsym.type.isParameterized();
	     t1 = types.supertype(t1)) {
	    for (Scope.Entry e1 = t1.tsym.members().elems;
		 e1 != null;
		 e1 = e1.sibling) {
		Symbol s1 = e1.sym;
		if (s1.kind != MTH ||
		    (s1.flags() & (STATIC|SYNTHETIC|BRIDGE)) != 0 ||
		    !s1.isInheritedIn(site.tsym, types) ||
		    ((MethodSymbol)s1).implementation(site.tsym,
						      types,
						      true) != s1)
		    continue;
		Type st1 = types.memberType(t1, s1);
		int s1ArgsLength = st1.argtypes().length();
		if (st1 == s1.type) continue;

		for (Type t2 = sup;
		     t2.tag == CLASS;
		     t2 = types.supertype(t2)) {
		    for (Scope.Entry e2 = t1.tsym.members().lookup(s1.name);
			 e2.scope != null;
			 e2 = e2.next()) {
			Symbol s2 = e2.sym;
			if (s2 == s1 ||
			    s2.kind != MTH ||
			    (s2.flags() & (STATIC|SYNTHETIC|BRIDGE)) != 0 ||
			    s2.type.argtypes().length() != s1ArgsLength ||
			    !s2.isInheritedIn(site.tsym, types) ||
			    ((MethodSymbol)s2).implementation(site.tsym,
							      types,
							      true) != s2)
			    continue;
			Type st2 = types.memberType(t2, s2);
			if (types.overrideEquivalent(st1, st2))
			    log.error(pos, "concrete.inheritance.conflict",
				      s1, t1, s2, t2, sup);
		    }
		}
	    }
	}
    }

    /** Check that classes (or interfaces) do not each define an abstract
     *  method with same name and arguments but incompatible return types.
     *  @param pos          Position to be used for error reporting.
     *  @param t1           The first argument type.
     *  @param t2           The second argument type.
     */
    public boolean checkCompatibleAbstracts(int pos,
					    Type t1,
					    Type t2) {
        return checkCompatibleAbstracts(pos, t1, t2,
                                        types.makeCompoundType(t1, t2));
    }

    public boolean checkCompatibleAbstracts(int pos,
					    Type t1,
					    Type t2,
					    Type site) {
	Symbol sym = firstIncompatibility(t1, t2, site);
	if (sym != null) {
	    log.error(pos, "types.incompatible.diff.ret",
		      t1, t2, sym.name +
		      "(" + types.memberType(t2, sym).argtypes() + ")");
	    return false;
	}
	return true;
    }

    /** Return the first method which is defined with same args
     *  but different return types in two given interfaces, or null if none
     *  exists.
     *  @param t1     The first type.
     *  @param t2     The second type.
     *  @param site   The most derived type.
     *  @returns symbol from t2 that conflicts with one in t1.
     */
    private Symbol firstIncompatibility(Type t1, Type t2, Type site) {
	Map<TypeSymbol,Type> interfaces1 = new HashMap<TypeSymbol,Type>();
	closure(t1, interfaces1);
	Map<TypeSymbol,Type> interfaces2;
	if (t1 == t2)
	    interfaces2 = interfaces1;
	else
	    closure(t2, interfaces1, interfaces2 = new HashMap<TypeSymbol,Type>());

	for (Type t3 : interfaces1.values()) {
	    for (Type t4 : interfaces2.values()) {
		Symbol s = firstDirectIncompatibility(t3, t4, site);
		if (s != null) return s;
	    }
	}
	return null;
    }

    /** Compute all the supertypes of t, indexed by type symbol. */
    private void closure(Type t, Map<TypeSymbol,Type> typeMap) {
	if (t.tag != CLASS) return;
	if (typeMap.put(t.tsym, t) == null) {
	    closure(types.supertype(t), typeMap);
	    for (Type i : types.interfaces(t))
		closure(i, typeMap);
	}
    }

    /** Compute all the supertypes of t, indexed by type symbol (except thise in typesSkip). */
    private void closure(Type t, Map<TypeSymbol,Type> typesSkip, Map<TypeSymbol,Type> typeMap) {
	if (t.tag != CLASS) return;
	if (typesSkip.get(t.tsym) != null) return;
	if (typeMap.put(t.tsym, t) == null) {
	    closure(types.supertype(t), typesSkip, typeMap);
	    for (Type i : types.interfaces(t))
		closure(i, typesSkip, typeMap);
	}
    }

    /** Return the first method in t2 that conflicts with a method from t1. */
    private Symbol firstDirectIncompatibility(Type t1, Type t2, Type site) {
	for (Scope.Entry e1 = t1.tsym.members().elems; e1 != null; e1 = e1.sibling) {
	    Symbol s1 = e1.sym;
	    Type st1 = null;
	    if (s1.kind != MTH || !s1.isInheritedIn(site.tsym, types)) continue;
            Symbol impl = ((MethodSymbol)s1).implementation(site.tsym, types, false);
            if (impl != null && (impl.flags() & ABSTRACT) == 0) continue;
	    for (Scope.Entry e2 = t2.tsym.members().lookup(s1.name); e2.scope != null; e2 = e2.next()) {
		Symbol s2 = e2.sym;
		if (s1 == s2) continue;
		if (s2.kind != MTH || !s2.isInheritedIn(site.tsym, types)) continue;
		if (st1 == null) st1 = types.memberType(t1, s1);
		Type st2 = types.memberType(t2, s2);
		if (types.overrideEquivalent(st1, st2)) {
		    List<Type> tvars1 = st1.typarams();
		    List<Type> tvars2 = st2.typarams();
		    Type rt1 = st1.restype();
		    Type rt2 = types.subst(st2.restype(), tvars2, tvars1);
		    boolean compat =
			types.isSameType(rt1, rt2) ||
                        rt1.tag >= CLASS && rt2.tag >= CLASS &&
                        (types.covariantReturnType(rt1, rt2, Warner.noWarnings) ||
                         types.covariantReturnType(rt2, rt1, Warner.noWarnings));
		    if (!compat) return s2;
		}
	    }
	}
	return null;
    }

    /** Check that a given method conforms with any method it overrides.
     *  @param tree         The tree from which positions are extracted
     *			    for errors.
     *  @param m            The overriding method.
     */
    void checkOverride(Tree tree, MethodSymbol m) {
	ClassSymbol origin = (ClassSymbol)m.owner;
	for (Type t = types.supertype(origin.type); t.tag == CLASS;
	     t = types.supertype(t)) {
	    TypeSymbol c = t.tsym;
	    Scope.Entry e = c.members().lookup(m.name);
	    while (e.scope != null) {
		if (m.overrides(e.sym, origin, types, false))
		    checkOverride(tree, m, (MethodSymbol)e.sym, origin);
		e = e.next();
	    }
	}
    }

    /** Check that all abstract members of given class have definitions.
     *  @param pos          Position to be used for error reporting.
     *  @param c            The class.
     */
    void checkAllDefined(int pos, ClassSymbol c) {
	try {
	    MethodSymbol undef = firstUndef(c, c);
	    if (undef != null) {
                if ((c.flags() & ENUM) != 0 &&
                    types.supertype(c.type).tsym == syms.enumSym &&
                    (c.flags() & FINAL) == 0) {
                    // add the ABSTRACT flag to an enum
                    c.flags_field |= ABSTRACT;
                } else {
                    MethodSymbol undef1 =
                        new MethodSymbol(undef.flags(), undef.name,
                                         types.memberType(c.type, undef), undef.owner);
                    log.error(pos, "does.not.override.abstract",
                              c, undef1, undef1.location());
                }
            }
	} catch (CompletionFailure ex) {
	    completionError(pos, ex);
	}
    }
//where
        /** Return first abstract member of class `c' that is not defined
	 *  in `impl', null if there is none.
	 */
	private MethodSymbol firstUndef(ClassSymbol impl, ClassSymbol c) {
	    MethodSymbol undef = null;
	    // Do not bother to search in classes that are not abstract,
	    // since they cannot have abstract members.
	    if (c == impl || (c.flags() & (ABSTRACT | INTERFACE)) != 0) {
		Scope s = c.members();
		for (Scope.Entry e = s.elems;
		     undef == null && e != null;
		     e = e.sibling) {
		    if (e.sym.kind == MTH &&
			(e.sym.flags() & (ABSTRACT|IPROXY)) == ABSTRACT) {
			MethodSymbol absmeth = (MethodSymbol)e.sym;
			MethodSymbol implmeth = absmeth.implementation(impl, types, true);
			if (implmeth == null || implmeth == absmeth)
			    undef = absmeth;
		    }
		}
		if (undef == null) {
		    Type st = types.supertype(c.type);
		    if (st.tag == CLASS)
			undef = firstUndef(impl, (ClassSymbol)st.tsym);
		}
		for (List<Type> l = types.interfaces(c.type);
		     undef == null && l.nonEmpty();
		     l = l.tail) {
		    undef = firstUndef(impl, (ClassSymbol)l.head.tsym);
		}
	    }
	    return undef;
	}

    /** Check for cyclic references. Issue an error if the
     *  symbol of the type referred to has a LOCKED flag set.
     *
     *  @param pos      Position to be used for error reporting.
     *  @param t        The type referred to.
     */
    void checkNonCyclic(int pos, Type t) {
	checkNonCyclicInternal(pos, t);
    }

    /** Check for cyclic references. Issue an error if the
     *  symbol of the type referred to has a LOCKED flag set.
     *
     *  @param pos      Position to be used for error reporting.
     *  @param t        The type referred to.
     *  @returns        True if the check completed on all attributed classes
     */
    private boolean checkNonCyclicInternal(int pos, Type t) {
	boolean complete = true; // was the check complete?
	//- System.err.println("checkNonCyclicInternal("+t+");");//DEBUG
	Symbol c = t.tsym;
	if ((c.flags_field & ACYCLIC) != 0) return true;

	if ((c.flags_field & LOCKED) != 0) {
	    noteCyclic(pos, (ClassSymbol)c);
	} else if (!c.type.isErroneous()) {
	    try {
		c.flags_field |= LOCKED;
		if (c.type.tag == CLASS) {
		    ClassType clazz = (ClassType)c.type;
		    if (clazz.interfaces_field != null)
			for (List<Type> l=clazz.interfaces_field; l.nonEmpty(); l=l.tail)
			    complete &= checkNonCyclicInternal(pos, l.head);
		    if (clazz.supertype_field != null) {
			Type st = clazz.supertype_field;
			if (st != null && st.tag == CLASS)
			    complete &= checkNonCyclicInternal(pos, st);
		    }
		    if (c.owner.kind == TYP)
			complete &= checkNonCyclicInternal(pos, c.owner.type);
		}
	    } finally {
		c.flags_field &= ~LOCKED;
	    }
	}
	if (complete)
	    complete = ((c.flags_field & UNATTRIBUTED) == 0) && c.completer == null;
	if (complete) c.flags_field |= ACYCLIC;
	return complete;
    }

    /** Note that we found an inheritance cycle. */
    private void noteCyclic(int pos, ClassSymbol c) {
	log.error(pos, "cyclic.inheritance", c);
	for (List<Type> l=types.interfaces(c.type); l.nonEmpty(); l=l.tail)
	    l.head = new ErrorType((ClassSymbol)l.head.tsym);
	Type st = types.supertype(c.type);
	if (st.tag == CLASS)
	    ((ClassType)c.type).supertype_field = new ErrorType((ClassSymbol)st.tsym);
	c.type = new ErrorType(c);
	c.flags_field |= ACYCLIC;
    }

    /** Check that all methods which implement some
     *  method conform to the method they implement.
     *  @param tree         The class definition whose members are checked.
     */
    void checkImplementations(ClassDef tree) {
	checkImplementations(tree, tree.sym);
    }
//where
        /** Check that all methods which implement some
	 *  method in `ic' conform to the method they implement.
	 */
	void checkImplementations(ClassDef tree, ClassSymbol ic) {
	    ClassSymbol origin = tree.sym;
	    if ((allowGenerics || origin != ic) && (ic.flags() & ABSTRACT) != 0) {
		for (Scope.Entry e=ic.members().elems; e != null; e=e.sibling) {
		    if (e.sym.kind == MTH &&
			(e.sym.flags() & (STATIC|ABSTRACT)) == ABSTRACT) {
			MethodSymbol absmeth = (MethodSymbol)e.sym;
			MethodSymbol implmeth = absmeth.implementation(origin, types, false);
			if (implmeth != null && implmeth != absmeth &&
			    (implmeth.owner.flags() & INTERFACE) ==
			    (origin.flags() & INTERFACE)) {
			    // don't check if implmeth is in a class, yet
			    // origin is an interface. This case arises only
			    // if implmeth is declared in Object. The reason is
			    // that interfaces really don't inherit from
			    // Object it's just that the compiler represents
			    // things that way.
			    checkOverride(tree, implmeth, absmeth, origin);
			}
		    }
		}
	    }
	    Type st = types.supertype(ic.type);
	    if (st.tag == CLASS)
		checkImplementations(tree, (ClassSymbol)st.tsym);
	    for (List<Type> l = types.interfaces(ic.type); l.nonEmpty(); l = l.tail)
		checkImplementations(tree, (ClassSymbol)l.head.tsym);
	}

    /** Check that all abstract methods implemented by a class are
     *  mutually compatible.
     *  @param pos          Position to be used for error reporting.
     *  @param c            The class whose interfaces are checked.
     */
    void checkCompatibleSupertypes(int pos, Type c) {
	List<Type> supertypes = types.interfaces(c);
	Type supertype = types.supertype(c);
	if (supertype.tag == CLASS &&
	    (supertype.tsym.flags() & ABSTRACT) != 0)
	    supertypes = supertypes.prepend(supertype);
	for (List<Type> l = supertypes; l.nonEmpty(); l = l.tail) {
	    if (allowGenerics && !l.head.typarams().isEmpty() &&
		!checkCompatibleAbstracts(pos, l.head, l.head, c))
		return;
	    for (List<Type> m = supertypes; m != l; m = m.tail)
		if (!checkCompatibleAbstracts(pos, l.head, m.head, c))
		    return;
	}
	checkCompatibleConcretes(pos, c);
    }

    /** Check that class c does not implement directly or indirectly
     *  the same parameterized interface with two different argument lists.
     *  @param pos          Position to be used for error reporting.
     *  @param type         The type whose interfaces are checked.
     */
    void checkClassBounds(int pos, Type type) {
	checkClassBounds(pos, new HashMap<TypeSymbol,Type>(), type);
    }
//where
        /** Enter all interfaces of type `type' into the hash table `seensofar'
	 *  with their class symbol as key and their type as value. Make
	 *  sure no class is entered with two different types.
	 */
	void checkClassBounds(int pos,
			      Map<TypeSymbol,Type> seensofar,
			      Type type) {
	    if (type.isErroneous()) return;
	    for (List<Type> l = types.interfaces(type); l.nonEmpty(); l = l.tail) {
		Type it = l.head;
		Type oldit = seensofar.put(it.tsym, it);
		if (oldit != null) {
		    List<Type> oldparams = oldit.allparams();
		    List<Type> newparams = it.allparams();
		    if (!types.containsTypeEquivalent(oldparams, newparams))
			log.error(pos, "cant.inherit.diff.arg",
				  it.tsym, Type.toString(oldparams),
				  Type.toString(newparams));
		}
		checkClassBounds(pos, seensofar, it);
	    }
	    Type st = types.supertype(type);
	    if (st != null) checkClassBounds(pos, seensofar, st);
	}

    /** Enter interface into into set.
     *  If it existed already, issue a "repeated interface" error.
     */
    void checkNotRepeated(int pos, Type it, Set<Type> its) {
	if (its.contains(it))
	    log.error(pos, "repeated.interface");
	else {
	    its.add(it);
	}
    }
	
/* *************************************************************************
 * Check annotations
 **************************************************************************/

    /** Annotation types are restricted to primitives, String, an
     *  enum, an annotation, Class, Class<?>, Class<? extends
     *  Anything>, arrays of the preceding.
     */
    void validateAnnotationType(Tree restype) {
        // restype may be null if an error occurred, so don't bother validating it
        if (restype != null) {
            validateAnnotationType(restype.pos, restype.type);
        }
    }

    void validateAnnotationType(int pos, Type type) {
	if (type.isPrimitive()) return;
	if (types.isSameType(type, syms.stringType)) return;
        if ((type.tsym.flags() & Flags.ENUM) != 0) return;
	if ((type.tsym.flags() & Flags.ANNOTATION) != 0) return;
	if (types.lowerBound(type).tsym == syms.classType.tsym) return;
	if (types.isArray(type) && !types.isArray(types.elemtype(type))) {
	    validateAnnotationType(pos, types.elemtype(type));
	    return;
	}
	log.error(pos, "invalid.annotation.member.type");
    }

    /** Check the annotations of a symbol.
     */
    public void validateAnnotations(List<Annotation> annotations, Symbol s) {
	if (skipAnnotations) return;
	for (Annotation a : annotations)
	    validateAnnotation(a, s);
    }

    /** Check an annotation of a symbol.
     */
    public void validateAnnotation(Annotation a, Symbol s) {
	validateAnnotation(a);

	if (!annotationApplicable(a, s))
	    log.error(a.pos, "annotation.type.not.applicable");

	if (a.annotationType.type.tsym == syms.overrideType.tsym) {
	    if (!isOverrider(s))
		log.error(a.pos, "method.does.not.override.superclass");
	}
    }

    /** Is s a method symbol that overrides a method in a superclass? */
    boolean isOverrider(Symbol s) {
	if (s.kind != MTH) return false;
	MethodSymbol m = (MethodSymbol)s;
	TypeSymbol owner = (TypeSymbol) m.owner;
	for (Type sup = types.supertype(m.owner.type);
	     sup.tag == CLASS;
	     sup = types.supertype(sup)) {
	    for (Scope.Entry e = sup.tsym.members().lookup(m.name);
		 e.scope != null; e = e.next()) {
		if (m.overrides(e.sym, owner, types, true)) return true;
	    }
	}
	return false;
    }

    /** Is the annotation applicable to the symbol? */
    boolean annotationApplicable(Annotation a, Symbol s) {
	Attribute.Compound atTarget =
	    a.annotationType.type.tsym.attribute(syms.annotationTargetType.tsym);
	if (atTarget == null) return true;
	Attribute atValue = atTarget.member(names.value);
	if (!(atValue instanceof Attribute.Array)) return true; // error recovery
	Attribute.Array arr = (Attribute.Array) atValue;
	for (Attribute app : arr.values) {
	    if (!(app instanceof Attribute.Enum)) return true; // recovery
	    Attribute.Enum e = (Attribute.Enum) app;
	    if (e.value.name == names.TYPE)
		{ if (s.kind == TYP) return true; }
	    else if (e.value.name == names.FIELD)
		{ if (s.kind == VAR && s.owner.kind != MTH) return true; }
	    else if (e.value.name == names.METHOD)
		{ if (s.kind == MTH && !s.isConstructor()) return true; }
	    else if (e.value.name == names.PARAMETER)
		{ if (s.kind == VAR &&
		      s.owner.kind == MTH &&
		      (s.flags() & PARAMETER) != 0)
		    return true;
		}
	    else if (e.value.name == names.CONSTRUCTOR)
		{ if (s.kind == MTH && s.isConstructor()) return true; }
	    else if (e.value.name == names.LOCAL_VARIABLE)
		{ if (s.kind == VAR && s.owner.kind == MTH &&
		      (s.flags() & PARAMETER) == 0)
		    return true;
		}
	    else if (e.value.name == names.ANNOTATION_TYPE)
		{ if (s.kind == TYP && (s.flags() & ANNOTATION) != 0)
		    return true;
		}
	    else if (e.value.name == names.PACKAGE)
		{ if (s.kind == PCK) return true; }
	    else
		return true; // recovery
	}
	return false;
    }

    /** Check an annotation value.
     */
    public void validateAnnotation(Annotation a) {
        if (a.type.isErroneous()) return;

	// collect an inventory of the members
	Set<MethodSymbol> members = new HashSet<MethodSymbol>();
	for (Scope.Entry e = a.annotationType.type.tsym.members().elems;
	     e != null;
	     e = e.sibling)
	    if (e.sym.kind == MTH)
                members.add((MethodSymbol) e.sym);

	// count them off as they're annotated
	for (Tree arg : a.args) {
	    if (arg.tag != Tree.ASSIGN) continue; // recovery
	    Assign assign = (Assign) arg;
	    Symbol m = TreeInfo.symbol(assign.lhs);
	    if (m == null || m.type.isErroneous()) continue;
	    if (!members.remove(m))
		log.error(arg.pos, "duplicate.annotation.member.value",
			  m.name, a.type);
	    if (assign.rhs.tag == ANNOTATION)
		validateAnnotation((Annotation)assign.rhs);
	}

	// all the remaining ones better have default values
	for (MethodSymbol m : members)
	    if (m.defaultValue == null && !m.type.isErroneous())
		log.error(a.pos, "annotation.missing.default.value", 
                          a.type, m.name);

	// special case: java.lang.annotation.Target must not have
	// repeated values in its value member
	if (a.annotationType.type.tsym != syms.annotationTargetType.tsym ||
	    a.args.tail == null)
	    return;

        if (a.args.head.tag != Tree.ASSIGN) return; // error recovery
	Assign assign = (Assign) a.args.head;
	Symbol m = TreeInfo.symbol(assign.lhs);
	if (m.name != names.value) return;
	Tree rhs = assign.rhs;
	if (rhs.tag != Tree.NEWARRAY) return;
	NewArray na = (NewArray) rhs;
	Set<Symbol> targets = new HashSet<Symbol>();
	for (Tree elem : na.elems) {
	    if (!targets.add(TreeInfo.symbol(elem))) {
		log.error(elem.pos, "repeated.annotation.target");
	    }
	}
    }

    void checkDeprecatedAnnotation(int pos, Symbol s) {
	if (checkDeprecatedAnnotation &&
	    allowAnnotations &&
	    (s.flags() & DEPRECATED) != 0 &&
	    !syms.deprecatedType.isErroneous() &&
	    s.attribute(syms.deprecatedType.tsym) == null)
	    log.warning(pos, "missing.deprecated.annotation");
    }

/* *************************************************************************
 * Check for recursive annotation elements.
 **************************************************************************/

    /** Check for cycles in the graph of annotation elements.
     */
    void checkNonCyclicElements(ClassDef tree) {
        if ((tree.sym.flags_field & ANNOTATION) == 0) return;
        assert (tree.sym.flags_field & LOCKED) == 0;
        try {
            tree.sym.flags_field |= LOCKED;
            for (Tree def : tree.defs) {
                if (def.tag != Tree.METHODDEF) continue;
                MethodDef meth = (MethodDef)def;
                checkAnnotationResType(meth.pos, meth.restype.type);
            }
        } finally {
            tree.sym.flags_field &= ~LOCKED;
            tree.sym.flags_field |= ACYCLIC_ANN;
        }
    }

    void checkNonCyclicElementsInternal(int pos, TypeSymbol tsym) {
        if ((tsym.flags_field & ACYCLIC_ANN) != 0)
            return;
        if ((tsym.flags_field & LOCKED) != 0) {
            log.error(pos, "cyclic.annotation.element");
            return;
        }
        try {
            tsym.flags_field |= LOCKED;
            for (Scope.Entry e = tsym.members().elems; e != null; e = e.sibling) {
                Symbol s = e.sym;
                if (s.kind != Kinds.MTH)
                    continue;
                checkAnnotationResType(pos, ((MethodSymbol)s).type.restype());
            }
        } finally {
            tsym.flags_field &= ~LOCKED;
            tsym.flags_field |= ACYCLIC_ANN;
        }
    }

    void checkAnnotationResType(int pos, Type type) {
        switch (type.tag) {
        case TypeTags.CLASS:
            if ((type.tsym.flags() & ANNOTATION) != 0)
                checkNonCyclicElementsInternal(pos, type.tsym);
            break;
        case TypeTags.ARRAY:
            checkAnnotationResType(pos, types.elemtype(type));
            break;
        default:
            break; // int etc
        }
    }

/* *************************************************************************
 * Check for cycles in the constructor call graph.
 **************************************************************************/

    /** Check for cycles in the graph of constructors calling other
     *  constructors.
     */
    void checkCyclicConstructors(ClassDef tree) {
	Map<Symbol,Symbol> callMap = new HashMap<Symbol, Symbol>();

	// enter each constructor this-call into the map
	for (List<Tree> l = tree.defs; l.nonEmpty(); l = l.tail) {
	    Apply app = TreeInfo.firstConstructorCall(l.head);
	    if (app == null) continue;
	    MethodDef meth = (MethodDef) l.head;
	    if (TreeInfo.name(app.meth) == names._this) {
		callMap.put(meth.sym, TreeInfo.symbol(app.meth));
	    } else {
		meth.sym.flags_field |= ACYCLIC;
	    }
	}

	// Check for cycles in the map
	Symbol[] ctors = new Symbol[0];
	ctors = callMap.keySet().toArray(ctors);
	for (Symbol caller : ctors) {
	    checkCyclicConstructor(tree, caller, callMap);
	}
    }

    /** Look in the map to see if the given constructor is part of a
     *  call cycle.
     */
    private void checkCyclicConstructor(ClassDef tree, Symbol ctor,
					Map<Symbol,Symbol> callMap) {
	if (ctor != null && (ctor.flags_field & ACYCLIC) == 0) {
	    if ((ctor.flags_field & LOCKED) != 0) {
		log.error(TreeInfo.positionFor(ctor, tree),
			  "recursive.ctor.invocation");
	    } else {
		ctor.flags_field |= LOCKED;
		checkCyclicConstructor(tree, callMap.remove(ctor), callMap);
		ctor.flags_field &= ~LOCKED;
	    }
	    ctor.flags_field |= ACYCLIC;
	}
    }

/* *************************************************************************
 * Miscellaneous
 **************************************************************************/

    /** Check that symbol is unique in given scope.
     *	@param pos	     Position for error reporting.
     *	@param sym	     The symbol.
     *	@param s	     The scope.
     */
    boolean checkUnique(int pos, Symbol sym, Scope s) {
	if (sym.owner.name == names.any) return false;
	for (Scope.Entry e = s.lookup(sym.name); e.scope == s; e = e.next()) {
	    if (sym != e.sym &&
		sym.kind == e.sym.kind &&
		sym.name != names.error &&
		(sym.kind != MTH || types.overrideEquivalent(sym.type, e.sym.type))) {
		duplicateError(pos, e.sym);
		return false;
	    }
	}
	return true;
    }

    /** Check that single-type import is not already imported or top-level defined,
     *	but make an exception for two single-type imports which denote the same type.
     *	@param pos	     Position for error reporting.
     *	@param sym	     The symbol.
     *	@param env	     The current environment.
     */
    boolean checkUniqueImport(int pos, Symbol sym, Scope s) {
	for (Scope.Entry e = s.lookup(sym.name); e.scope != null; e = e.next()) {
	    // is encountered class entered via a class declaration?
	    boolean isClassDecl = e.scope == s;
	    if ((isClassDecl || sym != e.sym) &&
		sym.kind == e.sym.kind &&
		sym.name != names.error) {
		if (!e.sym.type.isErroneous()) {
		    String what = e.sym.toString();
		    if (!isClassDecl)
			log.error(pos, "already.defined.single.import", what);
		    else if (sym != e.sym)
			log.error(pos, "already.defined.this.unit", what);
		}
		return false;
	    }
	}
	return true;
    }

    /** Check that a qualified name is in canonical form (for import decls).
     */
    public void checkCanonical(Tree tree) {
	if (!isCanonical(tree))
	    log.error(tree.pos, "import.requires.canonical",
		      TreeInfo.symbol(tree));
    }
        // where
	private boolean isCanonical(Tree tree) {
	    while (tree.tag == Tree.SELECT) {
		Select s = (Select) tree;
		if (s.sym.owner != TreeInfo.symbol(s.selected))
		    return false;
		tree = s.selected;
	    }
	    return true;
	}

    private class ConversionWarner extends Warner {
        final String key;
	final Type found;
        final Type expected;
	public ConversionWarner(int pos, String key, Type found, Type expected) {
            super(pos);
            this.key = key;
	    this.found = found;
	    this.expected = expected;
	}

	public void warnUnchecked() {
            boolean warned = this.warned;
            super.warnUnchecked();
            if (warned) return; // suppress redundant diagnostics
	    Object problem = new Diagnostic(key);
	    Check.this.warnUnchecked(pos(), "prob.found.req", problem, found, expected);
	}
    }

    public Warner castWarner(int pos, Type found, Type expected) {
	return new ConversionWarner(pos, "unchecked.cast.to.type", found, expected);
    }

    public Warner convertWarner(int pos, Type found, Type expected) {
	return new ConversionWarner(pos, "unchecked.assign", found, expected);
    }
}
