/**
 * @(#)Lower.java	1.145 04/06/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.comp;

import java.util.*;

import com.sun.tools.javac.code.*;
import com.sun.tools.javac.jvm.*;
import com.sun.tools.javac.tree.*;
import com.sun.tools.javac.util.*;
import com.sun.tools.javac.util.List;

import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.tree.Tree.*;
import com.sun.tools.javac.code.Type.*;

import com.sun.tools.javac.jvm.Target;

import static com.sun.tools.javac.code.Flags.*;
import static com.sun.tools.javac.code.Kinds.*;
import static com.sun.tools.javac.code.TypeTags.*;
import static com.sun.tools.javac.jvm.ByteCodes.*;

/** This pass translates away some syntactic sugar: inner classes,
 *  class literals, assertions, foreach loops, etc.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Lower extends TreeTranslator {
    protected static final Context.Key<Lower> lowerKey =
	new Context.Key<Lower>();

    public static Lower instance(Context context) {
	Lower instance = context.get(lowerKey);
	if (instance == null)
	    instance = new Lower(context);
	return instance;
    }

    private Name.Table names;
    private Log log;
    private Symtab syms;
    private Resolve rs;
    private Check chk;
    private Attr attr;
    private TreeMaker make;
    private ClassWriter writer;
    private ClassReader reader;
    private ConstFold cfolder;
    private Target target;
    private Source source;
    private boolean allowEnums;
    private final Name dollarAssertionsDisabled;
    private final Name classDollar;
    private Types types;

    protected Lower(Context context) {
	context.put(lowerKey, this);
	names = Name.Table.instance(context);
	log = Log.instance(context);
	syms = Symtab.instance(context);
	rs = Resolve.instance(context);
	chk = Check.instance(context);
	attr = Attr.instance(context);
	make = TreeMaker.instance(context);
	writer = ClassWriter.instance(context);
	reader = ClassReader.instance(context);
	cfolder = ConstFold.instance(context);
	target = Target.instance(context);
	source = Source.instance(context);
	allowEnums = source.allowEnums();
	dollarAssertionsDisabled = names.
	    fromString(target.syntheticNameChar() + "assertionsDisabled");
	classDollar = names.
	    fromString("class" + target.syntheticNameChar());
	
	types = Types.instance(context);
    }

    /** The currently enclosing class.
     */
    ClassSymbol currentClass;

    /** A queue of all translated classes.
     */
    ListBuffer<Tree> translated;

    /** Environment for symbol lookup, set by translateTopLevelClass.
     */
    Env<AttrContext> attrEnv;

    /** A hash table mapping syntax trees to their ending source positions.
     */
    Map<Tree,Integer> endPositions;

/**************************************************************************
 * Global mappings
 *************************************************************************/

    /** A hash table mapping local classes to their definitions.
     */
    Map<ClassSymbol,ClassDef> classdefs;

    /** A hash table mapping virtual accessed symbols in outer subclasses
     *  to the actually referred symbol in superclasses.
     */
    Map<Symbol,Symbol> actualSymbols;

    /** The current method definition.
     */
    MethodDef currentMethodDef;

    /** The current method symbol.
     */
    MethodSymbol currentMethodSym;

    /** The currently enclosing outermost class definition.
     */
    ClassDef outermostClassDef;

    /** The currently enclosing outermost member definition.
     */
    Tree outermostMemberDef;

    /** A navigator class for assembling a mapping from local class symbols
     *  to class definition trees.
     *  There is only one case; all other cases simply traverse down the tree.
     */
    class ClassMap extends TreeScanner {

	/** All encountered class defs are entered into classdefs table.
	 */
	public void visitClassDef(ClassDef tree) {
	    classdefs.put(tree.sym, tree);
	    super.visitClassDef(tree);
	}
    }
    ClassMap classMap = new ClassMap();

    /** Map a class symbol to its definition.
     *  @param c    The class symbol of which we want to determine the definition.
     */
    ClassDef classDef(ClassSymbol c) {
	// First lookup the class in the classdefs table.
	ClassDef def = classdefs.get(c);
	if (def == null && outermostMemberDef != null) {
	    // If this fails, traverse outermost member definition, entering all
	    // local classes into classdefs, and try again.
	    classMap.scan(outermostMemberDef);
	    def = classdefs.get(c);
	}
	if (def == null) {
	    // If this fails, traverse outermost class definition, entering all
	    // local classes into classdefs, and try again.
	    classMap.scan(outermostClassDef);
	    def = classdefs.get(c);
	}
	return def;
    }

    /** A hash table mapping class symbols to lists of free variables.
     *  accessed by them. Only free variables of the method immediately containing
     *  a class are associated with that class.
     */
    Map<ClassSymbol,List<VarSymbol>> freevarCache;

    /** A navigator class for collecting the free variables accessed
     *  from a local class.
     *  There is only one case; all other cases simply traverse down the tree.
     */
    class FreeVarCollector extends TreeScanner {

	/** The owner of the local class.
	 */
	Symbol owner;

	/** The local class.
	 */
	ClassSymbol clazz;

	/** The list of owner's variables accessed from within the local class,
	 *  without any duplicates.
	 */
	List<VarSymbol> fvs;

	FreeVarCollector(ClassSymbol clazz) {
	    this.clazz = clazz;
	    this.owner = clazz.owner;
	    this.fvs = VarSymbol.emptyList;
	}

	/** Add free variable to fvs list unless it is already there.
	 */
	private void addFreeVar(VarSymbol v) {
	    for (List<VarSymbol> l = fvs; l.nonEmpty(); l = l.tail)
		if (l.head == v) return;
	    fvs = fvs.prepend(v);
	}

	/** Add all free variables of class c to fvs list
	 *  unless they are already there.
	 */
	private void addFreeVars(ClassSymbol c) {
	    List<VarSymbol> fvs = freevarCache.get(c);
	    if (fvs != null) {
		for (List<VarSymbol> l = fvs; l.nonEmpty(); l = l.tail) {
		    addFreeVar(l.head);
		}
	    }
	}

	/** If tree refers to a variable in owner of local class, add it to
	 *  free variables list.
	 */
	public void visitIdent(Ident tree) {
	    result = tree;
	    visitSymbol(tree.sym);
	}
	// where
	private void visitSymbol(Symbol _sym) {
	    Symbol sym = _sym;
	    if (sym.kind == VAR) {
		while (sym != null && sym.owner != owner)
		    sym = proxies.lookup(proxyName(sym.name)).sym;
		if (sym != null && sym.owner == owner) {
		    VarSymbol v = (VarSymbol)sym;
		    if (v.constValue == null) {
			addFreeVar(v);
		    }
		} else {
		    if (outerThisStack.head != null &&
			outerThisStack.head != _sym)
			visitSymbol(outerThisStack.head);
		}
	    }
	}

	/** If tree refers to a class instance creation expression
	 *  add all free variables of the freshly created class.
	 */
        public void visitNewClass(NewClass tree) {
	    ClassSymbol c = (ClassSymbol)tree.constructor.owner;
	    addFreeVars(c);
	    if (tree.encl == null &&
		c.hasOuterInstance() &&
		outerThisStack.head != null)
		visitSymbol(outerThisStack.head);
	    super.visitNewClass(tree);
	}

	/** If tree refers to a qualified this or super expression
	 *  for anything but the current class, add the outer this
	 *  stack as a free variable.
	 */
	public void visitSelect(Select tree) {
	    if ((tree.name == names._this || tree.name == names._super) &&
		tree.selected.type.tsym != clazz &&
		outerThisStack.head != null)
		visitSymbol(outerThisStack.head);
	    super.visitSelect(tree);
	}

	/** If tree refers to a superclass constructor call,
	 *  add all free variables of the superclass.
	 */
        public void visitApply(Apply tree) {
	    if (TreeInfo.name(tree.meth) == names._super) {
		addFreeVars((ClassSymbol) TreeInfo.symbol(tree.meth).owner);
		Symbol constructor = TreeInfo.symbol(tree.meth);
		ClassSymbol c = (ClassSymbol)constructor.owner;
		if (c.hasOuterInstance() &&
		    tree.meth.tag != Tree.SELECT &&
		    outerThisStack.head != null)
		    visitSymbol(outerThisStack.head);
	    }
	    super.visitApply(tree);
	}
    }

    /** Return the variables accessed from within a local class, which
     *  are declared in the local class' owner.
     *  (in reverse order of first access).
     */
    List<VarSymbol> freevars(ClassSymbol c)  {
	if ((c.owner.kind & (VAR | MTH)) != 0) {
	    List<VarSymbol> fvs = freevarCache.get(c);
	    if (fvs == null) {
		FreeVarCollector collector = new FreeVarCollector(c);
		collector.scan(classDef(c));
		fvs = collector.fvs;
		freevarCache.put(c, fvs);
	    }
	    return fvs;
	} else {
	    return VarSymbol.emptyList;
	}
    }

    Map<TypeSymbol,EnumMapping> enumSwitchMap = new LinkedHashMap<TypeSymbol,EnumMapping>();

    EnumMapping mapForEnum(int pos, TypeSymbol enumClass) {
        EnumMapping map = enumSwitchMap.get(enumClass);
        if (map == null)
            enumSwitchMap.put(enumClass, map = new EnumMapping(pos, enumClass));
        return map;
    }

    /** This map gives a translation table to be used for enum
     *  switches.
     *
     *  <p>For each enum that appears as the type of a switch
     *  expression, we maintain an EnumMapping to assist in the
     *  translation, as exemplified by the following example:
     *
     *  <p>we translate
     *  <pre>
     *          switch(colorExpression) {
     *          case red: stmt1;
     *          case green: stmt2;
     *          }
     *  </pre>
     *  into
     *  <pre>
     *          switch(Outer$0.$EnumMap$Color[colorExpression.ordinal()]) {
     *          case 1: stmt1;
     *          case 2: stmt2
     *          }
     *  </pre>
     *  with the auxilliary table intialized as follows:
     *  <pre>
     *          class Outer$0 {
     *              synthetic final int[] $EnumMap$Color = new int[Color.values().length];
     *              static {
     *                  try { $EnumMap$Color[red.ordinal()] = 1; } catch (NoSuchFieldError ex) {}
     *                  try { $EnumMap$Color[green.ordinal()] = 2; } catch (NoSuchFieldError ex) {}
     *              }
     *          }
     *  </pre>
     *  class EnumMapping provides mapping data and support methods for this translation.
     */
    class EnumMapping {
        EnumMapping(int pos, TypeSymbol forEnum) {
            this.forEnum = forEnum;
            this.values = new LinkedHashMap<VarSymbol,Integer>();
            this.pos = pos;
            Name varName = names
                .fromString(target.syntheticNameChar() +
                            "SwitchMap" +
                            target.syntheticNameChar() +
                            writer.xClassName(forEnum.type).toString()
                            .replace('/', '.')
                            .replace('.', target.syntheticNameChar()));
            ClassSymbol outerCacheClass = outerCacheClass();
            this.mapVar = new VarSymbol(STATIC | SYNTHETIC | FINAL,
                                        varName,
                                        new ArrayType(syms.intType, syms.arrayClass),
                                        outerCacheClass);
            enterSynthetic(pos, mapVar, outerCacheClass.members());
        }

        int pos = Position.NOPOS;

        // the next value to use
        int next = 1; // 0 (unused map elements) go to the default label

        // the enum for which this is a map
        final TypeSymbol forEnum;

        // the field containing the map
        final VarSymbol mapVar;

        // the mapped values
        final Map<VarSymbol,Integer> values;

        Tree forConstant(VarSymbol v) {
            Integer result = values.get(v);
            if (result == null)
                values.put(v, result = next++);
            return make.Literal(result);
        }

        // generate the field initializer for the map
        void translate() {
            make.at(pos);
            ClassDef owner = classDef((ClassSymbol)mapVar.owner);

            // synthetic static final int[] $SwitchMap$Color = new int[Color.values().length];
            MethodSymbol valuesMethod = lookupMethod(pos,
                                                     names.values,
                                                     forEnum.type,
                                                     Type.emptyList);
            Tree size = make // Color.values().length
                .Select(make.App(make.QualIdent(valuesMethod), Tree.emptyList),
                        syms.lengthVar);
            Tree mapVarInit = make
                .NewArray(make.Type(syms.intType),
                          Tree.emptyList.prepend(size),
                          null)
                .setType(new ArrayType(syms.intType, syms.arrayClass));

            // try { $SwitchMap$Color[red.ordinal()] = 1; } catch (java.lang.NoSuchFieldError ex) {}
            ListBuffer<Tree> stmts = new ListBuffer<Tree>();
            Symbol ordinalMethod = lookupMethod(pos,
                                                names.ordinal,
                                                forEnum.type,
                                                Type.emptyList);
            List<Catch> catcher = Catch.emptyList
                .prepend(make.Catch(make.VarDef(new VarSymbol(PARAMETER, names.ex,
                                                              syms.noSuchFieldErrorType,
                                                              syms.noSymbol),
                                                null),
                                    make.Block(0, Tree.emptyList)));
            for (Map.Entry<VarSymbol,Integer> e : values.entrySet()) {
                VarSymbol enumerator = e.getKey();
                Integer mappedValue = e.getValue();
                Tree assign = make
                    .Assign(make.Indexed(mapVar,
                                         make.App(make.Select(make.QualIdent(enumerator),
                                                              ordinalMethod),
                                                  Tree.emptyList)),
                            make.Literal(mappedValue))
                    .setType(syms.intType);
                assign = make.Exec(assign);
                Tree _try = make.Try(make.Block(0, Tree.emptyList.prepend(assign)),
                                     catcher, null);
                stmts.append(_try);
            }

            owner.defs = owner.defs
                .prepend(make.Block(STATIC, stmts.toList()))
                .prepend(make.VarDef(mapVar, mapVarInit));
        }
    }


/**************************************************************************
 * Tree building blocks
 *************************************************************************/

    /** Make an attributed tree representing a literal. This will be an
     *  Ident node in the case of boolean literals, a Literal node in all
     *  other cases.
     *  @param type       The literal's type.
     *  @param value      The literal's value.
     */
    Tree makeLit(Type type, Object value) {
	if (type.tag == BOOLEAN) {
	    return make.Ident(
		((Integer)value).intValue() == 0
		? syms.falseConst
		: syms.trueConst);
	} else {
	    return make.Literal(type.tag, value).setType(type.constType(value));
	}
    }

    /** Make an attributed class instance creation expression.
     *  @param ctype    The class type.
     *  @param args     The constructor arguments.
     */
    NewClass makeNewClass(Type ctype, List<Tree> args) {
	NewClass tree = make.NewClass(null,
	    null, make.QualIdent(ctype.tsym), args, null);
	tree.constructor = rs.resolveConstructor(
	    make.pos, attrEnv, ctype, TreeInfo.types(args), null, false, false);
	tree.type = ctype;
	return tree;
    }

    /** Make an attributed unary expression.
     *  @param optag    The operators tree tag.
     *  @param arg      The operator's argument.
     */
    Tree makeUnary(int optag, Tree arg) {
	Unary tree = make.Unary(optag, arg);
	tree.operator = rs.resolveUnaryOperator(
	    make.pos, optag, attrEnv, arg.type);
	tree.type = tree.operator.type.restype();
	return tree;
    }

    /** Make an attributed binary expression.
     *  @param optag    The operators tree tag.
     *  @param lhs      The operator's left argument.
     *  @param rhs      The operator's right argument.
     */
    Binary makeBinary(int optag, Tree lhs, Tree rhs) {
	Binary tree = make.Binary(optag, lhs, rhs);
	tree.operator = rs.resolveBinaryOperator(
	    make.pos, optag, attrEnv, lhs.type, rhs.type);
	tree.type = tree.operator.type.restype();
	return tree;
    }

    /** Make an attributed assignop expression.
     *  @param optag    The operators tree tag.
     *  @param lhs      The operator's left argument.
     *  @param rhs      The operator's right argument.
     */
    Assignop makeAssignop(int optag, Tree lhs, Tree rhs) {
	Assignop tree = make.Assignop(optag, lhs, rhs);
	tree.operator = rs.resolveBinaryOperator(
	    make.pos, tree.tag - Tree.ASGOffset, attrEnv, lhs.type, rhs.type);
	tree.type = lhs.type;
	return tree;
    }

    /** Convert tree into string object, unless it has already a
     *  reference type..
     */
    Tree makeString(Tree tree) {
        if (tree.type.tag >= CLASS) {
	    return tree;
	} else {
	    Symbol valueOfSym = lookupMethod(tree.pos,
					     names.valueOf,
					     syms.stringType,
					     Type.emptyList.prepend(tree.type));
	    return make.App(make.QualIdent(valueOfSym), List.<Tree>make(tree));
	}
    }

    /** Create an empty anonymous class definition and enter and complete
     *  its symbol. Return the class definition's symbol.
     *  and create
     *  @param flags    The class symbol's flags
     *  @param owner    The class symbol's owner
     */
    ClassSymbol makeEmptyClass(long flags, ClassSymbol owner) {
	// Create class symbol.
	ClassSymbol c = reader.defineClass(names.empty, owner);
	c.flatname = chk.localClassName(c);
	c.sourcefile = owner.sourcefile;
	c.completer = null;
	c.members_field = new Scope(c);
	c.flags_field = flags;
	ClassType ctype = (ClassType) c.type;
	ctype.supertype_field = syms.objectType;
	ctype.interfaces_field = Type.emptyList;

	ClassDef odef = classDef(owner);

	// Enter class symbol in owner scope and compiled table.
	enterSynthetic(odef.pos, c, owner.members());
	chk.compiled.put(c.flatname, c);

	// Create class definition tree.
	ClassDef cdef = make.ClassDef(
            make.Modifiers(flags), names.empty,
	    TypeParameter.emptyList,
	    null, Tree.emptyList, Tree.emptyList);
	cdef.sym = c;
	cdef.type = c.type;

	// Append class definition tree to owner's definitions.
	odef.defs = odef.defs.prepend(cdef);

	return c;
    }

/**************************************************************************
 * Symbol manipulation utilities
 *************************************************************************/

    /** Report a conflict between a user symbol and a synthetic symbol.
     */
    private void duplicateError(int pos, Symbol sym) {
        if (!sym.type.isErroneous()) {
	    log.error(pos, "synthetic.name.conflict", sym, sym.location());
	}
    }

    /** Enter a synthetic symbol in a given scope, but complain if there was already one there.
     *  @param pos           Position for error reporting.
     *  @param sym           The symbol.
     *  @param s             The scope.
     */
    private void enterSynthetic(int pos, Symbol sym, Scope s) {
        if (sym.name != names.error && sym.name != names.empty) {
	    for (Scope.Entry e = s.lookup(sym.name); e.scope == s; e = e.next()) {
		if (sym != e.sym && sym.kind == e.sym.kind) {
		    // VM allows methods and variables with differing types
		    if ((sym.kind & (MTH|VAR)) != 0 &&
			!types.erasure(sym.type).equals(types.erasure(e.sym.type)))
			continue;
		    duplicateError(pos, e.sym);
		    break;
		}
	    }
	}
	s.enter(sym);
    }

    /** Look up a synthetic name in a given scope.
     *  @param scope	    The scope.
     *  @param name	    The name.
     */
    private Symbol lookupSynthetic(Name name, Scope s) {
	Symbol sym = s.lookup(name).sym;
	return (sym==null || (sym.flags()&SYNTHETIC)==0) ? null : sym;
    }

    /** Look up a method in a given scope.
     */
    private MethodSymbol lookupMethod(int pos, Name name, Type qual, List<Type> args) {
	return rs.resolveInternalMethod(pos, attrEnv, qual, name, args, null);
    }

    /** Look up a constructor.
     */
    private MethodSymbol lookupConstructor(int pos, Type qual, List<Type> args) {
	return rs.resolveInternalConstructor(pos, attrEnv, qual, args, null);
    }

    /** Look up a field.
     */
    private VarSymbol lookupField(int pos, Type qual, Name name) {
	return rs.resolveInternalField(pos, attrEnv, qual, name);
    }

/**************************************************************************
 * Access methods
 *************************************************************************/

    /** Access codes for dereferencing, assignment,
     *  and pre/post increment/decrement.
     *  Access codes for assignment operations are determined by method accessCode
     *  below.
     *
     *  All access codes for accesses to the current class are even.
     *  If a member of the superclass should be accessed instead (because
     *  access was via a qualified super), add one to the corresponding code
     *  for the current class, making the number odd.
     *  This numbering scheme is used by the backend to decide whether
     *  to issue an invokevirtual or invokespecial call.
     *
     *  @see Gen.visitSelect(Select tree)
     */
    private static final int
        DEREFcode = 0,
	ASSIGNcode = 2,
	PREINCcode = 4,
	PREDECcode = 6,
	POSTINCcode = 8,
	POSTDECcode = 10,
	FIRSTASGOPcode = 12;

    /** Number of access codes
     */
    private static final int NCODES = accessCode(ByteCodes.lushrl) + 2;

    /** A mapping from symbols to their access numbers.
     */
    private Map<Symbol,Integer> accessNums;

    /** A mapping from symbols to an array of access symbols, indexed by
     *  access code.
     */
    private Map<Symbol,MethodSymbol[]> accessSyms;

    /** A mapping from (constructor) symbols to access constructor symbols.
     */
    private Map<Symbol,MethodSymbol> accessConstrs;

    /** A queue for all accessed symbols.
     */
    private ListBuffer<Symbol> accessed;

    /** Map bytecode of binary operation to access code of corresponding
     *  assignment operation. This is always an even number.
     */
    private static int accessCode(int bytecode) {
	if (ByteCodes.iadd <= bytecode && bytecode <= ByteCodes.lxor)
	    return (bytecode - iadd) * 2 + FIRSTASGOPcode;
	else if (bytecode == ByteCodes.string_add)
	    return (ByteCodes.lxor + 1 - iadd) * 2 + FIRSTASGOPcode;
	else if (ByteCodes.ishll <= bytecode && bytecode <= ByteCodes.lushrl)
	    return (bytecode - ishll + ByteCodes.lxor + 2 - iadd) * 2 + FIRSTASGOPcode;
	else
	    return -1;
    }

    /** return access code for identifier,
     *  @param tree     The tree representing the identifier use.
     *  @param enclOp   The closest enclosing operation node of tree,
     *                  null if tree is not a subtree of an operation.
     */
    private static int accessCode(Tree tree, Tree enclOp) {
	if (enclOp == null)
	    return DEREFcode;
	else if (enclOp.tag == Tree.ASSIGN &&
		 tree == TreeInfo.skipParens(((Assign)enclOp).lhs))
	    return ASSIGNcode;
	else if (Tree.PREINC <= enclOp.tag && enclOp.tag <= Tree.POSTDEC &&
		 tree == TreeInfo.skipParens(((Unary)enclOp).arg))
	    return (enclOp.tag - Tree.PREINC) * 2 + PREINCcode;
	else if (Tree.BITOR_ASG <= enclOp.tag && enclOp.tag <= Tree.MOD_ASG &&
		 tree == TreeInfo.skipParens(((Assignop)enclOp).lhs))
	    return accessCode(((OperatorSymbol)((Assignop)enclOp).operator).opcode);
	else
	    return DEREFcode;
    }

    /** Return binary operator that corresponds to given access code.
     */
    private OperatorSymbol binaryAccessOperator(int acode) {
	for (Scope.Entry e = syms.predefClass.members().elems;
	     e != null;
	     e = e.sibling) {
	    if (e.sym instanceof OperatorSymbol) {
		OperatorSymbol op = (OperatorSymbol)e.sym;
		if (accessCode(op.opcode) == acode) return op;
	    }
	}
	return null;
    }

    /** Return tree tag for assignment operation corresponding
     *  to given binary operator.
     */
    private static int treeTag(OperatorSymbol operator) {
	switch (operator.opcode) {
	case ByteCodes.ior: case ByteCodes.lor:
	    return Tree.BITOR_ASG;
	case ByteCodes.ixor: case ByteCodes.lxor:
	    return Tree.BITXOR_ASG;
	case ByteCodes.iand: case ByteCodes.land:
	    return Tree.BITAND_ASG;
	case ByteCodes.ishl: case ByteCodes.lshl:
	case ByteCodes.ishll: case ByteCodes.lshll:
	    return Tree.SL_ASG;
	case ByteCodes.ishr: case ByteCodes.lshr:
	case ByteCodes.ishrl: case ByteCodes.lshrl:
	    return Tree.SR_ASG;
	case ByteCodes.iushr: case ByteCodes.lushr:
	case ByteCodes.iushrl: case ByteCodes.lushrl:
	    return Tree.USR_ASG;
	case ByteCodes.iadd: case ByteCodes.ladd:
	case ByteCodes.fadd: case ByteCodes.dadd:
	case ByteCodes.string_add:
	    return Tree.PLUS_ASG;
	case ByteCodes.isub: case ByteCodes.lsub:
	case ByteCodes.fsub: case ByteCodes.dsub:
	    return Tree.MINUS_ASG;
	case ByteCodes.imul: case ByteCodes.lmul:
	case ByteCodes.fmul: case ByteCodes.dmul:
	    return Tree.MUL_ASG;
	case ByteCodes.idiv: case ByteCodes.ldiv:
	case ByteCodes.fdiv: case ByteCodes.ddiv:
	    return Tree.DIV_ASG;
	case ByteCodes.imod: case ByteCodes.lmod:
	case ByteCodes.fmod: case ByteCodes.dmod:
	    return Tree.MOD_ASG;
	default:
	    throw new AssertionError();
	}
    }

    /** The name of the access method with number `anum' and access code `acode'.
     */
    Name accessName(int anum, int acode) {
	return names.fromString(
	    "access" + target.syntheticNameChar() + anum + acode / 10 + acode % 10);
    }

    /** Return access symbol for a private or protected symbol from an inner class.
     *  @param sym        The accessed private symbol.
     *  @param tree       The accessing tree.
     *  @param enclOp     The closest enclosing operation node of tree,
     *                    null if tree is not a subtree of an operation.
     *  @param protAccess Is access to a protected symbol in another
     *                    package?
     *  @param refSuper   Is access via a (qualified) C.super?
     */
    MethodSymbol accessSymbol(Symbol sym, Tree tree, Tree enclOp,
			      boolean protAccess, boolean refSuper) {
	ClassSymbol accOwner = refSuper && protAccess
	    // For access via qualified super (T.super.x), place the
	    // access symbol on T.
	    ? (ClassSymbol)((Select)tree).selected.type.tsym
	    // Otherwise pretend that the owner of an accessed
	    // protected symbol is the enclosing class of the current
	    // class which is a subclass of the symbol's owner.
	    : accessClass(sym, protAccess, tree);

	Symbol vsym = sym;
	if (sym.owner != accOwner) {
	    vsym = sym.clone(accOwner);
	    actualSymbols.put(vsym, sym);
	}

	Integer anum              // The access number of the access method.
	    = accessNums.get(vsym);
	if (anum == null) {
	    anum = accessed.length();
	    accessNums.put(vsym, anum);
	    accessSyms.put(vsym, new MethodSymbol[NCODES]);
	    accessed.append(vsym);
	    // System.out.println("accessing " + vsym + " in " + vsym.location());
        }

	int acode;                // The access code of the access method.
	List<Type> argtypes;      // The argument types of the access method.
	Type restype;             // The result type of the access method.
	List<Type> thrown;        // The thrown execeptions of the access method.
	switch (vsym.kind) {
	case VAR:
	    acode = accessCode(tree, enclOp);
	    if (acode >= FIRSTASGOPcode) {
	        OperatorSymbol operator = binaryAccessOperator(acode);
		if (operator.opcode == string_add)
		    argtypes = List.make(syms.objectType);
		else
		    argtypes = operator.type.argtypes().tail;
	    } else if (acode == ASSIGNcode)
		argtypes = Type.emptyList.prepend(vsym.erasure(types));
	    else
		argtypes = Type.emptyList;
	    restype = vsym.erasure(types);
	    thrown = Type.emptyList;
	    break;
	case MTH:
	    acode = DEREFcode;
	    argtypes = vsym.erasure(types).argtypes();
	    restype = vsym.erasure(types).restype();
	    thrown = vsym.type.thrown();
	    break;
	default:
	    throw new AssertionError();
	}

	// For references via qualified super, increment acode by one,
	// making it odd.
	if (protAccess && refSuper) acode++;

	// Instance access methods get instance as first parameter.
	// For protected symbols this needs to be the instance as a member
	// of the type containing the accessed symbol, not the class
	// containing the access method.
	if ((vsym.flags() & STATIC) == 0) {
	    argtypes = argtypes.prepend(vsym.owner.erasure(types));
	}
	MethodSymbol[] accessors = accessSyms.get(vsym);
	MethodSymbol accessor = accessors[acode];
	if (accessor == null) {
	    accessor = new MethodSymbol(
		STATIC | SYNTHETIC,
		accessName(anum.intValue(), acode),
		new MethodType(argtypes, restype, thrown, syms.methodClass),
		accOwner);
	    enterSynthetic(tree.pos, accessor, accOwner.members());
	    accessors[acode] = accessor;
	}
	return accessor;
    }

    /** The qualifier to be used for accessing a symbol in an outer class.
     *  This is either C.sym or C.this.sym, depending on whether or not
     *  sym is static.
     *  @param sym   The accessed symbol.
     */
    Tree accessBase(int pos, Symbol sym) {
	return (sym.flags() & STATIC) != 0
	    ? access(make.at(pos).QualIdent(sym.owner))
	    : makeOwnerThis(pos, sym, true);
    }

    /** Do we need an access method to reference private symbol?
     */
    boolean needsPrivateAccess(Symbol sym) {
	if ((sym.flags() & PRIVATE) == 0 || sym.owner == currentClass) {
	    return false;
	} else if (sym.name == names.init && (sym.owner.owner.kind & (VAR | MTH)) != 0) {
	    // private constructor in local class: relax protection
	    sym.flags_field &= ~PRIVATE;
	    return false;
	} else {
	    return true;
	}
    }

    /** Do we need an access method to reference symbol in other package?
     */
    boolean needsProtectedAccess(Symbol sym, Tree tree) {
	if ((sym.flags() & PROTECTED) == 0 ||
	    sym.owner.owner == currentClass.owner || // fast special case
	    sym.packge() == currentClass.packge())
	    return false;
	if (!currentClass.isSubClass(sym.owner, types))
	    return true;
	if ((sym.flags() & STATIC) != 0 ||
	    tree.tag != Tree.SELECT ||
	    TreeInfo.name(((Select)tree).selected) == names._super)
	    return false;
	return !((Select)tree).selected.type.tsym.isSubClass(currentClass, types);
    }

    /** The class in which an access method for given symbol goes.
     *  @param sym        The access symbol
     *  @param protAccess Is access to a protected symbol in another
     *                    package?
     */
    ClassSymbol accessClass(Symbol sym, boolean protAccess, Tree tree) {
	if (protAccess) {
	    Symbol qualifier = null;
	    ClassSymbol c = currentClass;
	    if (tree.tag == Tree.SELECT && (sym.flags() & STATIC) == 0) {
		qualifier = ((Select)tree).selected.type.tsym;
		while (!qualifier.isSubClass(c, types)) {
		    c = c.owner.enclClass();
		}
		return c;
	    } else {
		while (!c.isSubClass(sym.owner, types)) {
		    c = c.owner.enclClass();
		}
	    }
	    return c;
	} else {
	    // the symbol is private
	    return sym.owner.enclClass();
	}
    }

    /** Ensure that identifier is accessible, return tree accessing the identifier.
     *  @param sym      The accessed symbol.
     *  @param tree     The tree referring to the symbol.
     *  @param enclOp   The closest enclosing operation node of tree,
     *                  null if tree is not a subtree of an operation.
     *  @param refSuper Is access via a (qualified) C.super?
     */
    Tree access(Symbol sym, Tree tree, Tree enclOp, boolean refSuper) {
	// Access a free variable via its proxy, or its proxy's proxy
	while (sym.kind == VAR && sym.owner.kind == MTH &&
	    sym.owner.enclClass() != currentClass) {
	    // A constant is replaced by its constant value.
	    Object cv = ((VarSymbol)sym).constValue;
	    if (cv != null) {
		make.at(tree.pos);
		return makeLit(sym.type, cv);
	    }
	    // Otherwise replace the variable by its proxy.
	    sym = proxies.lookup(proxyName(sym.name)).sym;
	    assert sym != null && (sym.flags_field & FINAL) != 0;
	    tree = make.at(tree.pos).Ident(sym);
	}
	Tree base = (tree.tag == Tree.SELECT) ? ((Select)tree).selected : null;
	switch (sym.kind) {
	case TYP:
	    if (sym.owner.kind != PCK) {
		// Convert type idents to
		// <flat name> or <package name> . <flat name>
		Name flatname = Convert.shortName(sym.flatName());
		while (base != null &&
		       TreeInfo.symbol(base) != null &&
		       TreeInfo.symbol(base).kind != PCK) {
		    base = (base.tag == Tree.SELECT)
			? ((Select)base).selected
			: null;
		}
		if (tree.tag == Tree.IDENT) {
		    ((Ident)tree).name = flatname;
		} else if (base == null) {
		    tree = make.at(tree.pos).Ident(sym);
		    ((Ident)tree).name = flatname;
		} else {
		    ((Select)tree).selected = base;
		    ((Select)tree).name = flatname;
		}
	    }
	    break;
	case MTH: case VAR:
	    if (sym.owner.kind == TYP) {

		// Access methods are required for
		//  - private members,
		//  - protected members in a superclass of an
		//    enclosing class contained in another package.
		//  - all non-private members accessed via a qualified super.
		boolean protAccess = refSuper && !needsPrivateAccess(sym)
		    || needsProtectedAccess(sym, tree);
		boolean accReq = protAccess || needsPrivateAccess(sym);

		// A base has to be supplied for
		//  - simple identifiers accessing variables in outer classes.
		boolean baseReq =
		    base == null &&
		    sym.owner != syms.predefClass &&
		    !sym.isMemberOf(currentClass, types);

		if (accReq || baseReq) {
		    make.at(tree.pos);

		    // Constants are replaced by their constant value.
		    if (sym.kind == VAR) {
			Object cv = ((VarSymbol)sym).constValue;
			if (cv != null) return makeLit(sym.type, cv);
		    }

		    // Private variables and methods are replaced by calls
		    // to their access methods.
		    if (accReq) {
			List<Tree> args = Tree.emptyList;
			if ((sym.flags() & STATIC) == 0) {
			    // Instance access methods get instance
			    // as first parameter.
			    if (base == null)
				base = makeOwnerThis(tree.pos, sym, true);
			    args = args.prepend(base);
			    base = null;   // so we don't duplicate code
			}
			Symbol access = accessSymbol(sym, tree,
						     enclOp, protAccess,
						     refSuper);
			Tree receiver = make.Select(
			    base != null ? base : make.QualIdent(access.owner),
			    access);
			return make.App(receiver, args);

		    // Other accesses to members of outer classes get a
		    // qualifier.
		    } else if (baseReq) {
			return make.at(tree.pos).Select(
			    accessBase(tree.pos, sym), sym);
		    }
		}
	    }
	}
	return tree;
    }

    /** Ensure that identifier is accessible, return tree accessing the identifier.
     *  @param tree     The identifier tree.
     */
    Tree access(Tree tree) {
	Symbol sym = TreeInfo.symbol(tree);
	return sym == null ? tree : access(sym, tree, null, false);
    }

    /** Return access constructor for a private constructor,
     *  or the constructor itself, if no access constructor is needed.
     *  @param pos	 The position to report diagnostics, if any.
     *  @param constr    The private constructor.
     */
    Symbol accessConstructor(int pos, Symbol constr) {
	if (needsPrivateAccess(constr)) {
	    ClassSymbol accOwner = constr.owner.enclClass();
	    MethodSymbol aconstr = accessConstrs.get(constr);
	    if (aconstr == null) {
		List<Type> argtypes = constr.type.argtypes();
		if ((accOwner.flags_field & ENUM) != 0)
		    argtypes = argtypes
			.prepend(syms.intType)
			.prepend(syms.stringType);
		aconstr = new MethodSymbol(
		    SYNTHETIC,
		    names.init,
		    new MethodType(
			argtypes.append(
			    accessConstructorTag().erasure(types)),
			constr.type.restype(),
			constr.type.thrown(),
			syms.methodClass),
		    accOwner);
		enterSynthetic(pos, aconstr, accOwner.members());
		accessConstrs.put(constr, aconstr);
		accessed.append(constr);
	    }
	    return aconstr;
	} else {
	    return constr;
	}
    }

    /** Return an anonymous class nested in this toplevel class.
     */
    ClassSymbol accessConstructorTag() {
	ClassSymbol topClass = currentClass.outermostClass();
	Name flatname = names.fromString("" + topClass.fullName() +
                                         target.syntheticNameChar() +
                                         "1");
	ClassSymbol ctag = chk.compiled.get(flatname);
	if (ctag == null)
	    ctag = makeEmptyClass(STATIC | SYNTHETIC, topClass);
	return ctag;
    }

    /** Add all required access methods for a private symbol to enclosing class.
     *  @param sym       The symbol.
     */
    void makeAccessible(Symbol sym) {
	ClassDef cdef = classDef(sym.owner.enclClass());
	assert cdef != null : "class def not found: " + sym + " in " + sym.owner;
	if (sym.name == names.init) {
	    cdef.defs = cdef.defs.prepend(
		accessConstructorDef(cdef.pos, sym, accessConstrs.get(sym)));
	} else {
	    MethodSymbol[] accessors = accessSyms.get(sym);
	    for (int i = 0; i < NCODES; i++) {
		if (accessors[i] != null)
		    cdef.defs = cdef.defs.prepend(
			accessDef(cdef.pos, sym, accessors[i], i));
	    }
	}
    }

    /** Construct definition of an access method.
     *  @param pos        The source code position of the definition.
     *  @param sym        The private or protected symbol.
     *  @param accessor   The access method for the symbol.
     *  @param acode      The access code.
     */
    Tree accessDef(int pos, Symbol vsym, MethodSymbol accessor, int acode) {
//	System.err.println("access " + vsym + " with " + accessor);//DEBUG
	currentClass = vsym.owner.enclClass();
	make.at(pos);
	MethodDef md = make.MethodDef(accessor, null);

	// Find actual symbol
	Symbol sym = actualSymbols.get(vsym);
	if (sym == null) sym = vsym;

	Tree ref;           // The tree referencing the private symbol.
	List<Tree> args;    // Any additional arguments to be passed along.
	if ((sym.flags() & STATIC) != 0) {
	    ref = make.Ident(sym);
	    args = make.Idents(md.params);
	} else {
	    ref = make.Select(make.Ident(md.params.head), sym);
	    args = make.Idents(md.params.tail);
	}
	Tree stat;          // The statement accessing the private symbol.
	if (sym.kind == VAR) {
	    // Normalize out all odd access codes by taking floor modulo 2:
	    int acode1 = acode - (acode & 1);

	    Tree expr;      // The access method's return value.
	    switch (acode1) {
	    case DEREFcode:
		expr = ref;
		break;
	    case ASSIGNcode:
		expr = make.Assign(ref, args.head);
		break;
	    case PREINCcode: case POSTINCcode: case PREDECcode: case POSTDECcode:
		expr = makeUnary(
		    ((acode1 - PREINCcode) >> 1) + Tree.PREINC, ref);
		break;
	    default:
		expr = make.Assignop(
		    treeTag(binaryAccessOperator(acode1)), ref, args.head);
		((Assignop)expr).operator = binaryAccessOperator(acode1);
	    }
	    stat = make.Return(expr.setType(sym.type));
	} else {
	    stat = make.Call(make.App(ref, args));
	}
	md.body = make.Block(0, List.<Tree>make(stat));

	// Make sure all parameters, result types and thrown exceptions
	// are accessible.
	for (List<VarDef> l = md.params; l.nonEmpty(); l = l.tail)
	    l.head.vartype = access(l.head.vartype);
	md.restype = access(md.restype);
	for (List<Tree> l = md.thrown; l.nonEmpty(); l = l.tail)
	    l.head = access(l.head);

	return md;
    }

    /** Construct definition of an access constructor.
     *  @param pos        The source code position of the definition.
     *  @param constr     The private constructor.
     *  @param accessor   The access method for the constructor.
     */
    Tree accessConstructorDef(int pos, Symbol constr, MethodSymbol accessor) {
	make.at(pos);
	MethodDef md = make.MethodDef(accessor,
				      accessor.externalType(types),
				      null);
	Ident callee = make.Ident(names._this);
	callee.sym = constr;
	callee.type = constr.type;
	md.body =
            make.Block(0, List.<Tree>make(
		make.Call(
                    make.App(
                        callee,
		        make.Idents(md.params.reverse().tail.reverse())))));
	return md;
    }

/**************************************************************************
 * Free variables proxies and this$n
 *************************************************************************/

    /** A scope containing all free variable proxies for currently translated
     *  class, as well as its this$n symbol (if needed).
     *  Proxy scopes are nested in the same way classes are.
     *  Inside a constructor, proxies and any this$n symbol are duplicated
     *  in an additional innermost scope, where they represent the constructor
     *  parameters.
     */
    Scope proxies;

    /** A stack containing the this$n field of the currently translated
     *  classes (if needed) in innermost first order.
     *  Inside a constructor, proxies and any this$n symbol are duplicated
     *  in an additional innermost scope, where they represent the constructor
     *  parameters.
     */
    List<VarSymbol> outerThisStack;

    /** The name of a free variable proxy.
     */
    Name proxyName(Name name) {
	return names.fromString("val" + target.syntheticNameChar() + name);
    }

    /** Proxy definitions for all free variables in given list, in reverse order.
     *  @param pos        The source code position of the definition.
     *  @param freevars   The free variables.
     *  @param owner      The class in which the definitions go.
     */
    List<VarDef> freevarDefs(int pos, List<VarSymbol> freevars, Symbol owner) {
	long flags = FINAL | SYNTHETIC;
	if (owner.kind == TYP &&
	    target.usePrivateSyntheticFields())
	    flags |= PRIVATE;
	List<VarDef> defs = VarDef.emptyList;
	for (List<VarSymbol> l = freevars; l.nonEmpty(); l = l.tail) {
	    VarSymbol v = l.head;
	    VarSymbol proxy = new VarSymbol(
		flags, proxyName(v.name), v.erasure(types), owner);
	    proxies.enter(proxy);
	    VarDef vd = make.at(pos).VarDef(proxy, null);
	    vd.vartype = access(vd.vartype);
	    defs = defs.prepend(vd);
	}
	return defs;
    }

    /** The name of a this$n field
     *  @param target   The class referenced by the this$n field
     */
    Name outerThisName(Type type, Symbol owner) {
	Type t = type.outer();
	int nestingLevel = 0;
	while (t.tag == CLASS) {
	    t = t.outer();
	    nestingLevel++;
	}
	Name result = names.fromString("this" + target.syntheticNameChar() + nestingLevel);
	while (owner.kind == TYP && ((ClassSymbol)owner).members().lookup(result).scope != null)
	    result = names.fromString(result.toString() + target.syntheticNameChar());
	return result;
    }

    /** Definition for this$n field.
     *  @param pos        The source code position of the definition.
     *  @param owner      The class in which the definition goes.
     */
    VarDef outerThisDef(int pos, Symbol owner) {
	long flags = FINAL | SYNTHETIC;
	if (owner.kind == TYP &&
	    target.usePrivateSyntheticFields())
	    flags |= PRIVATE;
	Type target = types.erasure(owner.enclClass().type.outer());
	VarSymbol outerThis = new VarSymbol(
	    flags, outerThisName(target, owner), target, owner);
	outerThisStack = outerThisStack.prepend(outerThis);
	VarDef vd = make.at(pos).VarDef(outerThis, null);
	vd.vartype = access(vd.vartype);
	return vd;
    }

    /** Return a list of trees that load the free variables in given list,
     *  in reverse order.
     *  @param pos          The source code position to be used for the trees.
     *  @param freevars     The list of free variables.
     */
    List<Tree> loadFreevars(int pos, List<VarSymbol> freevars) {
	List<Tree> args = Tree.emptyList;
	for (List<VarSymbol> l = freevars; l.nonEmpty(); l = l.tail)
	    args = args.prepend(loadFreevar(pos, l.head));
	return args;
    }
//where
	Tree loadFreevar(int pos, VarSymbol v) {
	    return access(v, make.at(pos).Ident(v), null, false);
	}

    /** Construct a tree simulating the expression <C.this>.
     *  @param pos           The source code position to be used for the tree.
     *  @param c             The qualifier class.
     */
    Tree makeThis(int pos, TypeSymbol c) {
	if (currentClass == c) {
	    // in this case, `this' works fine
	    return make.at(pos).This(c.erasure(types));
        } else {
	    // need to go via this$n
	    return makeOuterThis(pos, c);
	}
    }

    /** Construct a tree that represents the outer instance
     *  <C.this>. Never pick the current `this'.
     *  @param pos           The source code position to be used for the tree.
     *  @param c             The qualifier class.
     */
    Tree makeOuterThis(int pos, TypeSymbol c) {
	List<VarSymbol> ots = outerThisStack;
	if (ots.isEmpty()) {
	    log.error(pos, "no.encl.instance.of.type.in.scope", c);
	    assert false;
	    return make.Ident(syms.nullConst);
	}
	VarSymbol ot = ots.head;
	Tree tree = access(make.at(pos).Ident(ot));
	TypeSymbol otc = ot.type.tsym;
	while (otc != c) {
	    do {
		ots = ots.tail;
		if (ots.isEmpty()) {
		    log.error(pos,
			      "no.encl.instance.of.type.in.scope",
			      c);
		    assert false; // should have been caught in Attr
		    return tree;
		}
		ot = ots.head;
	    } while (ot.owner != otc);
	    if (otc.owner.kind != PCK && !otc.hasOuterInstance()) {
		chk.earlyRefError(pos, c);
		assert false; // should have been caught in Attr
		return make.Ident(syms.nullConst);
	    }
	    tree = access(make.at(pos).Select(tree, ot));
	    otc = ot.type.tsym;
	}
	return tree;
    }

    /** Construct a tree that represents the closest outer instance
     *  <C.this> such that the given symbol is a member of C.
     *  @param pos           The source code position to be used for the tree.
     *  @param sym           The accessed symbol.
     *  @param preciseMatch  should we accept a type that is a subtype of
     *                       sym's owner, even if it doesn't contain sym
     *                       due to hiding, overriding, or non-inheritance
     *                       due to protection?
     */
    Tree makeOwnerThis(int pos, Symbol sym, boolean preciseMatch) {
	Symbol c = sym.owner;
	if (preciseMatch ? sym.isMemberOf(currentClass, types) : currentClass.isSubClass(sym.owner, types)) {
	    // in this case, `this' works fine
	    return make.at(pos).This(c.erasure(types));
	} else {
	    // need to go via this$n
	    List<VarSymbol> ots = outerThisStack;
	    if (ots.isEmpty()) {
		log.error(pos, "no.encl.instance.of.type.in.scope", c);
		assert false;
		return make.Ident(syms.nullConst);
	    }
	    VarSymbol ot = ots.head;
	    Tree tree = access(make.at(pos).Ident(ot));
	    TypeSymbol otc = ot.type.tsym;
	    while (!(preciseMatch ? sym.isMemberOf(otc, types) : otc.isSubClass(sym.owner, types))) {
		do {
		    ots = ots.tail;
		    if (ots.isEmpty()) {
			log.error(pos,
				  "no.encl.instance.of.type.in.scope",
				  c);
			assert false;
			return tree;
		    }
		    ot = ots.head;
		} while (ot.owner != otc);
		tree = access(make.at(pos).Select(tree, ot));
		otc = ot.type.tsym;
	    }
	    return tree;
	}
    }

    /** Return tree simulating the the assignment <this.name = name>, where
     *  name is the name of a free variable.
     */
    Tree initField(int pos, Name name) {
	Scope.Entry e = proxies.lookup(name);
	Symbol rhs = e.sym;
	assert rhs.owner.kind == MTH;
	Symbol lhs = e.next().sym;
	assert rhs.owner.owner == lhs.owner;
	make.at(pos);
	return
	    make.Exec(
		make.Assign(
		    make.Select(make.This(lhs.owner.erasure(types)), lhs),
		    make.Ident(rhs)).setType(lhs.erasure(types)));
    }

    /** Return tree simulating the the assignment <this.this$n = this$n>.
     */
    Tree initOuterThis(int pos) {
	VarSymbol rhs = outerThisStack.head;
	assert rhs.owner.kind == MTH;
	VarSymbol lhs = outerThisStack.tail.head;
	assert rhs.owner.owner == lhs.owner;
	make.at(pos);
	return
	    make.Exec(
		make.Assign(
		    make.Select(make.This(lhs.owner.erasure(types)), lhs),
		    make.Ident(rhs)).setType(lhs.erasure(types)));
    }

/**************************************************************************
 * Code for .class
 *************************************************************************/

    /** Return the symbol of a class to contain a cache of
     *  compiler-generated statics such as class$ and the
     *  $assertionsDisabled flag.  We create an anonymous nested class
     *  (unless one already exists) and return its symbol.  However,
     *  for backward compatibility in 1.4 and earlier we use the
     *  top-level class itself.
     */
    private ClassSymbol outerCacheClass() {
	ClassSymbol clazz = outermostClassDef.sym;
	if ((clazz.flags() & INTERFACE) == 0 &&
	    !target.useInnerCacheClass()) return clazz;
	Scope s = clazz.members();
	for (Scope.Entry e = s.elems; e != null; e = e.sibling)
	    if (e.sym.kind == TYP &&
		e.sym.name == names.empty &&
		(e.sym.flags() & INTERFACE) == 0) return (ClassSymbol) e.sym;
	return makeEmptyClass(STATIC | SYNTHETIC, clazz);
    }

    /** Return symbol for "class$" method. If there is no method definition
     *  for class$, construct one as follows:
     *
     *    class class$(String x0) {
     *      try {
     *        return Class.forName(x0);
     *      } catch (ClassNotFoundException x1) {
     *        throw new NoClassDefFoundError(x1.getMessage());
     *      }
     *    }
     */
    private MethodSymbol classDollarSym(int pos) {
	ClassSymbol outerCacheClass = outerCacheClass();
	MethodSymbol classDollarSym =
	    (MethodSymbol)lookupSynthetic(classDollar,
					  outerCacheClass.members());
	if (classDollarSym == null) {
	    classDollarSym = new MethodSymbol(
		STATIC | SYNTHETIC,
		classDollar,
		new MethodType(
		    Type.emptyList.prepend(syms.stringType),
		    types.erasure(syms.classType),
		    Type.emptyList,
		    syms.methodClass),
		outerCacheClass);
	    enterSynthetic(pos, classDollarSym, outerCacheClass.members());

	    MethodDef md = make.MethodDef(classDollarSym, null);
	    try {
		md.body = classDollarSymBody(pos, md);
	    } catch (CompletionFailure ex) {
		md.body = make.Block(0, Tree.emptyList);
		chk.completionError(pos, ex);
	    }
            ClassDef outerCacheClassDef = classDef(outerCacheClass);
            outerCacheClassDef.defs = outerCacheClassDef.defs.prepend(md);
	}
	return classDollarSym;
    }

    /** Generate code for class$(String name). */
    Block classDollarSymBody(int pos, MethodDef md) {
	MethodSymbol classDollarSym = md.sym;
	ClassSymbol outerCacheClass = (ClassSymbol)classDollarSym.owner;

	Tree returnResult;

	// in 1.4.2 and above, we use
	// Class.forName(String name, boolean init, ClassLoader loader);
	// which requires we cache the current loader in cl$
	if (target.classLiteralsNoInit()) {
	    // clsym = "private static ClassLoader cl$"
	    VarSymbol clsym = new VarSymbol(STATIC|SYNTHETIC,
					    names.fromString("cl" + target.syntheticNameChar()),
					    syms.classLoaderType,
					    outerCacheClass);
	    enterSynthetic(pos, clsym, outerCacheClass.members());

	    // emit "private static ClassLoader cl$;"
	    VarDef cldef = make.VarDef(clsym, null);
	    ClassDef outerCacheClassDef = classDef(outerCacheClass);
	    outerCacheClassDef.defs = outerCacheClassDef.defs.prepend(cldef);

	    // newcache := "new cache$1[0]"
	    Tree newcache = make.
		NewArray(make.Type(outerCacheClass.type),
			 Tree.emptyList.
			 prepend(make.Literal(INT, 0).
				 setType(syms.intType)),
			 null);
	    newcache.type = new ArrayType(types.erasure(outerCacheClass.type),
					  syms.arrayClass);

	    // forNameSym := java.lang.Class.forName(
	    //     String s,boolean init,ClassLoader loader)
	    Symbol forNameSym = lookupMethod(make.pos, names.forName,
					     types.erasure(syms.classType),
					     Type.emptyList
					     .prepend(syms.classLoaderType)
					     .prepend(syms.booleanType)
					     .prepend(syms.stringType));
	    // clvalue := "(cl$ == null) ?
	    // $newcache.getClass().getComponentType().getClassLoader() : cl$"
	    Tree clvalue =
		make.Conditional(
		    makeBinary(Tree.EQ, make.Ident(clsym),
			       make.Ident(syms.nullConst)),
		    make.Assign(
			make.Ident(clsym),
			makeCall(
			    makeCall(makeCall(newcache,
					      names.getClass,
					      Tree.emptyList),
				     names.getComponentType,
				     Tree.emptyList),
			    names.getClassLoader,
			    Tree.emptyList)).setType(syms.classLoaderType),
		    make.Ident(clsym)).setType(syms.classLoaderType);
				
	    // returnResult := "{ return Class.forName(param1, false, cl$); }"
	    List<Tree> args = Tree.emptyList.
		prepend(clvalue).
		prepend(make.Ident(syms.falseConst)).
		prepend(make.Ident(md.params.head.sym));
	    returnResult = make.
		Block(0, Tree.emptyList.
		      prepend(make.
			      Call(make. // return
				   App(make.
				       Ident(forNameSym), args))));
	} else {
	    // forNameSym := java.lang.Class.forName(String s)
	    Symbol forNameSym = lookupMethod(make.pos,
					     names.forName,
					     types.erasure(syms.classType),
					     Type.emptyList.prepend(syms.stringType));
	    // returnResult := "{ return Class.forName(param1); }"
	    returnResult = make.
		Block(0, Tree.emptyList.
		      prepend(make.
			      Call(make. // return
				   App(make.
				       QualIdent(forNameSym),
				       List.<Tree>make(make.
						 Ident(md.params.
						       head.sym))))));
	}

	// catchParam := ClassNotFoundException e1
	VarSymbol catchParam =
	    new VarSymbol(0, make.paramName(1),
			  syms.classNotFoundExceptionType,
			  classDollarSym);

	Tree rethrow;
	if (target.hasInitCause()) {
	    // rethrow = "throw new NoClassDefFoundError().initCause(e);
	    Tree throwExpr =
		makeCall(makeNewClass(syms.noClassDefFoundErrorType,
				      Tree.emptyList),
			 names.initCause,
			 Tree.emptyList.prepend(make.Ident(catchParam)));
	    rethrow = make.Throw(throwExpr);
	} else {
	    // getMessageSym := ClassNotFoundException.getMessage()
	    Symbol getMessageSym = lookupMethod(make.pos,
						names.getMessage,
						syms.classNotFoundExceptionType,
						Type.emptyList);
	    // rethrow = "throw new NoClassDefFoundError(e.getMessage());"
	    rethrow = make.
		Throw(makeNewClass(syms.noClassDefFoundErrorType,
				   List.
				   make(make.
					App(make.
					    Select(make.
						   Ident(catchParam),
						   getMessageSym),
					    Tree.emptyList))));
	}

	// rethrowStmt := "( $rethrow )"
	Tree rethrowStmt = make.Block(0, Tree.emptyList.prepend(rethrow));

	// catchBlock := "catch ($catchParam) $rethrowStmt"
	Catch catchBlock = make.Catch(make.VarDef(catchParam, null),
				      rethrowStmt);

	// tryCatch := "try $returnResult $catchBlock"
	Tree tryCatch = make.Try(returnResult,
				 Catch.emptyList.prepend(catchBlock), null);

	return make.Block(0, Tree.emptyList.prepend(tryCatch));
    }
    // where
        /** Create an attributed tree of the form left.name(). */
        private Tree makeCall(Tree left, Name name, List<Tree> args) {
	    assert left.type != null;
	    List<Type> types = Type.emptyList;
	    Symbol funcsym = lookupMethod(make.pos, name, left.type,
					  TreeInfo.types(args));
	    return make.App(make.Select(left, funcsym), args);
	}

    /** The Name Of The variable to cache T.class values.
     *  @param sig      The signature of type T.
     */
    private Name cacheName(String sig) {
	StringBuffer buf = new StringBuffer();
	if (sig.startsWith("[")) {
	    buf = buf.append("array");
	    while (sig.startsWith("[")) {
		buf = buf.append(target.syntheticNameChar());
		sig = sig.substring(1);
	    }
	    if (sig.startsWith("L")) {
		sig = sig.substring(0, sig.length() - 1);
	    }
	} else {
	    buf = buf.append("class" + target.syntheticNameChar());
	}
	buf = buf.append(sig.replace('.', target.syntheticNameChar()));
	return names.fromString(buf.toString());
    }

    /** The variable symbol that caches T.class values.
     *  If none exists yet, create a definition.
     *  @param sig      The signature of type T.
     *  @param pos	The position to report diagnostics, if any.
     */
    private VarSymbol cacheSym(int pos, String sig) {
	ClassSymbol outerCacheClass = outerCacheClass();
	Name cname = cacheName(sig);
	VarSymbol cacheSym =
	    (VarSymbol)lookupSynthetic(cname, outerCacheClass.members());
	if (cacheSym == null) {
	    cacheSym = new VarSymbol(
		STATIC | SYNTHETIC, cname, types.erasure(syms.classType), outerCacheClass);
	    enterSynthetic(pos, cacheSym, outerCacheClass.members());

	    VarDef cacheDef = make.VarDef(cacheSym, null);
            ClassDef outerCacheClassDef = classDef(outerCacheClass);
            outerCacheClassDef.defs = outerCacheClassDef.defs.prepend(cacheDef);
	}
	return cacheSym;
    }

    /** The tree simulating a T.class expression.
     *  @param clazz      The tree identifying type T.
     */
    private Tree classOf(Tree clazz) {
	return classOfType(clazz.type, clazz.pos);
    }

    private Tree classOfType(Type type, int pos) {
	switch (type.tag) {
	case BYTE: case SHORT: case CHAR: case INT: case LONG: case FLOAT:
	case DOUBLE: case BOOLEAN: case VOID:
	    // replace with <BoxedClass>.TYPE
	    ClassSymbol c = types.boxedClass(type);
	    Symbol typeSym =
		rs.access(
		    rs.findIdentInType(attrEnv, c.type, names.TYPE, VAR),
		    pos, c.type, names.TYPE, true);
	    if (typeSym.kind == VAR) attr.evalInit((VarSymbol)typeSym);
	    return make.QualIdent(typeSym);
	case CLASS: case ARRAY:
	    if (target.hasClassLiterals()) {
		VarSymbol sym = new VarSymbol(
			STATIC | PUBLIC | FINAL, names._class,
			syms.classType, type.tsym);
		return make.at(pos).Select(make.Type(type), sym);
	    }
	    // replace with <cache == null ? cache = class$(tsig) : cache>
	    // where
	    //  - <tsig>  is the type signature of T,
	    //  - <cache> is the cache variable for tsig.
	    String sig =
		writer.xClassName(type).toString().replace('/', '.');
	    Symbol cs = cacheSym(pos, sig);
	    return make.at(pos).Conditional(
		makeBinary(Tree.EQ, make.Ident(cs), make.Ident(syms.nullConst)),
		make.Assign(
		    make.Ident(cs),
		    make.App(
			make.Ident(classDollarSym(pos)),
			List.<Tree>make(
                            make.Literal(CLASS, sig).setType(syms.stringType))))
		.setType(types.erasure(syms.classType)),
		make.Ident(cs)).setType(types.erasure(syms.classType));
	default:
	    throw new AssertionError();
	}
    }

/**************************************************************************
 * Code for enabling/disabling assertions.
 *************************************************************************/

    // This code is not particularly robust if the user has
    // previously declared a member named '$assertionsDisabled'.
    // The same faulty idiom also appears in the translation of
    // class literals above.  We should report an error if a
    // previous declaration is not synthetic.

    private Tree assertFlagTest(int pos) {
	// Outermost class may be either true class or an interface.
	ClassSymbol outermostClass = outermostClassDef.sym;

	// note that this is a class, as an interface can't contain a statement.
	ClassSymbol container = currentClass;

	VarSymbol assertDisabledSym =
	    (VarSymbol)lookupSynthetic(dollarAssertionsDisabled,
				       container.members());
	if (assertDisabledSym == null) {
	    assertDisabledSym =
		new VarSymbol(STATIC | FINAL | SYNTHETIC,
			      dollarAssertionsDisabled,
			      syms.booleanType,
			      container);
	    enterSynthetic(pos, assertDisabledSym, container.members());
	    Symbol desiredAssertionStatusSym = lookupMethod(pos,
							    names.desiredAssertionStatus,
							    types.erasure(syms.classType),
							    Type.emptyList);
	    ClassDef containerDef = classDef(container);
	    make.at(containerDef.pos);
	    Tree notStatus = makeUnary(Tree.NOT, make.App(make.Select(
                    classOfType(types.erasure(outermostClass.type),
				containerDef.pos),
		    desiredAssertionStatusSym), Tree.emptyList));
	    VarDef assertDisabledDef = make.VarDef(assertDisabledSym,
						   notStatus);
	    containerDef.defs = containerDef.defs.prepend(assertDisabledDef);
	}
	make.at(pos);
	return makeUnary(Tree.NOT, make.Ident(assertDisabledSym));
    }


/**************************************************************************
 * Building blocks for let expressions
 *************************************************************************/

    interface TreeBuilder {
	Tree build(Tree arg);
    }

    /** Construct an expression using the builder, with the given rval
     *  expression as an argument to the builder.  However, the rval
     *  expression must be computed only once, even if used multiple
     *  times in the result of the builder.  We do that by
     *  constructing a "let" expression that saves the rvalue into a
     *  temporary variable and then uses the temporary variable in
     *  place of the expression built by the builder.  The complete
     *  resulting expression is of the form
     *  <pre>
     *    (let <b>TYPE</b> <b>TEMP</b> = <b>RVAL</b>;
     *     in (<b>BUILDER</b>(<b>TEMP</b>)))
     *  </pre>
     *  where <code><b>TEMP</b></code> is a newly declared variable
     *  in the let expression.
     */
    Tree abstractRval(Tree rval, Type type, TreeBuilder builder) {
	rval = TreeInfo.skipParens(rval);
	switch (rval.tag) {
	case Tree.LITERAL:
	    return builder.build(rval);
	case Tree.IDENT:
	    Ident id = (Ident) rval;
	    if ((id.sym.flags() & FINAL) != 0 && id.sym.owner.kind == MTH)
		return builder.build(rval);
	}
	VarSymbol var =
	    new VarSymbol(FINAL|SYNTHETIC,
			  Name.fromString(names,
					  target.syntheticNameChar()
					  + "" + rval.hashCode()),
				      type,
				      currentMethodSym);
	VarDef def = make.VarDef(var, rval);
	Tree built = builder.build(make.Ident(var));
	Tree res = make.LetExpr(def, built);
	res.type = built.type;
	return res;
    }

    // same as above, with the type of the temporary variable computed
    Tree abstractRval(Tree rval, TreeBuilder builder) {
	return abstractRval(rval, rval.type, builder);
    }

    // same as above, but for an expression that may be used as either
    // an rvalue or an lvalue.  This requires special handling for
    // Select expressions, where we place the left-hand-side of the
    // select in a temporary, and for Indexed expressions, where we
    // place both the indexed expression and the index value in temps.
    Tree abstractLval(Tree lval, final TreeBuilder builder) {
	lval = TreeInfo.skipParens(lval);
	switch (lval.tag) {
	case Tree.IDENT:
	    return builder.build(lval);
	case Tree.SELECT: {
	    final Select s = (Select)lval;
	    Tree selected = TreeInfo.skipParens(s.selected);
	    Symbol lid = TreeInfo.symbol(s.selected);
	    if (lid != null && lid.kind == TYP) return builder.build(lval);
	    return abstractRval(s.selected, new TreeBuilder() {
		    public Tree build(final Tree selected) {
			return builder.build(make.Select(selected, s.sym));
		    }
		});
	}
	case Tree.INDEXED: {
	    final Indexed i = (Indexed)lval;
	    return abstractRval(i.indexed, new TreeBuilder() {
		    public Tree build(final Tree indexed) {
			return abstractRval(i.index, syms.intType, new TreeBuilder() {
				public Tree build(final Tree index) {
				    Tree newLval = make.Indexed(indexed, index);
				    newLval.setType(i.type);
				    return builder.build(newLval);
				}
			    });
		    }
		});
	}
	}
	throw new AssertionError(lval);
    }

    // evaluate and discard the first expression, then evaluate the second.
    Tree makeComma(final Tree expr1, final Tree expr2) {
	return abstractRval(expr1, new TreeBuilder() {
		public Tree build(final Tree discarded) {
		    return expr2;
		}
	    });
    }

/**************************************************************************
 * Translation methods
 *************************************************************************/

    /** Visitor argument: enclosing operator node.
     */
    private Tree enclOp;

    /** Visitor method: Translate a single node.
     *  Attach the source position from the old tree to its replacement tree.
     */
    public Tree translate(Tree tree) {
	if (tree == null) {
	    return null;
	} else {
	    make.at(tree.pos);
	    tree.accept(this); // inlined super.translate(tree)
	    if (endPositions != null && result != tree) {
		Integer endPos = endPositions.remove(tree);
		if (endPos != null) endPositions.put(result, endPos);
	    }
	    return result;
	}
    }

    /** Visitor method: Translate a single node, boxing or unboxing if needed.
     */
    public Tree translate(Tree tree, Type type) {
	return (tree == null) ? null : boxIfNeeded(translate(tree), type);
    }

    /** Visitor method: Translate tree.
     */
    public Tree translate(Tree tree, Tree enclOp) {
	Tree prevEnclOp = this.enclOp;
	this.enclOp = enclOp;
	Tree res = translate(tree);
	this.enclOp = prevEnclOp;
	return res;
    }

    /** Visitor method: Translate list of trees.
     */
    public List<Tree> translate(List<Tree> trees, Tree enclOp) {
	Tree prevEnclOp = this.enclOp;
	this.enclOp = enclOp;
	List<Tree> res = translate(trees);
	this.enclOp = prevEnclOp;
	return res;
    }

    /** Visitor method: Translate list of trees.
     */
    public List<Tree> translate(List<Tree> trees, Type type) {
	if (trees == null) return null;
	for (List<Tree> l = trees; l.nonEmpty(); l = l.tail)
	    l.head = translate(l.head, type);
	return trees;
    }

    public void visitTopLevel(TopLevel tree) {
	if (tree.packageAnnotations.nonEmpty()) {
	    Name name = names.fromString("package-info");
	    long flags = Flags.SYNTHETIC | Flags.INTERFACE;
	    ClassDef packageAnnotationsClass = make.ClassDef(make.Modifiers(flags,
									    tree.packageAnnotations),
							     name, TypeParameter.emptyList,
							     null, Tree.emptyList, Tree.emptyList);
	    ClassSymbol c = reader.defineClass(name, tree.packge);
	    c.flatname = names.fromString(tree.packge + "." + name);
	    c.sourcefile = tree.sourcefile;
	    c.completer = null;
	    c.members_field = new Scope(c);
	    c.flags_field = flags;
	    c.attributes_field = tree.packge.attributes_field;
	    tree.packge.attributes_field = Attribute.Compound.emptyList;
	    ClassType ctype = (ClassType) c.type;
	    ctype.supertype_field = syms.objectType;
	    ctype.interfaces_field = Type.emptyList;
	    packageAnnotationsClass.sym = c;
	    

	    translated.append(packageAnnotationsClass);
	}
    }

    public void visitClassDef(ClassDef tree) {
	ClassSymbol currentClassPrev = currentClass;
	MethodSymbol currentMethodSymPrev = currentMethodSym;
	currentClass = tree.sym;
	currentMethodSym = null;
	classdefs.put(currentClass, tree);

	proxies = proxies.dup(currentClass);
	List<VarSymbol> prevOuterThisStack = outerThisStack;

	// If this is an enum definition
	if ((tree.mods.flags & ENUM) != 0 &&
	    (types.supertype(currentClass.type).tsym.flags() & ENUM) == 0)
	    visitEnumDef(tree);

	// If this is a nested class, define a this$n field for
	// it and add to proxies.
	VarDef otdef = null;
	if (currentClass.hasOuterInstance())
	    otdef = outerThisDef(tree.pos, currentClass);

	// If this is a local class, define proxies for all its free variables.
	List<VarDef> fvdefs = freevarDefs(
	    tree.pos, freevars(currentClass), currentClass);

	// Recursively translate superclass, interfaces.
	tree.extending = translate(tree.extending);
	tree.implementing = translate(tree.implementing);

	// Recursively translate members, taking into account that new members
	// might be created during the translation and prepended to the member
	// list `tree.defs'.
	List<Tree> seen = Tree.emptyList;
	while (tree.defs != seen) {
	    List<Tree> unseen = tree.defs;
	    for (List<Tree> l = unseen; l.nonEmpty() && l != seen; l = l.tail) {
		Tree outermostMemberDefPrev = outermostMemberDef;
		if (outermostMemberDefPrev == null) outermostMemberDef = l.head;
		l.head = translate(l.head);
		outermostMemberDef = outermostMemberDefPrev;
	    }
	    seen = unseen;
	}

	// Convert a protected modifier to public, mask static modifier.
	if ((tree.mods.flags & PROTECTED) != 0) tree.mods.flags |= PUBLIC;
	tree.mods.flags &= ClassFlags;

	// Convert name to flat representation, replacing '.' by '$'.
	tree.name = Convert.shortName(currentClass.flatName());

	// Add this$n and free variables proxy definitions to class.
	for (List<VarDef> l = fvdefs; l.nonEmpty(); l = l.tail) {
	    tree.defs = tree.defs.prepend(l.head);
	    enterSynthetic(tree.pos, l.head.sym, currentClass.members());
	}
	if (currentClass.hasOuterInstance()) {
	    tree.defs = tree.defs.prepend(otdef);
	    enterSynthetic(tree.pos, otdef.sym, currentClass.members());
	}

	proxies = proxies.leave();
	outerThisStack = prevOuterThisStack;

	// Append translated tree to `translated' queue.
	translated.append(tree);

	currentClass = currentClassPrev;
	currentMethodSym = currentMethodSymPrev;

	// Return empty block {} as a placeholder for an inner class.
	result = make.at(tree.pos).Block(0, Tree.emptyList);
    }

    /** Translate an enum class. */
    private void visitEnumDef(ClassDef tree) {
	make.at(tree.pos);

	// add the supertype, if needed
	if (tree.extending == null)
	    tree.extending = make.Type(types.supertype(tree.type));

	// process each enumeration constant, adding implicit constructor parameters
	int nextOrdinal = 0;
	ListBuffer<Tree> values = new ListBuffer<Tree>();
	ListBuffer<Tree> enumDefs = new ListBuffer<Tree>();
	ListBuffer<Tree> otherDefs = new ListBuffer<Tree>();
	for (List<Tree> defs = tree.defs;
	     defs.nonEmpty();
	     defs=defs.tail) {
	    if (defs.head.tag == Tree.VARDEF && (((VarDef)defs.head).mods.flags & ENUM) != 0) {
		VarDef var = (VarDef)defs.head;
		visitEnumConstantDef(var, nextOrdinal++);
		values.append(make.QualIdent(var.sym));
		enumDefs.append(var);
	    } else {
		otherDefs.append(defs.head);
	    }
	}

	// private static final T[] #VALUES = { a, b, c };
	Name valuesName = names.fromString(target.syntheticNameChar() + "VALUES");
        while (tree.sym.members().lookup(valuesName).scope != null) // avoid name clash
            valuesName = names.fromString(valuesName + "" + target.syntheticNameChar());
	Type arrayType = new ArrayType(types.erasure(tree.type), syms.arrayClass);
	VarSymbol valuesVar = new VarSymbol(PRIVATE|FINAL|STATIC|SYNTHETIC,
					    valuesName,
					    arrayType,
					    tree.type.tsym);
	NewArray newArray = make.NewArray(make.Type(types.erasure(tree.type)),
					  Tree.emptyList,
					  values.toList());
	newArray.type = arrayType;
	enumDefs.append(make.VarDef(valuesVar, newArray));
	tree.sym.members().enter(valuesVar);

	Symbol valuesSym = lookupMethod(tree.pos, names.values,
					tree.type, Type.emptyList);
	Tree valuesResult =
	    make.TypeCast(valuesSym.type.restype(),
			  make.App(make.Select(make.Ident(valuesVar),
					       syms.arrayCloneMethod),
				   Tree.emptyList));
	MethodDef valuesDef =
	    make.MethodDef((MethodSymbol)valuesSym,
			   make.Block(0, Tree.emptyList
				      .prepend(make.Return(valuesResult))));
	enumDefs.append(valuesDef);

	// public static E valueOf(String name) {
	//     for (E #e : VALUES)
	//         if (#e.name().equals(name))
	//             return #e;
	//     throw new IllegalArgumentException(name);
	// }
	Symbol valueOfSym = lookupMethod(tree.pos,
					 names.valueOf,
					 tree.sym.type,
					 Type.emptyList.prepend(syms.stringType));
	assert (valueOfSym.flags() & STATIC) != 0;
	VarSymbol nameArgSym = ((MethodSymbol)valueOfSym).params.head;
	Ident nameVal = (Ident)make.Ident(nameArgSym);
	VarSymbol eSym = new VarSymbol(SYNTHETIC|FINAL,
				       names.fromString(target.syntheticNameChar() + "E"),
				       tree.type,
				       valueOfSym);
	Tree returnE = make.Return(make.Ident(eSym));
	Tree eName = makeCall(make.Ident(eSym), names._name, Tree.emptyList);
	Tree condition = makeCall(eName, names.fromString("equals"), Tree.emptyList.prepend(nameVal));
	Tree ifStmt = make.If(condition, returnE, null);
	Tree foreachLoop = make.ForeachLoop(make.VarDef(eSym, null), make.Ident(valuesVar), ifStmt);
	NewClass newclass = make.NewClass(null,
				      null,
				      make.Ident(syms.illegalArgumentExceptionType.tsym),
				      Tree.emptyList.prepend(nameVal),
				      null);
	newclass.type = syms.illegalArgumentExceptionType;
	newclass.constructor = lookupMethod(tree.pos,
					    names.init,
					    syms.illegalArgumentExceptionType,
					    Type.emptyList.prepend(syms.stringType));
	Tree throwStatement = make.Throw(newclass);
	MethodDef valueOf = make.MethodDef((MethodSymbol)valueOfSym,
					   make.Block(0, Tree.emptyList.
						      prepend(throwStatement).
						      prepend(foreachLoop)));
	nameVal.sym = valueOf.params.head.sym;
	enumDefs.append(valueOf);

	enumDefs.appendList(otherDefs.toList());
	tree.defs = enumDefs.toList();

        // Add the necessary members for the EnumCompatibleMode
        if (target.compilerBootstrap(tree.sym)) {
            addEnumCompatibleMembers(tree);
        }
    }

    /** Translate an enumeration constant and its initializer. */
    private void visitEnumConstantDef(VarDef var, int ordinal) {
	NewClass varDef = (NewClass)var.init;
	varDef.args = varDef.args.
	    prepend(makeLit(syms.intType, ordinal)).
	    prepend(makeLit(syms.stringType, var.name.toString()));
    }

    public void visitMethodDef(MethodDef tree) {
	if (tree.name == names.init && (currentClass.flags_field&ENUM) != 0) {
	    // Add "String $enum$name, int $enum$ordinal" to the beginning of the
	    // argument list for each constructor of an enum.
	    VarDef nameParam = make.at(tree.pos).
		Param(names.fromString(target.syntheticNameChar() +
				       "enum" + target.syntheticNameChar() + "name"),
		      syms.stringType, tree.sym);
	    nameParam.mods.flags |= SYNTHETIC; nameParam.sym.flags_field |= SYNTHETIC;

	    VarDef ordParam = make.
		Param(names.fromString(target.syntheticNameChar() +
				       "enum" + target.syntheticNameChar() +
				       "ordinal"),
		      syms.intType, tree.sym);
	    ordParam.mods.flags |= SYNTHETIC; ordParam.sym.flags_field |= SYNTHETIC;

	    tree.params = tree.params.prepend(ordParam).prepend(nameParam);

	    MethodSymbol m = tree.sym;
	    Type olderasure = m.erasure(types);
	    m.erasure_field = new MethodType(
		olderasure.argtypes().prepend(syms.intType).prepend(syms.stringType),
		olderasure.restype(),
		olderasure.thrown(),
		syms.methodClass);

            if (target.compilerBootstrap(m.owner)) {
                // Initialize synthetic name field
                Symbol nameVarSym = lookupSynthetic(names.fromString("$name"),
                                                    tree.sym.owner.members());
                Tree nameIdent = make.Ident(nameParam.sym);
                Tree id1 = make.Ident(nameVarSym);
                Assign newAssign = make.Assign(id1, nameIdent);
                newAssign.type = id1.type;
                Exec nameAssign = make.Exec(newAssign);
                nameAssign.type = id1.type;
                tree.body.stats = tree.body.stats.prepend(nameAssign);

                // Initialize synthetic ordinal field
                Symbol ordinalVarSym = lookupSynthetic(names.fromString("$ordinal"),
                                                       tree.sym.owner.members());
                Tree ordIdent = make.Ident(ordParam.sym);
                id1 = make.Ident(ordinalVarSym);
                newAssign = make.Assign(id1, ordIdent);
                newAssign.type = id1.type;
                Exec ordinalAssign = make.Exec(newAssign);
                ordinalAssign.type = id1.type;
                tree.body.stats = tree.body.stats.prepend(ordinalAssign);
            }
	}

	MethodDef prevMethodDef = currentMethodDef;
	MethodSymbol prevMethodSym = currentMethodSym;
	try {
	    currentMethodDef = tree;
	    currentMethodSym = tree.sym;
	    visitMethodDefInternal(tree);
	} finally {
	    currentMethodDef = prevMethodDef;
	    currentMethodSym = prevMethodSym;
	}
    }
    //where
    private void visitMethodDefInternal(MethodDef tree) {
	if (tree.name == names.init &&
	    (currentClass.isInner() ||
	     (currentClass.owner.kind & (VAR | MTH)) != 0)) {
	    // We are seeing a constructor of an inner class.
	    MethodSymbol m = tree.sym;

	    // Push a new proxy scope for constructor parameters.
	    // and create definitions for any this$n and proxy parameters.
	    proxies = proxies.dup(m);
	    List<VarSymbol> prevOuterThisStack = outerThisStack;
	    List<VarSymbol> fvs = freevars(currentClass);
	    VarDef otdef = null;
	    if (currentClass.hasOuterInstance())
		otdef = outerThisDef(tree.pos, m);
	    List<VarDef> fvdefs = freevarDefs(tree.pos, fvs, m);

	    // Recursively translate result type, parameters and thrown list.
	    tree.restype = translate(tree.restype);
	    tree.params = translateVarDefs(tree.params);
	    tree.thrown = translate(tree.thrown);

	    // when compiling stubs, don't process body
	    if (tree.body == null) {
		result = tree;
		return;
	    }

	    // Add this$n (if needed) in front of and free variables behind
	    // constructor parameter list.
	    tree.params = tree.params.appendList(fvdefs);
	    if (currentClass.hasOuterInstance())
		tree.params = tree.params.prepend(otdef);

	    // If this is an initial constructor, i.e., it does not start with
	    // this(...), insert initializers for this$n and proxies
	    // before (pre-1.4, after) the call to superclass constructor.
	    Tree selfCall = translate(tree.body.stats.head);

	    List<Tree> added = Tree.emptyList;
	    if (fvs.nonEmpty()) {
		List<Type> addedargtypes = List.<Type>make();
		for (List<VarSymbol> l = fvs; l.nonEmpty(); l = l.tail) {
		    if (TreeInfo.isInitialConstructor(tree))
			added = added.prepend(
			    initField(tree.body.pos, proxyName(l.head.name)));
		    addedargtypes = addedargtypes.prepend(l.head.erasure(types));
		}
		Type olderasure = m.erasure(types);
		m.erasure_field = new MethodType(
		    olderasure.argtypes().appendList(addedargtypes),
		    olderasure.restype(),
		    olderasure.thrown(),
		    syms.methodClass);
	    }
	    if (currentClass.hasOuterInstance() &&
		TreeInfo.isInitialConstructor(tree))
	    {
		added = added.prepend(initOuterThis(tree.body.pos));
	    }

	    // pop local variables from proxy stack
	    proxies = proxies.leave();

	    // recursively translate following local statements and
	    // combine with this- or super-call
	    List<Tree> stats = translate(tree.body.stats.tail);
	    if (target.initializeFieldsBeforeSuper())
		tree.body.stats = stats.prepend(selfCall).prependList(added);
	    else
		tree.body.stats = stats.prependList(added).prepend(selfCall);

	    outerThisStack = prevOuterThisStack;
	} else {
	    super.visitMethodDef(tree);
	}
	result = tree;
    }

    public void visitTypeCast(TypeCast tree) {
	tree.clazz = translate(tree.clazz);
	if (tree.type.isPrimitive() != tree.expr.type.isPrimitive())
	    tree.expr = translate(tree.expr, tree.type);
	else
	    tree.expr = translate(tree.expr);
	result = tree;
    }

    public void visitNewClass(NewClass tree) {
	ClassSymbol c = (ClassSymbol)tree.constructor.owner;

	// Box arguments, if necessary
	boolean isEnum = (tree.constructor.owner.flags() & ENUM) != 0;
	List<Type> argTypes = tree.constructor.type.argtypes();
	if (isEnum) argTypes = argTypes.prepend(syms.intType).prepend(syms.stringType);
	tree.args = boxArgs(argTypes, tree.args, tree.varargsElement);
	tree.varargsElement = null;

	// If created class is local, add free variables after
	// explicit constructor arguments.
	if ((c.owner.kind & (VAR | MTH)) != 0) {
	    tree.args = tree.args.appendList(loadFreevars(tree.pos, freevars(c)));
	}

	// If an access constructor is used, append null as a last argument.
	Symbol constructor = accessConstructor(tree.pos, tree.constructor);
	if (constructor != tree.constructor) {
	    tree.args = tree.args.append(make.Ident(syms.nullConst));
	    tree.constructor = constructor;
	}

	// If created class has an outer instance, and new is qualified, pass
	// qualifier as first argument. If new is not qualified, pass the
	// correct outer instance as first argument.
	if (c.hasOuterInstance()) {
	    Tree thisArg;
	    if (tree.encl != null) {
		thisArg = attr.makeNullCheck(translate(tree.encl));
		thisArg.type = tree.encl.type;
	    } else if ((c.owner.kind & (MTH | VAR)) != 0) {
		// local class
		thisArg = makeThis(tree.pos, c.type.outer().tsym);
	    } else {
		// nested class
		thisArg = makeOwnerThis(tree.pos, c, false);
	    }
  	    tree.args = tree.args.prepend(thisArg);
	}
	tree.encl = null;

	// If we have an anonymous class, create its flat version, rather
	// than the class or interface following new.
	if (tree.def != null) {
	    translate(tree.def);
	    tree.clazz = access(make.at(tree.clazz.pos).Ident(tree.def.sym));
	    tree.def = null;
	} else {
	    tree.clazz = access(c, tree.clazz, enclOp, false);
	}
	result = tree;
    }

    // Simplify conditionals with known constant controlling expressions.
    // This allows us to avoid generating supporting declarations for
    // the dead code, which will not be eliminated during code generation.
    // Note that Flow.isFalse and Flow.isTrue only return true
    // for constant expressions in the sense of JLS 15.27, which
    // are guaranteed to have no side-effects.  More agressive
    // constant propagation would require that we take care to
    // preserve possible side-effects in the condition expression.

    /** Visitor method for conditional expressions.
     */
    public void visitConditional(Conditional tree) {
	Tree cond = tree.cond = translate(tree.cond, syms.booleanType);
	if (cond.type.isTrue()) {
	    result = convert(translate(tree.truepart, tree.type), tree.type);
	} else if (cond.type.isFalse()) {
	    result = convert(translate(tree.falsepart, tree.type), tree.type);
	} else {
	    // Condition is not a compile-time constant.
	    tree.truepart = translate(tree.truepart, tree.type);
	    tree.falsepart = translate(tree.falsepart, tree.type);
	    result = tree;
	}
    }
//where
	private Tree convert(Tree tree, Type pt) {
	    if (tree.type == pt) return tree;
	    Tree result = make.at(tree.pos).TypeCast(make.Type(pt), tree);
	    result.type = (tree.type.constValue != null) ? cfolder.coerce(tree.type, pt) : pt;
	    return result;
	}

    /** Visitor method for if statements.
     */
    public void visitIf(If tree) {
	Tree cond = tree.cond = translate(tree.cond, syms.booleanType);
	if (cond.type.isTrue()) {
	    result = translate(tree.thenpart);
	} else if (cond.type.isFalse()) {
	    if (tree.elsepart != null) {
		result = translate(tree.elsepart);
	    } else {
		result = make.Skip();
	    }
	} else {
	    // Condition is not a compile-time constant.
	    tree.thenpart = translate(tree.thenpart);
	    tree.elsepart = translate(tree.elsepart);
	    result = tree;
	}
    }

    /** Visitor method for assert statements. Translate them away.
     */
    public void visitAssert(Assert tree) {
	int detailPos = (tree.detail == null) ? tree.pos : tree.detail.pos;
	tree.cond = translate(tree.cond, syms.booleanType);
	if (!tree.cond.type.isTrue()) {
	    Tree cond = assertFlagTest(tree.pos);
	    List<Tree> exnArgs = (tree.detail == null) ?
		Tree.emptyList : List.make(translate(tree.detail));
	    if (!tree.cond.type.isFalse()) {
	        cond = makeBinary
		    (Tree.AND,
		     cond,
		     makeUnary(Tree.NOT, tree.cond));
	    }
	    result =
	        make.If(cond,
			make.at(detailPos).
			   Throw(makeNewClass(syms.assertionErrorType, exnArgs)),
			null);
	} else {
	    result = make.Skip();
	}
    }

    public void visitApply(Apply tree) {
	Symbol meth = TreeInfo.symbol(tree.meth);
	List<Type> argtypes = meth.type.argtypes();
        if (allowEnums &&
            meth.name==names.init &&
            meth.owner == syms.enumSym)
            argtypes = argtypes.tail.tail;
	tree.args = boxArgs(argtypes, tree.args, tree.varargsElement);
	tree.varargsElement = null;
	Name methName = TreeInfo.name(tree.meth);
	if (meth.name==names.init) {
	    // We are seeing a this(...) or super(...) constructor call.
	    // If an access constructor is used, append null as a last argument.
	    Symbol constructor = accessConstructor(tree.pos, meth);
	    if (constructor != meth) {
		tree.args = tree.args.append(make.Ident(syms.nullConst));
		TreeInfo.setSymbol(tree.meth, constructor);
	    }

	    // If we are calling a constructor of a local class, add
	    // free variables after explicit constructor arguments.
	    ClassSymbol c = (ClassSymbol)constructor.owner;
	    if ((c.owner.kind & (VAR | MTH)) != 0) {
		tree.args = tree.args.appendList(loadFreevars(tree.pos, freevars(c)));
	    }

	    // If we are calling a constructor of an enum class, pass
	    // along the name and ordinal arguments
	    if ((c.flags_field&ENUM) != 0 || c.fullName() == names.java_lang_Enum) {
		List<VarDef> params = currentMethodDef.params;
		if (currentMethodSym.owner.hasOuterInstance())
		    params = params.tail; // drop this$n
		tree.args = tree.args
		    .prepend(make.at(tree.pos).Ident(params.tail.head.sym)) // ordinal
		    .prepend(make.Ident(params.head.sym)); // name
	    }

	    // If we are calling a constructor of a class with an outer
	    // instance, and the call
	    // is qualified, pass qualifier as first argument in front of
	    // the explicit constructor arguments. If the call
	    // is not qualified, pass the correct outer instance as
	    // first argument.
	    if (c.hasOuterInstance()) {
		Tree thisArg;
		if (tree.meth.tag == Tree.SELECT) {
		    thisArg = attr.
			makeNullCheck(translate(((Select)tree.meth).selected));
		    tree.meth = make.Ident(constructor);
		    ((Ident)tree.meth).name = methName;
		} else if ((c.owner.kind & (MTH | VAR)) != 0 || methName == names._this){
		    // local class or this() call
		    thisArg = makeThis(tree.meth.pos, c.type.outer().tsym);
		} else {
		    // super() call of nested class
		    thisArg = makeOwnerThis(tree.meth.pos, c, false);
		}
		tree.args = tree.args.prepend(thisArg);
	    }
	} else {
	    // We are seeing a normal method invocation; translate this as usual.
	    tree.meth = translate(tree.meth);

	    // If the translated method itself is an Apply tree, we are
	    // seeing an access method invocation. In this case, append
	    // the method arguments to the arguments of the access method.
	    if (tree.meth.tag == Tree.APPLY) {
		Apply app = (Apply)tree.meth;
		app.args = tree.args.prependList(app.args);
		result = app;
		return;
	    }
	}
	result = tree;
    }

    List<Tree> boxArgs(List<Type> parameters, List<Tree> _args, Type varargsElement) {
	List<Tree> args = _args;
	if (parameters.isEmpty()) return args;
	boolean anyChanges = false;
	ListBuffer<Tree> result = new ListBuffer<Tree>();
	while (parameters.tail.nonEmpty()) {
	    Tree arg = translate(args.head, parameters.head);
	    anyChanges |= (arg != args.head);
	    result.append(arg);
	    args = args.tail;
	    parameters = parameters.tail;
	}
	Type parameter = parameters.head;
	if (varargsElement != null) {
	    anyChanges = true;
	    ListBuffer<Tree> elems = new ListBuffer<Tree>();
	    while (args.nonEmpty()) {
		Tree arg = translate(args.head, varargsElement);
		elems.append(arg);
		args = args.tail;
	    }
	    NewArray boxedArgs = make.NewArray(make.Type(varargsElement),
                                               Tree.emptyList,
                                               elems.toList());
	    boxedArgs.type = new ArrayType(varargsElement, syms.arrayClass);
	    result.append(boxedArgs);
	} else {
            if (args.length() != 1) throw new AssertionError(args);
	    Tree arg = translate(args.head, parameter);
	    anyChanges |= (arg != args.head);
	    result.append(arg);
	    if (!anyChanges) return _args;
	}
	return result.toList();
    }

    /** Expand a boxing or unboxing conversion if needed. */
    Tree boxIfNeeded(Tree tree, Type type) {
	boolean havePrimitive = tree.type.isPrimitive();
	if (havePrimitive == type.isPrimitive()) return tree;
	if (havePrimitive) {
            Type unboxedTarget = types.unboxedType(type);
            if (unboxedTarget.tag != NONE) {
                if (!types.isSubType(tree.type, unboxedTarget))
                    tree.type = unboxedTarget; // e.g. Character c = 89;
                return boxPrimitive(tree, type);
            } else {
                tree = boxPrimitive(tree);
            }
	} else {
	    tree = unbox(tree, type);
	}
	return tree;
    }

    /** Box up a single primitive expression. */
    Tree boxPrimitive(Tree tree) {
	return boxPrimitive(tree, types.boxedClass(tree.type).type);
    }

    /** Box up a single primitive expression. */
    Tree boxPrimitive(Tree tree, Type box) {
	make.at(tree.pos);
        if (target.boxWithConstructors()) {
            Symbol ctor = lookupConstructor(tree.pos,
                                            box,
                                            Type.emptyList
                                            .prepend(tree.type));
            return make.Create(ctor, List.make(tree));
        } else {
            Symbol valueOfSym = lookupMethod(tree.pos,
                                             names.valueOf,
                                             box,
                                             Type.emptyList
                                             .prepend(tree.type));
            return make.App(make.QualIdent(valueOfSym), List.make(tree));
        }
    }

    /** Unbox an object to a primitive value. */
    Tree unbox(Tree tree, Type primitive) {
	Type unboxedType = types.unboxedType(tree.type);
	// note: the "primitive" parameter is not used.  There muse be
	// a conversion from unboxedType to primitive.
	make.at(tree.pos);
	Symbol valueSym = lookupMethod(tree.pos,
				       unboxedType.tsym.name.append(names.Value), // x.intValue()
				       tree.type,
				       Type.emptyList);
	return make.App(make.Select(tree, valueSym), Tree.emptyList);
    }

    /** Visitor method for parenthesized expressions.
     *  If the subexpression has changed, omit the parens.
     */
    public void visitParens(Parens tree) {
	Tree expr = translate(tree.expr);
	result = ((expr == tree.expr) ? tree : expr);
    }

    public void visitIndexed(Indexed tree) {
	tree.indexed = translate(tree.indexed);
	tree.index = translate(tree.index, syms.intType);
	result = tree;
    }

    public void visitAssign(Assign tree) {
	tree.lhs = translate(tree.lhs, tree);
	tree.rhs = translate(tree.rhs, tree.lhs.type);

	// If translated left hand side is an Apply, we are
	// seeing an access method invocation. In this case, append
	// right hand side as last argument of the access method.
	if (tree.lhs.tag == Tree.APPLY) {
	    Apply app = (Apply)tree.lhs;
	    app.args = List.make(tree.rhs).prependList(app.args);
	    result = app;
	} else {
	    result = tree;
	}
    }

    public void visitAssignop(final Assignop tree) {
	if (!tree.lhs.type.isPrimitive() &&
	    tree.operator.type.restype().isPrimitive()) {
	    // boxing required; need to rewrite as x = (typeof x)(x op y);
	    // (but without recomputing x)
	    Tree newTree = abstractLval(tree.lhs, new TreeBuilder() {
		    public Tree build(final Tree lhs) {
			int newTag = tree.tag - Tree.ASGOffset;
			Symbol newOperator = rs.resolveBinaryOperator(tree.pos,
								      newTag,
								      attrEnv,
								      tree.lhs.type,
								      tree.rhs.type);
			Binary opResult = make.Binary(newTag, lhs, tree.rhs);
			opResult.operator = newOperator;
			opResult.type = newOperator.type.restype();
			Tree newRhs = make.TypeCast(types.unboxedType(lhs.type), opResult);
			return make.Assign(lhs, newRhs).setType(tree.type);
		    }
		});
	    result = translate(newTree);
	    return;
	}
	tree.lhs = translate(tree.lhs, tree);
	tree.rhs = translate(tree.rhs, tree.operator.type.argtypes().tail.head);

	// If translated left hand side is an Apply, we are
	// seeing an access method invocation. In this case, append
	// right hand side as last argument of the access method.
	if (tree.lhs.tag == Tree.APPLY) {
	    Apply app = (Apply)tree.lhs;
	    // if operation is a += on strings,
	    // make sure to convert argument to string
	    Tree rhs = (((OperatorSymbol)tree.operator).opcode == string_add)
	      ? makeString(tree.rhs)
	      : tree.rhs;
	    app.args = List.make(rhs).prependList(app.args);
	    result = app;
	} else {
	    result = tree;
	}
    }

    /** Lower a tree of the form e++ or e-- where e is an object type */
    Tree lowerBoxedPostop(final Unary tree) {
	// translate to tmp1=lval(e); tmp2=tmp1; tmp1+=1; tmp2
	// or
	// translate to tmp1=lval(e); tmp2=tmp1; tmp1-=1; tmp2
	return abstractLval(tree.arg, new TreeBuilder() {
		public Tree build(final Tree tmp1) {
		    return abstractRval(tmp1, new TreeBuilder() {
			    public Tree build(final Tree tmp2) {
				int opcode = (tree.tag == Tree.POSTINC)
				    ? Tree.PLUS_ASG : Tree.MINUS_ASG;
				Tree update = makeAssignop(opcode,
							   tmp1,
							   make.Literal(1));
				return makeComma(update, tmp2);
			    }
			});
		}
	    });
    }

    public void visitUnary(Unary tree) {
	boolean isUpdateOperator =
	    Tree.PREINC <= tree.tag && tree.tag <= Tree.POSTDEC;
	if (isUpdateOperator && !tree.arg.type.isPrimitive()) {
	    switch(tree.tag) {
	    case Tree.PREINC:            // ++ e
		    // translate to e += 1
	    case Tree.PREDEC:            // -- e
		    // translate to e -= 1
		{
		    int opcode = (tree.tag == Tree.PREINC)
			? Tree.PLUS_ASG : Tree.MINUS_ASG;
		    Assignop newTree = makeAssignop(opcode,
						    tree.arg,
						    make.Literal(1));
		    result = translate(newTree, tree.type);
		    return;
		}
	    case Tree.POSTINC:           // e ++
	    case Tree.POSTDEC:           // e --
		{
		    result = translate(lowerBoxedPostop(tree), tree.type);
		    return;
		}
	    }
	    throw new AssertionError(tree);
	}

	tree.arg = boxIfNeeded(translate(tree.arg, tree), tree.type);

	if (tree.tag == Tree.NOT && tree.arg.type.constValue != null) {
	    tree.type = cfolder.fold1(bool_not, tree.arg.type);
	}

	// If translated left hand side is an Apply, we are
	// seeing an access method invocation. In this case, return
	// that access method invokation as result.
	if (isUpdateOperator && tree.arg.tag == Tree.APPLY) {
	    result = tree.arg;
	} else {
	    result = tree;
	}
    }

    public void visitBinary(Binary tree) {
	List<Type> formals = tree.operator.type.argtypes();
	Tree lhs = tree.lhs = translate(tree.lhs, formals.head);
	switch (tree.tag) {
	case Tree.OR:
	    if (lhs.type.isTrue()) {
		result = lhs;
		return;
	    }
	    if (lhs.type.isFalse()) {
		result = translate(tree.rhs, formals.tail.head);
		return;
	    }
	    break;
	case Tree.AND:
	    if (lhs.type.isFalse()) {
		result = lhs;
		return;
	    }
	    if (lhs.type.isTrue()) {
		result = translate(tree.rhs, formals.tail.head);
		return;
	    }
	    break;
	}
	tree.rhs = translate(tree.rhs, formals.tail.head);
	result = tree;
    }

    public void visitIdent(Ident tree) {
	result = access(tree.sym, tree, enclOp, false);
    }

    /** Translate away the foreach loop.  */
    public void visitForeachLoop(ForeachLoop tree) {
	if (types.elemtype(tree.expr.type) == null)
	    visitIterableForeachLoop(tree);
	else
	    visitArrayForeachLoop(tree);
    }
        // where
        /**
	 * A statment of the form
	 *
	 * <pre>
	 *     for ( T v : arrayexpr ) stmt;
	 * </pre>
	 *
	 * (where arrayexpr is of an array type) gets translated to
	 *
	 * <pre>
	 *     for ( { arraytype #arr = arrayexpr;
	 *             int #len = array.length;
	 *             int #i = 0; };
	 *           #i < #len; i$++ ) {
	 *         T v = arr$[#i];
	 *         stmt;
	 *     }
	 * </pre>
	 *
	 * where #arr, #len, and #i are freshly named synthetic local variables.
	 */
        private void visitArrayForeachLoop(ForeachLoop tree) {
	    make.at(tree.expr.pos);
	    VarSymbol arraycache = new VarSymbol(0,
						 names.fromString("arr" + target.syntheticNameChar()),
						 tree.expr.type,
						 currentMethodSym);
	    VarDef arraycachedef = make.VarDef(arraycache, tree.expr);
	    VarSymbol lencache = new VarSymbol(0,
					       names.fromString("len" + target.syntheticNameChar()),
					       syms.intType,
					       currentMethodSym);
	    VarDef lencachedef = make.
		VarDef(lencache, make.Select(make.Ident(arraycache), syms.lengthVar));
	    VarSymbol index = new VarSymbol(0,
					    names.fromString("i" + target.syntheticNameChar()),
					    syms.intType,
					    currentMethodSym);

	    VarDef indexdef = make.VarDef(index, make.Literal(INT, 0));
	    indexdef.init.type = indexdef.type = syms.intType.constType(0);

	    List<Tree> loopinit = List.<Tree>make().
		prepend(indexdef).
		prepend(lencachedef).
		prepend(arraycachedef);
	    Tree cond = makeBinary(Tree.LT, make.Ident(index), make.Ident(lencache));

	    Tree step = make.Exec(makeUnary(Tree.PREINC, make.Ident(index)));

	    Type elemtype = types.elemtype(tree.expr.type);
	    Tree loopvarinit = make.
		VarDef(tree.var.sym,
		       make.
		       Indexed(make.Ident(arraycache), make.Ident(index)).
		       setType(elemtype));
	    Tree body = make.
		Block(0, List.<Tree>make().prepend(tree.body).prepend(loopvarinit));

	    result = translate(make.
			       ForLoop(loopinit,
				       cond,
				       List.<Tree>make().prepend(step),
				       body));
	    patchTargets(body, tree, result);
	}
        /** Patch up break and continue targets. */
	private void patchTargets(Tree body, final Tree src, final Tree dest) {
	    class Patcher extends TreeScanner {
		public void visitBreak(Break tree) {
		    if (tree.target == src)
			tree.target = dest;
		}
		public void visitContinue(Continue tree) {
		    if (tree.target == src)
			tree.target = dest;
		}
		public void visitClassDef(ClassDef tree) {}
	    }
	    new Patcher().scan(body);
	}
	/**
	 * A statement of the form
	 *
	 * <pre>
	 *     for ( T v : coll ) stmt ;
	 * </pre>
	 *
	 * (where coll implements Iterable<? extends T>) gets translated to
	 *
	 * <pre>
	 *     for ( Iterator<? extends T> #i = coll.iterator(); #i.hasNext(); ) {
	 *         T v = (T) #i.next();
	 *         stmt;
	 *     }
	 * </pre>
	 *
	 * where #i is a freshly named synthetic local variable.
	 */
        private void visitIterableForeachLoop(ForeachLoop tree) {
	    make.at(tree.expr.pos);
	    Type iteratorTarget = syms.objectType;
	    Type iterableType = types.asSuper(types.upperBound(tree.expr.type),
					      syms.iterableType.tsym);
	    if (iterableType.typarams().nonEmpty())
		iteratorTarget = types.erasure(iterableType.typarams().head);
            Type eType = tree.expr.type;
            tree.expr.type = types.erasure(eType);
            if (eType.tag == TYPEVAR && eType.bound().isCompound())
                tree.expr = make.TypeCast(types.erasure(iterableType), tree.expr);
	    Symbol iterator = lookupMethod(tree.expr.pos,
					   names.iterator,
					   types.erasure(syms.iterableType),
					   Type.emptyList);
	    VarSymbol itvar = new VarSymbol(0, names.fromString("i" + target.syntheticNameChar()),
					    types.erasure(iterator.type.restype()),
					    currentMethodSym);
	    VarDef itvardef = make.
		VarDef(itvar,
		       make.App(make.Select(tree.expr, iterator), Tree.emptyList));
	    List<Tree> loopinit = Tree.emptyList.prepend(itvardef);
	    Symbol hasNext = lookupMethod(tree.expr.pos,
					  names.hasNext,
					  itvar.type,
					  Type.emptyList);
	    Tree cond = make.
		App(make.Select(make.Ident(itvar), hasNext), Tree.emptyList);
	    Symbol next = lookupMethod(tree.expr.pos,
                                       names.next,
				       itvar.type,
                                       Type.emptyList);
	    Tree vardefinit = make.
		App(make.Select(make.Ident(itvar), next), Tree.emptyList);
	    if (iteratorTarget != syms.objectType)
		vardefinit = make.TypeCast(iteratorTarget, vardefinit);
	    VarDef indexDef = make.VarDef(tree.var.sym, vardefinit);
	    Tree body = make.
		Block(0, Tree.emptyList.prepend(tree.body).prepend(indexDef));
	    result = translate(make.
		ForLoop(loopinit,
			cond,
			Tree.emptyList,
			body));
	    patchTargets(body, tree, result);
	}

    public void visitVarDef(VarDef tree) {
	MethodSymbol oldMethodSym = currentMethodSym;
	tree.mods = (Modifiers)translate(tree.mods);
	tree.vartype = translate(tree.vartype);
	if (currentMethodSym == null) {
	    // A class or instance field initializer.
	    currentMethodSym =
		new MethodSymbol((tree.mods.flags&STATIC) | BLOCK,
				 names.empty, null,
				 currentClass);
	}
	if (tree.init != null) tree.init = translate(tree.init, tree.type);
	result = tree;
	currentMethodSym = oldMethodSym;
    }

    public void visitBlock(Block tree) {
	MethodSymbol oldMethodSym = currentMethodSym;
	if (currentMethodSym == null) {
	    // Block is a static or instance initializer.
	    currentMethodSym =
		new MethodSymbol(tree.flags | BLOCK,
				 names.empty, null,
				 currentClass);
	}
	super.visitBlock(tree);
	currentMethodSym = oldMethodSym;
    }

    public void visitDoLoop(DoLoop tree) {
	tree.body = translate(tree.body);
	tree.cond = translate(tree.cond, syms.booleanType);
	result = tree;
    }

    public void visitWhileLoop(WhileLoop tree) {
	tree.cond = translate(tree.cond, syms.booleanType);
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitForLoop(ForLoop tree) {
	tree.init = translate(tree.init);
	if (tree.cond != null)
	    tree.cond = translate(tree.cond, syms.booleanType);
	tree.step = translate(tree.step);
	tree.body = translate(tree.body);
	result = tree;
    }

    public void visitReturn(Return tree) {
	if (tree.expr != null)
	    tree.expr = translate(tree.expr,
				  types.erasure(currentMethodDef
						.restype.type));
	result = tree;
    }

    public void visitSwitch(Switch tree) {
	Type selsuper = types.supertype(tree.selector.type);
	boolean enumSwitch = selsuper != null &&
            (tree.selector.type.tsym.flags() & ENUM) != 0;
	Type target = enumSwitch ? tree.selector.type : syms.intType;
	tree.selector = translate(tree.selector, target);
	tree.cases = translateCases(tree.cases);
        if (enumSwitch) {
            result = visitEnumSwitch(tree);
            patchTargets(result, tree, result);
        } else {
            result = tree;
        }
    }

    public Tree visitEnumSwitch(Switch tree) {
        TypeSymbol enumSym = tree.selector.type.tsym;
        EnumMapping map = mapForEnum(tree.pos, enumSym);
        make.at(tree.pos);
        Symbol ordinalMethod = lookupMethod(tree.pos,
                                            names.ordinal,
                                            tree.selector.type,
                                            Type.emptyList);
        Tree selector = make.Indexed(map.mapVar,
                                     make.App(make.Select(tree.selector, ordinalMethod),
                                              Tree.emptyList));
        ListBuffer<Case> cases = new ListBuffer<Case>();
        for (Case c : tree.cases) {
            if (c.pat != null) {
                VarSymbol label = (VarSymbol)TreeInfo.symbol(c.pat);
                Tree pat = map.forConstant(label);
                cases.append(make.Case(pat, c.stats));
            } else {
                cases.append(c);
            }
        }
        return make.Switch(selector, cases.toList());
    }

    public void visitNewArray(NewArray tree) {
	tree.elemtype = translate(tree.elemtype);
	for (List<Tree> t = tree.dims; t.tail != null; t = t.tail)
	    if (t.head != null) t.head = translate(t.head, syms.intType);
	tree.elems = translate(tree.elems, types.elemtype(tree.type));
	result = tree;
    }

    public void visitSelect(Select tree) {
	// need to special case-access of the form C.super.x
	// these will always need an access method.
	boolean qualifiedSuperAccess =
	    tree.selected.tag == Tree.SELECT &&
	    TreeInfo.name(tree.selected) == names._super;
	tree.selected = translate(tree.selected);
	if (tree.name == names._class)
	    result = classOf(tree.selected);
	else if (tree.name == names._this || tree.name == names._super)
	    result = makeThis(tree.pos, tree.selected.type.tsym);
	else
	    result = access(tree.sym, tree, enclOp, qualifiedSuperAccess);
    }

    public void visitLetExpr(LetExpr tree) {
	tree.defs = translateVarDefs(tree.defs);
	tree.expr = translate(tree.expr, tree.type);
	result = tree;
    }

    // There ought to be nothing to rewrite here;
    // we don't generate code.
    public void visitAnnotation(Annotation tree) {
	result = tree;
    }

/**************************************************************************
 * main method
 *************************************************************************/

    /** Translate a toplevel class and return a list consisting of
     *  the translated class and translated versions of all inner classes.
     *  @param env   The attribution environment current at the class definition.
     *               We need this for resolving some additional symbols.
     *  @param cdef  The tree representing the class definition.
     */
    public List<Tree> translateTopLevelClass(Env<AttrContext> env, Tree cdef, TreeMaker make) {
	ListBuffer<Tree> translated = null;
	try {
	    attrEnv = env;
	    this.make = make;
	    endPositions = env.toplevel.endPositions;
	    currentClass = null;
	    currentMethodDef = null;
            outermostClassDef = (cdef.tag == Tree.CLASSDEF) ? (ClassDef)cdef : null;
	    outermostMemberDef = null;
	    this.translated = new ListBuffer<Tree>();
	    classdefs = new HashMap<ClassSymbol,ClassDef>();
	    actualSymbols = new HashMap<Symbol,Symbol>();
	    freevarCache = new HashMap<ClassSymbol,List<VarSymbol>>();
	    proxies = new Scope(syms.noSymbol);
	    outerThisStack = VarSymbol.emptyList;
	    accessNums = new HashMap<Symbol,Integer>();
	    accessSyms = new HashMap<Symbol,MethodSymbol[]>();
	    accessConstrs = new HashMap<Symbol,MethodSymbol>();
	    accessed = new ListBuffer<Symbol>();
	    translate(cdef, (Tree)null);
	    for (List<Symbol> l = accessed.toList(); l.nonEmpty(); l = l.tail)
		makeAccessible(l.head);
            for (EnumMapping map : enumSwitchMap.values())
                map.translate();
	    translated = this.translated;
	} finally {
	    // note that recursive invocations of this method fail hard
	    attrEnv = null;
	    this.make = null;
	    endPositions = null;
	    currentClass = null;
	    currentMethodDef = null;
	    outermostClassDef = null;
	    outermostMemberDef = null;
	    this.translated = null;
	    classdefs = null;
	    actualSymbols = null;
	    freevarCache = null;
	    proxies = null;
	    outerThisStack = null;
	    accessNums = null;
	    accessSyms = null;
	    accessConstrs = null;
	    accessed = null;
            enumSwitchMap.clear();
	}
	return translated.toList();
    }

    //////////////////////////////////////////////////////////////
    // The following contributed by Borland for bootstrapping purposes
    //////////////////////////////////////////////////////////////
    private void addEnumCompatibleMembers(ClassDef cdef) {
        make.at(Position.NOPOS);

        // Add the special enum fields
        VarSymbol ordinalFieldSym = addEnumOrdinalField(cdef);
        VarSymbol nameFieldSym = addEnumNameField(cdef);

        // Add the accessor methods for name and ordinal
        MethodSymbol ordinalMethodSym = addEnumFieldOrdinalMethod(cdef, ordinalFieldSym);
        MethodSymbol nameMethodSym = addEnumFieldNameMethod(cdef, nameFieldSym);

        // Add the toString method
        addEnumToString(cdef, nameFieldSym);

        // Add the compareTo method
        addEnumCompareTo(cdef, ordinalFieldSym);
    }

    private VarSymbol addEnumOrdinalField(ClassDef cdef) {
        VarSymbol ordinal = new VarSymbol(PRIVATE|FINAL|SYNTHETIC,
                                          names.fromString("$ordinal"),
                                          syms.intType,
                                          cdef.sym);
        cdef.sym.members().enter(ordinal);
        cdef.defs = cdef.defs.prepend(make.VarDef(ordinal, null));
        return ordinal;
    }

    private VarSymbol addEnumNameField(ClassDef cdef) {
        VarSymbol name = new VarSymbol(PRIVATE|FINAL|SYNTHETIC,
                                          names.fromString("$name"),
                                          syms.stringType,
                                          cdef.sym);
        cdef.sym.members().enter(name);
        cdef.defs = cdef.defs.prepend(make.VarDef(name, null));
        return name;
    }

    private MethodSymbol addEnumFieldOrdinalMethod(ClassDef cdef, VarSymbol ordinalSymbol) {
        // Add the accessor methods for ordinal
        Symbol ordinalSym = lookupMethod(cdef.pos,
                                         names.ordinal,
                                         cdef.type,
                                         Type.emptyList);

        assert(ordinalSym != null);
        assert(ordinalSym instanceof MethodSymbol);

        Return ret = make.Return(make.Ident(ordinalSymbol));
        List<Tree> statements = Tree.emptyList.prepend(ret);
        cdef.defs = cdef.defs.append(make.MethodDef((MethodSymbol)ordinalSym,
                                                    make.Block(0L, statements)));

        return (MethodSymbol)ordinalSym;
    }

    private MethodSymbol addEnumFieldNameMethod(ClassDef cdef, VarSymbol nameSymbol) {
        // Add the accessor methods for name
        Symbol nameSym = lookupMethod(cdef.pos,
                                   names._name,
                                   cdef.type,
                                   Type.emptyList);

        assert(nameSym != null);
        assert(nameSym instanceof MethodSymbol);

        Return ret = make.Return(make.Ident(nameSymbol));
        List<Tree> statements = Tree.emptyList.prepend(ret);

        cdef.defs = cdef.defs.append(make.MethodDef((MethodSymbol)nameSym,
                                                    make.Block(0L,
                                                               statements)));

        return (MethodSymbol)nameSym;
    }

    private MethodSymbol addEnumToString(ClassDef cdef,
                                         VarSymbol nameSymbol) {
        Symbol toStringSym = lookupMethod(cdef.pos,
                                          names.toString,
                                          cdef.type,
                                          Type.emptyList);

        Tree toStringDecl = null;
        if (toStringSym != null)
            toStringDecl = TreeInfo.declarationFor(toStringSym, cdef);

        if (toStringDecl != null)
            return (MethodSymbol)toStringSym;

        Return ret = make.Return(make.Ident(nameSymbol));
        List<Tree> statements = Tree.emptyList.prepend(ret);

        Tree resTypeTree = make.Type(syms.stringType);

        MethodType toStringType = new MethodType(Type.emptyList,
                                                 syms.stringType,
                                                 Type.emptyList,
                                                 cdef.sym);
        toStringSym = new MethodSymbol(PUBLIC,
                                       names.toString,
                                       toStringType,
                                       cdef.type.tsym);
        toStringDecl = make.MethodDef((MethodSymbol)toStringSym,
                                      make.Block(0L, statements));

        cdef.defs = cdef.defs.prepend(toStringDecl);
        cdef.sym.members().enter(toStringSym);

        return (MethodSymbol)toStringSym;
    }

    private MethodSymbol addEnumCompareTo(ClassDef cdef, VarSymbol ordinalSymbol) {
        Symbol compareToSym = lookupMethod(cdef.pos,
                                   names.compareTo,
                                   cdef.type,
                                   Type.emptyList.prepend(cdef.sym.type));

        assert(compareToSym != null);
        assert(compareToSym instanceof MethodSymbol);

        MethodDef compareToDecl = (MethodDef) TreeInfo.declarationFor(compareToSym, cdef);

        ListBuffer<Tree> blockStatements = new ListBuffer<Tree>();

        Modifiers mod1 = make.Modifiers(0L);
        Name oName = Name.fromString(names, "o");
        VarDef par1 = make.Param(oName, cdef.type, compareToSym);

        Ident paramId1 = make.Ident(names.java_lang_Object);
        paramId1.type = cdef.type;
        paramId1.sym = par1.sym;

        ((MethodSymbol)compareToSym).params = VarSymbol.emptyList.prepend(par1.sym);

        Tree par1UsageId = make.Ident(par1.sym);
        Tree castTargetIdent = make.Ident(cdef.sym);
        TypeCast cast = make.TypeCast(castTargetIdent, par1UsageId);
        cast.setType(castTargetIdent.type);

        Name otherName = Name.fromString(names, "other");

        VarSymbol otherVarSym = new VarSymbol(mod1.flags,
                                              otherName,
                                              cdef.type,
                                              compareToSym);
        VarDef otherVar = make.VarDef(otherVarSym, cast);
        blockStatements.append(otherVar);

        Tree id1 = make.Ident(ordinalSymbol);

        Tree fLocUsageId = make.Ident(otherVarSym);
        Tree sel = make.Select(fLocUsageId, ordinalSymbol);
        Binary bin = makeBinary(Tree.MINUS, id1, sel);
        Return ret = make.Return(bin);
        blockStatements.append(ret);
        MethodDef compareToMethod = make.MethodDef((MethodSymbol)compareToSym,
                                                   make.Block(0L,
                                                              blockStatements.toList()));
        compareToMethod.params = VarDef.emptyList.prepend(par1);
        cdef.defs = cdef.defs.append(compareToMethod);

        return (MethodSymbol)compareToSym;
    }
    //////////////////////////////////////////////////////////////
    // The above contributed by Borland for bootstrapping purposes
    //////////////////////////////////////////////////////////////
}
