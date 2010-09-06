/**
 * @(#)MemberEnter.java	1.35 04/05/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.comp;

import java.util.*;
import java.util.Set;

import com.sun.tools.javac.code.*;
import com.sun.tools.javac.jvm.*;
import com.sun.tools.javac.tree.*;
import com.sun.tools.javac.util.*;
import com.sun.tools.javac.util.List;

import com.sun.tools.javac.code.Type.*;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.tree.Tree.*;

import static com.sun.tools.javac.code.Flags.*;
import static com.sun.tools.javac.code.Kinds.*;
import static com.sun.tools.javac.code.TypeTags.*;

/** This is the second phase of Enter, in which classes are completed
 *  by entering their members into the class scope using
 *  MemberEnter.complete().  See Enter for an overview.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class MemberEnter extends Tree.Visitor implements Completer {
    protected static final Context.Key<MemberEnter> memberEnterKey =
	new Context.Key<MemberEnter>();

    /** A switch to determine whether we check for package/class conflicts
     */
    final static boolean checkClash = true;

    private final Name.Table names;
    private final Enter enter;
    private final Log log;
    private final Check chk;
    private final Attr attr;
    private final Symtab syms;
    private final TreeMaker make;
    private final ClassReader reader;
    private final Todo todo;
    private final Annotate annotate;
    private final Types types;
    private final Target target;

    private final boolean skipAnnotations;

    public static MemberEnter instance(Context context) {
	MemberEnter instance = context.get(memberEnterKey);
	if (instance == null)
	    instance = new MemberEnter(context);
	return instance;
    }

    protected MemberEnter(Context context) {
	context.put(memberEnterKey, this);
	names = Name.Table.instance(context);
	enter = Enter.instance(context);
	log = Log.instance(context);
	chk = Check.instance(context);
	attr = Attr.instance(context);
	syms = Symtab.instance(context);
	make = TreeMaker.instance(context);
	reader = ClassReader.instance(context);
	todo = Todo.instance(context);
	annotate = Annotate.instance(context);
	types = Types.instance(context);
        target = Target.instance(context);
	skipAnnotations =
	    Options.instance(context).get("skipAnnotations") != null;
    }

    /** A queue for classes whose members still need to be entered into the
     *	symbol table.
     */
    ListBuffer<Env<AttrContext>> halfcompleted = new ListBuffer<Env<AttrContext>>();

    /** Set to true only when the first of a set of classes is
     *  processed from the halfcompleted queue.
     */
    boolean isFirst = true;

    /** A flag to disable completion from time to time during member
     *  enter, as we only need to look up types.  This avoids
     *  unnecessarily deep recursion.
     */
    boolean completionEnabled = true;

    /* ---------- Processing import clauses ----------------
     */

    /** Import all classes of a class or package on demand.
     *  @param pos	     Position to be used for error reporting.
     *  @param tsym	     The class or package the members of which are imported.
     *  @param toScope   The (import) scope in which imported classes
     *		     are entered.
     */
    private void importAll(int pos,
			   final TypeSymbol tsym,
			   Env<AttrContext> env) {
	// Check that packages imported from exist (JLS ???).
	if (tsym.kind == PCK && tsym.members().elems == null && !tsym.exists()) {
	    // If we can't find java.lang, exit immediately.
	    if (((PackageSymbol)tsym).fullname.equals(names.java_lang)) {
		Diagnostic msg = new Diagnostic("fatal.err.no.java.lang");
		throw new FatalError(msg);
	    } else {
		log.error(pos, "doesnt.exist", tsym);
	    }
	}
	final Scope fromScope = tsym.members();
	final Scope toScope = env.toplevel.starImportScope;
	for (Scope.Entry e = fromScope.elems; e != null; e = e.sibling) {
	    if (e.sym.kind == TYP && !toScope.includes(e.sym))
		toScope.enter(e.sym, fromScope);
	}
    }

    /** Import all static members of a class or package on demand.
     *  @param pos	     Position to be used for error reporting.
     *  @param tsym	     The class or package the members of which are imported.
     *  @param toScope   The (import) scope in which imported classes
     *		     are entered.
     */
    private void importStaticAll(int pos,
				 final TypeSymbol tsym,
				 Env<AttrContext> env) {
        final Name sourcefile = env.toplevel.sourcefile;
	final Scope toScope = env.toplevel.starImportScope;
	final PackageSymbol packge = env.toplevel.packge;
        final TypeSymbol origin = tsym;

	// enter imported types immediately
        new Object() {
	    Set<Symbol> processed = new HashSet<Symbol>();
            void importFrom(TypeSymbol tsym) {
	        if (tsym == null || !processed.add(tsym))
                    return;
	    
	        // also import inherited names
	        importFrom(types.supertype(tsym.type).tsym);
	        for (Type t : types.interfaces(tsym.type))
                    importFrom(t.tsym);
	    
                final Scope fromScope = tsym.members();
                for (Scope.Entry e = fromScope.elems; e != null; e = e.sibling) {
                    Symbol sym = e.sym;
                    if (sym.kind == TYP &&
                        (sym.flags() & STATIC) != 0 &&
                        staticImportAccessible(sym, packge) &&
                        sym.isMemberOf(origin, types) &&
                        !toScope.includes(sym))
                        toScope.enter(sym, fromScope);
                }
            }
        }.importFrom(tsym);

	// enter non-types before annotations that might use them
	annotate.earlier(new Annotate.Annotator() {
            Set<Symbol> processed = new HashSet<Symbol>();

	    public String toString() {
	        return "import static " + tsym + ".*" + " in " + sourcefile;
	    }
	    void importFrom(TypeSymbol tsym) {
	        if (tsym == null || !processed.add(tsym))
                    return;
            
	        // also import inherited names
	        importFrom(types.supertype(tsym.type).tsym);
	        for (Type t : types.interfaces(tsym.type))
                    importFrom(t.tsym);
            
                final Scope fromScope = tsym.members();
	        for (Scope.Entry e = fromScope.elems; e != null; e = e.sibling) {
                    Symbol sym = e.sym;
                    if (sym.isStatic() && sym.kind != TYP &&
                        staticImportAccessible(sym, packge) &&
                        !toScope.includes(sym) &&
                        sym.isMemberOf(origin, types)) {
                        toScope.enter(sym, fromScope);
                    }
	        }
            }
	    public void enterAnnotation() {
                importFrom(tsym);
	    }
	});
    }

    // is the sym accessible everywhere in packge?
    boolean staticImportAccessible(Symbol sym, PackageSymbol packge) {
	int flags = (int)(sym.flags() & AccessFlags);
	switch (flags) {
        default:
	case PUBLIC:
	    return true;
	case PRIVATE:
	    return false;
        case 0:
        case PROTECTED:
	    return sym.packge() == packge;
	}
    }

    /** Import statics types of a given name.  Non-types are handled in Attr.
     *  @param pos	     Position to be used for error reporting.
     *  @param tsym	     The class from which the name is imported.
     *  @param name	     The (simple) name being imported.
     *  @param env	     The environment containing the named import
     *			scope to add to.
     */
    private void importNamedStatic(final int pos,
				   final TypeSymbol tsym,
				   final Name name,
				   final Env<AttrContext> env) {
	if (tsym.kind != TYP) {
	    log.error(pos, "static.imp.only.classes.and.interfaces");
	    return;
	}

        final Scope toScope = env.toplevel.namedImportScope;
        final PackageSymbol packge = env.toplevel.packge;
        final TypeSymbol origin = tsym;

        // enter imported types immediately
	new Object() {
	    Set<Symbol> processed = new HashSet<Symbol>();
	    void importFrom(TypeSymbol tsym) {
	        if (tsym == null || !processed.add(tsym))
                    return;
	    
	        // also import inherited names
	        importFrom(types.supertype(tsym.type).tsym);
	        for (Type t : types.interfaces(tsym.type))
                    importFrom(t.tsym);
	    
	        for (Scope.Entry e = tsym.members().lookup(name);
                     e.scope != null;
                     e = e.next()) {
                    Symbol sym = e.sym;
                    if (sym.isStatic() &&
                        sym.kind == TYP &&
                        staticImportAccessible(sym, packge) &&
                        sym.isMemberOf(origin, types) &&
                        chk.checkUniqueImport(pos, sym, toScope))
                        toScope.enter(sym, sym.owner.members());
	        }
            }
	}.importFrom(tsym);

	// enter non-types before annotations that might use them
	annotate.earlier(new Annotate.Annotator() {
            Set<Symbol> processed = new HashSet<Symbol>();
	    boolean found = false;
            
	    public String toString() {
	        return "import static " + tsym + "." + name;
	    }
	    void importFrom(TypeSymbol tsym) {
	        if (tsym == null || !processed.add(tsym))
                    return;
            
	        // also import inherited names
	        importFrom(types.supertype(tsym.type).tsym);
	        for (Type t : types.interfaces(tsym.type))
                    importFrom(t.tsym);
            
	        for (Scope.Entry e = tsym.members().lookup(name);
                     e.scope != null;
                     e = e.next()) {
                    Symbol sym = e.sym;
                    if (sym.isStatic() &&
                        staticImportAccessible(sym, packge) &&
                        sym.isMemberOf(origin, types)) {
                        found = true;
                        if (sym.kind == MTH ||
                            sym.kind != TYP && chk.checkUniqueImport(pos, sym, toScope))
                            toScope.enter(sym, sym.owner.members());
                    }
	        }
	    }
	    public void enterAnnotation() {
	        importFrom(tsym);
	        if (!found) {
                    Name prev = log.useSource(env.toplevel.sourcefile);
                    try {
                        log.error(pos, "cant.resolve.location",
                                  new Diagnostic("kindname.static"),
                                  name, "", "", Resolve.typeKindName(tsym.type),
                                  tsym.type);
                    } finally {
                        log.useSource(prev);
                    }
	        }
	    }
	});
    }
    /** Import given class.
     *  @param pos	     Position to be used for error reporting.
     *  @param tsym	     The class to be imported.
     *  @param env	     The environment containing the named import
     *			scope to add to.
     */
    private void importNamed(int pos, Symbol tsym, Env<AttrContext> env) {
	if (tsym.kind == TYP &&
	    chk.checkUniqueImport(pos, tsym, env.toplevel.namedImportScope))
	    env.toplevel.namedImportScope.enter(tsym, tsym.owner.members());
    }

    /** Construct method type from method signature.
     *  @param typarams    The method's type parameters.
     *  @param params      The method's value parameters.
     *  @param res	       The method's result type,
     *		       null if it is a constructor.
     *  @param thrown      The method's thrown exceptions.
     *  @param env	       The method's (local) environment.
     */
    Type signature(List<TypeParameter> typarams,
		   List<VarDef> params,
		   Tree res,
		   List<Tree> thrown,
		   Env<AttrContext> env) {

	// Enter and attribute type parameters.
	List<Type> tvars = enter.classEnter(typarams, env);
	attr.attribStats(typarams, env);

	// Enter and attribute value parameters.
	ListBuffer<Type> argbuf = new ListBuffer<Type>();
	for (List<VarDef> l = params; l.nonEmpty(); l = l.tail) {
            memberEnter(l.head, env);
	    argbuf.append(l.head.vartype.type);
        }

	// Attribute result type, if one is given.
	Type restype = res == null ? syms.voidType : attr.attribType(res, env);

	// Attribute thrown exceptions.
	ListBuffer<Type> thrownbuf = new ListBuffer<Type>();
	for (List<Tree> l = thrown; l.nonEmpty(); l = l.tail) {
	    Type exc = attr.attribType(l.head, env);
	    if (exc.tag != TYPEVAR)
		exc = chk.checkClassType(l.head.pos, exc);
	    thrownbuf.append(exc);
	}
	Type mtype = new MethodType(argbuf.toList(),
                                    restype,
				    thrownbuf.toList(),
                                    syms.methodClass);
	return tvars.isEmpty() ? mtype : new ForAll(tvars, mtype);
    }

/* ********************************************************************
 * Visitor methods for member enter
 *********************************************************************/

    /** Visitor argument: the current environment
     */
    protected Env<AttrContext> env;

    /** Enter field and method definitions and process import
     *  clauses, catching any completion failure exceptions.
     */
    protected void memberEnter(Tree tree, Env<AttrContext> env) {
	Env<AttrContext> prevEnv = this.env;
	try {
	    this.env = env;
	    tree.accept(this);
	}  catch (CompletionFailure ex) {
	    chk.completionError(tree.pos, ex);
	} finally {
	    this.env = prevEnv;
	}
    }

    /** Enter members from a list of trees.
     */
    void memberEnter(List<? extends Tree> trees, Env<AttrContext> env) {
	for (List<? extends Tree> l = trees; l.nonEmpty(); l = l.tail)
	    memberEnter(l.head, env);
    }

    /** Enter members for a class.
     */
    void finishClass(ClassDef tree, Env<AttrContext> env) {
	if ((tree.mods.flags & Flags.ENUM) != 0 &&
	    (types.supertype(tree.sym.type).tsym.flags() & Flags.ENUM) == 0) {
	    addEnumMembers(tree, env);
	}
	memberEnter(tree.defs, env);
    }

    /** Add the implicit members for an enum type
     *  to the symbol table.
     */
    private void addEnumMembers(ClassDef tree, Env<AttrContext> env) {
        Tree valuesType = make.Type(new ArrayType(tree.sym.type, syms.arrayClass));

        // public static final T[] values() { return ???; }
        Tree values = make.
            MethodDef(make.Modifiers(Flags.PUBLIC|Flags.FINAL|Flags.STATIC),
                      names.values,
                      valuesType,
                      TypeParameter.emptyList,
                      VarDef.emptyList,
                      Tree.emptyList, // thrown
                      null, //make.Block(0, Tree.emptyList.prepend(make.Return(make.Ident(names._null)))),
                      null);
        memberEnter(values, env);

        // public static T valueOf(String name) { return ???; }
        Tree valueOf = make.
            MethodDef(make.Modifiers(Flags.PUBLIC|Flags.STATIC),
                      names.valueOf,
                      make.Type(tree.sym.type),
                      TypeParameter.emptyList,
                      VarDef.emptyList.prepend(make.VarDef(make.Modifiers(0), names.fromString("name"),
                                                           make.Type(syms.stringType), null)),
                      Tree.emptyList, // thrown
                      null, //make.Block(0, Tree.emptyList.prepend(make.Return(make.Ident(names._null)))),
                      null);
        memberEnter(valueOf, env);

        // the remaining members are for bootstrapping only
        if (!target.compilerBootstrap(tree.sym)) return;

        // public final int ordinal() { return ???; }
        MethodDef ordinal = make.at(tree.pos).
            MethodDef(make.Modifiers(Flags.PUBLIC|Flags.FINAL),
                      names.ordinal,
                      make.Type(syms.intType),
                      TypeParameter.emptyList,
                      VarDef.emptyList,
                      Tree.emptyList,
                      null,
                      null);
        memberEnter(ordinal, env);

        // public final String name() { return ???; }
        MethodDef name = make.
            MethodDef(make.Modifiers(Flags.PUBLIC|Flags.FINAL),
                      names._name,
                      make.Type(syms.stringType),
                      TypeParameter.emptyList,
                      VarDef.emptyList,
                      Tree.emptyList,
                      null,
                      null);
        memberEnter(name, env);

        // public int compareTo(E other) { return ???; }
        MethodSymbol compareTo = new
            MethodSymbol(Flags.PUBLIC,
                         names.compareTo,
                         new MethodType(Type.emptyList.prepend(tree.sym.type),
                                        syms.intType,
                                        Type.emptyList,
                                        syms.methodClass),
                         tree.sym);
        memberEnter(make.MethodDef(compareTo, null), env);
    }

    public void visitTopLevel(TopLevel tree) {
	if (tree.starImportScope.elems != null) {
	    // we must have already processed this toplevel
	    return;
	}

	// check that no class exists with same fully qualified name as
	// toplevel package
	if (checkClash && tree.pid != null) {
	    Symbol p = tree.packge;
	    while (p.owner != syms.rootPackage) {
		p.owner.complete(); // enter all class members of p
		if (syms.classes.get(p.fullName()) != null) {
		    log.error(tree.pos,
			      "pkg.clashes.with.class.of.same.name",
			      p);
		}
		p = p.owner;
	    }
	}

	// process package annotations
	annotateLater(tree.packageAnnotations, env, tree.packge);

	// Import-on-demand java.lang.
	importAll(tree.pos, reader.enterPackage(names.java_lang), env);

	// Process all import clauses.
	memberEnter(tree.defs, env);
    }

    // process the non-static imports and the static imports of types.
    public void visitImport(Import tree) {
	Tree imp = tree.qualid;
	Name name = TreeInfo.name(imp);
	TypeSymbol p;

	// Create a local environment pointing to this tree to disable
	// effects of other imports in Resolve.findGlobalType
	Env<AttrContext> localEnv = env.dup(tree);

	// Attribute qualifying package or class.
	Select s = (Select) imp;
	p = attr.
	    attribTree(s.selected,
		       localEnv,
		       tree.staticImport ? TYP : (TYP | PCK),
		       Type.noType).tsym;
	if (name == names.asterisk) {
	    // Import on demand.
	    chk.checkCanonical(s.selected);
	    if (tree.staticImport)
		importStaticAll(tree.pos, p, env);
	    else
		importAll(tree.pos, p, env);
	} else {
	    // Named type import.
	    if (tree.staticImport) {
		importNamedStatic(tree.pos, p, name, localEnv);
		chk.checkCanonical(s.selected);
	    } else {
		TypeSymbol c = attribImportType(imp, localEnv).tsym;
		chk.checkCanonical(imp);
		importNamed(tree.pos, c, env);
	    }
	}
    }

    public void visitMethodDef(MethodDef tree) {
	Scope enclScope = enter.enterScope(env);
	MethodSymbol m = new MethodSymbol(0, tree.name, null, enclScope.owner);
	m.flags_field = chk.checkFlags(tree.pos, tree.mods.flags, m, tree);
	tree.sym = m;
	Env<AttrContext> localEnv = methodEnv(tree, env);

        // Compute the method type
	m.type = signature(tree.typarams, tree.params,
                           tree.restype, tree.thrown,
                           localEnv);

	// Set m.params
	ListBuffer<VarSymbol> params = new ListBuffer<VarSymbol>();
	VarDef lastParam = null;
	for (List<VarDef> l = tree.params; l.nonEmpty(); l = l.tail) {
	    VarDef param = lastParam = l.head;
            assert param.sym != null;
	    params.append(param.sym);
	}
	m.params = params.toList();

	// mark the method varargs, if necessary
	if (lastParam != null && (lastParam.mods.flags & Flags.VARARGS) != 0)
	    m.flags_field |= Flags.VARARGS;

	localEnv.info.scope.leave();
	if (chk.checkUnique(tree.pos, m, enclScope)) {
	    enclScope.enter(m);
	}
	annotateLater(tree.mods.annotations, localEnv, m);
	if (tree.defaultValue != null)
	    annotateDefaultValueLater(tree.defaultValue, localEnv, m);
    }

    /** Create a fresh environment for method bodies.
     *	@param tree	The method definition.
     *	@param env	The environment current outside of the method definition.
     */
    Env<AttrContext> methodEnv(MethodDef tree, Env<AttrContext> env) {
	Env<AttrContext> localEnv =
	    env.dup(tree, env.info.dup(env.info.scope.dupUnshared()));
	localEnv.enclMethod = tree;
	localEnv.info.scope.owner = tree.sym;
	if ((tree.mods.flags & STATIC) != 0) localEnv.info.staticLevel++;
	return localEnv;
    }

    public void visitVarDef(VarDef tree) {
	Env<AttrContext> localEnv = env;
	if ((tree.mods.flags & STATIC) != 0 ||
	    (env.info.scope.owner.flags() & INTERFACE) != 0) {
	    localEnv = env.dup(tree, env.info.dup());
	    localEnv.info.staticLevel++;
	}
	attr.attribType(tree.vartype, localEnv);
	Scope enclScope = enter.enterScope(env);
	VarSymbol v =
	    new VarSymbol(0, tree.name, tree.vartype.type, enclScope.owner);
	v.flags_field = chk.checkFlags(tree.pos, tree.mods.flags, v, tree);
	tree.sym = v;
	if (tree.init != null) {
	    v.flags_field |= HASINIT;
	    if ((v.flags_field & FINAL) != 0 && tree.init.tag != Tree.NEWCLASS)
		v.constValue = initEnv(tree, env);
	}
	if (chk.checkUnique(tree.pos, v, enclScope)) {
	    chk.checkTransparentVar(tree.pos, v, enclScope);
	    enclScope.enter(v);
	}
	annotateLater(tree.mods.annotations, localEnv, v);
	v.pos = tree.pos;
    }

    /** Create a fresh environment for a variable's initializer.
     *	If the variable is a field, the owner of the environment's scope
     *	is be the variable itself, otherwise the owner is the method
     *	enclosing the variable definition.
     *
     *	@param tree	The variable definition.
     *	@param env	The environment current outside of the variable definition.
     */
    Env<AttrContext> initEnv(VarDef tree, Env<AttrContext> env) {
	Env<AttrContext> localEnv = env.dupto(new AttrContextEnv(tree, env.info.dup()));
	if (tree.sym.owner.kind == TYP) {
	    localEnv.info.scope = new Scope.DelegatedScope(env.info.scope);
	    localEnv.info.scope.owner = tree.sym;
	}
	if ((tree.mods.flags & STATIC) != 0 ||
	    (env.enclClass.sym.flags() & INTERFACE) != 0)
	    localEnv.info.staticLevel++;
	return localEnv;
    }

    /** Default member enter visitor method: do nothing
     */
    public void visitTree(Tree tree) {
    }

/* ********************************************************************
 * Type completion
 *********************************************************************/

    Type attribImportType(Tree tree, Env<AttrContext> env) {
	assert completionEnabled;
	try {
	    // To prevent deep recursion, suppress completion of some
	    // types.
	    completionEnabled = false;
	    return attr.attribType(tree, env);
	} finally {
	    completionEnabled = true;
	}
    }

/* ********************************************************************
 * Annotation processing
 *********************************************************************/

    /** Queue annotations for later processing. */
    void annotateLater(final List<Annotation> annotations,
		       final Env<AttrContext> localEnv,
		       final Symbol s) {
	if (annotations.isEmpty()) return;
	if (s.kind != PCK) s.attributes_field = null; // mark it incomplete for now
	annotate.later(new Annotate.Annotator() {
		public String toString() {
		    return "annotate " + annotations + " onto " + s + " in " + s.owner;
		}
		public void enterAnnotation() {
		    assert s.kind == PCK || s.attributes_field == null;
		    Name prev = log.useSource(localEnv.toplevel.sourcefile);
		    try {
			if (s.attributes_field != null &&
			    s.attributes_field.nonEmpty() &&
			    annotations.nonEmpty())
			    log.error(annotations.head.pos, 
                                      "already.annotated",
                                      Resolve.kindName(s.kind), s);
			enterAnnotations(annotations, localEnv, s);
		    } finally {
			log.useSource(prev);
		    }
		}
	    });
    }


    /** Enter a set of annotations. */
    private void enterAnnotations(List<Annotation> annotations,
			  Env<AttrContext> env,
			  Symbol s) {
	ListBuffer<Attribute.Compound> buf =
	    new ListBuffer<Attribute.Compound>();
	Set<TypeSymbol> annotated = new HashSet<TypeSymbol>();
	if (!skipAnnotations)
	for (List<Annotation> al = annotations; al.nonEmpty(); al = al.tail) {
	    Annotation a = al.head;
	    Attribute.Compound c = annotate.enterAnnotation(a,
                                                            syms.annotationType,
                                                            env);
	    if (c == null) continue;
	    buf.append(c);
	    if (types.isSameType(c.type, syms.deprecatedType))
		s.flags_field |= Flags.DEPRECATED;
	    if (!annotated.add(a.type.tsym))
		log.error(a.pos, "duplicate.annotation");
	}
	s.attributes_field = buf.toList();
    }

    /** Queue processing of an attribute default value. */
    void annotateDefaultValueLater(final Tree defaultValue,
				   final Env<AttrContext> localEnv,
				   final MethodSymbol m) {
	annotate.later(new Annotate.Annotator() {
		public String toString() {
		    return "annotate " + m.owner + "." +
			m + " default " + defaultValue;
		}
		public void enterAnnotation() {
		    Name prev = log.useSource(localEnv.toplevel.sourcefile);
		    try {
			enterDefaultValue(defaultValue, localEnv, m);
		    } finally {
			log.useSource(prev);
		    }
		}
	    });
    }

    /** Enter a default value for an attribute method. */
    private void enterDefaultValue(final Tree defaultValue,
				   final Env<AttrContext> localEnv,
				   final MethodSymbol m) {
	m.defaultValue = annotate.enterAttributeValue(m.type.restype(),
						      defaultValue,
						      localEnv);
    }

/* ********************************************************************
 * Source completer
 *********************************************************************/

    /** Complete entering a class.
     *  @param sym	   The symbol of the class to be completed.
     */
    public void complete(Symbol sym) throws CompletionFailure {
	// Suppress some (recursive) MemberEnter invocations
	if (!completionEnabled) {
	    // Re-install same completer for next time around and return.
	    assert (sym.flags() & Flags.COMPOUND) == 0;
	    sym.completer = this;
	    return;
	}

	ClassSymbol c = (ClassSymbol)sym;
	ClassType ct = (ClassType)c.type;
	Env<AttrContext> env = enter.typeEnvs.get(c);
	ClassDef tree = (ClassDef)env.tree;
	boolean wasFirst = isFirst;
	isFirst = false;

	Name prev = log.useSource(env.toplevel.sourcefile);
	try {
	    // Save class environment for later member enter (2) processing.
	    halfcompleted.append(env);

	    // If this is a toplevel-class, make sure any preceding import
	    // clauses have been seen.
	    if (c.owner.kind == PCK) {
		memberEnter(env.toplevel, env.enclosing(Tree.TOPLEVEL));
		todo.append(env);
	    }

	    // Mark class as not yet attributed.
	    c.flags_field |= UNATTRIBUTED;

	    if (c.owner.kind == TYP)
		c.owner.complete();

	    // create an environment for evaluating the base clauses
	    Env<AttrContext> baseEnv = baseEnv(tree, env);

	    // Determine supertype.
	    Type supertype =
		(tree.extending != null)
		? attr.attribBase(tree.extending, baseEnv, true, false, true)
		: ((tree.mods.flags & Flags.ENUM) != 0 && !target.compilerBootstrap(c))
		? attr.attribBase(enumBase(tree.pos, c), baseEnv,
                                  true, false, false)
		: (c.fullname == names.java_lang_Object)
		? Type.noType
		: syms.objectType;

	    // Determine interfaces.
	    ListBuffer<Type> interfaces = new ListBuffer<Type>();
	    Set<Type> interfaceSet = new HashSet<Type>();
            List<Tree> interfaceTrees = tree.implementing;
            if ((tree.mods.flags & Flags.ENUM) != 0 && target.compilerBootstrap(c)) {
                // add interface Comparable<T>
                interfaceTrees =
                    interfaceTrees.prepend(make.Type(new ClassType(syms.comparableType.outer(),
                                                                   Type.emptyList.prepend(c.type),
                                                                   syms.comparableType.tsym)));
                // add interface Serializable
                interfaceTrees =
                    interfaceTrees.prepend(make.Type(syms.serializableType));
            }
	    for (List<Tree> l = interfaceTrees; l.nonEmpty(); l = l.tail) {
		Type i = attr.attribBase(l.head, baseEnv, false, true, true);
		if (i.tag == CLASS) {
		    interfaces.append(i);
		    chk.checkNotRepeated(l.head.pos, i, interfaceSet);
		}
	    }
	    ct.supertype_field = supertype;
	    if ((c.flags_field & ANNOTATION) != 0)
		ct.interfaces_field = Type.emptyList.prepend(syms.annotationType);
	    else
		ct.interfaces_field = interfaces.toList();

	    if (c.fullname == names.java_lang_Object) {
		if (tree.extending != null) {
		    chk.checkNonCyclic(tree.extending.pos,
				       supertype);
		    ct.supertype_field = Type.noType;
		}
		else if (tree.implementing.nonEmpty()) {
		    chk.checkNonCyclic(tree.implementing.head.pos,
				       ct.interfaces_field.head);
		    ct.interfaces_field = Type.emptyList;
		}
	    }

	    // Annotations
	    annotateLater(tree.mods.annotations, baseEnv, c);

	    // Attribute type parameters
	    attr.attribStats(tree.typarams, baseEnv);

	    chk.checkNonCyclic(tree.pos, c.type);

	    // Add default constructor if needed.
	    if ((c.flags() & INTERFACE) == 0 &&
		!TreeInfo.hasConstructors(tree.defs)) {
		List<Type> argtypes = Type.emptyList;
		List<Type> typarams = Type.emptyList;
		List<Type> thrown = Type.emptyList;
                long ctorFlags = 0;
		boolean based = false;
		if (c.name.len == 0) {
		    NewClass nc = (NewClass)env.next.tree;
		    if (nc.constructor != null) {
			Type superConstrType = types.memberType(c.type,
								nc.constructor);
			argtypes = superConstrType.argtypes();
			typarams = superConstrType.typarams();
                        ctorFlags = nc.constructor.flags() & VARARGS;
			if (nc.encl != null) {
			    argtypes = argtypes.prepend(nc.encl.type);
			    based = true;
			}
			thrown = superConstrType.thrown();
		    }
		}
		Tree constrDef = DefaultConstructor(make.at(tree.pos), c,
						    typarams, argtypes, thrown,
						    ctorFlags, based);
		tree.defs = tree.defs.prepend(constrDef);
	    }

	    // If this is a class, enter symbols for this and super into
	    // current scope.
	    if ((c.flags_field & INTERFACE) == 0) {
		VarSymbol thisSym =
		    new VarSymbol(FINAL | HASINIT, names._this, c.type, c);
		thisSym.pos = Position.FIRSTPOS;
		env.info.scope.enter(thisSym);
		if (ct.supertype_field.tag == CLASS) {
		    VarSymbol superSym =
			new VarSymbol(FINAL | HASINIT, names._super,
				      ct.supertype_field, c);
		    superSym.pos = Position.FIRSTPOS;
		    env.info.scope.enter(superSym);
		}
	    }

	    // Check that a generic class doesn't extend Throwable
	    if (tree.typarams.nonEmpty() && types.isSubType(supertype, syms.throwableType))
		log.error(tree.extending.pos, "generic.throwable");

	    // check that no package exists with same fully qualified name,
	    // but admit classes in the empty package which have the same
	    // name as a top-level package.
	    if (checkClash &&
		c.owner.kind == PCK && c.owner != syms.emptyPackage &&
		reader.packageExists(c.fullname))
		{
		    log.error(tree.pos, "clash.with.pkg.of.same.name", c);
		}

	} catch (CompletionFailure ex) {
	    chk.completionError(tree.pos, ex);
	} finally {
	    log.useSource(prev);
	}

	// Enter all member fields and methods of a set of half completed
	// classes in a second phase.
	if (wasFirst) {
	    try {
		while (halfcompleted.nonEmpty()) {
		    finish(halfcompleted.next());
		}
	    } finally {
		isFirst = true;
	    }	

	    // commit pending annotations
	    annotate.flush();
	}
    }

    private Env<AttrContext> baseEnv(ClassDef tree, Env<AttrContext> env) {
	Scope typaramScope = new Scope(tree.sym);
	if (tree.typarams != null)
	    for (List<TypeParameter> typarams = tree.typarams;
		 typarams.nonEmpty();
		 typarams = typarams.tail)
		typaramScope.enter(typarams.head.type.tsym);
	Env<AttrContext> outer = env.outer; // the base clause can't see members of this class
	Env<AttrContext> localEnv = outer.dup(tree, outer.info.dup(typaramScope));
        localEnv.baseClause = true;
	localEnv.outer = outer;
	localEnv.info.isSelfCall = false;
	return localEnv;
    }

    /** Enter member fields and methods of a class
     *  @param env	  the environment current for the class block.
     */
    private void finish(Env<AttrContext> env) {
	Name prev = log.useSource(env.toplevel.sourcefile);
	try {
	    ClassDef tree = (ClassDef)env.tree;
	    finishClass(tree, env);
	} finally {
	    log.useSource(prev);
	}
    }

    /** Generate a base clause for an enum type.
     *  @param pos		The position for trees and diagnostics, if any
     *  @param c		The class symbol of the enum
     */
    private Tree enumBase(int pos, ClassSymbol c) {
	Tree result = make.at(pos).
	    TypeApply(make.QualIdent(syms.enumSym),
		      Tree.emptyList.prepend(make.Type(c.type)));
	return result;
    }

/* ***************************************************************************
 * tree building
 ****************************************************************************/

    /** Generate default constructor for given class. For classes different
     *	from java.lang.Object, this is:
     *
     *	  c(argtype_0 x_0, ..., argtype_n x_n) throws thrown {
     *	    super(x_0, ..., x_n)
     *	  }
     *
     *	or, if based == true:
     *
     *	  c(argtype_0 x_0, ..., argtype_n x_n) throws thrown {
     *	    x_0.super(x_1, ..., x_n)
     *	  }
     *
     *	@param make	The tree factory.
     *	@param c	The class owning the default constructor.
     *	@param argtypes The parameter types of the constructor.
     *	@param thrown	The thrown exceptions of the constructor.
     *	@param based	Is first parameter a this$n?
     */
    Tree DefaultConstructor(TreeMaker make,
			    ClassSymbol c,
			    List<Type> typarams,
			    List<Type> argtypes,
			    List<Type> thrown,
                            long flags,
			    boolean based) {
	List<VarDef> params = make.Params(argtypes, syms.noSymbol);
	List<Tree> stats = Tree.emptyList;
	if (c.type != syms.objectType)
	    stats = stats.prepend(SuperCall(make, typarams, params, based));
	if ((c.flags() & ENUM) != 0 && 
	    types.supertype(c.type).tsym == syms.enumSym) {
	    // constructors of true enums are private
	    flags = (flags & ~AccessFlags) | PRIVATE | GENERATEDCONSTR;
	} else
	    flags |= (c.flags() & AccessFlags) | GENERATEDCONSTR;
	if (c.name.len == 0) flags |= ANONCONSTR;
	Tree result = make.MethodDef(
	    make.Modifiers(flags),
	    names.init,
	    null,
	    make.TypeParams(typarams),
	    params,
	    make.Types(thrown),
	    make.Block(0, stats),
	    null);
	return result;
    }

    /** Generate call to superclass constructor. This is:
     *
     *	  super(id_0, ..., id_n)
     *
     * or, if based == true
     *
     *	  id_0.super(id_1,...,id_n)
     *
     *	where id_0, ..., id_n are the names of the given parameters.
     *
     *	@param make    The tree factory
     *	@param params  The parameters that need to be passed to super
     *	@param typarams  The type parameters that need to be passed to super
     *	@param based   Is first parameter a this$n?
     */
    Tree SuperCall(TreeMaker make,
		   List<Type> typarams,
		   List<VarDef> params,
		   boolean based) {
	Tree meth;
	if (based) {
	    meth = make.Select(make.Ident(params.head), names._super);
	    params = params.tail;
	} else {
	    meth = make.Ident(names._super);
	}
	List<Tree> typeargs = typarams.nonEmpty() ? make.Types(typarams) : null;
	return make.Exec(make.Apply(typeargs, meth, make.Idents(params)));
    }
}
