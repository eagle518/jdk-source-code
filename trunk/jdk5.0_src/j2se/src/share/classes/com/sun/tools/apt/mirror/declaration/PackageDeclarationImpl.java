/*
 * @(#)PackageDeclarationImpl.java	1.3 04/05/19
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.ArrayList;
import java.util.Collection;

import com.sun.mirror.declaration.*;
import com.sun.mirror.util.*;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.*;


/**
 * Implementation of PackageDeclaration.
 */

public class PackageDeclarationImpl extends DeclarationImpl
				    implements PackageDeclaration {

    private PackageSymbol sym;


    public PackageDeclarationImpl(AptEnv env, PackageSymbol sym) {
	super(env, sym);
	this.sym = sym;
    }


    /**
     * Returns the qualified name.
     */
    public String toString() {
	return getQualifiedName();
    }

    /**
     * {@inheritDoc}
     */
    public String getSimpleName() {
	return (sym == env.symtab.emptyPackage)
	    ? ""
	    : sym.name.toString();
    }

    /**
     * {@inheritDoc}
     */
    public String getQualifiedName() {
	return (sym == env.symtab.emptyPackage)
	    ? ""
	    : sym.toString();
    }

    /**
     * {@inheritDoc}
     */
    public Collection<ClassDeclaration> getClasses() {
	return identityFilter.filter(getAllTypes(),
				     ClassDeclaration.class);
    }

    /**
     * {@inheritDoc}
     */
    public Collection<EnumDeclaration> getEnums() {
	return identityFilter.filter(getAllTypes(),
				     EnumDeclaration.class);
    }

    /**
     * {@inheritDoc}
     */
    public Collection<InterfaceDeclaration> getInterfaces() {
	return identityFilter.filter(getAllTypes(),
				     InterfaceDeclaration.class);
    }

    /**
     * {@inheritDoc}
     */
    public Collection<AnnotationTypeDeclaration> getAnnotationTypes() {
	return identityFilter.filter(getAllTypes(),
				     AnnotationTypeDeclaration.class);
    }

    /**
     * {@inheritDoc}
     */
    public void accept(DeclarationVisitor v) {
	v.visitPackageDeclaration(this);
    }


    // Cache of all top-level type declarations in this package.
    private Collection<TypeDeclaration> allTypes = null;

    /**
     * Caches and returns all top-level type declarations in this package.
     * Omits synthetic types.
     */
    private Collection<TypeDeclaration> getAllTypes() {
	if (allTypes != null) {
	    return allTypes;
	}
	allTypes = new ArrayList<TypeDeclaration>();
	for (Symbol s : getMembers(false)) {
	    allTypes.add(env.declMaker.getTypeDeclaration((ClassSymbol) s));
	}
	return allTypes;
    }
}
