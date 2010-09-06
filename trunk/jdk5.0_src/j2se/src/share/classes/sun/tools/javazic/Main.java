/*
 * @(#)Main.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 */

package sun.tools.javazic;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;

/**
 * Main class for the javazic time zone data compiler.
 *
 * @since 1.4
 */
public class Main {

    private static boolean verbose = false;
    static boolean outputSrc = false;
    static boolean outputDoc = false;

    private ArrayList ziFiles = new ArrayList();
    private static String zoneNamesFile = null;
    private static String versionName = "unknown";
    private static String outputDir = "zoneinfo";
    private static String mapFile = null;

    /**
     * Parses the specified arguments and sets up the variables.
     * @param argv the arguments
     */
    void processArgs(String[] argv) {
	for (int i = 0; i < argv.length; i++) {
	    String arg = argv[i];
	    if (arg.startsWith("-h")) {
		usage();
		System.exit(0);
	    } else if (arg.equals("-d")) {
		outputDir = argv[++i];
	    } else if (arg.equals("-v")) {
		verbose = true;
	    } else if (arg.equals("-V")) {
		versionName = argv[++i];
	    } else if (arg.equals("-source")) {
		outputSrc = true;
	    } else if (arg.equals("-doc")) {
		outputDoc = true;
	    } else if (arg.equals("-map")) {
		outputDoc = true;
		mapFile = argv[++i];
	    } else if (arg.equals("-f")) {
		zoneNamesFile = argv[++i];
	    } else if (arg.equals("-S")) {
		try {
		    Zoneinfo.setYear(Integer.parseInt(argv[++i]));
		} catch (Exception e) {
		    error("invalid year: " + argv[i]);
		    usage();
		    System.exit(1);
		}
	    } else {
		boolean isStartYear = arg.equals("-s");
		if (isStartYear || arg.equals("-e")) {
		    try {
			int year = Integer.parseInt(argv[++i]);
			if (isStartYear) {
			    Zoneinfo.setStartYear(year);
			} else {
			    Zoneinfo.setEndYear(year);
			}
		    } catch (Exception e) {
			error("invalid year: " + argv[i]);
			usage();
			System.exit(1);
		    }
		} else {
		    // the rest of args are zoneinfo source files
		    while (i < argv.length) {
			ziFiles.add(argv[i++]);
		    }
		}
	    }
	}
    }

    /**
     * Parses zoneinfo source files
     */
    int compile() {
	int nFiles = ziFiles.size();
	int status = 0;
	Mappings maps = new Mappings();
	BackEnd backend = BackEnd.getBackEnd();

	for (int i = 0; i < nFiles; i++) {
	    Zoneinfo frontend = Zoneinfo.parse((String)ziFiles.get(i));
	    Iterator keys = frontend.getZoneIterator();

	    while (keys.hasNext()) {
		String key = (String) keys.next();
		info(key);

		Timezone tz = frontend.phase2(key);
		status |= backend.processZoneinfo(tz);
	    }

	    maps.add(frontend);
	}

	// special code for dealing with the conflicting name "MET"
	Zone.addMET();

	maps.resolve();

	status |= backend.generateSrc(maps);

	return status;
    }

    public static void main(String[] argv) {
	Main zic = new Main();

	/*
	 * Parse args
	 */
	zic.processArgs(argv);

	/*
	 * Read target zone names
	 */
	if (zoneNamesFile != null) {
	    Zone.readZoneNames(zoneNamesFile);
	}

	int status = zic.compile();

	System.exit(status);
    }

    void usage() {
	System.err.println("Usage: javazic [options] file...\n"+
			   "         -f namefile  file containing zone names\n"+
			   "                      to be generated\n"+
			   "         -d dir       output directory\n"+
			   "         -v           verbose\n"+
			   "         -V datavers  specifies the tzdata version string (e.g., \"tzdata2000g\")"+
			   "         -S year      output only SimleTimeZone data of that year\n"+
			   "         -s year      start year (default: 1900)\n"+
			   "         -e year      end year (default: 2037)\n"+
			   "         -source      generates java program files\n"+
			   "         -doc         generates HTML documents\n"+
			   "         -map mapfile generates HTML documents with map information\n"+
			   "         file...      zoneinfo source file(s)");
    }

    /**
     * @return the output directory path name
     */
    static String getOutputDir() {
	return outputDir;
    }

    /**
     * @return the map file's path and name
     */
    static String getMapFile() {
	return mapFile;
    }

    /**
     * Returns the time zone data version string specified by the -V
     * option. If it is not specified, "unknown" is returned.
     * @return the time zone data version string
     */
    static String getVersionName() {
	return versionName;
    }

    /**
     * Prints out the specified fatal error message and calls {@link
     * java.lang.System#exit System.exit(1)}.
     * @param msg the fatal error message
     */
    static void panic(String msg) {
	printMessage("fatal error", msg);
	System.exit(1);
    }

    /**
     * Prints out the specified error message.
     * @param msg the error message
     */
    static void error(String msg) {
	printMessage("error", msg);
    }

    /**
     * Prints out the specified warning message.
     * @param msg the warning message
     */
    static void warning(String msg) {
	printMessage("warning", msg);
    }

    /**
     * Prints out the informative message.
     * @param msg the informative message
     */
    static void info(String msg) {
	if (verbose) {
	    printMessage(null, msg);
	}
    }

    private static void printMessage(String type, String msg) {
	if (type != null) {
	    type += ": ";
	} else {
	    type = "";
	}
	System.err.println("javazic: " + type + msg);
    }
}
