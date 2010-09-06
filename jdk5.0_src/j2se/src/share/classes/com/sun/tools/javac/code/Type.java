/**
 * @(#)Type.java	1.84 04/05/24
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.code;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.comp.Infer;

import static com.sun.tools.javac.code.Flags.*;
import static com.sun.tools.javac.code.Kinds.*;
import static com.sun.tools.javac.code.BoundKind.*;
import static com.sun.tools.javac.code.TypeTags.*;

/** This class represents GJ types. The class itself defines the behavior of
 *  the following types:
 *  <pre>
 *  base types (tags: BYTE, CHAR, SHORT, INT, LONG, FLOAT, DOUBLE, BOOLEAN),
 *  type `void' (tag: VOID),
 *  the bottom type (tag: BOT),
 *  the missing type (tag: NONE).
 *  </pre>
 *  <p>The behavior of the following types is defined in subclasses, which are
 *  all static inner classes of this class:
 *  <pre>
 *  class types (tag: CLASS, class: ClassType),
 *  array types (tag: ARRAY, class: ArrayType),
 *  method types (tag: METHOD, class: MethodType),
 *  package types (tag: PACKAGE, class: PackageType),
 *  type variables (tag: TYPEVAR, class: TypeVar),
 *  type arguments (tag: TYPEARG, class: ArgumentType),
 *  polymorphic types (tag: FORALL, class: ForAll),
 *  the error type (tag: ERROR, class: ErrorType).
 *  </pre>
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 *
 *  @see TypeTags
 */
public class Type {

    /** Constant type: no type at all. */
    public static final Type noType = new Type(NONE, null);

    /** If this switch is turned on, the names of type variables
     *  and anonymous classes are printed with hashcodes appended.
     */
    public static boolean moreInfo = false;

    /** The tag of this type.
     *
     *  @see TypeTags
     */
    public int tag;

    /** The defining class / interface / package / type variable
     */
    public TypeSymbol tsym;

    /** The constant value of this type, null if this type does not have
     *  a constant value attribute. Constant values can be set only for
     *  base types (numbers, booleans) and class types (strings).
     */
    public Object constValue = null;

    public void accept(Visitor v) { v.visitType(this); }

    /** Define a type given its tag and type symbol
     */
    public Type(int tag, TypeSymbol tsym) {
	this.tag = tag;
	this.tsym = tsym;
    }

    /** An abstract class for mappings from types to types
     */
    public static abstract class Mapping {
	public abstract Type apply(Type t);
    }

    /** map a type function over all immediate descendants of this type
     */
    public Type map(Mapping f) {
	return this;
    }

    /** map a type function over a list of types
     */
    public static List<Type> map(List<Type> these, Mapping f) {
	if (these.nonEmpty()) {
	    List<Type> tail1 = map(these.tail, f);
	    Type head1 = f.apply(these.head);
	    if (tail1 != these.tail || head1 != these.head)
		return tail1.prepend(head1);
	}
	return these;
    }

    /** Define a constant type, of the same kind as this type
     *  and with given constant value
     */
    public Type constType(Object constValue) {
	assert tag <= BOOLEAN;
	Type t = new Type(tag, tsym);
	t.constValue = constValue;
	return t;
    }

    /** If this is a constant type, return its underlying type.
     *  Otherwise, return the type itself.
     */
    public Type baseType() {
	if (constValue == null) return this;
	else return tsym.type;
    }

    /** Return the base types of a list of types.
     */
    public static List<Type> baseTypes(List<Type> types) {
	if (types.nonEmpty()) {
	    Type t = types.head.baseType();
	    List<Type> ts = baseTypes(types.tail);
	    if (t != types.head || ts != types.tail)
		return new List<Type>(t, ts);
	}
	return types;
    }

    /** The Java source which this type represents.
     */
    public String toString() {
	String s = (tsym == null || tsym.name == null)
	    ? "<none>"
	    : tsym.name.toString();
	if (moreInfo && tag == TYPEVAR) s = s + hashCode();
	return s;
    }

    /** The Java source which this type list represents.  A List is
     *  represented as a comma-spearated listing of the elements in
     *  that list.
     */
    public static String toString(List<Type> list) {
	if (list.isEmpty()) {
	    return "";
	} else {
	    StringBuffer buf = new StringBuffer();
	    buf.append(list.head.toString());
	    for (List<Type> l = list.tail; l.nonEmpty(); l = l.tail)
		buf.append(",").append(l.head.toString());
	    return buf.toString();
	}
    }

    /** The constant value of this type, converted to String
     *  PRE: Type has a non-null constValue field.
     */
    public String stringValue() {
	if (tag == BOOLEAN)
	    return ((Integer) constValue).intValue() == 0 ? "false" : "true";
	else if (tag == CHAR)
	    return String.valueOf((char) ((Integer) constValue).intValue());
	else
	    return constValue.toString();
    }

    /** This method is analogous to isSameType, but weaker, since we
     *  never complete classes. Where isSameType would complete a class,
     *  equals assumes that the two types are different.
     *  The code of this method is commented out since it is redundant.
     *  But it will be overridden in subclasses.
     */
//  public boolean equals(Object that) {
//      return super.equals(that);
//  }

    /** The code of this method is commented out since it is redundant.
     *  But it will be overridden in subclasses.
     */
//  public int hashCode() {
//	return super.hashCode();
//  }

    /** Is this a constant type whose value is false?
     */
    public boolean isFalse() {
	return
	    tag == BOOLEAN &&
	    constValue != null &&
	    ((Integer)constValue).intValue() == 0;
    }

    /** Is this a constant type whose value is true?
     */
    public boolean isTrue() {
	return
	    tag == BOOLEAN &&
	    constValue != null &&
	    ((Integer)constValue).intValue() != 0;
    }

    public String argtypes(boolean varargs) {
	List<Type> args = argtypes();
	if (!varargs) return args.toString();
	StringBuffer buf = new StringBuffer();
	while (args.tail.nonEmpty()) {
	    buf.append(args.head);
	    args = args.tail;
	    buf.append(',');
	}
	if (args.head.tag == ARRAY) {
	    buf.append(((ArrayType)args.head).elemtype);
	    buf.append("...");
	} else {
	    buf.append(args.head);
	}
	return buf.toString();
    }

    /** Access methods.
     */
    public List<Type>	     typarams() { return emptyList; }
    public Type		     outer()	{ return null; }
    public List<Type>	     argtypes() { return emptyList; }
    public Type		     restype()	{ return null; }
    public List<Type>	     thrown()	{ return Type.emptyList; }
    public Type		     bound()	{ return null; }

    public void setThrown(List<Type> t) {
	throw new AssertionError();
    }

    /** Navigation methods, these will work for classes, type variables,
     *  foralls, but will return null for arrays and methods.
     */

   /** Return all parameters of this type and all its outer types in order
    *  outer (first) to inner (last).
    */
    public List<Type> allparams() { return emptyList; }

    /** Does this type contain "error" elements?
     */
    public boolean isErroneous() {
	return false;
    }

    public static boolean isErroneous(List<Type> ts) {
	for (List<Type> l = ts; l.nonEmpty(); l = l.tail)
	    if (l.head.isErroneous()) return true;
	return false;
    }

    /** Is this type parameterized?
     *  A class type is parameterized if it has some parameters.
     *  An array type is parameterized if its element type is parameterized.
     *  All other types are not parameterized.
     */
    public boolean isParameterized() {
	return false;
    }

    /** Is this type a raw type?
     *  A class type is a raw type if it misses some of its parameters.
     *  An array type is a raw type if its element type is raw.
     *  All other types are not raw.
     *  Type validation will ensure that the only raw types
     *  in a program are types that miss all their type variables.
     */
    public boolean isRaw() {
	return false;
    }

    public boolean isCompound() {
	return (tsym.flags() & COMPOUND) != 0;
    }

    public boolean isPrimitive() {
	return tag < VOID;
    }

    /** Does this type contain occurrences of type `elem'?
     */
    public boolean contains(Type elem) {
	return elem == this;
    }

    public static boolean contains(List<Type> these, Type elem) {
	for (List<Type> l = these;
	     l.tail != null /*inlined: l.nonEmpty()*/;
	     l = l.tail)
	    if (l.head.contains(elem)) return true;
	return false;
    }

    /** Does this type contain an occurrence of some type in `elems'?
     */
    public boolean containsSome(List<Type> elems) {
	for (List<Type> l = elems; l.nonEmpty(); l = l.tail)
	    if (this.contains(elems.head)) return true;
	return false;
    }

    public boolean isSuperBound() { return false; }
    public boolean isExtendsBound() { return false; }
    public boolean isUnbound() { return false; }
    public Type withTypeVar(Type t) { return this; }

    public static List<Type> removeBounds(List<Type> types) {
	ListBuffer<Type> result = new ListBuffer<Type>();
	for(;types.nonEmpty(); types = types.tail) {
	    result.append(types.head.removeBounds());
	}
	return result.toList();
    }
    public Type removeBounds() {
	return this;
    }

    /** The underlying method type of this type.
     */
    public MethodType asMethodType() { throw new AssertionError(); }

    /** Complete loading all classes in this type.
     */
    public void complete() {}

    public Object clone() {
	try {
	    return super.clone();
	} catch (CloneNotSupportedException e) {
	    throw new AssertionError(e);
	}
    }

    /** An empty list of types.
     */
    public static final List<Type> emptyList = new List<Type>();

    /** VGJ: The class of type arguments. */
    public static class ArgumentType extends Type {

	public Type type;
	public BoundKind kind;
	public TypeVar bound;

	public void accept(Visitor v) {
	    v.visitArgumentType(this);
	}

	public ArgumentType(Type type, BoundKind kind, TypeSymbol tsym) {
	    super(TYPEARG, tsym);
	    assert(type != null);
	    this.kind = kind;
	    this.type = type;
	}
	public ArgumentType(ArgumentType that, TypeVar bound) {
	    this(that.type, that.kind, that.tsym, bound);
	}

	public ArgumentType(Type type, BoundKind kind, TypeSymbol tsym, TypeVar bound) {
	    this(type, kind, tsym);
	    this.bound = bound;
	}

	public boolean isSuperBound() {
	    return kind == SUPER ||
		kind == UNBOUND;
	}
	public boolean isExtendsBound() {
	    return kind == EXTENDS ||
		kind == UNBOUND;
	}
	public boolean isUnbound() {
	    return kind == UNBOUND;
	}

	public Type withTypeVar(Type t) {
	    //-System.err.println(this+".withTypeVar("+t+");");//DEBUG
	    if (bound == t)
		return this;
	    bound = (TypeVar)t;
	    return this;
	}

	boolean isPrintingBound = false;
	public String toString() {
	    StringBuffer s = new StringBuffer();
	    s.append(kind.toString());
	    if (kind != UNBOUND)
		s.append(type);
	    if (moreInfo && bound != null && !isPrintingBound)
		try {
		    isPrintingBound = true;
		    s.append("{:").append(bound.bound).append(":}");
		} finally {
		    isPrintingBound = false;
		}
	    return s.toString();
	}

	public Type map(Mapping f) {
	    //- System.err.println("   (" + this + ").map(" + f + ")");//DEBUG
	    Type t = type;
	    if (t != null)
		t = f.apply(t);
	    if (t == type)
		return this;
	    else
		return new ArgumentType(t, kind, tsym, bound);
	}

	public Type removeBounds() {
	    return isUnbound() ? this : type;
	}
    }

    public static class ClassType extends Type {

	/** The enclosing type of this type. If this is the type of an inner
	 *  class, outer_field refers to the type of its enclosing
	 *  instance class, in all other cases it referes to noType.
	 */
	public Type outer_field;

	/** The type parameters of this type (to be set once class is loaded).
	 */
	public List<Type> typarams_field;

	/** A cache variable for the type parameters of this type,
	 *  appended to all parameters of its enclosing class.
	 *  @see allparams()
	 */
	public List<Type> allparams_field;

	/** The supertype of this class (to be set once class is loaded).
	 */
	public Type supertype_field;

	/** The interfaces of this class (to be set once class is loaded).
	 */
	public List<Type> interfaces_field;

        public ClassType(Type outer, List<Type> typarams, TypeSymbol tsym) {
	    super(CLASS, tsym);
	    this.outer_field = outer;
	    this.typarams_field = typarams;
	    this.allparams_field = null;
	    this.supertype_field = null;
	    this.interfaces_field = null;
            /*
            // this can happen during error recovery
            assert
                outer.isParameterized() ?
                typarams.length() == tsym.type.typarams().length() :
                outer.isRaw() ?
                typarams.length() == 0 :
                true;
            */
	}

	public void accept(Visitor v) {
	    v.visitClassType(this);
	}

        public Type constType(Object constValue) {
	    Type t = new ClassType(outer_field, typarams_field, tsym);
	    t.constValue = constValue;
	    return t;
	}

        /** The Java source which this type represents.
	 */
	public String toString() {
	    StringBuffer buf = new StringBuffer();
	    if (outer().tag == CLASS && tsym.owner.kind == TYP) {
		buf.append(outer().toString());
		buf.append(".");
		buf.append(className(tsym, false));
	    } else {
		buf.append(className(tsym, true));
	    }
	    if (typarams().nonEmpty()) {
		buf.append('<');
		buf.append(typarams().toString());
		buf.append(">");
	    }
	    return buf.toString();
	}
//where
            private String className(Symbol sym, boolean longform) {
		if (sym.name.len == 0 && (sym.flags() & COMPOUND) != 0) {
		    StringBuffer s = new StringBuffer(supertype_field.toString());
		    for (List<Type> is=interfaces_field; is.nonEmpty(); is = is.tail) {
			s.append("&");
			s.append(is.head.toString());
		    }
		    return s.toString();
		} else if (sym.name.len == 0) {
		    String s;
                    ClassType norm = (ClassType) tsym.type;
		    if (norm.interfaces_field.nonEmpty()) {
			s = Log.getLocalizedString("anonymous.class",
						   norm.interfaces_field.head);
		    } else {
			s = Log.getLocalizedString("anonymous.class",
						   norm.supertype_field);
		    }
		    if (moreInfo)
			s += String.valueOf(sym.hashCode());
		    return s;
		} else if (longform) {
		    return sym.fullName().toString();
		} else {
		    return sym.name.toString();
		}
	    }

        public List<Type> typarams() {
	    if (typarams_field == null) {
		complete();
                if (typarams_field == null)
                    typarams_field = Type.emptyList;
	    }
	    return typarams_field;
	}

        public Type outer() {
	    return outer_field;
	}

        public List<Type> allparams() {
	    if (allparams_field == null) {
		allparams_field = typarams().prependList(outer().allparams());
	    }
	    return allparams_field;
	}

        public boolean isErroneous() {
	    return
		outer().isErroneous() ||
		isErroneous(typarams()) ||
		this != tsym.type && tsym.type.isErroneous();
	}

        public boolean isParameterized() {
	    return allparams().tail != null;
            // optimization, was: allparams().nonEmpty();
	}

	/** A cache for the rank. */
	int rank_field = -1;

	/** A class type is raw if it misses some
	 *  of its type parameter sections.
	 *  After validation, this is equivalent to:
	 *  allparams.isEmpty() && tsym.type.allparams.nonEmpty();
	 */
        public boolean isRaw() {
	    return
		this != tsym.type && // necessary, but not sufficient condition
		tsym.type.allparams().nonEmpty() &&
		allparams().isEmpty();
	}

	public Type map(Mapping f) {
	    Type outer = outer();
	    Type outer1 = f.apply(outer);
	    List<Type> typarams = typarams();
	    List<Type> typarams1 = map(typarams, f);
	    if (outer1 == outer && typarams1 == typarams) return this;
	    else return new ClassType(outer1, typarams1, tsym);
	}

        public boolean contains(Type elem) {
	    return
		elem == this
		|| (isParameterized()
		    && (outer().contains(elem) || contains(typarams(), elem)));
	}

	public void complete() {
	    if (tsym.completer != null) tsym.complete();
	}
	
    }

    public static class ArrayType extends Type {

	public Type elemtype;

        public ArrayType(Type elemtype, TypeSymbol arrayClass) {
	    super(ARRAY, arrayClass);
	    this.elemtype = elemtype;
	}

	public void accept(Visitor v) {
	    v.visitArrayType(this);
	}

        public String toString() {
	    return elemtype + "[]";
	}

	public boolean equals(Object that) {
	    return
		this == that ||
		(that instanceof ArrayType &&
		 this.elemtype.equals(((ArrayType)that).elemtype));
	}

	public int hashCode() {
	    return (ARRAY << 5) + elemtype.hashCode();
	}

        public List<Type> allparams() { return elemtype.allparams(); }

        public boolean isErroneous() {
	    return elemtype.isErroneous();
	}

        public boolean isParameterized() {
	    return elemtype.isParameterized();
	}

        public boolean isRaw() {
	    return elemtype.isRaw();
	}

	public Type map(Mapping f) {
	    Type elemtype1 = f.apply(elemtype);
	    if (elemtype1 == elemtype) return this;
	    else return new ArrayType(elemtype1, tsym);
	}

        public boolean contains(Type elem) {
	    return elem == this || elemtype.contains(elem);
	}

	public void complete() {
	    elemtype.complete();
	}
    }

    public static class MethodType extends Type implements Cloneable {

	public List<Type> argtypes;
        public Type restype;
	public List<Type> thrown;

	public MethodType(List<Type> argtypes,
			  Type restype,
			  List<Type> thrown,
			  TypeSymbol methodClass) {
	    super(METHOD, methodClass);
	    this.argtypes = argtypes;
	    this.restype = restype;
	    this.thrown = thrown;
	}

	public void accept(Visitor v) {
	    v.visitMethodType(this);
	}

        /** The Java source which this type represents.
	 *
	 *  XXX 06/09/99 iris This isn't correct Java syntax, but it probably
	 *  should be.
	 */
	public String toString() {
	    return "(" + argtypes + ")" + restype;
	}

	public boolean equals(Object that) {
	    if (this == that) return true;
	    if (!(that instanceof MethodType)) return false;
	    MethodType mthat = (MethodType)that;
	    List<Type> thisargs = this.argtypes;
	    List<Type> thatargs = mthat.argtypes;
	    while (thisargs.tail != null && thatargs.tail != null
		   /*inlined: thisargs.nonEmpty() && thatargs.nonEmpty()*/ &&
		   thisargs.head.equals(thatargs.head)) {
		thisargs = thisargs.tail;
		thatargs = thatargs.tail;
	    }
	    if (thisargs.tail != null || thatargs.tail != null
		/*inlined: !thisargs.isEmpty() || !thatargs.isEmpty();*/)
		return false;
	    return this.restype.equals(mthat.restype);
	}

	public int hashCode() {
	    int h = METHOD;
	    for (List<Type> thisargs = this.argtypes;
		 thisargs.tail != null; /*inlined: thisargs.nonEmpty()*/
		 thisargs = thisargs.tail)
		h = (h << 5) + thisargs.head.hashCode();
	    return (h << 5) + this.restype.hashCode();
	}

	public List<Type>        argtypes() { return argtypes; }
	public Type              restype()  { return restype; }
        public List<Type>        thrown()   { return thrown; }

	public void setThrown(List<Type> t) {
	    thrown = t;
	}

        public boolean isErroneous() {
	    return
		isErroneous(argtypes) ||
		restype != null && restype.isErroneous();
	}

	public Type map(Mapping f) {
	    List<Type> argtypes1 = map(argtypes, f);
	    Type restype1 = f.apply(restype);
	    List<Type> thrown1 = map(thrown, f);
	    if (argtypes1 == argtypes &&
		restype1 == restype &&
		thrown1 == thrown) return this;
	    else return new MethodType(argtypes1, restype1, thrown1, tsym);
	}

        public boolean contains(Type elem) {
	    return elem == this || contains(argtypes, elem) || restype.contains(elem);
	}

        public MethodType asMethodType() { return this; }

	public void complete() {
	    for (List<Type> l = argtypes; l.nonEmpty(); l = l.tail)
		l.head.complete();
	    restype.complete();
	    for (List<Type> l = thrown; l.nonEmpty(); l = l.tail)
		l.head.complete();
	}
    }

    public static class PackageType extends Type {

	PackageType(TypeSymbol tsym) {
	    super(PACKAGE, tsym);
	}

	public void accept(Visitor v) {
	    v.visitPackageType(this);
	}

        public String toString() {
	    return tsym.fullName().toString();
	}
    }

    public static class TypeVar extends Type {

	/** The bound of this type variable; set from outside.
	 *  Must be nonempty once it is set.
	 *  For a bound, `bound' is the bound type itself.
	 *  Multiple bounds are expressed as a single class type which has the
	 *  individual bounds as superclass, respectively interfaces.
	 *  The class type then has as `tsym' a compiler generated class `c',
	 *  which has a flag COMPOUND and whose owner is the type variable
	 *  itself. Furthermore, the erasure_field of the class
	 *  points to the first class or interface bound.
	 */
        public Type bound = null;

	public TypeVar(Name name, Symbol owner) {
	    super(TYPEVAR, null);
	    tsym = new TypeSymbol(0, name, this, owner);
	}

	public TypeVar(TypeSymbol tsym, Type bound) {
	    super(TYPEVAR, tsym);
	    this.bound = bound;
	}

	public void accept(Visitor v) {
	    v.visitTypeVar(this);
	}

        public Type bound() { return bound; }

	int rank_field = -1;
    }

    /** A captured type variable comes from wildcards which can have
     *  both upper and lower bound.  CapturedType extends TypeVar with
     *  a lower bound.
     */
    public static class CapturedType extends TypeVar {

	public Type lower;

	public CapturedType(Name name, Symbol owner, Type upper, Type lower) {
	    super(name, owner);
	    this.bound = upper;
	    this.lower = lower;
	}

	public void accept(Visitor v) {
	    v.visitCapturedType(this);
	}

    }

    public static abstract class DelegatedType extends Type {
	public Type qtype;
	public DelegatedType(int tag, Type qtype) {
	    super(tag, qtype.tsym);
	    this.qtype = qtype;
	}
	public void accept(Visitor v) {
	    v.visitDelegatedType(this);
	}
	public String toString() { return qtype.toString(); }
	public List<Type> typarams() { return qtype.typarams(); }
	public Type outer() { return qtype.outer(); }
	public List<Type> argtypes() { return qtype.argtypes(); }
	public Type restype() { return qtype.restype(); }
	public List<Type> thrown() { return qtype.thrown(); }
	public List<Type> allparams() { return qtype.allparams(); }
	public Type bound() { return qtype.bound(); }
	public Object clone() { DelegatedType t = (DelegatedType)super.clone(); t.qtype = (Type)qtype.clone(); return t; }
	public boolean isErroneous() { return qtype.isErroneous(); }
    }

    public static class ForAll extends DelegatedType implements Cloneable {
	public List<Type> tvars;

	public ForAll(List<Type> tvars, Type qtype) {
	    super(FORALL, qtype);
	    this.tvars = tvars;
	}

	public void accept(Visitor v) {
	    v.visitForAll(this);
	}

        public String toString() {
	    return "<" + tvars + ">" + qtype;
	}

	public List<Type> typarams()   { return tvars; }

	public void setThrown(List<Type> t) {
	    qtype.setThrown(t);
	}

	public Object clone() {
	    ForAll result = (ForAll)super.clone();
	    result.qtype = (Type)result.qtype.clone();
	    return result;
	}

	public boolean isErroneous()  {
	    return qtype.isErroneous();
	}

	public Type map(Mapping f) {
	    return f.apply(qtype);
	}

        public boolean contains(Type elem) {
	    return qtype.contains(elem);
	}

        public MethodType asMethodType() {
	    return qtype.asMethodType();
	}

	public void complete() {
	    for (List<Type> l = tvars; l.nonEmpty(); l = l.tail) {
		((TypeVar)l.head).bound.complete();
	    }
	    qtype.complete();
	}
    }

    /** A class for instantiatable variables, for use during type
     *  inference.
     */
    public static class UndetVar extends DelegatedType {
	public List<Type> lobounds = emptyList;
	public List<Type> hibounds = emptyList;
	public Type inst = null;

	public void accept(Visitor v) { v.visitUndetVar(this); }

	public UndetVar(Type origin) {
	    super(UNDETVAR, origin);
	}

	public String toString() {
	    if (inst != null) return inst.toString();
	    else return qtype + "?";
	}

	public Type baseType() {
	    if (inst != null) return inst.baseType();
	    else return this;
	}
    }

    public static class ErrorType extends ClassType {

	public ErrorType() {
	    super(noType, emptyList, null);
	    tag = ERROR;
	}

	public ErrorType(ClassSymbol c) {
	    this();
	    tsym = c;
	    c.type = this;
	    c.kind = ERR;
	    c.members_field = new Scope.ErrorScope(c);
	}

	public ErrorType(Name name, TypeSymbol container) {
	    this(new ClassSymbol(PUBLIC|STATIC|ACYCLIC, name, null, container));
	}

	public void accept(Visitor v) {
	    v.visitErrorType(this);
	}

        public Type constType(Object constValue) { return this; }
        public Type outer()                      { return this; }
        public Type restype()                	 { return this; }
        public Type asSub(Symbol sym)        	 { return this; }
	public Type map(Mapping f)               { return this; }

        public boolean isGenType(Type that)	 { return true; }
        public boolean isErroneous()         	 { return true; }

        public List<Type> allparams()		 { return emptyList; }
        public List<Type> typarams()		 { return emptyList; }
    }

    public static abstract class Visitor {
	public void visitClassType(ClassType that) { visitType(that); }
	public void visitArgumentType(ArgumentType that) { visitType(that); }
	public void visitArrayType(ArrayType that) { visitType(that); }
	public void visitMethodType(MethodType that) { visitType(that); }
	public void visitPackageType(PackageType that) { visitType(that); }
	public void visitTypeVar(TypeVar that) { visitType(that); }
	public void visitCapturedType(CapturedType that) { visitTypeVar(that); } // NB!
	public void visitDelegatedType(DelegatedType that) { that.qtype.accept(this); } // NB!
	public void visitForAll(ForAll that) { visitDelegatedType(that); } // NB!
	public void visitUndetVar(UndetVar that) { visitDelegatedType(that); } // NB!
	public void visitErrorType(ErrorType that) { visitType(that); }
	public abstract void visitType(Type that);
    }
}
