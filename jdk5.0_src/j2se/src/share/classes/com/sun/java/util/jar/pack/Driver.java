/*
 * @(#)Driver.java	1.22 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.util.jar.pack;

import java.lang.Error;
import java.io.*;
import java.util.*;
import java.util.jar.*;
import java.util.zip.*;

/** Command line interface for Pack200.
 */
class Driver {
    
    public static void main(String[] ava) throws IOException {
	ArrayList<String> av = new ArrayList<String>(Arrays.asList(ava));

	boolean doPack   = true;
	boolean doUnpack = false;
	boolean doRepack = false;
	boolean doForceRepack = false;
	boolean doZip = true;
	String logFile = null;
	String verboseProp = Utils.DEBUG_VERBOSE;

	{
	    // Non-standard, undocumented "--unpack" switch enables unpack mode.
	    String arg0 = av.isEmpty() ? "" : av.get(0);
	    if (arg0.equals("--pack")) {
		av.remove(0);
	    } else if (arg0.equals("--unpack")) {
		av.remove(0);
		doPack = false;
		doUnpack = true;
	    }
	}

	// Collect engine properties here:
	HashMap<String,String> engProps = new HashMap<String,String>();
	engProps.put(verboseProp, System.getProperty(verboseProp));

	String optionMap;
	String[] propTable;
	if (doPack) {
	    optionMap = PACK200_OPTION_MAP;
	    propTable = PACK200_PROPERTY_TO_OPTION;
	} else {
	    optionMap = UNPACK200_OPTION_MAP;
	    propTable = UNPACK200_PROPERTY_TO_OPTION;
	}

	// Collect argument properties here:
	HashMap<String,String> avProps = new HashMap<String,String>();
	try {
	    for (;;) {
		String state = parseCommandOptions(av, optionMap, avProps);
		// Translate command line options to Pack200 properties:
	    eachOpt:
		for (Iterator<String> opti = avProps.keySet().iterator();
		     opti.hasNext(); ) {
		    String opt = opti.next();
		    String prop = null;
		    for (int i = 0; i < propTable.length; i += 2) {
			if (opt.equals(propTable[1+i])) {
			    prop = propTable[0+i];
			    break;
			}
		    }
		    if (prop != null) {
			String val = avProps.get(opt);
			opti.remove();  // remove opt from avProps
			if (!prop.endsWith(".")) {
			    // Normal string or boolean.
			    if (!(opt.equals("--verbose")
				  || opt.endsWith("="))) {
				// Normal boolean; convert to T/F.
				boolean flag = (val != null);
				if (opt.startsWith("--no-"))
				    flag = !flag;
				val = flag? "true": "false";
			    }
			    engProps.put(prop, val);
			} else if (prop.contains(".attribute.")) {
			    for (String val1 : val.split("\0")) {
				String[] val2 = val1.split("=", 2);
				engProps.put(prop+val2[0], val2[1]);
			    }
			} else {
			    // Collection property: pack.pass.file.cli.NNN
			    int idx = 1;
			    for (String val1 : val.split("\0")) {
				String prop1;
				do {
				    prop1 = prop+"cli."+(idx++);
				} while (engProps.containsKey(prop1));
				engProps.put(prop1, val1);
			    }
			}
		    }
		}

		// See if there is any other action to take.
		if (state == "--config-file=") {
		    String propFile = av.remove(0);
		    InputStream propIn = new FileInputStream(propFile);
		    Properties fileProps = new Properties();
		    fileProps.load(new BufferedInputStream(propIn));
		    if (engProps.get(verboseProp) != null)
			fileProps.list(System.out);
		    propIn.close();
		    for (Map.Entry<Object,Object> me : fileProps.entrySet())
			engProps.put((String)me.getKey(), (String)me.getValue());
		} else if (state == "--version") {
		    System.out.println(Driver.class.getName()+" version 1.22, 04/05/05");
		    return;
		} else if (state == "--help") {
		    printUsage(doPack, true, System.out);
		    System.exit(1);
		    return;
		} else {
		    break;
		}
	    }
	} catch (IllegalArgumentException ee) {
	    System.err.println("Bad argument:  "+ee);
	    printUsage(doPack, false, System.err);
	    System.exit(2);
	    return;
	}

	// Deal with remaining non-engine properties:
	for (String opt : avProps.keySet()) {
	    String val = avProps.get(opt);
	    if (opt == "--repack") {
		doRepack = true;
	    } else if (opt == "--no-gzip") {
		doZip = (val == null);
	    } else if (opt == "--log-file=") {
		logFile = val;
	    } else {
		throw new InternalError("Bad option: "
					+opt+"="+avProps.get(opt));
	    }
	}

	if (logFile != null && !logFile.equals("")) {
	    if (logFile.equals("-")) {
		System.setErr(System.out);
	    } else {
		OutputStream log = new FileOutputStream(logFile);
		//log = new BufferedOutputStream(out);
		System.setErr(new PrintStream(log));
	    }
	}

	boolean verbose = (engProps.get(verboseProp) != null);

	String packfile = "";
	if (!av.isEmpty())
	    packfile = av.remove(0);

	String jarfile = "";
	if (!av.isEmpty())
	    jarfile = av.remove(0);

	String newfile = "";  // output JAR file if --repack
	String bakfile = "";  // temporary backup of input JAR
	String tmpfile = "";  // temporary file to be deleted
	if (doRepack) {
	    // The first argument is the target JAR file.
	    // (Note:  *.pac is nonstandard, but may be necessary
	    // if a host OS truncates file extensions.)
	    if (packfile.toLowerCase().endsWith(".pack") ||
		packfile.toLowerCase().endsWith(".pac") ||
		packfile.toLowerCase().endsWith(".gz")) {
		System.err.println("Bad --repack output: "+packfile);
		printUsage(doPack, false, System.err);
		System.exit(2);
	    }
	    newfile = packfile;
	    // The optional second argument is the source JAR file.
	    if (jarfile.equals("")) {
		// If only one file is given, it is the only JAR.
		// It serves as both input and output.
		jarfile = newfile;
	    }
	    tmpfile = createTempFile(newfile, ".pack").getPath();
	    packfile = tmpfile;
	    doZip = false;  // no need to zip the temporary file
	}

	if (!av.isEmpty()
	    // Accept jarfiles ending with .jar or .zip.
	    // Accept jarfile of "-" (stdout), but only if unpacking.
	    || !(jarfile.toLowerCase().endsWith(".jar")
		 || jarfile.toLowerCase().endsWith(".zip")
		 || (jarfile.equals("-") && !doPack))) {
	    printUsage(doPack, false, System.err);
	    System.exit(2);
	    return;
	}

	if (doRepack)
	    doPack = doUnpack = true;
	else if (doPack)
	    doUnpack = false;

	Pack200.Packer jpack = Pack200.newPacker();
	Pack200.Unpacker junpack = Pack200.newUnpacker();

	jpack.properties().putAll(engProps);
	junpack.properties().putAll(engProps);
	if (doRepack && newfile.equals(jarfile)) {
	    String zipc = getZipComment(jarfile);
	    if (verbose && zipc.length() > 0)
		System.out.println("Detected ZIP comment: "+zipc);
	    if (zipc.indexOf(Utils.PACK_ZIP_ARCHIVE_MARKER_COMMENT) >= 0) {
		System.out.println("Skipping because already repacked: "
				       +jarfile);
		doPack = false;
		doUnpack = false;
		doRepack = false;
	    }
	}

	try {
	    
	    if (doPack) {
		// Mode = Pack.
		JarFile in = new JarFile(new File(jarfile));
		OutputStream out;
		// Packfile must be -, *.gz, *.pack, or *.pac.
		if (packfile.equals("-")) {
		    out = System.out;
		    // Send warnings, etc., to stderr instead of stdout.
		    System.setOut(System.err);
		} else if (doZip) {
		    if (!packfile.endsWith(".gz")) {
			System.err.println("To write a *.pack file, specify --no-gzip: "+packfile);
			printUsage(doPack, false, System.err);
			System.exit(2);
		    }
		    out = new FileOutputStream(packfile);
		    out = new BufferedOutputStream(out);
		    out = new GZIPOutputStream(out);
		} else {
		    if (!packfile.toLowerCase().endsWith(".pack") &&
			!packfile.toLowerCase().endsWith(".pac")) {
			System.err.println("To write a *.pack.gz file, specify --gzip: "+packfile);
			printUsage(doPack, false, System.err);
			System.exit(2);
		    }
		    out = new FileOutputStream(packfile);
		    out = new BufferedOutputStream(out);
		}
		jpack.pack(in, out);
		//in.close();  // p200 closes in but not out
		out.close();
	    }

	    if (doRepack && newfile.equals(jarfile)) {
		// If the source and destination are the same,
		// we will move the input JAR aside while regenerating it.
		// This allows us to restore it if something goes wrong.
		File bakf = createTempFile(jarfile, ".bak");
		// On Windows target must be deleted see 4017593 
		bakf.delete(); 
		boolean okBackup = new File(jarfile).renameTo(bakf);
		if (!okBackup) {
		    throw new Error("Skipping unpack because move failed: "
				    +bakfile);
		} else {
		    // Open jarfile recovery bracket.
		    bakfile = bakf.getPath();
		}
	    }

	    if (doUnpack) {
		// Mode = Unpack.
		InputStream in;
		if (packfile.equals("-"))
		    in = System.in;
		else
		    in = new FileInputStream(new File(packfile));
		BufferedInputStream inBuf = new BufferedInputStream(in);
		in = inBuf;
		if (Utils.isGZIPMagic(Utils.readMagic(inBuf))) {
		    in = new GZIPInputStream(in);
		}
		String outfile = newfile.equals("")? jarfile: newfile;
		OutputStream fileOut;
		if (outfile.equals("-"))
		    fileOut = System.out;
		else
		    fileOut = new FileOutputStream(outfile);
		fileOut = new BufferedOutputStream(fileOut);
		JarOutputStream out = new JarOutputStream(fileOut);
		junpack.unpack(in, out);
		//in.close();  // p200 closes in but not out
		out.close();
		// At this point, we have a good jarfile (or newfile, if -r)
	    }

	    if (!bakfile.equals("")) {
		// On success, abort jarfile recovery bracket.
		new File(bakfile).delete();
		bakfile = "";
	    }

	} finally {
	    // Close jarfile recovery bracket.
	    if (!bakfile.equals("")) {
		File jarFile = new File(jarfile);
		jarFile.delete(); // Win32 requires this, see above
		new File(bakfile).renameTo(jarFile);
	    }
	    // In all cases, delete temporary *.pack.
	    if (!tmpfile.equals(""))
		new File(tmpfile).delete();
	}
    }

    static private
    File createTempFile(String basefile, String suffix) throws IOException {
	File base = new File(basefile);
	String prefix = base.getName();
	if (prefix.length() < 3)  prefix += "tmp";

	File where = base.getParentFile();

	if ( base.getParentFile() == null && suffix.equals(".bak"))
	    where = new File(".").getAbsoluteFile();

    
	File f = File.createTempFile(prefix, suffix, where); 
	return f;
    }

    static private
    void printUsage(boolean doPack, boolean full, PrintStream out) {
	String prog = doPack ? "pack200" : "unpack200";
	String[] packUsage = {
	    "Usage:  "+prog+" [-opt... | --option=value]... x.pack[.gz] y.jar",
	    "",
	    "Packing Options",
	    "  -g, --no-gzip                   output a plain *.pack file with no zipping",
	    "  --gzip                          (default) post-process the pack output with gzip",
	    "  -G, --strip-debug               remove debugging attributes while packing",
	    "  -O, --no-keep-file-order        do not transmit file ordering information",
	    "  --keep-file-order               (default) preserve input file ordering",
	    "  -S{N}, --segment-limit={N}      output segment limit (default N=1Mb)",
	    "  -E{N}, --effort={N}             packing effort (default N=5)",
	    "  -H{h}, --deflate-hint={h}       transmit deflate hint: true, false, or keep (default)",
	    "  -m{V}, --modification-time={V}  transmit modtimes: latest or keep (default)",
	    "  -P{F}, --pass-file={F}          transmit the given input element(s) uncompressed",
	    "  -U{a}, --unknown-attribute={a}  unknown attribute action: error, strip, or pass (default)",
	    "  -C{N}={L}, --class-attribute={N}={L}  (user-defined attribute)",
	    "  -F{N}={L}, --field-attribute={N}={L}  (user-defined attribute)",
	    "  -M{N}={L}, --method-attribute={N}={L} (user-defined attribute)",
	    "  -D{N}={L}, --code-attribute={N}={L}   (user-defined attribute)",
	    "  -f{F}, --config-file={F}        read file F for Pack200.Packer properties",
	    "  -v, --verbose                   increase program verbosity",
	    "  -q, --quiet                     set verbosity to lowest level",
	    "  -l{F}, --log-file={F}           output to the given log file, or '-' for System.out",
	    "  -?, -h, --help                  print this message",
	    "  -V, --version                   print program version",
	    "  -J{X}                           pass option X to underlying Java VM",
	    "",
	    "Notes:",
	    "  The -P, -C, -F, -M, and -D options accumulate.",
	    "  Example attribute definition:  -C SourceFile=RUH .",
	    "  Config. file properties are defined by the Pack200 API.",
	    "  For meaning of -S, -E, -H-, -m, -U values, see Pack200 API.",
	    "  Layout definitions (like RUH) are defined by JSR 200.",
	    "",
	    "Repacking mode updates the JAR file with a pack/unpack cycle:",
	    "    "+prog+" [-r|--repack] [-opt | --option=value]... [repackedy.jar] y.jar",
	};

	String[] unpackUsage = {
	    "Usage:  "+prog+" [-opt... | --option=value]... x.pack[.gz] y.jar",
	    "",
	    "Unpacking Options",
	    "  -H{h}, --deflate-hint={h}     override transmitted deflate hint: true, false, or keep (default)",
	    "  -r, --remove-pack-file        remove input file after unpacking",
	    "  -v, --verbose                 increase program verbosity",
	    "  -q, --quiet                   set verbosity to lowest level",
	    "  -l{F}, --log-file={F}         output to the given log file, or '-' for System.out",
	    "  -?, -h, --help                print this message",
	    "  -V, --version                 print program version",
	    "  -J{X}                         pass option X to underlying Java VM",
	};
	String[] usage = doPack? packUsage: unpackUsage;
	for (int i = 0; i < usage.length; i++) {
	    out.println(usage[i]);
	    if (!full) {
		out.println("(For more information, run "+prog+" --help .)");
		break;
	    }
	}
    }
    
    static private
	String getZipComment(String jarfile) throws IOException {
	byte[] tail = new byte[1000];
	long filelen = new File(jarfile).length();
	if (filelen <= 0)  return "";
	long skiplen = Math.max(0, filelen - tail.length);
	InputStream in = new FileInputStream(new File(jarfile));
	try {
	    in.skip(skiplen);
	    in.read(tail);
	    for (int i = tail.length-4; i >= 0; i--) {
		if (tail[i+0] == 'P' && tail[i+1] == 'K' &&
		    tail[i+2] ==  5  && tail[i+3] ==  6) {
		    // Skip sig4, disks4, entries4, clen4, coff4, cmt2
		    i += 4+4+4+4+4+2;
		    if (i < tail.length) 
			return new String(tail, i, tail.length-i, "UTF8");
		    return "";
		}
	    }
	    return "";
        } finally {
	    in.close();
	}
    }

    private static final String PACK200_OPTION_MAP =
	(""
	 +"--repack                 $ \n  -r +>- @--repack              $ \n"
	 +"--no-gzip                $ \n  -g +>- @--no-gzip             $ \n"
	 +"--strip-debug            $ \n  -G +>- @--strip-debug         $ \n"
	 +"--no-keep-file-order     $ \n  -O +>- @--no-keep-file-order  $ \n"
	 +"--segment-limit=      *> = \n  -S +>  @--segment-limit=      = \n"
	 +"--effort=             *> = \n  -E +>  @--effort=             = \n"
	 +"--deflate-hint=       *> = \n  -H +>  @--deflate-hint=       = \n"
	 +"--modification-time=  *> = \n  -m +>  @--modification-time=  = \n"
	 +"--pass-file=        *> &\0 \n  -P +>  @--pass-file=        &\0 \n"
	 +"--unknown-attribute=  *> = \n  -U +>  @--unknown-attribute=  = \n"
	 +"--class-attribute=  *> &\0 \n  -C +>  @--class-attribute=  &\0 \n"
	 +"--field-attribute=  *> &\0 \n  -F +>  @--field-attribute=  &\0 \n"
	 +"--method-attribute= *> &\0 \n  -M +>  @--method-attribute= &\0 \n"
	 +"--code-attribute=   *> &\0 \n  -D +>  @--code-attribute=   &\0 \n"
	 +"--config-file=      *>   . \n  -f +>  @--config-file=        . \n"

	 // Negative options as required by CLIP:
	 +"--no-strip-debug  !--strip-debug         \n"
	 +"--gzip            !--no-gzip             \n"
	 +"--keep-file-order !--no-keep-file-order  \n"

	 // Non-Standard Options
	 +"--verbose                $ \n  -v +>- @--verbose             $ \n"
	 +"--quiet        !--verbose  \n  -q +>- !--verbose               \n"
	 +"--log-file=           *> = \n  -l +>  @--log-file=           = \n"
	 //+"--java-option=      *> = \n  -J +>  @--java-option=        = \n"
	 +"--version                . \n  -V +>  @--version             . \n"
	 +"--help               . \n  -? +> @--help . \n  -h +> @--help . \n"

	 // Termination:
	 +"--           . \n"  // end option sequence here
	 +"-   +?    >- . \n"  // report error if -XXX present; else use stdout
	 );
    // Note: Collection options use "\0" as a delimiter between arguments.

    // For Java version of unpacker (used for testing only):
    private static final String UNPACK200_OPTION_MAP =
	(""
	 +"--deflate-hint=       *> = \n  -H +>  @--deflate-hint=       = \n"
	 +"--verbose                $ \n  -v +>- @--verbose             $ \n"
	 +"--quiet        !--verbose  \n  -q +>- !--verbose               \n"
	 +"--remove-pack-file       $ \n  -r +>- @--remove-pack-file    $ \n"
	 +"--log-file=           *> = \n  -l +>  @--log-file=           = \n"
	 //"--config-file=       *> . \n  -f +>  @--config-file=        . \n"

	 // Termination:
	 +"--           . \n"  // end option sequence here
	 +"-   +?    >- . \n"  // report error if -XXX present; else use stdin
	 +"--version                . \n  -V +>  @--version             . \n"
	 +"--help               . \n  -? +> @--help . \n  -h +> @--help . \n"
	 );

    private static final String[] PACK200_PROPERTY_TO_OPTION = {
	Pack200.Packer.SEGMENT_LIMIT, "--segment-limit=",
	Pack200.Packer.KEEP_FILE_ORDER, "--no-keep-file-order",
	Pack200.Packer.EFFORT, "--effort=",
	Pack200.Packer.DEFLATE_HINT, "--deflate-hint=",
	Pack200.Packer.MODIFICATION_TIME, "--modification-time=",
	Pack200.Packer.PASS_FILE_PFX, "--pass-file=",
	Pack200.Packer.UNKNOWN_ATTRIBUTE, "--unknown-attribute=",
	Pack200.Packer.CLASS_ATTRIBUTE_PFX, "--class-attribute=",
	Pack200.Packer.FIELD_ATTRIBUTE_PFX, "--field-attribute=",
	Pack200.Packer.METHOD_ATTRIBUTE_PFX, "--method-attribute=",
	Pack200.Packer.CODE_ATTRIBUTE_PFX, "--code-attribute=",
	//Pack200.Packer.PROGRESS, "--progress=",
	Utils.DEBUG_VERBOSE, "--verbose",
	Utils.COM_PREFIX+"strip.debug", "--strip-debug",
    };

    private static final String[] UNPACK200_PROPERTY_TO_OPTION = {
	Pack200.Unpacker.DEFLATE_HINT, "--deflate-hint=",
	//Pack200.Unpacker.PROGRESS, "--progress=",
	Utils.DEBUG_VERBOSE, "--verbose",
	Utils.UNPACK_REMOVE_PACKFILE, "--remove-pack-file",
    };

    /*-*
     * Remove a set of command-line options from args,
     * storing them in the map in a canonicalized form.
     * <p>
     * The options string is a newline-separated series of
     * option processing specifiers.
     */
    private static
    String parseCommandOptions(List<String> args,
			       String options,
			       Map<String,String> properties) {
	//System.out.println(args+" // "+properties);

	String resultString = null;

	// Convert options string into optLines dictionary.
	TreeMap<String,String[]> optmap = new TreeMap<String,String[]>();
    loadOptmap:
	for (String optline : options.split("\n")) {
	    String[] words = optline.split("\\p{Space}+");
	    if (words.length == 0)    continue loadOptmap;
	    String opt = words[0];
	    words[0] = "";  // initial word is not a spec
	    if (opt.length() == 0 && words.length >= 1) {
		opt = words[1];  // initial "word" is empty due to leading ' '
		words[1] = "";
	    }
	    if (opt.length() == 0)    continue loadOptmap;
	    String[] prevWords = optmap.put(opt, words);
	    if (prevWords != null)
		throw new RuntimeException("duplicate option: "
					   +optline.trim());
	}

	// State machine for parsing a command line.
	ListIterator<String> argp = args.listIterator();
	ListIterator<String> pbp = new ArrayList<String>().listIterator();
    doArgs:
	for (;;) {
	    // One trip through this loop per argument.
	    // Multiple trips per option only if several options per argument.
	    String arg;
	    if (pbp.hasPrevious()) {
		arg = pbp.previous();
		pbp.remove();
	    } else if (argp.hasNext()) {
		arg = argp.next();
	    } else {
		// No more arguments at all.
		break doArgs;
	    }
	tryOpt:
	    for (int optlen = arg.length(); ; optlen--) {
		// One time through this loop for each matching arg prefix.
		String opt;
		// Match some prefix of the argument to a key in optmap.
	    findOpt:
		for (;;) {
		    opt = arg.substring(0, optlen);
		    if (optmap.containsKey(opt))  break findOpt;
		    if (optlen == 0)              break tryOpt;
		    // Decide on a smaller prefix to search for.
		    SortedMap<String,String[]> pfxmap = optmap.headMap(opt);
		    // pfxmap.lastKey is no shorter than any prefix in optmap.
		    int len = pfxmap.isEmpty() ? 0 : pfxmap.lastKey().length();
		    optlen = Math.min(len, optlen - 1);
		    opt = arg.substring(0, optlen);
		    // (Note:  We could cut opt down to its common prefix with
		    // pfxmap.lastKey, but that wouldn't save many cycles.)
		}
		opt = opt.intern();
		assert(arg.startsWith(opt));
		assert(opt.length() == optlen);
		String val = arg.substring(optlen);  // arg == opt+val

		// Execute the option processing specs for this opt.
		// If no actions are taken, then look for a shorter prefix.
		boolean didAction = false;
		boolean isError = false;

		int pbpMark = pbp.nextIndex();  // in case of backtracking
		String[] specs = optmap.get(opt);
	    eachSpec:
		for (String spec : specs) {
		    if (spec.length() == 0)     continue eachSpec;
		    if (spec.startsWith("#"))   break eachSpec;
		    int sidx = 0;
		    char specop = spec.charAt(sidx++);

		    // Deal with '+'/'*' prefixes (spec conditions).
		    boolean ok;
		    switch (specop) {
		    case '+':
			// + means we want an non-empty val suffix.
			ok = (val.length() != 0);
			specop = spec.charAt(sidx++);
			break;
		    case '*':
			// * means we accept empty or non-empty
			ok = true;
			specop = spec.charAt(sidx++);
			break;
		    default:
			// No condition prefix means we require an exact
			// match, as indicated by an empty val suffix.
			ok = (val.length() == 0);
			break;
		    }
		    if (!ok)  continue eachSpec;

		    String specarg = spec.substring(sidx);
		    switch (specop) {
		    case '.':  // terminate the option sequence
			resultString = (specarg.length() != 0)? specarg.intern(): opt;
			break doArgs;
		    case '?':  // abort the option sequence
			resultString = (specarg.length() != 0)? specarg.intern(): arg;
			isError = true;
			break eachSpec;
		    case '@':  // change the effective opt name
			opt = specarg.intern();
			break;
		    case '>':  // shift remaining arg val to next arg
			pbp.add(specarg + val);  // push a new argument
			val = "";
			break;
		    case '!':  // negation option
			String negopt = (specarg.length() != 0)? specarg.intern(): opt;
			properties.remove(negopt);
			properties.put(negopt, null);  // leave placeholder
			didAction = true;
			break;
		    case '$':  // normal "boolean" option
			String boolval;
			if (specarg.length() != 0) {
			    // If there is a given spec token, store it.
			    boolval = specarg;
			} else {
			    String old = properties.get(opt);
			    if (old == null || old.length() == 0) {
				boolval = "1";
			    } else {
				// Increment any previous value as a numeral.
				boolval = ""+(1+Integer.parseInt(old));
			    }
			}
			properties.put(opt, boolval);
			didAction = true;
			break;
		    case '=':  // "string" option
		    case '&':  // "collection" option
			// Read an option.
			boolean append = (specop == '&');
			String strval;
			if (pbp.hasPrevious()) {
			    strval = pbp.previous();
			    pbp.remove();
			} else if (argp.hasNext()) {
			    strval = argp.next();
			} else {
			    resultString = arg + " ?";
			    isError = true;
			    break eachSpec;
			}
			if (append) {
			    String old = properties.get(opt);
			    if (old != null) {
				// Append new val to old with embedded delim.
				String delim = specarg;
				if (delim.length() == 0)  delim = " ";
				strval = old + specarg + strval;
			    }
			}
			properties.put(opt, strval);
			didAction = true;
			break;
		    default:
			throw new RuntimeException("bad spec for "
						   +opt+": "+spec);
		    }
		}

		// Done processing specs.
		if (didAction && !isError) {
		    continue doArgs;
		}

		// The specs should have done something, but did not.
		while (pbp.nextIndex() > pbpMark) {
		    // Remove anything pushed during these specs.
		    pbp.previous();
		    pbp.remove();
		}

		if (isError) {
		    throw new IllegalArgumentException(resultString);
		}

		if (optlen == 0) {
		    // We cannot try a shorter matching option.
		    break tryOpt;
		}
	    }

	    // If we come here, there was no matching option.
	    // So, push back the argument, and return to caller.
	    pbp.add(arg);
	    break doArgs;
	}
	// Report number of arguments consumed.
	args.subList(0, argp.nextIndex()).clear();
	// Report any unconsumed partial argument.
	while (pbp.hasPrevious())  args.add(0, pbp.previous());
	//System.out.println(args+" // "+properties+" -> "+resultString);
	return resultString;
    }
}


