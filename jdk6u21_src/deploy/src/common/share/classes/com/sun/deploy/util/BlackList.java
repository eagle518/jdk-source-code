/*
 * @(#)BlackList.java	1.7 10/05/20
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.io.Reader;
import java.io.StreamTokenizer;
import java.io.StreamTokenizer.*;
import java.security.AccessController;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;
import java.util.HashMap;
import java.util.Locale;
import sun.misc.JavaUtilJarAccess;
import sun.misc.SharedSecrets;
import sun.security.action.GetPropertyAction;
import sun.security.action.OpenFileInputStreamAction;
import com.sun.deploy.config.Config;
import java.security.GeneralSecurityException;
import java.io.InputStream;
import java.util.jar.*;
import java.util.*;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

/**
 * The format of the file is as follows:
 *
 * attribute : value
 *
 * The only attribute recognized is x-Digest-Manifest, where x is the digest
 * algorithm, and the value is the base64 encoded digest (or hash) of the
 * manifest file. Unrecognized attributes cause an exception to be thrown.
 * Comments are denoted by lines starting with the # (number) symbol.
 *
 * @author Sean Mullan, Dennis Gu
 * @version 1.7, 05/20/10
 */
public final class BlackList {

    private static BlackList INSTANCE = null;

    private final static String DIGEST_MANIFEST = "-DIGEST-MANIFEST";

    /**
     * The hashmap of blacklist entries. The key is the X-DIGEST-MANIFEST
     * attribute (where x is the digest algorithm, in upper case) and the
     * base 64 encoded digest value concatenated. The map entries are null.
     */
    private final HashMap entries = new HashMap();

    /**
     * Timestampt of last modification of blacklist files.
     */
    private static long lastModified = 0;

    // Trying to load BlackList object if the property is turned on
    // and it use JRE 1.4 and later
    static {
	if (Config.getBooleanProperty(Config.SEC_USE_BLACKLIST_CHECK_KEY) &&
	    Config.isJavaVersionAtLeast14() && 
	    Config.checkClassName("sun.security.action.OpenFileInputStreamAction") ) {
	    Trace.msgSecurityPrintln("downloadengine.check.blacklist.enabled");
	    INSTANCE = new BlackList();
	}
    }

    /**
     * Returns the singleton BlackList instance.
     *
     * @throws BlackListSyntaxException if there is an error parsing the
     *    blacklists
     */
    public static BlackList getInstance() {
        return INSTANCE;
    }

    // Should only be used for testing
    public static BlackList getInstance(File file) {
        return new BlackList(file);
    }

    private BlackList() {
        try {
	    // Get Blacklist file from user and system directory
  	    String _userBlacklistFilename = Config.getUserBlacklistFile();
  	    String _systemBlacklistFilename = Config.getSystemBlacklistFile();
  
 	    File userBlacklistFile = new File(_userBlacklistFilename);
  	    File systemBlacklistFile = new File(_systemBlacklistFilename);
  
 	    // Return Blacklist file from user directory first,
  	    // If it doesn't exist, return from system directory.
            if (userBlacklistFile.exists()) {
                setup(userBlacklistFile);
                lastModified = userBlacklistFile.lastModified();
            }
            if (systemBlacklistFile.exists()) {
                setup(systemBlacklistFile);
                long tm = systemBlacklistFile.lastModified();
                if (tm > lastModified) {
                    lastModified = tm;
                }
            } 
        } catch (IOException ioe) {
            throw new BlackListSyntaxException(ioe);
        }
    }

    private BlackList(File file) {
        try {
            setup(file);
        } catch (IOException ioe) {
            throw new BlackListSyntaxException(ioe);
        }
    }

    private void setup(File file) throws IOException {
        BufferedReader br = null;
        try {
            FileInputStream fis = (FileInputStream)AccessController.doPrivileged
                (new OpenFileInputStreamAction(file));
            br = new BufferedReader(new InputStreamReader(fis));
            parse(br);
        } catch (PrivilegedActionException pae) {
            throw (IOException) pae.getException();
        } finally {
            if (br != null) {
                br.close();
            }
        }
    }

    /**
     * Returns true if this blacklist contains no entry.
     *
     * @returns true if this blacklist contains no entry.
     *
     */
    public boolean isEmpty() {
	return entries.isEmpty();
    }

    private void setupTokenizer(StreamTokenizer st) {
        st.resetSyntax();
        st.wordChars('a', 'z');
        st.wordChars('A', 'Z');
        st.wordChars('0', '9');
        st.wordChars('.', '.');
        st.wordChars('-', '-');
        st.wordChars('_', '_');
        st.wordChars('+', '+');
        st.wordChars('/', '/');
        st.whitespaceChars(0, ' ');
        st.commentChar('#');
        st.eolIsSignificant(true);
    }

    private void parse(Reader r) throws IOException {
        StreamTokenizer st = new StreamTokenizer(r);
        setupTokenizer(st);
        while (true) {
            int token = st.nextToken();
            if (token == StreamTokenizer.TT_EOF) {
                break;
            }
            if (token == StreamTokenizer.TT_EOL) {
                continue;
            }
            if (token != StreamTokenizer.TT_WORD) {
                throw new IOException("Unexpected token: " + st);
            }
            String word = st.sval;
            if (word.toUpperCase(Locale.ENGLISH).endsWith(DIGEST_MANIFEST)) {
                parseJarEntry(st);
            } else {
		//@@@ ignore unrecognized attributes?
                throw new IOException
                    ("Unknown attribute `" + word + "', line " + st.lineno());
            }
        }
    }

    private void parseColon(StreamTokenizer st) throws IOException {
        int token = st.nextToken();
        if (token != ':') {
            throw new IOException("Expected ':', read " + st);
        }
    }

    private void parseJarEntry(StreamTokenizer st) throws IOException {
	String attribute = st.sval;

        parseColon(st);
        String hash = null;
        st.wordChars('=', '=');
        int token = st.nextToken();
        if (token != StreamTokenizer.TT_WORD) {
            throw new IOException("Unexpected value: " + st);
        }
        st.ordinaryChar('=');
        hash = st.sval;
        if (hash == null) {
            throw new IOException("hash must be specified");
        }

	// add entry to hashmap
        entries.put(attribute.toUpperCase(Locale.ENGLISH) + hash, null);
    }

    /**
     * Returns true if this blacklist contains an entry for the specified
     * digest manifest attribute and hash.
     *
     * @param attribute the digest manifest attribute
     * @param hash the base64 encoded hash value
     * @return true if this blacklist contains an entry for the specified
     *  attribute and hash
     */
    public boolean contains(String attribute, String hash) {
        return entries.containsKey
            (attribute.toUpperCase(Locale.ENGLISH) + hash);
    }


    /* Helper method to quickly get attributes from the entry */
    private static Attributes readAttributes(JarFile f, JarEntry e) throws IOException {
        final InputStream is = f.getInputStream(e);
        try {
            /* We want to read entry attributes without reading Manifest
             * for performance reasons.
             * For this we want to call
             *    Attributes.read(Manifest.FastInputStream, byte[])
             * As Manifest.FastInputStream and Attributes.read() are package
             * private we will use reflection to get access to them.
             *
             * If something fails then we try to revert to reading manifest.
             */

            Object o = AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws Exception {
                    //placeholder for return value
                    Attributes a = new Attributes();

                    //prepare input stream
                    Class fisClass = Class.forName("java.util.jar.Manifest$FastInputStream");
                    java.lang.reflect.Constructor[] cc = fisClass.getDeclaredConstructors();
                    Constructor ct = null;
                    
                    //we can not use getConstructor as it will throw exception
                    //but we can get all constructors and look for one we need
                    for(int i=0; i<cc.length; i++) {
                      Class types[] = cc[i].getParameterTypes();
                      //we need
                      //    Manifest.FastInputStream(InputStream)
                      if (types.length == 1 && types[0] == InputStream.class) {
                        ct = cc[i];
                        break;
                      }
                    }
                    if (ct == null) {
                        throw new Exception("Failed to find stream constructor");
                    }
                    ct.setAccessible(true);
                    Object arglist[] = {is};
                    Object faststream = ct.newInstance(arglist);

                    //now call Attribute.read(FastInputStream, byte[])
                    byte[] lbuf = new byte[512];
                    Class param[] = {fisClass, lbuf.getClass()};
                    Method m = Attributes.class.getDeclaredMethod("read", param);
                    if (m != null){
                        m.setAccessible(true);
                        Object arglist2[] = {faststream, lbuf};
                        m.invoke(a, arglist2);
                    }

                    return a;
                }
            });
            return ((Attributes) o);
        } catch (Exception ex) {
            return e.getAttributes();
        } finally {
            is.close();
        }
    }
    /* Returns:
         false - if given entry does not contain info to be verified
         true  - if given entry was sucessfully verified
                 (further verification of this jar is not needed)
         throws GeneralSecurityException if verification fails
     */
    public static boolean checkJarEntry(JarFile jar, JarEntry entry) 
             throws IOException, GeneralSecurityException {

	if (INSTANCE == null || INSTANCE.isEmpty()) {
            return true;
        }
        if (!entry.getName().toUpperCase(Locale.ENGLISH).endsWith(".SF")) {
	    return false;
	}
        Attributes mainAttrs = readAttributes(jar, entry);
        if (mainAttrs == null) {
            return false; // NB: can there be more than one .SF entry?
                          //     if not then perhaps we should return true
        }

        Iterator iterAttr = mainAttrs.keySet().iterator();
        while (iterAttr.hasNext()) {
            String key = iterAttr.next().toString();
            if (key.toUpperCase(Locale.ENGLISH).endsWith(DIGEST_MANIFEST)) {
                Attributes.Name sha1AttrName = new Attributes.Name(key);
                String hash = mainAttrs.getValue(sha1AttrName);
                if (INSTANCE.contains(key, hash)) {
                    Trace.msgSecurityPrintln("downloadengine.check.blacklist.found");
                    throw new GeneralSecurityException("blacklisted entry!");
                }
            }
        }
        Trace.msgSecurityPrintln("downloadengine.check.blacklist.notfound");
        return false;
    }

    /* Returns true if file is in the blacklist, false otherwise */
    public static boolean checkJarFile(JarFile jar) throws IOException {
        if (INSTANCE == null || INSTANCE.isEmpty()) {
            Trace.msgSecurityPrintln("downloadengine.check.blacklist.notexist");
            return false;
        }

	List digests = getManifestDigests(jar);
	if (digests != null) {
	    try {
	        Iterator itor = digests.iterator();
	        while (itor.hasNext()) {
		    String key = (String) itor.next();
		    String hash = (String) itor.next();
                    if (INSTANCE.contains(key, hash)) {
                        Trace.msgSecurityPrintln("downloadengine.check.blacklist.found");
                        return true;
                    }
	        }
	        return false;
	    } catch (NoSuchElementException nsee) {
		// shouldn't happen, fall through */
	    }
	}

        Enumeration entries = jar.entries();
        while (entries.hasMoreElements()) {
            JarEntry entry = (JarEntry)entries.nextElement();
            String uname = entry.getName().toUpperCase(Locale.ENGLISH);

            if ((uname.startsWith("META-INF/") ||
                 uname.startsWith("/META-INF/"))) {
                try {
                    if (checkJarEntry(jar, entry)) {
                        return false;
                    }
                } catch (GeneralSecurityException ge) {
                    return true;
                }
	    }
        }

        Trace.msgSecurityPrintln("downloadengine.check.blacklist.notsigned");
        return false;
    }

    /**
     * Returns true if blacklits are present and any of them is newer than
     * given timestampt. */
    public static boolean hasBeenModifiedSince(long tm) {
        if (INSTANCE == null || lastModified < tm) 
            return false;
        return true;
    }

    private static List getManifestDigests(JarFile jar) {
	try {
            JavaUtilJarAccess access = SharedSecrets.javaUtilJarAccess();
	    return access.getManifestDigests(jar);
	} catch (NoSuchMethodError e) {
	    return null;
	} catch (NoClassDefFoundError ncdfe) {
	    return null;
	}
    }
}

class BlackListSyntaxException extends RuntimeException {
    BlackListSyntaxException(Exception e) {
        super(e);
    }
}
