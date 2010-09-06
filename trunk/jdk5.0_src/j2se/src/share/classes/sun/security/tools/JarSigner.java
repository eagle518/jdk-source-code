/*
 * @(#)JarSigner.java	1.64 04/04/21
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.tools;

import java.io.*;
import java.util.*;
import java.util.zip.*;
import java.util.jar.*;
import java.math.BigInteger;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.SocketTimeoutException;
import java.text.Collator;
import java.text.MessageFormat;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.*;
import java.security.interfaces.*;
import java.lang.reflect.Constructor;

import com.sun.jarsigner.ContentSigner;
import com.sun.jarsigner.ContentSignerParameters;
import sun.net.www.MessageHeader;
import sun.security.x509.*;
import sun.security.pkcs.*;
import sun.security.util.*;
import sun.misc.BASE64Encoder;

/**
 * <p>The jarsigner utility.
 *
 * @version 1.64 04/21/04
 * @author Roland Schemers
 * @author Jan Luehe
 */

public class JarSigner {

    // for i18n
    private static final java.util.ResourceBundle rb =
	java.util.ResourceBundle.getBundle
	("sun.security.tools.JarSignerResources");
    private static final Collator collator = Collator.getInstance();
    static {
	// this is for case insensitive string comparisions
	collator.setStrength(Collator.PRIMARY);
    }

    private static final String META_INF = "META-INF/";

    // prefix for new signature-related files in META-INF directory
    private static final String SIG_PREFIX = META_INF + "SIG-";

    private static final Class[] PARAM_STRING = { String.class };

    private static final String NONE = "NONE";
    private static final String P11KEYSTORE = "PKCS11";

    private static final long SIX_MONTHS = 180*24*60*60*1000L; //milliseconds

    public static void main(String args[]) throws Exception {
	JarSigner js = new JarSigner();
	js.run(args);
    }

    static final String VERSION = "1.0";

    static final int IN_KEYSTORE = 0x01;
    static final int IN_SCOPE = 0x02;

    // signer's certificate chain (when composing)
    X509Certificate[] certChain;

    /*
     * private key
     */
    PrivateKey privateKey;
    KeyStore store;

    IdentityScope scope;

    String keystore; // key store file
    boolean nullStream = false; // null keystore input stream (NONE)
    boolean token = false; // token-based keystore
    String jarfile;  // jar file to sign
    String alias;    // alias to sign jar with
    char[] storepass; // keystore password
    boolean protectedPath; // protected authentication path
    String storetype; // keystore type
    String providerName; // provider name
    Vector providers = null; // list of providers
    HashMap providerArgs = new HashMap(); // arguments for provider constructors
    char[] keypass; // private key password
    String sigfile; // name of .SF file
    String signedjar; // output filename
    String tsaUrl; // location of the Timestamping Authority
    String tsaAlias; // alias for the Timestamping Authority's certificate
    boolean verify = false; // verify the jar
    boolean verbose = false; // verbose output when signing/verifying
    boolean showcerts = false; // show certs when verifying
    boolean debug = false; // debug
    boolean signManifest = true; // "sign" the whole manifest
    boolean externalSF = true; // leave the .SF out of the PKCS7 block

    // read zip entry raw bytes
    private ByteArrayOutputStream baos = new ByteArrayOutputStream(2048);
    private byte[] buffer = new byte[8192];
    private ContentSigner signingMechanism = null;
    private String altSignerClass = null;
    private String altSignerClasspath = null;
    private ZipFile zipFile = null;
    private boolean hasExpiredCert = false;
    private boolean hasExpiringCert = false;
    private boolean notYetValidCert = false;

    public void run(String args[]) {
	try {
	    parseArgs(args);

	    // Try to load and install the specified providers
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
			MessageFormat form = new MessageFormat(rb.getString
			    ("provName not a provider"));
			Object[] source = {provName};
			throw new Exception(form.format(source));
		    }
		    Security.addProvider((Provider)obj);
		}
	    }

	    hasExpiredCert = false;
	    hasExpiringCert = false;
	    notYetValidCert = false;

	    if (verify) {
		try {
		    loadKeyStore(keystore, false);
		    scope = IdentityScope.getSystemScope();
		} catch (Exception e) {
		    if ((keystore != null) || (storepass != null)) {
			System.out.println(rb.getString("jarsigner error: ") +
					e.getMessage());
			System.exit(1);
		    }
		}
		/*		if (debug) {
		    SignatureFileVerifier.setDebug(true);
		    ManifestEntryVerifier.setDebug(true);
		}
		*/
		verifyJar(jarfile);
	    } else {
		loadKeyStore(keystore, true);
		getAliasInfo(alias);

		// load the alternative signing mechanism
		if (altSignerClass != null) {
		    signingMechanism = loadSigningMechanism(altSignerClass,
			altSignerClasspath);
		}
		signJar(jarfile, alias, args);
	    }
	} catch (Exception e) {
	    System.out.println(rb.getString("jarsigner error: ") + e);
	    if (debug) {
		e.printStackTrace();
	    }
	    System.exit(1);
 	} finally {
	    // zero-out private key password
	    if (keypass != null) {
		Arrays.fill(keypass, ' ');
		keypass = null;
	    }
	    // zero-out keystore password
	    if (storepass != null) {
		Arrays.fill(storepass, ' ');
		storepass = null;
	    }
	}
    }

    /*
     * Parse command line arguments.
     */
    void parseArgs(String args[]) {
	/* parse flags */
	int n = 0;

	for (n=0; (n < args.length) && args[n].startsWith("-"); n++) {

	    String flags = args[n];

	    if (collator.compare(flags, "-keystore") == 0) {
		if (++n == args.length) usage();
		keystore = args[n];
	    } else if (collator.compare(flags, "-storepass") ==0) {
		if (++n == args.length) usage();
		storepass = args[n].toCharArray();
	    } else if (collator.compare(flags, "-storetype") ==0) {
		if (++n == args.length) usage();
		storetype = args[n];
	    } else if (collator.compare(flags, "-providerName") ==0) {
		if (++n == args.length) usage();
		providerName = args[n];
	    } else if ((collator.compare(flags, "-provider") == 0) ||
			(collator.compare(flags, "-providerClass") == 0)) {
		if (++n == args.length) usage();
		if (providers == null) {
		    providers = new Vector(3);
		}
		providers.add(args[n]);

		if (args.length > (n+1)) {
		    flags = args[n+1];
		    if (collator.compare(flags, "-providerArg") == 0) {
			if (args.length == (n+2)) usage();
			providerArgs.put(args[n], args[n+2]);
			n += 2;
		    }
		}
	    } else if (collator.compare(flags, "-protected") ==0) {
		protectedPath = true;
	    } else if (collator.compare(flags, "-debug") ==0) {
		debug = true;
	    } else if (collator.compare(flags, "-keypass") ==0) {
		if (++n == args.length) usage();
		keypass = args[n].toCharArray();
	    } else if (collator.compare(flags, "-sigfile") ==0) {
		if (++n == args.length) usage();
		sigfile = args[n];
	    } else if (collator.compare(flags, "-signedjar") ==0) {
		if (++n == args.length) usage();
		signedjar = args[n];
	    } else if (collator.compare(flags, "-tsa") ==0) {
		if (++n == args.length) usage();
		tsaUrl = args[n];
	    } else if (collator.compare(flags, "-tsacert") ==0) {
		if (++n == args.length) usage();
		tsaAlias = args[n];
	    } else if (collator.compare(flags, "-altsigner") ==0) {
		if (++n == args.length) usage();
		altSignerClass = args[n];
	    } else if (collator.compare(flags, "-altsignerpath") ==0) {
		if (++n == args.length) usage();
		altSignerClasspath = args[n];
	    } else if (collator.compare(flags, "-sectionsonly") ==0) {
		signManifest = false;
	    } else if (collator.compare(flags, "-internalsf") ==0) {
		externalSF = false;
	    } else if (collator.compare(flags, "-verify") ==0) {
		verify = true;
	    } else if (collator.compare(flags, "-verbose") ==0) {
		verbose = true;
	    } else if (collator.compare(flags, "-certs") ==0) {
		showcerts = true;
	    } else if (collator.compare(flags, "-h") == 0 ||
			collator.compare(flags, "-help") == 0) {
		usage();
	    } else {
		System.err.println(rb.getString("Illegal option: ") + flags);
		usage();
	    }
	}

	if (n == args.length) usage();
	jarfile = args[n++];

	if (!verify) {
	    if (n == args.length) usage();
	    alias = args[n++];
	}

	if (NONE.equals(keystore)) {
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

	if (token && keypass != null) {
	    System.err.println(rb.getString
		("-keypass can not be specified " +
		"if -storetype is " + P11KEYSTORE));
	    System.err.println();
	    usage();
	}

	if (protectedPath) {
	    if (storepass != null || keypass != null) {
		System.err.println(rb.getString
			("If -protected is specified, " +
			"then -storepass and -keypass must not be specified"));
		System.err.println();
		usage();
	    }
	}
    }

    void usage() {
	System.out.println(rb.getString
		("Usage: jarsigner [options] jar-file alias"));
	System.out.println(rb.getString
		("       jarsigner -verify [options] jar-file"));
	System.out.println();
	System.out.println(rb.getString
		("[-keystore <url>]           keystore location"));
	System.out.println();
	System.out.println(rb.getString
		("[-storepass <password>]     password for keystore integrity"));
	System.out.println();
	System.out.println(rb.getString
		("[-storetype <type>]         keystore type"));
	System.out.println();
	System.out.println(rb.getString
		("[-keypass <password>]       password for private key (if different)"));
	System.out.println();
	System.out.println(rb.getString
		("[-sigfile <file>]           name of .SF/.DSA file"));
	System.out.println();
	System.out.println(rb.getString
		("[-signedjar <file>]         name of signed JAR file"));
	System.out.println();
	System.out.println(rb.getString
		("[-verify]                   verify a signed JAR file"));
	System.out.println();
	System.out.println(rb.getString
		("[-verbose]                  verbose output when signing/verifying"));
	System.out.println();
	System.out.println(rb.getString
		("[-certs]                    display certificates when verbose and verifying"));
	System.out.println();
	System.out.println(rb.getString
		("[-tsa <url>]                location of the Timestamping Authority"));
	System.out.println();
	System.out.println(rb.getString
		("[-tsacert <alias>]          public key certificate for Timestamping Authority"));
	System.out.println();
	System.out.println(rb.getString
		("[-altsigner <class>]        class name of an alternative signing mechanism"));
	System.out.println();
	System.out.println(rb.getString
		("[-altsignerpath <pathlist>] location of an alternative signing mechanism"));
	System.out.println();
	System.out.println(rb.getString
		("[-internalsf]               include the .SF file inside the signature block"));
	System.out.println();
	System.out.println(rb.getString
		("[-sectionsonly]             don't compute hash of entire manifest"));
	System.out.println();
	System.out.println(rb.getString
		("[-protected]                keystore has protected authentication path"));
	System.out.println();
	System.out.println(rb.getString
		("[-providerName <name>]      provider name"));
	System.out.println();
	System.out.println(rb.getString
		("[-providerClass <class>     name of cryptographic service provider's"));
	System.out.println(rb.getString
		("  [-providerArg <arg>]] ... master class file and constructor argument"));
	System.out.println();

	System.exit(1);
    }

    void verifyJar(String jarName)
	throws Exception
    {
	boolean anySigned = false;
	boolean hasUnsignedEntry = false;
	try {

	    JarFile jf = new JarFile(jarName, true);

	    Vector entriesVec = new Vector();
	    byte[] buffer = new byte[8192];

	    Enumeration entries = jf.entries();
	    while (entries.hasMoreElements()) {
		JarEntry je = (JarEntry)entries.nextElement();
		entriesVec.addElement(je);
		InputStream is = jf.getInputStream(je);
		int n;
		while ((n = is.read(buffer, 0, buffer.length)) != -1) {
		    // we just read. this will throw a SecurityException
		    // if  a signature/digest check fails.
		}
		is.close();
	    }

	    Manifest man = jf.getManifest();
	    // Don't use "jf" after it's been closed!
	    jf.close();

	    if (man != null) {
		if (verbose) System.out.println();
		Enumeration e = entriesVec.elements();

		long now = System.currentTimeMillis();

		while (e.hasMoreElements()) {
		    JarEntry je = (JarEntry) e.nextElement();
		    String name = je.getName();
		    CodeSigner[] signers = je.getCodeSigners();
		    boolean isSigned = (signers != null);
		    anySigned |= isSigned;
		    hasUnsignedEntry |= !je.isDirectory() && !isSigned 
					&& !signatureRelated(name);
		   
		    if (verbose) {
			int inStoreOrScope = inKeyStore(signers);
			boolean inStore = (inStoreOrScope & IN_KEYSTORE) != 0;
			boolean inScope = (inStoreOrScope & IN_SCOPE) != 0;
			boolean inManifest =
			    ((man.getAttributes(name) != null) ||
			     (man.getAttributes("./"+name) != null) ||
			     (man.getAttributes("/"+name) != null));
			System.out.print(
			  (isSigned ? rb.getString("s") : rb.getString(" ")) +
			  (inManifest ? rb.getString("m") : rb.getString(" ")) +
			  (inStore ? rb.getString("k") : rb.getString(" ")) +
			  (inScope ? rb.getString("i") : rb.getString(" ")) +
			  rb.getString("  "));
			StringBuffer sb = new StringBuffer();
			String s = Long.toString(je.getSize());
			for (int i = 6 - s.length(); i > 0; --i) {
			    sb.append(' ');
			}
			sb.append(s).append(' ').
			            append(new Date(je.getTime()).toString());
			sb.append(' ').append(je.getName());
			System.out.println(sb.toString());

			if (signers != null && showcerts) {
			    String tab = rb.getString("      ");
			    for (int i = 0; i < signers.length; i++) {
				System.out.println();
				List<Certificate> certs = (List<Certificate>)
				    signers[i].getSignerCertPath()
					.getCertificates();
				// display the signature timestamp, if present
				Timestamp timestamp = signers[i].getTimestamp();
				if (timestamp != null) {
				    System.out.println(
					printTimestamp(tab, timestamp));
				}
				// display the certificate(s)
				for (Certificate c : certs) {
				    System.out.println(
					printCert(tab, c, true, now));
				}
			    }
			    System.out.println();
			}

		    }
		    if (isSigned && !showcerts) {
			for (int i = 0; i < signers.length; i++) {
			    Certificate cert = 
				signers[i].getSignerCertPath()
				    .getCertificates().get(0);
			    if (cert instanceof X509Certificate) {
				long notAfter = ((X509Certificate)cert)
				    .getNotAfter().getTime();

				if (notAfter < now) {
				    hasExpiredCert = true;
				} else if (notAfter < now + SIX_MONTHS) {
				    hasExpiringCert = true;
				}
			    }
			}
		    }
		}
	    }
	    if (verbose) {
		System.out.println();
		System.out.println(rb.getString(
		    "  s = signature was verified "));
		System.out.println(rb.getString(
		    "  m = entry is listed in manifest"));
		System.out.println(rb.getString(
		    "  k = at least one certificate was found in keystore"));
		System.out.println(rb.getString(
		    "  i = at least one certificate was found in identity scope"));
		System.out.println();
	    }

	    if (man == null)
		System.out.println(rb.getString("no manifest."));

	    if (!anySigned) {
		System.out.println(rb.getString(
                      "jar is unsigned. (signatures missing or not parsable)"));
	    } else {
		System.out.println(rb.getString("jar verified."));
		if (hasUnsignedEntry || hasExpiredCert || hasExpiringCert ||
		    notYetValidCert) {

		    System.out.println();
		    System.out.print(rb.getString("Warning: "));
		    if (hasUnsignedEntry) {
			System.out.print(rb.getString(
			    "This jar contains unsigned entries which have not been integrity-checked. "));
			if (! verbose) {
			    System.out.println(rb.getString(
				"Re-run with the -verbose option for more details."));
			} else {
			    System.out.println();
			}
		    }
		    if (hasExpiredCert) {
			System.out.print(rb.getString(
			    "This jar contains entries whose signer certificate has expired. "));
			if (! (verbose && showcerts)) {
			    System.out.println(rb.getString(
			       "Re-run with the -verbose and -certs options for more details."));
			} else {
			    System.out.println();
			}
		    }
		    if (hasExpiringCert) {
			System.out.print(rb.getString(
			    "This jar contains entries whose signer certificate will expire within six months. "));
			if (! (verbose && showcerts)) {
			    System.out.println(rb.getString(
				"Re-run with the -verbose and -certs options for more details."));
			} else {
			    System.out.println();
			}
		    }
		    if (notYetValidCert) {
			System.out.print(rb.getString(
			    "This jar contains entries whose signer certificate is not yet valid. "));
			if (! (verbose && showcerts)) {
			    System.out.println(rb.getString(
				"Re-run with the -verbose and -certs options for more details."));
			} else {
			    System.out.println();
			}
		    }
		}
	    }
	    System.exit(0);
	} catch (Exception e) {
	    System.out.println(rb.getString("jarsigner: ") + e);
	    if (debug) {
		e.printStackTrace();
	    }
	    System.exit(1);
	}
	System.exit(1);
    }

    /*
     * Display some details about a certificate:
     *
     * <cert-type> [", " <subject-DN>] [" (" <keystore-entry-alias> ")"]
     */
    String printCert(Certificate c) {
	return printCert("", c, false, 0);
    }

    private static MessageFormat validityTimeForm = null;
    private static MessageFormat notYetTimeForm = null;
    private static MessageFormat expiredTimeForm = null;
    private static MessageFormat expiringTimeForm = null;

    /*
     * Display some details about a certificate:
     *
     * [<tab>] <cert-type> [", " <subject-DN>] [" (" <keystore-entry-alias> ")"]
     * [<validity-period> | <expiry-warning>]
     */
    String printCert(String tab, Certificate c, boolean checkValidityPeriod,
	long now) {

	StringBuilder certStr = new StringBuilder();
	String space = rb.getString(" ");
	X509Certificate x509Cert = null;

	if (c instanceof X509Certificate) {
	    x509Cert = (X509Certificate) c;
	    certStr.append(tab).append(x509Cert.getType())
		.append(rb.getString(", "))
		.append(x509Cert.getSubjectDN().getName());
	} else {
	    certStr.append(tab).append(c.getType());
	}

	String alias = (String) storeHash.get(c);
	if (alias != null) {
	    certStr.append(space).append(alias);
	}

	if (checkValidityPeriod && x509Cert != null) {

	    certStr.append("\n").append(tab).append("[");
	    Date notAfter = x509Cert.getNotAfter();
	    try {
		x509Cert.checkValidity();
		// test if cert will expire within six months
		if (now == 0) {
		    now = System.currentTimeMillis();
		}
		if (notAfter.getTime() < now + SIX_MONTHS) {
		    hasExpiringCert = true;

		    if (expiringTimeForm == null) {
			expiringTimeForm = new MessageFormat(
			    rb.getString("certificate will expire on"));
		    }
		    Object[] source = { notAfter };
		    certStr.append(expiringTimeForm.format(source));

		} else {
		    if (validityTimeForm == null) {
			validityTimeForm = new MessageFormat(
			    rb.getString("certificate is valid from"));
		    }
		    Object[] source = { x509Cert.getNotBefore(), notAfter };
		    certStr.append(validityTimeForm.format(source));
		}
	    } catch (CertificateExpiredException cee) {
		hasExpiredCert = true;

		if (expiredTimeForm == null) {
		    expiredTimeForm = new MessageFormat(
			rb.getString("certificate expired on"));
		}
		Object[] source = { notAfter };
		certStr.append(expiredTimeForm.format(source));

	    } catch (CertificateNotYetValidException cnyve) {
		notYetValidCert = true;

		if (notYetTimeForm == null) {
		    notYetTimeForm = new MessageFormat(
			rb.getString("certificate is not valid until"));
		}
		Object[] source = { x509Cert.getNotBefore() };
		certStr.append(notYetTimeForm.format(source));
	    }
	    certStr.append("]");
	}
	return certStr.toString();
    }

    private static MessageFormat signTimeForm = null;

    private String printTimestamp(String tab, Timestamp timestamp) {

	if (signTimeForm == null) {
	    signTimeForm = 
		new MessageFormat(rb.getString("entry was signed on"));
	}
	Object[] source = { timestamp.getTimestamp() };

	return new StringBuilder().append(tab).append("[")
	    .append(signTimeForm.format(source)).append("]").toString();
    }

    Hashtable storeHash = new Hashtable();

    int inKeyStore(CodeSigner[] signers) {
	int result = 0;

	if (signers == null)
	    return 0;

	boolean found = false;

	for (int i = 0; i < signers.length; i++) {
	    found = false;
	    List<Certificate> certs = (List<Certificate>)
		signers[i].getSignerCertPath().getCertificates();

	    for (Certificate c : certs) {
		String alias = (String) storeHash.get(c);

	        if (alias != null) {
		    if (alias.startsWith("("))
			    result |= IN_KEYSTORE;
		    else if (alias.startsWith("["))
			    result |= IN_SCOPE;
	        } else {
		    if (store != null) {
		        try {
			    alias = store.getCertificateAlias(c);
		        } catch (KeyStoreException kse) {
			    // never happens, because keystore has been loaded
		        }
		        if (alias != null) {
			    storeHash.put(c, "("+alias+")");
			    found = true;
			    result |= IN_KEYSTORE;
		        }
		    }
		    if (!found && (scope != null)) {
		        Identity id = scope.getIdentity(c.getPublicKey());
		        if (id != null) {
			    result |= IN_SCOPE;
			    storeHash.put(c, "["+id.getName()+"]");
		        }
		    }
		}
	    }
	}
	return result;
    }

    void signJar(String jarName, String alias, String[] args) throws Exception {
	boolean aliasUsed = false;
	X509Certificate tsaCert = null;

	if (sigfile == null) {
	    sigfile = alias;
	    aliasUsed = true;
	}

	if (sigfile.length() > 8) {
	    sigfile = sigfile.substring(0, 8).toUpperCase();
	} else {
	    sigfile = sigfile.toUpperCase();
	}

	StringBuffer tmpSigFile = new StringBuffer(sigfile.length());
	for (int j = 0; j < sigfile.length(); j++) {
	    char c = sigfile.charAt(j);
	    if (!
		((c>= 'A' && c<= 'Z') ||
		(c>= '0' && c<= '9') ||
		(c == '-') ||
		(c == '_'))) {
		if (aliasUsed) {
		    // convert illegal characters from the alias to be _'s
		    c = '_';
		} else {
		 throw new
                   RuntimeException(rb.getString
			("signature filename must consist of the following characters: A-Z, 0-9, _ or -"));
		}
	    }
	    tmpSigFile.append(c);
	}

	sigfile = tmpSigFile.toString();

	String tmpJarName;
	if (signedjar == null) tmpJarName = jarName+".sig";
	else tmpJarName = signedjar;

	File jarFile = new File(jarName);
	File signedJarFile = new File(tmpJarName);

	// Open the jar (zip) file
	try {
	    zipFile = new ZipFile(jarName);
	} catch (IOException ioe) {
	    error(rb.getString("unable to open jar file: ")+jarName, ioe);
	}

	FileOutputStream fos = null;
	try {
	    fos = new FileOutputStream(signedJarFile);
	} catch (IOException ioe) {
	    error(rb.getString("unable to create: ")+tmpJarName, ioe);
	}

	PrintStream ps = new PrintStream(fos);
	ZipOutputStream zos = new ZipOutputStream(ps);

	/* First guess at what they might be - we don't xclude RSA ones. */
	String sfFilename = (META_INF + sigfile + ".SF").toUpperCase();
	String bkFilename = (META_INF + sigfile + ".DSA").toUpperCase();

	Manifest manifest = new Manifest();
	Map mfEntries = manifest.getEntries();

	boolean mfModified = false;
	boolean mfCreated = false;
	byte[] mfRawBytes = null;

	try {
	    // For now, hard-code the message digest algorithm to SHA-1
	    MessageDigest digests[] = { MessageDigest.getInstance("SHA1") };

	    // Check if manifest exists
	    ZipEntry mfFile;
	    if ((mfFile = getManifestFile(zipFile)) != null) {
		// Manifest exists. Read its raw bytes.
		mfRawBytes = getBytes(zipFile, mfFile);
		manifest.read(new ByteArrayInputStream(mfRawBytes));
	    } else {
		// Create new manifest
		Attributes mattr = manifest.getMainAttributes();
		mattr.putValue(Attributes.Name.MANIFEST_VERSION.toString(),
			       "1.0");
		String javaVendor = System.getProperty("java.vendor");
		String jdkVersion = System.getProperty("java.version");
		mattr.putValue("Created-By", jdkVersion + " (" +javaVendor
			       + ")");
		mfFile = new ZipEntry(JarFile.MANIFEST_NAME);
		mfCreated = true;
	    }

	    /*
	     * For each entry in jar
	     * (except for signature-related META-INF entries),
	     * do the following:
	     *
	     * - if entry is not contained in manifest, add it to manifest;
	     * - if entry is contained in manifest, calculate its hash and
	     *   compare it with the one in the manifest; if they are
	     *   different, replace the hash in the manifest with the newly
	     *   generated one. (This may invalidate existing signatures!)
	     */
	    BASE64Encoder encoder = new BASE64Encoder();
	    Vector mfFiles = new Vector();

	    for (Enumeration enum_=zipFile.entries();enum_.hasMoreElements();) {
		ZipEntry ze = (ZipEntry)enum_.nextElement();

		if (ze.getName().startsWith(META_INF)) {
		    // Store META-INF files in vector, so they can be written
		    // out first
		    mfFiles.addElement(ze);

		    if (signatureRelated(ze.getName())) {
			// ignore signature-related and manifest files
			continue;
		    }
		}

		if (manifest.getAttributes(ze.getName()) != null) {
		    // jar entry is contained in manifest, check and
		    // possibly update its digest attributes
		    if (updateDigests(ze, zipFile, digests, encoder,
				      manifest) == true) {
			mfModified = true;
		    }
		} else if (!ze.isDirectory()) {
		    // Add entry to manifest
		    Attributes attrs = getDigestAttributes(ze, zipFile,
							   digests,
							   encoder);
		    mfEntries.put(ze.getName(), attrs);
		    mfModified = true;
		}
	    }

	    // Recalculate the manifest raw bytes if necessary
	    if (mfModified) {
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		manifest.write(baos);
		mfRawBytes = baos.toByteArray();
	    }

	    // Write out the manifest
	    if (mfModified) {
		// manifest file has new length
		mfFile = new ZipEntry(JarFile.MANIFEST_NAME);
	    }
	    if (verbose) {
		if (mfCreated) {
		    System.out.println(rb.getString("   adding: ") +
					mfFile.getName());
		} else if (mfModified) {
		    System.out.println(rb.getString(" updating: ") +
					mfFile.getName());
		}
	    }
	    zos.putNextEntry(mfFile);
	    zos.write(mfRawBytes);

	    // Calculate SignatureFile (".SF") and SignatureBlockFile
	    ManifestDigester manDig = new ManifestDigester(mfRawBytes);
	    SignatureFile sf = new SignatureFile(digests, manifest, manDig,
						 sigfile, signManifest);

	    if (tsaAlias != null) {
		tsaCert = getTsaCert(tsaAlias);
	    }

	    SignatureFile.Block block = null;

	    try {
		block =
		    sf.generateBlock(privateKey, certChain, externalSF, tsaUrl,
			tsaCert, signingMechanism, args, zipFile);
	    } catch (SocketTimeoutException e) {
		// Provide a helpful message when TSA is beyond a firewall
		error(rb.getString("unable to sign jar: ") +
		rb.getString("no response from the Timestamping Authority. ") +
		rb.getString("When connecting from behind a firewall then an HTTP proxy may need to be specified. ") + 
		rb.getString("Supply the following options to jarsigner: ") +
		"\n  -J-Dhttp.proxyHost=<hostname> " +
		"\n  -J-Dhttp.proxyPort=<portnumber> ", e);
	    }

	    sfFilename = sf.getMetaName();
	    bkFilename = block.getMetaName();

	    ZipEntry sfFile = new ZipEntry(sfFilename);
	    ZipEntry bkFile = new ZipEntry(bkFilename);

	    long time = System.currentTimeMillis();
	    sfFile.setTime(time);
	    bkFile.setTime(time);

	    // signature file
	    zos.putNextEntry(sfFile);
	    sf.write(zos);
	    if (verbose) {
		if (zipFile.getEntry(sfFilename) != null) {
		    System.out.println(rb.getString(" updating: ") +
				sfFilename);
		} else {
		    System.out.println(rb.getString("   adding: ") +
				sfFilename);
		}
	    }

	    if (verbose) {
		if (tsaUrl != null || tsaCert != null) {
		    System.out.println(
			rb.getString("requesting a signature timestamp"));
		}
		if (tsaUrl != null) {
		    System.out.println(rb.getString("TSA location: ") + tsaUrl);
		}
		if (tsaCert != null) {
		    String certUrl =
			TimestampedSigner.getTimestampingUrl(tsaCert);
		    if (certUrl != null) {
			System.out.println(rb.getString("TSA location: ") +
			    certUrl);
		    }
		    System.out.println(
			rb.getString("TSA certificate: ") + printCert(tsaCert));
		}
		if (signingMechanism != null) {
		    System.out.println(
			rb.getString("using an alternative signing mechanism"));
		}
	    }

	    // signature block file
	    zos.putNextEntry(bkFile);
	    block.write(zos);
	    if (verbose) {
		if (zipFile.getEntry(bkFilename) != null) {
		    System.out.println(rb.getString(" updating: ") +
			bkFilename);
		} else {
		    System.out.println(rb.getString("   adding: ") +
			bkFilename);
		}
	    }

	    // Write out all other META-INF files that we stored in the
	    // vector
	    for (int i=0; i<mfFiles.size(); i++) {
		ZipEntry ze = (ZipEntry)mfFiles.elementAt(i);
		if (!ze.getName().equalsIgnoreCase(JarFile.MANIFEST_NAME)
		    && !ze.getName().equalsIgnoreCase(sfFilename)
		    && !ze.getName().equalsIgnoreCase(bkFilename)) {
		    writeEntry(zipFile, zos, ze);
		}
	    }

	    // Write out all other files
	    for (Enumeration enum_=zipFile.entries();enum_.hasMoreElements();) {
		ZipEntry ze = (ZipEntry)enum_.nextElement();

		if (!ze.getName().startsWith(META_INF)) {
		    if (verbose) {
			if (manifest.getAttributes(ze.getName()) != null)
			  System.out.println(rb.getString("  signing: ") +
				ze.getName());
			else
			  System.out.println(rb.getString("   adding: ") +
				ze.getName());
		    }
		    writeEntry(zipFile, zos, ze);
		}
	    }

	    zipFile.close();
	    zos.close();

	    if (signedjar == null) {
		// attempt an atomic rename. If that fails,
		// rename the original jar file, then the signed
		// one, then delete the original.
		if (!signedJarFile.renameTo(jarFile)) {
		    File origJar = new File(jarName+".orig");

		    if (jarFile.renameTo(origJar)) {
			if (signedJarFile.renameTo(jarFile)) {
			    origJar.delete();
			} else {
			    MessageFormat form = new MessageFormat(rb.getString
			("attempt to rename signedJarFile to jarFile failed"));
			    Object[] source = {signedJarFile, jarFile};
			    error(form.format(source));
			}
		    } else {
			MessageFormat form = new MessageFormat(rb.getString
			    ("attempt to rename jarFile to origJar failed"));
			Object[] source = {jarFile, origJar};
			error(form.format(source));
		    }
		}
	    }

	    if (hasExpiredCert || hasExpiringCert || notYetValidCert) {
		System.out.println();

		System.out.print(rb.getString("Warning: "));
		if (hasExpiredCert) {
		    System.out.println(
			rb.getString("The signer certificate has expired."));
		} else if (hasExpiringCert) {
		    System.out.println(
			rb.getString("The signer certificate will expire within six months."));
		} else if (notYetValidCert) {
		    System.out.println(
			rb.getString("The signer certificate is not yet valid."));
	    }
	}

	} catch(IOException ioe) {
	    error(rb.getString("unable to sign jar: ")+ioe, ioe);
	}
    }

    /**
     * signature-related files include:
     * . META-INF/MANIFEST.MF
     * . META-INF/SIG-*
     * . META-INF/*.SF
     * . META-INF/*.DSA
     * . META-INF/*.RSA
     */
    private boolean signatureRelated(String name) {
	String ucName = name.toUpperCase();
	if (ucName.equals(JarFile.MANIFEST_NAME) ||
	    ucName.equals(META_INF) ||
	    (ucName.startsWith(SIG_PREFIX) &&
		ucName.indexOf("/") == ucName.lastIndexOf("/"))) {
	    return true;
	}

	if (ucName.startsWith(META_INF) &&
	    SignatureFileVerifier.isBlockOrSF(ucName)) {
	    // .SF/.DSA/.RSA files in META-INF subdirs
	    // are not considered signature-related
	    return (ucName.indexOf("/") == ucName.lastIndexOf("/"));
	}

	return false;
    }

    private void writeEntry(ZipFile zf, ZipOutputStream os, ZipEntry ze)
    throws IOException
    {
	byte[] data = getBytes(zf, ze);
        ZipEntry ze2 = new ZipEntry(ze.getName());
        ze2.setMethod(ze.getMethod());
        ze2.setTime(ze.getTime());
        ze2.setComment(ze.getComment());
        ze2.setExtra(ze.getExtra());
        if (ze.getMethod() == ZipEntry.STORED) {
            ze2.setSize(ze.getSize());
            ze2.setCrc(ze.getCrc());
        }
	os.putNextEntry(ze2);
	if (data.length > 0) {
	    os.write(data);
	}
    }

    void loadKeyStore(String keyStoreName, boolean prompt) {

	if (!nullStream && keyStoreName == null) {
	    keyStoreName = System.getProperty("user.home") + File.separator
		+ ".keystore";
	}

	try {
	    if (providerName == null) {
		store = KeyStore.getInstance(storetype);
	    } else {
		store = KeyStore.getInstance(storetype, providerName);
	    }

	    // Get pass phrase
            // XXX need to disable echo; on UNIX, call getpass(char *prompt)Z
            // and on NT call ??
	    if (token && storepass == null && !protectedPath) {
		storepass = getPass
			(rb.getString("Enter Passphrase for keystore: "));
	    } else if (!token && storepass == null && prompt) {
		storepass = getPass
			(rb.getString("Enter Passphrase for keystore: "));
	    }

	    if (nullStream) {
		store.load(null, storepass);
	    } else {
		keyStoreName = keyStoreName.replace(File.separatorChar, '/');
		URL url = null;
		try {
		    url = new URL(keyStoreName);
		} catch (java.net.MalformedURLException e) {
		    // try as file
		    File kfile = new File(keyStoreName);
		    url = new URL("file:" + kfile.getCanonicalPath());
		}
		InputStream is = url.openStream();
		store.load(is, storepass);
		is.close();
	    }
	} catch (IOException ioe) {
	    throw new RuntimeException(rb.getString("keystore load: ") +
					ioe.getMessage());
	} catch (java.security.cert.CertificateException ce) {
	    throw new RuntimeException(rb.getString("certificate exception: ") +
					ce.getMessage());
	} catch (NoSuchProviderException pe) {
	    throw new RuntimeException(rb.getString("keystore load: ") +
					pe.getMessage());
	} catch (NoSuchAlgorithmException nsae) {
	    throw new RuntimeException(rb.getString("keystore load: ") +
					nsae.getMessage());
	} catch (KeyStoreException kse) {
	    throw new RuntimeException
		(rb.getString("unable to instantiate keystore class: ") +
		kse.getMessage());
	}
    }

    X509Certificate getTsaCert(String alias) {

	java.security.cert.Certificate cs = null;

	try {
	    cs = store.getCertificate(alias);
	} catch (KeyStoreException kse) {
	    // this never happens, because keystore has been loaded
	}
	if (cs == null || (!(cs instanceof X509Certificate))) {
	    MessageFormat form = new MessageFormat(rb.getString
		("Certificate not found for: alias.  alias must reference a valid KeyStore entry containing an X.509 public key certificate for the Timestamping Authority."));
	    Object[] source = {alias, alias};
	    error(form.format(source));
	}
	return (X509Certificate) cs;
    }

    void getAliasInfo(String alias) {

	Key key = null;

	try {

	    java.security.cert.Certificate[] cs = null;

	    try {
		cs = store.getCertificateChain(alias);
	    } catch (KeyStoreException kse) {
		// this never happens, because keystore has been loaded
	    }
	    if (cs == null) {
		MessageFormat form = new MessageFormat(rb.getString
		    ("Certificate chain not found for: alias.  alias must reference a valid KeyStore key entry containing a private key and corresponding public key certificate chain."));
		Object[] source = {alias, alias};
		error(form.format(source));
	    }

	    certChain = new X509Certificate[cs.length];
	    for (int i=0; i<cs.length; i++) {
		if (!(cs[i] instanceof X509Certificate)) {
		    error(rb.getString
			("found non-X.509 certificate in signer's chain"));
		}
		certChain[i] = (X509Certificate)cs[i];
	    }

	    // order the cert chain if necessary (put user cert first,
	    // root-cert last in the chain)
	    X509Certificate userCert
		= (X509Certificate)store.getCertificate(alias);

	    // check validity of signer certificate
	    try {
		userCert.checkValidity();

		if (userCert.getNotAfter().getTime() < 
		    System.currentTimeMillis() + SIX_MONTHS) {

		    hasExpiringCert = true;
		}
	    } catch (CertificateExpiredException cee) {
		hasExpiredCert = true;

	    } catch (CertificateNotYetValidException cnyve) {
		notYetValidCert = true;
	    }

	    if (!userCert.equals(certChain[0])) {
		// need to order ...
		X509Certificate[] certChainTmp
		    = new X509Certificate[certChain.length];
		certChainTmp[0] = userCert;
		Principal issuer = userCert.getIssuerDN();
		for (int i=1; i<certChain.length; i++) {
		    int j;
		    // look for the cert whose subject corresponds to the
		    // given issuer
		    for (j=0; j<certChainTmp.length; j++) {
			if (certChainTmp[j] == null)
			    continue;
			Principal subject = certChainTmp[j].getSubjectDN();
			if (issuer.equals(subject)) {
			    certChain[i] = certChainTmp[j];
			    issuer = certChainTmp[j].getIssuerDN();
			    certChainTmp[j] = null;
			    break;
			}
		    }
		    if (j == certChainTmp.length) {
			error(rb.getString("incomplete certificate chain"));
		    }

		}
		certChain = certChainTmp; // ordered
	    }

	    try {
		if (!token && keypass == null)
		    key = store.getKey(alias, storepass);
		else
		    key = store.getKey(alias, keypass);
	    } catch (UnrecoverableKeyException e) {
		if (token) {
		    throw e;
		} else if (keypass == null) {
		    // Did not work out, so prompt user for key password
		    MessageFormat form = new MessageFormat(rb.getString
			("Enter key password for alias: "));
		    Object[] source = {alias};
		    keypass = getPass(form.format(source));
		    key = store.getKey(alias, keypass);
		}
	    }
	} catch (NoSuchAlgorithmException e) {
	    error(e.getMessage());
	} catch (UnrecoverableKeyException e) {
	    error(rb.getString("unable to recover key from keystore"));
	} catch (KeyStoreException kse) {
	    // this never happens, because keystore has been loaded
	}

	if (!(key instanceof PrivateKey)) {
	    MessageFormat form = new MessageFormat(rb.getString
		("key associated with alias not a private key"));
	    Object[] source = {alias};
	    error(form.format(source));
	} else {
	    privateKey = (PrivateKey)key;
	}
    }

    void error(String message)
    {
	System.out.println(rb.getString("jarsigner: ")+message);
	System.exit(1);
    }


    void error(String message, Exception e)
    {
	System.out.println(rb.getString("jarsigner: ")+message);
	if (debug) {
	    e.printStackTrace();
	}
	System.exit(1);
    }

    char[] getPass(String prompt)
    {
	System.err.print(prompt);
	System.err.flush();
	try {
	    char[] pass = Password.readPassword(System.in);

	    if (pass == null) {
		error(rb.getString("you must enter key password"));
	    } else {
		return pass;
	    }
	} catch (IOException ioe) {
	    error(rb.getString("unable to read password: ")+ioe.getMessage());
	}
	// this shouldn't happen
	return null;
    }

    /*
     * Reads all the bytes for a given zip entry.
     */
    private synchronized byte[] getBytes(ZipFile zf,
					 ZipEntry ze) throws IOException {
	int n;

	InputStream is = zf.getInputStream(ze);
	baos.reset();
	long left = ze.getSize();

	while((left > 0) && (n = is.read(buffer, 0, buffer.length)) != -1) {
	    baos.write(buffer, 0, n);
	    left -= n;
	}

	is.close();

	return baos.toByteArray();
    }

    /*
     * Returns manifest entry from given jar file, or null if given jar file
     * does not have a manifest entry.
     */
    private ZipEntry getManifestFile(ZipFile zf) {
	ZipEntry ze = zf.getEntry(JarFile.MANIFEST_NAME);
	if (ze == null) {
	    // Check all entries for matching name
	    Enumeration enum_ = zf.entries();
	    while (enum_.hasMoreElements() && ze == null) {
		ze = (ZipEntry)enum_.nextElement();
		if (!JarFile.MANIFEST_NAME.equalsIgnoreCase
		    (ze.getName())) {
		    ze = null;
		}
	    }
	}
	return ze;
    }

    /*
     * Computes the digests of a zip entry, and returns them as an array
     * of base64-encoded strings.
     */
    private synchronized String[] getDigests(ZipEntry ze, ZipFile zf,
					     MessageDigest[] digests,
					     BASE64Encoder encoder)
	throws IOException {

	int n, i;
	InputStream is = zf.getInputStream(ze);
	long left = ze.getSize();
	while((left > 0)
	      && (n = is.read(buffer, 0, buffer.length)) != -1) {
	    for (i=0; i<digests.length; i++) {
		digests[i].update(buffer, 0, n);
	    }
	    left -= n;
	}
	is.close();

	// complete the digests
	String[] base64Digests = new String[digests.length];
	for (i=0; i<digests.length; i++) {
	    base64Digests[i] = encoder.encode(digests[i].digest());
	}
	return base64Digests;
    }

    /*
     * Computes the digests of a zip entry, and returns them as a list of
     * attributes
     */
    private Attributes getDigestAttributes(ZipEntry ze, ZipFile zf,
					   MessageDigest[] digests,
					   BASE64Encoder encoder)
	throws IOException {

	String[] base64Digests = getDigests(ze, zf, digests, encoder);
	Attributes attrs = new Attributes();

	for (int i=0; i<digests.length; i++) {
	    attrs.putValue(digests[i].getAlgorithm()+"-Digest",
			   base64Digests[i]);
	}
	return attrs;
    }

    /*
     * Updates the digest attributes of a manifest entry, by adding or
     * replacing digest values.
     * A digest value is added if the manifest entry does not contain a digest
     * for that particular algorithm.
     * A digest value is replaced if it is obsolete.
     *
     * Returns true if the manifest entry has been changed, and false
     * otherwise.
     */
    private boolean updateDigests(ZipEntry ze, ZipFile zf,
				  MessageDigest[] digests,
				  BASE64Encoder encoder,
				  Manifest mf) throws IOException {
	boolean	update = false;

	Attributes attrs = mf.getAttributes(ze.getName());
	String[] base64Digests = getDigests(ze, zf, digests, encoder);

	for (int i=0; i<digests.length; i++) {
	    String name = digests[i].getAlgorithm()+"-Digest";
	    String mfDigest = attrs.getValue(name);
	    if (mfDigest == null
		&& digests[i].getAlgorithm().equalsIgnoreCase("SHA")) {
		// treat "SHA" and "SHA1" the same
		mfDigest = attrs.getValue("SHA-Digest");
	    }
	    if (mfDigest == null) {
		// compute digest and add it to list of attributes
		attrs.putValue(name, base64Digests[i]);
		update=true;
	    } else {
		// compare digests, and replace the one in the manifest
		// if they are different
		if (!mfDigest.equalsIgnoreCase(base64Digests[i])) {
		    attrs.putValue(name, base64Digests[i]);
		    update=true;
		}
	    }
	}
	return update;
    }

    /*
     * Try to load the specified signing mechanism.
     * The URL class loader is used.
     */
    private ContentSigner loadSigningMechanism(String signerClassName,
	String signerClassPath) throws Exception {

	// construct class loader
	String cpString = null;   // make sure env.class.path defaults to dot

	// do prepends to get correct ordering
	cpString = appendPath(System.getProperty("env.class.path"), cpString);
	cpString = appendPath(System.getProperty("java.class.path"), cpString);
	cpString = appendPath(signerClassPath, cpString);
	URL[] urls = pathToURLs(cpString);
	ClassLoader appClassLoader = new URLClassLoader(urls);

	// attempt to find signer
	Class signerClass = appClassLoader.loadClass(signerClassName);

	// Check that it implements ContentSigner
	Object signer = signerClass.newInstance();
	if (!(signer instanceof ContentSigner)) {
	    MessageFormat form = new MessageFormat(
		rb.getString("signerClass is not a signing mechanism"));
	    Object[] source = {signerClass.getName()};
	    throw new IllegalArgumentException(form.format(source));
	}
	return (ContentSigner)signer;
    }

    private static String appendPath(String path1, String path2) {
        if (path1 == null || path1.length() == 0) {
            return path2 == null ? "." : path2;
        } else if (path2 == null || path2.length() == 0) {
            return path1;
        } else {
            return path1  + File.pathSeparator + path2;
        }
    }

    /**
     * Utility method for converting a search path string to an array
     * of directory and JAR file URLs.
     *
     * @param path the search path string
     * @return the resulting array of directory and JAR file URLs
     */
    private static URL[] pathToURLs(String path) {
        StringTokenizer st = new StringTokenizer(path, File.pathSeparator);
        URL[] urls = new URL[st.countTokens()];
        int count = 0;
        while (st.hasMoreTokens()) {
            URL url = fileToURL(new File(st.nextToken()));
            if (url != null) {
                urls[count++] = url;
            }
        }
        if (urls.length != count) {
            URL[] tmp = new URL[count];
            System.arraycopy(urls, 0, tmp, 0, count);
            urls = tmp;
        }
        return urls;
    }

    /**
     * Returns the directory or JAR file URL corresponding to the specified
     * local file name.
     *
     * @param file the File object
     * @return the resulting directory or JAR file URL, or null if unknown
     */
    private static URL fileToURL(File file) {
        String name;
        try {
            name = file.getCanonicalPath();
        } catch (IOException e) {
            name = file.getAbsolutePath();
        }
        name = name.replace(File.separatorChar, '/');
        if (!name.startsWith("/")) {
            name = "/" + name;
        }
        // If the file does not exist, then assume that it's a directory
        if (!file.isFile()) {
            name = name + "/";
        }
        try {
            return new URL("file", "", name);
        } catch (MalformedURLException e) {
            throw new IllegalArgumentException("file");
        }
    }
}


class SignatureFile {

    /** SignatureFile */
    Manifest sf;

    /** .SF base name */
    String baseName;

    public SignatureFile(MessageDigest digests[],
			 Manifest mf,
			 ManifestDigester md,
			 String baseName,
			 boolean signManifest)

    {
	this.baseName = baseName;

	String version = System.getProperty("java.version");
	String javaVendor = System.getProperty("java.vendor");

	sf = new Manifest();
	Attributes mattr = sf.getMainAttributes();
	BASE64Encoder encoder = new BASE64Encoder();

	mattr.putValue(Attributes.Name.SIGNATURE_VERSION.toString(), "1.0");
	mattr.putValue("Created-By", version + " (" + javaVendor + ")");

	if (signManifest) {
	    // sign the whole manifest
	    for (int i=0; i < digests.length; i++) {
		mattr.putValue(digests[i].getAlgorithm()+"-Digest-Manifest",
			       encoder.encode(md.manifestDigest(digests[i])));
	    }
	}

	// create digest of the manifest main attributes
	ManifestDigester.Entry mde =
		md.get(ManifestDigester.MF_MAIN_ATTRS, false);
	if (mde != null) {
	    for (int i=0; i < digests.length; i++) {
		mattr.putValue(digests[i].getAlgorithm() +
			"-Digest-" + ManifestDigester.MF_MAIN_ATTRS,
			encoder.encode(mde.digest(digests[i])));
	    }
	} else {
	    throw new IllegalStateException
		("ManifestDigester failed to create " +
		"Manifest-Main-Attribute entry");
	}

	/* go through the manifest entries and create the digests */

	Map entries = sf.getEntries();
	Iterator mit = mf.getEntries().entrySet().iterator();
	while(mit.hasNext()) {
	    Map.Entry e = (Map.Entry) mit.next();
	    String name = (String)e.getKey();
	    mde = md.get(name, false);
	    if (mde != null) {
		Attributes attr = new Attributes();
		for (int i=0; i < digests.length; i++) {
		    attr.putValue(digests[i].getAlgorithm()+"-Digest",
				  encoder.encode(mde.digest(digests[i])));
		}
		entries.put(name, attr);
	    }
	}
    }

    /**
     * Writes the SignatureFile to the specified OutputStream.
     *
     * @param out the output stream
     * @exception IOException if an I/O error has occurred
     */

    public void write(OutputStream out) throws IOException
    {
	sf.write(out);
    }

    /**
     * get .SF file name
     */
    public String getMetaName()
    {
	return "META-INF/"+ baseName + ".SF";
    }

    /**
     * get base file name
     */
    public String getBaseName()
    {
	return baseName;
    }

    /*
     * Generate a signed data block. 
     * If a URL or a certificate (containing a URL) for a Timestamping
     * Authority is supplied then a signature timestamp is generated and
     * inserted into the signed data block.
     *
     * @param tsaUrl The location of the Timestamping Authority. If null
     *               then no timestamp is requested.
     * @param tsaCert The certificate for the Timestamping Authority. If null
     *               then no timestamp is requested.
     * @param signingMechanism The signing mechanism to use.
     * @param args The command-line arguments to jarsigner.
     * @param zipFile The original source Zip file.
     */
    public Block generateBlock(PrivateKey privateKey,
			       X509Certificate[] certChain,
			       boolean externalSF, String tsaUrl,
			       X509Certificate tsaCert,
			       ContentSigner signingMechanism,
			       String[] args, ZipFile zipFile)
	throws NoSuchAlgorithmException, InvalidKeyException, IOException,
	    SignatureException, CertificateException
    {
	return new Block(this, privateKey, certChain, externalSF, tsaUrl,
	    tsaCert, signingMechanism, args, zipFile);
    }


    public static class Block {

	private byte[] block;
	private String blockFileName;

	/*
	 * Construct a new signature block.
	 */
	Block(SignatureFile sfg, PrivateKey privateKey, 
	    X509Certificate[] certChain, boolean externalSF, String tsaUrl,
	    X509Certificate tsaCert, ContentSigner signingMechanism,
	    String[] args, ZipFile zipFile)
	    throws NoSuchAlgorithmException, InvalidKeyException, IOException,
	    SignatureException, CertificateException {

	    Principal issuerName = certChain[0].getIssuerDN();
	    if (!(issuerName instanceof X500Name)) {
		// must extract the original encoded form of DN for subsequent
		// name comparison checks (converting to a String and back to
	        // an encoded DN could cause the types of String attribute 
                // values to be changed)
		X509CertInfo tbsCert = new 
		    X509CertInfo(certChain[0].getTBSCertificate());
		issuerName = (Principal) 
		    tbsCert.get(CertificateIssuerName.NAME + "." + 
				CertificateIssuerName.DN_NAME);
	    }
	    BigInteger serial = certChain[0].getSerialNumber();
	    String keyAlgorithm = privateKey.getAlgorithm();

	    String digestAlgorithm;
	    if (keyAlgorithm.equalsIgnoreCase("DSA"))
		digestAlgorithm = "SHA1";
	    else if (keyAlgorithm.equalsIgnoreCase("RSA"))
		digestAlgorithm = "MD5";
	    else {
		throw new RuntimeException("private key is not a DSA or "
					   + "RSA key");
	    }

	    String signatureAlgorithm = digestAlgorithm + "with" +
		keyAlgorithm;

	    blockFileName = "META-INF/"+sfg.getBaseName()+"."+keyAlgorithm;

	    AlgorithmId digestAlg = AlgorithmId.get(digestAlgorithm);
	    AlgorithmId sigAlg = AlgorithmId.get(signatureAlgorithm);
	    AlgorithmId digEncrAlg = AlgorithmId.get(keyAlgorithm);

	    Signature sig = Signature.getInstance(signatureAlgorithm);
	    sig.initSign(privateKey);

	    ByteArrayOutputStream baos = new ByteArrayOutputStream();
	    sfg.write(baos);

	    byte[] content = baos.toByteArray();

	    sig.update(content);
	    byte[] signature = sig.sign();

	    // Timestamp the signature and generate the signature block file
	    if (signingMechanism == null) {
		signingMechanism = new TimestampedSigner();
	    }
	    URI tsaUri = null;
	    try {
		if (tsaUrl != null) {
		    tsaUri = new URI(tsaUrl);
		}
	    } catch (URISyntaxException e) {
		IOException ioe = new IOException();
		ioe.initCause(e);
		throw ioe;
	    }

	    // Assemble parameters for the signing mechanism
	    ContentSignerParameters params =
		new JarSignerParameters(args, tsaUri, tsaCert, signature, 
		    signatureAlgorithm, certChain, content, zipFile);

	    // Generate the signature block
	    block = signingMechanism.generateSignedData(
		    params, externalSF, (tsaUrl != null || tsaCert != null));
	}

	/*
	 * get block file name.
	 */
	public String getMetaName()
	{
	    return blockFileName;
	}

	/**
	 * Writes the block file to the specified OutputStream.
	 *
	 * @param out the output stream
	 * @exception IOException if an I/O error has occurred
	 */

	public void write(OutputStream out) throws IOException
	{
	    out.write(block);
	}
    }
}


/*
 * This object encapsulates the parameters used to perform content signing.
 */
class JarSignerParameters implements ContentSignerParameters {

    private String[] args;
    private URI tsa;
    private X509Certificate tsaCertificate;
    private byte[] signature;
    private String signatureAlgorithm;
    private X509Certificate[] signerCertificateChain;
    private byte[] content;
    private ZipFile source;

    /**
     * Create a new object.
     */
    JarSignerParameters(String[] args, URI tsa, X509Certificate tsaCertificate,
	byte[] signature, String signatureAlgorithm,
	X509Certificate[] signerCertificateChain, byte[] content,
	ZipFile source) {

	if (signature == null || signatureAlgorithm == null || 
	    signerCertificateChain == null) {
	    throw new NullPointerException();
	}
	this.args = args;
	this.tsa = tsa;
	this.tsaCertificate = tsaCertificate;
	this.signature = signature;
	this.signatureAlgorithm = signatureAlgorithm;
	this.signerCertificateChain = signerCertificateChain;
	this.content = content;
	this.source = source;
    }

    /**
     * Retrieves the command-line arguments.
     *
     * @return The command-line arguments. May be null.
     */
    public String[] getCommandLine() {
	return args;
    }
 
    /**
     * Retrieves the identifier for a Timestamping Authority (TSA).
     *
     * @return The TSA identifier. May be null.
     */
    public URI getTimestampingAuthority() {
	return tsa;
    }
 
    /**
     * Retrieves the certificate for a Timestamping Authority (TSA).
     *
     * @return The TSA certificate. May be null.
     */
    public X509Certificate getTimestampingAuthorityCertificate() {
	return tsaCertificate;
    }
 
    /**
     * Retrieves the signature.
     *
     * @return The non-null signature bytes.
     */
    public byte[] getSignature() {
	return signature;
    }
 
    /**
     * Retrieves the name of the signature algorithm.
     *
     * @return The non-null string name of the signature algorithm.
     */
    public String getSignatureAlgorithm() {
	return signatureAlgorithm;
    }
 
    /**
     * Retrieves the signer's X.509 certificate chain.
     *
     * @return The non-null array of X.509 public-key certificates.
     */
    public X509Certificate[] getSignerCertificateChain() {
	return signerCertificateChain;
    }
 
    /**
     * Retrieves the content that was signed.
     *
     * @return The content bytes. May be null.
     */
    public byte[] getContent() {
	return content;
    }
 
    /**
     * Retrieves the original source ZIP file before it was signed.
     *
     * @return The original ZIP file. May be null.
     */
    public ZipFile getSource() {
	return source;
    }
}
