/**
 * @(#)Attribute.java	1.9 04/05/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.code;

import com.sun.tools.javac.code.Symbol.*;

import com.sun.tools.javac.util.*;

/** An annotation value.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public abstract class Attribute {

    /** The type of the annotation element. */
    public Type type;

    public Attribute(Type type) {
	this.type = type;
    }

    public abstract void accept(Visitor v);

    public static final Attribute[] emptyArray = new Attribute[0];


    /** The value for an annotation element of primitive type or String. */
    public static class Constant extends Attribute {
	public final Object value;
	public void accept(Visitor v) { v.visitConstant(this); }
	public Constant(Type type, Object value) {
	    super(type);
	    this.value = value;
	}
	public String toString() {
	    return value.toString();
	}
    }

    /** The value for an annotation element of type java.lang.Class,
     *  represented as a ClassSymbol.
     */
    public static class Class extends Attribute {
	public final Type type;
	public void accept(Visitor v) { v.visitClass(this); }
	public Class(Types types, Type type) {
	    super(makeClassType(types, type));
	    this.type = type;
	}
        static Type makeClassType(Types types, Type type) {
            Type arg = type.isPrimitive()
                ? types.boxedClass(type).type
                : types.erasure(type);
            return new Type.ClassType(types.syms.classType.outer(),
                                      Type.emptyList.prepend(arg),
                                      types.syms.classType.tsym);
        }
	public String toString() {
	    return type + ".class";
	}
    }

    /** A compound annotation element value, the type of which is an
     *  attribute interface.
     */
    public static class Compound extends Attribute {
	public static final List<Compound> emptyList = new List<Compound>();

	/** The attributes values, as pairs.  Each pair contains a
	 *  reference to the accessing method in the attribute interface
	 *  and the value to be returned when that method is called to
	 *  access this attribute.
	 */
	public final List<Pair<MethodSymbol,Attribute>> values;
	public Compound(Type type,
			List<Pair<MethodSymbol,Attribute>> values) {
	    super(type);
	    this.values = values;
	}
	public void accept(Visitor v) { v.visitCompound(this); }
	public String toString() {
	    StringBuffer buf = new StringBuffer();
	    buf.append("@");
	    buf.append(type.tsym.fullName());
	    buf.append("{");
	    boolean first = true;
	    for (List<Pair<MethodSymbol,Attribute>> v = values;
		 v.nonEmpty(); v = v.tail) {
		Pair<MethodSymbol,Attribute> value = v.head;
		if (!first) buf.append(",");
		first = false;
		buf.append(value.fst.name);
		buf.append("=");
		buf.append(value.snd);
	    }
	    buf.append("}");
	    return buf.toString();
	}

	public Attribute member(Name member) {
	    for (Pair<MethodSymbol,Attribute> pair : values)
		if (pair.fst.name == member) return pair.snd;
	    return null;
	}
    }

    /** The value for an annotation element of an array type.
     */
    public static class Array extends Attribute {
	public final Attribute[] values;
	public Array(Type type, Attribute[] values) {
	    super(type);
	    this.values = values;
	}
	public void accept(Visitor v) { v.visitArray(this); }
	public String toString() {
	    StringBuffer buf = new StringBuffer();
	    buf.append("{");
	    boolean first = true;
	    for (Attribute value : values) {
		if (!first) buf.append(",");
		first = false;
		buf.append(value);
	    }
	    buf.append("}");
	    return buf.toString();
	}
    }

    /** The value for an annotation element of an enum type.
     */
    public static class Enum extends Attribute {
	public VarSymbol value;
	public Enum(Type type, VarSymbol value) {
	    super(type);
	    assert value != null;
	    this.value = value;
	}
	public void accept(Visitor v) { v.visitEnum(this); }
	public String toString() {
	    return value == null ? "null" : value.name.toString();
	}
    }

    public static class Error extends Attribute {
	public Error(Type type) {
	    super(type);
	}
	public void accept(Visitor v) { v.visitError(this); }
	public String toString() {
	    return "<error>";
	}
    }

    /** A visitor type for dynamic dispatch on the kind of attribute value. */
    public static interface Visitor {
	void visitConstant(Attribute.Constant value);
	void visitClass(Attribute.Class clazz);
	void visitCompound(Attribute.Compound compound);
	void visitArray(Attribute.Array array);
	void visitEnum(Attribute.Enum e);
	void visitError(Attribute.Error e);
    }
}
