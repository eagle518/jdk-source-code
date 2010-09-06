/*
 * @(#)TypeMaker.java	1.1 04/01/25
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.type;


import java.util.Collection;
import java.util.ArrayList;

import com.sun.mirror.type.*;
import com.sun.mirror.type.PrimitiveType.Kind;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.util.Context;

import static com.sun.tools.javac.code.TypeTags.*;


/**
 * Utilities for constructing type objects.
 */

public class TypeMaker {

    private final AptEnv env;
    private final VoidType voidType;
    private PrimitiveType[] primTypes = new PrimitiveType[VOID];
						// VOID is past all prim types


    private static final Context.Key<TypeMaker> typeMakerKey =
	    new Context.Key<TypeMaker>();

    public static TypeMaker instance(Context context) {
	TypeMaker instance = context.get(typeMakerKey);
	if (instance == null) {
	    instance = new TypeMaker(context);
	}
	return instance;
    }

    private TypeMaker(Context context) {
	context.put(typeMakerKey, this);
	env = AptEnv.instance(context);

	voidType = new VoidTypeImpl(env);
	primTypes[BOOLEAN] = new PrimitiveTypeImpl(env, Kind.BOOLEAN);
	primTypes[BYTE]    = new PrimitiveTypeImpl(env, Kind.BYTE);
	primTypes[SHORT]   = new PrimitiveTypeImpl(env, Kind.SHORT);
	primTypes[INT]     = new PrimitiveTypeImpl(env, Kind.INT);
	primTypes[LONG]    = new PrimitiveTypeImpl(env, Kind.LONG);
	primTypes[CHAR]    = new PrimitiveTypeImpl(env, Kind.CHAR);
	primTypes[FLOAT]   = new PrimitiveTypeImpl(env, Kind.FLOAT);
	primTypes[DOUBLE]  = new PrimitiveTypeImpl(env, Kind.DOUBLE);
    }


    /**
     * Returns the TypeMirror corresponding to a javac Type object.
     */
    public TypeMirror getType(Type t) {
	if (t.isPrimitive()) {
	    return primTypes[t.tag];
	}
	switch (t.tag) {
	case ERROR:	// fall through
	case CLASS:	return getDeclaredType((Type.ClassType) t);
	case TYPEARG:	return new WildcardTypeImpl(env, (Type.ArgumentType) t);
	case TYPEVAR:	return new TypeVariableImpl(env, (Type.TypeVar) t);
	case ARRAY:	return new ArrayTypeImpl(env, (Type.ArrayType) t);
	case VOID:	return voidType;
	default:	throw new AssertionError();
	}
    }

    /**
     * Returns the declared type corresponding to a given ClassType.
     */
    public DeclaredType getDeclaredType(Type.ClassType t) {
	return
	    hasFlag(t.tsym, Flags.ANNOTATION) ? new AnnotationTypeImpl(env, t) :
	    hasFlag(t.tsym, Flags.INTERFACE)  ? new InterfaceTypeImpl(env, t) :
	    hasFlag(t.tsym, Flags.ENUM)	      ? new EnumTypeImpl(env, t) :
					        new ClassTypeImpl(env, t);
    }

    /**
     * Returns a collection of types corresponding to a list of javac Type
     * objects.
     */
    public Collection<TypeMirror> getTypes(Iterable<Type> types) {
	return getTypes(types, TypeMirror.class);
    }

    /**
     * Returns a collection of types corresponding to a list of javac Type
     * objects.  The element type of the result is specified explicitly.
     */
    public <T extends TypeMirror> Collection<T> getTypes(Iterable<Type> types,
							 Class<T> resType) {
	ArrayList<T> res = new ArrayList<T>();
	for (Type t : types) {
	    TypeMirror mir = getType(t);
	    if (resType.isInstance(mir)) {
		res.add(resType.cast(mir));
	    }
	}
	return res;
    }

    /**
     * Returns the string representation of a type.
     * Bounds of type variables are not included; bounds of wildcard types are.
     * Type names are qualified.
     */
    public String typeToString(Type t) {
	switch (t.tag) {
        case ARRAY:
	    return typeToString(env.jctypes.elemtype(t)) + "[]";
	case CLASS:
	    Type.ClassType c = (Type.ClassType) t;
	    return DeclaredTypeImpl.toString(env, c);
	case TYPEARG:
	    Type.ArgumentType a = (Type.ArgumentType) t;
	    return WildcardTypeImpl.toString(env, a);
	default:
	    return t.tsym.toString();
	}
    }


    /**
     * Does a symbol have a given flag?
     */
    private static boolean hasFlag(Symbol s, long flag) {
	return AptEnv.hasFlag(s, flag);
    }
}
