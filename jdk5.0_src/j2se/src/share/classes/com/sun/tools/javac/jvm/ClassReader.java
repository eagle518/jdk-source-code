/**
 * @(#)ClassReader.java	1.99 04/06/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.jvm;

import java.io.*;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.zip.ZipFile;
import java.util.zip.ZipEntry;

import com.sun.tools.javac.comp.Annotate;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Type.*;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.code.Symtab;
import com.sun.tools.javac.util.*;
import com.sun.tools.javac.util.List;

import static com.sun.tools.javac.code.Flags.*;
import static com.sun.tools.javac.code.Kinds.*;
import static com.sun.tools.javac.code.TypeTags.*;

/** This class provides operations to read a classfile into an internal
 *  representation. The internal representation is anchored in a
 *  ClassSymbol which contains in its scope symbol representations
 *  for all other definitions in the classfile. Top-level Classes themselves
 *  appear as members of the scopes of PackageSymbols.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class ClassReader extends ClassFile implements Completer {
    /** The context key for the class reader. */
    protected static final Context.Key<ClassReader> classReaderKey =
	new Context.Key<ClassReader>();

    Annotate annotate;

    /** Switch: verbose output.
     */
    boolean verbose;

    /** Switch: check class file for correct minor version, unrecognized
     *  attributes.
     */
    boolean checkClassFile;

    /** Switch: turn off variance.
     */
    boolean allowVariance;

    /** Switch: read constant pool and code sections. This switch is initially
     *  set to false but can be turned on from outside.
     */
    public boolean readAllOfClassFile = false;

    /** Switch: read GJ signature information.
     */
    boolean allowGenerics;

    /** Switch: read varargs attribute.
     */
    boolean allowVarargs;

    /** Switch: allow annotations.
     */
    boolean allowAnnotations;

    /** Switch: preserve parameter names from the variable table.
     */
    boolean saveParameterNames;

    /** The log to use for verbose output
     */
    final Log log;

    /** The symbol table. */
    Symtab syms;

    Types types;

    /** The name table. */
    final Name.Table names;

    /** Force a completion failure on this name
     */
    final Name completionFailureName;

    /** Encapsulates knowledge of paths
     */
    private final Paths paths;

    /** Can be reassigned from outside:
     *  the completer to be used for ".java" files. If this remains unassigned
     *  ".java" files will not be loaded.
     */
    public SourceCompleter sourceCompleter = null;

    /** A hashtable containing the encountered top-level and member classes,
     *  indexed by flat names. The table does not contain local classes.
     */
    private Map<Name,ClassSymbol> classes;

    /** A hashtable containing the encountered packages.
     */
    private Map<Name, PackageSymbol> packages;

    /** The current scope where type variables are entered.
     */
    Scope typevars;

    /** The path name of the class file currently being read.
     */
    String currentClassFileName = null;

    /** The class or method currently being read.
     */
    Symbol currentOwner = null;

    /** The buffer containing the currently read class file.
     */
    byte[] buf = new byte[0x0fff0];

    /** The current input pointer.
     */
    int bp;

    /** The objects of the constant pool.
     */
    Object[] poolObj;

    /** For every constant pool entry, an index into buf where the
     *  defining section of the entry is found.
     */
    int[] poolIdx;

    /** Get the ClassReader instance for this invocation. */
    public static ClassReader instance(Context context) {
	ClassReader instance = context.get(classReaderKey);
	if (instance == null)
	    instance = new ClassReader(context, true);
	return instance;
    }

    /** Initialize classes and packages, treating this as the definitive classreader. */
    public void init(Symtab syms) {
	init(syms, true);
    }

    /** Initialize classes and packages, optionally treating this as
     *  the definitive classreader.
     */
    private void init(Symtab syms, boolean definitive) {
	if (classes != null) return;

	if (definitive) {
	    assert packages == null || packages == syms.packages;
	    packages = syms.packages;
	    assert classes == null || classes == syms.classes;
	    classes = syms.classes;
	} else {
	    packages = new HashMap<Name, PackageSymbol>();
	    classes = new HashMap<Name, ClassSymbol>();
	}

	packages.put(syms.rootPackage.fullname, syms.rootPackage);
	syms.rootPackage.completer = this;
	packages.put(syms.emptyPackage.fullname, syms.emptyPackage);
	syms.emptyPackage.completer = this;
    }

    /** Construct a new class reader, optionally treated as the
     *  definitive classreader for this invocation.
     */
    protected ClassReader(Context context, boolean definitive) {
	if (definitive) context.put(classReaderKey, this);

	names = Name.Table.instance(context);
	syms = Symtab.instance(context);
	types = Types.instance(context);
	init(syms, definitive);
	log = Log.instance(context);

	Options options = Options.instance(context);
	annotate = Annotate.instance(context);
	verbose        = options.get("-verbose")        != null;
	checkClassFile = options.get("-checkclassfile") != null;
	Source source = Source.instance(context);
	allowGenerics    = source.allowGenerics();
	allowVarargs     = source.allowVarargs();
	allowAnnotations = source.allowAnnotations();
        saveParameterNames = options.get("save-parameter-names") != null;

	paths = Paths.instance(context);

	completionFailureName =
	    (options.get("failcomplete") != null)
	    ? names.fromString(options.get("failcomplete"))
	    : null;

	typevars = new Scope(syms.noSymbol);
    }

    /** Add member to class unless it is synthetic.
     */
    private void enterMember(ClassSymbol c, Symbol sym) {
	if ((sym.flags_field & (SYNTHETIC|BRIDGE)) != SYNTHETIC)
	    c.members_field.enter(sym);
    }

/************************************************************************
 * Error Diagnoses
 ***********************************************************************/

    public static class BadClassFile extends CompletionFailure {
	private static final long serialVersionUID = 0;

	/**
	 * @param msg A localized message.
	 */
	public BadClassFile(ClassSymbol c, Object cname, Object msg) {
	    super(c, Log.getLocalizedString("bad.class.file.header",
					    cname, msg));
	}
    }

    public BadClassFile badClassFile(String key, Object... args) {
	return new BadClassFile (
	    currentOwner.enclClass(),
	    currentClassFileName,
	    Log.getLocalizedString(key, args));
    }

/************************************************************************
 * Buffer Access
 ***********************************************************************/

    /** Read a character.
     */
    char nextChar() {
        return (char)(((buf[bp++] & 0xFF) << 8) + (buf[bp++] & 0xFF));
    }

    /** Read an integer.
     */
    int nextInt() {
        return
            ((buf[bp++] & 0xFF) << 24) +
            ((buf[bp++] & 0xFF) << 16) +
            ((buf[bp++] & 0xFF) << 8) +
            (buf[bp++] & 0xFF);
    }

    /** Extract a character at position bp from buf.
     */
    char getChar(int bp) {
        return
            (char)(((buf[bp] & 0xFF) << 8) + (buf[bp+1] & 0xFF));
    }

    /** Extract an integer at position bp from buf.
     */
    int getInt(int bp) {
        return
            ((buf[bp] & 0xFF) << 24) +
	    ((buf[bp+1] & 0xFF) << 16) +
            ((buf[bp+2] & 0xFF) << 8) +
            (buf[bp+3] & 0xFF);
    }


    /** Extract a long integer at position bp from buf.
     */
    long getLong(int bp) {
        DataInputStream bufin =
            new DataInputStream(new ByteArrayInputStream(buf, bp, 8));
        try {
            return bufin.readLong();
        } catch (IOException e) {
            throw new AssertionError();
        }
    }

    /** Extract a float at position bp from buf.
     */
    float getFloat(int bp) {
        DataInputStream bufin =
            new DataInputStream(new ByteArrayInputStream(buf, bp, 4));
        try {
            return bufin.readFloat();
        } catch (IOException e) {
            throw new AssertionError();
        }
    }

    /** Extract a double at position bp from buf.
     */
    double getDouble(int bp) {
        DataInputStream bufin =
            new DataInputStream(new ByteArrayInputStream(buf, bp, 8));
        try {
            return bufin.readDouble();
        } catch (IOException e) {
            throw new AssertionError();
        }
    }

/************************************************************************
 * Constant Pool Access
 ***********************************************************************/

    /** Index all constant pool entries, writing their start addresses into
     *  poolIdx.
     */
    void indexPool() {
        poolIdx = new int[nextChar()];
        poolObj = new Object[poolIdx.length];
        int i = 1;
        while (i < poolIdx.length) {
            poolIdx[i++] = bp;
            byte tag = buf[bp++];
            switch (tag) {
            case CONSTANT_Utf8: case CONSTANT_Unicode: {
                int len = nextChar();
                bp = bp + len;
                break;
            }
            case CONSTANT_Class:
            case CONSTANT_String:
                bp = bp + 2;
                break;
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:
            case CONSTANT_NameandType:
            case CONSTANT_Integer:
            case CONSTANT_Float:
                bp = bp + 4;
                break;
            case CONSTANT_Long:
            case CONSTANT_Double:
                bp = bp + 8;
                i++;
                break;
            default:
                throw badClassFile("bad.const.pool.tag.at",
				   Byte.toString(tag),
				   Integer.toString(bp -1));
            }
        }
    }

    /** Read constant pool entry at start address i, use pool as a cache.
     */
    Object readPool(int i) {
	Object result = poolObj[i];
	if (result != null) return result;

        int index = poolIdx[i];
        if (index == 0) return null;

        byte tag = buf[index];
        switch (tag) {
        case CONSTANT_Utf8:
            poolObj[i] = names.fromUtf(buf, index + 3, getChar(index + 1));
            break;
        case CONSTANT_Unicode:
	    throw badClassFile("unicode.str.not.supported");
        case CONSTANT_Class:
            poolObj[i] = readClassOrType(getChar(index + 1));
            break;
        case CONSTANT_String:
            poolObj[i] = readName(getChar(index + 1)).toString();
            break;
        case CONSTANT_Fieldref: {
            ClassSymbol owner =	readClassSymbol(getChar(index + 1));
            NameAndType nt = (NameAndType)readPool(getChar(index + 3));
            poolObj[i] = new VarSymbol(0, nt.name, nt.type, owner);
            break;
        }
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref: {
            ClassSymbol owner =	readClassSymbol(getChar(index + 1));
            NameAndType nt = (NameAndType)readPool(getChar(index + 3));
            poolObj[i] = new MethodSymbol(0, nt.name, nt.type, owner);
            break;
        }
        case CONSTANT_NameandType:
            poolObj[i] = new NameAndType(
                readName(getChar(index + 1)),
		readType(getChar(index + 3)));
            break;
        case CONSTANT_Integer:
            poolObj[i] = getInt(index + 1);
            break;
        case CONSTANT_Float:
            poolObj[i] = new Float(getFloat(index + 1));
            break;
        case CONSTANT_Long:
            poolObj[i] = new Long(getLong(index + 1));
            break;
        case CONSTANT_Double:
            poolObj[i] = new Double(getDouble(index + 1));
            break;
        default:
	    throw badClassFile("bad.const.pool.tag", Byte.toString(tag));
        }
        return poolObj[i];
    }

    /** Read signature and convert to type.
     */
    Type readType(int i) {
        int index = poolIdx[i];
	return sigToType(buf, index + 3, getChar(index + 1));
    }

    /** If name is an array type or class signature, return the
     *  corresponding type; otherwise return a ClassSymbol with given name.
     */
    Object readClassOrType(int i) {
	int index =  poolIdx[i];
	int len = getChar(index + 1);
	int start = index + 3;
        assert buf[start] == '[' || buf[start + len - 1] != ';';
        // by the above assertion, the following test can be
        // simplified to (buf[start] == '[')
	return (buf[start] == '[' || buf[start + len - 1] == ';')
	    ? (Object)sigToType(buf, start, len)
	    : (Object)enterClass(names.fromUtf(internalize(buf, start,
							   len)));
    }

    /** Read signature and convert to type parameters.
     */
    List<Type> readTypeParams(int i) {
        int index = poolIdx[i];
	return sigToTypeParams(buf, index + 3, getChar(index + 1));
    }

    /** Read class entry.
     */
    ClassSymbol readClassSymbol(int i) {
	return (ClassSymbol) (readPool(i));
    }

    /** Read name.
     */
    Name readName(int i) {
	return (Name) (readPool(i));
    }

/************************************************************************
 * Reading Types
 ***********************************************************************/

    /** The unread portion of the currently read type is
     *  signature[sigp..siglimit-1].
     */
    byte[] signature;
    int sigp;
    int siglimit;
    boolean sigEnterPhase = false;

    /** Convert signature to type, where signature is a name.
     */
    Type sigToType(Name sig) {
	return sig == null
	    ? null
	    : sigToType(sig.table.names, sig.index, sig.len);
    }

    /** Convert signature to type, where signature is a byte array segment.
     */
    Type sigToType(byte[] sig, int offset, int len) {
	signature = sig;
	sigp = offset;
	siglimit = offset + len;
	return sigToType();
    }

    /** Convert signature to type, where signature is implicit.
     */
    Type sigToType() {
        switch ((char) signature[sigp]) {
	case 'T':
	    sigp++;
	    int start = sigp;
            while (signature[sigp] != ';') sigp++;
	    sigp++;
	    return sigEnterPhase
		? Type.noType
		: findTypeVar(names.fromUtf(signature, start, sigp - 1 - start));
	case '+': {
	    sigp++;
	    Type t = sigToType();
	    return new ArgumentType(t, BoundKind.EXTENDS,
				    syms.boundClass);
	}
	case '*':
	    sigp++;
	    return new ArgumentType(syms.objectType, BoundKind.UNBOUND,
				    syms.boundClass);
	case '-': {
	    sigp++;
	    Type t = sigToType();
	    return new ArgumentType(t, BoundKind.SUPER,
				    syms.boundClass);
	}
        case 'B':
	    sigp++;
            return syms.byteType;
        case 'C':
	    sigp++;
            return syms.charType;
        case 'D':
	    sigp++;
            return syms.doubleType;
        case 'F':
	    sigp++;
            return syms.floatType;
        case 'I':
	    sigp++;
            return syms.intType;
        case 'J':
	    sigp++;
            return syms.longType;
        case 'L':
            {
                int oldsigp = sigp;
                Type t = classSigToType();
                if (sigp < siglimit && signature[sigp] == '.')
                    throw badClassFile("deprecated inner class signature syntax " +
                                       "(please recompile from source)");
                /*
                System.err.println(" decoded " +
                                   new String(signature, oldsigp, sigp-oldsigp) +
                                   " => " + t + " outer " + t.outer());
                */
                return t;
            }
        case 'S':
	    sigp++;
            return syms.shortType;
        case 'V':
	    sigp++;
            return syms.voidType;
        case 'Z':
	    sigp++;
            return syms.booleanType;
        case '[':
	    sigp++;
            return new ArrayType(sigToType(), syms.arrayClass);
        case '(':
            sigp++;
	    List<Type> argtypes = sigToTypes(')');
            Type restype = sigToType();
	    ListBuffer<Type> thrown = new ListBuffer<Type>();
	    while (signature[sigp] == '^') {
		sigp++;
		thrown.append(sigToType());
	    }
            return new MethodType(argtypes,
				  restype,
				  thrown.toList(),
				  syms.methodClass);
        case '<':
	    typevars = typevars.dup(currentOwner);
	    Type poly = new ForAll(sigToTypeParams(), sigToType());
	    typevars = typevars.leave();
	    return poly;
        default:
            throw badClassFile("bad.signature",
			       Convert.utf2string(signature, sigp, 10));
	}
    }

    /** Convert class signature to type, where signature is implicit.
     */
    Type classSigToType() {
        if (signature[sigp] != 'L')
            throw badClassFile("bad.class.signature",
                               Convert.utf2string(signature, sigp, 10));
        sigp++;
        Type outer = Type.noType;
        ByteBuffer buf = new ByteBuffer();

        while (true) {
            final byte c = signature[sigp++];
            switch (c) {

            case '.':           // inner class separator
            case ';': {         // end
                ClassSymbol t = enterClass(names.fromUtf(buf.elems, 0, buf.length));
                if (outer == Type.noType)
                    outer = t.erasure(types);
                else
                    outer = new ClassType(outer, Type.emptyList, t);
                if (c == ';') return outer;
                buf.appendByte((byte)'$');
                continue;
            }

            case '<':           // generic arguments
                ClassSymbol t = enterClass(names.fromUtf(buf.elems, 0, buf.length));
                outer = new ClassType(outer, sigToTypes('>'), t);
                switch (signature[sigp++]) {
                case ';': 
                    if (sigp < signature.length && signature[sigp] == '.') {
                        // support old-style GJC signatures
                        // The signature produced was
                        // Lfoo/Outer<Lfoo/X;>;.Lfoo/Outer$Inner<Lfoo/Y;>;
                        // rather than say
                        // Lfoo/Outer<Lfoo/X;>.Inner<Lfoo/Y;>;
                        // so we skip past ".Lfoo/Outer$"
                        sigp += buf.length + // "foo/Outer"
                            3;  // ".L" and "$"
                        buf.appendByte((byte)'$');
                        break;
                    } else {
                        return outer;
                    }
                case '.': buf.appendByte((byte)'$'); break;
                default: throw new AssertionError(signature[sigp-1]);
                }
                continue;

            case '/':
                buf.appendByte((byte)'.');
                continue;
            default:
                buf.appendByte(c);
                continue;
            }
        }
    }

    /** Convert (implicit) signature to list of types
     *  until `terminator' is encountered.
     */
    List<Type> sigToTypes(char terminator) {
	ListBuffer<Type> types = new ListBuffer<Type>();
	while (signature[sigp] != terminator) types.append(sigToType());
	sigp++;
	return types.toList();
    }

    /** Convert signature to type parameters, where signature is a name.
     */
    List<Type> sigToTypeParams(Name name) {
	return sigToTypeParams(name.table.names, name.index, name.len);
    }

    /** Convert signature to type parameters, where signature is a byte
     *  array segment.
     */
    List<Type> sigToTypeParams(byte[] sig, int offset, int len) {
	signature = sig;
	sigp = offset;
	siglimit = offset + len;
	return sigToTypeParams();
    }

    /** Convert signature to type parameters, where signature is implicit.
     */
    List<Type> sigToTypeParams() {
	ListBuffer<Type> tvars = new ListBuffer<Type>();
	if (signature[sigp] == '<') {
	    sigp++;
	    int start = sigp;
	    sigEnterPhase = true;
	    while (signature[sigp] != '>') tvars.append(sigToTypeParam());
	    sigEnterPhase = false;
	    sigp = start;
	    while (signature[sigp] != '>') sigToTypeParam();
	    sigp++;
	}
	return tvars.toList();
    }

    /** Convert (implicit) signature to type parameter.
     */
    Type sigToTypeParam() {
	int start = sigp;
	while (signature[sigp] != ':') sigp++;
	Name name = names.fromUtf(signature, start, sigp - start);
	TypeVar tvar;
	if (sigEnterPhase) {
	    tvar = new TypeVar(name, currentOwner);
	    typevars.enter(tvar.tsym);
	} else {
	    tvar = (TypeVar)findTypeVar(name);
	}
	ListBuffer<Type> bounds = new ListBuffer<Type>();
	Type st = null;
	if (signature[sigp] == ':' && signature[sigp+1] == ':') {
	    sigp++;
	    st = syms.objectType;
	}
	while (signature[sigp] == ':') {
	    sigp++;
	    bounds.append(sigToType());
	}
	if (!sigEnterPhase) {
	    types.setBounds(tvar, bounds.toList(), st);
	}
	return tvar;
    }

    /** Find type variable with given name in `typevars' scope.
     */
    Type findTypeVar(Name name) {
	Scope.Entry e = typevars.lookup(name);
	if (e.scope != null) return e.sym.type;
	else throw badClassFile("undecl.type.var", name);
    }

/************************************************************************
 * Reading Attributes
 ***********************************************************************/

    /** Report unrecognized attribute.
     */
    void unrecognized(Name attrName) {
        if (checkClassFile)
	    printCCF("ccf.unrecognized.attribute", attrName);
    }

    /** Read member attribute.
     */
    void readMemberAttr(Symbol sym, Name attrName, int attrLen) {
	//- System.err.println(" z " + sym + ", " + attrName + ", " + attrLen);
	if (attrName == names.ConstantValue) {
	    Object v = readPool(nextChar());
 	    // Ignore ConstantValue attribute if field not final.
 	    if ((sym.flags() & FINAL) != 0)
 		((VarSymbol)sym).constValue = v;
	} else if (attrName == names.Code) {
	    if (readAllOfClassFile) ((MethodSymbol)sym).code = readCode(sym);
	    else bp = bp + attrLen;
	} else if (attrName == names.Exceptions) {
	    int nexceptions = nextChar();
	    ListBuffer<Type> thrown = new ListBuffer<Type>();
	    for (int j = 0; j < nexceptions; j++)
		thrown.append(readClassSymbol(nextChar()).type);
	    if (sym.type.thrown().isEmpty())
		sym.type.asMethodType().thrown = thrown.toList();
	} else if (attrName == names.Synthetic) {
	    // bridge methods are visible when generics not enabled
	    if (allowGenerics || (sym.flags_field & BRIDGE) == 0)
		sym.flags_field |= SYNTHETIC;
	} else if (attrName == names.Bridge) {
	    sym.flags_field |= BRIDGE;
	    if (!allowGenerics)
		sym.flags_field &= ~SYNTHETIC;
	} else if (attrName == names.Deprecated) {
	    sym.flags_field |= DEPRECATED;
	} else if (attrName == names.Varargs) {
	    if (allowVarargs) sym.flags_field |= VARARGS;
	} else if (attrName == names.Annotation) {
	    if (allowAnnotations) sym.flags_field |= ANNOTATION;
	} else if (attrName == names.Enum) {
	    sym.flags_field |= ENUM;
	} else if (allowGenerics && attrName == names.Signature) {
	    List<Type> thrown = sym.type.thrown();
	    sym.type = readType(nextChar());
	    //- System.err.println(" # " + sym.type);
	    if (sym.kind == MTH && sym.type.thrown().isEmpty())
		sym.type.asMethodType().thrown = thrown;
	} else if (attrName == names.RuntimeVisibleAnnotations) {
	    attachAnnotations(sym, attrLen);
	} else if (attrName == names.RuntimeInvisibleAnnotations) {
	    attachAnnotations(sym, attrLen);
        } else if (attrName == names.RuntimeVisibleParameterAnnotations) {
            attachParameterAnnotations(sym, attrLen);
        } else if (attrName == names.RuntimeInvisibleParameterAnnotations) {
            attachParameterAnnotations(sym, attrLen);
        } else if (attrName == names.LocalVariableTable) {
            int newbp = bp + attrLen;
            if (saveParameterNames) {
                // pick up parameter names from the variable table
                List<VarSymbol> params = ((MethodSymbol)sym).params();
                int firstParam = ((sym.flags() & STATIC) == 0) ? 1 : 0;
                int endParam = firstParam + Code.width(sym.type.argtypes());
                int numEntries = nextChar();
                for (int i=0; i<numEntries; i++) {
                    int start_pc = nextChar();
                    int length = nextChar();
                    int nameIndex = nextChar();
                    int sigIndex = nextChar();
                    int register = nextChar();
                    if (start_pc == 0 &&
                        firstParam <= register &&
                        register < endParam) {
                        int index = firstParam;
                        for (VarSymbol s : params) {
                            if (index == register) {
                                s.name = readName(nameIndex);
                                break;
                            }
                            index += Code.width(s.type);
                        }
                    }
                }
            }
            bp = newbp;
	} else if (attrName == names.AnnotationDefault) {
	    attachAnnotationDefault(sym, attrLen);
	} else {
	    unrecognized(attrName);
	    bp = bp + attrLen;
	}
    }

    /** Read member attributes.
     */
    void readMemberAttrs(Symbol sym) {
        char ac = nextChar();
        for (int i = 0; i < ac; i++) {
	    Name attrName = readName(nextChar());
	    int attrLen = nextInt();
	    readMemberAttr(sym, attrName, attrLen);
	}
    }

    /** Read class attribute.
     */
    void readClassAttr(ClassSymbol c, Name attrName, int attrLen) {
	if (attrName == names.SourceFile) {
	    c.sourcefile = readName(nextChar());
	} else if (attrName == names.InnerClasses) {
	    readInnerClasses(c);
	} else if (allowGenerics && attrName == names.Signature) {
	    ClassType ct1 = (ClassType)c.type;
	    ct1.typarams_field = readTypeParams(nextChar());
	    ct1.supertype_field = sigToType();
	    ListBuffer<Type> is = new ListBuffer<Type>();
	    while (sigp != siglimit) is.append(sigToType());
	    ct1.interfaces_field = is.toList();
	} else {
	    readMemberAttr(c, attrName, attrLen);
	}
    }

    /** Read class attributes.
     */
    void readClassAttrs(ClassSymbol c) {
        char ac = nextChar();
        for (int i = 0; i < ac; i++) {
	    Name attrName = readName(nextChar());
	    int attrLen = nextInt();
	    readClassAttr(c, attrName, attrLen);
	}
    }

    /** Read code block.
     */
    Code readCode(Symbol owner) {
	return null;
    }

/************************************************************************
 * Reading Java-language annotations
 ***********************************************************************/

    /** Attach annotations.
     */
    void attachAnnotations(final Symbol sym, int attrLen) {
	int numAttributes = nextChar();
        if (numAttributes != 0) {
            ListBuffer<CompoundAnnotationProxy> proxies =
                new ListBuffer<CompoundAnnotationProxy>();
            for (int i = 0; i<numAttributes; i++)
                proxies.append(readCompoundAnnotation());
            annotate.later(new AnnotationCompleter(sym, proxies.toList()));
        }
    }

    /** Attach parameter annotations.
     */
    void attachParameterAnnotations(final Symbol method, int attrLen) {
        final MethodSymbol meth = (MethodSymbol)method;
        int numParameters = buf[bp++] & 0xFF;
        List<VarSymbol> parameters = meth.params();
        for (int i = 0; i < numParameters; i++) {
            attachAnnotations(parameters.head, -1);
            parameters = parameters.tail;
        }
    }

    /** Attach the default value for an annotation element.
     */
    void attachAnnotationDefault(final Symbol sym, int attrLen) {
	final MethodSymbol meth = (MethodSymbol)sym; // only on methods
	final Attribute value = readAttributeValue();
	annotate.later(new AnnotationDefaultCompleter(meth, value));
    }

    Type readTypeOrClassSymbol(int i) {
        // support preliminary jsr175-format class files
        if (buf[poolIdx[i]] == CONSTANT_Class)
            return readClassSymbol(i).type;
        return readType(i);
    }
    Type readEnumType(int i) {
        // support preliminary jsr175-format class files
        int index = poolIdx[i];
        int length = getChar(index + 1);
        if (buf[index + length + 2] != ';')
            return enterClass(readName(i)).type;
        return readType(i);
    }

    CompoundAnnotationProxy readCompoundAnnotation() {
	Type t = readTypeOrClassSymbol(nextChar());
	int numFields = nextChar();
	ListBuffer<Pair<Name,Attribute>> pairs =
	    new ListBuffer<Pair<Name,Attribute>>();
	for (int i=0; i<numFields; i++) {
	    Name name = readName(nextChar());
	    Attribute value = readAttributeValue();
	    pairs.append(new Pair<Name,Attribute>(name, value));
	}
	return new CompoundAnnotationProxy(t, pairs.toList());
    }

    Attribute readAttributeValue() {
	char c = (char) buf[bp++];
	switch (c) {
	case 'B':
	    return new Attribute.Constant(syms.byteType, readPool(nextChar()));
	case 'C':
	    return new Attribute.Constant(syms.charType, readPool(nextChar()));
	case 'D':
	    return new Attribute.Constant(syms.doubleType, readPool(nextChar()));
	case 'F':
	    return new Attribute.Constant(syms.floatType, readPool(nextChar()));
	case 'I':
	    return new Attribute.Constant(syms.intType, readPool(nextChar()));
	case 'J':
	    return new Attribute.Constant(syms.longType, readPool(nextChar()));
	case 'S':
	    return new Attribute.Constant(syms.shortType, readPool(nextChar()));
	case 'Z':
	    return new Attribute.Constant(syms.booleanType, readPool(nextChar()));
	case 's':
	    return new Attribute.Constant(syms.stringType, readPool(nextChar()).toString());
	case 'e':
	    return new EnumAttributeProxy(readEnumType(nextChar()), readName(nextChar()));
	case 'c':
	    return new Attribute.Class(types, readTypeOrClassSymbol(nextChar()));
	case '[': {
	    int n = nextChar();
	    ListBuffer<Attribute> l = new ListBuffer<Attribute>();
	    for (int i=0; i<n; i++)
		l.append(readAttributeValue());
	    return new ArrayAttributeProxy(l.toList());
	}
	case '@':
	    return readCompoundAnnotation();
	default:
	    throw new AssertionError("unknown annotation tag '" + c + "'");
	}
    }

    interface ProxyVisitor extends Attribute.Visitor {
	void visitEnumAttributeProxy(EnumAttributeProxy proxy);
	void visitArrayAttributeProxy(ArrayAttributeProxy proxy);
	void visitCompoundAnnotationProxy(CompoundAnnotationProxy proxy);
    }

    static class EnumAttributeProxy extends Attribute {
        Type enumType;
	Name enumerator;
	public EnumAttributeProxy(Type enumType, Name enumerator) {
	    super(null);
	    this.enumType = enumType;
	    this.enumerator = enumerator;
	}
	public void accept(Visitor v) { ((ProxyVisitor)v).visitEnumAttributeProxy(this); }
	public String toString() {
	    return "/*proxy enum*/" + enumType + "." + enumerator;
	}
    }

    static class ArrayAttributeProxy extends Attribute {
	List<Attribute> values;
	ArrayAttributeProxy(List<Attribute> values) {
	    super(null);
	    this.values = values;
	}
	public void accept(Visitor v) { ((ProxyVisitor)v).visitArrayAttributeProxy(this); }
	public String toString() {
	    return "{" + values + "}";
	}
    }

    /** A temporary proxy representing a compound attribute.
     */
    static class CompoundAnnotationProxy extends Attribute {
	final List<Pair<Name,Attribute>> values;
	public CompoundAnnotationProxy(Type type,
				      List<Pair<Name,Attribute>> values) {
	    super(type);
	    this.values = values;
	}
	public void accept(Visitor v) { ((ProxyVisitor)v).visitCompoundAnnotationProxy(this); }
	public String toString() {
	    StringBuffer buf = new StringBuffer();
	    buf.append("@");
	    buf.append(type.tsym.fullName());
	    buf.append("/*proxy*/{");
	    boolean first = true;
	    for (List<Pair<Name,Attribute>> v = values;
		 v.nonEmpty(); v = v.tail) {
		Pair<Name,Attribute> value = v.head;
		if (!first) buf.append(",");
		first = false;
		buf.append(value.fst);
		buf.append("=");
		buf.append(value.snd);
	    }
	    buf.append("}");
	    return buf.toString();
	}
    }

    class AnnotationDeproxy implements ProxyVisitor {
	List<Attribute.Compound> deproxyCompoundList(List<CompoundAnnotationProxy> pl) {
	    // also must fill in types!!!!
	    ListBuffer<Attribute.Compound> buf =
		new ListBuffer<Attribute.Compound>();
	    for (List<CompoundAnnotationProxy> l = pl; l.nonEmpty(); l=l.tail) {
		buf.append(deproxyCompound(l.head));
	    }
	    return buf.toList();
	}

	Attribute.Compound deproxyCompound(CompoundAnnotationProxy a) {
	    ListBuffer<Pair<Symbol.MethodSymbol,Attribute>> buf =
		new ListBuffer<Pair<Symbol.MethodSymbol,Attribute>>();
	    for (List<Pair<Name,Attribute>> l = a.values;
		 l.nonEmpty();
		 l = l.tail) {
		MethodSymbol meth = findAccessMethod(a.type, l.head.fst);
		buf.append(new Pair<Symbol.MethodSymbol,Attribute>
			   (meth, deproxy(meth.type.restype(), l.head.snd)));
	    }
	    return new Attribute.Compound(a.type, buf.toList());
	}

	MethodSymbol findAccessMethod(Type container, Name name) {
	    for (Scope.Entry e = container.tsym.members().lookup(name);
		 e.scope != null;
		 e = e.next()) {
		Symbol sym = e.sym;
		if (sym.kind == MTH && sym.type.argtypes().length() == 0)
		    return (MethodSymbol) sym;
	    }
	    // we will need better error recovery than this.
	    // we should fail soft, allowing some binary compatibility
	    throw new AssertionError("cannot find method " +
				     container + "." + name + "()");
	}

	Attribute result;
	Type type;
	Attribute deproxy(Type t, Attribute a) {
	    Type oldType = type;
	    try {
		type = t;
		a.accept(this);
		return result;
	    } finally {
		type = oldType;
	    }
	}

	// implement Attribute.Visitor below

	public void visitConstant(Attribute.Constant value) {
	    // assert value.type == type;
	    result = value;
	}

	public void visitClass(Attribute.Class clazz) {
	    result = clazz;
	}

	public void visitEnum(Attribute.Enum e) {
	    throw new AssertionError(); // shouldn't happen
	}

	public void visitCompound(Attribute.Compound compound) {
	    throw new AssertionError(); // shouldn't happen
	}

	public void visitArray(Attribute.Array array) {
	    throw new AssertionError(); // shouldn't happen
	}

	public void visitError(Attribute.Error e) {
	    throw new AssertionError(); // shouldn't happen
	}

	public void visitEnumAttributeProxy(EnumAttributeProxy proxy) {
	    // type.tsym.flatName() should == proxy.enumFlatName
	    TypeSymbol enumTypeSym = proxy.enumType.tsym;
	    VarSymbol enumerator = null;
	    for (Scope.Entry e = enumTypeSym.members().lookup(proxy.enumerator);
		 e.scope != null;
		 e = e.next()) {
		if (e.sym.kind == VAR) {
		    enumerator = (VarSymbol)e.sym;
		    break;
		}
	    }
	    if (enumerator == null) {
		log.error(Position.NOPOS, "unknown.enum.constant",
                          currentClassFileName, enumTypeSym, proxy.enumerator);
		result = new Attribute.Error(enumTypeSym.type);
	    } else {
		result = new Attribute.Enum(enumTypeSym.type, enumerator);
	    }
	}

	public void visitArrayAttributeProxy(ArrayAttributeProxy proxy) {
	    int length = proxy.values.length();
	    Attribute[] ats = new Attribute[length];
	    Type elemtype = types.elemtype(type);
	    int i = 0;
	    for (List<Attribute> p = proxy.values; p.nonEmpty(); p = p.tail) {
		ats[i++] = deproxy(elemtype, p.head);
	    }
	    result = new Attribute.Array(type, ats);
	}

	public void visitCompoundAnnotationProxy(CompoundAnnotationProxy proxy) {
	    result = deproxyCompound(proxy);
	}
    }

    class AnnotationDefaultCompleter extends AnnotationDeproxy implements Annotate.Annotator {
	final MethodSymbol sym;
	final Attribute value;
        final String classFileName = currentClassFileName;
        public String toString() {
            return " ClassReader store default for " + sym.owner + "." + sym + " is " + value;
        }
	AnnotationDefaultCompleter(MethodSymbol sym, Attribute value) {
	    this.sym = sym;
	    this.value = value;
	}
	// implement Annotate.Annotator.enterAnnotation()
	public void enterAnnotation() {
            String previousClassFileName = currentClassFileName;
            try {
                currentClassFileName = classFileName;
                sym.defaultValue = deproxy(sym.type.restype(), value);
            } finally {
                currentClassFileName = previousClassFileName;
            }
	}
    }

    class AnnotationCompleter extends AnnotationDeproxy implements Annotate.Annotator {
	final Symbol sym;
	final List<CompoundAnnotationProxy> l;
        final String classFileName;
        public String toString() {
            return " ClassReader annotate " + sym.owner + "." + sym + " with " + l;
        }
	AnnotationCompleter(Symbol sym, List<CompoundAnnotationProxy> l) {
	    this.sym = sym;
	    this.l = l;
            this.classFileName = currentClassFileName;
	}
	// implement Annotate.Annotator.enterAnnotation()
	public void enterAnnotation() {
            String previousClassFileName = currentClassFileName;
            try {
                currentClassFileName = classFileName;
                List<Attribute.Compound> newList = deproxyCompoundList(l);
                sym.attributes_field = ((sym.attributes_field == null)
                                        ? newList
                                        : newList.prependList(sym.attributes_field));
            } finally {
                currentClassFileName = previousClassFileName;
            }
	}
    }


/************************************************************************
 * Reading Symbols
 ***********************************************************************/

    /** Read a field.
     */
    VarSymbol readField() {
	long flags = adjustFieldFlags(nextChar());
        Name name = readName(nextChar());
        Type type = readType(nextChar());
        VarSymbol v = new VarSymbol(flags, name, type, currentOwner);
	readMemberAttrs(v);
        return v;
    }

    /** Read a method.
     */
    MethodSymbol readMethod() {
	long flags = adjustMethodFlags(nextChar());
        Name name = readName(nextChar());
        Type type = readType(nextChar());
	if (name == names.init && currentOwner.hasOuterInstance()) {
	    type = new MethodType(
		type.argtypes().tail, type.restype(), type.thrown(),
		syms.methodClass);
	}
        MethodSymbol m = new MethodSymbol(flags, name, type, currentOwner);
	Symbol prevOwner = currentOwner;
	currentOwner = m;
	readMemberAttrs(m);
	currentOwner = prevOwner;
        return m;
    }

    /** Skip a field or method
     */
    void skipMember() {
	bp = bp + 6;
        char ac = nextChar();
        for (int i = 0; i < ac; i++) {
            bp = bp + 2;
            int attrLen = nextInt();
	    bp = bp + attrLen;
	}
    }

    /** Enter type variables of this classtype and all enclosing ones in
     *  `typevars'.
     */
    void enterTypevars(Type t) {
	if (t.outer().tag == CLASS) enterTypevars(t.outer());
	for (List<Type> xs = t.typarams(); xs.nonEmpty(); xs = xs.tail)
	    typevars.enter(xs.head.tsym);
    }

    /** Read contents of a given class symbol `c'. Both external and internal
     *  versions of an inner class are read.
     */
    void readClass(ClassSymbol c) {
	ClassType ct = (ClassType)c.type;

	// allocate scope for members
	c.members_field = new Scope(c);

	// prepare type variable table
	typevars = typevars.dup(currentOwner);
	if (ct.outer().tag == CLASS) enterTypevars(ct.outer());

	// read flags, or skip if this is an inner class
	long flags = adjustClassFlags(nextChar());
	if (c.owner.kind == PCK) c.flags_field = flags;

	// read own class name and check that it matches
	ClassSymbol self = readClassSymbol(nextChar());
	if (c != self)
	    throw badClassFile("class.file.wrong.class",
			       self.flatname);

	// class attributes must be read before class
	// skip ahead to read class attributes
	int startbp = bp;
	nextChar();
	char interfaceCount = nextChar();
	bp += interfaceCount * 2;
	char fieldCount = nextChar();
	for (int i = 0; i < fieldCount; i++) skipMember();
	char methodCount = nextChar();
	for (int i = 0; i < methodCount; i++) skipMember();
	readClassAttrs(c);

	if (readAllOfClassFile) {
	    for (int i = 1; i < poolObj.length; i++) readPool(i);
	    c.pool = new Pool(poolObj.length, poolObj);
	}

	// reset and read rest of classinfo
	bp = startbp;
	int n = nextChar();
	if (ct.supertype_field == null)
	    ct.supertype_field = (n == 0)
		? Type.noType
		: readClassSymbol(n).erasure(types);
	n = nextChar();
	ListBuffer<Type> is = new ListBuffer<Type>();
	for (int i = 0; i < n; i++) {
	    Type _inter = readClassSymbol(nextChar()).erasure(types);
	    is.append(_inter);
	}
	if (ct.interfaces_field == null)
	    ct.interfaces_field = is.toList();

	if (fieldCount != nextChar()) assert false;
	for (int i = 0; i < fieldCount; i++) enterMember(c, readField());
	if (methodCount != nextChar()) assert false;
	for (int i = 0; i < methodCount; i++) enterMember(c, readMethod());

	typevars = typevars.leave();
    }

    /** Read inner class info. For each inner/outer pair allocate a
     *  member class.
     */
    void readInnerClasses(ClassSymbol c) {
	int n = nextChar();
	for (int i = 0; i < n; i++) {
	    nextChar(); // skip inner class symbol
	    ClassSymbol outer = readClassSymbol(nextChar());
	    Name name = readName(nextChar());
	    if (name == null) name = names.empty;
	    long flags = adjustClassFlags(nextChar());
	    if (outer != null) { // we have a member class
		if (name == names.empty)
		    name = names.one;
		ClassSymbol member = enterClass(name, outer);
		if ((flags & STATIC) == 0) {
		    ((ClassType)member.type).outer_field = outer.type;
                    if (member.erasure_field != null)
                        ((ClassType)member.erasure_field).outer_field = types.erasure(outer.type);
		}
		if (c == outer) {
		    member.flags_field = flags;
		    enterMember(c, member);
		}
	    }
	}
    }

    /** Read a class file.
     */
    private void readClassFile(ClassSymbol c) throws IOException {
	int magic = nextInt();
	if (magic != JAVA_MAGIC)
	    throw badClassFile("illegal.start.of.class.file");

	int minorVersion = nextChar();
	int majorVersion = nextChar();
	if (majorVersion > Target.MAX().majorVersion ||
	    majorVersion * 1000 + minorVersion <
	    Target.MIN().majorVersion * 1000 + Target.MIN().minorVersion)
	{
	    throw badClassFile("wrong.version",
			       Integer.toString(majorVersion),
			       Integer.toString(minorVersion),
			       Integer.toString(Target.MAX().majorVersion),
			       Integer.toString(Target.MAX().minorVersion));
	}
	else if (checkClassFile &&
		 majorVersion == Target.MAX().majorVersion &&
		 minorVersion > Target.MAX().minorVersion)
	{
	    printCCF("found.later.version",
		     Integer.toString(minorVersion));
	}
	indexPool();
	readClass(c);
    }

/************************************************************************
 * Adjusting flags
 ***********************************************************************/

    long adjustFieldFlags(long flags) {
	return flags;
    }
    long adjustMethodFlags(long flags) {
	if ((flags & ACC_BRIDGE) != 0) {
	    flags &= ~ACC_BRIDGE;
	    flags |= BRIDGE;
	    if (!allowGenerics)
		flags &= ~SYNTHETIC;
	}
	if ((flags & ACC_VARARGS) != 0) {
	    flags &= ~ACC_VARARGS;
	    flags |= VARARGS;
	}
	return flags;
    }
    long adjustClassFlags(long flags) {
	return flags;
    }

/************************************************************************
 * Accessing Files
 ***********************************************************************/

    /** Is this the name of a zip file?
     */
    static boolean isZip(String name) {
	return new File(name).isFile();
    }

    /** An archive consists of a zipfile and a list of zip entries in
     *  that file.
     */
    static class Archive {
	ZipFile zdir;
	List<ZipEntry> entries;
	Archive(ZipFile zdir, List<ZipEntry> entries) {
	    this.zdir = zdir;
	    this.entries = entries;
	}
    }

    /** A directory of zip files already opened.
     */
    Map<String,Archive> archives = new HashMap<String,Archive>();

    /** Open a new zip file directory.
     */
    Archive openArchive(String dirname) throws IOException {
	Archive archive = archives.get(dirname);
	if (archive == null) {
	    ZipFile zdir = new ZipFile(dirname);
	    ListBuffer<ZipEntry> entries = new ListBuffer<ZipEntry>();
	    for (java.util.Enumeration e = zdir.entries();
		 e.hasMoreElements(); ) {
		entries.append((ZipEntry)e.nextElement());
	    }
	    archive = new Archive(zdir, entries.toList());
	    archives.put(dirname, archive);
	}
	return archive;
    }

    /** Close the ClassReader, releasing resources.
     */
    public void close() {
	for (Iterator<Archive> i = archives.values().iterator(); i.hasNext(); ) {
	    Archive a = i.next();
	    i.remove();
	    try {
		a.zdir.close();
	    } catch (IOException e) {
	    }
	}
    }

/************************************************************************
 * Loading Classes
 ***********************************************************************/

    /** Define a new class given its name and owner.
     */
    public ClassSymbol defineClass(Name name, Symbol owner) {
	ClassSymbol c = new ClassSymbol(0, name, owner);
	c.completer = this;
	return c;
    }

    /** Create a new toplevel or member class symbol with given name
     *  and owner and enter in `classes' unless already there.
     */
    public ClassSymbol enterClass(Name name, TypeSymbol owner) {
	Name flatname = TypeSymbol.formFlatName(name, owner);
	ClassSymbol c = classes.get(flatname);
	if (c == null) {
	    c = defineClass(name, owner);
	    classes.put(flatname, c);
	} else if ((c.name != name || c.owner != owner) && owner.kind == TYP) {
	    // reassign fields of classes that might have been loaded with
	    // their flat names.
	    c.owner.members().remove(c);
	    c.name = name;
	    c.owner = owner;
	    c.fullname = ClassSymbol.formFullName(name, owner);
	}
	return c;
    }

    /** Create a new member or toplevel class symbol with given flat name
     *  and enter in `classes' unless already there.
     */
    public ClassSymbol enterClass(Name flatname) {
	ClassSymbol c = classes.get(flatname);
	if (c == null) {
	    Name packageName = Convert.packagePart(flatname);
	    if (packageName == names.empty) packageName = names.emptyPackage;
            c = defineClass(Convert.shortName(flatname),
			    enterPackage(packageName));
	    classes.put(flatname, c);
	}
        return c;
    }

    /** Completion for classes to be loaded. Before a class is loaded
     *  we make sure its enclosing class (if any) is loaded.
     */
    public void complete(Symbol sym) throws CompletionFailure {
	if (sym.kind == TYP) {
	    ClassSymbol c = (ClassSymbol)sym;
	    c.members_field = new Scope.ErrorScope(c); // make sure it's always defined
            completeOwners(c.owner);
	    fillIn(c);
	} else if (sym.kind == PCK) {
	    PackageSymbol p = (PackageSymbol)sym;
	    fillIn(p);
	}
	annotate.flush(); // finish attaching annotations
    }

    /** complete up through the enclosing package. */
    private void completeOwners(Symbol o) {
        if (o.kind != PCK) completeOwners(o.owner);
        o.complete();
    }

    /** We can only read a single class file at a time; this
     *  flag keeps track of when we are currently reading a class
     *  file.
     */
    private boolean filling = false;

    /** Fill in definition of class `c' from corresponding class or
     *  source file.
     */
    private void fillIn(ClassSymbol c) {
	if (completionFailureName == c.fullname) {
	    throw new CompletionFailure(c, "user-selected completion failure by class name");
	}
	currentOwner = c;
	FileEntry classfile = c.classfile;
	if (classfile != null) {
            String previousClassFile = currentClassFileName;
	    try {
		assert !filling;
		InputStream s = classfile.open();
		currentClassFileName = classfile.getPath();
		if (verbose) {
		    printVerbose("loading", currentClassFileName);
		}
		if (classfile.getName().endsWith(".class")) {
		    filling = true;
		    int size = (int)classfile.length();
		    if (buf.length < size) buf = new byte[size];
		    int n = 0;
		    while (n < size)
			n = n + s.read(buf, n, size - n);
		    s.close();
		    bp = 0;
		    readClassFile(c);
		} else {
		    sourceCompleter.complete(c, currentClassFileName, s);
		}
		return;
	    } catch (IOException ex) {
		throw badClassFile("unable.to.access.file", ex.getMessage());
	    } finally {
		filling = false;
                currentClassFileName = previousClassFile;
	    }
	} else {
	    String fn = externalizeFileName(c.flatname);
	    throw
		newCompletionFailure(c,
				     Log.
				     getLocalizedString("dot.class.not.found",
							fn));
	}
    }
    // where
	/** Static factory for CompletionFailure objects.
	 *  In practice, only one can be used at a time, so we share one
	 *  to reduce the expense of allocating new exception objects.
	 */
	private CompletionFailure newCompletionFailure(ClassSymbol c,
						       String localized) {
	    // return new CompletionFailure(c, localized);
	    CompletionFailure result = cachedCompletionFailure;
	    result.sym = c;
	    result.errmsg = localized;
	    return result;
	}
	private CompletionFailure cachedCompletionFailure =
	    new CompletionFailure(null, null);
        {
	    cachedCompletionFailure.setStackTrace(new StackTraceElement[0]);
        }

    /** Load a toplevel class with given fully qualified name
     *  The class is entered into `classes' only if load was successful.
     */
    public ClassSymbol loadClass(Name flatname) throws CompletionFailure {
	boolean absent = classes.get(flatname) == null;
	ClassSymbol c = enterClass(flatname);
	if (c.members_field == null && c.completer != null) {
	    try {
		c.complete();
	    } catch (CompletionFailure ex) {
		if (absent) classes.remove(flatname);
		throw ex;
	    }
	}
	return c;
    }

/************************************************************************
 * Loading Packages
 ***********************************************************************/

    /** Check to see if a package exists, given its fully qualified name.
     */
    public boolean packageExists(Name fullname) {
	return enterPackage(fullname).exists();
    }

    /** Make a package, given its fully qualified name.
     */
    public PackageSymbol enterPackage(Name fullname) {
	PackageSymbol p = packages.get(fullname);
	if (p == null) {
	    assert fullname.length() != 0 : "rootPackage missing!";
	    p = new PackageSymbol(
		Convert.shortName(fullname),
		enterPackage(Convert.packagePart(fullname)));
	    p.completer = this;
	    packages.put(fullname, p);
	}
	return p;
    }

    /** Make a package, given its unqualified name and enclosing package.
     */
    public PackageSymbol enterPackage(Name name, PackageSymbol owner) {
	return enterPackage(TypeSymbol.formFullName(name, owner));
    }

    /** Include class corresponding to given class file in package,
     *  unless (1) we already have one the same kind (.class or .java), or
     *         (2) we have one of the other kind, and the given class file
     *             is older.
     */
    private void includeClassFile(PackageSymbol p, FileEntry file) {
	if ((p.flags_field & EXISTS) == 0)
	    for (Symbol q = p; q != null && q.kind == PCK; q = q.owner)
		q.flags_field |= EXISTS;
	String filename = file.getName();
	int seen;
	int extlen;
	if (filename.endsWith(".class")) {
	    seen = CLASS_SEEN;
	    extlen = 6;
	} else {
	    seen = SOURCE_SEEN;
	    extlen = 5;
	}
	Name classname = names.
	    fromString(filename.substring(0, filename.length() - extlen));
	ClassSymbol c = (ClassSymbol) p.members_field.lookup(classname).sym;
	if (c == null) {
	    c = enterClass(classname, p);
	    if (c.classfile == null) // only update the file if's it's newly created
		c.classfile = file;
	    if (c.owner == p)  // it might be an inner class
		p.members_field.enter(c);
	} else if (c.classfile != null && (c.flags_field & seen) == 0) {
	    // if c.classfile == null, we are currently compiling this class
	    // and no further action is necessary.
	    // if (c.flags_field & seen) != 0, we have already encountered
	    // a file of the same kind; again no further action is necessary.
	    if ((c.flags_field & (CLASS_SEEN | SOURCE_SEEN)) != 0) {
		// we have encountered a file of the other kind,
		// i.e. ".class" instead of ".java" or vice versa.
		// now we have find out which one is newer.
		long fdate = file.lastModified();
		long cdate = c.classfile.lastModified();
		if (fdate >= 0 && cdate >= 0 && fdate > cdate) {
		    c.classfile = file;
		}
	    }
	}
	c.flags_field |= seen;
    }

    /** Insert all files in subdirectory `name' of `pathname'
     *  which end in one of the extensions in `extensions' into package.
     */
    private void list(String pathname, String name,
		      String[] extensions, PackageSymbol p) {
	try {
	    if (isZip(pathname)) {
		Archive archive = openArchive(pathname);
		if (name.length() != 0) {
		    name = name.replace('\\', '/');
		    if (!name.endsWith("/")) name = name + "/";
		}
		int namelen = name.length();
		for (List<ZipEntry> l = archive.entries;
		     l.nonEmpty();
		     l = l.tail)
		{
		    ZipEntry entry = l.head;
		    String ename = entry.getName();
		    if (ename.startsWith(name)) {
			if (endsWith(ename, extensions)) {
			    String suffix = ename.substring(namelen);
			    if (suffix.length() > 0 &&
				suffix.indexOf('/') < 0) {
				includeClassFile(
						 p,
						 new FileEntry.Zipped(suffix,
						     archive.zdir, entry));
			    }
			} else extraZipFileActions(p, ename, name, pathname);
		    }
		}
	    } else {
		File f = name.length() != 0
		    ? new File(pathname, name)
		    : new File(pathname);
		String[] names = f.list();
		if (names != null && caseMapCheck(f, name))
		{
		    for (int i = 0; i < names.length; i++) {
			String fname = names[i];
			if (isValidFile(fname, extensions)) {
			    includeClassFile(
                                p,
			        new FileEntry.Regular(fname,
						      new File(f, fname)));
			} else extraFileActions(p, fname, f);
		    }
		}
	    }
	} catch (IOException ex) {
	}
    }
//where
        private boolean endsWith(String s, String[] extensions) {
	    for (int i = 0; i < extensions.length; i++)
		if (s.endsWith(extensions[i])) return true;
	    return false;
	}
        private boolean isValidFile(String s, String[] extensions) {
	    for (int i = 0; i < extensions.length; i++) {
		String extension = extensions[i];
		if (s.endsWith(extension) &&
		    isJavaIdentifier(s.substring(0, s.length() -
						 extension.length())))
		    return true;
	    }
	    return false;
	}
        private boolean isJavaIdentifier(String s) {
	    if (s.length() < 1) return false;
            if (surrogatesSupported) {
                int cp = s.codePointAt(0);
                if (!Character.isJavaIdentifierStart(cp))
                    return false;
                for (int j=Character.charCount(cp); j<s.length(); j+=Character.charCount(cp)) {
                    cp = s.codePointAt(j);
                    if (!Character.isJavaIdentifierPart(cp))
                        return false;
                }
            } else {
                if (!Character.isJavaIdentifierStart(s.charAt(0)))
                    return false;
                for (int j=1; j<s.length(); j++)
                    if (!Character.isJavaIdentifierPart(s.charAt(j)))
                        return false;
            }
	    return true;
	}
        static final boolean fileSystemIsCaseSensitive =
	    File.separatorChar == '/';
	/** Hack to make Windows case sensitive. Test whether given path
	 *  ends in a string of characters with the same case as given name.
	 *  Ignore file separators in both path and name.
	 */
        private boolean caseMapCheck(File f, String name) throws IOException {
	    if (fileSystemIsCaseSensitive) return true;
	    // Note that getCanonicalPath() returns the case-sensitive
	    // spelled file name.
	    String path = f.getCanonicalPath();
	    char[] pcs = path.toCharArray();
	    char[] ncs = name.toCharArray();
	    int i = pcs.length - 1;
	    int j = ncs.length - 1;
	    while (i >= 0 && j >= 0) {
		while (i >= 0 && pcs[i] == File.separatorChar) i--;
		while (j >= 0 && ncs[j] == File.separatorChar) j--;
		if (i >= 0 && j >= 0) {
		    if (pcs[i] != ncs[j]) return false;
		    i--;
		    j--;
		}
	    }
	    return j < 0;
	}
	/** These are used to support javadoc
	 */
	protected void extraZipFileActions(PackageSymbol pack,
					   String zipEntryName,
					   String classPathName,
					   String zipName) {}
	protected void extraFileActions(PackageSymbol pack,
					String fileName,
					File fileDir) {}


    /** Are surrogates supported?
     */
    final static boolean surrogatesSupported = surrogatesSupported();
    private static boolean surrogatesSupported() {
        try {
            boolean b = Character.isHighSurrogate('a');
            return true;
        } catch (NoSuchMethodError ex) {
            return false;
        }
    }

    /** Insert all files in subdirectory `name' somewhere on
     *  class path `path' which end in one of the extensions in `extensions'
     *  into package.
     */
    private void listAll(Collection<String> files,
                         String name,
			 String[] extensions,
                         PackageSymbol p) {
	for (String file : files)
	    list(file, name, extensions, p);
    }

    private static final String[] classOnly   = {".class"};
    private static final String[] javaOnly    = {".java"};
    private static final String[] classOrJava = {".class", ".java"};

    /** Load directory of package into members scope.
     */
    private void fillIn(PackageSymbol p) {
	if (p.members_field == null) p.members_field = new Scope(p);
	Name packageName = p.fullname;
	if (packageName == names.emptyPackage) packageName = names.empty;
	String dirname = externalizeFileName(packageName);
	listAll(paths.bootClassPath(), dirname, classOnly, p);
	if (sourceCompleter != null && paths.sourcePath() == null) {
	    listAll(paths.userClassPath(), dirname, classOrJava, p);
	} else {
	    listAll(paths.userClassPath(), dirname, classOnly, p);
	    if (sourceCompleter != null) {
		listAll(paths.sourcePath(), dirname, javaOnly, p);
	    }
	}
    }

    /** Output for "-verbose" option.
     *  @param key The key to look up the correct internationalized string.
     *  @param arg An argument for substitution into the output string.
     */
    private void printVerbose(String key, String arg) {
	Log.printLines(log.noticeWriter, Log.getLocalizedString("verbose." + key, arg));
    }

    /** Output for "-checkclassfile" option.
     *  @param key The key to look up the correct internationalized string.
     *  @param arg An argument for substitution into the output string.
     */
    private void printCCF(String key, Object arg) {
	Log.printLines(log.noticeWriter, Log.getLocalizedString("verbose." + key, arg));
    }


    public interface SourceCompleter {
        void complete(ClassSymbol sym, String filename, InputStream f)
	    throws CompletionFailure;
    }
}
