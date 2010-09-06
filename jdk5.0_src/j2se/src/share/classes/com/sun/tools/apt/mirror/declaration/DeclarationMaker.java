/*
 * @(#)DeclarationMaker.java	1.6 04/07/13
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.HashMap;
import java.util.Map;

import com.sun.mirror.declaration.*;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Name;


/**
 * Utilities for constructing and caching declarations.
 */

public class DeclarationMaker {

    private AptEnv env;


    private static final Context.Key<DeclarationMaker> declarationMakerKey =
	    new Context.Key<DeclarationMaker>();

    public static DeclarationMaker instance(Context context) {
	DeclarationMaker instance = context.get(declarationMakerKey);
	if (instance == null) {
	    instance = new DeclarationMaker(context);
	}
	return instance;
    }

    private DeclarationMaker(Context context) {
	context.put(declarationMakerKey, this);
	env = AptEnv.instance(context);
    }



    // Cache of package declarations
    private Map<PackageSymbol, PackageDeclaration> packageDecls =
	    new HashMap<PackageSymbol, PackageDeclaration>();

    /**
     * Returns the package declaration for a package symbol.
     */
    public PackageDeclaration getPackageDeclaration(PackageSymbol p) {
	PackageDeclaration res = packageDecls.get(p);
	if (res == null) {
	    res = new PackageDeclarationImpl(env, p);
	    packageDecls.put(p, res);
	}
	return res;
    }

    /**
     * Returns the package declaration for the package with the given name.
     * Name is fully-qualified, or "" for the unnamed package.
     * Returns null if package declaration not found.
     */
    public PackageDeclaration getPackageDeclaration(String name) {
	PackageSymbol p =
	    name.equals("")
		? env.symtab.emptyPackage
		: env.symtab.packages.get(env.names.fromString(name));
	return (p == null)
	    ? null
	    : getPackageDeclaration(p);
    }

    // Cache of type declarations
    private Map<ClassSymbol, TypeDeclaration> typeDecls =
	    new HashMap<ClassSymbol, TypeDeclaration>();

    /**
     * Returns the type declaration for a class symbol.
     * Forces completion, and returns null on error.
     */
    public TypeDeclaration getTypeDeclaration(ClassSymbol c) {
	long flags = AptEnv.getFlags(c);	// forces symbol completion
	if (c.kind == Kinds.ERR) {
	    return null;
	}
	TypeDeclaration res = typeDecls.get(c);
	if (res == null) {
	    if ((flags & Flags.ANNOTATION) != 0) {
		res = new AnnotationTypeDeclarationImpl(env, c);
	    } else if ((flags & Flags.INTERFACE) != 0) {
		res = new InterfaceDeclarationImpl(env, c);
	    } else if ((flags & Flags.ENUM) != 0) {
		res = new EnumDeclarationImpl(env, c);
	    } else {
		res = new ClassDeclarationImpl(env, c);
	    }
	    typeDecls.put(c, res);
	}
	return res;
    }

    /**
     * Returns the type declaration for the type with the given canonical name.
     * Returns null if type declaration not found.
     */
    public TypeDeclaration getTypeDeclaration(String name) {
	ClassSymbol c = nameToClassSymbol(name);
	return (c == null)
	    ? null
	    : getTypeDeclaration(c);
    }

    /**
     * Returns a class symbol given the type's canonical name, or
     * null if symbol isn't found.
     */
    private ClassSymbol nameToClassSymbol(String name) {
	// Class symbols are indexed by flat name.
	// Candidate flat names are generated with successively shorter
	// package qualifiers and longer nested class qualifiers.
	char[] nm = name.toCharArray();
	int dot = nm.length;	// index of successive dots, starting from last

	while (true) {
	    ClassSymbol c = env.symtab.classes.get(
				env.names.fromChars(nm, 0, nm.length));
	    if (c != null) {
		return c;
	    }
	    dot = name.lastIndexOf('.', dot - 1);
	    if (dot >= 0) {
		nm[dot] = '$';
	    } else {
		return null;
	    }
	}
    }

    // Cache of method and constructor declarations
    private Map<MethodSymbol, ExecutableDeclaration> executableDecls =
	    new HashMap<MethodSymbol, ExecutableDeclaration>();

    /**
     * Returns the method or constructor declaration for a method symbol.
     */
    ExecutableDeclaration getExecutableDeclaration(MethodSymbol m) {
	ExecutableDeclaration res = executableDecls.get(m);
	if (res == null) {
	    if (m.isConstructor()) {
		res = new ConstructorDeclarationImpl(env, m);
	    } else if (isAnnotationTypeElement(m)) {
		res = new AnnotationTypeElementDeclarationImpl(env, m);
	    } else {
		res = new MethodDeclarationImpl(env, m);
	    }
	    executableDecls.put(m, res);
	}
	return res;
    }

    // Cache of field declarations
    private Map<VarSymbol, FieldDeclaration> fieldDecls =
	    new HashMap<VarSymbol, FieldDeclaration>();

    /**
     * Returns the field declaration for a var symbol.
     */
    FieldDeclaration getFieldDeclaration(VarSymbol v) {
	FieldDeclaration res = fieldDecls.get(v);
	if (res == null) {
	    if (hasFlag(v, Flags.ENUM)) {
		res = new EnumConstantDeclarationImpl(env, v);
	    } else {
		res = new FieldDeclarationImpl(env, v);
	    }
	    fieldDecls.put(v, res);
	}
	return res;
    }

    /**
     * Returns a parameter declaration.
     */
    ParameterDeclaration getParameterDeclaration(VarSymbol v) {
	return new ParameterDeclarationImpl(env, v);
    }

    /**
     * Returns a type parameter declaration.
     */
    public TypeParameterDeclaration getTypeParameterDeclaration(TypeSymbol t) {
	return new TypeParameterDeclarationImpl(env, t);
    }

    /**
     * Returns an annotation.
     */
    AnnotationMirror getAnnotationMirror(Attribute.Compound a, Declaration decl) {
	return new AnnotationMirrorImpl(env, a, decl);
    }


    /**
     * Is a string a valid Java identifier?
     */
    public static boolean isJavaIdentifier(String id) {
        if (id.length() == 0) {
	    return false;
	}
	int cp = id.codePointAt(0);
        if (!Character.isJavaIdentifierStart(cp)) {
	    return false;
	}
        for (int i = Character.charCount(cp);
		i < id.length();
		i += Character.charCount(cp)) {
	    cp = id.codePointAt(i);
            if (!Character.isJavaIdentifierPart(cp)) {
		return false;
	    }
        }
        return true;
    }


    /**
     * Is a method an annotation type element?
     * It is if it's declared in an annotation type.
     */
    private static boolean isAnnotationTypeElement(MethodSymbol m) {
	return hasFlag(m.enclClass(), Flags.ANNOTATION);
    }

    /**
     * Does a symbol have a given flag?
     */
    private static boolean hasFlag(Symbol s, long flag) {
	return AptEnv.hasFlag(s, flag);
    }
}
