/**
 * @(#)Types.java	1.48 04/07/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.code;

import java.util.*;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.util.List;

import com.sun.tools.javac.jvm.ClassReader;
import com.sun.tools.javac.comp.Infer;
import com.sun.tools.javac.comp.Check;

import static com.sun.tools.javac.code.Type.*;
import static com.sun.tools.javac.code.TypeTags.*;
import static com.sun.tools.javac.code.Symbol.*;
import static com.sun.tools.javac.code.Flags.*;
import static com.sun.tools.javac.code.BoundKind.*;

/** Utility class containing various operations on types.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Types {
    protected static final Context.Key<Types> typesKey =
	new Context.Key<Types>();

    final Symtab syms;
    final Name.Table names;
    final boolean allowBoxing;
    final ClassReader reader;
    final Source source;
    final Check chk;

    public static Types instance(Context context) {
	Types instance = context.get(typesKey);
	if (instance == null)
	    instance = new Types(context);
	return instance;
    }

    protected Types(Context context) {
	context.put(typesKey, this);
	syms = Symtab.instance(context);
	names = Name.Table.instance(context);
	allowBoxing = Source.instance(context).allowBoxing();
	reader = ClassReader.instance(context);
        source = Source.instance(context);
        chk = Check.instance(context);
    }

    /************************************************************
     * Type upperBound(Type t)
     ************************************************************/

    /** The "rvalue conversion". */
    public Type upperBound(Type t) {
	t.accept(upperBoundFcn);
	return upperBoundFcn.result;
    }
    private final UpperBoundFcn upperBoundFcn = new UpperBoundFcn();
    private class UpperBoundFcn extends Visitor {
	Type result;
	public void visitType(Type t) {
	    result = t;
	    return;
	}
        public void visitForAll(ForAll t) {
	    result = t;
	    return;
        }
	public void visitArgumentType(ArgumentType t) {
	    if (t.isSuperBound()) {
		if (t.bound == null) {
		    result = syms.objectType;
		    return;
		} else {
		    result = t.bound.bound;
		    return;
		}
	    } else {
		result = t.type;
		return;
	    }
	}
	public void visitCapturedType(CapturedType t) {
	    result = t.bound;
	}
	public void visitUndetVar(UndetVar t) {
	    result = t;
	    return;
	}
    }

    /************************************************************
     * Type lowerBound(Type t)
     ************************************************************/

    /** The "lvalue conversion". */
    public Type lowerBound(Type t) {
	t.accept(lowerBoundFcn);
	return lowerBoundFcn.result;
    }
    private final LowerBoundFcn lowerBoundFcn = new LowerBoundFcn();
    private class LowerBoundFcn extends Visitor {
	Type result;
	public void visitType(Type t) {
	    result = t;
	}
        public void visitForAll(ForAll t) {
	    result = t;
        }
	public void visitArgumentType(ArgumentType t) {
	    result = t.isExtendsBound() ? syms.botType : t.type;
	}
	public void visitCapturedType(CapturedType t) {
	    result = t.lower;
	}
	public void visitUndetVar(UndetVar t) {
	    result = t;
	}
    }

    /************************************************************
     * boolean isUnbounded(Type t)
     ************************************************************/

    /** Checks that all the arguments to a class are unbounded
     *  wildcards or something else that doesn't make any restrictions
     *  on the arguments. If a class isUnbounded, a raw super- or
     *  subclass can be cast to it without a warning.
     */
    public boolean isUnbounded(Type t) {
	t.accept(isUnboundedFcn);
	return isUnboundedFcn.result;
    }
    private final IsUnboundedFcn isUnboundedFcn = new IsUnboundedFcn();
    private class IsUnboundedFcn extends Visitor {
	boolean result;
	public void visitType(Type t) {
	    result = true;
	    return;
	}
	public void visitForAll(ForAll t) {
	    t.qtype.accept(this);
	}
	public void visitClassType(ClassType t) {
            List<Type> parms = t.tsym.type.allparams();
            List<Type> args = t.allparams();
            while (parms.nonEmpty()) {
                ArgumentType unb = new ArgumentType(syms.objectType,
						    BoundKind.UNBOUND,
						    syms.boundClass,
						    (TypeVar)parms.head);
                if (!containsType(args.head, unb)) {
		    result = false;
		    return;
		}
                parms = parms.tail;
                args = args.tail;
            }
	    result = true;
	    return;
	}
    }

    /************************************************************
     * Type asSub(Type t, Symbol sym)
     ************************************************************/

    /** Return the least specific subtype of t that starts
     *  with symbol sym. if none exists, return null.
     *  The least specific subtype is determined as follows:
     *
     *  If there is exactly one parameterized instance of sym
     *  that is a subtype of t, that parameterized instance is returned.
     *  Otherwise, if the plain type or raw type `sym' is a subtype
     *  of type t, the type `sym' itself is returned.
     *  Otherwise, null is returned.
     */
    public Type asSub(Type t, Symbol sym) {
	return asSubFcn.asSub(t, sym);
    }
    private final AsSubFcn asSubFcn = new AsSubFcn();
    private class AsSubFcn extends Visitor {
	Symbol sym;
	Type result;
	public Type asSub(Type t, Symbol sym) {
	    Symbol oldSym = this.sym;
	    this.sym = sym;
	    try {
		t.accept(this);
		return result;
	    } finally {
		this.sym = oldSym;
	    }
	}
	public void visitErrorType(ErrorType t) {
	    result = t;
	    return;
	}
	public void visitType(Type t) {
	    result = null;
	    return;
	}
	public void visitClassType(ClassType t) {
	    if (t.tsym == sym) {
		result = t;
		return;
	    } else {
		Type base = asSuper(sym.type, t.tsym);
		if (base == null) {
		    result = null;
		    return;
		}
		ListBuffer<Type> from = new ListBuffer<Type>();
		ListBuffer<Type> to = new ListBuffer<Type>();
		adapt(base, t, from, to);
		Type res = subst(sym.type, from.toList(), to.toList());
		if (!isSubType(res, t)) {
		    result = null;
		    return;
		}
                ListBuffer<Type> openVars = new ListBuffer<Type>();
		for (List<Type> l = sym.type.allparams();
		     l.nonEmpty(); l = l.tail)
		    if (res.contains(l.head) && !t.contains(l.head))
			openVars.append(l.head);
                if (openVars.nonEmpty()) {
                    if (t.isRaw()) {
                        // The subtype of a raw type is raw
                        res = erasure(res);
                    } else {
                        // Unbound type arguments default to ?
                        List<Type> opens = openVars.toList();
                        ListBuffer<Type> qs = new ListBuffer<Type>();
                        for (List<Type> iter = opens; iter.nonEmpty(); iter = iter.tail) {
                            qs.append(new ArgumentType(syms.objectType, BoundKind.UNBOUND, syms.boundClass, (TypeVar) iter.head));
                        }
                        res = subst(res, opens, qs.toList());
                    }
                }
		result = res;
		return;
	    }
	}
    }

    /************************************************************
     * boolean isConvertible(Type t, Type other, Warner warn)
     * boolean isConvertible(Type t, Type other)
     ************************************************************/

    /** Is t a subtype of or convertiable via boxing/unboxing
     *  convertions to other?
     */
    public boolean isConvertible(Type t, Type other, Warner warn) {
	boolean tPrimitive = t.isPrimitive();
	boolean otherPrimitive = other.isPrimitive();
	if (tPrimitive == otherPrimitive)
            return isSubTypeUnchecked(t, other, warn);
	if (!allowBoxing) return false;
	return tPrimitive
	    ? isSubType(boxedClass(t).type, other, warn)
	    : isSubType(unboxedType(t), other);
    }

    /** Is t a subtype of or convertiable via boxing/unboxing
     *  convertions to other?
     */
    public boolean isConvertible(Type t, Type other) {
	return isConvertible(t, other, Warner.noWarnings);
    }

    /************************************************************
     * boolean isSubType(Type t, Type other, Warner warn)
     * boolean isSubType(Type t, Type other)
     * boolean isSubTypeUnchecked(Type t, Type other)
     * boolean isSubTypeUnchecked(Type t, Type other, Warner warn)
     ************************************************************/

    /** Is t an unchecked subtype of other?
     */
    public boolean isSubTypeUnchecked(Type t, Type other) {
        return isSubTypeUnchecked(t, other, Warner.noWarnings);
    }
    /** Is t an unchecked subtype of other?
     */
    public boolean isSubTypeUnchecked(Type t, Type other, Warner warn) {
        if (t.tag == ARRAY && other.tag == ARRAY) {
            return (((ArrayType)t).elemtype.tag <= lastBaseTag)
                ? isSameType(elemtype(t), elemtype(other))
                : isSubTypeUnchecked(elemtype(t), elemtype(other), warn);
        } else if (isSubType(t, other, warn)) {
            return true;
        } else if (!other.isRaw()) {
            Type t2 = asSuper(t, other.tsym);
	    if (t2 != null && t2.isRaw()) {
                if (isReifiable(other))
                    warn.silentUnchecked();
                else
                    warn.warnUnchecked();
                return true;
            }
        }
        return false;
    }

    /** Is t a subtype of other?
     *  (not defined for Method and ForAll types)
     */
    public boolean isSubType(Type t, Type other, Warner warn) {
	return isSubTypeFcn.isSubType(t, other, warn);
    }
    public boolean isSubType(Type t, Type other) {
	return isSubTypeFcn.isSubType(t, other, Warner.noWarnings);
    }
    private IsSubTypeFcn isSubTypeFcn = new IsSubTypeFcn();
    private class IsSubTypeFcn extends Visitor {
	Type that;
	Warner warn;
	boolean result;
	boolean isSubType(Type t, Type that) {
	    return isSubType(t, that, Warner.noWarnings);
	}
	boolean isSubType(Type t, Type that, Warner warn) {
	    if (that.tag < firstPartialTag) t = capture(t);
	    Type oldThat = this.that;
	    Warner oldWarn = this.warn;
	    this.that = that;
	    this.warn = warn;
	    try {
		t.accept(this);
		return result;
	    } finally {
		this.that = oldThat;
		this.warn = oldWarn;
	    }
	}
	public void visitErrorType(ErrorType t) {
	    result = true;
	    return;
	}
	public void visitType(Type t) {
	    if (t == that) {
		result = true;
		return;
	    }
	    if (t == lowerBound(that)) { // Optimization
		result = true;
		return;
	    }
	    if (that.tag >= firstPartialTag) {
		result = isSuperType(that, t);
		return;
	    }
	    if (that.tag == TYPEARG) {
		result = isSubType(t, lowerBound(that), warn);
		return;
	    }
	    switch (t.tag) {
	    case BYTE: case CHAR:
		result = (t.tag == that.tag ||
			  t.tag + 2 <= that.tag && that.tag <= DOUBLE);
		return;
	    case SHORT: case INT: case LONG: case FLOAT: case DOUBLE:
		result = t.tag <= that.tag && that.tag <= DOUBLE;
		return;
	    case BOOLEAN: case VOID:
		result = t.tag == that.tag;
		return;
	    case TYPEVAR:
		result = isSubType(t.bound(), that, warn);
		return;
	    case BOT:
		result =
		    that.tag == BOT || that.tag == CLASS ||
		    that.tag == ARRAY || that.tag == TYPEVAR;
		return;
	    case NONE:
		result = false;
		return;
	    default:
		throw new AssertionError("isSubType " + t.tag);
	    }
	}
	public void visitArgumentType(ArgumentType t) {
	    result = isSubType(upperBound(t), lowerBound(that), warn);
	    return;
	}
	public void visitClassType(ClassType t) {
	    if (t == that) {
		result = true;
		return;
	    }
	    Type lower = lowerBound(that);
	    if (t == lower) { // Optimization
		result = true;
		return;
	    }
	    if (that.tag >= firstPartialTag) {
		result = isSuperType(that, t);
		return;
	    }
            if (that != lower) {
                result = isSubType(t, lower, warn);
                return;
            }
            Type sup = asSuper(t, that.tsym);
            result =
                sup != null &&
                sup.tsym == that.tsym &&
                (!that.isParameterized() ||
                 // You're not allowed to write
                 //     Vector<String> vec = new Vector<Object>();
                 // But with wildcards you can write
                 //     Vector<? extends String> vec = new Vector<Object>();
                 // which means that subtype checking must be done
                 // here instead of same-type checking.
                 containsType(that.typarams(), sup.typarams())) &&
                isSubType(sup.outer(), that.outer(), warn);
            return;
	}
	public void visitArrayType(ArrayType t) {
	    if (t == that) {
		result = true;
		return;
	    }
	    if (t == lowerBound(that)) { // Optimization
		result = true;
		return;
	    }
	    if (that.tag >= firstPartialTag) {
		result = isSuperType(that, t);
		return;
	    }
	    if (that.tag == TYPEARG) {
		result = isSubType(t, lowerBound(that), warn);
		return;
	    }
	    if (that.tag == ARRAY) {
		ArrayType athat = (ArrayType) that;
		if (t.elemtype.tag <= lastBaseTag) {
		    result = isSameType(t.elemtype, elemtype(that));
		    return;
		}
                result = isSubType(t.elemtype, elemtype(that), warn);
                return;
	    } else if (that.tag == CLASS) {
		Name thatname = that.tsym.fullName();
		result = (thatname == names.java_lang_Object ||
			  thatname == names.java_lang_Cloneable ||
			  thatname == names.java_io_Serializable);
		return;
	    } else {
		result = false;
		return;
	    }
	}
	public void visitUndetVar(UndetVar t) {
	    //todo: test against origin needed? or replace with substitution?
	    if (t == that || t.qtype == that ||
		that.tag == ERROR || that.tag == UNKNOWN) {
		result = true;
		return;
	    }
	    if (t.inst != null) {
		result = isSubType(t.inst, that); // TODO: ", warn"?
		return;
	    }
	    t.hibounds = t.hibounds.prepend(that);
	    result = true;
	    return;
	}
    }

    /************************************************************
     * boolean isSuperType(Type t, Type that)
     ************************************************************/

    /** Is Type t a supertype of that type?
     */
    public boolean isSuperType(Type t, Type that) {
	switch (t.tag) {
	case ERROR:
	    return true;
	case UNDETVAR: {
	    UndetVar undet = (UndetVar)t;
	    if (t == that ||
		undet.qtype == that ||
		that.tag == ERROR ||
		that.tag == BOT) return true;
	    if (undet.inst != null)
		return isSubType(that, undet.inst);
	    undet.lobounds = undet.lobounds.prepend(that);
	    return true;
	}
	default:
	    return isSubType(that, t);
	}
    }

    /************************************************************
     * boolean isSubTypeUnchecked(Type t, List<Type> those, Warner warn)
     ************************************************************/

    /** Is t a subtype of every type in given list `those'?
     *  (not defined for Method and ForAll types)
     *  Allows unchecked conversions.
     */
    public boolean isSubTypeUnchecked(Type t, List<Type> those, Warner warn) {
	for (List<Type> l = those; l.nonEmpty(); l = l.tail)
	    if (!isSubTypeUnchecked(t, l.head, warn))
                return false;
	return true;
    }

    /************************************************************
     * boolean isSubTypes(List<Type> these, List<Type> those)
     * boolean isSubTypesUnchecked(List<Type> these, List<Type> those, Warner warn)
     ************************************************************/

    /** Are corresponding elements of these subtypes of those?  If
     *  lists are of different length, return false.
     **/
    public boolean isSubTypes(List<Type> these, List<Type> those) {
	while (these.tail != null && those.tail != null
	       /*inlined: these.nonEmpty() && those.nonEmpty()*/ &&
	       isSubType(these.head, those.head)) {
	    these = these.tail;
	    those = those.tail;
	}
	return these.tail == null && those.tail == null;
        /*inlined: these.isEmpty() && those.isEmpty();*/
    }

    /** Are corresponding elements of these subtypes of those,
     *  allowing unchecked conversions?  If lists are of different
     *  length, return false.
     **/
    public boolean isSubTypesUnchecked(List<Type> these, List<Type> those, Warner warn) {
	while (these.tail != null && those.tail != null
	       /*inlined: these.nonEmpty() && those.nonEmpty()*/ &&
	       isSubTypeUnchecked(these.head, those.head, warn)) {
	    these = these.tail;
	    those = those.tail;
	}
	return these.tail == null && those.tail == null;
        /*inlined: these.isEmpty() && those.isEmpty();*/
    }

    /************************************************************
     * boolean isSameTypes(List<Type> these, List<Type> those)
     ************************************************************/

    /** Are corresponding elements of the type type lists the same
     ** type?  If lists are of different length, return false.
     */
    public boolean isSameTypes(List<Type> these, List<Type> those) {
	while (these.tail != null && those.tail != null
	       /*inlined: these.nonEmpty() && those.nonEmpty()*/ &&
	       isSameType(these.head, those.head)) {
	    these = these.tail;
	    those = those.tail;
	}
	return these.tail == null && those.tail == null;
        /*inlined: these.isEmpty() && those.isEmpty();*/
    }

    /************************************************************
     * boolean isSameType(Type t, Type that)
     ************************************************************/

    /** Is Type t the same as that?
     */
    public boolean isSameType(Type t, Type that) {
	return isSameTypeFcn.isSameType(t, that);
    }
    private IsSameTypeFcn isSameTypeFcn = new IsSameTypeFcn();
    private class IsSameTypeFcn extends Visitor {
	Type that;
	boolean result;
	boolean isSameType(Type t, Type that) {
	    Type oldThat = this.that;
	    this.that = that;
	    try {
		t.accept(this);
		return this.result;
	    } finally {
		this.that = oldThat;
	    }
	}
	public void visitType(Type t) {
	    if (t == that) {
		result = true;
		return;
	    }
	    if (that.tag >= firstPartialTag) {
		result = isSameType(that, t);
		return;
	    }
	    switch (t.tag) {
	    case BYTE: case CHAR: case SHORT: case INT: case LONG: case FLOAT:
	    case DOUBLE: case BOOLEAN: case VOID: case BOT: case NONE:
		result = t.tag == that.tag;
		return;
	    case TYPEVAR:
		result =
		    that.isSuperBound() &&
		    !that.isExtendsBound() &&
		    isSameType(t, upperBound(that));
		return;
	    default:
		throw new AssertionError("isSameType " + t.tag);
	    }
	}
	public void visitArgumentType(ArgumentType t) {
	    if (that.tag >= firstPartialTag) {
		result = isSameType(that, t);
		return;
	    }
	    result = false;
	    return;
	}
	public void visitClassType(ClassType t) {
	    if (t == that) {
		result = true;
		return;
	    }
	    if (that.tag >= firstPartialTag) {
		result = isSameType(that, t);
		return;
	    }
	    if (that.isSuperBound() && !that.isExtendsBound()) {
		result = isSameType(t, upperBound(that)) && isSameType(t, lowerBound(that));
		return;
	    }
            if ((t.tsym.flags() & that.tsym.flags() & COMPOUND) != 0) {
                if (!isSameType(supertype(t), supertype(that))) {
                    result = false;
                    return;
                }
                HashSet<SingletonType> set = new HashSet<SingletonType>();
                for (Type x : interfaces(t))
                    set.add(new SingletonType(x));
                for (Type x : interfaces(that)) {
                    if (!set.remove(new SingletonType(x))) {
                        result = false;
                        return;
                    }
                }
                result = (set.size() == 0);
                return;
            }
            result =
                t.tsym == that.tsym &&
                isSameType(t.outer(), that.outer()) &&
                containsTypeEquivalent(t.typarams(), that.typarams());
	    return;
	}
	public void visitArrayType(ArrayType t) {
	    if (t == that) {
		result = true;
		return;
	    }
	    if (that.tag >= firstPartialTag) {
		result = isSameType(that, t);
		return;
	    }
	    result =
		that.tag == ARRAY &&
		containsTypeEquivalent(t.elemtype, elemtype(that));
	    return;
	}
	/** isSameType for methods does not take thrown exceptions into account!
	 */
	public void visitMethodType(MethodType t) {
	    result = hasSameArgs(t, that) &&
                isSameType(t.restype(), that.restype());
	    return;
	}
	public void visitPackageType(PackageType t) {
	    result = (t == that);
	    return;
	}
	public void visitForAll(ForAll t) {
	    result =
		that.tag == FORALL &&
		hasSameBounds(t, (ForAll)that) &&
		isSameType(t.qtype,
			   subst(((ForAll)that).qtype,
				 ((ForAll)that).tvars,
				 t.tvars));
	    return;
	}
	public void visitUndetVar(UndetVar t) {
	    if (that.tag == TYPEARG) {
		// FIXME, this might be leftovers from before capture conversion
		result = false;
		return;
	    }
	    if (t == that || t.qtype == that ||
		that.tag == ERROR || that.tag == UNKNOWN) {
		result = true;
		return;
	    }
	    if (t.inst != null) {
		result = isSameType(t.inst, that);
		return;
	    }
	    t.inst = fromUnknownFun.apply(that);
	    for (List<Type> l = t.lobounds; l.nonEmpty(); l = l.tail) {
		if (!isSubType(l.head, t.inst)) {
		    result = false;
		    return;
		}
	    }
	    for (List<Type> l = t.hibounds; l.nonEmpty(); l = l.tail) {
		if (!isSubType(t.inst, l.head)) {
		    result = false;
		    return;
		}
	    }
	    result = true;
	    return;
	}
	public void visitErrorType(ErrorType t) {
	    result = true;
	    return;
	}
    }

    /** A wrapper for a type that allows use in sets. */
    class SingletonType {
        final Type t;
        SingletonType(Type t) {
            this.t = t;
        }
        public int hashCode() {
            return Types.this.hashCode(t);
        }
        public boolean equals(Object other) {
            return (other instanceof SingletonType) &&
                isSameType(t, ((SingletonType)other).t);
        }
        public String toString() {
            return t.toString();
        }
    }

    /************************************************************
     * Type.Mapping fromUnknownFun
     ************************************************************/

    /** A mapping that turns all unknown types in this type
     *  to fresh unknown variables.
     */
    public Type.Mapping fromUnknownFun = new Type.Mapping () {
	    public String toString() { return "fromUnknownFun"; }
	    public Type apply(Type t) {
		if (t.tag == UNKNOWN) return new UndetVar(t);
		else return t.map(this);
	    }
	};

    /************************************************************
     * boolean containedBy(Type t, Type other)
     ************************************************************/

    public boolean containedBy(Type t, Type other) {
	switch (t.tag) {
	case UNDETVAR:
	    if (other.tag != TYPEARG) return isSameType(t, other);
	    // other is a wildcard.  This situation should only happen
	    // when we can't infer a type for t based on the actual
	    // arguments, e.g. when a type variable is only used in
	    // the return type.  Normally, capture conversion should
	    // make sure that we do not end up here.
	    // FIXME: this should be moved to attribExpr in attr.
	    UndetVar undetvar = (UndetVar)t;
	    assert undetvar.inst == null;
	    assert undetvar.lobounds.isEmpty();
	    System.err.println("hibounds=" + undetvar.hibounds);
	    // assert undetvar.hibounds.isEmpty();
	    undetvar.inst = other;
	    return true;
	case ERROR:
	    return true;
	default:
	    return containsTypeFcn.containsType(other, t);
	}
    }

    /************************************************************
     * boolean containsType(Type t, Type other)
     ************************************************************/

    boolean containsType(List<Type> these, List<Type> those) {
	while (these.nonEmpty() && those.nonEmpty()
	       && containsType(these.head, those.head)) {
	    these = these.tail;
	    those = those.tail;
	}
	return these.isEmpty() && those.isEmpty();
    }

    /** Check if Type t contains other.
     *
     * T contains S if:
     *
     * L(T) <: L(S) && U(S) <: U(T)
     *
     * This relation is only used by ClassType.isSubType(), i.e.
     *
     * C<S> <: C<T> if T contains S.
     *
     * Because of F-bounds, this relation can lead to infinite
     * recursion.  Thus we must somehow break that recursion.
     * Notice that containsType() is only called from
     * ClassType.isSubType().  Since the arguments has already
     * been checked against their bounds, we know:
     *
     * U(S) <: U(T) if T is "super" bound (U(T) *is* the bound)
     *
     * L(T) <: L(S) if T is "extends" bound (L(T) is bottom)
     *
     * @param other Type to check if contained in t.
     */
    public boolean containsType(Type t, Type other) {
	return containsTypeFcn.containsType(t, other);
    }
    private ContainsTypeFcn containsTypeFcn = new ContainsTypeFcn();
    private class ContainsTypeFcn extends Visitor {
	Type other;
	boolean result;
	boolean containsType(Type t, Type other) {
	    Type oldOther = this.other;
	    this.other = other;
	    try {
		t.accept(this);
		return result;
	    } finally {
		this.other = oldOther;
	    }
	}
	public void visitType(Type t) {
	    if (other.tag >= firstPartialTag) {
		result = containedBy(other, t);
		return;
	    }
	    result = isSameType(t, other);
	    return;
	}
	public void visitErrorType(ErrorType t) {
	    result = true;
	    return;
	}
	public void visitArgumentType(ArgumentType t) {
	    if (other.tag >= firstPartialTag) {
		result = containedBy(other, t);
		return;
	    }
	    result =
		(t.isExtendsBound() || isSubType(t.type, lowerBound(other))) &&
		(t.isSuperBound() || isSubType(upperBound(other), t.type));
	    return;
	}

	public void visitUndetVar(UndetVar t) {
	    if (other.tag != TYPEARG)
		result = isSameType(t, other);
	    else
		result = false;
	    return;
	}

    }

    /************************************************************
     * boolean containsTypeEquivalent(List<Type> these, List<Type> those)
     ************************************************************/

    public boolean containsTypeEquivalent(List<Type> these, List<Type> those) {
	while (these.nonEmpty() && those.nonEmpty()
	       && containsTypeEquivalent(these.head, those.head)) {
	    these = these.tail;
	    those = those.tail;
	}
	return these.isEmpty() && those.isEmpty();
    }

    /************************************************************
     * boolean isCastable(Type t, Type that, Warner warn)
     * boolean isCastable(Type t, Type that)
     ************************************************************/

    /** Is Type t is castable to that?
     *  that is assumed to be an erased type.
     *  (not defined for Method and ForAll types).
     */
    public boolean isCastable(Type t, Type that, Warner warn) {
	return t==that || isCastableFcn.isCastable(t, that, warn);
    }
    public boolean isCastable(Type t, Type that) {
	return t==that || isCastableFcn.isCastable(t, that, Warner.noWarnings);
    }
    private IsCastableFcn isCastableFcn = new IsCastableFcn();
    private class IsCastableFcn extends Visitor {
	Type that;
	Warner warn;
	boolean result;
	public boolean isCastable(Type t, Type that, Warner warn) {
	    if (t.isPrimitive() != that.isPrimitive())
		return allowBoxing && isConvertible(t, that, warn);
	    return isCastableNoBoxing(t, that, warn);
	}
	public boolean isCastableNoBoxing(Type t, Type that, Warner warn) {
	    Type oldThat = this.that;
	    Warner oldWarn = this.warn;
	    try {
		this.that = that;
		this.warn = warn;
		t.accept(this);
		return result;
	    } finally {
		this.that = oldThat;
		this.warn = oldWarn;
	    }
	}
	public void visitType(Type t) {
	    if (that.tag == ERROR) {
		result = true;
		return;
	    }
	    switch (t.tag) {
	    case BYTE: case CHAR: case SHORT: case INT: case LONG: case FLOAT:
	    case DOUBLE:
		result = that.tag <= DOUBLE;
		return;
	    case BOOLEAN:
		result = that.tag == BOOLEAN;
		return;
	    case VOID:
		result = false;
		return;
	    case BOT:
		result = isSubType(t, that);
		return;
	    default:
		throw new AssertionError();
	    }
	}
	public void visitArgumentType(ArgumentType t) {
	    result = isCastable(upperBound(t), that, warn);
	    return;
	}
	/** The rules for castability are extended to parameterized types
	 *  as follows:
	 *  (1) One may always cast to a supertype
	 *  (2) One may cast to a subtype C<...> provided there is only
	 *      one type with the subtype's class part, C, that is a subtype
	 *      of this type. (This is equivalent to: C<...> = this.asSub(C).
	 *  (3) One may cast an interface to an unparameterized class.
	 *  (4) One may cast a non-final class to an unparameterized interface.
	 */
	public void visitClassType(ClassType t) {
            if (that.tag == ERROR || that.tag == BOT) {
		result = true;
		return;
	    }
            if (that.tag == TYPEVAR) {
		boolean _result = isCastable(that.bound(), t, Warner.noWarnings);
                if (_result)
                    warn.warnUnchecked();
		result = _result;
                return;
            }
	    if ((t.tsym.flags() & COMPOUND) != 0) {
		supertype(t).accept(this);
		if (!result) return;
		for (Type i : interfaces(t)) {
		    i.accept(this);
		    if (!result) return;
		}
		result = true;
		return;
	    }
	    if (that.tag == CLASS && (that.tsym.flags() & COMPOUND) != 0) {
		result = isCastable(that, t, warn);
		return;
	    }
            if (that.tag == CLASS || that.tag == ARRAY) {
                // Upcast
                if (isSubType(erasure(t), erasure(that))) {
                    if (that.isRaw()) {
                        result = true;
			return;
                    } else if (t.isRaw()) {
                        if (!isUnbounded(that))
			    warn.warnUnchecked();
                        result = true;
			return;
                    } else {
                        Type thatSub = asSub(that, t.tsym);
                        if (thatSub == null) {
                            // fall through to side cast
                        } else {
                            if (t.tsym != thatSub.tsym)
                                // Sanity check
                                throw new AssertionError();
                            boolean _result = !disjointTypes(t.typarams(), thatSub.typarams());
                            if (_result && giveWarning(t, thatSub))
				warn.warnUnchecked();
			    result = _result;
			    return;
                        }
                    }
                // Downcast
                } else if (isSubType(erasure(that), erasure(t))) {
                    if (that.tag == ARRAY) {
                        if (!isReifiable(that))
                            warn.warnUnchecked();
                        result = true;
			return;
                    } else if (that.isRaw()) {
                        result = true;
			return;
                    } else if (t.isRaw()) {
                        if (!isUnbounded(that))
			    warn.warnUnchecked();
                        result = true;
			return;
                    } else {
                        Type thisSub = asSub(t, that.tsym);
                        if (thisSub == null) {
                            // fall through to side cast
                        } else {
                            if (thisSub.tsym != that.tsym)
                                // Sanity check
                                throw new AssertionError();
                            boolean _result = !disjointTypes(thisSub.typarams(), that.typarams());
                            if (_result && giveWarning(thisSub, that))
				warn.warnUnchecked();
			    result = _result;
			    return;
                        }
                    }
                }
                // Sidecast
                if (that.tag == CLASS) {
                    if ((that.tsym.flags() & INTERFACE) != 0) {
                        result = ((t.tsym.flags() & FINAL) == 0)
                            ? sideCast(t, that, warn)
                            : sideCastFinal(t, that, warn);
                        return;
                    } else if ((t.tsym.flags() & INTERFACE) != 0) {
                        result = ((that.tsym.flags() & FINAL) == 0)
                            ? sideCast(t, that, warn)
                            : sideCastFinal(t, that, warn);
                        return;
                    } else {
                        // unrelated class types
                        result = false;
                        return;
                    }
		}
	    }
	    result = false;
	    return;
	}
	public void visitArrayType(ArrayType t) {
	    switch (that.tag) {
	    case ERROR:
	    case BOT:
		result = true;
		return;
	    case TYPEVAR:
		boolean _result = isCastable(that, t, Warner.noWarnings);
                if (_result)
                    warn.warnUnchecked();
		result = _result;
                return;
            case CLASS:
		result = isSubType(t, that);
		return;
            case ARRAY:
                if (elemtype(t).tag <= lastBaseTag) {
		    result = elemtype(t).tag == elemtype(that).tag;
		    return;
                } else {
		    result = isCastableNoBoxing(elemtype(t),
                                                elemtype(that),
                                                warn);
		    return;
                }
	    default:
		result = false;
		return;
            }
	}
	public void visitTypeVar(TypeVar t) {
	    switch (that.tag) {
	    case ERROR:
	    case BOT:
		result = true;
		return;
	    case TYPEVAR:
                boolean _result = isCastable(t.bound, that, Warner.noWarnings);
                if (_result)
                    warn.warnUnchecked();
		result = _result;
                return;
	    default:
		result = isCastable(t.bound, that, warn);
		return;
	    }
	}
	public void visitErrorType(ErrorType t) {
	    result = true;
	    return;
	}
    }

    /************************************************************
     * boolean disjointTypes(List<Type> these, List<Type> those)
     ************************************************************/

    public boolean disjointTypes(List<Type> these, List<Type> those) {
        while (these.tail != null && those.tail != null) {
            if (disjointType(these.head, those.head)) return true;
            these = these.tail;
            those = those.tail;
        }
        return false;
    }

    /************************************************************
     * boolean disjointTypes(Type t, Type that)
     ************************************************************/

    /**
     * Two types or wildcards are considered disjoint if it can be
     * proven that no type can be contained in both. It is
     * conservative in that it is allowed to say that two types are
     * not disjoint, even though they actually are.
     *
     * The type C<X> is castable to C<Y> exactly if X and Y are not
     * disjoint.
     */
    public boolean disjointType(Type t, Type that) {
	return disjointTypeFcn.disjointType(t, that);
    }
    private DisjointTypeFcn disjointTypeFcn = new DisjointTypeFcn();
    private class DisjointTypeFcn extends Visitor {
	Type that;
	boolean result;
	public boolean disjointType(Type t, Type that) {
	    Type oldThat = this.that;
	    try {
		this.that = that;
		t.accept(this);
		return this.result;
	    } finally {
		this.that = oldThat;
	    }
	}
        Set<TypePair> cache = new HashSet<TypePair>();
	public void visitTypeVar(TypeVar t) {
            TypePair pair = new TypePair(t, that);
            if (cache.add(pair)) {
                this.visitType(t);
                boolean r = result;
                cache.remove(pair);
                result = r;
            } else {
                result = false;
            }
            this.result = result;
            return;
	}
	public void visitType(Type t) {
	    if (that.tag == TYPEARG) {
		result = disjointType(that, t);
		return;
	    } else {
		result =
		    notSoftSubtype(t, that) ||
		    notSoftSubtype(that, t);
		return;
	    }
	}
	public void visitArgumentType(ArgumentType t) {
            if (t.isUnbound()) {
		result = false;
		return;
	    }
            if (that.tag != TYPEARG) {
                if (t.isExtendsBound()) {
                    result = notSoftSubtype(that, t.type);
		    return;
		} else { // isSuperBound()
                    result = notSoftSubtype(t.type, that);
		    return;
		}
            } else {
                if (that.isUnbound()) {
		    result = false;
		    return;
		}
                if (t.isExtendsBound()) {
                    if (that.isExtendsBound()) {
			result = !isCastable(t.type, upperBound(that));
			return;
                    } else if (that.isSuperBound()) {
			result = notSoftSubtype(lowerBound(that), t.type);
			return;
                    }
                } else if (t.isSuperBound()) {
                    if (that.isExtendsBound()) {
			result = notSoftSubtype(t.type, upperBound(that));
			return;
		    }
                }
            }
	    result = false;
	    return;
	}
    }

    /************************************************************
     * List<Type> lowerBoundArgtypes(Type t)
     ************************************************************/

    /** Returns the lower bounds of the formals of a method. */
    public List<Type> lowerBoundArgtypes(Type t) {
	return map(t.argtypes(), lowerBoundMapping);
    }
    private final Mapping lowerBoundMapping =
	new Mapping() {
		public Type apply(Type that) {
                    return lowerBound(that);
                }
            };

    /************************************************************
     * boolean notSoftSubtype(Type t, Type that)
     ************************************************************/

    /** This relation answers the question: is impossible that
     * something of type `t' can be a subtype of `that'? This is
     * different from the question "is `t' not a subtype of `that'?"
     * when type variables are involved: Integer is not a subtype of T
     * where <T extends Number> but it is not true that Integer cannot
     * possibly be a subtype of T.
     */
    public boolean notSoftSubtype(Type t, Type that) {
	if (t == that) return false;
	if (t.tag == TYPEVAR) {
	    TypeVar self = (TypeVar) t;
	    if (that.tag == TYPEVAR)
		that = that.bound();
	    return !isCastable(self.bound,
			       that,
			       Warner.noWarnings);
	}
        if (that.tag != TYPEARG)
	    that = upperBound(that);
	if (that.tag == TYPEVAR)
	    that = that.bound();
	return !isSubType(t, that);
    }

    /************************************************************
     * boolean isReifiable(Type t)
     ************************************************************/

    public boolean isReifiable(Type t) {
	return isReifiableFcn.isReifiable(t);
    }
    private IsReifiableFcn isReifiableFcn = new IsReifiableFcn();
    private class IsReifiableFcn extends Visitor {
	boolean result;
	public boolean isReifiable(Type t) {
	    t.accept(this);
	    return result;
	}
	public void visitType(Type t) {
	    result = true;
	    return;
	}
	public void visitClassType(ClassType t) {
	    if (!t.isParameterized()) {
		result = true;
		return;
	    }
	    for (Type param : t.allparams()) {
		if (!param.isUnbound()) {
		    result = false;
		    return;
		}
	    }
	    result = true;
	    return;
	}
	public void visitArrayType(ArrayType t) {
            t.elemtype.accept(this);
	}
	public void visitTypeVar(TypeVar t) {
	    result = false;
	    return;
	}
    }

    /************************************************************
     * boolean isArray(Type t)
     ************************************************************/

    public boolean isArray(Type t) {
	while (t.tag == TYPEARG)
	    t = upperBound(t);
	return t.tag == ARRAY;
    }

    /************************************************************
     * boolean elemtype(Type t)
     ************************************************************/

    /** The element type of an array. */
    public Type elemtype(Type t) {
	switch (t.tag) {
	case TYPEARG:
	    return elemtype(upperBound(t));
	case ARRAY:
	    return ((ArrayType)t).elemtype;
	case FORALL:
	    return elemtype(((ForAll)t).qtype);
	case ERROR:
	    return t;
	default:
	    return null;
	}
    }

    /** The number of dimensions of an array type. */
    public int dimensions(Type t) {
	int result = 0;
	while (t.tag == ARRAY) {
	    result++;
	    t = elemtype(t);
	}
	return result;
    }

    /************************************************************
     * Type asSuper(Type t, Symbol sym)
     ************************************************************/

    /** Return the (most specific) base type of this type that starts
     *  with symbol sym.  If none exists, return null.
     */
    public Type asSuper(Type t, Symbol sym) {
	return asSuperFcn.asSuper(t, sym);
    }
    private AsSuperFcn asSuperFcn = new AsSuperFcn();
    private class AsSuperFcn extends Visitor {
	Symbol sym;
	Type result;
	public Type asSuper(Type t, Symbol sym) {
	    Symbol oldSym = this.sym;
	    try {
		this.sym = sym;
		t.accept(this);
		return result;
	    } finally {
		this.sym = oldSym;
	    }
	}
	public void visitType(Type t) {
	    result = null;
	    return;
	}
	public void visitClassType(ClassType t) {
	    if (t.tsym == sym) {
		result = t;
		return;
	    }
	    Type st = supertype(t);
	    if (st.tag == CLASS || st.tag == ERROR) {
		Type x = asSuper(st, sym);
		if (x != null) {
		    result = x;
		    return;
		}
	    }
	    if ((sym.flags() & INTERFACE) != 0) {
		for (List<Type> l = interfaces(t); l.nonEmpty(); l = l.tail) {
		    Type x = asSuper(l.head, sym);
		    if (x != null) {
			result = x;
			return;
		    }
		}
	    }
	    result = null;
	    return;
	}
	public void visitArrayType(ArrayType t) {
	    result = isSubType(t, sym.type) ? sym.type : null;
	    return;
	}
	public void visitTypeVar(TypeVar t) {
	    result = asSuper(t.bound, sym);
	    return;
	}
	public void visitErrorType(ErrorType t) {
	    result = t;
	    return;
	}
    }

    /************************************************************
     * Type asOuterSuper(Type t, Symbol sym)
     * Type asEnclosingSuper(Type t, Symbol sym)
     ************************************************************/

    /** Return the base type of this type or any
     *  of its outer types that starts with symbol sym.
     *  If none exists, return null.
     */
    public Type asOuterSuper(Type t, Symbol sym) {
        switch (t.tag) {
        case CLASS:
	    do {
		Type s = asSuper(t, sym);
		if (s != null) return s;
		t = t.outer();
	    } while (t.tag == CLASS);
            return null;
        case ARRAY:
            return isSubType(t, sym.type) ? sym.type : null;
        case TYPEVAR:
            return asSuper(t, sym);
        case ERROR:
            return t;
        default:
            return null;
        }
    }

    /** Return the base type of this type or any
     *  of its enclosing types that starts with symbol sym.
     *  If none exists, return null.
     */
    public Type asEnclosingSuper(Type t, Symbol sym) {
        switch (t.tag) {
        case CLASS:
	    do {
		Type s = asSuper(t, sym);
		if (s != null) return s;
                Type outer = t.outer();
                t = (outer.tag == CLASS) ? outer :
                    (t.tsym.owner.enclClass() != null) ? t.tsym.owner.enclClass().type :
                    Type.noType;
	    } while (t.tag == CLASS);
            return null;
        case ARRAY:
            return isSubType(t, sym.type) ? sym.type : null;
        case TYPEVAR:
            return asSuper(t, sym);
        case ERROR:
            return t;
        default:
            return null;
        }
    }

    /************************************************************
     * Type memberType(Type t, Symbol sym)
     ************************************************************/

    /** The type of given symbol, seen as a member of this type
     */
    public Type memberType(Type t, Symbol sym) {
	return (sym.flags() & STATIC) != 0
	    ? sym.type
	    : memberTypeFcn.memberType(t, sym);
    }
    private MemberTypeFcn memberTypeFcn = new MemberTypeFcn();
    private class MemberTypeFcn extends Visitor {
	Symbol sym;
	Type result;
	public Type memberType(Type t, Symbol sym) {
	    Symbol oldSym = this.sym;
	    try {
		this.sym = sym;
		t.accept(this);
		return result;
	    } finally {
		this.sym = oldSym;
	    }
	}
	public void visitType(Type t) {
	    result = sym.type;
	    return;
	}
	public void visitArgumentType(ArgumentType t) {
	    result = memberType(upperBound(t), sym);
	    return;
	}
	public void visitClassType(ClassType t) {
	    Symbol owner = sym.owner;
	    long flags = sym.flags();
	    if (((flags & STATIC) == 0) && owner.type.isParameterized()) {
		Type base = asOuterSuper(t, owner);
		if (base != null) {
		    List<Type> ownerParams = owner.type.allparams();
		    List<Type> baseParams = base.allparams();
		    if (ownerParams.nonEmpty()) {
			if (baseParams.isEmpty()) {
			    // then base is a raw type
			    result = erasure(sym.type);
			    return;
			} else {
			    result = subst(sym.type, ownerParams, baseParams);
			    return;
			}
		    }
		}
	    }
	    result = sym.type;
	    return;
	}
	public void visitTypeVar(TypeVar t) {
	    result = memberType(t.bound, sym);
	    return;
	}
	public void visitErrorType(ErrorType t) {
	    result = t;
	    return;
	}
    }

    /************************************************************
     * boolean isAssignable(Type self, Type that, Warner warn)
     * boolean isAssignable(Type self, Type that)
     ************************************************************/

    public boolean isAssignable(Type self, Type that) {
	return isAssignable(self, that, Warner.noWarnings);
    }
    /** Is "self" assignable to "that"? Equivalent to subtype
     *  except for constant values and raw types. (not defined for
     *  Method and ForAll types)
     */
    public boolean isAssignable(Type self, Type that, Warner warn) {
	if (self.tag == ERROR)
	    return true;
	if (self.tag <= INT && self.constValue != null) {
	    int value = ((Number)self.constValue).intValue();
	    switch (that.tag) {
	    case BYTE:
		if (Byte.MIN_VALUE <= value && value <= Byte.MAX_VALUE)
		    return true;
		break;
	    case CHAR:
		if (Character.MIN_VALUE <= value && value <= Character.MAX_VALUE)
		    return true;
		break;
	    case SHORT:
		if (Short.MIN_VALUE <= value && value <= Short.MAX_VALUE)
		    return true;
		break;
	    case INT:
		return true;
            case CLASS:
                switch (unboxedType(that).tag) {
                case BYTE:
                case CHAR:
                case SHORT:
                    return isAssignable(self, unboxedType(that), warn);
                }
                break;
	    }
	}
	return isConvertible(self, that, warn);
    }

    /************************************************************
     * Type erasure(Type t)
     ************************************************************/

    /** The erasure of this type -- the type that results when
     *  all type parameters in this type are deleted.
     */
    public Type erasure(Type t) {
	if (t.tag <= lastBaseTag) return t; /*fast special case*/
	t.accept(erasureFcn);
	return erasureFcn.result;
    }
    private ErasureFcn erasureFcn = new ErasureFcn();
    private class ErasureFcn extends Visitor {
	Type result;
	Type erasure(Type t) {
	    t.accept(this);
	    return result;
	}
	public void visitType(Type t) {
	    if (t.tag <= lastBaseTag) {
		result = t; /*fast special case*/
		return;
	    }
	    result = t.map(erasureFun);
	    return;
	}
	public void visitArgumentType(ArgumentType t) {
	    result = erasure(upperBound(t));
	    return;
	}
	public void visitClassType(ClassType t) {
	    result = t.tsym.erasure(Types.this);
	    return;
	}
	public void visitTypeVar(TypeVar t) {
	    result = erasure(t.bound);
	    return;
	}
	public void visitErrorType(ErrorType t) {
	    result = t;
	    return;
	}
    }
    private Mapping erasureFun = new Mapping () {
	    public Type apply(Type t) { return erasure(t); }
	};

    /************************************************************
     * List<Type> erasure(List<Type> these)
     ************************************************************/

    public List<Type> erasure(List<Type> these) {
	return Type.map(these, erasureFun);
    }

    /************************************************************
     * Type makeCompoundType(List<Type> bounds, Type supertype)
     * Type makeCompoundType(List<Type> bounds)
     * Type makeCompoundType(Type bound1, Type bound2)
     ************************************************************/

    /** make a compound type from non-empty list of types
     * @param bounds            the types from which the compound type is formed
     * @param supertype         is objectType if all bounds are interfaces,
     *                          null otherwise.
     */
    public Type makeCompoundType(List<Type> bounds,
				 Type supertype) {
	ClassSymbol bc =
            new ClassSymbol(ABSTRACT|PUBLIC|SYNTHETIC|COMPOUND|ACYCLIC,
                            Type.moreInfo
                                ? names.fromString(bounds.toString())
                                : names.empty,
                            syms.noSymbol);
	bc.erasure_field = erasure(bounds.head);
	bc.members_field = new Scope(bc);
	ClassType bt = (ClassType)bc.type;
	bt.allparams_field = Type.emptyList;
	if (supertype != null) {
	    bt.supertype_field = supertype;
	    bt.interfaces_field = bounds;
	} else {
	    bt.supertype_field = bounds.head;
	    bt.interfaces_field = bounds.tail;
	}
	return bt;
    }

    /** Same as previous function, except that third parameter is
     *  computed directly. Note that this test might cause a symbol completion.
     *  Hence, this version of makeCompoundType may not be called during
     *  a classfile read.
     */
    public Type makeCompoundType(List<Type> bounds) {
	Type supertype = (bounds.head.tsym.flags() & INTERFACE) != 0 ?
	    supertype(bounds.head) : null;
	return makeCompoundType(bounds, supertype);
    }

    /** Same as previous function, except that third parameter is
     *  computed directly. Note that this test might cause a symbol completion.
     *  Hence, this version of makeCompoundType may not be called during
     *  a classfile read.
     */
    public Type makeCompoundType(Type bound1, Type bound2) {
        return makeCompoundType(Type.emptyList.prepend(bound2).prepend(bound1));
    }

    /************************************************************
     * Type supertype(Type t)
     ************************************************************/

    public Type supertype(Type t) {
	t.accept(supertypeFcn);
	return supertypeFcn.result;
    }
    private SupertypeFcn supertypeFcn = new SupertypeFcn();
    private class SupertypeFcn extends Visitor {
	Type result;
	// FIXME(VFORCE): we may not know how to deliver "the" supertype of this type
	public void visitType(Type t) {
	    result = null;
	    return;
	}
	public void visitClassType(ClassType t) {
	    if (t.supertype_field == null) {
		t.complete();
		Type st = ((ClassType)t.tsym.type).supertype_field;
		if (st == null) {
		    t.supertype_field = noType;
		} else if (t == t.tsym.type) {
		    t.supertype_field = st; // NOOP!!!
		} else {
		    List<Type> ownparams = classBound(t).allparams();
		    List<Type> symparams = t.tsym.type.allparams();
		    t.supertype_field =
			(ownparams.isEmpty() && symparams.nonEmpty())
			? erasure(st)
			: subst(st, symparams, ownparams);
		}
	    }
	    result = t.supertype_field;
	    return;
	}
	/** The supertype is always
	 *  a class type. If the type variable's bounds start with a
	 *  class type, this is also the supertype.
	 *  Otherwise, the supertype is java.lang.Object.
	 */
	public void visitTypeVar(TypeVar t) {
	    if (t.bound.tag == TYPEVAR ||
		(t.bound.tsym.flags() & (INTERFACE | COMPOUND)) == 0) {
		result = t.bound;
		return;
	    } else {
		result = supertype(t.bound);
		return;
	    }
	}
	public void visitErrorType(ErrorType t) {
	    result = t;
	    return;
	}
    }

    /************************************************************
     * List<Type> interfaces(Type t)
     ************************************************************/

    /** Return the interfaces implemented by this class. */
    public List<Type> interfaces(Type t) {
	t.accept(interfacesFcn);
	return interfacesFcn.result;
    }
    private InterfacesFcn interfacesFcn = new InterfacesFcn();
    private class InterfacesFcn extends Visitor {
	List<Type> result;
	public void visitType(Type t) {
	    result = Type.emptyList;
	    return;
	}
	public void visitClassType(ClassType t) {
	    if (t.interfaces_field == null) {
		t.complete();
		List<Type> is = ((ClassType)t.tsym.type).interfaces_field;
		if (is == null) {
		    t.interfaces_field = Type.emptyList;
		} else if (t == t.tsym.type) {
		    t.interfaces_field = is; // NOOP?!
		} else {
		    List<Type> ownparams = t.allparams();
		    List<Type> symparams = t.tsym.type.allparams();
		    t.interfaces_field =
			(ownparams.isEmpty() && symparams.nonEmpty())
			? erasure(is)
			: upperBounds(subst(is, symparams, ownparams));
		}
	    }
	    result = t.interfaces_field;
	    return;
	}
	public void visitTypeVar(TypeVar t) {
	    long flags = t.bound.tsym.flags();
	    if ((flags & INTERFACE) != 0) {
		result = emptyList.prepend(t.bound);
		return;
	    }
	    if ((flags & COMPOUND) != 0) {
		result = interfaces(t.bound);
		return;
	    }
	    result = emptyList;
	    return;
	}
    }

    /************************************************************
     * boolean isDerivedRaw(Type t)
     ************************************************************/

    Map<Type,Boolean> isDerivedRawCache = new HashMap<Type,Boolean>();

    public boolean isDerivedRaw(Type t) {
	Boolean result = isDerivedRawCache.get(t);
	if (result == null) {
	    result = isDerivedRawInternal(t);
	    isDerivedRawCache.put(t, result);
	}
	return result;
    }

    public boolean isDerivedRawInternal(Type t) {
	if (t.isErroneous())
	    return false;
	return
	    t.isRaw() ||
	    supertype(t) != null && isDerivedRaw(supertype(t)) ||
	    isDerivedRaw(interfaces(t));
    }

    /************************************************************
     * boolean isDerivedRaw(List<Type> ts)
     ************************************************************/

    public boolean isDerivedRaw(List<Type> ts) {
	List<Type> l = ts;
	while (l.nonEmpty() && !isDerivedRaw(l.head)) l = l.tail;
	return l.nonEmpty();
    }

    /************************************************************
     * void setBounds(TypeVar self, List<Type> bounds, Type supertype)
     ************************************************************/

    /** Set a TypeVar's bounds field to reflect a (possibly multiple) list of bounds
     * @param bounds            the bounds, must be nonempty
     * @param supertype         is objectType if all bounds are interfaces,
     *                          null otherwise.
     */
    public void setBounds(TypeVar self, List<Type> bounds, Type supertype) {
	if (bounds.tail.isEmpty())
	    self.bound = bounds.head;
	else
	    self.bound = makeCompoundType(bounds, supertype);
	self.rank_field = -1;
    }

    /************************************************************
     * void setBounds(TypeVar self, List<Type> bounds)
     ************************************************************/

    /** Same as previous function, except that second parameter is
     *  computed directly. Note that this test might cause a symbol completion.
     *  Hence, this version of setBounds may not be called during
     *  a classfile read.
     */
    public void setBounds(TypeVar self, List<Type> bounds) {
	Type supertype = (bounds.head.tsym.flags() & INTERFACE) != 0 ?
	    supertype(bounds.head) : null;
	setBounds(self, bounds, supertype);
	self.rank_field = -1;
    }

    /************************************************************
     * List<Type> getBounds(TypeVar self)
     ************************************************************/

    /** Return list of bounds of this type variable.
     */
    public List<Type> getBounds(TypeVar self) {
	if ((self.bound.tsym.flags() & COMPOUND) == 0)
	    return emptyList.prepend(self.bound);
	else if ((erasure(self).tsym.flags() & INTERFACE) == 0)
	    return interfaces(self).prepend(supertype(self));
	else
	    // No superclass was given in bounds.
	    // In this case, supertype is Object, erasure is first interface.
	    return interfaces(self);
    }

    /************************************************************
     * Type classBound(Type self)
     ************************************************************/

    /** If this type is a (possibly selected) type variable,
     *  return the bounding class of this type, otherwise
     *  return the type itself
     */
    public Type classBound(Type self) {
	self.accept(classBoundFcn);
	return classBoundFcn.result;
    }
    private ClassBoundFcn classBoundFcn = new ClassBoundFcn();
    private class ClassBoundFcn extends Visitor {
	Type result;
	public void visitType(Type t) {
	    result = t;
	    return;
	}
	public void visitClassType(ClassType t) {
	    Type outer1 = classBound(t.outer());
	    if (outer1 != t.outer_field) {
		result = new ClassType(outer1, t.typarams(), t.tsym);
		return;
	    } else {
		result = t;
		return;
	    }
	}
	public void visitTypeVar(TypeVar t) {
	    result = classBound(supertype(t));
	    return;
	}
	public void visitErrorType(ErrorType t) {
	    result = t;
	    return;
	}
    }

    /************************************************************
     * boolean hasSameArgs(Type self, Type that)
     * boolean isSubSignature(Type s, Type t)
     * boolean overrideEquivalent(Type s, Type t)
     ************************************************************/

    /** Does this type have the same arguments as that type?
     *  It is assumed that both types are (possibly polymorphic) method types.
     *  Monomorphic method types "have the same arguments",
     *  if their argument lists are equal.
     *  Polymorphic method types "have the same arguments",
     *  if they have the same arguments after renaming all type variables
     *  of one to corresponding type variables in the other, where
     *  correspondence is by position in the type parameter list.
     */
    public boolean hasSameArgs(Type self, Type that) {
	return hasSameArgsFcn.visit(self, that);
    }
    /** Returns true iff the first signature is a <em>sub
     *  signature</em> of the other.  This is <b>not</b> an
     *  equivalence relation.
     *
     * @see "The Java Language Specification, Third Ed. (8.4.2)."
     * @see #overrideEquivalent(Type s, Type t)
     * @param s first signature (possibly raw).
     * @param t second signature (could be subjected to erasure).
     * @return true if s is a sub signature of t.
     */
    public boolean isSubSignature(Type s, Type t) {
	return hasSameArgs(s, t) || hasSameArgs(s, erasure(t));
    }
    /** Returns true iff these signatures are related by <em>override
     *  equivalence</em>.  This is the natural extension of
     *  isSubSignature to an equivalence relation.
     *
     * @see "The Java Language Specification, Third Ed. (8.4.2)."
     * @see #isSubSignature(Type s, Type t)
     * @param s a signature (possible raw, could be subjected to
     * erasure).
     * @param t a signature (possible raw, could be subjected to
     * erasure).
     * @return true if either argument is a sub signature of the other.
     */
    public boolean overrideEquivalent(Type s, Type t) {
	return hasSameArgs(s, t) ||
	    hasSameArgs(s, erasure(t)) || hasSameArgs(erasure(s), t);
    }
    private HasSameArgsFcn hasSameArgsFcn = new HasSameArgsFcn();
    private class HasSameArgsFcn extends Visitor {
	Type that;
	boolean result;
	final boolean visit(Type self, Type that) {
	    Type oldThat = this.that;
	    try {
		this.that = that;
		self.accept(this);
		return result;
	    } finally {
		this.that = oldThat;
	    }
	}
	public void visitType(Type t) {
	    throw new AssertionError();
	}
	public void visitMethodType(MethodType t) {
	    result =
		that.tag == METHOD &&
		containsTypeEquivalent(t.argtypes, that.argtypes());
	    return;
	}
	public void visitForAll(ForAll t) {
	    result =
		that.tag == FORALL &&
		hasSameBounds(t, (ForAll)that) &&
		visit(t.qtype,
		      subst(((ForAll)that).qtype,
			    ((ForAll)that).tvars,
			    t.tvars));
	    return;
	}
	public void visitErrorType(ErrorType t) {
	    result = false;
	    return;
	}
    }

    /************************************************************
     * List<Type> subst(List<Type> these, List<Type> from, List<Type> to)
     ************************************************************/

    public List<Type> subst(List<Type> that,
			    List<Type> from,
			    List<Type> to) {
	return new SubstFcn(from, to).subst(that);
    }

    /************************************************************
     * Type subst(Type self, List<Type> from, List<Type> to)
     ************************************************************/

    /** Substitute all occurrences of a type in `from' with the
     *  corresponding type in `to' in 'self'. Match lists `from' and `to' from the right:
     *  If lists have different length, discard leading elements of the longer list.
     */
    public Type subst(Type self, List<Type> from, List<Type> to) {
	return new SubstFcn(from, to).subst(self);
    }

    private class SubstFcn extends Visitor {
	List<Type> from;
	List<Type> to;
	Type result;

	Type subst(Type that) {
	    if (from.tail == null)
		return that;
            that.accept(this);
            return result;
        }

	List<Type> subst(List<Type> these) {
	    if (from.tail == null)
		return these;
            boolean wild = false;
            if (these.nonEmpty() && from.nonEmpty()) {
                Type head1 = subst(these.head);
                List<Type> tail1 = subst(these.tail);
                if (head1 != these.head || tail1 != these.tail)
                    return tail1.prepend(head1);
            }
            return these;
        }

        public SubstFcn(List<Type> from, List<Type> to) {
	    int fromLength = from.length();
	    int toLength = to.length();
	    while (fromLength > toLength) {
		fromLength--;
		from = from.tail;
	    }
	    while (fromLength < toLength) {
		toLength--;
		to = to.tail;
	    }
            this.from = from;
            this.to = to;
        }

	public void visitMethodType(MethodType t) {
            List<Type> argtypes = subst(t.argtypes);
            Type restype = subst(t.restype);
            List<Type> thrown = subst(t.thrown);
            if (argtypes == t.argtypes &&
                restype == t.restype &&
                thrown == t.thrown)
		result = t;
            else
		result = new MethodType(argtypes, restype, thrown, t.tsym);
	}

	public void visitTypeVar(TypeVar self) {
            for (List<Type> from = this.from, to = this.to;
                 from.nonEmpty();
                 from = from.tail, to = to.tail) {
                if (self == from.head) {
                    result = to.head.withTypeVar(self);
                    return;
                }
            }
            result = self;
	}

	public void visitClassType(ClassType t) {
	    t.complete();
            if ((t.tsym.flags() & Flags.COMPOUND) == 0) {
                Type outer = t.outer();
                List<Type> typarams = t.typarams();
                List<Type> typarams1 = subst(typarams);
                Type outer1 = subst(outer);
                if (typarams1 == typarams && outer1 == outer) result = t;
                else result = new ClassType(outer1, typarams1, t.tsym);
            } else {
                Type st = subst(supertype(t));
                List<Type> is = upperBounds(subst(interfaces(t)));
                if (st == supertype(t) && is == interfaces(t))
		    result = t;
                else
		    result = makeCompoundType(is.prepend(st));
            }
	}

        public void visitArgumentType(ArgumentType that) {
            Type t = that.type;
            if (that.kind != BoundKind.UNBOUND)
		t = subst(t);
	    if (t == that.type)
		result = that;
	    else
		result = new ArgumentType(t, that.kind, syms.boundClass, that.bound);
        }

	public void visitArrayType(ArrayType t) {
            Type elemtype = subst(t.elemtype);
            if (elemtype == t.elemtype)
                result = t;
            else
                result = new ArrayType(upperBound(result), t.tsym);
	}

	public void visitForAll(ForAll t) {
	    List<Type> tvars1 = substBounds(t.tvars, from, to);
	    Type qtype1 = subst(t.qtype);
	    if (tvars1 == t.tvars && qtype1 == t.qtype) {
		result = t;
		return;
	    } else if (tvars1 == t.tvars) {
		result = new ForAll(tvars1, qtype1);
		return;
	    } else {
		result = new ForAll(tvars1, Types.this.subst(qtype1, t.tvars, tvars1));
		return;
	    }
	}

	public void visitType(Type t) {
	    result = t;
	    return;
	}

	public void visitErrorType(ErrorType t) {
	    result = t;
	    return;
	}
    }

    /************************************************************
     * List<Type> substBounds(List<Type> tvars, final List<Type> from, final List<Type> to)
     ************************************************************/

    public List<Type> substBounds(List<Type> tvars,
				  final List<Type> from,
				  final List<Type> to) {
	return map(tvars, new Mapping () {
		public Type apply(Type t) {
		    return substBound(((TypeVar)t), from, to);
		}
	    });
    }

    /************************************************************
     * TypeVar substBound(TypeVar t, List<Type> from, List<Type> to)
     ************************************************************/

    public TypeVar substBound(TypeVar t, List<Type> from, List<Type> to) {
	Type bound1 = subst(t.bound, from, to);
	if (bound1 == t.bound)
	    return t;
	else
	    return new TypeVar(t.tsym, bound1);
    }

    /** Does this type have the same bounds for quantified variables
     *  as that type?
     */
    boolean hasSameBounds(ForAll self, ForAll that) {
	List<Type> l1 = self.tvars;
	List<Type> l2 = that.tvars;
	while (l1.nonEmpty() && l2.nonEmpty() &&
	       isSameType(l1.head.bound(),
			  subst(l2.head.bound(),
				that.tvars,
				self.tvars))) {
	    l1 = l1.tail;
	    l2 = l2.tail;
	}
	return l1.isEmpty() && l2.isEmpty();
    }

    /************************************************************
     * List<Type> newInstances(List<Type> tvars)
     ************************************************************/

    /** Create new vector of type variables from list of variables
     *  changing all recursive bounds from old to new list.
     */
    public List<Type> newInstances(List<Type> tvars) {
	List<Type> tvars1 = Type.map(tvars, newInstanceFun);
	for (List<Type> l = tvars1; l.nonEmpty(); l = l.tail) {
	    TypeVar tv = (TypeVar) l.head;
	    tv.bound = subst(tv.bound, tvars, tvars1);
	}
	return tvars1;
    }
    static private Mapping newInstanceFun = new Mapping() {
	    public Type apply(Type t) { return new TypeVar(t.tsym, t.bound()); }
	};

    /************************************************************
     * int rank(Type t)
     ************************************************************/

    /** The rank of a class is the length of the longest path
     *  between the class and java.lang.Object in the class inheritance
     *  graph. Undefined for all but reference types.
     */
    public int rank(Type t) {
	switch(t.tag) {
	case CLASS: {
	    ClassType self = (ClassType) t;
	    if (self.rank_field < 0) {
		Name fullname = self.tsym.fullName();
		if (fullname == fullname.table.java_lang_Object)
		    self.rank_field = 0;
		else {
		    int r = rank(supertype(self));
		    for (List<Type> l = interfaces(self);
			 l.nonEmpty();
			 l = l.tail) {
			if (rank(l.head) > r)
			    r = rank(l.head);
		    }
		    self.rank_field = r + 1;
		}
	    }
	    return self.rank_field;
	}
	case TYPEVAR: {
	    TypeVar self = (TypeVar) t;
	    if (self.rank_field < 0) {
		int r = rank(supertype(self));
		for (List<Type> l = interfaces(self);
		     l.nonEmpty();
		     l = l.tail) {
		    if (rank(l.head) > r) r = rank(l.head);
		}
		self.rank_field = r + 1;
	    }
	    return self.rank_field;
	}
	case ERROR:
	    return 0;
	default:
	    throw new AssertionError();
	}
    }

    /************************************************************
     * String toString(Type t)
     ************************************************************/

    /** This toString is slightly more descriptive than the one on Type. */
    public String toString(Type t) {
	if (t.tag == FORALL) {
	    ForAll self = (ForAll) t;
	    return typaramsString(self.tvars) + self.qtype;
	}
	return "" + t;
    }
    private String typaramsString(List<Type> tvars) {
	StringBuffer s = new StringBuffer();
	s.append('<');
	boolean first = true;
	for (Type t : tvars) {
	    if (!first) s.append(", ");
	    first = false;
	    appendTyparamString(((TypeVar)t), s);
	}
	s.append('>');
	return s.toString();
    }
    private void appendTyparamString(TypeVar self, StringBuffer buf) {
	buf.append(self);
	if (self.bound == null ||
	    self.bound.tsym.fullName() == names.java_lang_Object)
	    return;
	buf.append(" extends "); // Java syntax; no need for i18n
	Type bound = self.bound;
	if ((bound.tsym.flags() & COMPOUND) == 0) {
	    buf.append(bound);
	} else if ((erasure(self).tsym.flags() & INTERFACE) == 0) {
	    buf.append(supertype(self));
	    for (Type t : interfaces(self)) {
		buf.append('&');
		buf.append(t);
	    }
	} else {
	    // No superclass was given in bounds.
	    // In this case, supertype is Object, erasure is first interface.
	    boolean first = true;
	    for (Type t : interfaces(self)) {
		if (!first) buf.append('&');
		first = false;
		buf.append(t);
	    }
	}
    }

/***************************************************************************
 * Determining least upper bounds of types
 ***************************************************************************/

    /** a cache for closures.
     *  a closure is a list of all the supertypes and interfaces of a class
     *  or interface type, ordered by ClassSymbol.precedes (i.e. subclasses
     *  come first, arbitrary but fixed otherwise).
     */
    private Map<Type,List<Type>> closureCache = new HashMap<Type,List<Type>>();

    /** returns the closure of a class or interface type.
     */
    private List<Type> closure(Type t) {
	List<Type> cl = closureCache.get(t);
	if (cl == null) {
	    Type st = supertype(t);
	    if (!t.isCompound()) {
		if (st.tag == CLASS) cl = insert(closure(st), t);
		else cl = Type.emptyList.prepend(t);
	    } else {
		cl = closure(supertype(t));
	    }
	    for (List<Type> l = interfaces(t); l.nonEmpty(); l = l.tail)
		cl = union(cl, closure(l.head));
	    closureCache.put(t, cl);
	}
	return cl;
    }

    /** insert a type in a closure
     */
    public List<Type> insert(List<Type> cl, Type t) {
	if (cl.isEmpty() || t.tsym.precedes(cl.head.tsym, this)) {
	    return cl.prepend(t);
	} else if (cl.head.tsym.precedes(t.tsym, this)) {
	    return insert(cl.tail, t).prepend(cl.head);
	} else {
	    return cl;
	}
    }

    /** form the union of two closures
     */
    public List<Type> union(List<Type> cl1, List<Type> cl2) {
	if (cl1.isEmpty()) {
	    return cl2;
	} else if (cl2.isEmpty()) {
	    return cl1;
	} else if (cl1.head.tsym.precedes(cl2.head.tsym, this)) {
	    return union(cl1.tail, cl2).prepend(cl1.head);
	} else if (cl2.head.tsym.precedes(cl1.head.tsym, this)) {
	    return union(cl1, cl2.tail).prepend(cl2.head);
	} else {
	    return union(cl1.tail, cl2.tail).prepend(cl1.head);
	}
    }

    /** intersect two closures
     */
    public List<Type> intersect(List<Type> cl1, List<Type> cl2) {
	if (cl1 == cl2)
	    return cl1;
	if (cl1.isEmpty() || cl2.isEmpty())
	    return Type.emptyList;
	if (cl1.head.tsym.precedes(cl2.head.tsym, this))
	    return intersect(cl1.tail, cl2);
	if (cl2.head.tsym.precedes(cl1.head.tsym, this))
	    return intersect(cl1, cl2.tail);
	if (isSameType(cl1.head, cl2.head))
	    return intersect(cl1.tail, cl2.tail).prepend(cl1.head);
	if (cl1.head.tsym == cl2.head.tsym &&
	    cl1.head.tag == CLASS && cl2.head.tag == CLASS) {
	    if (cl1.head.isParameterized() && cl2.head.isParameterized()) {
		Type merge = merge(cl1.head,cl2.head);
		return intersect(cl1.tail, cl2.tail).prepend(merge);
	    }
	    if (cl1.head.isRaw() || cl2.head.isRaw())
		return intersect(cl1.tail, cl2.tail).prepend(erasure(cl1.head));
	}
	return intersect(cl1.tail, cl2.tail);
    }
    // where
        class TypePair {
            final Type t1;
            final Type t2;
            TypePair(Type t1, Type t2) {
                this.t1 = t1;
                this.t2 = t2;
            }
            public int hashCode() {
                return 127 * Types.this.hashCode(t1) + Types.this.hashCode(t2);
            }
            public boolean equals(Object other) {
                if (!(other instanceof TypePair)) return false;
                TypePair o = (TypePair)other;
                return isSameType(t1, o.t1) && isSameType(t2, o.t2);
            }
        }
        Set<TypePair> mergeCache = new HashSet<TypePair>();
	private Type merge(Type c1, Type c2) {
	    ClassType class1 = (ClassType) c1;
	    List<Type> act1 = class1.typarams();
	    ClassType class2 = (ClassType) c2;
	    List<Type> act2 = class2.typarams();
	    ListBuffer<Type> merged = new ListBuffer<Type>();
	    List<Type> typarams = class1.tsym.type.typarams();

	    while (act1.nonEmpty() && act2.nonEmpty() && typarams.nonEmpty()) {
		if (containsType(act1.head, act2.head)) {
		    merged.append(act1.head);
		} else if (containsType(act2.head, act1.head)) {
		    merged.append(act2.head);
		} else {
                    TypePair pair = new TypePair(c1, c2);
		    Type m;
		    if (mergeCache.add(pair)) {
			m = new ArgumentType(lub(upperBound(act1.head),
						 upperBound(act2.head)),
					     BoundKind.EXTENDS,
					     syms.boundClass);
			mergeCache.remove(pair);
		    } else {
			m = new ArgumentType(syms.objectType,
					     BoundKind.UNBOUND,
					     syms.boundClass);
		    }
		    merged.append(m.withTypeVar(typarams.head));
		}
		act1 = act1.tail;
		act2 = act2.tail;
		typarams = typarams.tail;
	    }
	    assert(act1.isEmpty() && act2.isEmpty() && typarams.isEmpty());
	    return new ClassType(class1.outer_field, merged.toList(), class1.tsym);
	}

    /** Compute a hash code on a type. */
    public int hashCode(Type t) {
        t.accept(hashCodeFcn);
        return hashCodeFcn.result;
    }
    private final HashCodeFcn hashCodeFcn = new HashCodeFcn();
    private class HashCodeFcn extends Visitor {
        int result;
	public void visitClassType(ClassType that) {
            int result = Types.this.hashCode(that.outer());
            result *= 127;
            result += that.tsym.flatName().hashCode();
            for (Type t : that.typarams()) {
                result *= 127;
                result += Types.this.hashCode(t);
            }
            this.result = result;
        }
	public void visitArgumentType(ArgumentType that) {
            int result = that.kind.hashCode();
            if (that.type != null) {
                result *= 127;
                result += Types.this.hashCode(that.type);
            }
            this.result = result;
        }
	public void visitArrayType(ArrayType that) {
            result = Types.this.hashCode(that.elemtype) + 12;
        }
	public void visitTypeVar(TypeVar that) { result = System.identityHashCode(that.tsym); }
	public void visitUndetVar(UndetVar that) { result = System.identityHashCode(that); }
	public void visitErrorType(ErrorType that) { result = 0; }
	public void visitType(Type that) {
            result = that.tag;
        }
    }

    /** Mapping to take element type of an arraytype
     */
    private Mapping elemTypeFun = new Mapping () {
        public String toString() { return "elemTypeFun"; }
	public Type apply(Type t) { return elemtype(t); }
    };

    /** return the minimum type of a closure, a compound type if no
     * unique minimum exists.
     */
    private Type compoundMin(List<Type> cl) {
	if (cl.isEmpty()) return syms.objectType;
	ListBuffer<Type> compound = new ListBuffer<Type>();
	ListBuffer<Type> candidates = new ListBuffer<Type>();
	while (cl.nonEmpty()) {
	    Type current = cl.head;
	    compound.append(current);
	    cl = cl.tail;
	    for (;cl.nonEmpty(); cl = cl.tail) {
		if (!isSubType(current, cl.head))
		    candidates.append(cl.head);
	    }
	    cl = candidates.toList();
	    candidates = new ListBuffer<Type>();
	}
	if (compound.count == 1)
	    return compound.toList().head;
	else if (compound.count == 0)
	    return null;
	else
	    return makeCompoundType(compound.toList());
    }

    /** return the least upper bound of pair of types.
     *  if the lub does not exist return null.
     */
    public Type lub(Type t1, Type t2) {
	return lub(Type.emptyList.prepend(t2).prepend(t1));
    }

    /** return the least upper bound of set of types.
     *  if the lub does not exist return null.
     */
    public Type lub(List<Type> ts) {
	final int ARRAY_BOUND = 1, CLASS_BOUND = 2;
	int boundkind = 0;
	for (List<Type> l = ts; l.nonEmpty(); l = l.tail) {
	    if (l.head.tag == CLASS) boundkind |= CLASS_BOUND;
	    else if (l.head.tag == ARRAY) boundkind |= ARRAY_BOUND;
	    else if (l.head.tag == TYPEVAR) {
		Type b = l.head.bound();
		while (b.tag == TYPEVAR) b = b.bound();
		if (b.tag == ARRAY) boundkind |= ARRAY_BOUND;
		else boundkind |= CLASS_BOUND;
	    }
	}
	switch (boundkind) {
	case 0:
	    return syms.botType;
	case ARRAY_BOUND:
	    // what about int[] and float[]?
	    return new ArrayType(lub(Type.map(ts, elemTypeFun)), syms.arrayClass);
	case CLASS_BOUND:
	    List<Type> cl = null;
	    while (ts.nonEmpty()) {
		if (ts.head.tag == CLASS || ts.head.tag == TYPEVAR)
		    if (cl == null) cl = closure(ts.head);
		    else cl = intersect(cl, closure(ts.head));
		ts = ts.tail;
	    }
	    return compoundMin(cl);
	default: // boundkind == ARRAY_BOUND | CLASS_BOUND
	    // What about Cloneable, Serializable, etc?
	    return syms.objectType;
	}
    }

    /** Does type have a result type that is a subtype of the
     *  result type of other, suitable for covariant returns?  It is
     *  assumed that both types are (possibly polymorphic) method
     *  types.  Monomorphic method types are handled in the obvious
     *  way.  Polymorphic method types require renaming all type
     *  variables of one to corresponding type variables in the other,
     *  where correspondence is by position in the type parameter
     *  list.  */
    public boolean resultSubtype(Type type, Type other, Warner warner) {
	List<Type> mtvars = type.typarams();
	List<Type> otvars = other.typarams();
	Type mtres = type.restype();
	Type otres = subst(other.restype(), otvars, mtvars);
	return covariantReturnType(mtres, otres, warner);
    }

    /*** Return-Type-Substitutable.
     * @see "The Java Language Specification, Third Ed. (8.4.5)
     */
    public boolean returnTypeSubstitutable(Type r1, Type r2) {
	if (hasSameArgs(r1, r2))
	    return resultSubtype(r1, r2, Warner.noWarnings);
	else
	    return covariantReturnType(r1.restype(),
				       erasure(r2.restype()),
				       Warner.noWarnings);
    }

    public boolean returnTypeSubstitutable(Type r1,
					   Type r2, Type r2res,
					   Warner warner) {
	if (isSameType(r1.restype(), r2res))
	    return true;
	if (r1.restype().isPrimitive() || r2res.isPrimitive())
	    return false;

	if (hasSameArgs(r1, r2))
	    return covariantReturnType(r1.restype(), r2res, warner);
	if (!source.allowCovariantReturns())
	    return false;
	if (isSubTypeUnchecked(r1.restype(), r2res, warner))
	    return true;
	if (!isSubType(r1.restype(), erasure(r2res)))
	    return false;
	warner.warnUnchecked();
	return true;
    }

    /** Is mtres an appropriate return type in an overrider for a
     *  method that returns otres? */
    public boolean covariantReturnType(Type mtres, Type otres, Warner warner) {
	return
            isSameType(mtres, otres) ||
            source.allowCovariantReturns() &&
            !mtres.isPrimitive() &&
            !otres.isPrimitive() &&
            isAssignable(mtres, otres, warner);
    }

    /*************************************************************
     * Box/unbox support
     ************************************************************/

    /** Return the class that boxes the given primitive. */
    public ClassSymbol boxedClass(Type type) {
	return reader.enterClass(syms.boxedName[type.tag]);
    }

    /** Return the primitive type corresponding to a boxed type. */
    public Type unboxedType(Type type) {
	for (int i=0; i<syms.boxedName.length; i++) {
	    Name box = syms.boxedName[i];
	    if (box != null &&
		asSuper(type, reader.enterClass(box)) != null)
		return syms.typeOfTag[i];
	}
	return Type.noType;
    }

    /*************************************************************
     * Capture conversion
     ************************************************************/

    public Type capture(Type type) {
	if (type.tag == CLASS) {
	    ClassType t = (ClassType)type;
	    Type result = t;
	    List<Type> capturedArgs = Type.emptyList;
	    boolean makeNew = false;
	    for (Type actual : t.typarams()) {
		if (actual.tag == TYPEARG) {
		    ArgumentType wildcard = (ArgumentType)actual;
		    Type capturedType = convert(wildcard);
		    capturedArgs = capturedArgs.append(capturedType);
		    makeNew = true;
		} else {
		    capturedArgs = capturedArgs.append(actual);
		}
	    }
	    if (makeNew) {
		result = new ClassType(t.outer(), capturedArgs, t.tsym);
	    }
	    return result;
	} else {
	    return type;
	}
    }
    // where
        public Type convert(ArgumentType wildcard) {
	    Type fbound = wildcard.bound == null ? null : wildcard.bound.bound;
	    Type upper = null;
	    Type lower = lowerBound(wildcard);
	    if (wildcard.isSuperBound()) {
		upper = fbound;
	    } else if (wildcard.isExtendsBound()) {
		upper = wildcard.type;
		if (fbound != null && !isSubType(upper, fbound))
		    upper = fbound; // FIXME: this should be glb.
	    }
	    if (upper == null)
		upper = syms.objectType;
	    if (upper == lower)
		return upper;
	    TypeVar capture
		= new CapturedType(names.fromString("capture of "+wildcard),
				   wildcard.type.tsym.owner, upper, lower);
	    return capture;
	}

    /*************************************************************
     * Internal utility methods
     ************************************************************/

    private List<Type> upperBounds(List<Type> those) {
        if (those.isEmpty()) return those;
        Type head = upperBound(those.head);
        List<Type> tail = upperBounds(those.tail);
        if (head != those.head || tail != those.tail)
            return tail.prepend(head);
        else
            return those;
    }

    private boolean sideCast(Type from, Type to, Warner warn) {
        // We are casting from type $from$ to type $to$, which are
        // non-final unrelated types.  This method
	// tries to reject a cast by transferring type parameters
	// from $to$ to $from$ by common superinterfaces.
        boolean reverse = false;
        Type target = to;
        if ((to.tsym.flags() & INTERFACE) == 0) {
            assert (from.tsym.flags() & INTERFACE) != 0;
            reverse = true;
            to = from;
            from = target;
        }
	boolean giveWarning = false;
	List<Type> commonSupers = superClosure(to, erasure(from));
	// The arguments to the supers could be unified here to
	// get a more accurate analysis
	while (commonSupers.nonEmpty()) {
            Type t1 = asSuper(from, commonSupers.head.tsym);
            Type t2 = commonSupers.head; // same as asSuper(to, commonSupers.head.tsym);
	    if (disjointTypes(t1.typarams(), t2.typarams()))
		return false;
	    giveWarning = giveWarning || (reverse ? giveWarning(t2, t1) : giveWarning(t1, t2));
	    commonSupers = commonSupers.tail;
	}
	if (giveWarning && !isReifiable(to))
	    warn.warnUnchecked();
        if (!source.allowCovariantReturns())
            // reject if there is a common method signature with
            // incompatible return types.
            chk.checkCompatibleAbstracts(warn.pos(), from, to);
	return true;
    }

    private boolean sideCastFinal(Type from, Type to, Warner warn) {
        // We are casting from type $from$ to type $to$, which are
        // unrelated types one of which is final and the other of
        // which is an interface.  This method
	// tries to reject a cast by transferring type parameters
	// from the final class to the interface.
        boolean reverse = false;
        Type target = to;
        if ((to.tsym.flags() & INTERFACE) == 0) {
            assert (from.tsym.flags() & INTERFACE) != 0;
            reverse = true;
            to = from;
            from = target;
        }
        assert (from.tsym.flags() & FINAL) != 0;
        Type t1 = asSuper(from, to.tsym);
        if (t1 == null) return false;
        Type t2 = to;
        if (disjointTypes(t1.typarams(), t2.typarams()))
            return false;
        if (!source.allowCovariantReturns())
            // reject if there is a common method signature with
            // incompatible return types.
            chk.checkCompatibleAbstracts(warn.pos(), from, to);
        if (!isReifiable(target) &&
            (reverse ? giveWarning(t2, t1) : giveWarning(t1, t2)))
            warn.warnUnchecked();
	return true;
    }

    private boolean giveWarning(Type from, Type to) {
	// To and from are (possibly different) parameterizations
	// of the same class or interface
	return to.isParameterized() && !containsType(to.typarams(), from.typarams());
    }

    private List<Type> superClosure(Type t, Type s) {
	List<Type> cl = Type.emptyList;
	for (List<Type> l = interfaces(t); l.nonEmpty(); l = l.tail) {
	    if (isSubType(s, erasure(l.head))) {
		cl = insert(cl, l.head);
	    } else {
		cl = union(cl, superClosure(l.head, s));
	    }
	}
	return cl;
    }

    private boolean containsTypeEquivalent(Type t, Type other) {
	return
	    isSameType(t, other) || // shortcut
	    containsType(t, other) && containsType(other, t);
    }

    /** Adapt a type by computing a substitution which
     *  maps a source type to a target type.
     *  @param from      the source type
     *  @param target    the target type
     *  @param from      the type variables of the computed substitution
     *  @param to        the types of the computed substitution.
     */
    private void adapt(Type source,
			      Type target,
			      ListBuffer<Type> from,
			      ListBuffer<Type> to) {
	if (source.tag == TYPEVAR) {
	    // A check should be made here to see if there is
	    // already a mapping for $source$, in which case
	    // appropriate action should be taken.
	    from.append(source);
	    to.append(target);
	} else if (source.tag == target.tag) {
	    switch (source.tag) {
	    case CLASS:
		adapt(source.allparams(), target.allparams(), from, to);
		break;
	    case ARRAY:
		adapt(elemtype(source), elemtype(target), from, to);
		break;
	    case TYPEARG:
		if (source.isExtendsBound())
		    adapt(upperBound(source), upperBound(target), from, to);
		else if (source.isSuperBound())
		    adapt(lowerBound(source), lowerBound(target), from, to);
		break;
	    }
	}
    }

    /** Adapt a type by computing a substitution which
     *  maps a list of source types to a list of target types.
     *  @param source    the source type
     *  @param target    the target type
     *  @param from      the type variables of the computed substitution
     *  @param to        the types of the computed substitution.
     */
    private void adapt(List<Type> source,
		       List<Type> target,
		       ListBuffer<Type> from,
		       ListBuffer<Type> to) {
	if (source.length() == target.length()) {
	    while (source.nonEmpty()) {
		adapt(source.head, target.head, from, to);
		source = source.tail;
		target = target.tail;
	    }
	}
    }
}

// outline-regexp: "\\s-*/[*]\\{10,\\}\n\\s-+[*]\\s-*"
// Local Variables:
// mode: java
// mode: outline-minor
// outline-regexp: "\\s-+[*]\\s-.*\n\\s-*[*]\\{10,\\}/\\|\\`"
// outline-heading-end-regexp: "\n"
// End:
