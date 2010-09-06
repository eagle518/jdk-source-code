/**
 * @(#)JavaCompiler.java	1.67 04/04/15
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.main;

import java.io.*;
import java.util.Set;
import java.util.HashSet;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.tree.*;
import com.sun.tools.javac.parser.*;
import com.sun.tools.javac.comp.*;
import com.sun.tools.javac.jvm.*;

import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.tree.Tree.*;

/** This class could be the main entry point for GJC when GJC is used as a
 *  component in a larger software system. It provides operations to
 *  construct a new compiler, and to run a new compiler on a set of source
 *  files.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class JavaCompiler implements ClassReader.SourceCompleter {
    /** The context key for the compiler. */
    protected static final Context.Key<JavaCompiler> compilerKey =
	new Context.Key<JavaCompiler>();

    /** Get the JavaCompiler instance for this context. */
    public static JavaCompiler instance(Context context) {
	JavaCompiler instance = context.get(compilerKey);
	if (instance == null)
	    instance = new JavaCompiler(context);
	return instance;
    }

    /** The current version number as a string.
     */
    public static String version() { return System.getProperty("java.version"); }

    /** The log to be used for error reporting.
     */
    Log log;

    /** The tree factory module.
     */
    TreeMaker make;

    /** The class reader.
     */
    ClassReader reader;

    /** The class writer.
     */
    ClassWriter writer;

    /** The module for the symbol table entry phases.
     */
    Enter enter;

    /** The symbol table.
     */
    Symtab syms;

    /** The language version.
     */
    Source source;

    /** The module for code generation.
     */
    Gen gen;

    /** The name table.
     */
    Name.Table names;

    /** The attributor.
     */
    Attr attr;

    /** The attributor.
     */
    Check chk;

    /** The flow analyzer.
     */
    Flow flow;

    /** The type eraser.
     */
    TransTypes transTypes;

    /** The syntactic sugar desweetener.
     */
    Lower lower;

    /** The annotation annotator.
     */
    Annotate annotate;

    /** Force a completion failure on this name
     */
    final Name completionFailureName;

    /** Type utilities.
     */
    Types types;

    Scanner.Factory scannerFactory;
    Parser.Factory parserFactory;

    /** Construct a new compiler from a log, a symbol table and an options table.
     */
    public JavaCompiler(Context context) {
	context.put(compilerKey, this);
	names = Name.Table.instance(context);
	log = Log.instance(context);
	reader = ClassReader.instance(context);
	make = TreeMaker.instance(context);
	writer = ClassWriter.instance(context);
	enter = Enter.instance(context);
	todo = Todo.instance(context);
	parserFactory = Parser.Factory.instance(context);
	scannerFactory = Scanner.Factory.instance(context);
	try {
	    // catch completion problems with predefineds
	    syms = Symtab.instance(context);
	} catch (CompletionFailure ex) {
	    log.error(Position.NOPOS, ex.getMessage());
	}
	source = Source.instance(context);
	attr = Attr.instance(context);
	chk = Check.instance(context);
	gen = Gen.instance(context);
	flow = Flow.instance(context);
	transTypes = TransTypes.instance(context);
	lower = Lower.instance(context);
	annotate = Annotate.instance(context);
	types = Types.instance(context);

	reader.sourceCompleter = this;

	Options options = Options.instance(context);

	verbose       = options.get("-verbose")       != null;
	sourceOutput  = options.get("-s")             != null;
	stubOutput    = options.get("-stubs")         != null;
	relax         = options.get("-relax")         != null;
	printFlat     = options.get("-printflat")     != null;
	deprecation   = options.lint("deprecation");
	warnunchecked = options.lint("unchecked");
	attrParseOnly = options.get("-attrparseonly") != null;
	encoding      = options.get("-encoding");

	completionFailureName =
	    (options.get("failcomplete") != null)
	    ? names.fromString(options.get("failcomplete"))
	    : null;
    }

    /* Switches:
     */

    /** Verbose output.
     */
    public boolean verbose;

    /** Emit plain Java source files rather than class files.
     */
    public boolean sourceOutput;

    /** Emit stub source files rather than class files.
     */
    public boolean stubOutput;

    /** Generate attributed parse tree only.
     */
    public boolean attrParseOnly;

    /** Switch: relax some constraints for producing the jsr14 prototype.
     */
    boolean relax;

    /** Debug switch: Emit Java sources after inner class flattening.
     */
    public boolean printFlat;

    /** Give detailed deprecation warnings.
     */
    public boolean deprecation;

    /** Give detailed unchecked warnings.
     */
    public boolean warnunchecked;

    /** The encoding to be used for source input.
     */
    public String encoding;

    /** A queue of all as yet unattributed classes.
     */
    private Todo todo;

    /** The set of currently compiled inputfiles, needed to ensure
     *  we don't accidentally overwrite an input file when -s is set.
     *  initialized by `compile'.
     */
    Set<File> inputFiles = new HashSet<File>();

    /** The number of errors reported so far.
     */
    public int errorCount() {
	return log.nerrors;
    }

    /** The number of errors reported so far.
     */
    public int warningCount() {
	return log.nwarnings;
    }

    /** Try to open input stream with given name.
     *  Report an error if this fails.
     *  @param filename   The file name of the input stream to be opened.
     */
    public InputStream openSource(String filename) {
        try {
	    File f = new File(filename);
	    inputFiles.add(f);
	    return new FileInputStream(f);
        } catch (IOException e) {
	    log.error(Position.NOPOS, "cant.read.file", filename);
            return null;
        }
    }

    /** Parse contents of input stream.
     *  @param filename     The name of the file from which input stream comes.
     *  @param input        The input stream to be parsed.
     */
    public TopLevel parse(String filename, InputStream input) {
        long msec = System.currentTimeMillis();
	Name prev = log.useSource(names.fromString(filename));
	TopLevel tree = make.TopLevel(Tree.Annotation.emptyList,
				      null, Tree.emptyList);
        if (input != null) {
            if (verbose) {
		printVerbose("parsing.started", filename);
            }
	    try {
		Scanner scanner = scannerFactory.newScanner(input, encoding);
		input.close();
		Parser parser = parserFactory.newParser(scanner, keepComments());
		tree = parser.compilationUnit();
		if (verbose) {
		    printVerbose("parsing.done",
				 Long.toString(System.currentTimeMillis() - msec));
		}
	    }  catch (IOException e) {
		log.error(Position.NOPOS,
			  "error.reading.file", filename, e);
	    }
	}
	log.useSource(prev);
	tree.sourcefile = names.fromString(filename);
	return tree;
    }
    // where
	protected boolean keepComments() {
	    return sourceOutput || stubOutput;
	}


    /** Parse contents of file.
     *  @param filename     The name of the file to be parsed.
     */
    public Tree.TopLevel parse(String filename) {
        return parse(filename, openSource(filename));
    }

    /** Emit plain Java source for a class.
     *  @param env    The attribution environment of the outermost class
     *                containing this class.
     *  @param cdef   The class definition to be printed.
     */
    void printSource(Env<AttrContext> env, ClassDef cdef) throws IOException {
	File outFile = writer.outputFile(cdef.sym, ".java");
	if (inputFiles.contains(outFile)) {
	    log.error(cdef.pos, "source.cant.overwrite.input.file", outFile);
	} else {
	    PrintWriter out = new PrintWriter
		(new BufferedWriter
		    (new OutputStreamWriter
			(new FileOutputStream(outFile))));
	    try {
		new Pretty(out, true).printUnit(env.toplevel, cdef);
		if (verbose)
		    printVerbose("wrote.file", outFile.getPath());
	    } finally {
		out.close();
	    }
	}
    }

    /** Generate code and emit a class file for a given class
     *  @param env    The attribution environment of the outermost class
     *                containing this class.
     *  @param cdef   The class definition from which code is generated.
     */
    void genCode(Env<AttrContext> env, ClassDef cdef) throws IOException {
	try {
	    if (gen.genClass(env, cdef)) writer.writeClass(cdef.sym);
	} catch (ClassWriter.PoolOverflow ex) {
	    log.error(cdef.pos, "limit.pool");
	} catch (ClassWriter.StringOverflow ex) {
	    log.error(cdef.pos, "limit.string.overflow",
		      ex.value.substring(0, 20));
	} catch (CompletionFailure ex) {
	    log.error(Position.NOPOS, ex.getMessage());
	}
    }

    /** Complete compiling a source file that has been accessed
     *  by the class file reader.
     *  @param c          The class the source file of which needs to be compiled.
     *  @param filename   The name of the source file.
     *  @param f          An input stream that reads the source file.
     */
    public void complete(ClassSymbol c,
			 String filename,
			 InputStream f) throws CompletionFailure {
//      System.err.println("completing " + c);//DEBUG
	if (completionFailureName == c.fullname) {
	    throw new CompletionFailure(c, "user-selected completion failure by class name");
	}
	Tree tree = parse(filename, f);
	enter.complete(List.make(tree), c);
	if (enter.getEnv(c) == null) {
	    throw new
		ClassReader.BadClassFile(c, filename, log.
					 getLocalizedString("file.doesnt.contain.class",
							    c.fullname));
	}
    }

    /** Track when the JavaCompiler has been used to compile something. */
    private boolean hasBeenUsed = false;

    /** Main method: compile a list of files, return all compiled classes
     *  @param filenames     The names of all files to be compiled.
     */
    public List<ClassSymbol> compile(List<String> filenames) throws Throwable {
	// as a JavaCompiler can only be used once, throw an exception if
	// it has been used before.
	assert !hasBeenUsed : "attempt to reuse JavaCompiler";
	hasBeenUsed = true;

	long msec = System.currentTimeMillis();
	ListBuffer<ClassSymbol> classes = new ListBuffer<ClassSymbol>();
	try {
	    //parse all files
	    ListBuffer<Tree> trees = new ListBuffer<Tree>();
	    for (List<String> l = filenames; l.nonEmpty(); l = l.tail)
		trees.append(parse(l.head));

            //enter symbols for all files
            List<Tree> roots = trees.toList();
            if (errorCount() == 0)
		enter.main(roots);

            //If generating source, remember the classes declared in
            //the original compilation units listed on the command line.
            List<ClassDef> rootClasses = null;
            if (sourceOutput || stubOutput) {
                ListBuffer<ClassDef> cdefs = new ListBuffer <ClassDef>();
                for (List<Tree> l = roots; l.nonEmpty(); l = l.tail) {
                    for (List<Tree> defs = ((TopLevel)l.head).defs;
			 defs.nonEmpty();
			 defs = defs.tail) {
                        if (defs.head instanceof ClassDef)
                            cdefs.append((ClassDef)defs.head);
                    }
                }
                rootClasses = cdefs.toList();
            }

	    while (todo.nonEmpty()) {
		Env<AttrContext> env = todo.next();

		//save tree prior to rewriting
                Tree untranslated = env.tree;

		//attribution phase
		if (verbose)
		    printVerbose("checking.attribution", env.enclClass.sym);
		Name prev = log.useSource(env.enclClass.sym.sourcefile);
		attr.attribClass(env.tree.pos, env.enclClass.sym);
		if (attrParseOnly) continue;

		make.at(Position.FIRSTPOS);
		TreeMaker localMake = make.forToplevel(env.toplevel);

		//dataflow checks: definite assignments, unreachable statements
		if (errorCount() == 0  && !relax) {
		    flow.analyzeTree(env.tree, localMake);
		}

		ClassDef cdef = null;
		try {
		    if (errorCount() == 0) {
                        if (env.tree instanceof TopLevel) {
                            List<Tree> pdef = lower.translateTopLevelClass(env, env.tree, localMake);
                            if (pdef.head != null) {
                                assert pdef.tail.isEmpty();
                                genCode(env, (ClassDef)pdef.head);
                            }
                            continue;
                        }

			if (stubOutput) {
                            //emit stub Java source file, only for compilation
                            //units enumerated explicitly on the command line
			    cdef = (ClassDef)env.tree;
                            if (untranslated instanceof ClassDef &&
				rootClasses.contains((ClassDef)untranslated) &&
				((cdef.mods.flags & (Flags.PROTECTED|Flags.PUBLIC)) != 0 ||
				 cdef.sym.packge().fullName() == names.java_lang)) {
                                printSource(env, removeMethodBodies(cdef));
                            }
			    continue;
			}

                        env.tree = transTypes.translateTopLevelClass(env.tree, localMake);

			if (errorCount() != 0) continue;

			if (sourceOutput) {
                            //emit standard Java source file, only for compilation
                            //units enumerated explicitly on the command line
			    cdef = (ClassDef)env.tree;
                            if (untranslated instanceof ClassDef &&
				rootClasses.contains((ClassDef)untranslated)) {
                                printSource(env, cdef);
                            }
			    continue;
			}

			//translate out inner classes
			List<Tree> cdefs = lower
			    .translateTopLevelClass(env, env.tree, localMake);

			//generate code for each class
			if (errorCount() != 0) continue;
			for (List<Tree> l = cdefs;
			     errorCount() == 0 && l.nonEmpty();
			     l = l.tail) {
			    cdef = (ClassDef)l.head;
			    if (printFlat)
				printSource(env, cdef);
			    else
				genCode(env, cdef);
			    classes.append(cdef.sym);
			}
		    }
		} catch (IOException ex) {
		    log.error(cdef.pos, "class.cant.write",
			      cdef.sym, ex.getMessage());
		} finally {
		    log.useSource(prev);
		}
	    }
	} catch (Abort ex) {
	}

	if (verbose)
	    printVerbose("total", Long.toString(System.currentTimeMillis() - msec));
	if (chk.deprecatedSource != null && !deprecation)
	    noteDeprecated(chk.deprecatedSource);
	if (chk.uncheckedSource != null && !warnunchecked)
	    makeNotes(chk.uncheckedSource.toString());

	int errCount = errorCount();
	if (errCount == 1)
	    printCount("error", errCount);
	else
	    printCount("error.plural", errCount);

	if (log.nwarnings == 1)
	    printCount("warn", log.nwarnings);
	else
	    printCount("warn.plural", log.nwarnings);

	return classes.toList();
    }
	// where
	ClassDef removeMethodBodies(ClassDef cdef) {
	    final boolean isInterface = (cdef.mods.flags & Flags.INTERFACE) != 0;
	    class MethodBodyRemover extends TreeTranslator {
		public void visitMethodDef(MethodDef tree) {
		    tree.mods.flags &= ~Flags.SYNCHRONIZED;
		    for (VarDef vd : tree.params)
			vd.mods.flags &= ~Flags.FINAL;
		    tree.body = null;
		    super.visitMethodDef(tree);
		}
		public void visitVarDef(VarDef tree) {
		    if (tree.init != null && tree.init.type.constValue == null)
			tree.init = null;
		    super.visitVarDef(tree);
		}
		public void visitClassDef(ClassDef tree) {
		    ListBuffer<Tree> newdefs = new ListBuffer<Tree>();
		    for (List<Tree> it = tree.defs; it.tail != null; it = it.tail) {
			Tree t = it.head;
			switch (t.tag) {
			case Tree.CLASSDEF:
			    if (isInterface ||
				(((ClassDef)t).mods.flags & (Flags.PROTECTED|Flags.PUBLIC)) != 0 ||
				(((ClassDef)t).mods.flags & (Flags.PRIVATE)) == 0 && ((ClassDef)t).sym.packge().fullName() == names.java_lang)
				newdefs.append(t);
			    break;
			case Tree.METHODDEF:
			    if (isInterface ||
				(((MethodDef)t).mods.flags & (Flags.PROTECTED|Flags.PUBLIC)) != 0 ||
				((MethodDef)t).sym.name == names.init ||
				(((MethodDef)t).mods.flags & (Flags.PRIVATE)) == 0 && ((MethodDef)t).sym.packge().fullName() == names.java_lang)
				newdefs.append(t);
			    break;
			case Tree.VARDEF:
			    if (isInterface || (((VarDef)t).mods.flags & (Flags.PROTECTED|Flags.PUBLIC)) != 0 ||
				(((VarDef)t).mods.flags & (Flags.PRIVATE)) == 0 && ((VarDef)t).sym.packge().fullName() == names.java_lang)
				newdefs.append(t);
			    break;
			default:
			    break;
			}
		    }
		    tree.defs = newdefs.toList();
		    super.visitClassDef(tree);
		}
	    }
	    MethodBodyRemover r = new MethodBodyRemover();
	    return (ClassDef)r.translate(cdef);
	}

    /** Close the compiler, flushing the logs
     */
    public void close() {
	log.flush();
	reader.close();
	names.dispose();
    }

    /** Output for "-verbose" option.
     *  @param key The key to look up the correct internationalized string.
     *  @param arg An argument for substitution into the output string.
     */
    private void printVerbose(String key, Object arg) {
	Log.printLines(log.noticeWriter, log.getLocalizedString("verbose." + key, arg));
    }

    /** Print note that deprecated API's are used.
     */
    private void noteDeprecated(Object input) {
	if (input.equals("*"))
	    log.note("deprecated.plural");
	else
	    log.note("deprecated.filename", input);
	log.note("deprecated.recompile");
    }

    /** Print note that unchecked operations are used.
     */
    void makeNotes(Object input) {
	if (input.toString().equals("*"))
	    log.note("unchecked.plural");
	else
	    log.note("unchecked.filename", input);
	log.note("unchecked.recompile");
    }

    /** Print numbers of errors and warnings.
     */
    void printCount(String kind, int count) {
        if (count != 0) {
	    Log.printLines(log.errWriter,
			   log.getLocalizedString("count." + kind,
						  Integer.toString(count)));
	    log.errWriter.flush();
	}
    }
}
