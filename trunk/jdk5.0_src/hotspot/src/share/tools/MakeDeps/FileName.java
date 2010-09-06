/*
 * @(#)FileName.java	1.4 03/12/23 16:38:40
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

public class FileName {
    private String dir;
    private String prefix;
    private String stem;
    private String suffix;
    private String inverseDir;
    private String altSuffix;

    private String dpss;
    private String psa;
    private String dpsa;
    private String pss;

    private Platform plat;

    /** None of the passed strings may be null. */

    public FileName(Platform plat, String dir, String prefix,
		    String stem, String suffix,
		    String inverseDir, String altSuffix) {
	if ((dir == null) ||
	    (prefix == null) ||
	    (stem == null) ||
	    (suffix == null) ||
	    (inverseDir == null) ||
	    (altSuffix == null)) {
	    throw new NullPointerException("All arguments must be non-null");
	}

	this.plat = plat;

	this.dir = dir;
	this.prefix = prefix;
	this.stem = stem;
	this.suffix = suffix;
	this.inverseDir = inverseDir;
	this.altSuffix = altSuffix;

	pss = prefix + stem + suffix;
	dpss = dir + prefix + stem + suffix;
	psa = prefix + stem + altSuffix;
	dpsa = dir + prefix + stem + altSuffix;

	checkLength(plat);
    }

    public void checkLength(Platform p) {
	int len;
	String s;
	int suffLen = suffix.length();
	int altSuffLen = altSuffix.length();
	if (suffLen >= altSuffLen) {
	    len = suffLen;
	    s = suffix;
	} else {
	    len = altSuffLen;
	    s = altSuffix;
	}
	len += prefix.length() + stem.length();
	int lim = p.fileNameLengthLimit();
	if (len > lim) {
	    p.fatalError(prefix + stem + s + " is too long: " +
			 len + " >= " + lim);
	}
    }

    public String dirPreStemSuff() {
	return dpss;
    }

    public String preStemSuff() {
	return pss;
    }

    public String dirPreStemAltSuff() {
	return dpsa;
    }

    public String preStemAltSuff() {
	return psa;
    }

    public FileName copyStem(String newStem) {
	return new FileName(plat, dir, prefix, newStem,
			    suffix, inverseDir, altSuffix);
    }

    String nameOfList() {
	return stem;
    }

    String getInvDir() {
	return inverseDir;
    }
}
