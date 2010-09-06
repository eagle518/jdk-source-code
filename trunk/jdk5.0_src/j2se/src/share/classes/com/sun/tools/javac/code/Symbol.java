/**
 * @(#)Symbol.java	1.75 04/07/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.code;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.code.Type.*;
import com.sun.tools.javac.jvm.*;

import static com.sun.tools.javac.code.Flags.*;
import static com.sun.tools.javac.code.Kinds.*;
import static com.sun.tools.javac.code.TypeTags.*;

/** Root class for Java symbols. It contains subclasses
 *  for specific sorts of symbols, such as variables, methods and operators,
 *  types, packages. Each subclass is represented as a static inner class
 *  inside Symbol.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Symbol {

    /** The kind of this symbol.
     *  @see Kinds.
     */
    public int kind;

    /** The flags of this symbol.
     */
    public long flags_field;

    /** An accessor method for the flags of this symbol.
     *  Flags of class symbols should be accessed through the accessor
     *  method to make sure that the class symbol is loaded.
     */
    public long flags() { return flags_field; }

    /** The attributes of this symbol.
     */
    public List<Attribute.Compound> attributes_field;

    /** An accessor method for the attributes of this symbol.
     *  Flags of class symbols should be accessed through the accessor
     *  method to make sure that the class symbol is loaded.
     */
    public List<Attribute.Compound> attributes() {
	assert attributes_field != null;
	return attributes_field;
    }

    /** Fetch a particular annotation from a symbol. */
    public Attribute.Compound attribute(Symbol anno) {
	for (Attribute.Compound a : attributes())
	    if (a.type.tsym == anno) return a;
	return null;
    }

    /** The name of this symbol in Utf8 representation.
     */
    public Name name;

    /** The type of this symbol.
     */
    public Type type;

    /** The owner of this symbol.
     */
    public Symbol owner;

    /** The completer of this symbol.
     */
    public Completer completer;

    /** A cache for the type erasure of this symbol.
     */
    public Type erasure_field;

    /** Construct a symbol with given kind, flags, name, type and owner.
     */
    public Symbol(int kind, long flags, Name name, Type type, Symbol owner) {
	this.kind = kind;
        this.flags_field = flags;
        this.type = type;
	this.owner = owner;
	this.completer = null;
	this.erasure_field = null;
	this.attributes_field = Attribute.Compound.emptyList;
	this.name = name;
    }

    /** Clone this symbol with new owner.
     *  Legal only for fields and methods.
     */
    public Symbol clone(Symbol newOwner) {
	throw new AssertionError();
    }

    /** The Java source which this symbol represents.
     *  A description of this symbol; overrides Object.
     */
    public String toString() {
        return name.toString();
    }

    /** A Java source description of the location of this symbol; used for
     *  error reporting.  Use of this method may result in the loss of the
     *  symbol's description.
     */
    public String location() {
	if (owner.name == null || owner.name.len == 0) return "";
	else return owner.toString();
    }

    public String location(Type site, Types types) {
	if (owner.name == null || owner.name.len == 0) {
	    return "";
	}
	if (owner.type.tag == CLASS) {
	    Type ownertype = types.asOuterSuper(site, owner);
	    if (ownertype != null) return ownertype.toString();
	}
	return owner.toString();
    }

    /** The symbol's erased type.
     */
    public Type erasure(Types types) {
	if (erasure_field == null)
	    erasure_field = types.erasure(type);
	return erasure_field;
    }

    /** The external type of a symbol. This is the symbol's erased type
     *  except for constructors of inner classes which get the enclosing
     *  instance class added as first argument.
     */
    public Type externalType(Types types) {
	Type t = erasure(types);
	if (name == name.table.init && owner.hasOuterInstance()) {
	    Type outerThisType = types.erasure(owner.type.outer());
	    return new MethodType(t.argtypes().prepend(outerThisType),
				  t.restype(),
				  t.thrown(),
				  t.tsym);
	} else {
	    return t;
	}
    }

    public boolean isStatic() {
	return
	    (flags() & STATIC) != 0 ||
	    (owner.flags() & INTERFACE) != 0 && kind != MTH;
    }

    /** Is this symbol declared (directly or indirectly) local
     *  to a method or variable initializer?
     *  Also includes fields of inner classes which are in
     *  turn local to a method or variable initializer.
     */
    public boolean isLocal() {
	return
	    (owner.kind & (VAR | MTH)) != 0 ||
	    (owner.kind == TYP && owner.isLocal());
    }

    /** Is this symbol a constructor?
     */
    public boolean isConstructor() {
	return name == name.table.init;
    }

    /** The fully qualified name of this symbol.
     *  This is the same as the symbol's name except for class symbols,
     *  which are handled separately.
     */
    public Name fullName() {
	return name;
    }

    /** The fully qualified name of this symbol after converting to flat
     *  representation. This is the same as the symbol's name except for
     *  class symbols, which are handled separately.
     */
    public Name flatName() {
	return fullName();
    }

    /** If this is a class or package, its members, otherwise null.
     */
    public Scope members() {
	return null;
    }

    /** A class is an inner class if it it has an enclosing instance class.
     */
    public boolean isInner() {
	return type.outer().tag == CLASS;
    }

    /** An inner class has an outer instance if it is not an interface
     *  it has an enclosing instance class which might be referenced from the class.
     *  Nested classes can see instance members of their enclosing class.
     *  Their constructors carry an additional this$n parameter, inserted
     *  implicitly by the compiler.
     *
     *  @see isInner
     */
    public boolean hasOuterInstance() {
	return
	    type.outer().tag == CLASS && (flags() & (INTERFACE | NOOUTERTHIS)) == 0;
    }

    /** The closest enclosing class of this symbol's declaration.
     */
    public ClassSymbol enclClass() {
	Symbol c = this;
	while (c != null &&
	       ((c.kind & TYP) == 0 || c.type.tag != CLASS)) {
	    c = c.owner;
	}
	return (ClassSymbol)c;
    }

    /** The outermost class which indirectly owns this symbol.
     */
    public ClassSymbol outermostClass() {
	Symbol sym = this;
	Symbol prev = null;
	while (sym.kind != PCK) {
	    prev = sym;
	    sym = sym.owner;
	}
	return (ClassSymbol) prev;
    }

    /** The package which indirectly owns this symbol.
     */
    public PackageSymbol packge() {
	Symbol sym = this;
	while (sym.kind != PCK) {
	    sym = sym.owner;
	}
	return (PackageSymbol) sym;
    }

    /** Is this symbol a subclass of `base'? Only defined for ClassSymbols.
     */
    public boolean isSubClass(Symbol base, Types types) {
	throw new AssertionError("isSubClass " + this);
    }

    /** Fully check membership: hierarchy, protection, and hiding.
     *  Does not exclude methods not inherited due to overriding.
     */
    public boolean isMemberOf(TypeSymbol clazz, Types types) {
	return
	    owner == clazz ||
	    clazz.isSubClass(owner, types) &&
	    isInheritedIn(clazz, types) &&
	    !hiddenIn((ClassSymbol)clazz, types);
    }

    /** Is this symbol the same as or enclosed by the given class? */
    public boolean isEnclosedBy(ClassSymbol clazz) {
	for (Symbol sym = this; sym.kind != PCK; sym = sym.owner)
	    if (sym == clazz) return true;
	return false;
    }

    /** Check for hiding.  Note that this doesn't handle multiple
     *  (interface) inheritance. */
    private boolean hiddenIn(ClassSymbol clazz, Types types) {
	if (kind == MTH && (flags() & STATIC) == 0) return false;
	while (true) {
	    if (owner == clazz) return false;
	    Scope.Entry e = clazz.members().lookup(name);
	    while (e.scope != null) {
		if (e.sym == this) return false;
		if (e.sym.kind == kind &&
		    (kind != MTH ||
		     (e.sym.flags() & STATIC) != 0 &&
		     types.isSubSignature(e.sym.type, type)))
		    return true;
		e = e.next();
	    }
	    Type superType = types.supertype(clazz.type);
	    if (superType.tag != TypeTags.CLASS) return false;
	    clazz = (ClassSymbol)superType.tsym;
	}
    }

    /** Is this symbol inherited into a given class?
     *  PRE: If symbol's owner is a interface,
     *       it is already assumed that the interface is a superinterface
     *       of given class.
     *  @param clazz  The class for which we want to establish membership.
     *                This must be a subclass of the member's owner.
     */
    public boolean isInheritedIn(Symbol clazz, Types types) {
	switch ((int)(flags_field & Flags.AccessFlags)) {
	default: // error recovery
	case PUBLIC:
	    return true;
	case PRIVATE:
	    return this.owner == clazz;
	case PROTECTED:
	    // we model interfaces as extending Object
	    return (clazz.flags() & INTERFACE) == 0;
	case 0:
	    PackageSymbol thisPackage = this.packge();
	    for (Symbol sup = clazz; sup != null && sup != this.owner; sup = types.supertype(sup.type).tsym)
		if (sup.packge() != thisPackage) return false;
	    return (clazz.flags() & INTERFACE) == 0;
	}
    }

    /** The (variable or method) symbol seen as a member of given
     *  class type`site' (this might change the symbol's type).
     *  This is used exclusively for producing diagnostics.
     */
    public Symbol asMemberOf(Type site, Types types) {
	throw new AssertionError();
    }

    /** Does this method symbol override `other' symbol, when both are seen as
     *  members of class `origin'?  It is assumed that _other is a member
     *  of origin.
     *
     *  It is assumed that both symbols have the same name.  The static
     *  modifier is ignored for this test.
     *
     *  See JLS 8.4.6.1 (without transitivity) and 8.4.6.4
     */
    public boolean overrides(Symbol _other, TypeSymbol origin, Types types, boolean checkResult) {
	return false;
    }

    /** Complete the elaboration of this symbol's definition.
     */
    public void complete() throws CompletionFailure {
	if (completer != null) {
	    Completer c = completer;
	    completer = null;
	    c.complete(this);
	}
    }

    /** True if the symbol represents an entity that exists.
     */
    public boolean exists() {
        return true;
    }

    public static class DelegatedSymbol extends Symbol {
	protected Symbol other;
	public DelegatedSymbol(Symbol other) {
	    super(other.kind, other.flags_field, other.name, other.type, other.owner);
	    this.other = other;
	}
	public String toString() { return other.toString(); }
	public String location() { return other.location(); }
	public String location(Type site, Types types) { return other.location(site, types); }
	public Type erasure(Types types) { return other.erasure(types); }
	public Type externalType(Types types) { return other.externalType(types); }
	public boolean isLocal() { return other.isLocal(); }
	public boolean isConstructor() { return other.isConstructor(); }
	public Name fullName() { return other.fullName(); }
	public Name flatName() { return other.flatName(); }
	public Scope members() { return other.members(); }
	public boolean isInner() { return other.isInner(); }
	public boolean hasOuterInstance() { return other.hasOuterInstance(); }
	public ClassSymbol enclClass() { return other.enclClass(); }
	public ClassSymbol outermostClass() { return other.outermostClass(); }
	public PackageSymbol packge() { return other.packge(); }
	public boolean isSubClass(Symbol base, Types types) { return other.isSubClass(base, types); }
	public boolean isMemberOf(TypeSymbol clazz, Types types) { return other.isMemberOf(clazz, types); }
	public boolean isEnclosedBy(ClassSymbol clazz) { return other.isEnclosedBy(clazz); }
	public boolean isInheritedIn(Symbol clazz, Types types) { return other.isInheritedIn(clazz, types); }
	public Symbol asMemberOf(Type site, Types types) { return other.asMemberOf(site, types); }
	public void complete() throws CompletionFailure { other.complete(); }
    }

    /** A class for type symbols. Type variables are represented by instances
     *  of this class, classes and packages by instances of subclasses.
     */
    public static class TypeSymbol extends Symbol {

	public TypeSymbol(long flags, Name name, Type type, Symbol owner) {
	    super(TYP, flags, name, type, owner);
	}

        public String toString() {
	    return name.toString();
	}

	/** form a fully qualified name from a name and an owner
	 */
	static public Name formFullName(Name name, Symbol owner) {
	    if (owner == null) return name;
	    if (((owner.kind != ERR)) &&
		((owner.kind & (VAR | MTH)) != 0
		 || (owner.kind == TYP && owner.type.tag == TYPEVAR)
		 )) return name;
	    Name prefix = owner.fullName();
	    if (prefix == null || prefix == prefix.table.empty || prefix == prefix.table.emptyPackage)
		return name;
	    else return prefix.append('.', name);
	}

	/** form a fully qualified name from a name and an owner, after
	 *  converting to flat representation
	 */
	static public Name formFlatName(Name name, Symbol owner) {
	    if (owner == null ||
		(owner.kind & (VAR | MTH)) != 0
		|| (owner.kind == TYP && owner.type.tag == TYPEVAR)
		) return name;
	    char sep = owner.kind == TYP ? '$' : '.';
	    Name prefix = owner.flatName();
	    if (prefix == null || prefix == prefix.table.empty || prefix == prefix.table.emptyPackage)
		return name;
	    else return prefix.append(sep, name);
	}

        /** A total ordering between type symbols that refines the class
	 *  inheritance graph. Typevariables always precede other type symbols.
	 */
	public boolean precedes(TypeSymbol that, Types types) {
	    return this != that;
	}
    }

    /** A class for package symbols
     */
    public static class PackageSymbol extends TypeSymbol {

	public Scope members_field;
	public Name fullname;

	public PackageSymbol(Name name, Type type, Symbol owner) {
	    super(0, name, type, owner);
	    this.kind = PCK;
	    this.members_field = null;
	    this.fullname = formFullName(name, owner);
	}

	public PackageSymbol(Name name, Symbol owner) {
	    this(name, null, owner);
	    this.type = new PackageType(this);
	}

	/** The Java source which this symbol represents.
	 */
	public String toString() {
	    return fullname.toString();
	}

        public Name fullName() {
	    return fullname;
	}

        public Scope members() {
	    if (completer != null) complete();
	    return members_field;
	}

        public long flags() {
	    if (completer != null) complete();
	    return flags_field;
        }

	/** A package "exists" if a type or package that exists has
	 *  been seen within it.
	 */
        public boolean exists() {
	    return (flags_field & EXISTS) != 0;
	}
    }

    /** A class for class symbols
     */
    public static class ClassSymbol extends TypeSymbol {

	/** a scope for all class members; variables, methods and inner classes
	 *  type parameters are not part of this scope
	 */
	public Scope members_field;

	/** the fully qualified name of the class, i.e. pck.outer.inner.
	 *  null for anonymous classes
	 */
	public Name fullname;

	/** the fully qualified name of the class after converting to flat
	 *  representation, i.e. pck.outer$inner,
	 *  set externally for local and anonymous classes
	 */
	public Name flatname;

	/** the sourcefile where the class came from
	 */
	public Name sourcefile;

	/** the classfile from where to load this class
	 *  this will have extension .class or .java
	 */
	public FileEntry classfile;

	/** the constant pool of the class
	 */
	public Pool pool;

	public ClassSymbol(long flags, Name name, Type type, Symbol owner) {
	    super(flags, name, type, owner);
	    this.members_field = null;
	    this.fullname = formFullName(name, owner);
	    this.flatname = formFlatName(name, owner);
	    this.sourcefile = null;
	    this.classfile = null;
	    this.pool = null;
	}

	public ClassSymbol(long flags, Name name, Symbol owner) {
	    this(
		flags,
		name,
		new ClassType(Type.noType, null, null),
		owner);
	    this.type.tsym = this;
	}

	public static final List<ClassSymbol> emptyList = new List<ClassSymbol>();

	/** The Java source which this symbol represents.
	 */
	public String toString() {
	    return className();
	}

        public long flags() {
	    if (completer != null) complete();
	    return flags_field;
        }

        public Scope members() {
	    if (completer != null) complete();
	    return members_field;
	}

        public Type erasure(Types types) {
	    if (erasure_field == null)
		erasure_field = new ClassType(types.erasure(type.outer()),
					      Type.emptyList, this);
	    return erasure_field;
	}

        public String className() {
	    if (name.len == 0)
		return
		    Log.getLocalizedString("anonymous.class", flatname);
	    else
		return fullname.toString();
	}

        public Name fullName() {
	    return fullname;
	}

        public Name flatName() {
	    return flatname;
	}

	public boolean isSubClass(Symbol base, Types types) {
	    if (this == base) {
		return true;
	    } else if ((base.flags() & INTERFACE) != 0) {
		for (Type t = type; t.tag == CLASS; t = types.supertype(t))
		    for (List<Type> is = types.interfaces(t);
			 is.nonEmpty();
			 is = is.tail)
			if (is.head.tsym.isSubClass(base, types)) return true;
	    } else {
		for (Type t = type; t.tag == CLASS; t = types.supertype(t))
		    if (t.tsym == base) return true;
	    }
	    return false;
	}

        /** A total ordering between class symbols that refines the class
	 *  inheritance graph.
	 */
	public boolean precedes(TypeSymbol that, Types types) {
	    return
		types.rank(that.type) < types.rank(this.type) ||
		types.rank(that.type) == types.rank(this.type) &&
		that.fullName().compareTo(this.fullname) < 0;
	}

	/** Complete the elaboration of this symbol's definition.
	 */
	public void complete() throws CompletionFailure {
	    try {
		super.complete();
	    } catch (CompletionFailure ex) {
		// quiet error recovery
		flags_field |= (PUBLIC|STATIC);
		this.type = new ErrorType(this);
		throw ex;
	    }
	}
    }


    /** A class for variable symbols
     */
    public static class VarSymbol extends Symbol {

        /** The variable's declaration position.
         */
	public int pos = Position.NOPOS;

	/** The variable's address. Used for different purposes during
	 *  flow analysis, translation and code generation.
	 *  Flow analysis:
	 *    If this is a blank final or local variable, its sequence number.
	 *  Translation:
	 *    If this is a private field, its access number.
         *  Code generation:
         *    If this is a local variable, its logical slot number.
         */
        public int adr = -1;

        /** The variable's constant value, if this is a constant.
	 *  Before the constant value is evaluated, it points to
	 *  an initalizer environment.
         */
	public Object constValue;

	/** Construct a variable symbol, given its flags, name, type and owner.
	 */
	public VarSymbol(long flags, Name name, Type type, Symbol owner) {
	    super(VAR, flags, name, type, owner);
	}

	/** Clone this symbol with new owner.
	 */
	public VarSymbol clone(Symbol newOwner) {
	    VarSymbol v = new VarSymbol(flags_field, name, type, newOwner);
	    v.pos = pos;
	    v.adr = adr;
	    v.constValue = constValue;
//	    System.out.println("clone " + v + " in " + newOwner);//DEBUG
	    return v;
	}

	public static final List<VarSymbol> emptyList = new List<VarSymbol>();

        public String toString() {
	    return name.toString();
	}

        public Symbol asMemberOf(Type site, Types types) {
	    return new VarSymbol(flags_field, name, types.memberType(site, this), owner);
	}
    }

    /** A class for method symbols.
     */
    public static class MethodSymbol extends Symbol {

	/** The code of the method. */
	public Code code = null;

	/** The parameters of the method. */
	public List<VarSymbol> params = null;

	/** For an attribute field accessor, its default value if any.
	 *  The value is null if none appeared in the method
	 *  declaration.
	 */
	public Attribute defaultValue = null;

	/** Construct a method symbol, given its flags, name, type and owner.
	 */
	public MethodSymbol(long flags, Name name, Type type, Symbol owner) {
	    super(MTH, flags, name, type, owner);
	}

	public static final List<MethodSymbol> emptyList =
	    new List<MethodSymbol>();

	/** Clone this symbol with new owner.
	 */
	public MethodSymbol clone(Symbol newOwner) {
	    MethodSymbol m = new MethodSymbol(flags_field, name, type, newOwner);
	    m.code = code;
	    return m;
	}

	/** The Java source which this symbol represents.
	 */
	public String toString() {
	    if ((flags() & BLOCK) != 0) {
		return owner.name.toString();
	    } else {
		String s = (name == name.table.init)
		    ? owner.name.toString()
		    : name.toString();
		if (type != null) {
		    if (type.tag == FORALL)
			s = "<" + ((ForAll)type).typarams() + ">" + s;
		    s += "(" + type.argtypes((flags() & VARARGS) != 0) + ")";
		}
		return s;
	    }
	}

	/** find a symbol that this (proxy method) symbol implements.
	 *  @param    c       The class whose members are searched for
	 *                    implementations
	 */
	public Symbol implemented(TypeSymbol c, Types types) {
	    Symbol impl = null;
	    for (List<Type> is = types.interfaces(c.type);
		 impl == null && is.nonEmpty();
		 is = is.tail) {
		TypeSymbol i = is.head.tsym;
		for (Scope.Entry e = i.members().lookup(name);
		     impl == null && e.scope != null;
		     e = e.next()) {
		    if (this.overrides(e.sym, (TypeSymbol)owner, types, true) &&
			// FIXME: I suspect the following requires a
			// subst() for a parametric return type.
			types.isSameType(type.restype(),
					 types.memberType(owner.type, e.sym).restype())) {
			impl = e.sym;
		    }
		    if (impl == null)
			impl = implemented(i, types);
		}
	    }
	    return impl;
	}

	/** Will the erasure of this method be considered by the VM to
	 *  override the erasure of the other when seen from class `origin'?
	 */
	public boolean binaryOverrides(Symbol _other, TypeSymbol origin, Types types) {
	    if (isConstructor() || _other.kind != MTH) return false;

	    if (this == _other) return true;
	    MethodSymbol other = (MethodSymbol)_other;

	    // check for a direct implementation
	    if (other.isOverridableIn((TypeSymbol)owner) &&
		types.asSuper(owner.type, other.owner) != null &&
		types.isSameType(erasure(types), other.erasure(types)))
		return true;

	    // check for an inherited implementation
	    return
		(flags() & ABSTRACT) == 0 &&
		other.isOverridableIn(origin) &&
		this.isMemberOf(origin, types) &&
		types.isSameType(erasure(types), other.erasure(types));
	}

	/** The implementation of this (abstract) symbol in class origin,
	 *  from the VM's point of view, null if method does not have an
	 *  implementation in class.
	 *  @param origin   The class of which the implementation is a member.
	 */
	public MethodSymbol binaryImplementation(ClassSymbol origin, Types types) {
	    for (TypeSymbol c = origin; c != null; c = types.supertype(c.type).tsym) {
		for (Scope.Entry e = c.members().lookup(name);
		     e.scope != null;
		     e = e.next()) {
		    if (e.sym.kind == MTH &&
			((MethodSymbol)e.sym).binaryOverrides(this, origin, types))
			return (MethodSymbol)e.sym;
		}
	    }
	    return null;
	}

	/** Does this symbol override `other' symbol, when both are seen as
	 *  members of class `origin'?  It is assumed that _other is a member
	 *  of origin.
	 *
	 *  It is assumed that both symbols have the same name.  The static
	 *  modifier is ignored for this test.
	 *
	 *  See JLS 8.4.6.1 (without transitivity) and 8.4.6.4
	 */
	public boolean overrides(Symbol _other, TypeSymbol origin, Types types, boolean checkResult) {
	    if (isConstructor() || _other.kind != MTH) return false;

	    if (this == _other) return true;
	    MethodSymbol other = (MethodSymbol)_other;

	    // check for a direct implementation
	    if (other.isOverridableIn((TypeSymbol)owner) &&
		types.asSuper(owner.type, other.owner) != null) {
		Type mt = types.memberType(owner.type, this);
		Type ot = types.memberType(owner.type, other);
		if (types.isSubSignature(mt, ot)) {
		    if (!checkResult)
			return true;
		    if (types.returnTypeSubstitutable(mt, ot))
			return true;
		}
	    }

	    // check for an inherited implementation
	    if ((flags() & ABSTRACT) != 0 ||
		(other.flags() & ABSTRACT) == 0 ||
		!other.isOverridableIn(origin) ||
		!this.isMemberOf(origin, types))
		return false;

	    // assert types.asSuper(origin.type, other.owner) != null;
	    Type mt = types.memberType(origin.type, this);
	    Type ot = types.memberType(origin.type, other);
	    return
		types.isSubSignature(mt, ot) &&
		(!checkResult || types.resultSubtype(mt, ot, Warner.noWarnings));
	}

	private boolean isOverridableIn(TypeSymbol origin) {
	    // JLS3 8.4.6.1
	    switch ((int)(flags_field & Flags.AccessFlags)) {
	    case Flags.PRIVATE:
		return false;
	    case Flags.PUBLIC:
		return true;
	    case Flags.PROTECTED:
		return (origin.flags() & INTERFACE) == 0;
	    case 0:
		// for package private: can only override in the same
		// package
		return
		    this.packge() == origin.packge() &&
		    (origin.flags() & INTERFACE) == 0;
	    default:
		return false;
	    }
	}

        /** The implementation of this (abstract) symbol in class origin;
         *  null if none exists. Synthetic methods are not considered
	 *  as possible implementations.
         */
	public MethodSymbol implementation(TypeSymbol origin, Types types, boolean checkResult) {
	    for (Type t = origin.type; t.tag == CLASS; t = types.supertype(t)) {
		TypeSymbol c = t.tsym;
		for (Scope.Entry e = c.members().lookup(name);
		     e.scope != null;
		     e = e.next()) {
		    if (e.sym.kind == MTH) {
			MethodSymbol m = (MethodSymbol) e.sym;
			if (m.overrides(this, origin, types, checkResult) &&
			    (m.flags() & SYNTHETIC) == 0)
			    return m;
		    }
		}
	    }
	    // if origin is derived from a raw type, we might have missed
	    // an implementation because we do not know enough about instantiations.
	    // in this case continue with the supertype as origin.
	    if (types.isDerivedRaw(origin.type))
		return implementation(types.supertype(origin.type).tsym, types, checkResult);
	    else
		return null;
	}

        public List<VarSymbol> params() {
            owner.complete();
            if (params == null) {
                ListBuffer<VarSymbol> buf = new ListBuffer<VarSymbol>();
                int i = 0;
                for (Type t : type.argtypes())
                    buf.append(new VarSymbol(PARAMETER,
                                             name.table.fromString("arg" + i),
                                             t, this));
                params = buf.toList();
            }
            return params;
        }

        public Symbol asMemberOf(Type site, Types types) {
	    return new MethodSymbol(flags_field, name, types.memberType(site, this), owner);
	}
    }

    /** A class for predefined operators.
     */
    public static class OperatorSymbol extends MethodSymbol {

        public int opcode;

	public OperatorSymbol(Name name, Type type, int opcode, Symbol owner) {
	    super(PUBLIC | STATIC, name, type, owner);
	    this.opcode = opcode;
	}
    }

    /** Symbol completer interface.
     */
    public static interface Completer {
	void complete(Symbol sym) throws CompletionFailure;
    }

    public static class CompletionFailure extends RuntimeException {
	private static final long serialVersionUID = 0;
	public Symbol sym;

	/** A localized string describing the failure.
	 */
	public String errmsg;

        public CompletionFailure(Symbol sym, String errmsg) {
	    this.sym = sym;
	    this.errmsg = errmsg;
//	    this.printStackTrace();//DEBUG
	}

        public String getMessage() {
	    return errmsg;
	}
    }
}
