/*
 * @(#)AnnotationProxyMaker.java	1.3 04/06/06
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.lang.annotation.*;
import java.lang.reflect.Array;
import java.lang.reflect.Method;
import java.util.*;
import sun.reflect.annotation.*;

import com.sun.mirror.type.TypeMirror;
import com.sun.mirror.type.MirroredTypeException;
import com.sun.mirror.type.MirroredTypesException;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.util.Name;
import com.sun.tools.javac.util.Pair;


/**
 * A generator of dynamic proxy implementations of
 * java.lang.annotation.Annotation.
 *
 * <p> The "dynamic proxy return form" of an attribute element value is
 * the form used by sun.reflect.annotation.AnnotationInvocationHandler.
 */

class AnnotationProxyMaker {

    private final AptEnv env;
    private final Attribute.Compound attrs;
    private final Class<? extends Annotation> annoType;


    private AnnotationProxyMaker(AptEnv env,
				 Attribute.Compound attrs,
				 Class<? extends Annotation> annoType) {
	this.env = env;
	this.attrs = attrs;
	this.annoType = annoType;
    }


    /**
     * Returns a dynamic proxy for an annotation mirror.
     */
    public static <A extends Annotation> A generateAnnotation(
	    AptEnv env, Attribute.Compound attrs, Class<A> annoType) {

	return (A) new AnnotationProxyMaker(env, attrs, annoType)
						.generateAnnotation();
    }


    /**
     * Returns a dynamic proxy for an annotation mirror.
     */
    private Annotation generateAnnotation() {
	return AnnotationParser.annotationForMap(annoType,
						 getAllReflectedValues());
    }

    /**
     * Returns a map from element names to their values in "dynamic
     * proxy return form".  Includes all elements, whether explicit or
     * defaulted.
     */
    private Map<String, Object> getAllReflectedValues() {
	Map<String, Object> res = new LinkedHashMap<String, Object>();

	for (Map.Entry<MethodSymbol, Attribute> entry :
						  getAllValues().entrySet()) {
	    MethodSymbol meth = entry.getKey();
	    Object value = generateValue(meth, entry.getValue());
	    if (value != null) {
		res.put(meth.name.toString(), value);
	    } else {
		// Ignore this element.  May lead to
		// IncompleteAnnotationException somewhere down the line.
	    }
	}
	return res;
    }

    /**
     * Returns a map from element symbols to their values.
     * Includes all elements, whether explicit or defaulted.
     */
    private Map<MethodSymbol, Attribute> getAllValues() {
	Map<MethodSymbol, Attribute> res =
	    new LinkedHashMap<MethodSymbol, Attribute>();

	// First find the default values.
	ClassSymbol sym = (ClassSymbol) attrs.type.tsym;
	for (Scope.Entry e = sym.members().elems; e != null; e = e.sibling) {
	    if (e.sym.kind == Kinds.MTH) {
		MethodSymbol m = (MethodSymbol) e.sym;
		Attribute def = m.defaultValue;
		if (def != null) {
		    res.put(m, def);
		}
	    }
	}
	// Next find the explicit values, possibly overriding defaults.
	for (Pair<MethodSymbol, Attribute> p : attrs.values) {
	    res.put(p.fst, p.snd);
	}
	return res;
    }

    /**
     * Converts an element value to its "dynamic proxy return form".
     * Returns an exception proxy on some errors, but may return null if
     * a useful exception cannot or should not be generated at this point.
     */
    private Object generateValue(MethodSymbol meth, Attribute attr) {
	ValueVisitor vv = new ValueVisitor(meth);
	return vv.getValue(attr);
    }


    private class ValueVisitor implements Attribute.Visitor {

	private MethodSymbol meth;	// annotation element being visited
	private Class<?> runtimeType;	// runtime type of annotation element
	private Object value;		// value in "dynamic proxy return form"

	ValueVisitor(MethodSymbol meth) {
	    this.meth = meth;
	}

	Object getValue(Attribute attr) {
	    Method method;		// runtime method of annotation element
	    try {
		method = annoType.getMethod(meth.name.toString());
	    } catch (NoSuchMethodException e) {
		return null;
	    }
	    runtimeType = method.getReturnType();
	    attr.accept(this);
	    if (!(value instanceof ExceptionProxy) &&
		!AnnotationType.invocationHandlerReturnType(runtimeType)
							.isInstance(value)) {
		typeMismatch(method, attr);
	    }
	    return value;
	}

	
	public void visitConstant(Attribute.Constant c) {
	    value = Constants.decodeConstant(c.value, c.type);
	}

	public void visitClass(Attribute.Class c) {
	    value = new MirroredTypeExceptionProxy(
				env.typeMaker.getType(c.type));
	}

	public void visitArray(Attribute.Array a) {
	    Type elemtype = env.jctypes.elemtype(a.type);

	    if (elemtype.tsym == env.symtab.classType.tsym) {	// Class[]
		// Construct a proxy for a MirroredTypesException
		ArrayList<TypeMirror> elems = new ArrayList<TypeMirror>();
		for (int i = 0; i < a.values.length; i++) {
		    Type elem = ((Attribute.Class) a.values[i]).type;
		    elems.add(env.typeMaker.getType(elem));
		}
		value = new MirroredTypesExceptionProxy(elems);

	    } else {
		int len = a.values.length;
		Class<?> runtimeTypeSaved = runtimeType;
		runtimeType = runtimeType.getComponentType();
		try {
		    Object res = Array.newInstance(runtimeType, len);
		    for (int i = 0; i < len; i++) {
			a.values[i].accept(this);
			if (value == null || value instanceof ExceptionProxy) {
			    return;
			}
			try {
			    Array.set(res, i, value);
			} catch (IllegalArgumentException e) {
			    value = null;	// indicates a type mismatch
			    return;
			}
		    }
		    value = res;
		} finally {
		    runtimeType = runtimeTypeSaved;
		}
	    }
	}

	public void visitEnum(Attribute.Enum e) {
	    try {
		Class<? extends Enum> enumType =
		    runtimeType.asSubclass(Enum.class);
		String constName = e.value.toString();
		try {
		    value = Enum.valueOf(enumType, constName);
		} catch (IllegalArgumentException ex) {
		    value = new EnumConstantNotPresentExceptionProxy(
							enumType, constName);
		}
	    } catch (ClassCastException ex) {
		value = null;	// indicates a type mismatch
	    }
	}

	public void visitCompound(Attribute.Compound c) {
	    try {
		Class<? extends Annotation> nested =
		    runtimeType.asSubclass(Annotation.class);
		value = generateAnnotation(env, c, nested);
	    } catch (ClassCastException ex) {
		value = null;	// indicates a type mismatch
	    }
	}

	public void visitError(Attribute.Error e) {
	    value = null;	// indicates a type mismatch
	}


	/**
	 * Sets "value" to an ExceptionProxy indicating a type mismatch.
	 */
	private void typeMismatch(final Method method, final Attribute attr) {
	    value = new ExceptionProxy() {
		private RuntimeException ex =
		    new AnnotationTypeMismatchException(method,
							attr.type.toString());
		public String toString() {
		    return "<error>";	// eg:  @Anno(value=<error>)
		}
		protected RuntimeException generateException() {
		    return ex;
		}
	    };
	}
    }


    /**
     * ExceptionProxy for MirroredTypeException.
     * The toString, hashCode, and equals methods foward to the underlying
     * type.
     */
    private static class MirroredTypeExceptionProxy extends ExceptionProxy {

	private MirroredTypeException ex;

	MirroredTypeExceptionProxy(TypeMirror t) {
	    ex = new MirroredTypeException(t);
	}

	public String toString() {
	    return ex.getQualifiedName();
	}

	public int hashCode() {
	    TypeMirror t = ex.getTypeMirror();
	    return (t != null)
		    ? t.hashCode()
		    : ex.getQualifiedName().hashCode();
	}

	public boolean equals(Object obj) {
	    TypeMirror t = ex.getTypeMirror();
	    return t != null &&
		   obj instanceof MirroredTypeExceptionProxy &&
		   t.equals(
			((MirroredTypeExceptionProxy) obj).ex.getTypeMirror());
	}

	protected RuntimeException generateException() {
	    return ex;
	}
    }


    /**
     * ExceptionProxy for MirroredTypesException.
     * The toString, hashCode, and equals methods foward to the underlying
     * types.
     */
    private static class MirroredTypesExceptionProxy extends ExceptionProxy {

	private MirroredTypesException ex;

	MirroredTypesExceptionProxy(Collection<TypeMirror> ts) {
	    ex = new MirroredTypesException(ts);
	}

	public String toString() {
	    return ex.getQualifiedNames().toString();
	}

	public int hashCode() {
	    Collection<TypeMirror> ts = ex.getTypeMirrors();
	    return (ts != null)
		    ? ts.hashCode()
		    : ex.getQualifiedNames().hashCode();
	}

	public boolean equals(Object obj) {
	    Collection<TypeMirror> ts = ex.getTypeMirrors();
	    return ts != null &&
		   obj instanceof MirroredTypesExceptionProxy &&
		   ts.equals(
		      ((MirroredTypesExceptionProxy) obj).ex.getTypeMirrors());
	}

	protected RuntimeException generateException() {
	    return ex;
	}
    }
}
