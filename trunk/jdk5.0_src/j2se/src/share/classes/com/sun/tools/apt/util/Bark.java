/**
 * @(#)Bark.java	1.2 04/05/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.apt.util;

import java.io.*;
import java.util.Set;
import java.util.HashSet;
import java.util.ResourceBundle;
import java.util.MissingResourceException;
import java.text.MessageFormat;
import com.sun.tools.javac.util.Log;
import com.sun.tools.javac.util.*;

/** A class for error logs. Reports errors and warnings, and
 *  keeps track of error numbers and positions.
 *
 *  <p><b>This is NOT part of any API supported by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Bark implements /* imports */ LayoutCharacters {
    /** The context key for the bark. */
    protected static final Context.Key<Bark> barkKey =
	new Context.Key<Bark>();

    /** The context key for the output PrintWriter. */
    public static final Context.Key<PrintWriter> outKey =
	new Context.Key<PrintWriter>();

    public final PrintWriter errWriter;
    public final PrintWriter warnWriter;
    public final PrintWriter noticeWriter;

    /** The maximum number of errors/warnings that are reported,
     *  can be reassigned from outside.
     */
    final protected int MaxErrors;
    final protected int MaxWarnings;

    /** Switch: prompt user on each error.
     */
    public boolean promptOnError;

    /** Switch: emit warning messages.
     */
    public boolean emitWarnings;

    /** Print stack trace on errors?
     */
    public boolean dumpOnError;


    /** Print multiple errors for same source locations.
     */
    public boolean multipleErrors;

    private Log log; // Companion log for this context

    /** Construct a bark with given I/O redirections.
     */
    protected Bark(Context context, PrintWriter errWriter, PrintWriter warnWriter, PrintWriter noticeWriter) {
	context.put(barkKey, this);
	this.errWriter = errWriter;
	this.warnWriter = warnWriter;
	this.noticeWriter = noticeWriter;

	Options options = Options.instance(context);
	this.log = Log.instance(context);
	this.dumpOnError = options.get("-doe") != null;
	this.promptOnError = options.get("-prompt") != null;
	this.emitWarnings = options.get("-nowarn") == null;
	this.multipleErrors = true;
	this.sourcename = Name.Table.instance(context).__input;
	
	this.MaxErrors = Integer.MAX_VALUE;
	this.MaxWarnings = Integer.MAX_VALUE;
    }
    // where
	private int getIntOption(Options options, String optionName, int defaultValue) {
	    String s = options.get(optionName);
	    try {
		if (s != null) return Integer.parseInt(s);
	    } catch (NumberFormatException e) {
		// silently ignore ill-formed numbers
	    }
	    return defaultValue;
	}

    /** The default writer for diagnostics
     */
    static final PrintWriter defaultWriter(Context context) {
	PrintWriter result = context.get(outKey);
	if (result == null)
	    context.put(outKey, result = new PrintWriter(System.err));
	return result;
    }

    /** Construct a bark with default settings; use Log's streams.
     */
    protected Bark(Context context) {
	this(context,
	     Log.instance(context).errWriter,
	     Log.instance(context).warnWriter,
	     Log.instance(context).noticeWriter);
    }

    /** Construct a bark with all output redirected.
     */
    protected Bark(Context context, PrintWriter defaultWriter) {
	this(context, defaultWriter, defaultWriter, defaultWriter);
    }

    /** Get the Bark instance for this context. */
    public static Bark instance(Context context) {
	Bark instance = context.get(barkKey);
	if (instance == null)
	    instance = new Bark(context);
	return instance;
    }

    /** The name of the file that's currently translated.
     */
    private Name sourcename;

    /** The number of errors encountered so far.
     */
    public int nerrors = 0;

    /** The number of warnings encountered so far.
     */
    public int nwarnings = 0;

    /** A set of all errors generated so far. This is used to avoid printing an
     *  error message more than once. For each error, a pair consisting of the
     *  source file name and source code position of the errir is added to the set.
     */
    private Set<Pair<Name,Integer>> recorded = new HashSet<Pair<Name,Integer>>();

    /** The buffer containing the file that's currently translated.
     */
    private byte[] buf = null;

    /** The position in the buffer at which last error was reported
     */
    private int bp;

    /** the source line at which last error was reported.
     */
    private int lastLine;

    /** Re-assign source name, returning previous setting.
     */
    public Name useSource(Name name) {
	Name prev = sourcename;
	sourcename = name;
	if (prev != sourcename) buf = null;
	return prev;
    }

    /** Return current source name.
     */
    public Name currentSource() {
	return sourcename;
    }

    /** Flush the barks
     */
    public void flush() {
	errWriter.flush();
	warnWriter.flush();
	noticeWriter.flush();
    }

    /** Prompt user after an error.
     */
    public void prompt() {
        if (promptOnError) {
            System.err.println(getLocalizedString("resume.abort"));
            char ch;
            try {
                while (true) {
                    switch (System.in.read()) {
                    case 'a': case 'A':
			System.exit(-1);
			return;
                    case 'r': case 'R':
                        return;
                    case 'x': case 'X':
                        throw new AssertionError("user abort");
                    default:
                    }
                }
            } catch (IOException e) {}
        }
    }

    /** Print a faulty source code line and point to the error.
     *  @param line    The line number of the printed line in the current buffer.
     *  @param col     The number of the column to be highlighted as error position.
     */
    private void printErrLine(int line, int col, PrintWriter writer) {
        try {
	    if (buf == null) {
		FileInputStream in = new FileInputStream(sourcename.toString());
		buf = new byte[in.available()];
		in.read(buf);
		in.close();
		bp = 0;
		lastLine = 1;
	    } else if (lastLine > line) {
		bp = 0;
		lastLine = 1;
	    }
	    while (bp < buf.length && lastLine < line) {
		switch (buf[bp]) {
		case CR:
		    bp++;
		    if (bp < buf.length && buf[bp] == LF) bp++;
		    lastLine++;
		    break;
		case LF:
		    bp++;
		    lastLine++;
		    break;
		default:
		    bp++;
		}
	    }
	    int lineEnd = bp;
	    while (lineEnd < buf.length &&
		   buf[lineEnd] != CR && buf[lineEnd] != LF) lineEnd++;
	    printLines(writer, new String(buf, bp, lineEnd - bp));
	    if (col == 0) col = 1;
	    byte[] ptr = new byte[col];
	    for (int i = 0; i < col - 1; i++) ptr[i] = (byte)' ';
	    ptr[col-1] = (byte)'^';
	    printLines(writer, new String(ptr, 0, col));
	} catch (IOException e) {
	    printLines(errWriter, getLocalizedString("source.unavailable"));
	} finally {
	    errWriter.flush();
	}
    }

    /** Print the text of a message, translating newlines appropriately
     *  for the platform.
     */
    public static void printLines(PrintWriter writer, String msg) {
	int nl;
	while ((nl = msg.indexOf('\n')) != -1) {
	    writer.println(msg.substring(0, nl));
	    msg = msg.substring(nl+1);
	}
	if (msg.length() != 0) writer.println(msg);
    }

    /** Print an error or warning message.
     *  @param pos    The source position at which to report the message.
     *  @param msg    The message to report.
     */
    protected void printDiagnostic(int pos, String msg, PrintWriter writer) {
	if (pos == Position.NOPOS) {
	    printLines(writer, msg);
	} else {
	    int line = Position.line(pos);
	    int col  = Position.column(pos);
	    printLines(writer, sourcename + ":" + line + ": " + msg);
	    printErrLine(line, col, writer);
	}
	if (dumpOnError)
	    new RuntimeException().printStackTrace(writer);
	writer.flush();
    }

    /** Report an error, unless another error was already reported at same
     *  source position.
     *  @param pos    The source position at which to report the error.
     *  @param key    The key for the localized error message.
     *  @param args   Fields of the error message.
     */
    public void error(int pos, String key, Object ... args) {
        if (nerrors < MaxErrors) {
	    Pair<Name,Integer> coords =
		new Pair<Name,Integer>(sourcename, pos);
		
	    if (multipleErrors || !recorded.contains(coords) ) {
		recorded.add(coords);
		printDiagnostic(pos,
				(((pos == Position.NOPOS) ?
				  getText("err.error") : "")
				 + getText("err." + key, args)),
				errWriter);
		prompt();
		nerrors++;
	    }
        }
    }

    /** Report a warning.
     *  @param pos    The source position at which to report the warning.
     *  @param key    The key for the localized warning message.
     *  @param args   Fields of the warning message.
     */
    public void warning(int pos, String key, Object ... args) {
        if (nwarnings < MaxWarnings && emitWarnings) {
	    printDiagnostic(pos,
			    (getText("warn.warning") +
			     getText("warn." + key, args)),
			    warnWriter);
	    prompt();
	    nwarnings++;
        }
    }

    /** Provide a non-fatal notification.
     *  @param key    The key for the localized notification message.
     *  @param args   Fields of the notification message.
     */
    public void note(String key, Object ... args) {
	// print out notes only when we are permitted to report warnings
	if (emitWarnings) {
	    noticeWriter.print(getText("note.note"));
	    String msg = getText("note." + key, args);
	    printLines(noticeWriter, msg);
	    noticeWriter.flush();
	}
    }

    /** Find a localized string in the resource bundle.
     *  @param pos    The source position at which to report the notice
     *  @param key    The key for the localized notification message.
     *  @param args   Fields of the notification message.
     */
    public void note(int pos, String key, Object ... args) {
	// print out notes only when we are permitted to report warnings
	if (emitWarnings) {
	    printDiagnostic(pos,
			    (getText("note.note") +
			     getText("note." + key, args)),
			    noticeWriter);
	}
    }

    /** Find a localized string in the resource bundle.
     *  @param key    The key for the localized string.
     *  @param args   Fields to substitute into the string.
     */
    public static String getLocalizedString(String key, Object ... args) {
	return getText("misc." + key, args);
    }

    /* ************************************************************************
     * Internationalization
     *************************************************************************/

    private static final String aptRB =
	"com.sun.tools.apt.resources.apt";

    private static final String compilerRB =
	"com.sun.tools.javac.resources.compiler";

    private static ResourceBundle messageRBapt;
    private static ResourceBundle messageRBcompiler;

    /** Initialize ResourceBundle.
     */
    private static void initResource() {
	try {
	    messageRBapt = ResourceBundle.getBundle(aptRB);
	    messageRBcompiler = ResourceBundle.getBundle(compilerRB);
	} catch (MissingResourceException e) {
	    throw new Error("Fatal: Resource for apt or compiler is missing");
	}
    }

    /** Get and format message string from resource.
     */
    public static String getText(String key, Object ... _args) {
	String[] args = new String[_args.length];
	for (int i=0; i<_args.length; i++)
	    args[i] = ""+_args[i];
	if (messageRBapt == null || messageRBcompiler == null)
	    initResource();
	try {
	    return MessageFormat.format(messageRBapt.getString("apt." + key), (Object[])args);
	} catch (MissingResourceException e1) {
	    try {
		return MessageFormat.format(messageRBcompiler.getString("compiler." + key), (Object[])args);
	    } catch (MissingResourceException e2) {
		String msg = "apt or compiler message file broken: key=" + key +
		    " arguments={0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}";
		return MessageFormat.format(msg, (Object[])args);
	    }
	}
    }

/***************************************************************************
 * raw error messages without internationalization; used for experimentation
 * and quick prototyping
 ***************************************************************************/

/** print an error or warning message:
 */
    private void printRawError(int pos, String msg) {
	if (pos == Position.NOPOS) {
	    printLines(errWriter, "error: " + msg);
	} else {
	    int line = Position.line(pos);
	    int col  = Position.column(pos);
	    printLines(errWriter, sourcename + ":" + line + ": " + msg);
	    printErrLine(line, col, errWriter);
	}
	errWriter.flush();
    }

/** report an error:
 */
    public void rawError(int pos, String msg) {
        if (nerrors < MaxErrors) {
	    Pair<Name,Integer> coords =
		new Pair<Name,Integer>(sourcename, pos);
	    if (multipleErrors || !recorded.contains(coords)) {
		recorded.add(coords);
		printRawError(pos, msg);
		prompt();
		nerrors++;
	    }
        }
	errWriter.flush();
    }

/** report a warning:
 */
    public void rawWarning(int pos, String msg) {
        if (nwarnings < MaxWarnings && emitWarnings) {
	    printRawError(pos, "warning: " + msg);
        }
	prompt();
	nwarnings++;
	errWriter.flush();
    }
}
