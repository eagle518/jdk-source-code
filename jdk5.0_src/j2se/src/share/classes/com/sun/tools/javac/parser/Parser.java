/**
 * @(#)Parser.java	1.76 04/04/26
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.parser;

import java.util.*;

import com.sun.tools.javac.tree.*;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.util.*;
import com.sun.tools.javac.util.List;

import com.sun.tools.javac.tree.Tree.*;

import static com.sun.tools.javac.parser.Tokens.*;

/** The parser maps a token sequence into an abstract syntax
 *  tree. It operates by recursive descent, with code derived
 *  systematically from an LL(1) grammar. For efficiency reasons, an
 *  operator precedence scheme is used for parsing binary operation
 *  expressions.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Parser {

    /** A factory for creating parsers. */
    public static class Factory {
	/** The context key for the parser factory. */
	protected static final Context.Key<Parser.Factory> parserFactoryKey =
	    new Context.Key<Parser.Factory>();

	/** Get the Factory instance for this context. */
	public static Factory instance(Context context) {
	    Factory instance = context.get(parserFactoryKey);
	    if (instance == null)
		instance = new Factory(context);
	    return instance;
	}

	final TreeMaker F;
	final Log log;
	final Keywords keywords;
	final Source source;
	final Name.Table names;
	final Options options;

	/** Create a new parser factory. */
	protected Factory(Context context) {
	    context.put(parserFactoryKey, this);
	    this.F = TreeMaker.instance(context);
	    this.log = Log.instance(context);
	    this.names = Name.Table.instance(context);
	    this.keywords = Keywords.instance(context);
	    this.source = Source.instance(context);
	    this.options = Options.instance(context);
	}

	public Parser newParser(Scanner S,
				boolean keepDocComments) {
	    return new Parser(this, S, keepDocComments);
	}
    }


    /** The number of precedence levels of infix operators.
     */
    private static final int infixPrecedenceLevels = 10;

    /** The scanner used for lexical analysis.
     */
    private Scanner S;

    /** The factory to be used for abstract syntax tree construction.
     */
    private TreeMaker F;

    /** The log to be used for error diagnostics.
     */
    private Log log;

    /** The keyword table. */
    private Keywords keywords;

    /** The Source language setting. */
    private Source source;

    /** The name table. */
    private Name.Table names;

    /** Construct a parser from a given scanner, tree factory and log.
     *  @param genEndPos Should endPositions be generated?
     */
    protected Parser(Factory fac,
		     Scanner S,
		     boolean keepDocComments) {
	this.S = S;
	this.F = fac.F;
	this.log = fac.log;
	this.names = fac.names;
	this.keywords = fac.keywords;
	this.source = fac.source;
	Options options = fac.options;
	this.allowGenerics = source.allowGenerics();
	this.allowVarargs = source.allowVarargs();
	this.allowAsserts = source.allowAsserts();
	this.allowEnums = source.allowEnums();
        this.allowForeach = source.allowForeach();
        this.allowStaticImport = source.allowStaticImport();
        this.allowAnnotations = source.allowAnnotations();
	this.keepDocComments = keepDocComments;
	this.genEndPos = options.get("-Xjcov") != null;;
	if (keepDocComments) docComments = new HashMap<Tree,String>();
	if (genEndPos) endPositions = new HashMap<Tree,Integer>();
    }

    /** Switch: Should generics be recognized?
     */
    boolean allowGenerics;

    /** Switch: Should varargs be recognized?
     */
    boolean allowVarargs;

    /** Switch: should we recognize assert statements, or just give a warning?
     */
    boolean allowAsserts;

    /** Switch: should we recognize enums, or just give a warning?
     */
    boolean allowEnums;

    /** Switch: should we recognize foreach?
     */
    boolean allowForeach;

    /** Switch: should we recognize foreach?
     */
    boolean allowStaticImport;

    /** Switch: should we recognize annotations?
     */
    boolean allowAnnotations;

    /** Switch: should we keep docComments?
     */
    boolean keepDocComments;

    /** Switch: should we store the ending positions?
     */
    boolean genEndPos;

    /** When terms are parsed, the mode determines which is expected:
     *     mode = EXPR        : an expression
     *     mode = TYPE        : a type
     *     mode = NOPARAMS    : no parameters allowed for type
     *	   mode = TYPEARG     : type argument
     */
    static final int EXPR = 1;
    static final int TYPE = 2;
    static final int NOPARAMS = 4;
    static final int TYPEARG = 8;

    /** The current mode.
     */
    private int mode = 0;

    /** The mode of the term that was parsed last.
     */
    private int lastmode = 0;

/* ---------- error recovery -------------- */

    static Tree errorTree = new Tree.Erroneous();

    /** Skip forward until a suitable stop token is found.
     */
    private void skip() {
        int nbraces = 0;
        int nparens = 0;
        while (true) {
            switch (S.token()) {
            case EOF: case CLASS: case INTERFACE:
	    case ENUM:
                return;
            case SEMI:
                if (nbraces == 0 && nparens == 0) return;
                break;
            case RBRACE:
                if (nbraces == 0) return;
                nbraces--;
                break;
            case RPAREN:
                if (nparens > 0) nparens--;
                break;
            case LBRACE:
                nbraces++;
                break;
            case LPAREN:
                nparens++;
                break;
            default:
            }
            S.nextToken();
        }
    }

    /** Generate a syntax error at given position using the given argument
     *  unless one was already reported at the same position, then skip.
     */
    private Tree syntaxError(int pos, String key, String arg) {
	if (pos != S.errPos()) log.error(pos, key, arg);
	skip();
	S.errPos(pos);
	return errorTree;
    }

    /** Generate a syntax error at given position unless one was already
     *  reported at the same position, then skip.
     */
    private Tree syntaxError(int pos, String key) {
	return syntaxError(pos, key, null);
    }

    /** Generate a syntax error at current position unless one was already
     *  reported at the same position, then skip.
     */
    private Tree syntaxError(String key) {
	return syntaxError(S.pos(), key, null);
    }

    /** Generate a syntax error at current position unless one was
     *  already reported at the same position, then skip.
     */
    private Tree syntaxError(String key, String arg) {
	return syntaxError(S.pos(), key, arg);
    }

    /** If next input token matches given token, skip it, otherwise report
     *  an error.
     */
    private void accept(Tokens token) {
	if (S.token() == token) {
	    S.nextToken();
	} else {
	    int pos = Position.line(S.pos()) > Position.line(S.prevEndPos() + 1)
		? S.prevEndPos() + 1
		: S.pos();
	    syntaxError(pos, "expected", keywords.token2string(token));
	    if (S.token() == token) S.nextToken();
	}
    }

    /** Report an illegal start of expression/type error at given position.
     */
    Tree illegal(int pos) {
	if ((mode & EXPR) != 0)
	    return syntaxError(pos, "illegal.start.of.expr");
	else
	    return syntaxError(pos, "illegal.start.of.type");
			
    }

    /** Report an illegal start of expression/type error at current position.
     */
    Tree illegal() {
	return illegal(S.pos());
    }

    /** Diagnose a modifier flag from the set, if any. */
    void checkNoMods(long mods) {
        if (mods != 0) {
            long lowestMod = mods & -mods;
            log.error(S.pos(), "mod.not.allowed.here",
                      Flags.toString(lowestMod).trim());
        }
    }

/* ---------- doc comments --------- */

    /** A hashtable to store all documentation comments
     *  indexed by the tree nodes they refer to.
     *  defined only if option flag keepDocComment is set.
     */
    Map<Tree,String> docComments;

    /** Make an entry into docComments hashtable,
     *  provided flag keepDocComments is set and given doc comment is non-null.
     *  @param tree   The tree to be used as index in the hashtable
     *  @param dc     The doc comment to associate with the tree, or null.
     */
    void attach(Tree tree, String dc) {
        if (keepDocComments && dc != null) {
//	    System.out.println("doc comment = ");System.out.println(dc);//DEBUG
	    docComments.put(tree, dc);
	}
    }

/* -------- source positions ------- */

    /** A hashtable to store ending positions
     *  of source ranges indexed by the tree nodes.
     *  Defined only if option flag genEndPos is set.
     */
    Map<Tree,Integer> endPositions;

    /** Make an entry into endPositions hashtable, provided flag
     *  genEndPos is set. Note that this method is usually hand-inlined.
     *  @param tree   The tree to be used as index in the hashtable
     *  @param endPos The ending position to associate with the tree.
     */
    void storeEnd(Tree tree, int endpos) {
	if (genEndPos) endPositions.put(tree, endpos);
    }

/* ---------- parsing -------------- */

    /**
     * Ident = IDENTIFIER
     */
    Name ident() {
	if (S.token() == IDENTIFIER) {
	    Name name = S.name();
	    S.nextToken();
	    return name;
	} else if (S.token() == ASSERT) {
	    if (allowAsserts) {
		log.error(S.pos(), "assert.as.identifier");
		S.nextToken();
		return names.error;
	    } else {
		log.warning(S.pos(), "assert.as.identifier");
		Name name = S.name();
		S.nextToken();
		return name;
	    }
	} else if (S.token() == ENUM) {
	    if (allowEnums) {
		log.error(S.pos(), "enum.as.identifier");
		S.nextToken();
		return names.error;
	    } else {
		log.warning(S.pos(), "enum.as.identifier");
		Name name = S.name();
		S.nextToken();
		return name;
	    }
	} else {
	    accept(IDENTIFIER);
	    return names.error;
	}
}

    /**
     * Qualident = Ident { DOT Ident }
     */
    Tree qualident() {
	Tree t = F.at(S.pos()).Ident(ident());
	while (S.token() == DOT) {
	    int pos = S.pos();
	    S.nextToken();
	    t = F.at(pos).Select(t, ident());
	}
	return t;
    }

    /**
     * Literal =
     *     INTLITERAL
     *   | LONGLITERAL
     *   | FLOATLITERAL
     *   | DOUBLELITERAL
     *   | CHARLITERAL
     *   | STRINGLITERAL
     *   | TRUE
     *   | FALSE
     *   | NULL
     */
    Tree literal(Name prefix) {
	int pos = S.pos();
	Tree t = errorTree;
	switch (S.token()) {
	case INTLITERAL:
	    try {
		t = F.at(pos).Literal(
		    TypeTags.INT,
		    Convert.string2int(strval(prefix), S.radix()));
	    } catch (NumberFormatException ex) {
		log.error(S.pos(), "int.number.too.large", strval(prefix));
	    }
	    break;
	case LONGLITERAL:
	    try {
		t = F.at(pos).Literal(
		    TypeTags.LONG,
		    new Long(Convert.string2long(strval(prefix), S.radix())));
	    } catch (NumberFormatException ex) {
		log.error(S.pos(), "int.number.too.large", strval(prefix));
	    }
	    break;
	case FLOATLITERAL: {
	    String proper = (S.radix() == 16 ? ("0x"+ S.stringVal()) : S.stringVal());
	    Float n;
            try {
                n = Float.valueOf(proper);
            } catch (NumberFormatException ex) {
                // error already repoted in scanner
                n = Float.NaN;
            }
	    if (n.floatValue() == 0.0f && !isZero(proper))
		log.error(S.pos(), "fp.number.too.small");
	    else if (n.floatValue() == Float.POSITIVE_INFINITY)
		log.error(S.pos(), "fp.number.too.large");
	    else
		t = F.at(pos).Literal(TypeTags.FLOAT, n);
	    break;
	}
	case DOUBLELITERAL: {
	    String proper = (S.radix() == 16 ? ("0x"+ S.stringVal()) : S.stringVal());
	    Double n;
            try {
                n = Double.valueOf(proper);
            } catch (NumberFormatException ex) {
                // error already reported in scanner
                n = Double.NaN;
            }
	    if (n.doubleValue() == 0.0d && !isZero(proper))
		log.error(S.pos(), "fp.number.too.small");
	    else if (n.doubleValue() == Double.POSITIVE_INFINITY)
		log.error(S.pos(), "fp.number.too.large");
	    else
		t = F.at(pos).Literal(TypeTags.DOUBLE, n);
	    break;
	}
	case CHARLITERAL:
	    t = F.at(pos).Literal(
		TypeTags.CHAR,
                S.stringVal().charAt(0) + 0);
	    break;
	case STRINGLITERAL:
	    t = F.at(pos).Literal(
		TypeTags.CLASS,
		S.stringVal());
	    break;
	case TRUE: case FALSE: case NULL:
	    t = F.at(pos).Ident(S.name());
	    break;
	default:
	    assert false;
	}
	S.nextToken();
	return t;
    }
//where
        boolean isZero(String s) {
	    char[] cs = s.toCharArray();
	    int base = ((Character.toLowerCase(s.charAt(1)) == 'x') ? 16 : 10);
	    int i = ((base==16) ? 2 : 0);
	    while (i < cs.length && (cs[i] == '0' || cs[i] == '.')) i++;
	    return !(i < cs.length && (Character.digit(cs[i], base) > 0));
	}

        String strval(Name prefix) {
	    String s = S.stringVal();
	    return (prefix.len == 0) ? s : prefix + s;
	}

    /** terms can be either expressions or types.
     */
    Tree expression() {
	return term(EXPR);
    }

    Tree type() {
	return term(TYPE);
    }

    Tree term(int newmode) {
	int prevmode = mode;
	mode = newmode;
	Tree t = term();
	lastmode = mode;
	mode = prevmode;
	return t;
    }

    /**
     *  Expression = Expression1 [ExpressionRest]
     *  ExpressionRest = [AssignmentOperator Expression1]
     *  AssignmentOperator = "=" | "+=" | "-=" | "*=" | "/=" |
     *                       "&=" | "|=" | "^=" |
     *                       "%=" | "<<=" | ">>=" | ">>>="
     *  Type = Type1
     *  TypeNoParams = TypeNoParams1
     *  StatementExpression = Expression
     *  ConstantExpression = Expression
     */
    Tree term() {
	Tree t = term1();
	if ((mode & EXPR) != 0 &&
	    S.token() == EQ || PLUSEQ.compareTo(S.token()) <= 0 && S.token().compareTo(GTGTGTEQ) <= 0)
	    return termRest(t);
	else
	    return t;
    }
	
    Tree termRest(Tree t) {
	switch (S.token()) {
	case EQ: {
	    int pos = S.pos();
	    S.nextToken();
	    mode = EXPR;
	    Tree t1 = term();
	    return F.at(pos).Assign(t, t1);
	}
	case PLUSEQ:
	case SUBEQ:
	case STAREQ:
	case SLASHEQ:
	case PERCENTEQ:
	case AMPEQ:
	case BAREQ:
	case CARETEQ:
	case LTLTEQ:
	case GTGTEQ:
	case GTGTGTEQ:
	    int pos = S.pos();
	    Tokens token = S.token();
	    S.nextToken();
	    mode = EXPR;
	    Tree t1 = term();
	    return F.at(pos).Assignop(optag(token), t, t1);
	default:
	    return t;
	}
    }

    /** Expression1   = Expression2 [Expression1Rest]
     *  Type1         = Type2
     *  TypeNoParams1 = TypeNoParams2
     */
    Tree term1() {
        Tree t = term2();
        if ((mode & EXPR) != 0 & S.token() == QUES) {
	    mode = EXPR;
	    return term1Rest(t);
	} else {
	    return t;
	}
    }

    /** Expression1Rest = ["?" Expression ":" Expression1]
     */
    Tree term1Rest(Tree t) {
	if (S.token() == QUES) {
	    int pos = S.pos();
	    S.nextToken();
	    Tree t1 = term();
	    accept(COLON);
	    Tree t2 = term1();
	    return F.at(pos).Conditional(t, t1, t2);
	} else {
	    return t;
	}
    }

    /** Expression2   = Expression3 [Expression2Rest]
     *  Type2         = Type3
     *  TypeNoParams2 = TypeNoParams3
     */
    Tree term2() {
        Tree t = term3();
	if ((mode & EXPR) != 0 && prec(S.token()) >= TreeInfo.orPrec) {
	    mode = EXPR;
	    return term2Rest(t, TreeInfo.orPrec);
	} else {
	    return t;
	}
    }

    /*  Expression2Rest = {infixop Expression3}
     *                  | Expression3 instanceof Type
     *  infixop         = "||"
     *                  | "&&"
     *                  | "|"
     *                  | "^"
     *                  | "&"
     *                  | "==" | "!="
     *                  | "<" | ">" | "<=" | ">="
     *                  | "<<" | ">>" | ">>>"
     *                  | "+" | "-"
     *                  | "*" | "/" | "%"
     */
    Tree term2Rest(Tree t, int minprec) {
	List<Tree[]> savedOd = odStackSupply.elems;
	Tree[] odStack = newOdStack();
	List<Tokens[]> savedOp = opStackSupply.elems;
	Tokens[] opStack = newOpStack();
	// optimization, was odStack = new Tree[...]; opStack = new Tree[...];
	int top = 0;
	odStack[0] = t;
	int startPos = S.pos();
	Tokens topOp = ERROR;
	while (prec(S.token()) >= minprec) {
	    opStack[top] = topOp;
	    top++;
	    topOp = S.token();
	    int pos = S.pos();
	    S.nextToken();
	    odStack[top] = topOp == INSTANCEOF ? type() : term3();
	    while (top > 0 && prec(topOp) >= prec(S.token())) {
		odStack[top-1] = makeOp(pos, topOp, odStack[top-1],
					odStack[top]);
		top--;
		topOp = opStack[top];
	    }
	}
	assert top == 0;
	t = odStack[0];
	
	if (t.tag == Tree.PLUS) {
	    StringBuffer buf = foldStrings(t);
	    if (buf != null) {
		t = F.at(startPos).Literal(TypeTags.CLASS, buf.toString());
	    }
	}

	odStackSupply.elems = savedOd; // optimization
	opStackSupply.elems = savedOp; // optimization
	return t;
    }
//where
        /** Construct a binary or type test node.
	 */
        private Tree makeOp(int pos, Tokens topOp, Tree od1, Tree od2) {
	    if (topOp == INSTANCEOF) {
		return F.at(pos).TypeTest(od1, od2);
	    } else {
		return F.at(pos).Binary(optag(topOp), od1, od2);
	    }
	}
	/** The empty List<String>. */
        private static List<String> emptyStringList = new List<String>();
        /** If tree is a concatenation of string literals, replace it
	 *  by a single literal representing the concatenated string.
	 */
        private static StringBuffer foldStrings(Tree tree) {
	    List<String> buf = emptyStringList;
	    while (true) {
		if (tree.tag == Tree.LITERAL) {
		    Literal lit = (Literal) tree;
		    if (lit.typetag == TypeTags.CLASS) {
			StringBuffer sbuf =
			    new StringBuffer((String)lit.value);
			while (buf.nonEmpty()) {
			    sbuf.append(buf.head);
			    buf = buf.tail;
			}
			return sbuf;
		    }
		} else if (tree.tag == Tree.PLUS) {
		    Binary op = (Binary)tree;
		    if (op.rhs.tag == Tree.LITERAL) {
			Literal lit = (Literal) op.rhs;
			if (lit.typetag == TypeTags.CLASS) {
			    buf = buf.prepend((String) lit.value);
			    tree = op.lhs;
			    continue;
			}
		    }
		}
		return null;
	    }
	}

        /** optimization: To save allocating a new operand/operator stack
	 *  for every binary operation, we use supplys.
	 */
        ListBuffer<Tree[]> odStackSupply = new ListBuffer<Tree[]>();
        ListBuffer<Tokens[]> opStackSupply = new ListBuffer<Tokens[]>();

        private Tree[] newOdStack() {
	    if (odStackSupply.elems == odStackSupply.last)
		odStackSupply.append(new Tree[infixPrecedenceLevels + 1]);
	    Tree[] odStack = odStackSupply.elems.head;
	    odStackSupply.elems = odStackSupply.elems.tail;
	    return odStack;
	}

        private Tokens[] newOpStack() {
	    if (opStackSupply.elems == opStackSupply.last)
		opStackSupply.append(new Tokens[infixPrecedenceLevels + 1]);
	    Tokens[] opStack = opStackSupply.elems.head;
	    opStackSupply.elems = opStackSupply.elems.tail;
	    return opStack;
	}

    /** Expression3    = PrefixOp Expression3
     *                 | "(" Expr | TypeNoParams ")" Expression3
     *                 | Primary {Selector} {PostfixOp}
     *  Primary        = "(" Expression ")"
     *                 | Literal
     *                 | [TypeArguments] THIS [Arguments]
     *                 | [TypeArguments] SUPER SuperSuffix
     *                 | NEW [TypeArguments] Creator
     *                 | Ident { "." Ident }
     *                   [ "[" ( "]" BracketsOpt "." CLASS | Expression "]" )
     *                   | Arguments
     *                   | "." ( CLASS | THIS | [TypeArguments] SUPER Arguments | NEW [TypeArguments] InnerCreator )
     *                   ]
     *                 | BasicType BracketsOpt "." CLASS
     *  PrefixOp       = "++" | "--" | "!" | "~" | "+" | "-"
     *  PostfixOp      = "++" | "--"
     *  Type3          = Ident { "." Ident } [TypeArguments] {TypeSelector} BracketsOpt
     *                 | BasicType
     *  TypeNoParams3  = Ident { "." Ident } BracketsOpt
     *  Selector       = "." [TypeArguments] Ident [Arguments]
     *                 | "." THIS
     *                 | "." [TypeArguments] SUPER SuperSuffix
     *                 | "." NEW [TypeArguments] InnerCreator
     *                 | "[" Expression "]"
     *  TypeSelector   = "." Ident [TypeArguments]
     *  SuperSuffix    = Arguments | "." Ident [Arguments]
     */
    Tree term3() {
	int pos = S.pos();
	Tree t;
	List<Tree> typeArgs = typeArgumentsOpt(EXPR);
	switch (S.token()) {
	case QUES:
	    if ((mode & TYPE) != 0 && (mode & (TYPEARG|NOPARAMS)) == TYPEARG) {
                mode = TYPE;
                return typeArgument();
            } else
                return illegal();
	case PLUSPLUS: case SUBSUB: case BANG: case TILDE: case PLUS: case SUB:
	    if (typeArgs == null && (mode & EXPR) != 0) {
		Tokens token = S.token();
                S.nextToken();
		mode = EXPR;
		if (token == SUB &&
		    (S.token() == INTLITERAL || S.token() == LONGLITERAL) &&
		    S.radix() == 10) {
                    mode = EXPR;
		    t = literal(names.hyphen);
		} else {
		    t = term3();
		    return F.at(pos).Unary(unoptag(token), t);
		}
	    } else return illegal();
	    break;
	case LPAREN:
	    if (typeArgs == null && (mode & EXPR) != 0) {
		S.nextToken();
		mode = EXPR | TYPE | NOPARAMS;
		t = term3();
		if ((mode & TYPE) != 0 && S.token() == LT) {
		    // Could be a cast to a parameterized type
		    int op = Tree.LT;
		    int pos1 = S.pos();
		    S.nextToken();
		    mode &= (EXPR | TYPE);
		    mode |= TYPEARG;
		    Tree t1 = term3();
		    if ((mode & TYPE) != 0 &&
			(S.token() == COMMA || S.token() == GT)) {
			mode = TYPE;
			ListBuffer<Tree> args = new ListBuffer<Tree>();
			args.append(t1);
			while (S.token() == COMMA) {
			    S.nextToken();
			    args.append(typeArgument());
			}
			accept(GT);
			t = F.at(pos1).TypeApply(t, args.toList());
                        checkGenerics();
			t = bracketsOpt(t);
		    } else if ((mode & EXPR) != 0) {
			mode = EXPR;
			t = F.at(pos1).Binary(op, t, term2Rest(t1, TreeInfo.shiftPrec));
			t = termRest(term1Rest(term2Rest(t, TreeInfo.orPrec)));
		    } else {
			accept(GT);
		    }
		} else {
		    t = termRest(term1Rest(term2Rest(t, TreeInfo.orPrec)));
		}
		accept(RPAREN);
		lastmode = mode;
		mode = EXPR;
		if ((lastmode & EXPR) == 0) {
		    Tree t1 = term3();
		    return F.at(pos).TypeCast(t, t1);
		} else if ((lastmode & TYPE) != 0) {
		    switch (S.token()) {
		    /*case PLUSPLUS: case SUBSUB: */
		    case BANG: case TILDE:
		    case LPAREN: case THIS: case SUPER:
		    case INTLITERAL: case LONGLITERAL: case FLOATLITERAL:
		    case DOUBLELITERAL: case CHARLITERAL: case STRINGLITERAL:
		    case TRUE: case FALSE: case NULL:
		    case NEW: case IDENTIFIER: case ASSERT: case ENUM:
		    case BYTE: case SHORT: case CHAR: case INT:
		    case LONG: case FLOAT: case DOUBLE: case BOOLEAN: case VOID:
			Tree t1 = term3();
			return F.at(pos).TypeCast(t, t1);
		    }
		}
	    } else return illegal();
	    t = F.at(pos).Parens(t);
	    break;
	case THIS:
	    if ((mode & EXPR) != 0) {
		mode = EXPR;
		t = F.at(pos).Ident(names._this);
		S.nextToken();
		if (typeArgs == null)
		    t = argumentsOpt(null, t);
		else
		    t = arguments(typeArgs, t);
		typeArgs = null;
	    } else return illegal();
	    break;
	case SUPER:
	    if ((mode & EXPR) != 0) {
		mode = EXPR;
		t = superSuffix(typeArgs, F.at(pos).Ident(names._super));
		typeArgs = null;
	    } else return illegal();
	    break;
	case INTLITERAL: case LONGLITERAL: case FLOATLITERAL: case DOUBLELITERAL:
	case CHARLITERAL: case STRINGLITERAL:
	case TRUE: case FALSE: case NULL:
	    if (typeArgs == null && (mode & EXPR) != 0) {
		mode = EXPR;
		t = literal(names.empty);
	    } else return illegal();
	    break;
	case NEW:
	    if (typeArgs != null) return illegal();
	    if ((mode & EXPR) != 0) {
		mode = EXPR;
		S.nextToken();
		if (S.token() == LT) typeArgs = typeArguments();
		t = creator(pos, typeArgs);
		typeArgs = null;
	    } else return illegal();
	    break;
	case IDENTIFIER: case ASSERT: case ENUM:
	    if (typeArgs != null) return illegal();
	    t = F.at(S.pos()).Ident(ident());
	    loop: while (true) {
		pos = S.pos();
		switch (S.token()) {
		case LBRACKET:
		    S.nextToken();
                    if (S.token() == RBRACKET) {
 			S.nextToken();
 			t = bracketsSuffix(F.at(pos).TypeArray(bracketsOpt(t)));
                    } else {
			if ((mode & EXPR) != 0) {
			    mode = EXPR;
			    Tree t1 = term();
			    t = F.at(pos).Indexed(t, t1);
			}
			accept(RBRACKET);
		    }
		    break loop;
		case LPAREN:
		    if ((mode & EXPR) != 0) {
			mode = EXPR;
			t = arguments(typeArgs, t);
			typeArgs = null;
		    }
		    break loop;
		case DOT:
		    S.nextToken();
                    typeArgs = typeArgumentsOpt(EXPR);
		    if ((mode & EXPR) != 0) {
			switch (S.token()) {
			case CLASS:
			    if (typeArgs != null) return illegal();
			    mode = EXPR;
			    t = F.at(pos).Select(t, names._class);
			    S.nextToken();
			    break loop;
			case THIS:
			    if (typeArgs != null) return illegal();
			    mode = EXPR;
			    t = F.at(pos).Select(t, names._this);
			    S.nextToken();
			    break loop;
			case SUPER:
			    mode = EXPR;
			    t = superSuffix(typeArgs, F.at(pos).Select(t, names._super));
			    typeArgs = null;
			    break loop;
			case NEW:
			    if (typeArgs != null) return illegal();
			    mode = EXPR;
			    int pos1 = S.pos();
			    S.nextToken();
			    if (S.token() == LT) typeArgs = typeArguments();
			    t = innerCreator(pos1, typeArgs, t);
			    typeArgs = null;
			    break loop;
			}
		    }
		    // typeArgs saved for next loop iteration.
		    t = F.at(pos).Select(t, ident());
		    break;
		default:
		    break loop;
		}
	    }
	    if (typeArgs != null) illegal();
	    t = typeArgumentsOpt(t);
	    break;
	case BYTE: case SHORT: case CHAR: case INT: case LONG: case FLOAT:
	case DOUBLE: case BOOLEAN:
	    if (typeArgs != null) illegal();
	    t = bracketsSuffix(bracketsOpt(basicType()));
	    break;
	case VOID:
	    if (typeArgs != null) illegal();
	    if ((mode & EXPR) != 0) {
		S.nextToken();
		if (S.token() == DOT) {
		    t = bracketsSuffix(F.at(pos).TypeIdent(TypeTags.VOID));
		} else {
		    return illegal(pos);
		}
	    } else {
		return illegal();
	    }
	    break;
	default:
	    return illegal();
	}
	if (typeArgs != null) illegal();
	while (true) {
	    int pos1 = S.pos();
	    if (S.token() == LBRACKET) {
		S.nextToken();
		if ((mode & TYPE) != 0) {
		    int oldmode = mode;
		    mode = TYPE;
                    if (S.token() == RBRACKET) {
			S.nextToken();
			return F.at(pos1).TypeArray(bracketsOpt(t));
		    }
		    mode = oldmode;
		}
		if ((mode & EXPR) != 0) {
		    mode = EXPR;
		    Tree t1 = term();
		    t = F.at(pos1).Indexed(t, t1);
		}
		accept(RBRACKET);
	    } else if (S.token() == DOT) {
		S.nextToken();
		typeArgs = typeArgumentsOpt(EXPR);
		if (S.token() == SUPER && (mode & EXPR) != 0) {
		    mode = EXPR;
		    t = F.at(pos).Select(t, names._super);
		    S.nextToken();
		    t = arguments(typeArgs, t);
		    typeArgs = null;
		} else if (S.token() == NEW && (mode & EXPR) != 0) {
		    if (typeArgs != null) return illegal();
		    mode = EXPR;
		    int pos2 = S.pos();
		    S.nextToken();
		    if (S.token() == LT) typeArgs = typeArguments();
		    t = innerCreator(pos2, typeArgs, t);
		    typeArgs = null;
		} else {
		    t = argumentsOpt(typeArgs,
			typeArgumentsOpt(F.at(pos).Select(t, ident())));
		    typeArgs = null;
		}
	    } else {
		break;
	    }
	}
	while ((S.token() == PLUSPLUS || S.token() == SUBSUB) && (mode & EXPR) != 0) {
	    mode = EXPR;
	    t = F.at(S.pos()).Unary(
		  S.token() == PLUSPLUS ? Tree.POSTINC : Tree.POSTDEC, t);
	    S.nextToken();
	}
	if (genEndPos) endPositions.put(t, S.prevEndPos());
	return t;
    }

    /** SuperSuffix = Arguments | "." [TypeArguments] Ident [Arguments]
     */
    Tree superSuffix(List<Tree> typeArgs, Tree t) {
	S.nextToken();
	if (S.token() == LPAREN || typeArgs != null) {
	    t = arguments(typeArgs, t);
	} else {
	    int pos = S.pos();
	    accept(DOT);
            t = argumentsOpt((S.token() == LT) ? typeArguments() : null,
                             F.at(pos).Select(t, ident()));
	}
	return t;
    }

    /** BasicType = BYTE | SHORT | CHAR | INT | LONG | FLOAT | DOUBLE | BOOLEAN
     */
    Tree basicType() {
	Tree t = F.at(S.pos()).TypeIdent(typetag(S.token()));
	S.nextToken();
	return t;
    }

    /** ArgumentsOpt = [ Arguments ]
     */
    Tree argumentsOpt(List<Tree> typeArgs, Tree t) {
	if ((mode & EXPR) != 0 && S.token() == LPAREN || typeArgs != null) {
	    mode = EXPR;
	    return arguments(typeArgs, t);
	} else {
	    return t;
	}
    }

    /** Arguments = "(" [Expression { COMMA Expression }] ")"
     */
    List<Tree> arguments() {
	int pos = S.pos();
	ListBuffer<Tree> args = new ListBuffer<Tree>();
	if (S.token() == LPAREN) {
	    S.nextToken();
	    if (S.token() != RPAREN) {
		args.append(expression());
		while (S.token() == COMMA) {
		    S.nextToken();
		    args.append(expression());
		}
	    }
	    accept(RPAREN);
	} else {
	    syntaxError(S.pos(), "expected", keywords.token2string(LPAREN));
	}
	return args.toList();
    }

    Tree arguments(List<Tree> typeArgs, Tree t) {
	int pos = S.pos();
	List<Tree> args = arguments();
	return F.at(pos).Apply(typeArgs, t, args);
    }

    /**  TypeArgumentsOpt = [ TypeArguments ]
     */
    Tree typeArgumentsOpt(Tree t) {
	if (S.token() == LT &&
	    (mode & TYPE) != 0 &&
	    (mode & NOPARAMS) == 0) {
	    mode = TYPE;
            checkGenerics();
	    return typeArguments(t);
	} else {
	    return t;
	}
    }
    List<Tree> typeArgumentsOpt() {
        return typeArgumentsOpt(TYPE);
    }

    List<Tree> typeArgumentsOpt(int useMode) {
	if (S.token() == LT) {
            checkGenerics();
	    if ((mode & useMode) == 0 ||
                (mode & NOPARAMS) != 0) {
                illegal();
            }
            mode = useMode;
            return typeArguments();
	}
        return null;
    }

    /**  TypeArguments  = "<" TypeArgument {"," TypeArgument} ">"
     */
    List<Tree> typeArguments() {
	int pos = S.pos();
	ListBuffer<Tree> args = new ListBuffer<Tree>();
	if (S.token() == LT) {
	    S.nextToken();
            args.append(((mode & EXPR) == 0) ? typeArgument() : type());
	    while (S.token() == COMMA) {
		S.nextToken();
                args.append(((mode & EXPR) == 0) ? typeArgument() : type());
	    }
	    switch (S.token()) {
	    case GTGTGTEQ:
		S.token(GTGTEQ);
		break;
	    case GTGTEQ:
		S.token(GTEQ);
		break;
	    case GTEQ:
		S.token(EQ);
		break;
	    case GTGTGT:
		S.token(GTGT);
		break;
	    case GTGT:
		S.token(GT);
		break;
	    default:
		accept(GT);
		break;
	    }
	} else {
	    syntaxError(S.pos(), "expected", keywords.token2string(LT));
	}
	return args.toList();
    }

    /** TypeArgument = Type
     *               | "?"
     *               | "?" EXTENDS Type {"&" Type}
     *               | "?" SUPER Type
     */
    Tree typeArgument() {
	Tree t = null;
	if (S.token() != QUES) return type();
	S.nextToken();
	if (S.token() == EXTENDS) {
	    S.nextToken();
            return F.at(S.pos()).TypeArgument(F.TypeBoundKind(BoundKind.EXTENDS), type());
	} else if (S.token() == SUPER) {
	    S.nextToken();
            return F.at(S.pos()).TypeArgument(F.TypeBoundKind(BoundKind.SUPER), type());
	} else {
            return F.at(S.pos()).TypeArgument(F.TypeBoundKind(BoundKind.UNBOUND), null);
	}
    }


    Tree typeArguments(Tree t) {
	int pos = S.pos();
	List<Tree> args = typeArguments();
	return F.at(pos).TypeApply(t, args);
    }

    /** BracketsOpt = {"[" "]"}
     */
    private Tree bracketsOpt(Tree t) {
	if (S.token() == LBRACKET) {
	    int pos = S.pos();
	    S.nextToken();
	    t = bracketsOptCont(t, pos);
	}
	return t;
    }

    private Tree bracketsOptCont(Tree t, int pos) {
	accept(RBRACKET);
        t = bracketsOpt(t);
	return F.at(pos).TypeArray(t);
    }

    /** BracketsSuffixExpr = "." CLASS
     *  BracketsSuffixType =
     */
    Tree bracketsSuffix(Tree t) {
	if ((mode & EXPR) != 0 && S.token() == DOT) {
	    mode = EXPR;
	    int pos = S.pos();
	    S.nextToken();
	    accept(CLASS);
	    t = F.at(pos).Select(t, names._class);
	} else if ((mode & TYPE) != 0) {
	    mode = TYPE;
	} else {
	    syntaxError(S.pos(), "dot.class.expected");
	}
	return t;
    }

    /** Creator = Qualident [TypeArguments] ( ArrayCreatorRest | ClassCreatorRest )
     */
    Tree creator(int newpos, List<Tree> typeArgs) {
	switch (S.token()) {
	case BYTE: case SHORT: case CHAR: case INT: case LONG: case FLOAT:
	case DOUBLE: case BOOLEAN:
	    if (typeArgs == null) return arrayCreatorRest(newpos, basicType());
	    // else fall through for syntax error
	default:
	    Tree t = qualident();
	    int oldmode = mode;
	    mode = TYPE;
	    if (S.token() == LT) {
                checkGenerics();
                t = typeArguments(t);
            }
	    mode = oldmode;
	    if (S.token() == LBRACKET) return arrayCreatorRest(newpos, t);
	    else if (S.token() == LPAREN) return classCreatorRest(newpos, null, typeArgs, t);
	    else return syntaxError("left-paren.or.left-square-bracket.expected");
	}
    }

    /** InnerCreator = Ident [TypeArguments] ClassCreatorRest
     */
    Tree innerCreator(int newpos, List<Tree> typeArgs, Tree encl) {
	Tree t = F.at(S.pos()).Ident(ident());
	if (S.token() == LT) {
            checkGenerics();
            t = typeArguments(t);
        }
	return classCreatorRest(newpos, encl, typeArgs, t);
    }

    /** ArrayCreatorRest = "[" ( "]" BracketsOpt ArrayInitializer
     *                         | Expression "]" {"[" Expression "]"} BracketsOpt )
     */
    Tree arrayCreatorRest(int newpos, Tree elemtype) {
	accept(LBRACKET);
	if (S.token() == RBRACKET) {
	    accept(RBRACKET);
	    elemtype = bracketsOpt(elemtype);
	    if (S.token() == LBRACE) {
		return arrayInitializer(elemtype);
	    } else {
		syntaxError(S.pos(), "array.dimension.missing");
		return errorTree;
	    }
	} else {
	    ListBuffer<Tree> dims = new ListBuffer<Tree>();
	    dims.append(expression());
	    accept(RBRACKET);
	    while (S.token() == LBRACKET) {
		int pos = S.pos();
		S.nextToken();
		if (S.token() == RBRACKET) {
		    elemtype = bracketsOptCont(elemtype, pos);
		} else {
		    dims.append(expression());
		    accept(RBRACKET);
		}
	    }
	    return F.at(newpos).NewArray(elemtype, dims.toList(), null);
        }
    }

    /** ClassCreatorRest = Arguments [ClassBody]
     */
    Tree classCreatorRest(int newpos, Tree encl, List<Tree> typeArgs, Tree t) {
	List<Tree> args = arguments();
	ClassDef body = null;
	if (S.token() == LBRACE)
	     body = F.at(S.pos()).ClassDef(
		 F.Modifiers(0, Annotation.emptyList),
		 names.empty,
		 TypeParameter.emptyList,
		 null,
		 Tree.emptyList,
		 classOrInterfaceBody(names.empty, false));
	return F.at(newpos).NewClass(encl, typeArgs, t, args, body);
    }

    /** ArrayInitializer = "{" [VariableInitializer {"," VariableInitializer}] [","] "}"
     */
    Tree arrayInitializer(Tree t) {
        int pos = S.pos();
        accept(LBRACE);
        ListBuffer<Tree> elems = new ListBuffer<Tree>();
	if (S.token() == COMMA) {
	    S.nextToken();
	} else if (S.token() != RBRACE) {
	    elems.append(variableInitializer());
            while (S.token() == COMMA) {
                S.nextToken();
                if (S.token() == RBRACE) break;
                elems.append(variableInitializer());
            }
        }
        accept(RBRACE);
        return F.at(pos).NewArray(t, Tree.emptyList, elems.toList());
    }

    /** VariableInitializer = ArrayInitializer | Expression
     */
    Tree variableInitializer() {
	return S.token() == LBRACE ? arrayInitializer(null) : expression();
    }

    /** ParExpression = "(" Expression ")"
     */
    Tree parExpression() {
	int pos = S.pos();
	accept(LPAREN);
	Tree t = expression();
	accept(RPAREN);
	return genEndPos ? F.at(pos).Parens(t) : t;
    }

    /** Block = "{" BlockStatements "}"
     */
    Block block(long flags) {
        int pos = S.pos();
        accept(LBRACE);
	List<Tree> stats = blockStatements();
        Block t = F.at(pos).Block(flags, stats);
        while (S.token() == CASE || S.token() == DEFAULT) {
	    syntaxError("orphaned", keywords.token2string(S.token()));
	    blockStatements();
	}
	t.endpos = S.pos();
        accept(RBRACE);
        return t;
    }

    Block block() {
	return block(0);
    }

    /** BlockStatements = { BlockStatement }
     *  BlockStatement  = LocalVariableDeclarationStatement
     *                  | ClassOrInterfaceOrEnumDeclaration
     *                  | [Ident ":"] Statement
     *  LocalVariableDeclarationStatement
     *                  = { FINAL | '@' Annotation } Type VariableDeclarators ";"
     */
    List<Tree> blockStatements() {
//todo: skip to anchor on error(?)
	ListBuffer<Tree> stats = new ListBuffer<Tree>();
        while (true) {
	    int pos = S.pos();
            switch (S.token()) {
            case RBRACE: case CASE: case DEFAULT: case EOF:
                return stats.toList();
	    case LBRACE: case IF: case FOR: case WHILE: case DO: case TRY:
	    case SWITCH: case SYNCHRONIZED: case RETURN: case THROW: case BREAK:
	    case CONTINUE: case SEMI: case ELSE: case FINALLY: case CATCH:
		stats.append(statement());
		break;
	    case MONKEYS_AT:
	    case FINAL: {
	        String dc = S.docComment();
		Modifiers mods = modifiersOpt();
		if (S.token() == INTERFACE ||
		    S.token() == CLASS ||
		    allowEnums && S.token() == ENUM) {
		    stats.append(classOrInterfaceOrEnumDeclaration(mods, dc));
		} else {
		    pos = S.pos();
		    Tree t = type();
		    stats.appendList(variableDeclarators(mods, t));
		    accept(SEMI);
		}
		break;
	    }
	    case ABSTRACT: case STRICTFP: {
	        String dc = S.docComment();
		Modifiers mods = modifiersOpt();
		stats.append(classOrInterfaceOrEnumDeclaration(mods, dc));
		break;
	    }
	    case INTERFACE:
	    case CLASS:
		stats.append(classOrInterfaceOrEnumDeclaration(modifiersOpt(),
							       S.docComment()));
		break;
	    case ENUM:
	    case ASSERT:
		if (allowEnums && S.token() == ENUM) {
                    log.error(S.pos(), "local.enum");
		    stats.
			append(classOrInterfaceOrEnumDeclaration(modifiersOpt(),
								 S.docComment()));
		    break;
		} else if (allowAsserts && S.token() == ASSERT) {
		    stats.append(statement());
		    break;
		}
		/* fall through to default */
            default:
		Name name = S.name();
		Tree t = term(EXPR | TYPE);
		if (S.token() == COLON && t.tag == Tree.IDENT) {
		    S.nextToken();
		    Tree stat = statement();
		    stats.append(F.at(pos).Labelled(name, stat));
		} else if ((lastmode & TYPE) != 0 &&
			   (S.token() == IDENTIFIER ||
			    S.token() == ASSERT ||
			    S.token() == ENUM)) {
		    stats.appendList(variableDeclarators(F.at(S.pos()).
							 Modifiers(0),
							 t));
		    accept(SEMI);
		} else {
		    stats.append(F.at(pos).Exec(checkExprStat(t)));
		    accept(SEMI);
		}
	    }
	}
    }

    /** Statement =
     *       Block
     *     | IF ParExpression Statement [ELSE Statement]
     *     | FOR "(" ForInitOpt ";" [Expression] ";" ForUpdateOpt ")" Statement
     *     | FOR "(" FormalParameter : Expression ")" Statement
     *     | WHILE ParExpression Statement
     *     | DO Statement WHILE ParExpression ";"
     *     | TRY Block ( Catches | [Catches] FinallyPart )
     *     | SWITCH ParExpression "{" SwitchBlockStatementGroups "}"
     *     | SYNCHRONIZED ParExpression Block
     *     | RETURN [Expression] ";"
     *     | THROW Expression ";"
     *     | BREAK [Ident] ";"
     *     | CONTINUE [Ident] ";"
     *     | ASSERT Expression [ ":" Expression ] ";"
     *     | ";"
     *     | ExpressionStatement
     *     | Ident ":" Statement
     */
    Tree statement() {
	int pos = S.pos();
	switch (S.token()) {
	case LBRACE:
	    return block();
	case IF: {
	    S.nextToken();
	    Tree cond = parExpression();
	    Tree thenpart = statement();
	    Tree elsepart = null;
	    if (S.token() == ELSE) {
		S.nextToken();
		elsepart = statement();
	    }
	    return F.at(pos).If(cond, thenpart, elsepart);
	}
	case FOR: {
	    S.nextToken();
	    accept(LPAREN);
	    List<Tree> inits = S.token() == SEMI ? Tree.emptyList : forInit();
	    if (inits.length() == 1 &&
		inits.head.tag == Tree.VARDEF &&
		((VarDef)inits.head).init == null &&
		S.token() == COLON) {
                checkForeach();
		VarDef var = (VarDef)inits.head;
		accept(COLON);
		Tree expr = expression();
		accept(RPAREN);
		Tree body = statement();
		return F.at(pos).ForeachLoop(var, expr, body);
	    } else {
		accept(SEMI);
		Tree cond = S.token() == SEMI ? null : expression();
		accept(SEMI);
		List<Tree> steps = S.token() == RPAREN ? Tree.emptyList : forUpdate();
		accept(RPAREN);
		Tree body = statement();
		return F.at(pos).ForLoop(inits, cond, steps, body);
	    }
	}
	case WHILE: {
	    S.nextToken();
	    Tree cond = parExpression();
	    Tree body = statement();
	    return F.at(pos).WhileLoop(cond, body);
	}
	case DO: {
	    S.nextToken();
	    Tree body = statement();
	    accept(WHILE);
	    Tree cond = parExpression();
	    Tree t = F.at(pos).DoLoop(body, cond);
	    if (genEndPos) endPositions.put(t, S.endPos());
	    accept(SEMI);
	    return t;
	}
	case TRY: {
	    S.nextToken();
	    Tree body = block();
	    ListBuffer<Catch> catchers = new ListBuffer<Catch>();
	    Tree finalizer = null;
	    if (S.token() == CATCH || S.token() == FINALLY) {
		while (S.token() == CATCH) catchers.append(catchClause());
		if (S.token() == FINALLY) {
		    S.nextToken();
		    finalizer = block();
		}
	    } else {
		log.error(pos, "try.without.catch.or.finally");
	    }
	    return F.at(pos).Try(body, catchers.toList(), finalizer);
	}
	case SWITCH: {
	    S.nextToken();
	    Tree selector = parExpression();
	    accept(LBRACE);
	    List<Case> cases = switchBlockStatementGroups();
	    Tree t = F.at(pos).Switch(selector, cases);
	    if (genEndPos) endPositions.put(t, S.endPos());
	    accept(RBRACE);
	    return t;
	}
	case SYNCHRONIZED: {
	    S.nextToken();
	    Tree lock = parExpression();
	    Tree body = block();
	    return F.at(pos).Synchronized(lock, body);
	}
	case RETURN: {
	    S.nextToken();
	    Tree result = S.token() == SEMI ? null : expression();
	    Tree t = F.at(pos).Return(result);
	    if (genEndPos) endPositions.put(t, S.endPos());
	    accept(SEMI);
	    return t;
	}
	case THROW: {
	    S.nextToken();
	    Tree exc = expression();
	    Tree t = F.at(pos).Throw(exc);
	    if (genEndPos) endPositions.put(t, S.endPos());
	    accept(SEMI);
	    return t;
	}
	case BREAK: {
	    S.nextToken();
	    Name label = (S.token() == IDENTIFIER || S.token() == ASSERT || S.token() == ENUM) ? ident() : null;
	    Tree t = F.at(pos).Break(label);
	    if (genEndPos) endPositions.put(t, S.prevEndPos());
	    accept(SEMI);
	    return t;
	}
	case CONTINUE: {
	    S.nextToken();
	    Name label = (S.token() == IDENTIFIER || S.token() == ASSERT || S.token() == ENUM) ? ident() : null;
	    Tree t =  F.at(pos).Continue(label);
	    if (genEndPos) endPositions.put(t, S.prevEndPos());
	    accept(SEMI);
	    return t;
	}
	case SEMI:
	    S.nextToken();
	    return F.at(pos).Skip();
	case ELSE:
	    return syntaxError("else.without.if");
	case FINALLY:
	    return syntaxError("finally.without.try");
	case CATCH:
	    return syntaxError("catch.without.try");
	case ASSERT: {
	    if (allowAsserts && S.token() == ASSERT) {
		S.nextToken();
		Tree assertion = expression();
		Tree message = null;
		if (S.token() == COLON) {
		    S.nextToken();
		    message = expression();
		}
		Tree t = F.at(pos).Assert(assertion, message);
		accept(SEMI);
		return t;
	    }
	    /* else fall through to default case */
	}
	case ENUM:
	default:
	    Name name = S.name();
	    Tree expr = expression();
	    if (S.token() == COLON && expr.tag == Tree.IDENT) {
		S.nextToken();
		Tree stat = statement();
		return F.at(pos).Labelled(name, stat);
	    } else {
		Tree stat = F.at(pos).Exec(checkExprStat(expr));
		accept(SEMI);
		return stat;
	    }
	}
    }
	
    /** CatchClause	= CATCH "(" FormalParameter ")" Block
     */
    Catch catchClause() {
	int pos = S.pos();
	accept(CATCH);
	accept(LPAREN);
	VarDef formal =
	    variableDeclaratorId(optFinal(Flags.PARAMETER),
				 qualident());
	accept(RPAREN);
	Tree body = block();
	return F.at(pos).Catch(formal, body);
    }

    /** SwitchBlockStatementGroups = { SwitchBlockStatementGroup }
     *  SwitchBlockStatementGroup = SwitchLabel BlockStatements
     *  SwitchLabel = CASE ConstantExpression ":" | DEFAULT ":"
     */
    List<Case> switchBlockStatementGroups() {
	ListBuffer<Case> cases = new ListBuffer<Case>();
	while (true) {
	    int pos = S.pos();
	    switch (S.token()) {
	    case CASE: {
		S.nextToken();
		Tree pat = expression();
		accept(COLON);
		List<Tree> stats = blockStatements();
		cases.append(F.at(pos).Case(pat, stats));
		break;
	    }
	    case DEFAULT: {
		S.nextToken();
		accept(COLON);
		List<Tree> stats = blockStatements();
		cases.append(F.at(pos).Case(null, stats));
		break;
	    }
            case RBRACE: case EOF:
                return cases.toList();
	    default:
		S.nextToken(); // to ensure progress
		syntaxError(pos, "case.default.or.right-brace.expected");
	    }
	}
    }

    /** MoreStatementExpressions = { COMMA StatementExpression }
     */
    List<Tree> moreStatementExpressions(int pos, Tree first) {
	ListBuffer<Tree> stats = new ListBuffer<Tree>();
	stats.append(F.at(pos).Exec(checkExprStat(first)));
	while (S.token() == COMMA) {
	    S.nextToken();
	    pos = S.pos();
	    Tree t = expression();
	    stats.append(F.at(pos).Exec(checkExprStat(t)));
	}
	return stats.toList();
    }

    /** ForInit = StatementExpression MoreStatementExpressions
     *           |  { FINAL | '@' Annotation } Type VariableDeclarators
     */
    List<Tree> forInit() {
	int pos = S.pos();
	if (S.token() == FINAL || S.token() == MONKEYS_AT) {
	    return variableDeclarators(optFinal(0), type());
	} else {
	    Tree t = term(EXPR | TYPE);
	    if ((lastmode & TYPE) != 0 &&
		(S.token() == IDENTIFIER || S.token() == ASSERT || S.token() == ENUM))
		return variableDeclarators(modifiersOpt(), t);
	    else
		return moreStatementExpressions(pos, t);
	}
    }

    /** ForUpdate = StatementExpression MoreStatementExpressions
     */
    List<Tree> forUpdate() {
	return moreStatementExpressions(S.pos(), expression());
    }

    /** AnnotationsOpt = { '@' Annotation }
     */
    List<Annotation> annotationsOpt() {
	if (S.token() != MONKEYS_AT) return Annotation.emptyList; // optimization
	ListBuffer<Annotation> buf = new ListBuffer<Annotation>();
	while (S.token() == MONKEYS_AT) {
	    S.nextToken();
	    buf.append(annotation());
	}
	return buf.toList();
    }

    /** ModifiersOpt = { Modifier }
     *  Modifier = PUBLIC | PROTECTED | PRIVATE | STATIC | ABSTRACT | FINAL
     *           | NATIVE | SYNCHRONIZED | TRANSIENT | VOLATILE | "@"
     *           | "@" Annotation
     */
    Modifiers modifiersOpt() {
	return modifiersOpt(null);
    }
    Modifiers modifiersOpt(Modifiers partial) {
        long flags = (partial == null) ? 0 : partial.flags;
	if (S.deprecatedFlag()) {
	    flags = Flags.DEPRECATED;
	    S.resetDeprecatedFlag();
	}
	ListBuffer<Annotation> annotations = new ListBuffer<Annotation>();
	if (partial != null) annotations.appendList(partial.annotations);
    loop:
        while (true) {
	    long flag;
            switch (S.token()) {
            case PRIVATE     : flag = Flags.PRIVATE; break;
            case PROTECTED   : flag = Flags.PROTECTED; break;
            case PUBLIC      : flag = Flags.PUBLIC; break;
            case STATIC      : flag = Flags.STATIC; break;
            case TRANSIENT   : flag = Flags.TRANSIENT; break;
            case FINAL       : flag = Flags.FINAL; break;
            case ABSTRACT    : flag = Flags.ABSTRACT; break;
            case NATIVE      : flag = Flags.NATIVE; break;
            case VOLATILE    : flag = Flags.VOLATILE; break;
            case SYNCHRONIZED: flag = Flags.SYNCHRONIZED; break;
            case STRICTFP    : flag = Flags.STRICTFP; break;
	    case MONKEYS_AT  : flag = Flags.ANNOTATION; break;
            default: break loop;
            }
	    if ((flags & flag) != 0) log.error(S.pos(), "repeated.modifier");
	    S.nextToken();
	    if (flag == Flags.ANNOTATION) {
                checkAnnotations();
                if (S.token() != INTERFACE) {
                    annotations.append(annotation());
                    flag = 0;
                }
            }
            flags |= flag;
        }
	switch (S.token()) {
	case ENUM: flags |= Flags.ENUM; break;
	case INTERFACE: flags |= Flags.INTERFACE; break;
	default: break;
	}
	return new Modifiers(flags, annotations.toList());
    }

    /** Annotation		= "@" Qualident [ "(" AnnotationFieldValues ")" ] */
    Annotation annotation() {
	// accept(AT); // AT consumed by caller
        checkAnnotations();
	return F.Annotation(qualident(), annotationFieldValuesOpt());
    }

    List<Tree> annotationFieldValuesOpt() {
	return (S.token() == LPAREN) ? annotationFieldValues() : Tree.emptyList;
    }

    /** AnnotationFieldValues	= "(" [ AnnotationFieldValue { "," AnnotationFieldValue } ] ")" */
    List<Tree> annotationFieldValues() {
	int pos = S.pos();
	accept(LPAREN);
	ListBuffer<Tree> buf = new ListBuffer<Tree>();
	if (S.token() != RPAREN) {
	    buf.append(annotationFieldValue());
	    while (S.token() == COMMA) {
		S.nextToken();
		buf.append(annotationFieldValue());
	    }
	}
	accept(RPAREN);
	return buf.toList();
    }

    /** AnnotationFieldValue	= AnnotationValue
     *				| Identifier "=" AnnotationValue
     */
    Tree annotationFieldValue() {
	if (S.token() == IDENTIFIER) {
	    mode = EXPR;
	    Tree t1 = term1();
	    if (t1.tag == Tree.IDENT && S.token() == EQ) {
		int pos = S.pos();
		accept(EQ);
		return F.at(pos).Assign(t1, annotationValue());
	    } else {
		return t1;
	    }
	}
	return annotationValue();
    }

    /* AnnotationValue		= ConditionalExpression
     *				| Annotation
     *				| "{" [ AnnotationValue { "," AnnotationValue } ] "}"
     */
    Tree annotationValue() {
	switch (S.token()) {
	case MONKEYS_AT:
	    S.nextToken();
	    return annotation();
	case LBRACE:
	    int pos = S.pos();
	    accept(LBRACE);
	    ListBuffer<Tree> buf = new ListBuffer<Tree>();
	    if (S.token() != RBRACE) {
		buf.append(annotationValue());
		while (S.token() == COMMA) {
		    S.nextToken();
		    if (S.token() == RPAREN) break;
		    buf.append(annotationValue());
		}
	    }
	    accept(RBRACE);
	    return F.at(pos).NewArray(null, Tree.emptyList, buf.toList());
	default:
	    mode = EXPR;
	    return term1();
	}
    }

    /** VariableDeclarators = VariableDeclarator { "," VariableDeclarator }
     */
    List<Tree> variableDeclarators(Modifiers mods, Tree type) {
	return variableDeclaratorsRest(S.pos(), mods, type, ident(), false, null);
    }

    /** VariableDeclaratorsRest = VariableDeclaratorRest { "," VariableDeclarator }
     *  ConstantDeclaratorsRest = ConstantDeclaratorRest { "," ConstantDeclarator }
     *
     *  @param reqInit  Is an initializer always required?
     *  @param dc       The documentation comment for the variable declarations, or null.
     */
    List<Tree> variableDeclaratorsRest(int pos, Modifiers mods, Tree type,
				       Name name, boolean reqInit, String dc) {
	ListBuffer<Tree> vdefs = new ListBuffer<Tree>();
	vdefs.append(variableDeclaratorRest(pos, mods, type, name, reqInit, dc));
	while (S.token() == COMMA) {
	    S.nextToken();
	    vdefs.append(variableDeclarator(mods, type, reqInit, dc));
	}
	return vdefs.toList();
    }

    /** VariableDeclarator = Ident VariableDeclaratorRest
     *  ConstantDeclarator = Ident ConstantDeclaratorRest
     */
    VarDef variableDeclarator(Modifiers mods, Tree type, boolean reqInit, String dc) {
	return variableDeclaratorRest(S.pos(), mods, type, ident(), reqInit, dc);
    }

    /** VariableDeclaratorRest = BracketsOpt ["=" VariableInitializer]
     *  ConstantDeclaratorRest = BracketsOpt "=" VariableInitializer
     *
     *  @param reqInit  Is an initializer always required?
     *  @param dc       The documentation comment for the variable declarations, or null.
     */
    VarDef variableDeclaratorRest(int pos, Modifiers mods, Tree type, Name name,
				  boolean reqInit, String dc) {
	type = bracketsOpt(type);
	Tree init = null;
	if (S.token() == EQ) {
	    S.nextToken();
	    init = variableInitializer();
	}
	else if (reqInit) syntaxError(S.pos(), "expected", keywords.token2string(EQ));
	VarDef result =
	    F.at(pos).VarDef(mods, name, type, init);
	if (genEndPos) endPositions.put(result, S.prevEndPos());
	attach(result, dc);
	return result;
    }

    /** VariableDeclaratorId = Ident BracketsOpt
     */
    VarDef variableDeclaratorId(Modifiers mods, Tree type) {
	int pos = S.pos();
	Name name = ident();
	if ((mods.flags & Flags.VARARGS) == 0)
	    type = bracketsOpt(type);
	return F.at(pos).VarDef(mods, name, type, null);
    }

    /** CompilationUnit = [ { "@" Annotation } PACKAGE Qualident ";"] {ImportDeclaration} {TypeDeclaration}
     */
    public Tree.TopLevel compilationUnit() {
        int pos = S.pos();
	Tree pid = null;
	String dc = S.docComment();
	Modifiers mods = null;
	List<Annotation> packageAnnotations = Tree.Annotation.emptyList;
	if (S.token() == MONKEYS_AT)
	    mods = modifiersOpt();

	if (S.token() == PACKAGE) {
	    if (mods != null) {
                checkNoMods(mods.flags);
		packageAnnotations = mods.annotations;
		mods = null;
	    }
            S.nextToken();
            pid = qualident();
            accept(SEMI);
        }
        ListBuffer<Tree> defs = new ListBuffer<Tree>();
        while (mods == null && S.token() == IMPORT)
	    defs.append(importDeclaration());
        while (mods != null || S.token() != EOF) {
	    defs.append(typeDeclaration(mods));
	    mods = null;
	}
	Tree.TopLevel toplevel = F.at(pos).TopLevel(packageAnnotations, pid, defs.toList());
	attach(toplevel, dc);
	if (keepDocComments) toplevel.docComments = docComments;
	if (genEndPos) toplevel.endPositions = endPositions;
	return toplevel;
    }

    /** ImportDeclaration = IMPORT [ STATIC ] Ident { "." Ident } [ "." "*" ] ";"
     */
    Tree importDeclaration() {
        int pos = S.pos();
        S.nextToken();
	boolean importStatic = false;
	if (S.token() == STATIC) {
            checkStaticImports();
	    importStatic = true;
	    S.nextToken();
	}
        Tree pid = F.at(S.pos()).Ident(ident());
	do {
	    accept(DOT);
	    if (S.token() == STAR) {
		pid = F.at(S.pos()).Select(pid, names.asterisk);
		S.nextToken();
		break;
	    } else {
		pid = F.at(S.pos()).Select(pid, ident());
	    }
        } while (S.token() == DOT);
        accept(SEMI);
        return F.at(pos).Import(pid, importStatic);
    }

    /** TypeDeclaration = ClassOrInterfaceOrEnumDeclaration
     *                  | ";"
     */
    Tree typeDeclaration(Modifiers mods) {
        if (S.pos() == S.errPos()) {
	    // error recovery
	    modifiersOpt();
	    while (S.token() != CLASS &&
		   S.token() != INTERFACE &&
		   !(allowEnums && S.token() == ENUM) &&
                   S.token() != EOF) {
		S.nextToken();
		modifiersOpt();
	    }
	}
	int pos = S.pos();
        if (mods == null && S.token() == SEMI) {
            S.nextToken();
	    return F.at(pos).Skip();
        } else {
	    String dc = S.docComment();
	    return classOrInterfaceOrEnumDeclaration(modifiersOpt(mods), dc);
        }
    }
	
    /** ClassOrInterfaceOrEnumDeclaration = ModifiersOpt
     *           (ClassDeclaration | InterfaceDeclaration | EnumDeclaration)
     *  @param mods     Any modifiers starting the class or interface declaration
     *  @param dc       The documentation comment for the class, or null.
     */
    Tree classOrInterfaceOrEnumDeclaration(Modifiers mods, String dc) {
	if (S.token() == CLASS) return classDeclaration(mods, dc);
	else if (S.token() == INTERFACE) return interfaceDeclaration(mods, dc);
	else if (allowEnums && S.token() == ENUM) return enumDeclaration(mods, dc);
	else return syntaxError("class.or.intf.expected");
    }

    /** ClassDeclaration = CLASS Ident TypeParametersOpt [EXTENDS Type]
     *                     [IMPLEMENTS TypeList] ClassBody
     *  @param mods    The modifiers starting the class declaration
     *  @param dc       The documentation comment for the class, or null.
     */
    Tree classDeclaration(Modifiers mods, String dc) {
        int pos = S.pos();
        accept(CLASS);
        Name name = ident();

	List<TypeParameter> typarams = typeParametersOpt();

        Tree extending = null;
        if (S.token() == EXTENDS) {
            S.nextToken();
            extending = type();
        }
        List<Tree> implementing = Tree.emptyList;
        if (S.token() == IMPLEMENTS) {
            S.nextToken();
	    implementing = typeList();
        }
        List<Tree> defs = classOrInterfaceBody(name, false);
        Tree result = F.at(pos).ClassDef(
	    mods, name, typarams, extending, implementing, defs);
	attach(result, dc);
	return result;
    }

    /** InterfaceDeclaration = INTERFACE Ident TypeParametersOpt
     *                         [EXTENDS TypeList] InterfaceBody
     *  @param mods    The modifiers starting the interface declaration
     *  @param dc       The documentation comment for the interface, or null.
     */
    Tree interfaceDeclaration(Modifiers mods, String dc) {
        int pos = S.pos();
        accept(INTERFACE);
        Name name = ident();

	List<TypeParameter> typarams = typeParametersOpt();

        List<Tree> extending = Tree.emptyList;
        if (S.token() == EXTENDS) {
            S.nextToken();
            extending = typeList();
        }
        List<Tree> defs = classOrInterfaceBody(name, true);
        Tree result = F.at(pos).ClassDef(
	    mods, name, typarams, null, extending, defs);
	attach(result, dc);
	return result;
    }

    /** EnumDeclaration = ENUM Ident [IMPLEMENTS TypeList] EnumBody
     *  @param mods    The modifiers starting the enum declaration
     *  @param dc       The documentation comment for the enum, or null.
     */
    Tree enumDeclaration(Modifiers mods, String dc) {
	int pos = S.pos();
	accept(ENUM);
	Name name = ident();

        List<Tree> implementing = Tree.emptyList;
        if (S.token() == IMPLEMENTS) {
            S.nextToken();
	    implementing = typeList();
        }

        List<Tree> defs = enumBody(name);
        Tree result = F.at(pos).
	    ClassDef(new Modifiers(mods.flags|Flags.ENUM, mods.annotations),
		     name, TypeParameter.emptyList,
		     null, implementing, defs);
	attach(result, dc);
	return result;
    }
	
    /** EnumBody = "{" { EnumeratorDeclarationList } [","]
     *			[ ";" {ClassBodyDeclaration} ] "}"
     */
    List<Tree> enumBody(Name enumName) {
	int pos = S.pos();
	accept(LBRACE);
	ListBuffer<Tree> defs = new ListBuffer<Tree>();
	if (S.token() == COMMA) {
	    S.nextToken();
	} else if (S.token() != RBRACE && S.token() != SEMI) {
	    defs.append(enumeratorDeclaration(enumName));
	    while (S.token() == COMMA) {
		S.nextToken();
		if (S.token() == RBRACE || S.token() == SEMI) break;
		defs.append(enumeratorDeclaration(enumName));
	    }
	}
	if (S.token() == SEMI) {
	    S.nextToken();
	    while (S.token() != RBRACE && S.token() != EOF) {
		defs.appendList(classOrInterfaceBodyDeclaration(enumName,
								false));
	    }
	}
	accept(RBRACE);
	return defs.toList();
    }

    /** EnumeratorDeclaration = AnnotationsOpt [TypeArguments] IDENTIFIER [ Arguments ] [ "{" ClassBody "}" ]
     */
    Tree enumeratorDeclaration(Name enumName) {
	int pos = S.pos();
	String dc = S.docComment();
	List<Annotation> annotations = annotationsOpt();
	List<Tree> typeArgs = typeArgumentsOpt();
	Name name = ident();
	List<Tree> args = (S.token() == LPAREN)
	    ? arguments() : Tree.emptyList;
	ClassDef body = (S.token() != LBRACE) ? null :
	     F.at(S.pos()).ClassDef(
		 F.Modifiers(Flags.ENUM | Flags.STATIC),
		 names.empty, TypeParameter.emptyList,
		 null, Tree.emptyList,
		 classOrInterfaceBody(names.empty, false));
	Tree create = F.at(pos).NewClass(null, typeArgs, F.Ident(enumName), args, body);
	Tree result = F.at(pos).
	    VarDef(F.Modifiers(Flags.PUBLIC|Flags.STATIC|Flags.FINAL|Flags.ENUM,
			       annotations),
		   name,
		   F.Ident(enumName),
		   create);
	attach(result, dc);
	return result;
    }

    /** TypeList = Type {"," Type}
     */
    List<Tree> typeList() {
	ListBuffer<Tree> ts = new ListBuffer<Tree>();
	ts.append(type());
	while (S.token() == COMMA) {
	    S.nextToken();
	    ts.append(type());
	}
	return ts.toList();
    }

    /** ClassBody     = "{" {ClassBodyDeclaration} "}"
     *  InterfaceBody = "{" {InterfaceBodyDeclaration} "}"
     */
    List<Tree> classOrInterfaceBody(Name className, boolean isInterface) {
	int pos = S.pos();
	accept(LBRACE);
	ListBuffer<Tree> defs = new ListBuffer<Tree>();
	while (S.token() != RBRACE && S.token() != EOF) {
	    defs.appendList(classOrInterfaceBodyDeclaration(className, isInterface));
	}
	accept(RBRACE);
	return defs.toList();
    }

    /** ClassBodyDeclaration =
     *      ";"
     *    | [STATIC] Block
     *    | ModifiersOpt
     *      ( Type Ident
     *        ( VariableDeclaratorsRest ";" | MethodDeclaratorRest )
     *      | VOID Ident MethodDeclaratorRest
     *      | TypeParameters (Type | VOID) Ident MethodDeclaratorRest
     *      | Ident ConstructorDeclaratorRest
     *      | TypeParameters Ident ConstructorDeclaratorRest
     *      | ClassOrInterfaceOrEnumDeclaration
     *      )
     *  InterfaceBodyDeclaration =
     *      ";"
     *    | ModifiersOpt Type Ident
     *      ( ConstantDeclaratorsRest | InterfaceMethodDeclaratorRest ";" )
     */
    List<Tree> classOrInterfaceBodyDeclaration(Name className, boolean isInterface) {
	int pos = S.pos();
	if (S.token() == SEMI) {
	    S.nextToken();
	    return Tree.emptyList.prepend(F.at(pos).Block(0, Tree.emptyList));
	} else {
	    String dc = S.docComment();
	    Modifiers mods = modifiersOpt();
	    if (S.token() == CLASS ||
		S.token() == INTERFACE ||
		allowEnums && S.token() == ENUM) {
		return Tree.emptyList.
		    prepend(classOrInterfaceOrEnumDeclaration(mods, dc));
	    } else if (S.token() == LBRACE && !isInterface &&
		       (mods.flags & Flags.StandardFlags & ~Flags.STATIC) == 0 &&
		       mods.annotations.isEmpty()) {
		return Tree.emptyList.prepend(block(mods.flags));
	    } else {
	        List<TypeParameter> typarams = typeParametersOpt();
		Tokens token = S.token();
		Name name = S.name();
		pos = S.pos();
		Tree type;
		boolean isVoid = S.token() == VOID;
		if (isVoid) {
		    type = F.at(pos).TypeIdent(TypeTags.VOID);
		    S.nextToken();
		} else {
		    type = type();
		}
		if (S.token() == LPAREN && !isInterface && type.tag == Tree.IDENT) {
		    if (isInterface || name != className)
			log.error(pos, "invalid.meth.decl.ret.type.req");
		    return Tree.emptyList.prepend(methodDeclaratorRest(
			pos, mods, null, names.init, typarams,
			isInterface, true, dc));
		} else {
		    pos = S.pos();
		    name = ident();
		    if (S.token() == LPAREN) {
			return Tree.emptyList.prepend(methodDeclaratorRest(
			    pos, mods, type, name, typarams,
			    isInterface, isVoid, dc));
		    } else if (!isVoid && typarams.isEmpty()) {
			List<Tree> defs = variableDeclaratorsRest(
			    pos, mods, type, name, isInterface, dc);
			accept(SEMI);
			return defs;
		    } else {
			syntaxError(S.pos(), "expected", keywords.token2string(LPAREN));
			return Tree.emptyList;
		    }
		}
	    }
	}
    }

    /** MethodDeclaratorRest =
     *      FormalParameters BracketsOpt [Throws TypeList] ( MethodBody | [DEFAULT AnnotationValue] ";")
     *  VoidMethodDeclaratorRest =
     *      FormalParameters [Throws TypeList] ( MethodBody | ";")
     *  InterfaceMethodDeclaratorRest =
     *      FormalParameters BracketsOpt [THROWS TypeList] ";"
     *  VoidInterfaceMethodDeclaratorRest =
     *      FormalParameters [THROWS TypeList] ";"
     *  ConstructorDeclaratorRest =
     *      "(" FormalParameterListOpt ")" [THROWS TypeList] MethodBody
     */
    Tree methodDeclaratorRest(int pos,
			      Modifiers mods,
			      Tree type,
			      Name name,
			      List<TypeParameter> typarams,
			      boolean isInterface, boolean isVoid,
			      String dc) {
	List<VarDef> params = formalParameters();
	if (!isVoid) type = bracketsOpt(type);
	List<Tree> thrown = Tree.emptyList;
	if (S.token() == THROWS) {
	    S.nextToken();
	    thrown = qualidentList();
	}
	Block body;
	Tree defaultValue;
	if (S.token() == LBRACE) {
	    body = block();
	    defaultValue = null;
	} else {
	    if (S.token() == DEFAULT) {
		accept(DEFAULT);
		defaultValue = annotationValue();
	    } else {
		defaultValue = null;
	    }
	    accept(SEMI);
	    body = null;
	}
	Tree result =
   	    F.at(pos).MethodDef(mods, name, type, typarams,
				params, thrown,
				body, defaultValue);
	attach(result, dc);
	return result;
    }

    /** QualidentList = Qualident {"," Qualident}
     */
    List<Tree> qualidentList() {
	ListBuffer<Tree> ts = new ListBuffer<Tree>();
	ts.append(qualident());
	while (S.token() == COMMA) {
	    S.nextToken();
	    ts.append(qualident());
	}
	return ts.toList();
    }

    /** TypeParametersOpt = ["<" TypeParameter {"," TypeParameter} ">"]
     */
    List<TypeParameter> typeParametersOpt() {
	if (S.token() == LT) {
            checkGenerics();
	    ListBuffer<TypeParameter> typarams = new ListBuffer<TypeParameter>();
	    S.nextToken();
	    typarams.append(typeParameter());
	    while (S.token() == COMMA) {
		S.nextToken();
		typarams.append(typeParameter());
	    }
	    accept(GT);
	    return typarams.toList();
	} else {
	    return TypeParameter.emptyList;
	}
    }

    /** TypeParameter = TypeVariable [TypeParameterBound]
     *  TypeParameterBound = EXTENDS Type {"&" Type}
     *  TypeVariable = Ident
     */
    TypeParameter typeParameter() {
	int pos = S.pos();
	Name name = ident();
	ListBuffer<Tree> bounds = new ListBuffer<Tree>();
	if (S.token() == EXTENDS) {
	    S.nextToken();
	    bounds.append(type());
	    while (S.token() == AMP) {
		S.nextToken();
		bounds.append(type());
	    }
	}
	return F.at(pos).TypeParameter(name, bounds.toList());
    }

    /** FormalParameters = "(" [ FormalParameterList ] ")"
     *  FormalParameterList = [ FormalParameterListNovarargs , ] LastFormalParameter
     *  FormalParameterListNovarargs = [ FormalParameterListNovarargs , ] FormalParameter
     */
    List<VarDef> formalParameters() {
	ListBuffer<VarDef> params = new ListBuffer<VarDef>();
	VarDef lastParam = null;
	accept(LPAREN);
	if (S.token() != RPAREN) {
	    params.append(lastParam = formalParameter());
	    while ((lastParam.mods.flags & Flags.VARARGS) == 0 && S.token() == COMMA) {
		S.nextToken();
		params.append(lastParam = formalParameter());
	    }
	}
	accept(RPAREN);
	return params.toList();
    }

    Modifiers optFinal(long flags) {
	Modifiers mods = modifiersOpt();
        checkNoMods(mods.flags & ~(Flags.FINAL | Flags.DEPRECATED));
	mods.flags |= flags;
	return mods;
    }

    /** FormalParameter = { FINAL | '@' Annotation } Type VariableDeclaratorId
     *  LastFormalParameter = { FINAL | '@' Annotation } Type '...' Ident | FormalParameter
     */
    VarDef formalParameter() {
	Modifiers mods = optFinal(Flags.PARAMETER);
	Tree type = type();
	if (S.token() == ELLIPSIS) {
            checkVarargs();
	    mods.flags |= Flags.VARARGS;
	    type = F.at(S.pos()).TypeArray(type);
	    S.nextToken();
	}
	return variableDeclaratorId(mods, type);
    }

/* ---------- auxiliary methods -------------- */

    /** Share the terminator when making lists of trees.
     *  This is am optimized implementation of List.make(a, b). */
    private List<Tree> makeList(Tree a, Tree b) {
	return new List<Tree>(a,
			      new List<Tree>(b, Tree.emptyList));
    }

    /** Share the terminator when making lists of trees.
     *  This is am optimized implementation of List.make(a). */
    private List<Tree> makeList(Tree a) {
	return new List<Tree>(a, Tree.emptyList);
    }

    /** Check that given tree is a legal expression statement.
     */
    Tree checkExprStat(Tree t) {
        switch(t.tag) {
	case Tree.PREINC: case Tree.PREDEC:
	case Tree.POSTINC: case Tree.POSTDEC:
	case Tree.ASSIGN:
	case Tree.BITOR_ASG: case Tree.BITXOR_ASG: case Tree.BITAND_ASG:
	case Tree.SL_ASG: case Tree.SR_ASG: case Tree.USR_ASG:
	case Tree.PLUS_ASG: case Tree.MINUS_ASG:
	case Tree.MUL_ASG: case Tree.DIV_ASG: case Tree.MOD_ASG:
	case Tree.APPLY: case Tree.NEWCLASS:
	case Tree.ERRONEOUS:
	    return t;
	default:
	    log.error(t.pos, "not.stmt");
	    return errorTree;
	}
    }

    /** Return precedence of operator represented by token,
     *  -1 if token is not a binary operator. @see TreeInfo.opPrec
     */
    static int prec(Tokens token) {
	int oc = optag(token);
	return (oc >= 0) ? TreeInfo.opPrec(oc) : -1;
    }

    /** Return operation tag of binary operator represented by token,
     *  -1 if token is not a binary operator.
     */
    static int optag(Tokens token) {
	switch (token) {
        case BARBAR:
	    return Tree.OR;
        case AMPAMP:
	    return Tree.AND;
        case BAR:
	    return Tree.BITOR;
	case BAREQ:
	    return Tree.BITOR_ASG;
        case CARET:
	    return Tree.BITXOR;
	case CARETEQ:
	    return Tree.BITXOR_ASG;
        case AMP:
	    return Tree.BITAND;
	case AMPEQ:
	    return Tree.BITAND_ASG;
        case EQEQ:
	    return Tree.EQ;
        case BANGEQ:
	    return Tree.NE;
        case LT:
	    return Tree.LT;
        case GT:
	    return Tree.GT;
        case LTEQ:
	    return Tree.LE;
        case GTEQ:
	    return Tree.GE;
        case LTLT:
	    return Tree.SL;
	case LTLTEQ:
	    return Tree.SL_ASG;
        case GTGT:
	    return Tree.SR;
	case GTGTEQ:
	    return Tree.SR_ASG;
        case GTGTGT:
	    return Tree.USR;
	case GTGTGTEQ:
	    return Tree.USR_ASG;
        case PLUS:
	    return Tree.PLUS;
	case PLUSEQ:
	    return Tree.PLUS_ASG;
        case SUB:
	    return Tree.MINUS;
	case SUBEQ:
	    return Tree.MINUS_ASG;
        case STAR:
	    return Tree.MUL;
	case STAREQ:
	    return Tree.MUL_ASG;
        case SLASH:
	    return Tree.DIV;
	case SLASHEQ:
	    return Tree.DIV_ASG;
        case PERCENT:
	    return Tree.MOD;
	case PERCENTEQ:
	    return Tree.MOD_ASG;
	case INSTANCEOF:
	    return Tree.TYPETEST;
	default:
	    return -1;
	}
    }

    /** Return operation tag of unary operator represented by token,
     *  -1 if token is not a binary operator.
     */
    static int unoptag(Tokens token) {
	switch (token) {
        case PLUS:
	    return Tree.POS;
        case SUB:
	    return Tree.NEG;
	case BANG:
	    return Tree.NOT;
	case TILDE:
	    return Tree.COMPL;
	case PLUSPLUS:
	    return Tree.PREINC;
	case SUBSUB:
	    return Tree.PREDEC;
	default:
	    return -1;
	}
    }

    /** Return type tag of basic type represented by token,
     *  -1 if token is not a basic type identifier.
     */
    static int typetag(Tokens token) {
	switch (token) {
	case BYTE:
	    return TypeTags.BYTE;
	case CHAR:
	    return TypeTags.CHAR;
	case SHORT:
	    return TypeTags.SHORT;
	case INT:
	    return TypeTags.INT;
	case LONG:
	    return TypeTags.LONG;
	case FLOAT:
	    return TypeTags.FLOAT;
	case DOUBLE:
	    return TypeTags.DOUBLE;
	case BOOLEAN:
	    return TypeTags.BOOLEAN;
	default:
	    return -1;
	}
    }

    void checkGenerics() {
        if (!allowGenerics) {
            log.error(S.pos(), "generics.not.supported.in.source", source.name);
            allowGenerics = true;
        }
    }
    void checkVarargs() {
        if (!allowVarargs) {
            log.error(S.pos(), "varargs.not.supported.in.source", source.name);
            allowVarargs = true;
        }
    }
    void checkForeach() {
        if (!allowForeach) {
            log.error(S.pos(), "foreach.not.supported.in.source", source.name);
            allowForeach = true;
        }
    }
    void checkStaticImports() {
        if (!allowStaticImport) {
            log.error(S.pos(), "static.import.not.supported.in.source", source.name);
            allowStaticImport = true;
        }
    }
    void checkAnnotations() {
        if (!allowAnnotations) {
            log.error(S.pos(), "annotations.not.supported.in.source", source.name);
            allowAnnotations = true;
        }
    }
}	
