/**
 * @(#)TreeMaker.java	1.54 04/06/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.tree;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.code.*;

import com.sun.tools.javac.tree.Tree.*;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.code.Type.*;

import static com.sun.tools.javac.code.Flags.*;
import static com.sun.tools.javac.code.Kinds.*;
import static com.sun.tools.javac.code.TypeTags.*;

/** Factory class for trees.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class TreeMaker implements Tree.Factory {

    /** The context key for the tree factory. */
    protected static final Context.Key<TreeMaker> treeMakerKey =
	new Context.Key<TreeMaker>();

    /** Get the TreeMaker instance. */
    public static TreeMaker instance(Context context) {
	TreeMaker instance = context.get(treeMakerKey);
	if (instance == null)
	    instance = new TreeMaker(context);
	return instance;
    }

    /** The position at which subsequent trees will be created.
     */
    public int pos = Position.NOPOS;

    /** The toplevel tree to which created trees belong.
     */
    public TopLevel toplevel;

    /** The current name table. */
    Name.Table names;

    Types types;

    /** The current symbol table. */
    Symtab syms;

    /** Create a tree maker with null toplevel and NOPOS as initial position.
     */
    protected TreeMaker(Context context) {
	context.put(treeMakerKey, this);
	this.pos = Position.NOPOS;
	this.toplevel = null;
	this.names = Name.Table.instance(context);
	this.syms = Symtab.instance(context);
	this.types = Types.instance(context);
    }

    /** Create a tree maker with a given toplevel and FIRSTPOS as initial position.
     */
    TreeMaker(TopLevel toplevel, Types types, Symtab syms) {
	this.pos = Position.FIRSTPOS;
	this.toplevel = toplevel;
	this.names = toplevel.sourcefile.table;
	this.types = types;
	this.syms = syms;
    }

    /** Create a new tree maker for a given toplevel.
     */
    public TreeMaker forToplevel(TopLevel toplevel) {
        return new TreeMaker(toplevel, types, syms);
    }

    /** Reassign current position.
     */
    public TreeMaker at(int pos) {
	this.pos = pos;
	return this;
    }

    /** Create given tree node at current position.
     */
    public TopLevel TopLevel(List<Annotation> packageAnnotations,
			     Tree pid, List<Tree> defs) {
	assert packageAnnotations != null;
        TopLevel tree = new TopLevel(packageAnnotations, pid, defs,
				     null, null, null, null);
        tree.pos = pos;
	return tree;
    }

    public Import Import(Tree qualid, boolean importStatic) {
        Import tree = new Import(qualid, importStatic);
        tree.pos = pos;
	return tree;
    }

    public ClassDef ClassDef(Modifiers mods, Name name, List<TypeParameter> typarams, Tree extending, List<Tree> implementing, List<Tree> defs) {
        ClassDef tree = new ClassDef(mods, name, typarams, extending, implementing, defs, null);
        tree.pos = pos;
	return tree;
    }

    public MethodDef MethodDef(Modifiers mods, Name name, Tree restype, List<TypeParameter> typarams,
			       List<VarDef> params, List<Tree> thrown, Block body, Tree defaultValue) {
        MethodDef tree = new MethodDef(mods, name, restype, typarams, params, thrown, body, defaultValue, null);
        tree.pos = pos;
	return tree;
    }

    public VarDef VarDef(Modifiers mods, Name name, Tree vartype, Tree init) {
        VarDef tree = new VarDef(mods, name, vartype, init, null);
        tree.pos = pos;
	return tree;
    }

    public Skip Skip() {
        Skip tree = new Skip();
        tree.pos = pos;
	return tree;
    }

    public Block Block(long flags, List<Tree> stats) {
        Block tree = new Block(flags, stats);
        tree.pos = pos;
	return tree;
    }

    public DoLoop DoLoop(Tree body, Tree cond) {
        DoLoop tree = new DoLoop(body, cond);
        tree.pos = pos;
	return tree;
    }

    public WhileLoop WhileLoop(Tree cond, Tree body) {
        WhileLoop tree = new WhileLoop(cond, body);
        tree.pos = pos;
	return tree;
    }

    public ForLoop ForLoop(List<Tree> init, Tree cond, List<Tree> step, Tree body) {
        ForLoop tree = new ForLoop(init, cond, step, body);
        tree.pos = pos;
	return tree;
    }

    public ForeachLoop ForeachLoop(VarDef var, Tree expr, Tree body) {
        ForeachLoop tree = new ForeachLoop(var, expr, body);
        tree.pos = pos;
	return tree;
    }

    public Labelled Labelled(Name label, Tree body) {
        Labelled tree = new Labelled(label, body);
        tree.pos = pos;
	return tree;
    }

    public Switch Switch(Tree selector, List<Case> cases) {
        Switch tree = new Switch(selector, cases);
        tree.pos = pos;
	return tree;
    }

    public Case Case(Tree pat, List<Tree> stats) {
        Case tree = new Case(pat, stats);
        tree.pos = pos;
	return tree;
    }

    public Synchronized Synchronized(Tree lock, Tree body) {
        Synchronized tree = new Synchronized(lock, body);
        tree.pos = pos;
	return tree;
    }

    public Try Try(Tree body, List<Catch> catchers, Tree finalizer) {
        Try tree = new Try(body, catchers, finalizer);
        tree.pos = pos;
	return tree;
    }

    public Catch Catch(VarDef param, Tree body) {
        Catch tree = new Catch(param, body);
        tree.pos = pos;
	return tree;
    }

    public Conditional Conditional(Tree cond, Tree thenpart, Tree elsepart) {
        Conditional tree = new Conditional(cond, thenpart, elsepart);
        tree.pos = pos;
	return tree;
    }

    public If If(Tree cond, Tree thenpart, Tree elsepart) {
        If tree = new If(cond, thenpart, elsepart);
        tree.pos = pos;
	return tree;
    }

    public Exec Exec(Tree expr) {
        Exec tree = new Exec(expr);
        tree.pos = pos;
	return tree;
    }

    public Break Break(Name label) {
        Break tree = new Break(label, null);
        tree.pos = pos;
	return tree;
    }

    public Continue Continue(Name label) {
        Continue tree = new Continue(label, null);
        tree.pos = pos;
	return tree;
    }

    public Return Return(Tree expr) {
        Return tree = new Return(expr);
        tree.pos = pos;
	return tree;
    }

    public Throw Throw(Tree expr) {
        Throw tree = new Throw(expr);
        tree.pos = pos;
	return tree;
    }

    public Assert Assert(Tree cond, Tree detail) {
        Assert tree = new Assert(cond, detail);
        tree.pos = pos;
	return tree;
    }

    public Apply Apply(List<Tree> typeargs, Tree fn, List<Tree> args) {
        Apply tree = new Apply(typeargs, fn, args);
        tree.pos = pos;
	return tree;
    }

    public NewClass NewClass(Tree encl, List<Tree> typeargs, Tree clazz, List<Tree> args, ClassDef def) {
        NewClass tree = new NewClass(encl, typeargs, clazz, args, def);
        tree.pos = pos;
	return tree;
    }

    public NewArray NewArray(Tree elemtype, List<Tree> dims, List<Tree> elems) {
        NewArray tree = new NewArray(elemtype, dims, elems);
        tree.pos = pos;
	return tree;
    }

    public Parens Parens(Tree expr) {
	Parens tree = new Parens(expr);
	tree.pos = pos;
	return tree;
    }

    public Assign Assign(Tree lhs, Tree rhs) {
        Assign tree = new Assign(lhs, rhs);
        tree.pos = pos;
	return tree;
    }

    public Assignop Assignop(int opcode, Tree lhs, Tree rhs) {
        Assignop tree = new Assignop(opcode, lhs, rhs, null);
        tree.pos = pos;
	return tree;
    }

    public Unary Unary(int opcode, Tree arg) {
        Unary tree = new Unary(opcode, arg);
        tree.pos = pos;
	return tree;
    }

    public Binary Binary(int opcode, Tree lhs, Tree rhs) {
        Binary tree = new Binary(opcode, lhs, rhs, null);
        tree.pos = pos;
	return tree;
    }

    public TypeCast TypeCast(Tree clazz, Tree expr) {
        TypeCast tree = new TypeCast(clazz, expr);
        tree.pos = pos;
	return tree;
    }

    public TypeTest TypeTest(Tree expr, Tree clazz) {
        TypeTest tree = new TypeTest(expr, clazz);
        tree.pos = pos;
	return tree;
    }

    public Indexed Indexed(Tree indexed, Tree index) {
        Indexed tree = new Indexed(indexed, index);
        tree.pos = pos;
	return tree;
    }

    public Select Select(Tree selected, Name selector) {
        Select tree = new Select(selected, selector, null);
        tree.pos = pos;
	return tree;
    }

    public Ident Ident(Name name) {
        Ident tree = new Ident(name, null);
        tree.pos = pos;
	return tree;
    }

    public Literal Literal(int tag, Object value) {
        Literal tree = new Literal(tag, value);
        tree.pos = pos;
	return tree;
    }

    public TypeIdent TypeIdent(int typetag) {
        TypeIdent tree = new TypeIdent(typetag);
        tree.pos = pos;
	return tree;
    }

    public TypeArray TypeArray(Tree elemtype) {
        TypeArray tree = new TypeArray(elemtype);
        tree.pos = pos;
	return tree;
    }

    public TypeApply TypeApply(Tree clazz, List<Tree> arguments) {
        TypeApply tree = new TypeApply(clazz, arguments);
        tree.pos = pos;
	return tree;
    }

    public TypeParameter TypeParameter(Name name, List<Tree> bounds) {
        TypeParameter tree = new TypeParameter(name, bounds);
        tree.pos = pos;
	return tree;
    }

    public TypeArgument TypeArgument(TypeBoundKind kind, Tree type) {
        TypeArgument tree = new TypeArgument(kind, type);
        tree.pos = pos;
        return tree;
    }

    public TypeBoundKind TypeBoundKind(BoundKind kind) {
        TypeBoundKind tree = new TypeBoundKind(kind);
        tree.pos = pos;
        return tree;
    }

    public Annotation Annotation(Tree annotationType, List<Tree> args) {
        Annotation tree = new Annotation(annotationType, args);
        tree.pos = pos;
        return tree;
    }

    public Modifiers Modifiers(long flags, List<Annotation> annotations) {
        Modifiers tree = new Modifiers(flags, annotations);
        tree.pos = pos;
        return tree;
    }

    public Modifiers Modifiers(long flags) {
	return Modifiers(flags, Annotation.emptyList);
    }

    public Erroneous Erroneous() {
        Erroneous tree = new Erroneous();
        tree.pos = pos;
	return tree;
    }

    public LetExpr LetExpr(List<VarDef> defs, Tree expr) {
	LetExpr tree = new LetExpr(defs, expr);
        tree.pos = pos;
	return tree;
    }

/* ***************************************************************************
 * Derived building blocks.
 * The following operations all set type and symbol attributes.
 ****************************************************************************/

    public LetExpr LetExpr(VarDef def, Tree expr) {
	LetExpr tree = new LetExpr(new List<VarDef>().prepend(def), expr);
        tree.pos = pos;
	return tree;
    }

    /** Create an identifier from a symbol.
     */
    public Tree Ident(Symbol sym) {
 	return new Ident((sym.name != names.empty)
                         ? sym.name
                         : sym.flatName(), sym)
            .setPos(pos)
            .setType(sym.type);
    }

    /** Create a selection node from a qualifier tree and a symbol.
     *  @param base   The qualifier tree.
     */
    public Tree Select(Tree base, Symbol sym) {
	return new Select(base, sym.name, sym).setPos(pos).setType(sym.type);
    }

    /** Create a qualified identifier from a symbol, adding enough qualifications
     *  to make the reference unique.
     */
    public Tree QualIdent(Symbol sym) {
	return isUnqualifiable(sym)
	    ? Ident(sym)
	    : Select(QualIdent(sym.owner), sym);
    }

    /** Create an identifier that refers to the variable declared in given variable
     *  declaration.
     */
    public Tree Ident(VarDef param) {
	return Ident(param.sym);
    }

    /** Create a list of identifiers referring to the variables declared
     *  in given list of variable declarations.
     */
    public List<Tree> Idents(List<VarDef> params) {
	ListBuffer<Tree> ids = new ListBuffer<Tree>();
	for (List<VarDef> l = params; l.nonEmpty(); l = l.tail)
	    ids.append(Ident(l.head));
	return ids.toList();
    }

    /** Create a tree representing `this', given its type.
     */
    public Tree This(Type t) {
	return Ident(new VarSymbol(FINAL, names._this, t, t.tsym));
    }

    /** Create a tree representing a class literal.
     */
    public Tree ClassLiteral(ClassSymbol clazz) {
        return ClassLiteral(clazz.type);
    }

    /** Create a tree representing a class literal.
     */
    public Tree ClassLiteral(Type t) {
	VarSymbol lit = new VarSymbol(STATIC | PUBLIC | FINAL,
				      names._class,
                                      t,
				      t.tsym);
	return Select(Type(t), lit);
    }

    /** Create a tree representing `super', given its type and owner.
     */
    public Tree Super(Type t, TypeSymbol owner) {
	return Ident(new VarSymbol(FINAL, names._super, t, owner));
    }

    /** Create a method invocation from a method tree and a list of argument trees.
     */
    public Tree App(Tree meth, List<Tree> args) {
	return Apply(null, meth, args).setType(meth.type.restype());
    }

    /** Create a method invocation from a method tree and a list of argument trees.
     */
    public Tree Create(Symbol ctor, List<Tree> args) {
        Type t = ctor.owner.erasure(types);
        NewClass newclass = NewClass(null, null, Type(t), args, null);
        newclass.constructor = ctor;
        newclass.setType(t);
        return newclass;
    }

    /** Create a tree representing given type.
     */
    public Tree Type(Type t) {
	if (t == null) return null;
	Tree tp;
	switch (t.tag) {
	case BYTE: case CHAR: case SHORT: case INT: case LONG: case FLOAT:
	case DOUBLE: case BOOLEAN: case VOID:
	    tp = TypeIdent(t.tag);
	    break;
	case TYPEVAR:
	    tp = Ident(t.tsym);
	    break;
	case TYPEARG: {
	    ArgumentType a = ((ArgumentType) t);
	    tp = TypeArgument(TypeBoundKind(a.kind), Type(a.type));
	    break;
	}
	case CLASS:
	    Type outer = t.outer();
	    Tree clazz = outer.tag == CLASS && t.tsym.owner.kind == TYP
		? Select(Type(outer), t.tsym)
		: QualIdent(t.tsym);
	    tp = t.typarams().isEmpty()
		? clazz
		: TypeApply(clazz, Types(t.typarams()));
	    break;
	case ARRAY:
	    tp = TypeArray(Type(types.elemtype(t)));
	    break;
	case ERROR:
	    tp = TypeIdent(ERROR);
	    break;
	default:
	    throw new AssertionError("unexpected type: " + t);
	}
	return tp.setType(t);
    }
//where
        private Tree Selectors(Tree base, Symbol sym, Symbol limit) {
	    if (sym == limit) return base;
	    else return Select(Selectors(base, sym.owner, limit), sym);
	}

    /** Create a list of trees representing given list of types.
     */
    public List<Tree> Types(List<Type> ts) {
	ListBuffer<Tree> types = new ListBuffer<Tree>();
	for (List<Type> l = ts; l.nonEmpty(); l = l.tail)
	    types.append(Type(l.head));
	return types.toList();
    }

    /** Create a variable definition from a variable symbol and an initializer
     *  expression.
     */
    public VarDef VarDef(VarSymbol v, Tree init) {
	return (VarDef)
	    new VarDef(
		Modifiers(v.flags(), Annotations(v.attributes())),
		v.name,
		Type(v.type),
		init,
		v).setPos(pos).setType(v.type);
    }

    /** Create annotation trees from annotations.
     */
    public List<Annotation> Annotations(List<Attribute.Compound> attributes) {
	if (attributes == null) return Annotation.emptyList;
	ListBuffer<Annotation> result = new ListBuffer<Annotation>();
	for (List<Attribute.Compound> i = attributes; i.nonEmpty(); i=i.tail) {
	    Attribute a = i.head;
	    result.append(Annotation(a));
	}
	return result.toList();
    }

    public Tree Literal(Object value) {
	Tree result = null;
	if (value instanceof String) {
	    result = Literal(CLASS, value).
		setType(syms.stringType.constType(value));
	} else if (value instanceof Integer) {
	    result = Literal(INT, value).
		setType(syms.intType.constType(value));
	} else if (value instanceof Long) {
	    result = Literal(LONG, value).
		setType(syms.longType.constType(value));
	} else if (value instanceof Byte) {
	    result = Literal(BYTE, value).
		setType(syms.byteType.constType(value));
	} else if (value instanceof Character) {
	    result = Literal(CHAR, value).
		setType(syms.charType.constType(value));
	} else if (value instanceof Double) {
	    result = Literal(DOUBLE, value).
		setType(syms.doubleType.constType(value));
	} else if (value instanceof Float) {
	    result = Literal(FLOAT, value).
		setType(syms.floatType.constType(value));
	} else if (value instanceof Short) {
	    result = Literal(SHORT, value).
		setType(syms.shortType.constType(value));
	} else {
	    throw new AssertionError(value);
	}
	return result;
    }

    class AnnotationBuilder implements Attribute.Visitor {
	Tree result = null;
	public void visitConstant(Attribute.Constant v) {
	    result = Literal(v.value);
	}
	public void visitClass(Attribute.Class clazz) {
	    result = ClassLiteral(clazz.type).setType(syms.classType);
	}
	public void visitEnum(Attribute.Enum e) {
	    throw new AssertionError("unimplemented");
	}
	public void visitError(Attribute.Error e) {
	    throw new AssertionError("unimplemented");
	}
	public void visitCompound(Attribute.Compound compound) {
	    result = visitCompoundInternal(compound);
	}
	public Annotation visitCompoundInternal(Attribute.Compound compound) {
	    ListBuffer<Tree> args = new ListBuffer<Tree>();
	    for (List<Pair<Symbol.MethodSymbol,Attribute>> values = compound.values; values.nonEmpty(); values=values.tail) {
		Pair<MethodSymbol,Attribute> pair = values.head;
		Tree valueTree = translate(pair.snd);
		args.append(Assign(Ident(pair.fst), valueTree).setType(valueTree.type));
	    }
	    return Annotation(Type(compound.type), args.toList());
	}
	public void visitArray(Attribute.Array array) {
	    throw new AssertionError(array);
	}
	Tree translate(Attribute a) {
	    a.accept(this);
	    return result;
	}
	Annotation translate(Attribute.Compound a) {
	    return visitCompoundInternal(a);
	}
    }
    AnnotationBuilder annotationBuilder = new AnnotationBuilder();

    /** Create an annotation tree from an attribute.
     */
    public Annotation Annotation(Attribute a) {
	return annotationBuilder.translate((Attribute.Compound)a);
    }

    /** Create a method definition from a method symbol and a method body.
     */
    public MethodDef MethodDef(MethodSymbol m, Block body) {
	return MethodDef(m, m.type, body);
    }

    /** Create a method definition from a method symbol, method type
     *  and a method body.
     */
    public MethodDef MethodDef(MethodSymbol m, Type mtype, Block body) {
	return (MethodDef)
	    new MethodDef(
		Modifiers(m.flags(), Annotations(m.attributes())),
		m.name,
		Type(mtype.restype()),
		TypeParams(mtype.typarams()),
		Params(mtype.argtypes(), m),
		Types(mtype.thrown()),
		body,
		null,
		m).setPos(pos).setType(mtype);
    }

    /** Create a type parameter tree from its name and type.
     */
    public TypeParameter TypeParam(Name name, TypeVar tvar) {
	return (TypeParameter)
	    TypeParameter(name, Types(types.getBounds(tvar))).setPos(pos).setType(tvar);
    }

    /** Create a list of type parameter trees from a list of type variables.
     */
    public List<TypeParameter> TypeParams(List<Type> typarams) {
	ListBuffer<TypeParameter> tparams = new ListBuffer<TypeParameter>();
	int i = 0;
	for (List<Type> l = typarams; l.nonEmpty(); l = l.tail)
	    tparams.append(TypeParam(l.head.tsym.name, (TypeVar)l.head));
	return tparams.toList();
    }

    /** Create a value parameter tree from its name, type, and owner.
     */
    public VarDef Param(Name name, Type argtype, Symbol owner) {
	return VarDef(new VarSymbol(0, name, argtype, owner), null);
    }

    /** Create a a list of value parameter trees x0, ..., xn from a list of
     *  their types and an their owner.
     */
    public List<VarDef> Params(List<Type> argtypes, Symbol owner) {
	ListBuffer<VarDef> params = new ListBuffer<VarDef>();
        MethodSymbol mth = (owner.kind == MTH) ? ((MethodSymbol)owner) : null;
        if (mth != null && mth.params != null && argtypes.length() == mth.params.length()) {
            for (VarSymbol param : ((MethodSymbol)owner).params)
                params.append(VarDef(param, null));
        } else {
            int i = 0;
            for (List<Type> l = argtypes; l.nonEmpty(); l = l.tail)
                params.append(Param(paramName(i++), l.head, owner));
        }
        return params.toList();
    }

    /** Wrap a method invocation in an expression statement or return statement,
     *  depending on whether the method invocation expression's type is void.
     */
    public Tree Call(Tree apply) {
	return apply.type.tag == VOID ? (Tree)Exec(apply) : (Tree)Return(apply);
    }

    /** Construct an assignment from a variable symbol and a right hand side.
     */
    public Tree Assignment(Symbol v, Tree rhs) {
	return Exec(Assign(Ident(v), rhs).setType(v.type));
    }

    /** Construct an index expression from a variable and an expression.
     */
    public Indexed Indexed(Symbol v, Tree index) {
        Indexed tree = new Indexed(QualIdent(v), index);
        tree.type = ((ArrayType)v.type).elemtype;
	return tree;
    }

    /** Make an attributed type cast expression.
     */
    public TypeCast TypeCast(Type type, Tree expr) {
        return (TypeCast)TypeCast(Type(type), expr).setType(type);
    }

/* ***************************************************************************
 * Helper methods.
 ****************************************************************************/

    /** Can given symbol be referred to in unqualified form?
     */
    boolean isUnqualifiable(Symbol sym) {
	if (sym.name == names.empty ||
            sym.owner == null ||
	    sym.owner.kind == MTH || sym.owner.kind == VAR) {
	    return true;
	} else if (sym.kind == TYP && toplevel != null) {
	    Scope.Entry e;
	    e = toplevel.namedImportScope.lookup(sym.name);
	    if (e.scope != null) {
		return
		  e.sym == sym &&
		  e.next().scope == null;
	    }
	    e = toplevel.packge.members().lookup(sym.name);
	    if (e.scope != null) {
	        return
		  e.sym == sym &&
		  e.next().scope == null;
	    }
	    e = toplevel.starImportScope.lookup(sym.name);
	    if (e.scope != null) {
	        return
		  e.sym == sym &&
		  e.next().scope == null;
	    }
	}
	return false;
    }

    /** The name of synthetic parameter number `i'.
     */
    public Name paramName(int i)   { return names.fromString("x" + i); }

    /** The name of synthetic type parameter number `i'.
     */
    public Name typaramName(int i) { return names.fromString("A" + i); }
}
