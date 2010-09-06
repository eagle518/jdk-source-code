/*
 * @(#)AptEnv.java	1.1 04/01/26
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror;


import com.sun.tools.apt.mirror.declaration.DeclarationMaker;
import com.sun.tools.apt.mirror.type.TypeMaker;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.CompletionFailure;
import com.sun.tools.javac.comp.Attr;
import com.sun.tools.javac.comp.Enter;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Name;


/**
 * The environment for a run of apt.
 */

public class AptEnv {

    public Name.Table names;		// javac's name table
    public Symtab symtab;		// javac's predefined symbols
    public Types jctypes;		// javac's type utilities
    public Enter enter;			// javac's enter phase
    public Attr attr;			// javac's attr phase (to evaluate
					//   constant initializers)
    public TypeMaker typeMaker;		// apt's internal type utilities
    public DeclarationMaker declMaker;	// apt's internal declaration utilities


    private static final Context.Key<AptEnv> aptEnvKey =
	    new Context.Key<AptEnv>();

    public static AptEnv instance(Context context) {
	AptEnv instance = context.get(aptEnvKey);
	if (instance == null) {
	    instance = new AptEnv(context);
	}
	return instance;
    }

    private AptEnv(Context context) {
	context.put(aptEnvKey, this);

	names = Name.Table.instance(context);
	symtab = Symtab.instance(context);
	jctypes = Types.instance(context);
	enter = Enter.instance(context);
	attr = Attr.instance(context);
	typeMaker = TypeMaker.instance(context);
	declMaker = DeclarationMaker.instance(context);
    }


    /**
     * Does a symbol have a given flag?  Forces symbol completion.
     */
    public static boolean hasFlag(Symbol sym, long flag) {
	return (getFlags(sym) & flag) != 0;
    }

    /**
     * Returns a symbol's flags.  Forces completion.
     */
    public static long getFlags(Symbol sym) {
	complete(sym);
	return sym.flags();
    }

    /**
     * Completes a symbol, ignoring completion failures.
     */
    private static void complete(Symbol sym) {
	while (true) {
	    try {
		sym.complete();
		return;
	    } catch (CompletionFailure e) {
		// Should never see two in a row, but loop just to be sure.
	    }
	}
    }
}
