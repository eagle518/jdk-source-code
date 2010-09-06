/*
 * @(#)KeyTool.java	1.72 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.tools;

import java.io.*;
import java.security.GeneralSecurityException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.Key;
import java.security.PublicKey;
import java.security.PrivateKey;
import java.security.Security;
import java.security.Signature;
import java.security.SignatureException;
import java.security.UnrecoverableKeyException;
import java.security.Principal;
import java.security.Provider;
import java.security.Identity;
import java.security.Signer;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.text.Collator;
import java.text.MessageFormat;
import java.util.*;
import java.lang.reflect.Constructor;

import sun.misc.BASE64Decoder;
import sun.misc.BASE64Encoder;
import sun.security.x509.AlgorithmId;
import sun.security.x509.X509CertImpl;
import sun.security.x509.X509CertInfo;
import sun.security.x509.X500Name;
import sun.security.x509.CertAndKeyGen;
import sun.security.x509.CertificateSubjectName;
import sun.security.x509.CertificateIssuerName;
import sun.security.x509.CertificateValidity;
import sun.security.x509.CertificateSerialNumber;
import sun.security.x509.CertificateAlgorithmId;
import sun.security.x509.X509Key;
import sun.security.x509.X500Signer;
import sun.security.pkcs.PKCS10;
import sun.security.provider.IdentityDatabase;
import sun.security.provider.SystemSigner;
import sun.security.provider.SystemIdentity;
import sun.security.provider.X509Factory;
import sun.security.util.DerOutputStream;
import sun.security.util.Password;
import sun.security.util.Resources;

import static java.security.KeyStore.*;

/**
 * This tool manages keystores.
 *
 * @author Jan Luehe
 *
 * @version 1.72, 12/19/03
 *
 * @see java.security.KeyStore
 * @see sun.security.provider.KeyProtector
 * @see sun.security.provider.JavaKeyStore
 *
 * @since JDK1.2
 */

public final class KeyTool {

    private boolean debug = false;
    private String command = null;
    private String sigAlgName = null;
    private String keyAlgName = "DSA";
    private boolean verbose = false;
    private int	keysize = 1024;
    private boolean rfc = false;
    private long validity = (long)90;
    private String alias = null;
    private String dname = null;
    private String keyAlias = "mykey";
    private String dest = null;
    private String filename = null;
    private Vector providers = null;
    private final HashMap providerArgs = new HashMap();
    private String storetype = null;
    private String providerName = null;
    private char[] storePass = null;
    private char[] storePassNew = null;
    private char[] keyPass = null;
    private char[] keyPassNew = null;
    private char[] oldPass = null;
    private char[] newPass = null;
    private String ksfname = null;
    private File ksfile = null;
    private InputStream ksStream = null; // keystore stream
    private InputStream inStream = null;
    private KeyStore keyStore = null;
    private boolean token = false;
    private boolean nullStream = false;
    private boolean kssave = false;
    private boolean noprompt = false;
    private boolean trustcacerts = false;
    private boolean protectedPath = false;
    private CertificateFactory cf = null;
    private KeyStore caks = null; // "cacerts" keystore

    private static final Class[] PARAM_STRING = { String.class };

    private static final String JKS = "jks";
    private static final String NONE = "NONE";
    private static final String P11KEYSTORE = "PKCS11";

    // for i18n
    private static final java.util.ResourceBundle rb =
	java.util.ResourceBundle.getBundle("sun.security.util.Resources");
    private static final Collator collator = Collator.getInstance();
    static {
	// this is for case insensitive string comparisons
	collator.setStrength(Collator.PRIMARY);
    };

    private KeyTool() { }

    public static void main(String[] args) {
	KeyTool kt = new KeyTool();
	kt.run(args, System.out);
    }

    public void run(String[] args, PrintStream out) {
	try {
	    parseArgs(args);
	    doCommands(out);
	} catch (Exception e) {
	    System.out.println(rb.getString("keytool error: ") + e);
	    if (debug) {
		e.printStackTrace();
	    }
	    System.exit(1);
	} finally {
	    // zero-out passwords
	    if (storePass != null) {
		Arrays.fill(storePass, ' ');
		storePass = null;
	    }
	    if (storePassNew != null) {
		Arrays.fill(storePassNew , ' ');
		storePassNew = null;
	    }
	    if (keyPass != null) {
		Arrays.fill(keyPass, ' ');
		keyPass = null;
	    }
	    if (keyPassNew != null) {
		Arrays.fill(keyPassNew, ' ');
		keyPassNew = null;
	    }
	    if (oldPass != null) {
		Arrays.fill(oldPass, ' ');
		oldPass = null;
	    }
	    if (newPass != null) {
		Arrays.fill(newPass, ' ');
		newPass = null;
	    }
	}
    }

    /**
     * Parse command line arguments.
     */
    void parseArgs(String[] args) {

	if (args.length == 0) usage();

	int i=0;

	for (i=0; (i < args.length) && args[i].startsWith("-"); i++) {

	    String flags = args[i];
	    /*
	     * command modes
	     */
	    if (collator.compare(flags, "-certreq") == 0) {
		command = "certreq";
	    } else if (collator.compare(flags, "-delete") == 0) {
		command = "delete";
	    } else if (collator.compare(flags, "-export") == 0) {
		command = "export";
	    } else if (collator.compare(flags, "-genkey") == 0) {
		command = "genkey";
	    } else if (collator.compare(flags, "-help") == 0) {
		usage();
		return;
	    } else if (collator.compare(flags, "-identitydb") == 0) {
		command = "identitydb";
	    } else if (collator.compare(flags, "-import") == 0) {
		command = "import";
	    } else if (collator.compare(flags, "-keyclone") == 0) {
		command = "keyclone";
	    } else if (collator.compare(flags, "-keypasswd") == 0) {
		command = "keypasswd";
	    } else if (collator.compare(flags, "-list") == 0) {
		command = "list";
	    } else if (collator.compare(flags, "-printcert") == 0) {
		command = "printcert";
	    } else if (collator.compare(flags, "-selfcert") == 0) {
		command = "selfcert";
	    } else if (collator.compare(flags, "-storepasswd") == 0) {
		command = "storepasswd";
	    }

	    /*
	     * specifiers
	     */
	    else if (collator.compare(flags, "-keystore") == 0) {
		if (++i == args.length) usage();
		ksfname = args[i];
	    } else if (collator.compare(flags, "-storepass") == 0) {
		if (++i == args.length) usage();
		storePass = args[i].toCharArray();
	    } else if (collator.compare(flags, "-storetype") == 0) {
		if (++i == args.length) usage();
		storetype = args[i];
	    } else if (collator.compare(flags, "-providerName") == 0) {
		if (++i == args.length) usage();
		providerName = args[i];
	    } else if (collator.compare(flags, "-keypass") == 0) {
		if (++i == args.length) usage();
		keyPass = args[i].toCharArray();
	    } else if (collator.compare(flags, "-new") == 0) {
		if (++i == args.length) usage();
		newPass = args[i].toCharArray();
	    } else if (collator.compare(flags, "-alias") == 0) {
		if (++i == args.length) usage();
		alias = args[i];
	    } else if (collator.compare(flags, "-dest") == 0) {
		if (++i == args.length) usage();
		dest = args[i];
	    } else if (collator.compare(flags, "-dname") == 0) {
		if (++i == args.length) usage();
		dname = args[i];
	    } else if (collator.compare(flags, "-keysize") == 0) {
		if (++i == args.length) usage();
		keysize = Integer.parseInt(args[i]);
	    } else if (collator.compare(flags, "-keyalg") == 0) {
		if (++i == args.length) usage();
		keyAlgName = args[i];
	    } else if (collator.compare(flags, "-sigalg") == 0) {
		if (++i == args.length) usage();
		sigAlgName = args[i];
	    } else if (collator.compare(flags, "-validity") == 0) {
		if (++i == args.length) usage();
		validity = Long.parseLong(args[i]);
	    } else if (collator.compare(flags, "-file") == 0) {
		if (++i == args.length) usage();
		filename = args[i];
	    } else if ((collator.compare(flags, "-provider") == 0) ||
			(collator.compare(flags, "-providerClass") == 0)) {
		if (++i == args.length) usage();
		if (providers == null) {
		    providers = new Vector(3);
		}
		providers.add(args[i]);

		if (args.length > (i+1)) {
		    flags = args[i+1];
		    if (collator.compare(flags, "-providerArg") == 0) {
			if (args.length == (i+2)) usage();
			providerArgs.put(args[i], args[i+2]);
			i += 2;
		    }
		}
	    }

	    /*
	     * options
	     */
	    else if (collator.compare(flags, "-v") == 0) {
		verbose = true;
	    } else if (collator.compare(flags, "-debug") == 0) {
		debug = true;
	    } else if (collator.compare(flags, "-rfc") == 0) {
		rfc = true;
	    } else if (collator.compare(flags, "-noprompt") == 0) {
		noprompt = true;
	    } else if (collator.compare(flags, "-trustcacerts") == 0) {
		trustcacerts = true;
	    } else if (collator.compare(flags, "-protected") == 0) {
		protectedPath = true;
	    } else  {
		System.err.println(rb.getString("Illegal option:  ") + flags);
		usage();
	    }
	}

	if (i<args.length || command==null) usage();
    }

    /**
     * Execute the commands.
     */
    void doCommands(PrintStream out) throws Exception {

	if (NONE.equals(ksfname)) {
	    nullStream = true;
	}
	if (storetype == null) {
	    storetype = KeyStore.getDefaultType();
	}
	if (P11KEYSTORE.equalsIgnoreCase(storetype)) {
	    token = true;
	}

	if (token && !nullStream) {
            System.err.println(rb.getString
                ("-keystore must be NONE if -storetype is " + P11KEYSTORE));
            System.err.println();
            usage();
        }

	if (token &&
	    (command.equals("keypasswd") || command.equals("storepasswd"))) {
	    throw new UnsupportedOperationException(rb.getString
			("-storepasswd and -keypasswd commands not supported " +
			"if -storetype is " +
			P11KEYSTORE));
	}

	if (token && (keyPass != null || newPass != null)) {
	    throw new IllegalArgumentException(rb.getString
		("-keypass and -new " +
		"can not be specified if -storetype is " +
		P11KEYSTORE));
	}

	if (protectedPath) {
	    if (storePass != null || keyPass != null || newPass != null) {
		throw new IllegalArgumentException(rb.getString
			("if -protected is specified, " +
			"then -storepass, -keypass, and -new " +
			"must not be specified"));
	    }
	}

	if (validity <= (long)0) {
	    throw new Exception
		(rb.getString("Validity must be greater than zero"));
	}

	// Try to load and install specified provider
	if (providers != null) {
	    ClassLoader cl = ClassLoader.getSystemClassLoader();
	    Enumeration e = providers.elements();
	    while (e.hasMoreElements()) {
		String provName = (String)e.nextElement();
		Class provClass;
		if (cl != null) {
		    provClass = cl.loadClass(provName);
		} else {
		    provClass = Class.forName(provName);
		}

		String provArg = (String)providerArgs.get(provName);
		Object obj;
		if (provArg == null) {
		    obj = provClass.newInstance();
		} else {
		    Constructor c = provClass.getConstructor(PARAM_STRING);
		    obj = c.newInstance(provArg);
		}
		if (!(obj instanceof Provider)) {
		    MessageFormat form = new MessageFormat
			(rb.getString("provName not a provider"));
		    Object[] source = {provName};
		    throw new Exception(form.format(source));
		}
		Security.addProvider((Provider)obj);
	    }
	}

	if (command.equals("list") && verbose && rfc) {
	    System.err.println(rb.getString
		("Must not specify both -v and -rfc with 'list' command"));
	    usage();
	}

	// Make sure provided passwords are at least 6 characters long
	if (command.equals("genkey") && keyPass!=null && keyPass.length<6) {
	    throw new Exception(rb.getString
		("Key password must be at least 6 characters"));
	}
	if (newPass != null && newPass.length < 6) {
	    throw new Exception(rb.getString
		("New password must be at least 6 characters"));
	}
	
	// Check if keystore exists.
	// If no keystore has been specified at the command line, try to use
	// the default, which is located in $HOME/.keystore.
	// If the command is "genkey", "identitydb", "import", or "printcert",
	// it is OK not to have a keystore. 
	if (!command.equals("printcert")) {
	    if (ksfname == null) {
		ksfname = System.getProperty("user.home") + File.separator
		    + ".keystore";
	    }

	    if (!nullStream) {
		try {
		    ksfile = new File(ksfname);
		    // Check if keystore file is empty
		    if (ksfile.exists() && ksfile.length() == 0) {
			throw new Exception(rb.getString
			("Keystore file exists, but is empty: ") + ksfname);
		    }
		    ksStream = new FileInputStream(ksfile);
		} catch (FileNotFoundException e) {
		    if (!command.equals("genkey") &&
			!command.equals("identitydb") &&
			!command.equals("import")) {
			throw new Exception(rb.getString
				("Keystore file does not exist: ") + ksfname);
		    }
		}
	    }
	}

	if (command.equals("keyclone") && dest == null) {
	    dest = getAlias("destination");
	    if (dest.equals("")) {
		throw new Exception(rb.getString
			("Must specify destination alias"));
	    }
	}

	if (command.equals("delete") && alias == null) {
	    alias = getAlias(null);
	    if (alias.equals("")) {
		throw new Exception(rb.getString("Must specify alias"));
	    }
	}

	// Create new keystore
	if (providerName == null) {
	    keyStore = KeyStore.getInstance(storetype);
	} else {
	    keyStore = KeyStore.getInstance(storetype, providerName);
	}

	/*
	 * Load the keystore data.
	 *
	 * At this point, it's OK if no keystore password has been provided.
	 * We want to make sure that we can load the keystore data, i.e.,
	 * the keystore data has the right format. If we cannot load the
	 * keystore, why bother asking the user for his or her password?
	 * Only if we were able to load the keystore, and no keystore
	 * password has been provided, will we prompt the user for the
	 * keystore password to verify the keystore integrity.
	 * This means that the keystore is loaded twice: first load operation
	 * checks the keystore format, second load operation verifies the
	 * keystore integrity.
	 *
	 * If the keystore password has already been provided (at the
	 * command line), however, the keystore is loaded only once, and the
	 * keystore format and integrity are checked "at the same time".
	 *
	 * Null stream keystores are loaded later.
	 */
	if (!nullStream) {
	    keyStore.load(ksStream, storePass);
	    if (ksStream != null) {
		ksStream.close();
	    }
	}

	// All commands that create or modify the keystore require a keystore
	// password.

	if (nullStream && storePass != null) {
	    keyStore.load(null, storePass);
	} else if (!nullStream && storePass != null) {
	    // If we are creating a new non nullStream-based keystore,
	    // insist that the password be at least 6 characters
	    if (ksStream == null && storePass.length < 6) {
		throw new Exception(rb.getString
			("Keystore password must be at least 6 characters"));
	    }
	} else if (storePass == null) {

	    // only prompt if (protectedPath == false)

	    if (!protectedPath &&
		(command.equals("certreq") ||
			command.equals("delete") ||
			command.equals("genkey") ||
			command.equals("import") ||
			command.equals("keyclone") ||
			command.equals("selfcert") ||
			command.equals("storepasswd") ||
			command.equals("keypasswd") ||
			command.equals("identitydb"))) {
		int count = 0;
		do {
		    System.err.print
			(rb.getString("Enter keystore password:  "));
		    System.err.flush();
		    storePass = Password.readPassword(System.in);

		    // If we are creating a new non nullStream-based keystore,
		    // insist that the password be at least 6 characters
		    if (!nullStream && storePass.length < 6) {
			System.err.println(rb.getString
				("Keystore password is too short - " +
				"must be at least 6 characters"));
			storePass = null;
		    }
		    count++;
		} while ((storePass == null) && count < 3);
		if (storePass == null) {
		    System.err.println
			(rb.getString("Too many failures - try later"));
		    return;
		}
	    } else if (!protectedPath && !(command.equals("printcert"))) {
		System.err.print(rb.getString("Enter keystore password:  "));
		System.err.flush();
		storePass = Password.readPassword(System.in);
	    }

	    // Now load a nullStream-based keystore,
	    // or verify the integrity of an input stream-based keystore
	    if (nullStream) {
		keyStore.load(null, storePass);
	    } else if (ksStream != null) {
		ksStream = new FileInputStream(ksfile);
		keyStore.load(ksStream, storePass);
		ksStream.close();
	    }
	}

	// Create a certificate factory
	if (command.equals("printcert") || command.equals("import")
	       || command.equals("identitydb")) {
	    cf = CertificateFactory.getInstance("X509");
	}

	if (trustcacerts)
	    caks = getCacertsKeyStore();

	// Perform the specified command
	if (command.equals("certreq")) {
	    if (filename != null) {
		PrintStream ps = new PrintStream(new FileOutputStream
						 (filename));
		out = ps;
	    }
	    doCertReq(alias, sigAlgName, out);
	    if (verbose && filename != null) {
		MessageFormat form = new MessageFormat(rb.getString
			("Certification request stored in file <filename>"));
		Object[] source = {filename};
		System.err.println(form.format(source));
		System.err.println(rb.getString("Submit this to your CA"));
	    }
	} else if (command.equals("delete")) {
	    doDeleteEntry(alias);
	    kssave = true;
	} else if (command.equals("export")) {
	    if (filename != null) {
		PrintStream ps = new PrintStream(new FileOutputStream
						 (filename));
		out = ps;
	    }
	    doExportCert(alias, out);
	    if (filename != null) {
		MessageFormat form = new MessageFormat(rb.getString
			("Certificate stored in file <filename>"));
		Object[] source = {filename};
		System.err.println(form.format(source));
	    }
	} else if (command.equals("genkey")) {
	    doGenKeyPair(alias, dname, keyAlgName, keysize, sigAlgName);
	    kssave = true;
	} else if (command.equals("identitydb")) {
	    InputStream inStream = System.in;
	    if (filename != null) {
		inStream = new FileInputStream(filename);
	    }
	    doImportIdentityDatabase(inStream);
	} else if (command.equals("import")) {
	    InputStream inStream = System.in;
	    if (filename != null) {
		inStream = new FileInputStream(filename);
	    }
	    String importAlias = (alias!=null)?alias:keyAlias;
	    if (keyStore.isKeyEntry(importAlias) == true) { 
		kssave = installReply(importAlias, inStream);
		if (kssave)
		    System.err.println(rb.getString
			("Certificate reply was installed in keystore"));
		else
		    System.err.println(rb.getString
			("Certificate reply was not installed in keystore"));
	    } else {
		kssave = addTrustedCert(importAlias, inStream);
		if (kssave)
		    System.err.println(rb.getString
			("Certificate was added to keystore"));
		else
		    System.err.println(rb.getString
			("Certificate was not added to keystore"));
	    }
	} else if (command.equals("keyclone")) {
	    keyPassNew = newPass;
	    doCloneKey(alias, dest);
	    kssave = true;
	} else if (command.equals("keypasswd")) {
	    keyPassNew = newPass;
	    doChangeKeyPasswd(alias);
	    kssave = true;
	} else if (command.equals("list")) {
	    if (alias != null) {
		doPrintEntry(alias, out, true);
	    } else {
		doPrintEntries(out);
	    }
	} else if (command.equals("printcert")) {
	    InputStream inStream = System.in;
	    if (filename != null) {
		inStream = new FileInputStream(filename);
	    }
	    doPrintCert(inStream, out);
	} else if (command.equals("selfcert")) {
	    doSelfCert(alias, dname, sigAlgName);
	    kssave = true;
	} else if (command.equals("storepasswd")) {
	    storePassNew = newPass;
	    if (storePassNew == null) {
		storePassNew = getNewPasswd("keystore password", storePass);
	    }
	    kssave = true;
	}

	// If we need to save the keystore, do so.
	if (kssave) {
	    if (verbose) {
		MessageFormat form = new MessageFormat
			(rb.getString("[Storing ksfname]"));
		Object[] source = {nullStream ? "keystore" : ksfname};
		System.err.println(form.format(source));
	    }

	    if (token) {
		keyStore.store(null, null);
	    } else {
		FileOutputStream fout = (nullStream ?
					(FileOutputStream)null :
					new FileOutputStream(ksfname));
		keyStore.store
			(fout,
			(storePassNew!=null) ? storePassNew : storePass);
		if (fout != null) {
		    fout.close();
		}
	    }
	}
    }

    /**
     * Creates a PKCS#10 cert signing request, corresponding to the
     * keys (and name) associated with a given alias.
     */
    private void doCertReq(String alias, String sigAlgName, PrintStream out)
	throws Exception
    {
	if (alias == null) {
	    alias = keyAlias;
	}

	Object[] objs = recoverPrivateKey(alias, storePass, keyPass);
	PrivateKey privKey = (PrivateKey)objs[0];
	if (keyPass == null)
	    keyPass = (char[])objs[1];
	
	Certificate cert = keyStore.getCertificate(alias);
	if (cert == null) {
	    MessageFormat form = new MessageFormat
		(rb.getString("alias has no public key (certificate)"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}
	PKCS10 request = new PKCS10(cert.getPublicKey());

	// Construct an X500Signer object, so that we can sign the request
	if (sigAlgName == null) {
	    // If no signature algorithm was specified at the command line,
	    // we choose one that is compatible with the selected private key
	    String keyAlgName = privKey.getAlgorithm();
	    if (keyAlgName.equalsIgnoreCase("DSA")
		   || keyAlgName.equalsIgnoreCase("DSS")) {
		sigAlgName = "SHA1WithDSA";
	    } else if (keyAlgName.equalsIgnoreCase("RSA")) {
		sigAlgName = "MD5WithRSA";
	    } else {
		throw new Exception(rb.getString
			("Cannot derive signature algorithm"));
	    }
	}

	Signature signature = Signature.getInstance(sigAlgName);
	signature.initSign(privKey);
	X500Name subject =
	    new X500Name(((X509Certificate)cert).getSubjectDN().toString());
        X500Signer signer = new X500Signer(signature, subject);

	// Sign the request and base-64 encode it
	request.encodeAndSign(signer);
	request.print(out);
    }

    /**
     * Deletes an entry from the keystore.
     */
    private void doDeleteEntry(String alias) throws Exception {
	if (keyStore.containsAlias(alias) == false) {
	    MessageFormat form = new MessageFormat
		(rb.getString("Alias <alias> does not exist"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}
	keyStore.deleteEntry(alias);
    }

    /**
     * Exports a certificate from the keystore.
     */
    private void doExportCert(String alias, PrintStream out)
	throws Exception
    {
	if (storePass == null) {
	    printWarning();
	}
	if (alias == null) {
	    alias = keyAlias;
	}
	if (keyStore.containsAlias(alias) == false) {
	    MessageFormat form = new MessageFormat
		(rb.getString("Alias <alias> does not exist"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}

	X509Certificate cert = (X509Certificate)keyStore.getCertificate(alias);
	if (cert == null) {
	    MessageFormat form = new MessageFormat
		(rb.getString("Alias <alias> has no certificate"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}
	dumpCert(cert, out);
    }

    /**
     * Creates a new key pair and self-signed certificate.
     */
    private void doGenKeyPair(String alias, String dname, String keyAlgName,
			      int keysize, String sigAlgName)
	throws Exception
    {	
	if (alias == null) {
	    alias = keyAlias;
	}

	if (keyStore.containsAlias(alias) == true) {
	    MessageFormat form = new MessageFormat(rb.getString
		("Key pair not generated, alias <alias> already exists"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}

	if (sigAlgName == null) {
	    if (keyAlgName.equalsIgnoreCase("DSA")) {
		sigAlgName = "SHA1WithDSA";
	    } else if (keyAlgName.equalsIgnoreCase("RSA")) {
		sigAlgName = "MD5WithRSA";
	    } else {
		throw new Exception(rb.getString
			("Cannot derive signature algorithm"));
	    }
	}
	CertAndKeyGen keypair =
		new CertAndKeyGen(keyAlgName, sigAlgName, providerName);

	// If DN is provided, parse it. Otherwise, prompt the user for it.
	X500Name x500Name;
	if (dname == null) {
	    x500Name = getX500Name();
	} else {
	    x500Name = new X500Name(dname);
	}

	if (verbose) {
	    MessageFormat form = new MessageFormat(rb.getString
		("Generating keysize bit keyAlgName key pair and self-signed certificate (sigAlgName)\n\tfor: x500Name"));
	    Object[] source = {new Integer(keysize),
				keyAlgName,
				sigAlgName,
				x500Name};
	    System.err.println(form.format(source));
	}

	keypair.generate(keysize);
	PrivateKey privKey = keypair.getPrivateKey();
	X509Certificate[] chain = new X509Certificate[1];
	chain[0] = keypair.getSelfCertificate
			(x500Name, (long)validity*24*60*60);

	if (!token && keyPass == null) {
	    // Prompt for key password
	    int count;
	    for (count = 0; (count) < 3 && (keyPass == null); count++) {
		MessageFormat form = new MessageFormat(rb.getString
			("Enter key password for <alias>"));
		Object[] source = {alias};
		System.err.println(form.format(source));
		System.err.print(rb.getString
			("\t(RETURN if same as keystore password):  "));
		System.err.flush();
		keyPass = Password.readPassword(System.in);
		if (keyPass == null) {
		    keyPass = storePass;
		} else if (keyPass.length < 6) {
		    System.err.println(rb.getString
			("Key password is too short - must be at least 6 characters"));
		    keyPass = null;
		}
	    }
	    if (count == 3) {
		throw new Exception(rb.getString
			("Too many failures - key not added to keystore"));
	    }
	}

	keyStore.setKeyEntry(alias, privKey, keyPass, chain);
    }

    /**
     * Clones a key entry (a private key and its associated certificate chain)
     */
    private void doCloneKey(String orig, String dest)
	throws Exception
    {
	if (orig == null) {
	    orig = keyAlias;
	}

 	if (keyStore.containsAlias(dest) == true) {
	    MessageFormat form = new MessageFormat
		(rb.getString("Destination alias <dest> already exists"));
	    Object[] source = {dest};
	    throw new Exception(form.format(source));
	}

	Object[] objs = recoverPrivateKey(orig, storePass, keyPass);
	PrivateKey privKey = (PrivateKey)objs[0];
	if (keyPass == null)
	    keyPass = (char[])objs[1];

	// User may choose to encrypt the private key in the cloned entry
	// using a different password
	if (!token && keyPassNew == null) {
	    int count = 0;
	    do {
		keyPassNew = getKeyPasswd(dest, orig, keyPass);
		if (keyPassNew.length < 6) {
		    System.err.println(rb.getString
			("Password is too short - " +
			"must be at least 6 characters"));
		    keyPassNew = null;
		}
		count++;
	    } while ((keyPassNew == null) && count < 3);
	    if (keyPassNew == null) {
		throw new Exception(rb.getString
			("Too many failures. Key entry not cloned"));
	    }
	}
	keyStore.setKeyEntry(dest, privKey, keyPassNew,
			     keyStore.getCertificateChain(orig));
    }

    /**
     * Changes a key password.
     */
    private void doChangeKeyPasswd(String alias) throws Exception
    {
	if (alias == null) {
	    alias = keyAlias;
	}
	Object[] objs = recoverPrivateKey(alias, storePass, keyPass);
	PrivateKey privKey = (PrivateKey)objs[0];
	if (keyPass == null)
	    keyPass = (char[])objs[1];

	if (keyPassNew == null) {
	    MessageFormat form = new MessageFormat
		(rb.getString("key password for <alias>"));
	    Object[] source = {alias};
	    keyPassNew = getNewPasswd(form.format(source), keyPass);
	}
	keyStore.setKeyEntry(alias, privKey, keyPassNew,
			     keyStore.getCertificateChain(alias));
    }

    /**
     * Imports a JDK 1.1-style identity database. We can only store one
     * certificate per identity, because we use the identity's name as the
     * alias (which references a keystore entry), and aliases must be unique.
     */
    private void doImportIdentityDatabase(InputStream in)
	throws Exception
    {
	byte[] encoded;
	ByteArrayInputStream bais;
	java.security.cert.X509Certificate newCert;
	java.security.cert.Certificate[] chain = null;
	PrivateKey privKey;
	boolean modified = false;

	IdentityDatabase idb = IdentityDatabase.fromStream(in);
	for (Enumeration enum_ = idb.identities(); enum_.hasMoreElements();) {
	    Identity id = (Identity)enum_.nextElement();
	    newCert = null;
	    // only store trusted identities in keystore
	    if ((id instanceof SystemSigner && ((SystemSigner)id).isTrusted())
		|| (id instanceof SystemIdentity
		    && ((SystemIdentity)id).isTrusted())) {
		// ignore if keystore entry with same alias name already exists
		if (keyStore.containsAlias(id.getName()) == true) {
		    MessageFormat form = new MessageFormat
			(rb.getString("Keystore entry for <id.getName()> already exists"));
		    Object[] source = {id.getName()};
		    System.err.println(form.format(source));
		    continue;
		}
		java.security.Certificate[] certs = id.certificates();
		if (certs!=null && certs.length>0) {
		    // we can only store one user cert per identity.
		    // convert old-style to new-style cert via the encoding
		    DerOutputStream dos = new DerOutputStream();
		    certs[0].encode(dos);
		    encoded = dos.toByteArray();
		    bais = new ByteArrayInputStream(encoded);
		    newCert = (X509Certificate)cf.generateCertificate(bais);
		    bais.close();

		    // if certificate is self-signed, make sure it verifies
		    if (isSelfSigned(newCert)) {
			PublicKey pubKey = newCert.getPublicKey();
			try {
			    newCert.verify(pubKey);
			} catch (Exception e) {
			    // ignore this cert
			    continue;
			}
		    }

		    if (id instanceof SystemSigner) {
			MessageFormat form = new MessageFormat(rb.getString
			    ("Creating keystore entry for <id.getName()> ..."));
			Object[] source = {id.getName()};
			System.err.println(form.format(source));
			if (chain==null) {
			    chain = new java.security.cert.Certificate[1];
			}
			chain[0] = newCert;
			privKey = ((SystemSigner)id).getPrivateKey();
			keyStore.setKeyEntry(id.getName(), privKey, storePass,
					     chain);
		    } else {
			keyStore.setCertificateEntry(id.getName(), newCert);
		    }
		    kssave = true;
		}
	    }
	}
	if (!kssave) {
	    System.err.println(rb.getString
		("No entries from identity database added"));
	}
    }

    /**
     * Prints a single keystore entry.
     */
    private void doPrintEntry(String alias, PrintStream out,
			      boolean printWarning)
	throws Exception
    {
	if (storePass == null && printWarning == true) {
	    printWarning();
	}

	if (keyStore.containsAlias(alias) == false) {
	    MessageFormat form = new MessageFormat
		(rb.getString("Alias <alias> does not exist"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}

	if (verbose || rfc || debug) {
	    MessageFormat form = new MessageFormat
		(rb.getString("Alias name: alias"));
	    Object[] source = {alias};
	    out.println(form.format(source));

	    if (!token) {
		form = new MessageFormat(rb.getString
		    ("Creation date: keyStore.getCreationDate(alias)"));
		Object[] src = {keyStore.getCreationDate(alias)};
		out.println(form.format(src));
	    }
	} else {
	    if (!token) {
		MessageFormat form = new MessageFormat
		    (rb.getString("alias, keyStore.getCreationDate(alias), "));
		Object[] source = {alias, keyStore.getCreationDate(alias)};
		out.print(form.format(source));
	    } else {
		MessageFormat form = new MessageFormat
		    (rb.getString("alias, "));
		Object[] source = {alias};
		out.print(form.format(source));
	    }
	}

	if (keyStore.isKeyEntry(alias)) {
	    // We have a key entry
	    if (verbose || rfc || debug) {
		out.println(rb.getString("Entry type: keyEntry"));
	    } else {
		out.println(rb.getString("keyEntry,"));
	    }

	    // Get the chain
	    Certificate[] chain = keyStore.getCertificateChain(alias);
	    if (chain != null) {
		if (verbose || rfc || debug) {
		    out.println(rb.getString
			("Certificate chain length: ") + chain.length);
		    for (int i = 0; i < chain.length; i ++) {
			MessageFormat form = new MessageFormat
				(rb.getString("Certificate[(i + 1)]:"));
			Object[] source = {new Integer((i + 1))};
			out.println(form.format(source));
			if (verbose && (chain[i] instanceof X509Certificate)) {
			    printX509Cert((X509Certificate)(chain[i]), out);
			} else if (debug) {
			    out.println(chain[i].toString());
			} else {
			    dumpCert(chain[i], out);
			}
		    }
		} else {
		    // Print the digest of the user cert only
		    out.println
			(rb.getString("Certificate fingerprint (MD5): ") +
			getCertFingerPrint("MD5", chain[0]));
		}
	    }
	} else {
	    // We have a trusted certificate entry
	    Certificate cert = keyStore.getCertificate(alias);
	    if (verbose && (cert instanceof X509Certificate)) {
		out.println(rb.getString("Entry type: trustedCertEntry\n"));
		printX509Cert((X509Certificate)cert, out);
	    } else if (rfc) {
		out.println(rb.getString("Entry type: trustedCertEntry\n"));
		dumpCert(cert, out);
	    } else if (debug) {
		out.println(cert.toString());
	    } else {
		out.println(rb.getString("trustedCertEntry,"));
		out.println(rb.getString("Certificate fingerprint (MD5): ")
			    + getCertFingerPrint("MD5", cert));
	    }
	}
    }

    /**
     * Prints all keystore entries.
     */
    private void doPrintEntries(PrintStream out)
	throws Exception
    {
	if (storePass == null)
	    printWarning();
	else
	    out.println();

	out.println(rb.getString("Keystore type: ") + keyStore.getType());
	out.println(rb.getString("Keystore provider: ") +
		keyStore.getProvider().getName());
	out.println();

	MessageFormat form;
	form = (keyStore.size() == 1) ?
		new MessageFormat(rb.getString
			("Your keystore contains keyStore.size() entry")) :
		new MessageFormat(rb.getString
			("Your keystore contains keyStore.size() entries"));
	Object[] source = {new Integer(keyStore.size())};
	out.println(form.format(source));
	out.println();

	for (Enumeration e = keyStore.aliases(); e.hasMoreElements(); ) {
	    String alias = (String)e.nextElement();
	    doPrintEntry(alias, out, false);
	    if (verbose || rfc) {
		out.println(rb.getString("\n"));
		out.println(rb.getString
			("*******************************************"));
		out.println(rb.getString
			("*******************************************\n\n"));
	    }
	}
    }

    /**
     * Reads a certificate (or certificate chain) and prints its contents in
     * a human readbable format.
     */
    private void doPrintCert(InputStream in, PrintStream out)
	throws Exception
    {
	Collection c = null;
	try {
	    c = cf.generateCertificates(in);
	} catch (CertificateException ce) {
	    throw new Exception(rb.getString("Failed to parse input"), ce);
	}
	if (c.isEmpty()) {
	    throw new Exception(rb.getString("Empty input"));
	}
	Certificate[] certs = 
	    (Certificate[]) c.toArray(new Certificate[c.size()]);
	for (int i=0; i<certs.length; i++) {
	    X509Certificate x509Cert = null;
	    try {
		x509Cert = (X509Certificate)certs[i];
	    } catch (ClassCastException cce) {
		throw new Exception(rb.getString("Not X.509 certificate"));
	    }
	    if (certs.length > 1) {
		MessageFormat form = new MessageFormat
			(rb.getString("Certificate[(i + 1)]:"));
		Object[] source = {new Integer(i + 1)};
		out.println(form.format(source));
	    }
	    printX509Cert(x509Cert, out);
	    if (i < (certs.length-1))
		out.println();
	}
    }

    /**
     * Creates a self-signed certificate, and stores it as a single-element
     * certificate chain.
     */
    private void doSelfCert(String alias, String dname, String sigAlgName)
	throws Exception
    {
	if (alias == null) {
	    alias = keyAlias;
	}

	Object[] objs = recoverPrivateKey(alias, storePass, keyPass);
	PrivateKey privKey = (PrivateKey)objs[0];
	if (keyPass == null)
	    keyPass = (char[])objs[1];

	// Determine the signature algorithm
	if (sigAlgName == null) {
	    // If no signature algorithm was specified at the command line,
	    // we choose one that is compatible with the selected private key
	    String keyAlgName = privKey.getAlgorithm();
	    if (keyAlgName.equalsIgnoreCase("DSA")
		   || keyAlgName.equalsIgnoreCase("DSS")) {
		sigAlgName = "SHA1WithDSA";
	    } else if (keyAlgName.equalsIgnoreCase("RSA")) {
		sigAlgName = "MD5WithRSA";
	    } else {
		throw new Exception
			(rb.getString("Cannot derive signature algorithm"));
	    }
	}

	// Get the old certificate
	Certificate oldCert = keyStore.getCertificate(alias);
	if (oldCert == null) {
	    MessageFormat form = new MessageFormat
		(rb.getString("alias has no public key"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}
	if (!(oldCert instanceof X509Certificate)) {
	    MessageFormat form = new MessageFormat
		(rb.getString("alias has no X.509 certificate"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}

	// convert to X509CertImpl, so that we can modify selected fields
	// (no public APIs available yet)
	byte[] encoded = oldCert.getEncoded();
	X509CertImpl certImpl = new X509CertImpl(encoded);
	X509CertInfo certInfo = (X509CertInfo)certImpl.get(X509CertImpl.NAME
							   + "." +
							   X509CertImpl.INFO);

	// Extend its validity
	Date firstDate = new Date();
	Date lastDate = new Date();
	lastDate.setTime(lastDate.getTime() + (long)validity*1000*24*60*60);
	CertificateValidity interval = new CertificateValidity(firstDate,
							       lastDate);
	certInfo.set(X509CertInfo.VALIDITY, interval);

	// Make new serial number
	certInfo.set(X509CertInfo.SERIAL_NUMBER, new CertificateSerialNumber
		     ((int)(firstDate.getTime()/1000)));

	// Set owner and issuer fields
	X500Name owner;
	if (dname == null) {
	    // Get the owner name from the certificate
	    owner = (X500Name)certInfo.get(X509CertInfo.SUBJECT + "." +
					   CertificateSubjectName.DN_NAME);
	} else {
	    // Use the owner name specified at the command line
	    owner = new X500Name(dname);
	    certInfo.set(X509CertInfo.SUBJECT + "." +
			 CertificateSubjectName.DN_NAME, owner);
	}
	// Make issuer same as owner (self-signed!)
	certInfo.set(X509CertInfo.ISSUER + "." +
		     CertificateIssuerName.DN_NAME, owner);

	// The inner and outer signature algorithms have to match.
	// The way we achieve that is really ugly, but there seems to be no
	// other solution: We first sign the cert, then retrieve the
	// outer sigalg and use it to set the inner sigalg
	X509CertImpl newCert = new X509CertImpl(certInfo);
	newCert.sign(privKey, sigAlgName);
	AlgorithmId sigAlgid = (AlgorithmId)newCert.get(X509CertImpl.SIG_ALG);
	certInfo.set(CertificateAlgorithmId.NAME + "." +
		     CertificateAlgorithmId.ALGORITHM, sigAlgid);

	// Sign the new certificate
	newCert = new X509CertImpl(certInfo);
	newCert.sign(privKey, sigAlgName);

	// Store the new certificate as a single-element certificate chain
	keyStore.setKeyEntry(alias, privKey,
			     (keyPass != null) ? keyPass : storePass,
			     new Certificate[] { newCert } );

	if (verbose) {
	    System.err.println(rb.getString("New certificate (self-signed):"));
	    System.err.print(newCert.toString());
	    System.err.println();
	}
    }

    /**
     * Processes a certificate reply from a certificate authority.
     *
     * <p>Builds a certificate chain on top of the certificate reply,
     * using trusted certificates from the keystore. The chain is complete
     * after a self-signed certificate has been encountered. The self-signed
     * certificate is considered a root certificate authority, and is stored
     * at the end of the chain.
     *
     * <p>The newly generated chain replaces the old chain associated with the
     * key entry.
     *
     * @return true if the certificate reply was installed, otherwise false.
     */
    private boolean installReply(String alias, InputStream in)
	throws Exception
    {
	if (alias == null) {
	    alias = keyAlias;
	}

	Object[] objs = recoverPrivateKey(alias, storePass, keyPass);
	PrivateKey privKey = (PrivateKey)objs[0];
	if (keyPass == null)
	    keyPass = (char[])objs[1];

	Certificate userCert = keyStore.getCertificate(alias);
	if (userCert == null) {
	    MessageFormat form = new MessageFormat
		(rb.getString("alias has no public key (certificate)"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}

	// Read the certificates in the reply
	Collection c = cf.generateCertificates(in);
	if (c.isEmpty()) {
	    throw new Exception(rb.getString("Reply has no certificates"));
	}
	Certificate[] replyCerts = 
	    (Certificate[]) c.toArray (new Certificate[c.size()]);
	Certificate[] newChain;
	if (replyCerts.length == 1) {
	    // single-cert reply
	    newChain = establishCertChain(userCert, replyCerts[0]);
	} else {
	    // cert-chain reply (e.g., PKCS#7)
	    newChain = validateReply(alias, userCert, replyCerts);
	}

	// Now store the newly established chain in the keystore. The new
	// chain replaces the old one.
	if (newChain != null) {
	    keyStore.setKeyEntry(alias, privKey,
				 (keyPass != null) ? keyPass : storePass,
				 newChain);
	    return true;
	} else {
	    return false;	    
	}
    }

    /**
     * Imports a certificate and adds it to the list of trusted certificates.
     *
     * @return true if the certificate was added, otherwise false.
     */
    private boolean addTrustedCert(String alias, InputStream in)
	throws Exception
    {
	if (alias == null) {
	    throw new Exception(rb.getString("Must specify alias"));
	}
	if (keyStore.containsAlias(alias) == true) {
	    MessageFormat form = new MessageFormat(rb.getString
		("Certificate not imported, alias <alias> already exists"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}

	// Read the certificate
	X509Certificate cert = null;
	try {
	    cert = (X509Certificate)cf.generateCertificate(in);
	} catch (ClassCastException cce) {
	    throw new Exception(rb.getString("Input not an X.509 certificate"));
	} catch (CertificateException ce) {
	    throw new Exception(rb.getString("Input not an X.509 certificate"));
	}

	// if certificate is self-signed, make sure it verifies
	boolean selfSigned = false;
	if (isSelfSigned(cert)) {
	    cert.verify(cert.getPublicKey());
	    selfSigned = true;
	}

	if (noprompt) {
	    keyStore.setCertificateEntry(alias, cert);
	    return true;
	}

	// check if cert already exists in keystore
	String reply = null;
	String trustalias = keyStore.getCertificateAlias(cert);
	if (trustalias != null) {
	    MessageFormat form = new MessageFormat(rb.getString
		("Certificate already exists in keystore under alias <trustalias>"));
	    Object[] source = {trustalias};
	    System.err.println(form.format(source));
	    reply = getYesNoReply
		(rb.getString("Do you still want to add it? [no]:  "));
	} else if (selfSigned) {
	    if (trustcacerts && (caks != null) &&
		    ((trustalias=caks.getCertificateAlias(cert)) != null)) {
		MessageFormat form = new MessageFormat(rb.getString
			("Certificate already exists in system-wide CA keystore under alias <trustalias>"));
		Object[] source = {trustalias};
		System.err.println(form.format(source));
		reply = getYesNoReply
			(rb.getString("Do you still want to add it to your own keystore? [no]:  "));
	    }
	    if (trustalias == null) {
		// Print the cert and ask user if they really want to add
		// it to their keystore
		printX509Cert(cert, System.out);
		reply = getYesNoReply
			(rb.getString("Trust this certificate? [no]:  "));
	    }
	}
	if (reply != null) {
	    if (reply.equals("YES")) {
		keyStore.setCertificateEntry(alias, cert);
		return true;
	    } else {
		return false;
	    }
	}

	// Try to establish trust chain
	try {
	    Certificate[] chain = establishCertChain(null, cert);
	    if (chain != null) {
		keyStore.setCertificateEntry(alias, cert);
		return true;
	    }
	} catch (Exception e) {
	    // Print the cert and ask user if they really want to add it to
	    // their keystore
	    printX509Cert(cert, System.out);
	    reply = getYesNoReply
		(rb.getString("Trust this certificate? [no]:  "));
	    if (reply.equals("YES")) {
		keyStore.setCertificateEntry(alias, cert);
		return true;
	    } else {
		return false;
	    }
	}

	return false;
    }

    /**
     * Prompts user for new password. New password must be different from
     * old one.
     *
     * @param prompt the message that gets prompted on the screen
     * @param oldPasswd the current (i.e., old) password
     */
    private char[] getNewPasswd(String prompt, char[] oldPasswd)
	throws Exception
    {
	char[] entered = null;
	char[] reentered = null;

	for (int count = 0; count < 3; count++) {
	    MessageFormat form = new MessageFormat
		(rb.getString("New prompt: "));
	    Object[] source = {prompt};
	    System.err.print(form.format(source));
	    entered = Password.readPassword(System.in);
	    if (entered.length < 6) {
		System.err.println(rb.getString
		    ("Password is too short - must be at least 6 characters"));
	    } else if (Arrays.equals(entered, oldPasswd)) {
		System.err.println(rb.getString("Passwords must differ"));
	    } else {
		form = new MessageFormat
			(rb.getString("Re-enter new prompt: "));
		Object[] src = {prompt};
		System.err.print(form.format(src));
		reentered = Password.readPassword(System.in);
		if (!Arrays.equals(entered, reentered)) {
		    System.err.println
			(rb.getString("They don't match; try again"));
		} else {
		    Arrays.fill(reentered, ' ');
		    return entered;
		}
	    }
	    if (entered != null) {
		Arrays.fill(entered, ' ');
		entered = null;
	    }
	    if (reentered != null) {
		Arrays.fill(reentered, ' ');
		reentered = null;
	    }
	}
	throw new Exception(rb.getString("Too many failures - try later"));
    }

    /**
     * Prompts user for alias name.
     */
    private String getAlias(String prompt) throws Exception {
	if (prompt != null) {
	    MessageFormat form = new MessageFormat
		(rb.getString("Enter prompt alias name:  "));
	    Object[] source = {prompt};
	    System.err.print(form.format(source));
	} else {
	    System.err.print(rb.getString("Enter alias name:  "));
	}
	return (new BufferedReader(new InputStreamReader(
                                        System.in))).readLine();
    }

    /**
     * Prompts user for key password. User may select to choose the same
     * password (<code>otherKeyPass</code>) as for <code>otherAlias</code>.
     */
    private char[] getKeyPasswd(String alias, String otherAlias,
				char[] otherKeyPass)
	throws Exception
    {
	int count = 0;
	char[] keyPass = null;

	do {
	    if (otherKeyPass != null) {
		MessageFormat form = new MessageFormat(rb.getString
			("Enter key password for <alias>"));
		Object[] source = {alias};
		System.err.println(form.format(source));
		
		form = new MessageFormat(rb.getString
			("\t(RETURN if same as for <otherAlias>)"));
		Object[] src = {otherAlias};
		System.err.print(form.format(src));
	    } else {
		MessageFormat form = new MessageFormat(rb.getString
			("Enter key password for <alias>"));
		Object[] source = {alias};
		System.err.print(form.format(source));
	    }
	    System.err.flush();
	    keyPass = Password.readPassword(System.in);
	    if (keyPass == null) {
		keyPass = otherKeyPass;
	    }
	    count++;
	} while ((keyPass == null) && count < 3);

	if (keyPass == null) {
	    throw new Exception(rb.getString("Too many failures - try later"));
	}

	return keyPass;
    }

    /**
     * Prints a certificate in a human readable format.
     */
    private void printX509Cert(X509Certificate cert, PrintStream out)
	throws Exception
    {
	/*
	out.println("Owner: "
		    + cert.getSubjectDN().toString()
		    + "\n"
		    + "Issuer: "
		    + cert.getIssuerDN().toString()
		    + "\n"
		    + "Serial number: " + cert.getSerialNumber().toString(16)
		    + "\n"
		    + "Valid from: " + cert.getNotBefore().toString()
		    + " until: " + cert.getNotAfter().toString()
		    + "\n"
		    + "Certificate fingerprints:\n"
		    + "\t MD5:  " + getCertFingerPrint("MD5", cert)
		    + "\n"
		    + "\t SHA1: " + getCertFingerPrint("SHA1", cert));
	*/

	MessageFormat form = new MessageFormat
		(rb.getString("*PATTERN* printX509Cert"));
	Object[] source = {cert.getSubjectDN().toString(),
			cert.getIssuerDN().toString(),
			cert.getSerialNumber().toString(16),
			cert.getNotBefore().toString(),
			cert.getNotAfter().toString(),
			getCertFingerPrint("MD5", cert),
			getCertFingerPrint("SHA1", cert)};
	out.println(form.format(source));
    }

    /**
     * Returns true if the certificate is self-signed, false otherwise.
     */
    private boolean isSelfSigned(X509Certificate cert) {
	return cert.getSubjectDN().equals(cert.getIssuerDN());
    }

    /**
     * Returns true if the given certificate is trusted, false otherwise.
     */
    private boolean isTrusted(Certificate cert)
	throws Exception
    {
	if (keyStore.getCertificateAlias(cert) != null)
	    return true; // found in own keystore
	if (trustcacerts && (caks != null) &&
	        (caks.getCertificateAlias(cert) != null))
	    return true; // found in CA keystore
	return false;
    }

    /**
     * Gets an X.500 name suitable for inclusion in a certification request.
     */
    private X500Name getX500Name() throws IOException {
	BufferedReader in;
	in = new BufferedReader(new InputStreamReader(System.in));
	String commonName = "Unknown";
	String organizationalUnit = "Unknown";
	String organization = "Unknown";
	String city = "Unknown";
	String state = "Unknown";
	String country = "Unknown";
	X500Name name;
	String userInput = null;

	do {
	    commonName = inputString(in,
		    rb.getString("What is your first and last name?"),
		    commonName);
	    organizationalUnit = inputString(in,
		    rb.getString
			("What is the name of your organizational unit?"),
		    organizationalUnit);
	    organization = inputString(in,
		    rb.getString("What is the name of your organization?"),
		    organization);
	    city = inputString(in,
		    rb.getString("What is the name of your City or Locality?"),
		    city);
	    state = inputString(in,
		    rb.getString("What is the name of your State or Province?"),
		    state);
	    country = inputString(in,
		    rb.getString
			("What is the two-letter country code for this unit?"),
		    country);
	    name = new X500Name(commonName, organizationalUnit, organization,
				city, state, country);
	    MessageFormat form = new MessageFormat
		(rb.getString("Is <name> correct?"));
	    Object[] source = {name};
	    userInput = inputString
		(in, form.format(source), rb.getString("no"));
	} while (collator.compare(userInput, rb.getString("yes")) != 0 &&
		 collator.compare(userInput, rb.getString("y")) != 0);

	System.err.println();
	return name;
    }

    private String inputString(BufferedReader in, String prompt,
			       String defaultValue)
	throws IOException
    {
	System.err.println(prompt);
	MessageFormat form = new MessageFormat
		(rb.getString("  [defaultValue]:  "));
	Object[] source = {defaultValue};
	System.err.print(form.format(source));
	System.err.flush();

	String value = in.readLine();
	if (value == null || collator.compare(value, "") == 0)
	    value = defaultValue;
	return value;
    }

    /**
     * Writes an X.509 certificate in base64 or binary encoding to an output
     * stream.
     */
    private void dumpCert(Certificate cert, PrintStream out)
	throws IOException, CertificateException
    {
	if (rfc) {
	    BASE64Encoder encoder = new BASE64Encoder();
	    out.println(X509Factory.BEGIN_CERT);
	    encoder.encodeBuffer(cert.getEncoded(), out);
	    out.println(X509Factory.END_CERT);
	} else {
	    out.write(cert.getEncoded()); // binary
	}
    }

    /**
     * Converts a byte to hex digit and writes to the supplied buffer
     */
    private void byte2hex(byte b, StringBuffer buf) {
        char[] hexChars = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
                            '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        int high = ((b & 0xf0) >> 4);
        int low = (b & 0x0f);
        buf.append(hexChars[high]);
        buf.append(hexChars[low]);
    }

    /**
     * Converts a byte array to hex string
     */
    private String toHexString(byte[] block) {
        StringBuffer buf = new StringBuffer();
	int len = block.length;
        for (int i = 0; i < len; i++) {
             byte2hex(block[i], buf);
	     if (i < len-1) {
		 buf.append(":");
	     }
        } 
        return buf.toString();
    }

    /**
     * Recovers (private) key associated with given alias.
     *
     * @return an array of objects, where the 1st element in the array is the
     * recovered private key, and the 2nd element is the password used to
     * recover it.
     */
    private Object[] recoverPrivateKey(String alias, char[] storePass,
				       char[] keyPass)
	throws Exception
    {
	Key key = null;

	if (keyStore.containsAlias(alias) == false) {
	    MessageFormat form = new MessageFormat
		(rb.getString("Alias <alias> does not exist"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}
	if (keyStore.isKeyEntry(alias) == false) {
	    MessageFormat form = new MessageFormat
		(rb.getString("Alias <alias> has no (private) key"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}

	if (keyPass == null) {
	    // Try to recover the key using the keystore password
	    try {
		key = keyStore.getKey(alias, storePass);
		keyPass = storePass;
	    } catch (UnrecoverableKeyException e) {
		// Did not work out, so prompt user for key password
		if (!token) {
		    keyPass = getKeyPasswd(alias, null, null);
		    key = keyStore.getKey(alias, keyPass);
		}
		throw e;
	    }
	} else {
	    key = keyStore.getKey(alias, keyPass);
	}
	if (!(key instanceof PrivateKey)) {
	    throw new Exception
		(rb.getString("Recovered key is not a private key"));
	}
	return new Object[] {(PrivateKey)key, keyPass};
    }

    /**
     * Gets the requested finger print of the certificate.
     */
    private String getCertFingerPrint(String mdAlg, Certificate cert)
	throws Exception
    {
	byte[] encCertInfo = cert.getEncoded();
	MessageDigest md = MessageDigest.getInstance(mdAlg);
	byte[] digest = md.digest(encCertInfo);
        return toHexString(digest); 
    }

    /**
     * Prints warning about missing integrity check.
     */
    private void printWarning() {
	System.err.println();
	System.err.println(rb.getString
	    ("*****************  WARNING WARNING WARNING  *****************"));
	System.err.println(rb.getString
	    ("* The integrity of the information stored in your keystore  *"));
	System.err.println(rb.getString
	    ("* has NOT been verified!  In order to verify its integrity, *"));
	System.err.println(rb.getString
	    ("* you must provide your keystore password.                  *"));
	System.err.println(rb.getString
	    ("*****************  WARNING WARNING WARNING  *****************"));
	System.err.println();
    }

    /**
     * Validates chain in certification reply, and returns the ordered
     * elements of the chain (with user certificate first, and root
     * certificate last in the array).
     *
     * @param alias the alias name
     * @param userCert the user certificate of the alias
     * @param replyCerts the chain provided in the reply
     */
    private Certificate[] validateReply(String alias,
					Certificate userCert,
					Certificate[] replyCerts)
	throws Exception
    {
	// order the certs in the reply (bottom-up).
	// we know that all certs in the reply are of type X.509, because
	// we parsed them using an X.509 certificate factory
	int i;
	PublicKey userPubKey = userCert.getPublicKey();
	for (i=0; i<replyCerts.length; i++) {
	    if (userPubKey.equals(replyCerts[i].getPublicKey()))
		break;
	}
	if (i == replyCerts.length) {
	    MessageFormat form = new MessageFormat(rb.getString
		("Certificate reply does not contain public key for <alias>"));
	    Object[] source = {alias};
	    throw new Exception(form.format(source));
	}

	Certificate tmpCert = replyCerts[0];
	replyCerts[0] = replyCerts[i];
	replyCerts[i] = tmpCert;
	Principal issuer = ((X509Certificate)replyCerts[0]).getIssuerDN();

	for (i=1; i < replyCerts.length-1; i++) {
	    // find a cert in the reply whose "subject" is the same as the
	    // given "issuer"
	    int j;
	    for (j=i; j<replyCerts.length; j++) {
		Principal subject;
		subject = ((X509Certificate)replyCerts[j]).getSubjectDN();
		if (subject.equals(issuer)) {
		    tmpCert = replyCerts[i];
		    replyCerts[i] = replyCerts[j];
		    replyCerts[j] = tmpCert;
		    issuer = ((X509Certificate)replyCerts[i]).getIssuerDN();
		    break;
		}		
	    }
	    if (j == replyCerts.length) {
		throw new Exception
		    (rb.getString("Incomplete certificate chain in reply"));
	    }
	}
	
	// now verify each cert in the ordered chain
	for (i=0; i<replyCerts.length-1; i++) {
	    PublicKey pubKey = replyCerts[i+1].getPublicKey();
	    try {
		replyCerts[i].verify(pubKey);
	    } catch (Exception e) {
		throw new Exception(rb.getString
			("Certificate chain in reply does not verify: ") +
			e.getMessage());
	    }
	}

	if (noprompt)
	    return replyCerts;

	// do we trust the (root) cert at the top?
	Certificate topCert = replyCerts[replyCerts.length-1];
	if (!isTrusted(topCert)) {
	    boolean verified = false;
	    Certificate rootCert = null;
	    if (trustcacerts && (caks!= null)) {
		for (Enumeration aliases = caks.aliases();
		     aliases.hasMoreElements(); ) {
		    String name = (String)aliases.nextElement();
		    rootCert = caks.getCertificate(name);
		    if (rootCert != null) {
			try {
			    topCert.verify(rootCert.getPublicKey());
			    verified = true;
			    break;
			} catch (Exception e) {
			}
		    }
		}
	    }
	    if (!verified) {
		System.err.println();
		System.err.println
			(rb.getString("Top-level certificate in reply:\n"));
		printX509Cert((X509Certificate)topCert, System.out);
		System.err.println();
		System.err.print(rb.getString("... is not trusted. "));
		String reply = getYesNoReply
			(rb.getString("Install reply anyway? [no]:  ")); 
		if (reply.equals("NO")) {
		    return null;
		}
	    } else {
		if (!isSelfSigned((X509Certificate)topCert)) {
		    // append the (self-signed) root CA cert to the chain
		    Certificate[] tmpCerts =
			new Certificate[replyCerts.length+1];
		    System.arraycopy(replyCerts, 0, tmpCerts, 0,
				     replyCerts.length);
		    tmpCerts[tmpCerts.length-1] = rootCert;
		    replyCerts = tmpCerts;
		}
	    }
	}

	return replyCerts;
    }

    /**
     * Establishes a certificate chain (using trusted certificates in the
     * keystore), starting with the user certificate
     * and ending at a self-signed certificate found in the keystore.
     *
     * @param userCert the user certificate of the alias
     * @param certToVerify the single certificate provided in the reply
     */
    private Certificate[] establishCertChain(Certificate userCert,
					     Certificate certToVerify)
	throws Exception
    {
	if (userCert != null) {
	    // Make sure that the public key of the certificate reply matches
	    // the original public key in the keystore
	    PublicKey origPubKey = userCert.getPublicKey();
	    PublicKey replyPubKey = certToVerify.getPublicKey();
	    if (!origPubKey.equals(replyPubKey)) {
		throw new Exception(rb.getString
			("Public keys in reply and keystore don't match"));
	    }

	    // If the two certs are identical, we're done: no need to import
	    // anything
	    if (certToVerify.equals(userCert)) {
		throw new Exception(rb.getString
			("Certificate reply and certificate in keystore are identical"));
	    }
	}

	// Build a hash table of all certificates in the keystore.
	// Use the subject distinguished name as the key into the hash table.
	// All certificates associated with the same subject distinguished
	// name are stored in the same hash table entry as a vector.
	Hashtable certs = null;
	if (keyStore.size() > 0) {
	    certs = new Hashtable(11);
	    keystorecerts2Hashtable(keyStore, certs);
	}
	if (trustcacerts) {
	    if (caks!=null && caks.size()>0) {
		if (certs == null)
		    certs = new Hashtable(11);
		keystorecerts2Hashtable(caks, certs);
	    }
	}

	// start building chain
	Vector chain = new Vector(2);
	if (buildChain((X509Certificate)certToVerify, chain, certs)) {
	    Certificate[] newChain = new Certificate[chain.size()];
	    // buildChain() returns chain with self-signed root-cert first and
	    // user-cert last, so we need to invert the chain before we store
	    // it
	    int j=0;
	    for (int i=chain.size()-1; i>=0; i--) {
		newChain[j] = (Certificate)chain.elementAt(i);
		j++;
	    }
	    return newChain;
	} else {
	    throw new Exception
		(rb.getString("Failed to establish chain from reply"));
	}
    }

    /**
     * Recursively tries to establish chain from pool of trusted certs.
     *
     * @param certToVerify the cert that needs to be verified.
     * @param chain the chain that's being built.
     * @param certs the pool of trusted certs
     *
     * @return true if successful, false otherwise.
     */
    private boolean buildChain(X509Certificate certToVerify, Vector chain,
			       Hashtable certs) {
	Principal subject = certToVerify.getSubjectDN();
	Principal issuer = certToVerify.getIssuerDN();	
	if (subject.equals(issuer)) {
	    // reached self-signed root cert;
	    // no verification needed because it's trusted.
	    chain.addElement(certToVerify);
	    return true;
	}

	// Get the issuer's certificate(s)
	Vector vec = (Vector)certs.get(issuer);
	if (vec == null)
	    return false;
	
	// Try out each certificate in the vector, until we find one
	// whose public key verifies the signature of the certificate
	// in question.
	for (Enumeration issuerCerts = vec.elements();
	     issuerCerts.hasMoreElements(); ) {
	    X509Certificate issuerCert
		= (X509Certificate)issuerCerts.nextElement();
	    PublicKey issuerPubKey = issuerCert.getPublicKey();
	    try {
		certToVerify.verify(issuerPubKey);
	    } catch(Exception e) {
		continue;
	    }
	    if (buildChain(issuerCert, chain, certs)) {
		chain.addElement(certToVerify);
		return true;
	    }
	}
	return false;
    }

    /**
     * Prompts user for yes/no decision.
     *
     * @return the user's decision
     */
    private String getYesNoReply(String prompt)
	throws IOException
    {
	String reply = null;
	do {
	    System.err.print(prompt);
	    System.err.flush();
	    reply = (new BufferedReader(new InputStreamReader
					(System.in))).readLine();
	    if (collator.compare(reply, "") == 0 ||
		collator.compare(reply, rb.getString("n")) == 0 ||
		collator.compare(reply, rb.getString("no")) == 0) {
		reply = "NO";
	    } else if (collator.compare(reply, rb.getString("y")) == 0 ||
		       collator.compare(reply, rb.getString("yes")) == 0) {
		reply = "YES";
	    } else {
		System.err.println(rb.getString("Wrong answer, try again"));
		reply = null;
	    }
	} while (reply == null);
	return reply;
    }

    /**
     * Returns the keystore with the configured CA certificates.
     */
    private KeyStore getCacertsKeyStore()
	throws Exception
    {
	String sep = File.separator;
	File file = new File(System.getProperty("java.home") + sep
			     + "lib" + sep + "security" + sep
			     + "cacerts");
	if (!file.exists())
	    return null;
	FileInputStream fis = new FileInputStream(file);
	KeyStore caks = KeyStore.getInstance(JKS);
	caks.load(fis, null);
	fis.close();
	return caks;
    }

    /**
     * Stores the (leaf) certificates of a keystore in a hashtable.
     * All certs belonging to the same CA are stored in a vector that
     * in turn is stored in the hashtable, keyed by the CA's subject DN
     */
    private void keystorecerts2Hashtable(KeyStore ks, Hashtable hash)
	throws Exception
    {
	for (Enumeration aliases = ks.aliases(); aliases.hasMoreElements(); ) {
	    String alias = (String)aliases.nextElement();
	    Certificate cert = ks.getCertificate(alias);
	    if (cert != null) {
		Principal subjectDN = ((X509Certificate)cert).getSubjectDN();
		Vector vec = (Vector)hash.get(subjectDN);
		if (vec == null) {
		    vec = new Vector();
		    vec.addElement(cert);
		} else {
		    if (!vec.contains(cert))
			vec.addElement(cert);
		}
		hash.put(subjectDN, vec);
	    }
	}
    }

    /**
     * Prints the usage of this tool.
     */
    private void usage() {
	System.err.println(rb.getString("keytool usage:\n"));

	System.err.println(rb.getString
		("-certreq     [-v] [-protected]"));
	System.err.println(rb.getString
		("\t     [-alias <alias>] [-sigalg <sigalg>]"));
	System.err.println(rb.getString
		("\t     [-file <csr_file>] [-keypass <keypass>]"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
	System.err.println(rb.getString
		("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.err.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.err.println(rb.getString
		("-delete      [-v] [-protected] -alias <alias>"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
	System.err.println(rb.getString
		("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.err.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.err.println(rb.getString
		("-export      [-v] [-rfc] [-protected]"));
	System.err.println(rb.getString
		("\t     [-alias <alias>] [-file <cert_file>]"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
	System.err.println(rb.getString
		("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.err.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.err.println(rb.getString
		("-genkey      [-v] [-protected]"));
	System.err.println(rb.getString
		("\t     [-alias <alias>]"));
	System.err.println(rb.getString
		("\t     [-keyalg <keyalg>] [-keysize <keysize>]"));
	System.err.println(rb.getString
		("\t     [-sigalg <sigalg>] [-dname <dname>]"));
	System.err.println(rb.getString
		("\t     [-validity <valDays>] [-keypass <keypass>]"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
	System.err.println(rb.getString
		("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.err.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.err.println(rb.getString("-help"));
	System.err.println();

	System.err.println(rb.getString
		("-identitydb  [-v] [-protected]"));
	System.err.println(rb.getString
		("\t     [-file <idb_file>]"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
	System.err.println(rb.getString
		("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.out.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.err.println(rb.getString
		("-import      [-v] [-noprompt] [-trustcacerts] [-protected]"));
	System.err.println(rb.getString
		("\t     [-alias <alias>]"));
	System.err.println(rb.getString
		("\t     [-file <cert_file>] [-keypass <keypass>]"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
	System.err.println(rb.getString
		("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.err.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.err.println(rb.getString
		("-keyclone    [-v] [-protected]"));
	System.err.println(rb.getString
		("\t     [-alias <alias>] -dest <dest_alias>"));
	System.err.println(rb.getString
		("\t     [-keypass <keypass>] [-new <new_keypass>]"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
	System.err.println(rb.getString
		("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.err.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.err.println(rb.getString
		("-keypasswd   [-v] [-alias <alias>]"));
	System.err.println(rb.getString
		("\t     [-keypass <old_keypass>] [-new <new_keypass>]"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
	System.err.println(rb.getString
		("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.err.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.err.println(rb.getString
		("-list        [-v | -rfc] [-protected]"));
	System.err.println(rb.getString
		("\t     [-alias <alias>]"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
	System.err.println(rb.getString
		("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.err.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.err.println(rb.getString
		("-printcert   [-v] [-file <cert_file>]"));
	System.err.println();

	System.err.println(rb.getString
		("-selfcert    [-v] [-protected]"));
	System.err.println(rb.getString
		("\t     [-alias <alias>]"));
	System.err.println(rb.getString
		("\t     [-dname <dname>] [-validity <valDays>]"));
	System.err.println(rb.getString
		("\t     [-keypass <keypass>] [-sigalg <sigalg>]"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
	System.err.println(rb.getString
		("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.err.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.err.println(rb.getString
		("-storepasswd [-v] [-new <new_storepass>]"));
	System.err.println(rb.getString
		("\t     [-keystore <keystore>] [-storepass <storepass>]"));
        System.err.println(rb.getString
                ("\t     [-storetype <storetype>] [-providerName <name>]"));
	System.err.println(rb.getString
		("\t     [-providerClass <provider_class_name> [-providerArg <arg>]] ..."));
	System.err.println();

	System.exit(1);
    }
}
