/*
 * @(#)UnixPlatform.java	1.4 03/12/23 16:38:42
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

public class UnixPlatform extends Platform {
    public void setupFileTemplates() {
	inclFileTemplate = new FileName(this,
	    "incls/", "_", "",             ".incl", "", ""
	);
	giFileTemplate = new FileName(this,
	    "incls/", "",  "_precompiled", ".incl", "", ""
	);
	gdFileTemplate = new FileName(this,
	    "",       "",  "Dependencies", "",      "", ""
	);
    }
    
    private static String[] suffixes = { ".cpp", ".c", ".s" };

    public String[] outerSuffixes() {
	return suffixes;
    }

    public String objFileSuffix() {
	return ".o";
    }

    public String asmFileSuffix() {
	return ".i";
    }

    public String dependentPrefix() {
	return "";
    }

    /** Do not change this; unless you fix things so precompiled
	header files get translated into make dependencies. - Ungar */
    public int defaultGrandIncludeThreshold() {
	return 1 << 30;
    }

    /** For Unix make, include the dependencies for precompiled header
        files. */
    public boolean includeGIDependencies() {
	return true;
    }
}
