/**
 * @(#)ExecutableMemberDocImpl.java	1.39 04/05/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import com.sun.javadoc.*;

import com.sun.tools.javac.util.List;
import com.sun.tools.javac.util.ListBuffer;
import com.sun.tools.javac.code.Flags;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.tree.Tree.MethodDef;

import java.text.CollationKey;

import java.lang.reflect.Modifier;

/**
 * Represents a method or constructor of a java class.
 *
 * @since JDK1.2
 * @author Robert Field
 * @author Neal Gafter (rewrite)
 * @author Scott Seligman (generics, annotations)
 */

public abstract class ExecutableMemberDocImpl 
	extends MemberDocImpl implements ExecutableMemberDoc {

    protected final MethodSymbol sym;

    /**
     * Constructor.
     */
    public ExecutableMemberDocImpl(DocEnv env, MethodSymbol sym,
				   String rawDocs, MethodDef tree) {
        super(env, sym, rawDocs, tree);
	this.sym = sym;
    }

    /**
     * Constructor.
     */
    public ExecutableMemberDocImpl(DocEnv env, MethodSymbol sym) {
	this(env, sym, null, null);
    }

    /**
     * Returns the flags in terms of javac's flags
     */
    protected long getFlags() {
	return sym.flags();
    }

    /**
     * Identify the containing class
     */
    protected ClassSymbol getContainingClass() {
	return sym.enclClass();
    }

    /** 
     * Return true if this method is native 
     */
    public boolean isNative() {
        return Modifier.isNative(getModifiers());
    }

    /** 
     * Return true if this method is synchronized 
     */
    public boolean isSynchronized() {
        return Modifier.isSynchronized(getModifiers());
    }

    /**
     * Return true if this method was declared to take a variable number
     * of arguments.
     */
    public boolean isVarArgs() {
        return ((sym.flags() & Flags.VARARGS) != 0
		&& !env.legacyDoclet);
    }

    /** 
     * Returns true if this field was synthesized by the compiler.
     */
    public boolean isSynthetic() {
        return ((sym.flags() & Flags.SYNTHETIC) != 0);
    }

    public boolean isIncluded() {
	return containingClass().isIncluded() && env.shouldDocument(sym);
    }

    /**
     * Return the throws tags in this method.
     * 
     * @return an array of ThrowTagImpl containing all {@code @exception}
     * and {@code @throws} tags.
     */
    public ThrowsTag[] throwsTags() {
        return comment().throwsTags();
    }

    /**
     * Return the param tags in this method, excluding the type
     * parameter tags.
     * 
     * @return an array of ParamTagImpl containing all {@code @param} tags.
     */
    public ParamTag[] paramTags() {
        return comment().paramTags();
    }

    /**
     * Return the type parameter tags in this method.
     */
    public ParamTag[] typeParamTags() {
	return env.legacyDoclet
	    ? new ParamTag[0]
	    : comment().typeParamTags();
    }

    /**
     * Return exceptions this method or constructor throws. 
     *
     * @return an array of ClassDoc[] representing the exceptions
     * thrown by this method.
     */
    public ClassDoc[] thrownExceptions() {
        ListBuffer<ClassDocImpl> l = new ListBuffer<ClassDocImpl>();
	for (Type ex : sym.type.thrown()) {
	    ex = env.types.erasure(ex);
	    //### Will these casts succeed in the face of static semantic
	    //### errors in the documented code?
	    ClassDocImpl cdi = env.getClassDoc((ClassSymbol)ex.tsym);
	    if (cdi != null) l.append(cdi);
	}
	return l.toArray(new ClassDocImpl[l.length()]);
    }

    /**
     * Return exceptions this method or constructor throws.
     * Each array element is either a <code>ClassDoc</code> or a
     * <code>TypeVariable</code>.
     */
    public com.sun.javadoc.Type[] thrownExceptionTypes() {
	return TypeMaker.getTypes(env, sym.type.thrown());
    }

    /**
     * Get argument information.
     *
     * @see ParameterImpl
     *
     * @return an array of ParameterImpl, one element per argument
     * in the order the arguments are present.
     */
    public Parameter[] parameters() {
	// generate the parameters on the fly:  they're not cached
	List<VarSymbol> params = sym.params();
	Parameter result[] = new Parameter[params.length()];

	int i = 0;
	for (VarSymbol param : params) {
	    result[i++] = new ParameterImpl(env, param);
	}
	return result;
    }

    /**
     * Return the formal type parameters of this method or constructor.
     * Return an empty array if there are none.
     */
    public TypeVariable[] typeParameters() {
	if (env.legacyDoclet) {
	    return new TypeVariable[0];
	}
	TypeVariable res[] = new TypeVariable[sym.type.typarams().length()];
	TypeMaker.getTypes(env, sym.type.typarams(), res);
	return res;
    }

    /**
     * Get the signature. It is the parameter list, type is qualified.  
     * For instance, for a method <code>mymethod(String x, int y)</code>, 
     * it will return <code>(java.lang.String,int)</code>.
     */
    public String signature() {
        return makeSignature(true);
    }

    /**
     * Get flat signature.  All types are not qualified.
     * Return a String, which is the flat signiture of this member.
     * It is the parameter list, type is not qualified.  
     * For instance, for a method <code>mymethod(String x, int y)</code>, 
     * it will return <code>(String, int)</code>. 
     */
    public String flatSignature() {
        return makeSignature(false);
    }

    private String makeSignature(boolean full) {
	StringBuffer result = new StringBuffer();
	result.append("(");
	for (List<Type> types = sym.type.argtypes(); types.nonEmpty(); ) {
	    Type t = (Type)types.head;
	    result.append(TypeMaker.getTypeString(env, t, full));
	    types = types.tail;
	    if (types.nonEmpty()) {
	        result.append(", ");
	    }
	}
	if (isVarArgs()) {
	    int len = result.length();
	    result.replace(len - 2, len, "...");
	}
	result.append(")");
	return result.toString();
    }
    
    protected String typeParametersString() {
	return TypeMaker.typeParametersString(env, sym, true);
    }

    /**
     * Generate a key for sorting.
     */
    CollationKey generateKey() {
	String k = name() + flatSignature() + typeParametersString();
	// ',' and '&' are between '$' and 'a':  normalize to spaces.
	k = k.replace(',', ' ').replace('&', ' ');
	// System.out.println("COLLATION KEY FOR " + this + " is \"" + k + "\"");
        return env.doclocale.collator.getCollationKey(k);
    }

    /**
     * Return the source position of the entity, or null if 
     * no position is available.
     */
    public SourcePosition position() { 
	if (sym.enclClass().sourcefile == null) return null;
	return SourcePositionImpl.make(sym.enclClass().sourcefile.toString(), 
				       (tree==null) ? 0 : tree.pos);
    }
}
