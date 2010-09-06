/**
 * @(#)Main.java	1.81 04/05/24
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.main;

import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.MessageFormat;
import java.util.ResourceBundle;
import java.util.MissingResourceException;
import java.util.StringTokenizer;

import com.sun.tools.javac.code.Source;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.jvm.Target;
import com.sun.tools.javac.util.*;

/** This class provides a commandline interface to the GJC compiler.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Main {

    /** The name of the compiler, for use in diagnostics.
     */
    String ownName;

    /** The writer to use for diagnostic output.
     */
    PrintWriter out;

    /** Result codes.
     */
    static final int
        EXIT_OK = 0,        // Compilation completed with no errors.
	EXIT_ERROR = 1,     // Completed but reported errors.
	EXIT_CMDERR = 2,    // Bad command-line arguments
	EXIT_SYSERR = 3,    // System error or resource exhaustion.
	EXIT_ABNORMAL = 4;  // Compiler terminated abnormally

    /** This class represents an option recognized by the main program
     */
    private class Option {

	/** Option string.
	 */
	String name;

	/** Documentation key for arguments.
	 */
	String argsNameKey;

	/** Documentation key for description.
	 */
	String descrKey;

	/** Suffix option (-foo=bar or -foo:bar)
	 */
	boolean hasSuffix;

	Option(String name, String argsNameKey, String descrKey) {
	    this.name = name;
	    this.argsNameKey = argsNameKey;
	    this.descrKey = descrKey;
	    char lastChar = name.charAt(name.length()-1);
	    hasSuffix = lastChar == ':' || lastChar == '=';
	}
	Option(String name, String descrKey) {
	    this(name, null, descrKey);
	}

	public String toString() {
	    return name;
	}

	/** Does this option take a (separate) operand?
	 */
	boolean hasArg() {
	    return argsNameKey != null && !hasSuffix;
	}

	/** Does argument string match option pattern?
	 *  @param arg        The command line argument string.
	 */
        boolean matches(String arg) {
	    return hasSuffix ? arg.startsWith(name) : arg.equals(name);
	}

	/** Print a line of documentation describing this option, if standard.
	 */
	void help() {
	    String s = "  " + helpSynopsis();
	    out.print(s);
	    for (int j = s.length(); j < 29; j++) out.print(" ");
	    Log.printLines(out, getLocalizedString(descrKey));
	}
	String helpSynopsis() {
	    return name +
		(argsNameKey == null ? "" :
		 ((hasSuffix ? "" : " ") +
		  getLocalizedString(argsNameKey)));
	}

	/** Print a line of documentation describing this option, if non-standard.
	 */
	void xhelp() {}

	/** Process the option (with arg). Return true if error detected.
	 */
	boolean process(String option, String arg) {
	    options.put(option, arg);
	    return false;
	}

	/** Process the option (without arg). Return true if error detected.
	 */
	boolean process(String option) {
	    if (hasSuffix)
		return process(name, option.substring(name.length()));
	    else
		return process(option, option);
	}
    };

    /** A nonstandard or extended (-X) option
     */
    private class XOption extends Option {
	XOption(String name, String argsNameKey, String descrKey) {
	    super(name, argsNameKey, descrKey);
	}
	XOption(String name, String descrKey) {
	    this(name, null, descrKey);
	}
	void help() {}
	void xhelp() { super.help(); }
    };

    /** A hidden (implementor) option
     */
    private class HiddenOption extends Option {
	HiddenOption(String name) {
	    super(name, null, null);
	}
	HiddenOption(String name, String argsNameKey) {
	    super(name, argsNameKey, null);
	}
	void help() {}
	void xhelp() {}
    };

    private Option[] recognizedOptions = {
	new Option("-g",					"opt.g"),
	new Option("-g:none",					"opt.g.none") {
	    boolean process(String option) {
		options.put("-g:", "none");
		return false;
	    }
	},

	new Option("-g:{lines,vars,source}",			"opt.g.lines.vars.source") {
	    boolean matches(String s) {
		return s.startsWith("-g:");
	    }
	    boolean process(String option) {
		String suboptions = option.substring(3);
		options.put("-g:", suboptions);
		// enter all the -g suboptions as "-g:suboption"
		for (StringTokenizer t = new StringTokenizer(suboptions, ","); t.hasMoreTokens(); ) {
		    String tok = t.nextToken();
		    String opt = "-g:" + tok;
		    options.put(opt, opt);
		}
		return false;
	    }
	},

	new XOption("-Xlint",					"opt.Xlint"),
	new XOption("-Xlint:{all,deprecation,unchecked,fallthrough,path,serial,finally,-deprecation,-unchecked,-fallthrough,-path,-serial,-finally}",
		    						"opt.Xlint.suboptlist") {
	    boolean matches(String s) {
		return s.startsWith("-Xlint:");
	    }
	    boolean process(String option) {
		String suboptions = option.substring(7);
		options.put("-Xlint:", suboptions);
		// enter all the -Xlint suboptions as "-Xlint:suboption"
		for (StringTokenizer t = new StringTokenizer(suboptions, ","); t.hasMoreTokens(); ) {
		    String tok = t.nextToken();
		    String opt = "-Xlint:" + tok;
		    options.put(opt, opt);
		}
		return false;
	    }
	},

	new Option("-nowarn",					"opt.nowarn"),

	new Option("-verbose",					"opt.verbose"),

	// -deprecation is retained for command-line backward compatibility
	new Option("-deprecation",				"opt.deprecation") {
		boolean process(String option) {
		    options.put("-Xlint:deprecation", option);
		    return false;
		}
	    },

	new Option("-classpath",	   "opt.arg.path",	"opt.classpath"),
	new Option("-cp",		   "opt.arg.path",	"opt.classpath") {
	    boolean process(String option, String arg) {
		return super.process("-classpath", arg);
	    }
	},
	new Option("-sourcepath",	   "opt.arg.path",	"opt.sourcepath"),
	new Option("-bootclasspath",	   "opt.arg.path",	"opt.bootclasspath") {
	    boolean process(String option, String arg) {
	 	options.remove("-Xbootclasspath/p:");
	 	options.remove("-Xbootclasspath/a:");
		return super.process(option, arg);
	    }
	},
	new XOption("-Xbootclasspath/p:",  "opt.arg.path", "opt.Xbootclasspath.p"),
	new XOption("-Xbootclasspath/a:",  "opt.arg.path", "opt.Xbootclasspath.a"),
	new XOption("-Xbootclasspath:",    "opt.arg.path", "opt.bootclasspath") {
	    boolean process(String option, String arg) {
	 	options.remove("-Xbootclasspath/p:");
	 	options.remove("-Xbootclasspath/a:");
	 	return super.process("-bootclasspath", arg);
	    }
	},
	new Option("-extdirs",		   "opt.arg.dirs",	"opt.extdirs"),
	new XOption("-Djava.ext.dirs=",	   "opt.arg.dirs",	"opt.extdirs") {
	    boolean process(String option, String arg) {
	 	return super.process("-extdirs", arg);
	    }
	},
	new Option("-endorseddirs",	   "opt.arg.dirs",	"opt.endorseddirs"),
	new XOption("-Djava.endorsed.dirs=","opt.arg.dirs",	"opt.endorseddirs") {
	    boolean process(String option, String arg) {
	 	return super.process("-endorseddirs", arg);
	    }
	},
	new Option("-d",		"opt.arg.directory",	"opt.d"),
	new Option("-encoding",		"opt.arg.encoding",	"opt.encoding"),
	new Option("-source",		"opt.arg.release",	"opt.source") {
	    boolean process(String option, String operand) {
		Source source = Source.lookup(operand);
		if (source == null) {
		    error("err.invalid.source", operand);
		    return true;
		}
		return super.process(option, operand);
	    }
	},
	new Option("-target",		"opt.arg.release",	"opt.target") {
	    boolean process(String option, String operand) {
		Target target = Target.lookup(operand);
		if (target == null) {
		    error("err.invalid.target", operand);
		    return true;
		}
		return super.process(option, operand);
	    }
	},
	new Option("-version",					"opt.version") {
	    boolean process(String option) {
		Log.printLines(out, ownName + " " + JavaCompiler.version());
		return super.process(option);
	    }
	},
	new Option("-help",					"opt.help") {
	    boolean process(String option) {
		Main.this.help();
		return super.process(option);
	    }
	},
	new Option("-X",					"opt.X") {
	    boolean process(String option) {
		Main.this.xhelp();
		return super.process(option);
	    }
	},

	// This option exists only for the purpose of documenting itself.
	// It's actually implemented by the launcher.
	new Option("-J",		"opt.arg.flag",		"opt.J") {
	    String helpSynopsis() {
		hasSuffix = true;
		return super.helpSynopsis();
	    }
	    boolean process(String option) {
		throw new AssertionError
		    ("the -J flag should be caught by the launcher.");
	    }
	},

	// stop after parsing and attributing.
	// new HiddenOption("-attrparseonly"),

	// new Option("-moreinfo",					"opt.moreinfo") {
	new HiddenOption("-moreinfo") {
	    boolean process(String option) {
		Type.moreInfo = true;
		return super.process(option);
	    }
	},

	// treat warnings as errors
	new HiddenOption("-Werror"),

	// use complex inference from context in the position of a method call argument
	new HiddenOption("-complexinference"),

	// generare source stubs
	// new HiddenOption("-stubs"),

	// relax some constraints to allow compiling from stubs
	// new HiddenOption("-relax"),

	// output source after translating away inner classes
	// new Option("-printflat",				"opt.printflat"),
	// new HiddenOption("-printflat"),

	// display scope search details
	// new Option("-printsearch",				"opt.printsearch"),
	// new HiddenOption("-printsearch"),

	// prompt after each error
	// new Option("-prompt",					"opt.prompt"),
	new HiddenOption("-prompt"),

	// dump stack on error
	new HiddenOption("-doe"),

	// output source after type erasure
	// new Option("-s",					"opt.s"),
	// new HiddenOption("-s"),

	// output shrouded class files
	// new Option("-scramble",				"opt.scramble"),
	// new Option("-scrambleall",				"opt.scrambleall"),

	// display warnings for generic unchecked operations
	new HiddenOption("-warnunchecked") {
	    boolean process(String option) {
		options.put("-Xlint:unchecked", option);
		return false;
	    }
	},

	new XOption("-Xmaxerrs",	"opt.arg.number",	"opt.maxerrs"),
	new XOption("-Xmaxwarns",	"opt.arg.number",	"opt.maxwarns"),
	new XOption("-Xstdout",		"opt.arg.file",		"opt.Xstdout") {
	    boolean process(String option, String arg) {
		try {
		    out = new PrintWriter(new FileWriter(arg), true);
		} catch (java.io.IOException e) {
		    error("err.error.writing.file", arg, e);
		    return true;
		}
		return super.process(option, arg);
	    }
	},

	/* -O is a no-op, accepted for backward compatibility. */
	new HiddenOption("-O"),

	/* -Xjcov produces tables to support the code coverage tool jcov. */
	new HiddenOption("-Xjcov"),

	/* This is a back door to the compiler's option table.
	 * -Dx=y sets the option x to the value y.
	 * -Dx sets the option x to the value x.
	 */
	new HiddenOption("-XD") {
	    String s;
	    boolean matches(String s) {
		this.s = s;
		return s.startsWith(name);
	    }
	    boolean process(String option) {
		s = s.substring(name.length());
		int eq = s.indexOf('=');
		String key = (eq < 0) ? s : s.substring(0, eq);
		String value = (eq < 0) ? s : s.substring(eq+1);
		options.put(key, value);
		return false;
	    }
	},

	new HiddenOption("sourcefile") {
	    String s;
	    boolean matches(String s) {
		this.s = s;
		return s.endsWith(".java");
	    }
	    boolean process(String option) {
		if (!filenames.contains(s))
		    filenames.append(s);
		return false;
	    }
	},
    };

    /**
     * Construct a compiler instance.
     */
    public Main(String name) {
	this(name, new PrintWriter(System.err, true));
    }

    /**
     * Construct a compiler instance.
     */
    public Main(String name, PrintWriter out) {
	this.ownName = name;
	this.out = out;
    }

    /** A table of all options that's passed to the JavaCompiler constructor.  */
    private Options options = null;

    /** The list of files to process
     */
    ListBuffer<String> filenames = null;

    /** Print a string that explains usage.
     */
    void help() {
	Log.printLines(out, getLocalizedString("msg.usage.header", ownName));
	for (int i=0; i<recognizedOptions.length; i++) {
	    recognizedOptions[i].help();
	}
	out.println();
    }

    /** Print a string that explains usage for X options.
     */
    void xhelp() {
	for (int i=0; i<recognizedOptions.length; i++) {
	    recognizedOptions[i].xhelp();
	}
	out.println();
	Log.printLines(out, getLocalizedString("msg.usage.nonstandard.footer"));
    }

    /** Report a usage error.
     */
    void error(String key, Object... args) {
	warning(key, args);
	help();
    }

    /** Report a warning.
     */
    void warning(String key, Object... args) {
	Log.printLines(out, ownName + ": "
		       + getLocalizedString(key, args));
    }

    /** Process command line arguments: store all command line options
     *  in `options' table and return all source filenames.
     *  @param args    The array of command line arguments.
     */
    protected List<String> processArgs(String[] flags) {
	int ac = 0;
	while (ac < flags.length) {
	    String flag = flags[ac];
	    ac++;

	    int j;
	    for (j=0; j<recognizedOptions.length; j++)
		if (recognizedOptions[j].matches(flag)) break;

	    if (j == recognizedOptions.length) {
		error("err.invalid.flag", flag);
		return null;
	    }

	    Option option = recognizedOptions[j];
	    if (option.hasArg()) {
		if (ac == flags.length) {
		    error("err.req.arg", flag);
		    return null;
		}
		String operand = flags[ac];
		ac++;
		if (option.process(flag, operand)) return null;
	    } else {
		if (option.process(flag)) return null;
	    }
	}

	String sourceString = options.get("-source");
	Source source = (sourceString != null)
	    ? Source.lookup(sourceString)
	    : Source.DEFAULT;
	String targetString = options.get("-target");
	Target target = (targetString != null)
	    ? Target.lookup(targetString)
	    : Target.DEFAULT;
	// We don't check source/target consistency for CLDC, as J2ME
	// profiles are not aligned with J2SE targets; moreover, a
	// single CLDC target may have many profiles.  In addition,
	// this is needed for the continued functioning of the JSR14
	// prototype.
	if (Character.isDigit(target.name.charAt(0)) &&
	    target.compareTo(source.requiredTarget()) < 0) {
            if (targetString != null) {
                if (sourceString == null) {
                    warning("warn.target.default.source.conflict",
                            targetString,
                            source.requiredTarget().name);
                } else {
                    warning("warn.source.target.conflict",
                            sourceString,
                            source.requiredTarget().name);
                }
                return null;
	    } else {
                options.put("-target", source.requiredTarget().name);
	    }
        }
	return filenames.toList();
    }

    /** Programmatic interface for main function.
     * @param args    The command line parameters.
     */
    public int compile(String[] args) {
	return compile(args, new Context());
    }

    /** Programmatic interface for main function.
     * @param args    The command line parameters.
     */
    public int compile(String[] args, Context context) {
	boolean assertionsEnabled = false;
	assert assertionsEnabled = true;
	if (!assertionsEnabled) {
	    // Log.printLines(out, "fatal error: assertions must be enabled when running javac");
	    // return EXIT_ABNORMAL;
	}

	options = Options.instance(context); // creates a new one

	filenames = new ListBuffer<String>();
	JavaCompiler comp = null;
	try {
	    if (args.length == 0) {
		help();
		return EXIT_CMDERR;
	    }

	    List<String> filenames;
	    try {
		filenames = processArgs(CommandLine.parse(args));
		if (filenames == null) {
		    return EXIT_CMDERR;
		} else if (filenames.isEmpty()) {
		    // it is allowed to compile nothing if just asking for help
		    if (options.get("-help") != null ||
			options.get("-X") != null)
			return EXIT_OK;
		    error("err.no.source.files");
		    return EXIT_CMDERR;
		}
	    } catch (java.io.FileNotFoundException e) {
		Log.printLines(out, ownName + ": " +
			       getLocalizedString("err.file.not.found",
						  e.getMessage()));
		return EXIT_SYSERR;
	    }

	    context.put(Log.outKey, out);
	    comp = JavaCompiler.instance(context);
	    if (comp == null) return EXIT_SYSERR;

	    List<Symbol.ClassSymbol> cs = comp.compile(filenames);

	    if (comp.errorCount() != 0 ||
		options.get("-Werror") != null && comp.warningCount() != 0)
		return EXIT_ERROR;
	} catch (IOException ex) {
	    ioMessage(ex);
	    return EXIT_SYSERR;
	} catch (OutOfMemoryError ex) {
	    resourceMessage(ex);
	    return EXIT_SYSERR;
	} catch (StackOverflowError ex) {
	    resourceMessage(ex);
	    return EXIT_SYSERR;
	} catch (FatalError ex) {
	    feMessage(ex);
	    return EXIT_SYSERR;
	} catch (Throwable ex) {
            // Nasty.  If we've already reported an error, compensate
            // for buggy compiler error recovery by swallowing thrown
            // exceptions.
	    if (comp.errorCount() == 0 || options.get("dev") != null)
                bugMessage(ex);
	    return EXIT_ABNORMAL;
	} finally {
	    if (comp != null) comp.close();
	    filenames = null;
	    options = null;
	}
	return EXIT_OK;
    }

    /** Print a message reporting an internal error.
     */
    void bugMessage(Throwable ex) {
	Log.printLines(out, getLocalizedString("msg.bug",
					       JavaCompiler.version()));
	ex.printStackTrace(out);
    }

    /** Print a message reporting an fatal error.
     */
    void feMessage(Throwable ex) {
	Log.printLines(out, ex.getMessage());
    }

    /** Print a message reporting an input/output error.
     */
    void ioMessage(Throwable ex) {
	Log.printLines(out, getLocalizedString("msg.io"));
	ex.printStackTrace(out);
    }

    /** Print a message reporting an out-of-resources error.
     */
    void resourceMessage(Throwable ex) {
	Log.printLines(out, getLocalizedString("msg.resource"));
//	System.out.println("(name buffer len = " + Name.names.length + " " + Name.nc);//DEBUG
	ex.printStackTrace(out);
    }

    /* ************************************************************************
     * Internationalization
     *************************************************************************/

    /** Find a localized string in the resource bundle.
     *  @param key     The key for the localized string.
     */
    private static String getLocalizedString(String key, Object... args) {
	return getText("javac." + key, args);
    }

    private static final String javacRB =
	"com.sun.tools.javac.resources.javac";

    private static ResourceBundle messageRB;

    /** Initialize ResourceBundle.
     */
    private static void initResource() {
	try {
	    messageRB = ResourceBundle.getBundle(javacRB);
	} catch (MissingResourceException e) {
	    Error x = new FatalError("Fatal Error: Resource for javac is missing");
	    x.initCause(e);
	    throw x;
	}
    }

    /** Get and format message string from resource.
     */
    private static String getText(String key, Object... _args) {
	String[] args = new String[_args.length];
	for (int i=0; i<_args.length; i++) {
	    args[i] = "" + _args[i];
	}
	if (messageRB == null)
	    initResource();
	try {
	    return MessageFormat.format(messageRB.getString(key), (Object[])args);
	} catch (MissingResourceException e) {
	    String msg = "javac message file broken: key={0} "
		+ "arguments={1}, {2}";
	    return MessageFormat.format(msg, (Object[])args);
	}
    }
}
