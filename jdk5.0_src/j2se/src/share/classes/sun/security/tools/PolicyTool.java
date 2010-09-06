/*
 * @(#)PolicyTool.java	1.59 04/04/21
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.tools;

import java.io.*;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Vector;
import java.util.Enumeration;
import java.net.URL;
import java.net.MalformedURLException;
import java.lang.reflect.*;
import java.text.Collator;
import java.text.MessageFormat;
import sun.misc.BASE64Decoder;
import sun.security.util.PropertyExpander;
import sun.security.util.PropertyExpander.ExpandException;
import java.awt.*;
import java.awt.event.*;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.*;
import sun.security.provider.*;

/**
 * PolicyTool may be used by users and administrators to configure the
 * overall java security policy (currently stored in the policy file).
 * Using PolicyTool administators may add and remove policies from
 * the policy file. <p>
 *
 * @version 1.59, 04/21/04
 * @see java.security.Policy
 * @since   JDK1.2
 */

class PolicyTool {

    // for i18n
    static final java.util.ResourceBundle rb =
	java.util.ResourceBundle.getBundle("sun.security.util.Resources");
    static final Collator collator = Collator.getInstance();
    static {
	// this is for case insensitive string comparisons
	collator.setStrength(Collator.PRIMARY);
    };

    // anyone can add warnings
    Vector warnings;
    boolean newWarning = false;

    // set to true if policy modified.
    // this way upon exit we know if to ask the user to save changes
    boolean modified = false;

    private static final boolean testing = false;
    private static final Class[] TWOPARAMS = { String.class, String.class };
    private static final Class[] ONEPARAMS = { String.class };
    private static final Class[] NOPARAMS  = {};
    /*
     * All of the policy entries are read in from the
     * policy file and stored here.  Updates to the policy entries
     * using addEntry() and removeEntry() are made here.  To ultimately save
     * the policy entries back to the policy file, the SavePolicy button
     * must be clicked.
     **/
    private static String policyFileName = null;
    private Vector policyEntries = null;
    private PolicyParser parser = null;

    /* The public key alias information is stored here.  */
    private KeyStore keyStore = null;
    private String keyStoreName = " ";
    private String keyStoreType = " ";

    /**
     * default constructor
     */
    private PolicyTool() {
	policyEntries = new Vector();
	parser = new PolicyParser();
	warnings = new Vector();
    }

    /**
     * get the PolicyFileName
     */
    String getPolicyFileName() {
	return policyFileName;
    }

    /**
     * set the PolicyFileName
     */
    void setPolicyFileName(String policyFileName) {
	this.policyFileName = policyFileName;
    }

    /**
     * set the keyStore URL name
     */
    void setKeyStoreInfo(String keyStoreName, String keyStoreType) {
	this.keyStoreName = keyStoreName;
	this.keyStoreType = keyStoreType;
	modified = true;
    }

    /**
     * get the keyStore URL name
     */
    String getKeyStoreName() {
	return keyStoreName;
    }

    /**
     * get the keyStore Type
     */
    String getKeyStoreType() {
	return keyStoreType;
    }

    /**
     * Open and read a policy file
     */
    void openPolicy(String filename)
    throws MalformedURLException, NoSuchMethodException,
    ClassNotFoundException, InstantiationException, IllegalAccessException,
    InvocationTargetException, PolicyParser.ParsingException,
    IOException, FileNotFoundException, CertificateException, Exception {

	newWarning = false;

	// start fresh - blow away the current state
	policyEntries = new Vector();
	parser = new PolicyParser();
	warnings = new Vector();
	setPolicyFileName(null);
	setKeyStoreInfo(null, null);

	// see if user is opening a NEW policy file
	if (filename == null) {
	    modified = false;
	    return;
	}

	// Read in the policy entries from the file and
	// populate the parser vector table.  The parser vector
	// table only holds the entries as strings, so it only
	// guarantees that the policies are syntactically
	// correct.
	setPolicyFileName(filename);
	parser.read(new FileReader(filename));
	setKeyStoreInfo(parser.getKeyStoreUrl(), parser.getKeyStoreType());

	// Update the local vector with the same policy entries.
	// This guarantees that the policy entries are not only
	// syntactically correct, but semantically valid as well.
	Enumeration enum_ = parser.grantElements();
	while (enum_.hasMoreElements()) {
	    PolicyParser.GrantEntry ge =
				(PolicyParser.GrantEntry)enum_.nextElement();

	    // see if all the signers have public keys
	    if (ge.signedBy != null) {

		String signers[] = parseSigners(ge.signedBy);
		for (int i = 0; i < signers.length; i++) {
		    PublicKey pubKey = getPublicKeyAlias(signers[i]);
		    if (pubKey == null) {
			newWarning = true;
			MessageFormat form = new MessageFormat(rb.getString
				("Warning: A public key for alias 'signers[i]' does not exist."));
			Object[] source = {signers[i]};
			warnings.addElement(form.format(source));
		    }
		}
	    }

	    // check to see if the Principals are valid
	    ListIterator prinList = ge.principals.listIterator(0);
	    while (prinList.hasNext()) {
		PolicyParser.PrincipalEntry pe =
			(PolicyParser.PrincipalEntry)prinList.next();
		try {
		    verifyPrincipal(pe.getPrincipalClass(), pe.getPrincipalName());
		} catch (ClassNotFoundException fnfe) {
		    newWarning = true;
		    warnings.addElement(rb.getString
			("Warning: Class not found: ") + pe.getPrincipalClass());
		}
	    }

	    // check to see if the Permissions are valid
	    Enumeration perms = ge.permissionElements();
	    while (perms.hasMoreElements()) {
		PolicyParser.PermissionEntry pe =
			(PolicyParser.PermissionEntry)perms.nextElement();
		try {
		    verifyPermission(pe.permission, pe.name, pe.action);
		} catch (ClassNotFoundException fnfe) {
		    newWarning = true;
		    warnings.addElement(rb.getString
			("Warning: Class not found: ") + pe.permission);
		} catch (InvocationTargetException ite) {
		    newWarning = true;
		    warnings.addElement(rb.getString
			("Warning: Invalid argument(s) for constructor: ") +
			pe.permission);
		}

		// see if all the permission signers have public keys
		if (pe.signedBy != null) {

		    String signers[] = parseSigners(pe.signedBy);

		    for (int i = 0; i < signers.length; i++) {
			PublicKey pubKey = getPublicKeyAlias(signers[i]);
			if (pubKey == null) {
			    newWarning = true;
			    MessageFormat form = new MessageFormat(rb.getString
				("Warning: A public key for alias 'signers[i]' does not exist."));
			    Object[] source = {signers[i]};
			    warnings.addElement(form.format(source));
			}
		    }
		}
	    }
	    PolicyEntry pEntry = new PolicyEntry(this, ge);
	    policyEntries.addElement(pEntry);
	}
	if (newWarning == false)
	    warnings.addElement
		(rb.getString("Policy File opened successfully"));

	// just read in the policy -- nothing has been modified yet
	modified = false;
    }

    /**
     * Save a policy to a file
     */
    void savePolicy(String filename)
    throws FileNotFoundException, IOException {
	// save the policy entries to a file
	parser.setKeyStoreUrl(keyStoreName);
	parser.setKeyStoreType(keyStoreType);
	parser.write(new FileWriter(filename));
	modified = false;
    }

    /**
     * Open the KeyStore
     */
    void openKeyStore(String keyStoreName, char[] password)
	throws KeyStoreException, NoSuchAlgorithmException,
	UnrecoverableKeyException, IOException,
	CertificateException, ExpandException {

	    if (keyStoreName == null || keyStoreName.length() == 0)
		throw new IOException
			(rb.getString("null Keystore name"));

	    keyStoreName = PropertyExpander.expand
				(keyStoreName).replace
				(File.separatorChar, '/');
	    URL url = null;
	    try {
		url = new URL(keyStoreName);
	    } catch (java.net.MalformedURLException e) {
		File pfile = new File(policyFileName);
		URL policyURL = new URL("file:" + pfile.getCanonicalPath());
		url = new URL(policyURL , keyStoreName);
	    }
	    InputStream is = url.openStream();

	    // XXX We should add a command-line option that lets users
	    // specify the keystore type.
	    keyStore = KeyStore.getInstance(KeyStore.getDefaultType());
	    keyStore.load(is, password);
	    is.close();
    }

    /**
     * Add a Grant entry to the overall policy at the specified index.
     * A policy entry consists of a CodeSource.
     */
    boolean addEntry(PolicyEntry pe, int index) {

	if (index < 0) {
	    // new entry -- just add it to the end
	    policyEntries.addElement(pe);
	    parser.add(pe.getGrantEntry());
	} else {
	    // existing entry -- replace old one
	    PolicyEntry origPe = (PolicyEntry)policyEntries.elementAt(index);
	    parser.replace(origPe.getGrantEntry(), pe.getGrantEntry());
	    policyEntries.setElementAt(pe, index);
	}
	return true;
    }

    /**
     * Add a Principal entry to an existing PolicyEntry at the specified index.
     * A Principal entry consists of a class, and name.
     *
     * If the principal already exists, it is not added again.
     */
    boolean addPrinEntry(PolicyEntry pe,
			PolicyParser.PrincipalEntry newPrin,
			int index) {

	// first add the principal to the Policy Parser entry
	PolicyParser.GrantEntry grantEntry = pe.getGrantEntry();
	if (grantEntry.contains(newPrin) == true)
	    return false;

	LinkedList prinList = grantEntry.principals;
	if (index != -1)
	    prinList.set(index, newPrin);
	else
	    prinList.add(newPrin);

	modified = true;
	return true;
    }

    /**
     * Add a Permission entry to an existing PolicyEntry at the specified index.
     * A Permission entry consists of a permission, name, and actions.
     *
     * If the permission already exists, it is not added again.
     */
    boolean addPermEntry(PolicyEntry pe,
			PolicyParser.PermissionEntry newPerm,
			int index) {

	// first add the permission to the Policy Parser Vector
	PolicyParser.GrantEntry grantEntry = pe.getGrantEntry();
	if (grantEntry.contains(newPerm) == true)
	    return false;

	Vector permList = grantEntry.permissionEntries;
	if (index != -1)
	    permList.setElementAt(newPerm, index);
	else
	    permList.addElement(newPerm);

	modified = true;
	return true;
    }

    /**
     * Remove a Permission entry from an existing PolicyEntry.
     */
    boolean removePermEntry(PolicyEntry pe,
			PolicyParser.PermissionEntry perm) {

	// remove the Permission from the GrantEntry
	PolicyParser.GrantEntry ppge = pe.getGrantEntry();
	modified = ppge.remove(perm);
	return modified;
    }

    /**
     * remove an entry from the overall policy
     */
    boolean removeEntry(PolicyEntry pe) {

	parser.remove(pe.getGrantEntry());
	modified = true;
	return (policyEntries.removeElement(pe));
    }

    /**
     * retrieve all Policy Entries
     */
    PolicyEntry[] getEntry() {

	if (policyEntries.size() > 0) {
	    PolicyEntry entries[] = new PolicyEntry[policyEntries.size()];
	    for (int i = 0; i < policyEntries.size(); i++)
		entries[i] = (PolicyEntry)policyEntries.elementAt(i);
	    return entries;
	}
	return null;
    }

    /**
     * Retrieve the public key mapped to a particular name.
     * If the key has expired, a KeyException is thrown.
     */
    PublicKey getPublicKeyAlias(String name)
	throws CertificateException, NoSuchAlgorithmException,
	UnrecoverableKeyException, KeyStoreException, ExpandException {

	try {
	    openKeyStore(keyStoreName, null);
	    Certificate cert = keyStore.getCertificate(name);

	    if (cert == null)
		return null;
	    PublicKey pubKey = cert.getPublicKey();
	    return pubKey;
	} catch (IOException ioe) {
	    // no keystore
	    // XXX: should i display an error dialog to the user?
	    newWarning = true;
	    warnings.addElement
		(rb.getString("Warning: Unable to open Keystore: ") +
		keyStoreName);
	    return null;
	}
    }

    /**
     * Retrieve all the alias names stored in the certificate database
     */
    String[] getPublicKeyAlias()
	throws CertificateException, NoSuchAlgorithmException,
	UnrecoverableKeyException, KeyStoreException, ExpandException {

	try {
	    int numAliases = 0;
	    String aliases[] = null;

	    openKeyStore(keyStoreName, null);
	    Enumeration enum_ = keyStore.aliases();

	    // first count the number of elements
	    while (enum_.hasMoreElements()) {
		enum_.nextElement();
		numAliases++;
	    }

	    if (numAliases > 0) {
		// now copy them into an array
		aliases = new String[numAliases];
		numAliases = 0;
		enum_ = keyStore.aliases();
		while (enum_.hasMoreElements()) {
		    aliases[numAliases] =
				new String((String)enum_.nextElement());
		    numAliases++;
		}
	    }
	    return aliases;
	} catch (IOException ioe) {
	    // no keystore
	    // XXX: should i display an error dialog to the user?
	    newWarning = true;
	    warnings.addElement
		(rb.getString("Warning: Unable to open Keystore: ") +
		keyStoreName);
	    return null;
	}
    }

    /**
     * This method parses a single string of signers separated by commas
     * ("jordan, duke, pippen") into an array of individual strings.
     */
    String[] parseSigners(String signedBy) {

	String signers[] = null;
	int numSigners = 1;
	int signedByIndex = 0;
	int commaIndex = 0;
	int signerNum = 0;

	// first pass thru "signedBy" counts the number of signers
	while (commaIndex >= 0) {
	    commaIndex = signedBy.indexOf(',', signedByIndex);
	    if (commaIndex >= 0) {
		numSigners++;
		signedByIndex = commaIndex + 1;
	    }
	}
	signers = new String[numSigners];

	// second pass thru "signedBy" transfers signers to array
	commaIndex = 0;
	signedByIndex = 0;
	while (commaIndex >= 0) {
	    if ((commaIndex = signedBy.indexOf(',', signedByIndex)) >= 0) {
		// transfer signer and ignore trailing part of the string
		signers[signerNum] =
			signedBy.substring(signedByIndex, commaIndex).trim();
		signerNum++;
		signedByIndex = commaIndex + 1;
	    } else {
		// we are at the end of the string -- transfer signer
		signers[signerNum] = signedBy.substring(signedByIndex).trim();
	    }
	}
	return signers;
    }

    /**
     * Check to see if the Principal contents are OK
     */
    void verifyPrincipal(String type, String name)
	throws ClassNotFoundException,
	       InstantiationException
    {
	if (type.equals(PolicyParser.PrincipalEntry.WILDCARD_CLASS) ||
	    type.equals(PolicyParser.REPLACE_NAME)) {
	    return;
	};
	Class PRIN = Class.forName("java.security.Principal");
	Class pc = Class.forName(type, true, 
                Thread.currentThread().getContextClassLoader());
	if (!PRIN.isAssignableFrom(pc)) {
	    throw new InstantiationException(rb.getString("Illegal Principal Type"));
	}
    }

    /**
     * Check to see if the Permission contents are OK
     */
    void verifyPermission(String type,
				    String name,
				    String actions)
	throws ClassNotFoundException,
	       InstantiationException,
	       IllegalAccessException,
	       NoSuchMethodException,
	       InvocationTargetException
    {

	//XXX we might want to keep a hash of created factories...
	Class pc = Class.forName(type, true, 
		Thread.currentThread().getContextClassLoader());
	Constructor c = null;
	Vector objects = new Vector(2);
	if (name != null) objects.add(name);
	if (actions != null) objects.add(actions);
	switch (objects.size()) {
	case 0:
	    try {
	        c = pc.getConstructor(NOPARAMS);
	        break;
	    } catch (NoSuchMethodException ex) {
		// proceed to the one-param constructor
		objects.add(null);
	    }
	case 1: 
	    try {
	        c = pc.getConstructor(ONEPARAMS);
	        break;
            } catch (NoSuchMethodException ex) {
                // proceed to the two-param constructor
                objects.add(null);
            }
	case 2:
	    c = pc.getConstructor(TWOPARAMS);
	    break;
	}
	Object parameters[] = objects.toArray();
	Permission p = (Permission)c.newInstance(parameters);
    }

    /*
     * Parse command line arguments.
     */
    static void parseArgs(String args[]) {
	/* parse flags */
	int n = 0;

	for (n=0; (n < args.length) && args[n].startsWith("-"); n++) {

	    String flags = args[n];

	    if (collator.compare(flags, "-file") == 0) {
		if (++n == args.length) usage();
		policyFileName = args[n];
	    } else {
		System.err.println(rb.getString("Illegal option: ") + flags);
		usage();
	    }
	}
    }

    static void usage() {
	System.out.println(rb.getString("Usage: policytool [options]"));
	System.out.println();
	System.out.println(rb.getString
		("  [-file <file>]    policy file location"));
	System.out.println();

	System.exit(1);
    }

    /**
     * run the PolicyTool
     */
    public static void main(String args[]) {
	parseArgs(args);
	ToolWindow tw = new ToolWindow(new PolicyTool());
	tw.displayToolWindow(args);
    }
}

/**
 * Each entry in the policy configuration file is represented by a
 * PolicyEntry object.
 *
 * A PolicyEntry is a (CodeSource,Permission) pair.  The
 * CodeSource contains the (URL, PublicKey) that together identify
 * where the Java bytecodes come from and who (if anyone) signed
 * them.  The URL could refer to localhost.  The URL could also be
 * null, meaning that this policy entry is given to all comers, as
 * long as they match the signer field.  The signer could be null,
 * meaning the code is not signed.
 *
 * The Permission contains the (Type, Name, Action) triplet.
 *
 */
class PolicyEntry {

    private CodeSource codesource;
    private PolicyTool tool;
    private PolicyParser.GrantEntry grantEntry;
    private boolean testing = false;

    /**
     * Create a PolicyEntry object from the information read in
     * from a policy file.
     */
    PolicyEntry(PolicyTool tool, PolicyParser.GrantEntry ge)
    throws MalformedURLException, NoSuchMethodException,
    ClassNotFoundException, InstantiationException, IllegalAccessException,
    InvocationTargetException, CertificateException,
    IOException, NoSuchAlgorithmException, UnrecoverableKeyException {

	this.tool = tool;

	URL location = null;

	// construct the CodeSource
	if (ge.codeBase != null)
	    location = new URL(ge.codeBase);
	this.codesource = new CodeSource(location, 
	    (java.security.cert.Certificate[]) null);

	if (testing) {
	    System.out.println("Adding Policy Entry:");
	    System.out.println("    CodeBase = " + location);
	    System.out.println("    Signers = " + ge.signedBy);
	    System.out.println("    with " + ge.principals.size() +
		    " Principals");
	}

	this.grantEntry = ge;
    }

    /**
     * get the codesource associated with this PolicyEntry
     */
    CodeSource getCodeSource() {
	return codesource;
    }

    /**
     * get the GrantEntry associated with this PolicyEntry
     */
    PolicyParser.GrantEntry getGrantEntry() {
	return grantEntry;
    }

    /**
     * convert the header portion, i.e. codebase, signer, principals, of
     * this policy entry into a string
     */
    String headerToString() {
	return codebaseToString() + " " + principalsToString();
    }

    /**
     * convert the Codebase/signer portion of this policy entry into a string
     */
    String codebaseToString() {

	String stringEntry = new String();

	if (grantEntry.codeBase != null &&
	    grantEntry.codeBase.equals("") == false)
	    stringEntry = stringEntry.concat
				("CodeBase \"" +
				grantEntry.codeBase +
				"\"");

	if (grantEntry.signedBy != null &&
	    grantEntry.signedBy.equals("") == false)
	    stringEntry = ((stringEntry.length() > 0) ?
		stringEntry.concat(", SignedBy \"" +
				grantEntry.signedBy +
				"\"") :
		stringEntry.concat("SignedBy \"" +
				grantEntry.signedBy +
				"\""));

	if (stringEntry.length() == 0)
	    return new String("CodeBase <ALL>");
	return stringEntry;
    }

    /**
     * convert the Principals portion of this policy entry into a string
     */
    String principalsToString() {
	String result = "";
	if ((grantEntry.principals != null) &&
	    (!grantEntry.principals.isEmpty())) {
	    StringBuffer buffer = new StringBuffer(200);
	    ListIterator list = grantEntry.principals.listIterator();
	    while (list.hasNext()) {
		PolicyParser.PrincipalEntry pppe =
		    (PolicyParser.PrincipalEntry) list.next();
		buffer.append(" Principal " + pppe.getDisplayClass() + " " +
		    pppe.getDisplayName(true));
		if (list.hasNext()) buffer.append(", ");
	    }
	    result = buffer.toString();
	}
	return result;
    }

    /**
     * convert this policy entry into a PolicyParser.PermissionEntry
     */
    PolicyParser.PermissionEntry toPermissionEntry(Permission perm) {

	String actions = null;

	// get the actions
	if (perm.getActions() != null &&
	    perm.getActions().trim() != "")
		actions = perm.getActions();

	PolicyParser.PermissionEntry pe = new PolicyParser.PermissionEntry
			(perm.getClass().getName(),
			perm.getName(),
			actions);
	return pe;
    }
}

/**
 * The main window for the PolicyTool
 */
class ToolWindow extends Frame {
    // use serialVersionUID from JDK 1.2.2 for interoperability
    private static final long serialVersionUID = 5682568601210376777L;

    /* external paddings */
    public static final Insets TOP_PADDING = new Insets(25,0,0,0);
    public static final Insets BOTTOM_PADDING = new Insets(0,0,25,0);
    public static final Insets LITE_BOTTOM_PADDING = new Insets(0,0,10,0);
    public static final Insets LR_PADDING = new Insets(0,10,0,10);

    /* buttons and menus */
    public static final String NEW_POLICY_FILE		=
			PolicyTool.rb.getString("New");
    public static final String OPEN_POLICY_FILE		=
			PolicyTool.rb.getString("Open");
    public static final String SAVE_POLICY_FILE		=
			PolicyTool.rb.getString("Save");
    public static final String SAVE_AS_POLICY_FILE	=
			PolicyTool.rb.getString("Save As");
    public static final String VIEW_WARNINGS		=
			PolicyTool.rb.getString("View Warning Log");
    public static final String QUIT			=
			PolicyTool.rb.getString("Exit");
    public static final String ADD_POLICY_ENTRY		=
			PolicyTool.rb.getString("Add Policy Entry");
    public static final String EDIT_POLICY_ENTRY	=
			PolicyTool.rb.getString("Edit Policy Entry");
    public static final String REMOVE_POLICY_ENTRY	=
			PolicyTool.rb.getString("Remove Policy Entry");
    public static final String CHANGE_KEYSTORE		=
			PolicyTool.rb.getString("Change KeyStore");
    public static final String ADD_PUBKEY_ALIAS		=
			PolicyTool.rb.getString("Add Public Key Alias");
    public static final String REMOVE_PUBKEY_ALIAS	=
			PolicyTool.rb.getString("Remove Public Key Alias");

    /* gridbag index for components in the main window (MW) */
    public static final int MW_FILENAME_LABEL		= 0;
    public static final int MW_FILENAME_TEXTFIELD	= 1;
    public static final int MW_KEYSTORE_LABEL		= 2;
    public static final int MW_KEYSTORE_TEXTFIELD	= 3;
    public static final int MW_PANEL			= 4;
    public static final int MW_ADD_BUTTON		= 0;
    public static final int MW_EDIT_BUTTON		= 1;
    public static final int MW_REMOVE_BUTTON		= 2;
    public static final int MW_POLICY_LIST		= 5;

    private PolicyTool tool;

    /**
     * Constructor
     */
    ToolWindow(PolicyTool tool) {
	this.tool = tool;
    }

    /**
     * Initialize the PolicyTool window with the necessary components
     */
    private void initWindow() {

	// create the top menu bar
	MenuBar menuBar = new MenuBar();

	// create a File menu
	Menu menu = new Menu(PolicyTool.rb.getString("File"));
	menu.add(NEW_POLICY_FILE);
	menu.add(OPEN_POLICY_FILE);
	menu.add(SAVE_POLICY_FILE);
	menu.add(SAVE_AS_POLICY_FILE);
	menu.add(VIEW_WARNINGS);
	menu.add(QUIT);
	menu.addActionListener(new FileMenuListener(tool, this));
	menuBar.add(menu);
	setMenuBar(menuBar);

	// create an Edit menu
	menu = new Menu(PolicyTool.rb.getString("Edit"));
	menu.add(ADD_POLICY_ENTRY);
	menu.add(EDIT_POLICY_ENTRY);
	menu.add(REMOVE_POLICY_ENTRY);

	/*
	 * XXX: the next two lines are commented out on purpose.
	 *	the code is left here just in case we decide
	 *	to restore this functionality later.
	 */
	// menu.add(ADD_PUBKEY_ALIAS);
	// menu.add(REMOVE_PUBKEY_ALIAS);

	menu.add(CHANGE_KEYSTORE);
	menu.addActionListener(new MainWindowListener(tool, this));
	menuBar.add(menu);
	setMenuBar(menuBar);


	// policy entry listing
	Label label = new Label(PolicyTool.rb.getString("Policy File:"));
	addNewComponent(this, label, MW_FILENAME_LABEL,
			0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			TOP_PADDING);
	TextField tf = new TextField(50);
	tf.setEditable(false);
	addNewComponent(this, tf, MW_FILENAME_TEXTFIELD,
			1, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			TOP_PADDING);

	// KeyStore URL
	label = new Label(PolicyTool.rb.getString("Keystore:"));
	addNewComponent(this, label, MW_KEYSTORE_LABEL,
			0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			BOTTOM_PADDING);
	tf = new TextField(50);
	tf.setEditable(false);
	addNewComponent(this, tf, MW_KEYSTORE_TEXTFIELD,
			1, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			BOTTOM_PADDING);


	// add ADD/REMOVE/EDIT buttons in a new panel
	Panel panel = new Panel();
	panel.setLayout(new GridBagLayout());

	Button button = new Button(ADD_POLICY_ENTRY);
	button.addActionListener(new MainWindowListener(tool, this));
	addNewComponent(panel, button, MW_ADD_BUTTON,
			0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			LR_PADDING);

	button = new Button(EDIT_POLICY_ENTRY);
	button.addActionListener(new MainWindowListener(tool, this));
	addNewComponent(panel, button, MW_EDIT_BUTTON,
			1, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			LR_PADDING);

	button = new Button(REMOVE_POLICY_ENTRY);
	button.addActionListener(new MainWindowListener(tool, this));
	addNewComponent(panel, button, MW_REMOVE_BUTTON,
			2, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			LR_PADDING);

	addNewComponent(this, panel, MW_PANEL,
			0, 2, 2, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			BOTTOM_PADDING);


	String policyFile = tool.getPolicyFileName();
	if (policyFile == null) {
	    String userHome;
	    userHome = (String) java.security.AccessController.doPrivileged(
                    new sun.security.action.GetPropertyAction("user.home"));
	    policyFile = userHome + File.separatorChar + ".java.policy";
	}

	try {
	    // open the policy file
	    tool.openPolicy(policyFile);

	    // display the policy entries via the policy list textarea
	    List list = new List(40, false);
	    list.addActionListener(new PolicyListListener(tool, this));
	    PolicyEntry entries[] = tool.getEntry();
	    if (entries != null) {
		for (int i = 0; i < entries.length; i++)
		    list.add(entries[i].headerToString());
	    }
	    TextField newFilename = (TextField)
				getComponent(MW_FILENAME_TEXTFIELD);
	    newFilename.setText(policyFile);
	    TextField ksName = (TextField)
				getComponent(MW_KEYSTORE_TEXTFIELD);
	    if (tool.getKeyStoreType() != null &&
		tool.getKeyStoreType().length() > 0)
		ksName.setText(tool.getKeyStoreName() +
				", " +
				tool.getKeyStoreType());
	    else
		ksName.setText(tool.getKeyStoreName());

	    initPolicyList(list);

	} catch (PolicyParser.ParsingException pppe) {
	    // add blank policy listing
	    List list = new List(40, false);
	    initPolicyList(list);
	    setVisible(true);

	    // display the error
	    MessageFormat form = new MessageFormat(PolicyTool.rb.getString
		("Error parsing policy file policyFile: pppe.getMessage()"));
	    Object[] source = {policyFile,pppe.getMessage()};
	    displayErrorDialog(null, form.format(source));

	    // exit
	    System.exit(1);
	} catch (FileNotFoundException fnfe) {

	    // add blank policy listing
	    List list = new List(40, false);
	    list.addActionListener(new PolicyListListener(tool, this));
	    initPolicyList(list);
	    tool.modified = false;
	    setVisible(true);

	    // display the error
	    displayErrorDialog(null,
		PolicyTool.rb.getString("Could not find Policy File: ") +
		policyFile);

	} catch (Exception ee) {
	    // add blank policy listing
	    List list = new List(40, false);
	    list.addActionListener(new PolicyListListener(tool, this));
	    initPolicyList(list);
	    tool.modified = false;
	}
    }

    /**
     * Add a component to the PolicyTool window
     */
    void addNewComponent(Container container, Component component,
	int index, int gridx, int gridy, int gridwidth, int gridheight,
	double weightx, double weighty, int fill, Insets is) {

	// add the component at the specified gridbag index
	container.add(component, index);

	// set the constraints
	GridBagLayout gbl = (GridBagLayout)container.getLayout();
	GridBagConstraints gbc = new GridBagConstraints();
	gbc.gridx = gridx;
	gbc.gridy = gridy;
	gbc.gridwidth = gridwidth;
	gbc.gridheight = gridheight;
	gbc.weightx = weightx;
	gbc.weighty = weighty;
	gbc.fill = fill;
	if (is != null) gbc.insets = is;
	gbl.setConstraints(component, gbc);
    }


    /**
     * Add a component to the PolicyTool window without external padding
     */
    void addNewComponent(Container container, Component component,
	int index, int gridx, int gridy, int gridwidth, int gridheight,
	double weightx, double weighty, int fill) {

        // delegate with "null" external padding
        addNewComponent(container, component, index, gridx, gridy,
			gridwidth, gridheight, weightx, weighty,
			fill, null);
    }


    /**
     * Init the policy_entry_list TEXTAREA component in the
     * PolicyTool window
     */
    void initPolicyList(List policyList) {

	// add the policy list to the window
	addNewComponent(this, policyList, MW_POLICY_LIST,
			0, 3, 2, 1, 1.0, 1.0, GridBagConstraints.BOTH);
    }

    /**
     * Replace the policy_entry_list TEXTAREA component in the
     * PolicyTool window with an updated one.
     */
    void replacePolicyList(List policyList) {

	// remove the original list of Policy Entries
	// and add the new list of entries
	List list = (List)getComponent(MW_POLICY_LIST);
	list.removeAll();
	String newItems[] = policyList.getItems();
	for (int i = 0; i < newItems.length; i++)
	    list.add(newItems[i]);
    }

    /**
     * display the main PolicyTool window
     */
    void displayToolWindow(String args[]) {

	setTitle(PolicyTool.rb.getString("Policy Tool"));
	setResizable(true);
	addWindowListener(new ToolWindowListener(this));
	setBounds(135, 80, 500, 500);
	setLayout(new GridBagLayout());

	initWindow();

	// display it
	setVisible(true);

	if (tool.newWarning == true) {
	    displayStatusDialog(this, PolicyTool.rb.getString
		("Errors have occurred while opening the policy configuration.  View the Warning Log for more information."));
	}
    }

    /**
     * displays a dialog box describing an error which occurred.
     */
    void displayErrorDialog(Window w, String error) {
	ToolDialog ed = new ToolDialog
		(PolicyTool.rb.getString("Error"), tool, this, true);

	// find where the PolicyTool gui is
	Point location = ((w == null) ?
		getLocationOnScreen() : w.getLocationOnScreen());
	ed.setBounds(location.x + 50, location.y + 50, 600, 100);
	ed.setLayout(new GridBagLayout());

	Label label = new Label(error);
	addNewComponent(ed, label, 0,
			0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH);

	Button okButton = new Button(PolicyTool.rb.getString("OK"));
	okButton.addActionListener(new ErrorOKButtonListener(ed));
	addNewComponent(ed, okButton, 1,
			0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL);

	ed.pack();
	ed.setVisible(true);
    }

    /**
     * displays a dialog box describing an error which occurred.
     */
    void displayErrorDialog(Window w, Exception e) {
	displayErrorDialog(w, e.toString());
    }

    /**
     * displays a dialog box describing the status of an event
     */
    void displayStatusDialog(Window w, String status) {
	ToolDialog sd = new ToolDialog
		(PolicyTool.rb.getString("Status"), tool, this, true);

	// find the location of the PolicyTool gui
	Point location = ((w == null) ?
		getLocationOnScreen() : w.getLocationOnScreen());
	sd.setBounds(location.x + 50, location.y + 50, 500, 100);
	sd.setLayout(new GridBagLayout());

	Label label = new Label(status);
	addNewComponent(sd, label, 0,
			0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH);

	Button okButton = new Button(PolicyTool.rb.getString("OK"));
	okButton.addActionListener(new StatusOKButtonListener(sd));
	addNewComponent(sd, okButton, 1,
			0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL);
	sd.pack();
	sd.setVisible(true);
    }

    /**
     * display the warning log
     */
    void displayWarningLog(Window w) {

	ToolDialog wd = new ToolDialog
		(PolicyTool.rb.getString("Warning"), tool, this, true);

	// find the location of the PolicyTool gui
	Point location = ((w == null) ?
		getLocationOnScreen() : w.getLocationOnScreen());
	wd.setBounds(location.x + 50, location.y + 50, 500, 100);
	wd.setLayout(new GridBagLayout());

	TextArea ta = new TextArea();
	ta.setEditable(false);
	for (int i = 0; i < tool.warnings.size(); i++) {
	    ta.append((String)tool.warnings.elementAt(i));
	    ta.append(PolicyTool.rb.getString("\n"));
	}
	addNewComponent(wd, ta, 0,
			0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			BOTTOM_PADDING);

	Button okButton = new Button(PolicyTool.rb.getString("OK"));
	okButton.addActionListener(new CancelButtonListener(wd));
	addNewComponent(wd, okButton, 1,
			0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL,
			LR_PADDING);
	wd.pack();
	wd.setVisible(true);
    }
}

/**
 * General dialog window
 */
class ToolDialog extends Dialog {
    // use serialVersionUID from JDK 1.2.2 for interoperability
    private static final long serialVersionUID = -372244357011301190L;

    /* necessary constants */
    public static final int NOACTION		= 0;
    public static final int QUIT		= 1;
    public static final int NEW			= 2;
    public static final int OPEN		= 3;

    public static final String ALL_PERM_CLASS	= "java.security.AllPermission";
    public static final String AUDIO_PERM_CLASS	= "javax.sound.sampled.AudioPermission";
    public static final String AUTH_PERM_CLASS  = "javax.security.auth.AuthPermission";
    public static final String AWT_PERM_CLASS	= "java.awt.AWTPermission";
    public static final String DELEGATION_PERM_CLASS  =
	        "javax.security.auth.kerberos.DelegationPermission";
    public static final String FILE_PERM_CLASS	= "java.io.FilePermission";
    public static final String LOG_PERM_CLASS   = 
	        "java.util.logging.LoggingPermission";
    public static final String NET_PERM_CLASS	= "java.net.NetPermission";
    public static final String PRIVCRED_PERM_CLASS  =
		"javax.security.auth.PrivateCredentialPermission";
    public static final String PROP_PERM_CLASS	= 
	        "java.util.PropertyPermission";
    public static final String REFLECT_PERM_CLASS	=
		"java.lang.reflect.ReflectPermission";
    public static final String RUNTIME_PERM_CLASS  = "java.lang.RuntimePermission";
    public static final String SECURITY_PERM_CLASS =
		"java.security.SecurityPermission";
    public static final String SERIAL_PERM_CLASS   =
		"java.io.SerializablePermission";
    public static final String SERVICE_PERM_CLASS  =
		"javax.security.auth.kerberos.ServicePermission";
    public static final String SOCK_PERM_CLASS	= "java.net.SocketPermission";
    public static final String SQL_PERM_CLASS	= "java.sql.SQLPermission";
    public static final String SSL_PERM_CLASS	= 
	        "javax.net.ssl.SSLPermission";


    public static final String KERBEROS_PRIN_CLASS       =
		"javax.security.auth.kerberos.KerberosPrincipal";
    public static final String NTDOM_PRIN_CLASS       =
		"com.sun.security.auth.NTDomainPrincipal";
    public static final String NTSIDDOM_PRIN_CLASS    =
		"com.sun.security.auth.NTSidDomainPrincipal";
    public static final String NTSIDGRP_PRIN_CLASS    =
		"com.sun.security.auth.NTSidGroupPrincipal";
    public static final String NTSIDPRIMGRP_PRIN_CLASS    =
		"com.sun.security.auth.NTSidPrimaryGroupPrincipal";
    public static final String NTSIDUSER_PRIN_CLASS   =
		"com.sun.security.auth.NTSidUserPrincipal";
    public static final String NTUSER_PRIN_CLASS      =
		"com.sun.security.auth.NTUserPrincipal";
    public static final String UNIXNUMGRP_PRIN_CLASS   =
		"com.sun.security.auth.UnixNumericGroupPrincipal";
    public static final String UNIXNUMUSER_PRIN_CLASS  =
		"com.sun.security.auth.UnixNumericUserPrincipal";
    public static final String UNIX_PRIN_CLASS         =
		"com.sun.security.auth.UnixPrincipal";
    public static final String X500_PRIN_CLASS         =
		"javax.security.auth.x500.X500Principal";

    /* popup menus */
    public static final String PERM		=
	PolicyTool.rb.getString
	("Permission:                                                       ");
    public static final String ALL_PERM		= "AllPermission";
    public static final String AUDIO_PERM       = "AudioPermission";
    public static final String AUTH_PERM        = "AuthPermission";
    public static final String AWT_PERM		= "AWTPermission";
    public static final String DELEGATION_PERM  = "DelegationPermission";
    public static final String FILE_PERM	= "FilePermission";
    public static final String LOG_PERM    	= "LoggingPermission";
    public static final String NET_PERM		= "NetPermission";
    public static final String PRIVCRED_PERM    = "PrivateCredentialPermission";
    public static final String PROPERTY_PERM	= "PropertyPermission";
    public static final String REFLECT_PERM	= "ReflectPermission";
    public static final String RUNTIME_PERM	= "RuntimePermission";
    public static final String SECURITY_PERM	= "SecurityPermission";
    public static final String SERIAL_PERM	= "SerializablePermission";
    public static final String SERVICE_PERM     = "ServicePermission";
    public static final String SOCKET_PERM	= "SocketPermission";
    public static final String SQL_PERM	        = "SQLPermission";
    public static final String SSL_PERM         = "SSLPermission";

    public static final String PRIN_TYPE	=
        PolicyTool.rb.getString("Principal Type:");
    public static final String PRIN_NAME	=
	PolicyTool.rb.getString("Principal Name:");
    public static final String KERBEROS_PRIN    = "KerberosPrincipal";
    public static final String NTDOM_PRIN       = "NTDomainPrincipal";
    public static final String NTSIDDOM_PRIN    = "NTSidDomainPrincipal";
    public static final String NTSIDGRP_PRIN    = "NTSidGroupPrincipal";
    public static final String NTSIDPRIMGRP_PRIN    = "NTSidPrimaryGroupPrincipal";
    public static final String NTSIDUSER_PRIN   = "NTSidUserPrincipal";
    public static final String NTUSER_PRIN      = "NTUserPrincipal";
    public static final String UNIXNUMGRP_PRIN   = "UnixNumericGroupPrincipal";
    public static final String UNIXNUMUSER_PRIN  = "UnixNumericUserPrincipal";
    public static final String UNIX_PRIN         = "UnixPrincipal";
    public static final String X500_PRIN         = "X500Principal";

    /* more popu menus */
    public static final String PERM_NAME	=
	PolicyTool.rb.getString
	("Target Name:                                                    ");
    public static final String AUDIO_TARGETS[] = {
	"play", "record"
    };
    public static final String AUTH_TARGETS[]	= {
	"doAs", "doAsPrivileged", "getSubject", 
	"getSubjectFromDomainCombiner", "setReadOnly", "modifyPrincipals", 
	"modifyPublicCredentials", "modifyPrivateCredentials",
	"refreshCredential", "destroyCredential",
	"createLoginContext.<" + PolicyTool.rb.getString("name") + ">",
	"getLoginConfiguration", "setLoginConfiguration",
	"refreshLoginConfiguration"
    };
    public static final String AWT_TARGETS[] = {
        "accessClipboard", "accessEventQueue", "createRobot", 
        "fullScreenExclusive", "listenToAllAWTEvents",
        "readDisplayPixels", "replaceKeyboardFocusManager",
        "showWindowWithoutWarningBanner", "watchMousePointer",
        "setWindowAlwaysOnTop", "setAppletStub"
    };
    public static final String FILE_TARGETS[] 	= {
	"<<ALL FILES>>"
    };
    public static final String LOG_TARGETS[]	= {
	"control"
    };
    public static final String NET_TARGETS[]	= {
	"setDefaultAuthenticator", "requestPasswordAuthentication", 
	"specifyStreamHandler"
    };
    public static final String REF_TARGETS[] = {
	"suppressAccessChecks" 
    };
    public static final String RUN_TARGETS[] = {
	"createClassLoader", "getClassLoader", "setContextClassLoader", 
	"setSecurityManager", "createSecurityManager", "exitVM", 
	"shutdownHooks", "setFactory", "setIO", "modifyThread", 
	"stopThread", "modifyThreadGroup", "getProtectionDomain",
	"readFileDescriptor", "writeFileDescriptor",
	"loadLibrary.<" + PolicyTool.rb.getString("library name") + ">",
	"accessClassInPackage.<" + PolicyTool.rb.getString("package name") + ">",
	"defineClassInPackage.<" + PolicyTool.rb.getString("package name") + ">", 
	"accessDeclaredMembers", "queuePrintJob", "usePolicy",
	"enableContextClassLoaderOverride"
    };
    public static final String SEC_TARGETS[] = {
	"createAccessControlContext", "getDomainCombiner", "getPolicy",
	"setPolicy",
	"getProperty.<" + PolicyTool.rb.getString("property name") + ">",
	"setProperty.<" + PolicyTool.rb.getString("property name") + ">",
	"insertProvider.<" + PolicyTool.rb.getString("provider name") + ">",
	"removeProvider.<" + PolicyTool.rb.getString("provider name") + ">",
	"setSystemScope", "setIdentityPublicKey", "setIdentityInfo",
	"addIdentityCertificate", "removeIdentityCertificate", 
	"printIdentity",
	"clearProviderProperties.<" + PolicyTool.rb.getString("provider name") + ">",
	"putProviderProperty.<" + PolicyTool.rb.getString("provider name") + ">",
	"removeProviderProperty.<" + PolicyTool.rb.getString("provider name") + ">",
	"getSignerPrivateKey", "setSignerKeyPair" 
    };
    public static final String SER_TARGETS[] = {
	"enableSubclassImplementation", "enableSubstitution"
    };
    public static final String SQL_TARGETS[] = {
	"setLog"
    };
    public static final String SSL_TARGETS[] = {
	"setHostnameVerifier", "getSSLSessionContext"
    };

    /* and more popup menus */
    public static final String PERM_ACTIONS		=
      PolicyTool.rb.getString
      ("Actions:                                                             ");
    public static final String FILE_PERM_READ		= "read";
    public static final String FILE_PERM_WRITE		= "write";
    public static final String FILE_PERM_DELETE		= "delete";
    public static final String FILE_PERM_EXECUTE	= "execute";
    public static final String FILE_PERM_ALL		=
    				"read, write, delete, execute";
    public static final String PRIVCRED_PERM_READ       = "read";
    public static final String SERVICE_PERM_INIT        = "initiate";
    public static final String SERVICE_PERM_ACCEPT      = "accept";
    public static final String SERVICE_PERM_ALL         = "initiate, accept";

    public static final String PROP_PERM_READ		= "read";
    public static final String PROP_PERM_WRITE		= "write";
    public static final String PROP_PERM_ALL		= "read, write";
    public static final String SOCKET_PERM_ACCEPT	= "accept";
    public static final String SOCKET_PERM_CONNECT	= "connect";
    public static final String SOCKET_PERM_LISTEN	= "listen";
    public static final String SOCKET_PERM_RESOLVE	= "resolve";
    public static final String SOCKET_PERM_ALL		=
    				"accept, connect, listen, resolve";

    /* gridbag index for display OverWriteFile (OW) components */
    public static final int OW_LABEL			= 0;
    public static final int OW_OK_BUTTON		= 1;
    public static final int OW_CANCEL_BUTTON		= 2;

    /* gridbag index for display PolicyEntry (PE) components */
    public static final int PE_CODEBASE_LABEL		= 0;
    public static final int PE_CODEBASE_TEXTFIELD	= 1;
    public static final int PE_SIGNEDBY_LABEL		= 2;
    public static final int PE_SIGNEDBY_TEXTFIELD	= 3;

    public static final int PE_PANEL0			= 4;
    public static final int PE_ADD_PRIN_BUTTON          = 0;
    public static final int PE_EDIT_PRIN_BUTTON         = 1;
    public static final int PE_REMOVE_PRIN_BUTTON       = 2;

    public static final int PE_PRIN_LABEL               = 5;
    public static final int PE_PRIN_LIST                = 6;

    public static final int PE_PANEL1			= 7;
    public static final int PE_ADD_PERM_BUTTON		= 0;
    public static final int PE_EDIT_PERM_BUTTON 	= 1;
    public static final int PE_REMOVE_PERM_BUTTON	= 2;

    public static final int PE_PERM_LIST		= 8;

    public static final int PE_PANEL2			= 9;
    public static final int PE_CANCEL_BUTTON		= 0;
    public static final int PE_DONE_BUTTON		= 1;

    /* the gridbag index for components in the Principal Dialog (PRD) */
    public static final int PRD_DESC_LABEL              = 0;
    public static final int PRD_PRIN_CHOICE             = 1;
    public static final int PRD_PRIN_TEXTFIELD          = 2;
    public static final int PRD_NAME_LABEL              = 3;
    public static final int PRD_NAME_TEXTFIELD          = 4;
    public static final int PRD_CANCEL_BUTTON           = 5;
    public static final int PRD_OK_BUTTON               = 6;

    /* the gridbag index for components in the Permission Dialog (PD) */
    public static final int PD_DESC_LABEL		= 0;
    public static final int PD_PERM_CHOICE		= 1;
    public static final int PD_PERM_TEXTFIELD		= 2;
    public static final int PD_NAME_CHOICE		= 3;
    public static final int PD_NAME_TEXTFIELD		= 4;
    public static final int PD_ACTIONS_CHOICE		= 5;
    public static final int PD_ACTIONS_TEXTFIELD	= 6;
    public static final int PD_SIGNEDBY_LABEL		= 7;
    public static final int PD_SIGNEDBY_TEXTFIELD	= 8;
    public static final int PD_CANCEL_BUTTON		= 9;
    public static final int PD_OK_BUTTON		= 10;

    /* modes for KeyStore */
    public static final int CHANGE_KEYSTORE		= 0;

    /* the gridbag index for components in the Change KeyStore Dialog (KSD) */
    public static final int KSD_NAME_LABEL		= 0;
    public static final int KSD_NAME_TEXTFIELD		= 1;
    public static final int KSD_TYPE_LABEL		= 2;
    public static final int KSD_TYPE_TEXTFIELD		= 3;
    public static final int KSD_CANCEL_BUTTON		= 4;
    public static final int KSD_OK_BUTTON		= 5;

    /* the gridbag index for components in the User Save Changes Dialog (USC) */
    public static final int USC_LABEL			= 0;
    public static final int USC_PANEL			= 1;
    public static final int USC_YES_BUTTON		= 0;
    public static final int USC_NO_BUTTON		= 1;
    public static final int USC_CANCEL_BUTTON		= 2;

    /* gridbag index for the ConfirmRemovePolicyEntryDialog (CRPE) */
    public static final int CRPE_LABEL1			= 0;
    public static final int CRPE_LABEL2			= 1;

    /* some private static finals */
    private static final int PERMISSION			= 0;
    private static final int PERMISSION_NAME		= 1;
    private static final int PERMISSION_ACTIONS		= 2;
    private static final int PERMISSION_SIGNEDBY	= 3;
    private static final int PRINCIPAL_TYPE		= 4;
    private static final int PRINCIPAL_NAME		= 5;

    PolicyTool tool;
    ToolWindow tw;

    ToolDialog(String title, PolicyTool tool, ToolWindow tw, boolean modal) {
	super(tw, modal);
	setTitle(title);
	this.tool = tool;
	this.tw = tw;
	addWindowListener(new ChildWindowListener(this));
    }

    /**
     * ask user if they want to overwrite an existing file
     */
    void displayOverWriteFileDialog(String filename, int nextEvent) {

	// find where the PolicyTool gui is
	Point location = tw.getLocationOnScreen();
	setBounds(location.x + 75, location.y + 100, 400, 150);
	setLayout(new GridBagLayout());

	// ask the user if they want to over write the existing file
	MessageFormat form = new MessageFormat(PolicyTool.rb.getString
		("OK to overwrite existing file filename?"));
	Object[] source = {filename};
	Label label = new Label(form.format(source));
	tw.addNewComponent(this, label, OW_LABEL,
			   0, 0, 2, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.TOP_PADDING);

	// OK button
	Button button = new Button(PolicyTool.rb.getString("OK"));
	button.addActionListener(new OverWriteFileOKButtonListener
		(tool, tw, this, filename, nextEvent));
	tw.addNewComponent(this, button, OW_OK_BUTTON,
			   0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL,
			   tw.TOP_PADDING);

	// Cancel button
	// -- if the user hits cancel, do NOT go on to the next event
	button = new Button(PolicyTool.rb.getString("Cancel"));
	button.addActionListener(new CancelButtonListener(this));
	tw.addNewComponent(this, button, OW_CANCEL_BUTTON,
			   1, 1, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL,
			   tw.TOP_PADDING);

	setVisible(true);
    }

    /**
     * pop up a dialog so the user can enter info to add a new PolicyEntry
     * - if edit is TRUE, then the user is editing an existing entry
     *   and we should display the original info as well.
     *
     * - the other reason we need the 'edit' boolean is we need to know
     *   when we are adding a NEW policy entry.  in this case, we can
     *   not simply update the existing entry, because it doesn't exist.
     *   we ONLY update the GUI listing/info, and then when the user
     *   finally clicks 'OK' or 'DONE', then we can collect that info
     *   and add it to the policy.
     */
    void displayPolicyEntryDialog(boolean edit) {

	int listIndex = 0;
	PolicyEntry entries[] = null;
        List prinList = new List(3, false);
	prinList.addActionListener
		(new EditPrinButtonListener(tool, tw, this, edit));
	List permList = new List(10, false);
	permList.addActionListener
		(new EditPermButtonListener(tool, tw, this, edit));

	// find where the PolicyTool gui is
	Point location = tw.getLocationOnScreen();
	setBounds(location.x + 75, location.y + 200, 650, 500);
	setLayout(new GridBagLayout());
	setResizable(true);

	if (edit) {
	    // get the selected item
	    entries = tool.getEntry();
	    List policyList = (List)tw.getComponent(tw.MW_POLICY_LIST);
	    listIndex = policyList.getSelectedIndex();

	    // get principal list
	    LinkedList principals =
		entries[listIndex].getGrantEntry().principals;
	    for (int i = 0; i < principals.size(); i++) {
		String prinString = null;
		PolicyParser.PrincipalEntry nextPrin =
			(PolicyParser.PrincipalEntry)principals.get(i);
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		nextPrin.write(pw);
                prinString = sw.toString();
		prinList.add(prinString);
	    }

	    // get permission list
	    Vector permissions =
		entries[listIndex].getGrantEntry().permissionEntries;
	    for (int i = 0; i < permissions.size(); i++) {
		String permString = null;
		PolicyParser.PermissionEntry nextPerm =
			(PolicyParser.PermissionEntry)permissions.elementAt(i);
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		nextPerm.write(pw);
		if (File.separatorChar == '\\' &&
		    nextPerm.permission.equals(FILE_PERM_CLASS)) {
		    /*
		     * XXX  in windows, the double backslash originally in
		     *      the policy file gets automatically converted
		     *      into a single backslash every time we read it in.
		     *      so, we have to add the double backslash again.
		     *      sigh ...
		     */
		    nextPerm.name = addSingleBackSlash(nextPerm.name);
		    permString = addSingleBackSlash(sw.toString());
		} else {
		    permString = sw.toString();
		}
		permString = permString.substring
				(0, permString.indexOf(";") + 1);
		permList.add(permString);
	    }
	}

	// codebase label and textfield
	Label label = new Label(PolicyTool.rb.getString("CodeBase:"));
	tw.addNewComponent(this, label, PE_CODEBASE_LABEL,
		0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH);
	TextField tf;
	tf = (edit ?
		new TextField(entries[listIndex].getGrantEntry().codeBase, 60) :
		new TextField(60));
	tw.addNewComponent(this, tf, PE_CODEBASE_TEXTFIELD,
		1, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH);

	// signedby label and textfield
	label = new Label(PolicyTool.rb.getString("SignedBy:"));
	tw.addNewComponent(this, label, PE_SIGNEDBY_LABEL,
			   0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH);
	tf = (edit ?
		new TextField(entries[listIndex].getGrantEntry().signedBy, 60) :
		new TextField(60));
	tw.addNewComponent(this, tf, PE_SIGNEDBY_TEXTFIELD,
			   1, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH);

	// panel for principal buttons
	Panel panel = new Panel();
	panel.setLayout(new GridBagLayout());

	Button button = new Button(PolicyTool.rb.getString("Add Principal"));
	button.addActionListener(new AddPrinButtonListener(tool, tw, this, edit));
	tw.addNewComponent(panel, button, PE_ADD_PRIN_BUTTON,
			   0, 0, 1, 1, 100.0, 0.0, GridBagConstraints.HORIZONTAL);

	button = new Button(PolicyTool.rb.getString("Edit Principal"));
	button.addActionListener(new EditPrinButtonListener
						(tool, tw, this, edit));
	tw.addNewComponent(panel, button, PE_EDIT_PRIN_BUTTON,
			   1, 0, 1, 1, 100.0, 0.0, GridBagConstraints.HORIZONTAL);

	button = new Button(PolicyTool.rb.getString("Remove Principal"));
	button.addActionListener(new RemovePrinButtonListener
					(tool, tw, this, edit));
	tw.addNewComponent(panel, button, PE_REMOVE_PRIN_BUTTON,
			   2, 0, 1, 1, 100.0, 0.0, GridBagConstraints.HORIZONTAL);

	tw.addNewComponent(this, panel, PE_PANEL0,
			   1, 2, 1, 1, 0.0, 0.0, GridBagConstraints.HORIZONTAL);

	// principal label and list
	label = new Label(PolicyTool.rb.getString("Principals:"));
	tw.addNewComponent(this, label, PE_PRIN_LABEL,
			   0, 3, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.BOTTOM_PADDING);
	tw.addNewComponent(this, prinList, PE_PRIN_LIST,
			   1, 3, 3, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.BOTTOM_PADDING);

	// panel for permission buttons
	panel = new Panel();
	panel.setLayout(new GridBagLayout());

	button = new Button(PolicyTool.rb.getString("  Add Permission"));
	button.addActionListener(new AddPermButtonListener
						(tool, tw, this, edit));
	tw.addNewComponent(panel, button, PE_ADD_PERM_BUTTON,
			    0, 0, 1, 1, 100.0, 0.0, GridBagConstraints.HORIZONTAL);

	button = new Button(PolicyTool.rb.getString("  Edit Permission"));
	button.addActionListener(new EditPermButtonListener
						(tool, tw, this, edit));
	tw.addNewComponent(panel, button, PE_EDIT_PERM_BUTTON,
			   1, 0, 1, 1, 100.0, 0.0, GridBagConstraints.HORIZONTAL);


	button = new Button(PolicyTool.rb.getString("Remove Permission"));
	button.addActionListener(new RemovePermButtonListener
					(tool, tw, this, edit));
	tw.addNewComponent(panel, button, PE_REMOVE_PERM_BUTTON,
			   2, 0, 1, 1, 100.0, 0.0, GridBagConstraints.HORIZONTAL);

	tw.addNewComponent(this, panel, PE_PANEL1,
			   0, 4, 2, 1, 0.0, 0.0, GridBagConstraints.HORIZONTAL,
			    tw.LITE_BOTTOM_PADDING);

	// permission list
	tw.addNewComponent(this, permList, PE_PERM_LIST,
			   0, 5, 3, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.BOTTOM_PADDING);


	// panel for Done and Cancel buttons
	panel = new Panel();
	panel.setLayout(new GridBagLayout());

	// Cancel Button
	button = new Button(PolicyTool.rb.getString("Cancel"));
	button.addActionListener(new CancelButtonListener(this));
	tw.addNewComponent(panel, button, PE_CANCEL_BUTTON,
			   1, 0, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL,
			   tw.LR_PADDING);

	// Done Button
	button = new Button(PolicyTool.rb.getString("Done"));
	button.addActionListener
		(new AddEntryDoneButtonListener(tool, tw, this, edit));
	tw.addNewComponent(panel, button, PE_DONE_BUTTON,
			   0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL,
			   tw.LR_PADDING);

	// add the panel
	tw.addNewComponent(this, panel, PE_PANEL2,
		0, 6, 2, 1, 0.0, 0.0, GridBagConstraints.VERTICAL);

	setVisible(true);
    }

    /**
     * Read all the Policy information data in the dialog box
     * and construct a PolicyEntry object with it.
     */
    PolicyEntry getPolicyEntryFromDialog()
    throws InvalidParameterException, MalformedURLException,
    NoSuchMethodException, ClassNotFoundException, InstantiationException,
    IllegalAccessException, InvocationTargetException,
    CertificateException, IOException, Exception {

	// get the Codebase
	TextField tf = (TextField)getComponent(PE_CODEBASE_TEXTFIELD);
	String codebase = null;
	if (tf.getText().trim().equals("") == false)
		codebase = new String(tf.getText().trim());

	// get the SignedBy
	tf = (TextField)getComponent(PE_SIGNEDBY_TEXTFIELD);
	String signedby = null;
	if (tf.getText().trim().equals("") == false)
		signedby = new String(tf.getText().trim());

	// construct a new GrantEntry
	PolicyParser.GrantEntry ge =
			new PolicyParser.GrantEntry(signedby, codebase);

	// get the new Principals
	LinkedList prins = new LinkedList();
	List prinList = (List)getComponent(PE_PRIN_LIST);
	String prinStrings[] = prinList.getItems();
	for (int i = 0; i < prinStrings.length; i++) {
	    PolicyParser.PrincipalEntry pppe = new PolicyParser.PrincipalEntry
		(parsePrincipalString(prinStrings[i], PRINCIPAL_TYPE),
		parsePrincipalString(prinStrings[i], PRINCIPAL_NAME));
	    prins.add(pppe);
	}
	ge.principals = prins;

	// get the new Permissions
	Vector perms = new Vector();
	List permList = (List)getComponent(PE_PERM_LIST);
	String permStrings[] = permList.getItems();
	for (int i = 0; i < permStrings.length; i++) {
	    PolicyParser.PermissionEntry pppe = new PolicyParser.PermissionEntry
		(parsePermissionString(permStrings[i], PERMISSION),
		parsePermissionString(permStrings[i], PERMISSION_NAME),
		parsePermissionString(permStrings[i], PERMISSION_ACTIONS));
	    pppe.signedBy =
		parsePermissionString(permStrings[i], PERMISSION_SIGNEDBY);
	    perms.addElement(pppe);
	}
	ge.permissionEntries = perms;

	// construct a new PolicyEntry object
	PolicyEntry entry = new PolicyEntry(tool, ge);

	return entry;
    }

    /**
     * display a dialog box for the user to enter KeyStore information
     */
    void keyStoreDialog(int mode) {

	// find where the PolicyTool gui is
	Point location = tw.getLocationOnScreen();
	setBounds(location.x + 25, location.y + 100, 500, 200);
	setLayout(new GridBagLayout());

	if (mode == CHANGE_KEYSTORE) {

	    // KeyStore label and textfield
	    Label label = new Label
			(PolicyTool.rb.getString("New KeyStore URL:"));
	    tw.addNewComponent(this, label, KSD_NAME_LABEL,
			       0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			       tw.BOTTOM_PADDING);
	    TextField tf = new TextField(tool.getKeyStoreName(), 30);
	    tw.addNewComponent(this, tf, KSD_NAME_TEXTFIELD,
			       1, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			       tw.BOTTOM_PADDING);

	    // KeyStore type and textfield
	    label = new Label(PolicyTool.rb.getString("New KeyStore Type:"));
	    tw.addNewComponent(this, label, KSD_TYPE_LABEL,
			       0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			       tw.BOTTOM_PADDING);
	    tf = new TextField(tool.getKeyStoreType(), 30);
	    tw.addNewComponent(this, tf, KSD_TYPE_TEXTFIELD,
			       1, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			       tw.BOTTOM_PADDING);

	    // cancel button
	    Button cancelButton = new Button(PolicyTool.rb.getString("Cancel"));
	    cancelButton.addActionListener(new CancelButtonListener(this));
	    tw.addNewComponent(this, cancelButton, KSD_CANCEL_BUTTON,
			1, 2, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL);

	    // OK button
	    Button okButton = new Button(PolicyTool.rb.getString("OK"));
	    okButton.addActionListener
			(new ChangeKeyStoreOKButtonListener(tool, tw, this));
	    tw.addNewComponent(this, okButton, KSD_OK_BUTTON,
			0, 2, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL);

	}
	setVisible(true);
    }

    /**
     * display a dialog box for the user to input Principal info
     *
     * if editPolicyEntry is false, then we are adding Principals to
     * a new PolicyEntry, and we only update the GUI listing
     * with the new Principal.
     *
     * if edit is true, then we are editing an existing Policy entry.
     */
    void displayPrincipalDialog(boolean editPolicyEntry, boolean edit) {

	PolicyParser.PrincipalEntry editMe = null;

	// get the Principal selected from the Principal List
	List prinList = (List)getComponent(PE_PRIN_LIST);
	int prinIndex = prinList.getSelectedIndex();

	if (!editPolicyEntry && edit) {

	    // construct a PolicyParser.PrincipalEntry from the
	    // selected item in the Principal List

	    String prin = parsePrincipalString
			(prinList.getItem(prinIndex), PRINCIPAL_TYPE);
	    String prinName = parsePrincipalString
			(prinList.getItem(prinIndex), PRINCIPAL_NAME);
	    editMe = new PolicyParser.PrincipalEntry
				(prin, prinName);

	} else if (editPolicyEntry && edit) {

	    // get the PolicyEntry selected from the main ToolWindow
	    List policyList = (List)tw.getComponent(tw.MW_POLICY_LIST);
	    int policyIndex = policyList.getSelectedIndex();

	    PolicyEntry entries[] = tool.getEntry();
	    LinkedList prins =
			entries[policyIndex].getGrantEntry().principals;
	    editMe = (PolicyParser.PrincipalEntry)prins.get(prinIndex);
	}

	ToolDialog newTD = new ToolDialog
		(PolicyTool.rb.getString("Principals"), tool, tw, true);
	newTD.addWindowListener(new ChildWindowListener(newTD));

	// find where the PolicyTool gui is
	Point location = getLocationOnScreen();
	newTD.setBounds(location.x + 50, location.y + 100, 650, 190);
	newTD.setLayout(new GridBagLayout());
	newTD.setResizable(true);

	// description label
	Label label = (edit ?
		new Label(PolicyTool.rb.getString("  Edit Principal:")) :
		new Label(PolicyTool.rb.getString("  Add New Principal:")));
	tw.addNewComponent(newTD, label, PRD_DESC_LABEL,
			   0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.BOTTOM_PADDING);

	// principal choice
	Choice choice = new Choice();
	choice.add(PRIN_TYPE);
	choice.add(KERBEROS_PRIN);
	choice.add(NTDOM_PRIN);
	choice.add(NTSIDDOM_PRIN);
	choice.add(NTSIDGRP_PRIN);
	choice.add(NTSIDPRIMGRP_PRIN);
	choice.add(NTSIDUSER_PRIN);
	choice.add(NTUSER_PRIN);
	choice.add(UNIXNUMGRP_PRIN);
	choice.add(UNIXNUMUSER_PRIN);
        choice.add(UNIX_PRIN);
	choice.add(X500_PRIN);

	choice.addItemListener(new PrincipalTypeMenuListener(newTD));
	if (edit)
	    setPrincipals(editMe.getPrincipalClass(), choice);

	tw.addNewComponent(newTD, choice, PRD_PRIN_CHOICE,
			   0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);

	// principal textfield
	TextField tf;
	tf = (edit ? new TextField(editMe.getDisplayClass(), 30) : new TextField(30));

        tw.addNewComponent(newTD, tf, PRD_PRIN_TEXTFIELD,
			   1, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);

	// name label and textfield
	label = new Label(PRIN_NAME);
	tf = (edit ? new TextField(editMe.getDisplayName(), 40) : new TextField(40));

        tw.addNewComponent(newTD, label, PRD_NAME_LABEL,
			   0, 2, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);
	tw.addNewComponent(newTD, tf, PRD_NAME_TEXTFIELD,
			   1, 2, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);

	// cancel button
	Button cancelButton = new Button(PolicyTool.rb.getString("Cancel"));
	cancelButton.addActionListener(new CancelButtonListener(newTD));
	tw.addNewComponent(newTD, cancelButton, PRD_CANCEL_BUTTON,
			   1, 3, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL,
			   tw.TOP_PADDING);

	// OK button
	Button okButton = new Button(PolicyTool.rb.getString("OK"));
	if (editPolicyEntry)
	    okButton.addActionListener(
		new EditPolicyPrinOKButtonListener
					(tool, tw, this, newTD, edit));
	else
	    okButton.addActionListener(
		new NewPolicyPrinOKButtonListener
					(tool, tw, this, newTD, edit));
	tw.addNewComponent(newTD, okButton, PRD_OK_BUTTON,
			   0, 3, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL,
			   tw.TOP_PADDING);
	newTD.setVisible(true);
    }

    /**
     * display a dialog box for the user to input Permission info
     *
     * if editPolicyEntry is false, then we are adding Permissions to
     * a new PolicyEntry, and we only update the GUI listing
     * with the new Permission.
     *
     * if edit is true, then we are editing an existing Permission entry.
     */
    void displayPermissionDialog(boolean editPolicyEntry, boolean edit) {

	PolicyParser.PermissionEntry editMe = null;

	// get the Permission selected from the Permission List
	List permList = (List)getComponent(PE_PERM_LIST);
	int permIndex = permList.getSelectedIndex();

	if (!editPolicyEntry && edit) {

	    // construct a PolicyParser.PermissionEntry from the
	    // selected item in the Permission List

	    String perm = parsePermissionString
			(permList.getItem(permIndex), PERMISSION);
	    String permName = parsePermissionString
			(permList.getItem(permIndex), PERMISSION_NAME);
	    String permActions = parsePermissionString
			(permList.getItem(permIndex), PERMISSION_ACTIONS);
	    editMe = new PolicyParser.PermissionEntry
				(perm, permName, permActions);
	    editMe.signedBy = parsePermissionString
			(permList.getItem(permIndex), PERMISSION_SIGNEDBY);

	} else if (editPolicyEntry && edit) {

	    // get the PolicyEntry selected from the main ToolWindow
	    List policyList = (List)tw.getComponent(tw.MW_POLICY_LIST);
	    int policyIndex = policyList.getSelectedIndex();

	    PolicyEntry entries[] = tool.getEntry();
	    Vector perms =
			entries[policyIndex].getGrantEntry().permissionEntries;
	    editMe = (PolicyParser.PermissionEntry)perms.elementAt(permIndex);
	}

	ToolDialog newTD = new ToolDialog
		(PolicyTool.rb.getString("Permissions"), tool, tw, true);
	newTD.addWindowListener(new ChildWindowListener(newTD));

	// find where the PolicyTool gui is
	Point location = getLocationOnScreen();
	newTD.setBounds(location.x + 50, location.y + 100, 700, 250);
	newTD.setLayout(new GridBagLayout());
	newTD.setResizable(true);

	// description label
	Label label = (edit ?
		new Label(PolicyTool.rb.getString("  Edit Permission:")) :
		new Label(PolicyTool.rb.getString("  Add New Permission:")));
	tw.addNewComponent(newTD, label, PD_DESC_LABEL,
			   0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.BOTTOM_PADDING);

	// permission choice (added in alphabetical order)
	Choice choice = new Choice();
	choice.add(PERM);
	choice.add(ALL_PERM);
	choice.add(AUDIO_PERM);
	choice.add(AUTH_PERM);
	choice.add(AWT_PERM);
	choice.add(DELEGATION_PERM);
	choice.add(FILE_PERM);
	choice.add(LOG_PERM);
	choice.add(NET_PERM);
	choice.add(PRIVCRED_PERM);
	choice.add(PROPERTY_PERM);
	choice.add(REFLECT_PERM);
	choice.add(RUNTIME_PERM);
	choice.add(SECURITY_PERM);
	choice.add(SERIAL_PERM);
	choice.add(SERVICE_PERM);
	choice.add(SOCKET_PERM);
	choice.add(SQL_PERM);
	choice.add(SSL_PERM);
	choice.addItemListener(new PermissionMenuListener(newTD));
	tw.addNewComponent(newTD, choice, PD_PERM_CHOICE,
			   0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);

	// permission textfield
	TextField tf;
	tf = (edit ? new TextField(editMe.permission, 30) : new TextField(30));
	if (edit)
	    setPermissions(editMe.permission, choice);
	tw.addNewComponent(newTD, tf, PD_PERM_TEXTFIELD,
			   1, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);

	// name label and textfield
	choice = new Choice();
	choice.add(PERM_NAME);
	choice.addItemListener(new PermissionNameMenuListener(newTD));
	tf = (edit ? new TextField(editMe.name, 40) : new TextField(40));
	if (edit)
	    setPermissionNames(editMe.permission, choice, tf);
	tw.addNewComponent(newTD, choice, PD_NAME_CHOICE,
			   0, 2, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);
	tw.addNewComponent(newTD, tf, PD_NAME_TEXTFIELD,
			   1, 2, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);

	// actions label and textfield
	choice = new Choice();
	choice.add(PERM_ACTIONS);
	choice.addItemListener(new PermissionActionsMenuListener(newTD));
	tf = (edit ? new TextField(editMe.action, 40) : new TextField(40));
	if (edit)
	    setPermissionActions(editMe.permission, choice, tf);
	tw.addNewComponent(newTD, choice, PD_ACTIONS_CHOICE,
			   0, 3, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);
	tw.addNewComponent(newTD, tf, PD_ACTIONS_TEXTFIELD,
			   1, 3, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);

	// signedby label and textfield
	label = new Label(PolicyTool.rb.getString("Signed By:"));
	tw.addNewComponent(newTD, label, PD_SIGNEDBY_LABEL,
			   0, 4, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);
	tf = (edit ? new TextField(editMe.signedBy, 40) : new TextField(40));
	tw.addNewComponent(newTD, tf, PD_SIGNEDBY_TEXTFIELD,
			   1, 4, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.LR_PADDING);

	// cancel button
	Button cancelButton = new Button(PolicyTool.rb.getString("Cancel"));
	cancelButton.addActionListener(new CancelButtonListener(newTD));
	tw.addNewComponent(newTD, cancelButton, PD_CANCEL_BUTTON,
			   1, 5, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL,
			   tw.TOP_PADDING);

	// OK button
	Button okButton = new Button(PolicyTool.rb.getString("OK"));
	if (editPolicyEntry)
	    okButton.addActionListener(
		new EditPolicyPermOKButtonListener
					(tool, tw, this, newTD, edit));
	else
	    okButton.addActionListener(
		new NewPolicyPermOKButtonListener
					(tool, tw, this, newTD, edit));
	tw.addNewComponent(newTD, okButton, PD_OK_BUTTON,
			   0, 5, 1, 1, 0.0, 0.0, GridBagConstraints.VERTICAL,
			   tw.TOP_PADDING);

	newTD.setVisible(true);
    }

    /**
     * construct a Principal object from the Principal Info Dialog Box
     */
    PolicyParser.PrincipalEntry getPrinFromDialog() {

	TextField tf = (TextField)getComponent(PRD_PRIN_TEXTFIELD);
	String pclass = new String(tf.getText().trim());
	tf = (TextField)getComponent(PRD_NAME_TEXTFIELD);
	String pname = new String(tf.getText().trim());
	if (pclass.equals("*")) {
	    pclass = PolicyParser.PrincipalEntry.WILDCARD_CLASS;
	}
	if (pname.equals("*")) {
	    pname = PolicyParser.PrincipalEntry.WILDCARD_NAME;
	}

	PolicyParser.PrincipalEntry pppe = null;
	try {
	    if ((pclass.equals(PolicyParser.PrincipalEntry.WILDCARD_CLASS)) &&
	        (!pname.equals(PolicyParser.PrincipalEntry.WILDCARD_NAME))) {
	        throw new Exception
		    (PolicyTool.rb.getString("Cannot Specify Principal with a Wildcard Class without a Wildcard Name"));
	    }
	    else if (pclass.equals("")) {
		// make this consistent with what PolicyParser does
		// when it sees an empty principal class
		pclass = PolicyParser.REPLACE_NAME;
		tool.warnings.addElement(
			"Warning: Principal name '" + pname +
				"' specified without a Principal class.\n" +
			"\t'" + pname + "' will be interpreted " +
				"as a key store alias.\n" +
			"\tThe final principal class will be " +
				ToolDialog.X500_PRIN_CLASS + ".\n" +
			"\tThe final principal name will be " +
				"determined by the following:\n" +
			"\n" +
			"\tIf the key store entry identified by '"
				+ pname + "'\n" +
			"\tis a key entry, then the principal name will be\n" +
			"\tthe subject distinguished name from the first\n" +
			"\tcertificate in the entry's certificate chain.\n" +
			"\n" +
			"\tIf the key store entry identified by '" +
				pname + "'\n" +
			"\tis a trusted certificate entry, then the\n" +
			"\tprincipal name will be the subject distinguished\n" +
			"\tname from the trusted public key certificate.");
		tw.displayStatusDialog(this,
			"'" + pname + "' will be interpreted as a key " +
			"store alias.  View Warning Log for details.");
	    }
	    else if (pname.equals("")) {
		throw new Exception
		    (PolicyTool.rb.getString("Cannot Specify Principal without a Name"));
	    }
	    pppe = new PolicyParser.PrincipalEntry(pclass, pname);
	}
	catch (Exception ex) {
	    tw.displayErrorDialog(this,ex);
	}
	return pppe;
    }


    /**
     * construct a Permission object from the Permission Info Dialog Box
     */
    PolicyParser.PermissionEntry getPermFromDialog() {

	TextField tf = (TextField)getComponent(PD_PERM_TEXTFIELD);
	String permission = new String(tf.getText().trim());
	tf = (TextField)getComponent(PD_NAME_TEXTFIELD);
	String name = null;
	if (tf.getText().trim().equals("") == false)
	    name = new String(tf.getText().trim());
	if (permission.equals("") ||
	    (!permission.equals(ALL_PERM_CLASS) && name == null)) {
	    throw new InvalidParameterException(PolicyTool.rb.getString
		("Permission and Target Name must have a value"));
	}

	// get the Actions
	tf = (TextField)getComponent(PD_ACTIONS_TEXTFIELD);
	String actions = null;
	if (tf.getText().trim().equals("") == false)
	    actions = new String(tf.getText().trim());

	// get the Signed By
	tf = (TextField)getComponent(PD_SIGNEDBY_TEXTFIELD);
	String signedBy = null;
	if (tf.getText().trim().equals("") == false)
	    signedBy = new String(tf.getText().trim());

	PolicyParser.PermissionEntry pppe = new PolicyParser.PermissionEntry
				(permission, name, actions);
	pppe.signedBy = signedBy;

	// see if the signers have public keys
	if (signedBy != null) {
		String signers[] = tool.parseSigners(pppe.signedBy);
		for (int i = 0; i < signers.length; i++) {
		try {
		    PublicKey pubKey = tool.getPublicKeyAlias(signers[i]);
		    if (pubKey == null) {
			MessageFormat form = new MessageFormat
				(PolicyTool.rb.getString
				("Warning: A public key for alias 'signers[i]' does not exist."));
			Object[] source = {signers[i]};
			tool.warnings.addElement(form.format(source));
			tw.displayStatusDialog(this, form.format(source));
		    }
		} catch (Exception e) {
		    tw.displayErrorDialog(this, e);
		}
	    }
	}
	return pppe;
    }

    /**
     * confirm that the user REALLY wants to remove the Policy Entry
     */
    void displayConfirmRemovePolicyEntry() {

	// find the entry to be removed
	List list = (List)tw.getComponent(tw.MW_POLICY_LIST);
	int index = list.getSelectedIndex();
	PolicyEntry entries[] = tool.getEntry();

	// find where the PolicyTool gui is
	Point location = tw.getLocationOnScreen();
	setBounds(location.x + 25, location.y + 100, 600, 400);
	setLayout(new GridBagLayout());

	// ask the user do they really want to do this?
	Label label = new Label
		(PolicyTool.rb.getString("Remove this Policy Entry?"));
	tw.addNewComponent(this, label, CRPE_LABEL1,
			   0, 0, 2, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			   tw.BOTTOM_PADDING);

	// display the policy entry
	label = new Label("Grant " + entries[index].codebaseToString());
	tw.addNewComponent(this, label, CRPE_LABEL2,
			1, 1, 2, 1, 0.0, 0.0, GridBagConstraints.BOTH);
	label = new Label(entries[index].principalsToString());
	tw.addNewComponent(this, label, CRPE_LABEL2+1,
			1, 2, 2, 1, 0.0, 0.0, GridBagConstraints.BOTH);
	Vector perms = entries[index].getGrantEntry().permissionEntries;
	for (int i = 0; i < perms.size(); i++) {
	    PolicyParser.PermissionEntry nextPerm =
			(PolicyParser.PermissionEntry)perms.elementAt(i);
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    nextPerm.write(pw);
	    String permString = sw.toString();
	    label = new Label("    " +
		permString.substring(0, permString.indexOf(";") + 1));
	    if (i == (perms.size()-1)) {
		tw.addNewComponent(this, label, CRPE_LABEL2 + 2 + i,
				 1, 3 + i, 2, 1, 0.0, 0.0,
				 GridBagConstraints.BOTH, tw.BOTTOM_PADDING);
	    } else {
		tw.addNewComponent(this, label, CRPE_LABEL2 + 2 + i,
				 1, 3 + i, 2, 1, 0.0, 0.0,
				 GridBagConstraints.BOTH);
	    }
	}

	// cancel button
	Button cancelButton = new Button(PolicyTool.rb.getString("Cancel"));
	cancelButton.addActionListener(new CancelButtonListener(this));
	tw.addNewComponent(this, cancelButton, CRPE_LABEL2 + 2 + perms.size(),
			   2, 3 + perms.size(), 1, 1, 0.0, 0.0,
			   GridBagConstraints.VERTICAL, tw.BOTTOM_PADDING);

	// OK button
	Button okButton = new Button(PolicyTool.rb.getString("OK"));
	okButton.addActionListener
		(new ConfirmRemovePolicyEntryOKButtonListener(tool, tw, this));
	tw.addNewComponent(this, okButton, CRPE_LABEL2 + 3 + perms.size(),
			   1, 3 + perms.size(), 1, 1, 0.0, 0.0,
			   GridBagConstraints.VERTICAL, tw.BOTTOM_PADDING);

	pack();
	setVisible(true);
    }

    /**
     * perform SAVE AS
     */
    void displaySaveAsDialog(int nextEvent) {

	// pop up a dialog box for the user to enter a filename.
	FileDialog fd = new FileDialog
		(tw, PolicyTool.rb.getString("Save As"), FileDialog.SAVE);
	fd.setVisible(true);

	// see if the user hit cancel
	if (fd.getFile() == null ||
	    fd.getFile().equals(""))
	    return;

	// get the entered filename
	String filename = new String(fd.getDirectory() + fd.getFile());
	fd.dispose();

	// see if the file already exists
	File saveAsFile = new File(filename);
	if (saveAsFile.exists()) {
	    // display a dialog box for the user to enter policy info
	    ToolDialog td = new ToolDialog
		(PolicyTool.rb.getString("Overwrite File"), tool, tw, true);
	    td.displayOverWriteFileDialog(filename, nextEvent);
	} else {
	    try {
		// save the policy entries to a file
		tool.savePolicy(filename);

		// display status
		MessageFormat form = new MessageFormat(PolicyTool.rb.getString
			("Policy successfully written to filename"));
		Object[] source = {filename};
		tw.displayStatusDialog(null, form.format(source));

		// display the new policy filename
		TextField newFilename = (TextField)tw.getComponent
				(tw.MW_FILENAME_TEXTFIELD);
		newFilename.setText(filename);
		tw.setVisible(true);

		// now continue with the originally requested command
		// (QUIT, NEW, or OPEN)
		userSaveContinue(tool, tw, this, nextEvent);

	    } catch (FileNotFoundException fnfe) {
		if (filename == null || filename.equals("")) {
		    tw.displayErrorDialog(null, new FileNotFoundException
				(PolicyTool.rb.getString("null filename")));
		} else {
		    MessageFormat form = new MessageFormat
			(PolicyTool.rb.getString("filename not found"));
		    Object[] source = {filename};
		    tw.displayErrorDialog(null, new FileNotFoundException
					(form.format(source)));
		}
	    } catch (Exception ee) {
		tw.displayErrorDialog(null, ee);
	    }
	}
    }

    /**
     * ask user if they want to save changes
     */
    void displayUserSave(int select) {

	if (tool.modified == true) {

	    // find where the PolicyTool gui is
	    Point location = tw.getLocationOnScreen();
	    setBounds(location.x + 75, location.y + 100, 400, 150);
	    setLayout(new GridBagLayout());

	    Label label = new Label
		(PolicyTool.rb.getString("     Save changes?"));
	    tw.addNewComponent(this, label, USC_LABEL,
			       0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH,
			       tw.BOTTOM_PADDING);

	    Panel panel = new Panel();
	    panel.setLayout(new GridBagLayout());

	    Button yesButton = new Button(PolicyTool.rb.getString("Yes"));
	    yesButton.addActionListener
			(new UserSaveYesButtonListener(this, tool, tw, select));
	    tw.addNewComponent(panel, yesButton, USC_YES_BUTTON,
			       0, 0, 1, 1, 0.0, 0.0,
			       GridBagConstraints.VERTICAL,
			       tw.LR_PADDING);
	    Button noButton = new Button(PolicyTool.rb.getString("No"));
	    noButton.addActionListener
			(new UserSaveNoButtonListener(this, tool, tw, select));
	    tw.addNewComponent(panel, noButton, USC_NO_BUTTON,
			       1, 0, 1, 1, 0.0, 0.0,
			       GridBagConstraints.VERTICAL,
			       tw.LR_PADDING);
	    Button cancelButton = new Button(PolicyTool.rb.getString("Cancel"));
	    cancelButton.addActionListener
			(new UserSaveCancelButtonListener(this));
	    tw.addNewComponent(panel, cancelButton, USC_CANCEL_BUTTON,
			       2, 0, 1, 1, 0.0, 0.0,
			       GridBagConstraints.VERTICAL,
			       tw.LR_PADDING);

	    tw.addNewComponent(this, panel, USC_PANEL,
			       0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.BOTH);

	    pack();
	    setVisible(true);
	} else {
	    // just do the original request (QUIT, NEW, or OPEN)
	    userSaveContinue(tool, tw, this, select);
	}
    }

    /**
     * when the user sees the 'YES', 'NO', 'CANCEL' buttons on the
     * displayUserSave dialog, and the click on one of them,
     * we need to continue the originally requested action
     * (either QUITting, opening NEW policy file, or OPENing an existing
     * policy file.  do that now.
     */
    void userSaveContinue(PolicyTool tool, ToolWindow tw,
			ToolDialog us, int select) {

	// now either QUIT, open a NEW policy file, or OPEN an existing policy
	switch(select) {
	case ToolDialog.QUIT:

	    tw.setVisible(false);
	    tw.dispose();
	    System.exit(0);

	case ToolDialog.NEW:

	    try {
		tool.openPolicy(null);
	    } catch (Exception ee) {
		tool.modified = false;
		tw.displayErrorDialog(null, ee);
	    }

	    // display the policy entries via the policy list textarea
	    List list = new List(40, false);
	    list.addActionListener(new PolicyListListener(tool, tw));
	    tw.replacePolicyList(list);

	    // display null policy filename and keystore
	    TextField newFilename = (TextField)
				tw.getComponent(tw.MW_FILENAME_TEXTFIELD);
	    newFilename.setText("");
	    TextField ksName = (TextField)
				tw.getComponent(tw.MW_KEYSTORE_TEXTFIELD);
	    ksName.setText("");
	    tw.setVisible(true);
	    break;

	case ToolDialog.OPEN:

	    // pop up a dialog box for the user to enter a filename.
	    FileDialog fd = new FileDialog
		(tw, PolicyTool.rb.getString("Open"), FileDialog.LOAD);
	    fd.setVisible(true);

	    // see if the user hit 'cancel'
	    if (fd.getFile() == null ||
		fd.getFile().equals(""))
		return;

	    // get the entered filename
	    String filename = new String(fd.getDirectory() + fd.getFile());

	    try {
		// open and read the policy file
		tool.openPolicy(filename);

	    } catch (FileNotFoundException fnfe) {
		if (filename == null || filename.equals("")) {
		    tw.displayErrorDialog(null, new FileNotFoundException
				(PolicyTool.rb.getString("null filename")));
		} else {
		    MessageFormat form = new MessageFormat
			(PolicyTool.rb.getString("filename not found"));
		    Object[] source = {filename};
		    tw.displayErrorDialog(null, new FileNotFoundException
					(form.format(source)));
		}
	    } catch (PolicyParser.ParsingException pppe) {
		MessageFormat form = new MessageFormat(PolicyTool.rb.getString
			("Error parsing policy file policyFile: pppe.getMessage()"));
		Object[] source = {filename, pppe.getMessage()};
		tw.displayErrorDialog(null, form.format(source));
		form = new MessageFormat(PolicyTool.rb.getString
			("Error: Could not open policy file, filename, because of parsing error: pppe.getMessage()"));
		Object[] src = {filename, pppe.getMessage()};
		tool.warnings.addElement(form.format(source));
	    } catch (NoSuchMethodException nsme) {
		tw.displayErrorDialog
		  (null, new Exception(PolicyTool.rb.getString
		  ("Permission could not be mapped to an appropriate class")));
	    } catch (Exception ee) {
		tw.displayErrorDialog(null, ee);
	    } finally {
		tool.modified = false;
	    }

	    // display the policy entries via the policy list textarea
	    list = new List(40, false);
	    list.addActionListener(new PolicyListListener(tool, tw));
	    PolicyEntry entries[] = tool.getEntry();
	    if (entries != null) {
		for (int i = 0; i < entries.length; i++)
		    list.add(entries[i].headerToString());
	    }
	    tw.replacePolicyList(list);

	    // display the new policy filename and keystore
	    newFilename = (TextField)tw.getComponent(tw.MW_FILENAME_TEXTFIELD);
	    newFilename.setText(filename);
	    ksName = (TextField)tw.getComponent(tw.MW_KEYSTORE_TEXTFIELD);
	    if (tool.getKeyStoreType() != null &&
		tool.getKeyStoreType().length() > 0)
		ksName.setText(tool.getKeyStoreName() +
				", "
				+ tool.getKeyStoreType());
	    else
		ksName.setText(tool.getKeyStoreName());
	    tw.setVisible(true);

	    // inform user of warnings
	    if (tool.newWarning == true)
		tw.displayStatusDialog(null, PolicyTool.rb.getString
			("Errors have occurred while opening the policy configuration.  View the Warning Log for more information."));
	    break;
	}
    }

    /*
     * return a Menu list of actions for a given permission
     */
    void setPermissionActions(String permission, Choice actions,
				TextField field) {

	actions.removeAll();
	actions.add(PERM_ACTIONS);
	field.setEditable(true);

	if (permission.equals(FILE_PERM_CLASS)) {
	    actions.add(FILE_PERM_READ);
	    actions.add(FILE_PERM_WRITE);
	    actions.add(FILE_PERM_DELETE);
	    actions.add(FILE_PERM_EXECUTE);
	    actions.add(FILE_PERM_ALL);
	} else if (permission.equals(PRIVCRED_PERM_CLASS)) {
	    actions.add(PRIVCRED_PERM_READ);
	} else if (permission.equals(PROP_PERM_CLASS)) {
	    actions.add(PROP_PERM_READ);
	    actions.add(PROP_PERM_WRITE);
	    actions.add(PROP_PERM_ALL);
	} else if (permission.equals(SERVICE_PERM_CLASS)) {
	    actions.add(SERVICE_PERM_INIT);
	    actions.add(SERVICE_PERM_ACCEPT);
	    actions.add(SERVICE_PERM_ALL);
	} else if (permission.equals(SOCK_PERM_CLASS)) {
	    actions.add(SOCKET_PERM_LISTEN);
	    actions.add(SOCKET_PERM_CONNECT);
	    actions.add(SOCKET_PERM_ACCEPT);
	    actions.add(SOCKET_PERM_RESOLVE);
	    actions.add(SOCKET_PERM_ALL);
	} else {
	    field.setEditable(false);
	}
    }

    /*
     * return a Menu list of names for a given permission
     */
    void setPermissionNames(String permission, Choice names, TextField field) {

	names.removeAll();
	names.add(PERM_NAME);
	field.setEditable(true);

	String targets[] = null;
	if (permission.equals(ALL_PERM_CLASS)) {
	    field.setEditable(false);
	} else if (permission.equals(AUDIO_PERM_CLASS)) {
	    targets = AUDIO_TARGETS;
	} else if (permission.equals(AUTH_PERM_CLASS)) {
	    targets = AUTH_TARGETS;
	} else if (permission.equals(AWT_PERM_CLASS)) {
	    targets = AWT_TARGETS;
	} else if (permission.equals(DELEGATION_PERM_CLASS)) {
	    // should be pairs of PrincipalClass names which is too 
	    // complicated to handle in current GUI
	} else if (permission.equals(FILE_PERM_CLASS)) {
	    targets = FILE_TARGETS;
	} else if (permission.equals(LOG_PERM_CLASS)) {
	    targets = LOG_TARGETS;
	} else if (permission.equals(NET_PERM_CLASS)) {
	    targets = NET_TARGETS;
	} else if (permission.equals(PRIVCRED_PERM_CLASS)) {
	    // should be pairs of PrincipalClass/PrincipalName which is too
	    // complicated to handle in current GUI.
	} else if (permission.equals(PROP_PERM_CLASS)) {
	} else if (permission.equals(REFLECT_PERM_CLASS)) {
	    targets = REF_TARGETS;
	} else if (permission.equals(RUNTIME_PERM_CLASS)) {
	    targets = RUN_TARGETS;
	} else if (permission.equals(SECURITY_PERM_CLASS)) {
	    targets = SEC_TARGETS;
	} else if (permission.equals(SERIAL_PERM_CLASS)) {
	    targets = SER_TARGETS;
	} else if (permission.equals(SERVICE_PERM_CLASS)) {
	} else if (permission.equals(SOCK_PERM_CLASS)) {
	} else if (permission.equals(SQL_PERM_CLASS)) {
	    targets = SQL_TARGETS;
	} else if (permission.equals(SSL_PERM_CLASS)) {
	    targets = SSL_TARGETS;
	}
	if (targets != null) {
	    for (int i=0; i<targets.length; i++) {
		names.add(targets[i]);
	    }
	}
    }

    /*
     * Initially set the Principal pulldown menu
     */
    void setPrincipals(String principal, Choice choice) {
	if (principal.equals(PolicyParser.PrincipalEntry.WILDCARD_CLASS)) {
	    choice.select(PRIN_TYPE);
	} else if (principal.equals(KERBEROS_PRIN_CLASS)) {
	    choice.select(KERBEROS_PRIN);
	} else if (principal.equals(NTDOM_PRIN_CLASS)) {
	    choice.select(NTDOM_PRIN);
	} else if (principal.equals(NTSIDDOM_PRIN_CLASS)) {
	    choice.select(NTSIDDOM_PRIN);
	} else if (principal.equals(NTSIDGRP_PRIN_CLASS)) {
	    choice.select(NTSIDGRP_PRIN);
	} else if (principal.equals(NTSIDPRIMGRP_PRIN_CLASS)) {
	    choice.select(NTSIDPRIMGRP_PRIN);
	} else if (principal.equals(NTSIDUSER_PRIN_CLASS)) {
	    choice.select(NTSIDUSER_PRIN);
	} else if (principal.equals(NTUSER_PRIN_CLASS)) {
	    choice.select(NTUSER_PRIN);
	} else if (principal.equals(UNIX_PRIN_CLASS)) {
	    choice.select(UNIX_PRIN);
	} else if (principal.equals(UNIXNUMGRP_PRIN_CLASS)) {
	    choice.select(UNIXNUMGRP_PRIN);
	} else if (principal.equals(UNIXNUMUSER_PRIN_CLASS)) {
	    choice.select(UNIXNUMUSER_PRIN);
	} else if (principal.equals(X500_PRIN_CLASS)) {
	    choice.select(X500_PRIN);
	}
    }

    /*
     * Initially set the Permission pulldown menu
     */
    void setPermissions(String permission, Choice choice) {
	if (permission.equals(ALL_PERM_CLASS)) {
	    choice.select(ALL_PERM);
	} else if (permission.equals(AUDIO_PERM_CLASS)) {
	    choice.select(AUDIO_PERM);
	} else if (permission.equals(AUTH_PERM_CLASS)) {
	    choice.select(AUTH_PERM);
	} else if (permission.equals(AWT_PERM_CLASS)) {
	    choice.select(AWT_PERM);
	} else if (permission.equals(DELEGATION_PERM_CLASS)) {
	    choice.select(DELEGATION_PERM);
	} else if (permission.equals(FILE_PERM_CLASS)) {
	    choice.select(FILE_PERM);
	} else if (permission.equals(LOG_PERM_CLASS)) {
	    choice.select(LOG_PERM);
	} else if (permission.equals(NET_PERM_CLASS)) {
	    choice.select(NET_PERM);
	} else if (permission.equals(PRIVCRED_PERM_CLASS)) {
	    choice.select(PRIVCRED_PERM);
	} else if (permission.equals(PROP_PERM_CLASS)) {
	    choice.select(PROPERTY_PERM);
	} else if (permission.equals(REFLECT_PERM_CLASS)) {
	    choice.select(REFLECT_PERM);
	} else if (permission.equals(RUNTIME_PERM_CLASS)) {
	    choice.select(RUNTIME_PERM);
	} else if (permission.equals(SECURITY_PERM_CLASS)) {
	    choice.select(SECURITY_PERM);
	} else if (permission.equals(SERIAL_PERM_CLASS)) {
	    choice.select(SERIAL_PERM);
	} else if (permission.equals(SERVICE_PERM_CLASS)) {
	    choice.select(SERVICE_PERM);
	} else if (permission.equals(SOCK_PERM_CLASS)) {
	    choice.select(SOCKET_PERM);
	} else if (permission.equals(SQL_PERM_CLASS)) {
	    choice.select(SQL_PERM);
	} else if (permission.equals(SSL_PERM_CLASS)) {
	    choice.select(SSL_PERM);
	}
    }

    /*
     * see if there are any single backslashes (check only on Windows)
     */
    String addSingleBackSlash(String filename) {

	String newFile = new String();
	int length = filename.length();
	int index = 0;
	char nextChar[] = new char[1];

	while (index < length) {
	    nextChar[0] = filename.charAt(index);
	    newFile = newFile.concat(new String(nextChar));
	    if (nextChar[0] == '\\') {
		newFile = newFile.concat(new String(nextChar));
		if (filename.charAt(index + 1) == '\\')
		    index++;
	    }
	    index++;
	}
	return newFile;
    }

    /*
     * parse a principal string (in the Principal Listing)
     * into its principal class, and its name
     * format: "Principal <principal class> <principal name>
     */
    private String parsePrincipalString(String prin, int select) {

        int index1 = prin.indexOf(" ");
	int index2 = prin.indexOf(" ", index1 + 1);
	String result = null;

        switch (select) {
        case PRINCIPAL_TYPE:
            result = prin.substring(index1+1, index2);
	    if (result.equals("*")) {
		return PolicyParser.PrincipalEntry.WILDCARD_CLASS;
	    }
	    break;
        case PRINCIPAL_NAME:
            result = prin.substring(index2).trim();
            // name
            if (result.indexOf("\"") != -1) {
                return result.substring(result.indexOf("\"") + 1,
                        result.indexOf("\"", result.indexOf("\"") + 1));
            }
            else if (result.equals("*")) {
                return PolicyParser.PrincipalEntry.WILDCARD_NAME;
            }
            break;
        default:
	    //assert(false, select);
            break;
        }
	return result;
    }

    /*
     * parse a permission string (in the Permission Listing)
     * into its permission, its name, and its actions
     *
     * XXX should be a StringTokenizer, huh...
     *     because of this implementation, we have to make a special
     *     case for the java.security.ALLPermission.
     */
    private String parsePermissionString(String perm, int select) {

	perm = perm.substring(perm.indexOf(" ") + 1);

        switch (select) {
        case PERMISSION:

		if (perm.indexOf(ALL_PERM_CLASS) != -1) {
		    return new String(ALL_PERM_CLASS);
		}

                // permission
                if (perm.indexOf("\"") == -1)
                    return perm.substring(0, perm.indexOf(";"));
                return perm.substring(0, perm.indexOf("\"") - 1);
        case PERMISSION_NAME:

		if (perm.indexOf(ALL_PERM_CLASS) != -1) {
		    return null;
		}

                // name
                if (perm.indexOf("\"") == -1)
                    return null;
                else
                    return perm.substring
                        (perm.indexOf("\"") + 1,
                        perm.indexOf("\"", perm.indexOf("\"") + 1));
        case PERMISSION_ACTIONS:

		if (perm.indexOf(ALL_PERM_CLASS) != -1) {
		    return null;
		}

                // actions
                if (perm.indexOf("\"") == -1 || perm.indexOf("\", \"") == -1) {
                    return null;
                } else {
		    String tmpString = perm.substring
                        (perm.indexOf("\", \"") + 4, perm.indexOf(";") - 1);
		    if (tmpString.indexOf("signedBy") != -1)
			return tmpString.substring
			    (0, tmpString.indexOf("signedBy") - 3);
		    else
			return tmpString;
		}
	case PERMISSION_SIGNEDBY:
		// signedBy
		if (perm.indexOf("signedBy") == -1)
		    return null;
		else
		    return perm.substring
			(perm.indexOf("signedBy") + 10, perm.indexOf(";") - 1);
        }
        return null;
    }
}

/**
 * Event handler for the PolicyTool window
 */
class ToolWindowListener implements WindowListener {

    private ToolWindow tw;

    ToolWindowListener(ToolWindow tw) {
	this.tw = tw;
    }

    public void windowOpened(WindowEvent we) {
    }

    public void windowClosing(WindowEvent we) {

	// XXX
	// should we ask user if they want to save changes?
	// (we do if they choose the Menu->Exit)
	// seems that if they kill the application by hand,
	// we don't have to ask.

	tw.setVisible(false);
	tw.dispose();
	System.exit(0);
    }

    public void windowClosed(WindowEvent we) {
	System.exit(0);
    }

    public void windowIconified(WindowEvent we) {
    }

    public void windowDeiconified(WindowEvent we) {
    }

    public void windowActivated(WindowEvent we) {
    }

    public void windowDeactivated(WindowEvent we) {
    }
}

/**
 * Event handler for the Policy List
 */
class PolicyListListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;

    PolicyListListener(PolicyTool tool, ToolWindow tw) {
	this.tool = tool;
	this.tw = tw;

    }

    public void actionPerformed(ActionEvent e) {

	// display the permission list for a policy entry
	ToolDialog td = new ToolDialog
		(PolicyTool.rb.getString("Policy Entry"), tool, tw, true);
	td.displayPolicyEntryDialog(true);
    }
}

/**
 * Event handler for the File Menu
 */
class FileMenuListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;

    FileMenuListener(PolicyTool tool, ToolWindow tw) {
	this.tool = tool;
	this.tw = tw;
    }

    public void actionPerformed(ActionEvent e) {

	if (PolicyTool.collator.compare(e.getActionCommand(), tw.QUIT) == 0) {

	    // ask user if they want to save changes
	    ToolDialog td = new ToolDialog
		(PolicyTool.rb.getString("Save Changes"), tool, tw, true);
	    td.displayUserSave(td.QUIT);

	    // the above method will perform the QUIT as long as the
	    // user does not CANCEL the request

	} else if (PolicyTool.collator.compare(e.getActionCommand(),
					tw.NEW_POLICY_FILE) == 0) {

	    // ask user if they want to save changes
	    ToolDialog td = new ToolDialog
		(PolicyTool.rb.getString("Save Changes"), tool, tw, true);
	    td.displayUserSave(td.NEW);

	    // the above method will perform the NEW as long as the
	    // user does not CANCEL the request

	} else if (PolicyTool.collator.compare(e.getActionCommand(),
					tw.OPEN_POLICY_FILE) == 0) {

	    // ask user if they want to save changes
	    ToolDialog td = new ToolDialog
		(PolicyTool.rb.getString("Save Changes"), tool, tw, true);
	    td.displayUserSave(td.OPEN);

	    // the above method will perform the OPEN as long as the
	    // user does not CANCEL the request

	} else if (PolicyTool.collator.compare(e.getActionCommand(),
					tw.SAVE_POLICY_FILE) == 0) {

	    // get the previously entered filename
	    String filename = ((TextField)
		    tw.getComponent(tw.MW_FILENAME_TEXTFIELD)).getText();

	    // if there is no filename, do a SAVE_AS
	    if (filename == null || filename.length() == 0) {
		// user wants to SAVE AS
		ToolDialog td = new ToolDialog
			(PolicyTool.rb.getString("Save As"), tool, tw, true);
		td.displaySaveAsDialog(td.NOACTION);
	    } else {
		try {
		    // save the policy entries to a file
		    tool.savePolicy(filename);

		    // display status
		    MessageFormat form = new MessageFormat
			(PolicyTool.rb.getString
			("Policy successfully written to filename"));
		    Object[] source = {filename};
		    tw.displayStatusDialog(null, form.format(source));
		} catch (FileNotFoundException fnfe) {
		    if (filename == null || filename.equals("")) {
			tw.displayErrorDialog(null, new FileNotFoundException
				(PolicyTool.rb.getString("null filename")));
		    } else {
			MessageFormat form = new MessageFormat
				(PolicyTool.rb.getString("filename not found"));
			Object[] source = {filename};
			tw.displayErrorDialog(null, new FileNotFoundException
					(form.format(source)));
		    }
		} catch (Exception ee) {
		    tw.displayErrorDialog(null, ee);
		}
	    }
	} else if (PolicyTool.collator.compare(e.getActionCommand(),
						tw.SAVE_AS_POLICY_FILE) == 0) {

	    // user wants to SAVE AS
	    ToolDialog td = new ToolDialog
		(PolicyTool.rb.getString("Save As"), tool, tw, true);
	    td.displaySaveAsDialog(td.NOACTION);

	} else if (PolicyTool.collator.compare(e.getActionCommand(),
						tw.VIEW_WARNINGS) == 0) {
	    tw.displayWarningLog(null);
	}
    }
}

/**
 * Event handler for the main window buttons and Edit Menu
 */
class MainWindowListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;

    MainWindowListener(PolicyTool tool, ToolWindow tw) {
	this.tool = tool;
	this.tw = tw;
    }

    public void actionPerformed(ActionEvent e) {

	if (PolicyTool.collator.compare(e.getActionCommand(),
					tw.ADD_POLICY_ENTRY) == 0) {

	    // display a dialog box for the user to enter policy info
	    ToolDialog td = new ToolDialog
		(PolicyTool.rb.getString("Policy Entry"), tool, tw, true);
	    td.displayPolicyEntryDialog(false);

	} else if (PolicyTool.collator.compare(e.getActionCommand(),
					tw.REMOVE_POLICY_ENTRY) == 0) {

	    // get the selected entry
	    List list = (List)tw.getComponent(tw.MW_POLICY_LIST);
	    int index = list.getSelectedIndex();
	    if (index < 0) {
		tw.displayErrorDialog(null, new Exception
			(PolicyTool.rb.getString("No Policy Entry selected")));
		return;
	    }

	    // ask the user if they really want to remove the policy entry
	    ToolDialog td = new ToolDialog(PolicyTool.rb.getString
		("Remove Policy Entry"), tool, tw, true);
	    td.displayConfirmRemovePolicyEntry();

	} else if (PolicyTool.collator.compare(e.getActionCommand(),
					tw.EDIT_POLICY_ENTRY) == 0) {

	    // get the selected entry
	    List list = (List)tw.getComponent(tw.MW_POLICY_LIST);
	    int index = list.getSelectedIndex();
	    if (index < 0) {
		tw.displayErrorDialog(null, new Exception
			(PolicyTool.rb.getString("No Policy Entry selected")));
		return;
	    }

	    // display the permission list for a policy entry
	    ToolDialog td = new ToolDialog
		(PolicyTool.rb.getString("Policy Entry"), tool, tw, true);
	    td.displayPolicyEntryDialog(true);

	} else if (PolicyTool.collator.compare(e.getActionCommand(),
					tw.CHANGE_KEYSTORE) == 0) {

	    // display a dialog box for the user to enter keystore info
	    ToolDialog td = new ToolDialog
		(PolicyTool.rb.getString("Keystore"), tool, tw, true);
	    td.keyStoreDialog(td.CHANGE_KEYSTORE);

	}
    }
}

/**
 * Event handler for OverWriteFileOKButton button
 */
class OverWriteFileOKButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog td;
    private String filename;
    private int nextEvent;

    OverWriteFileOKButtonListener(PolicyTool tool, ToolWindow tw,
				ToolDialog td, String filename, int nextEvent) {
	this.tool = tool;
	this.tw = tw;
	this.td = td;
	this.filename = filename;
	this.nextEvent = nextEvent;
    }

    public void actionPerformed(ActionEvent e) {
	try {
	    // save the policy entries to a file
	    tool.savePolicy(filename);

	    // display status
	    MessageFormat form = new MessageFormat
		(PolicyTool.rb.getString
		("Policy successfully written to filename"));
	    Object[] source = {filename};
	    tw.displayStatusDialog(null, form.format(source));

	    // display the new policy filename
	    TextField newFilename = (TextField)tw.getComponent
				(tw.MW_FILENAME_TEXTFIELD);
	    newFilename.setText(filename);
	    tw.setVisible(true);

	    // now continue with the originally requested command
	    // (QUIT, NEW, or OPEN)
	    td.setVisible(false);
	    td.dispose();
	    td.userSaveContinue(tool, tw, td, nextEvent);

	} catch (FileNotFoundException fnfe) {
	    if (filename == null || filename.equals("")) {
		tw.displayErrorDialog(null, new FileNotFoundException
				(PolicyTool.rb.getString("null filename")));
	    } else {
		MessageFormat form = new MessageFormat
			(PolicyTool.rb.getString("filename not found"));
		Object[] source = {filename};
		tw.displayErrorDialog(null, new FileNotFoundException
					(form.format(source)));
	    }
	    td.setVisible(false);
	    td.dispose();
	} catch (Exception ee) {
	    tw.displayErrorDialog(null, ee);
	    td.setVisible(false);
	    td.dispose();
	}
    }
}

/**
 * Event handler for AddEntryDoneButton button
 *
 * -- if edit is TRUE, then we are EDITing an existing PolicyEntry
 *    and we need to update both the policy and the GUI listing.
 *    if edit is FALSE, then we are ADDing a new PolicyEntry,
 *    so we only need to update the GUI listing.
 */
class AddEntryDoneButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog td;
    private boolean edit;

    AddEntryDoneButtonListener(PolicyTool tool, ToolWindow tw,
				ToolDialog td, boolean edit) {
	this.tool = tool;
	this.tw = tw;
	this.td = td;
	this.edit = edit;
    }

    public void actionPerformed(ActionEvent e) {

	try {
	    // get a PolicyEntry object from the dialog policy info
	    PolicyEntry newEntry = td.getPolicyEntryFromDialog();
	    PolicyParser.GrantEntry newGe = newEntry.getGrantEntry();

	    // see if all the signers have public keys
	    if (newGe.signedBy != null) {
		String signers[] = tool.parseSigners(newGe.signedBy);
		for (int i = 0; i < signers.length; i++) {
		    PublicKey pubKey = tool.getPublicKeyAlias(signers[i]);
		    if (pubKey == null) {
			MessageFormat form = new MessageFormat
				(PolicyTool.rb.getString
				("Warning: A public key for alias 'signers[i]' does not exist."));
			Object[] source = {signers[i]};
			tool.warnings.addElement(form.format(source));
			tw.displayStatusDialog(td, form.format(source));
		    }
		}
	    }

	    // add the entry
	    List policyList = (List)tw.getComponent(tw.MW_POLICY_LIST);
	    if (edit) {
		int listIndex = policyList.getSelectedIndex();
		tool.addEntry(newEntry, listIndex);
		String newCodeBaseStr = newEntry.headerToString();
		if (PolicyTool.collator.compare
			(newCodeBaseStr, policyList.getItem(listIndex)) != 0)
		    tool.modified = true;
		policyList.replaceItem(newCodeBaseStr, listIndex);
	    } else {
		tool.addEntry(newEntry, -1);
		policyList.add(newEntry.headerToString());
		tool.modified = true;
	    }
	    td.setVisible(false);
	    td.dispose();

	} catch (Exception eee) {
	    tw.displayErrorDialog(td, eee);
	}
    }
}

/**
 * Event handler for ChangeKeyStoreOKButton button
 */
class ChangeKeyStoreOKButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog td;

    ChangeKeyStoreOKButtonListener(PolicyTool tool, ToolWindow tw,
					ToolDialog td) {
	this.tool = tool;
	this.tw = tw;
	this.td = td;
    }

    public void actionPerformed(ActionEvent e) {

	String URLString = ((TextField)
		td.getComponent(td.KSD_NAME_TEXTFIELD)).getText().trim();
	String type = ((TextField)
		td.getComponent(td.KSD_TYPE_TEXTFIELD)).getText().trim();

	if ((URLString == null || URLString.equals("")) &&
	    type != null && type.equals("") == false) {
		tw.displayErrorDialog(td, new Exception(PolicyTool.rb.getString
			("KeyStore URL must have a valid value")));
		return;
	}
	tool.setKeyStoreInfo(URLString, type);
	TextField ks = (TextField)tw.getComponent(tw.MW_KEYSTORE_TEXTFIELD);
	if (type != null && type.length() > 0)
	    ks.setText(URLString + ", " + type);
	else
	    ks.setText(URLString);
	td.dispose();
    }
}

/**
 * Event handler for AddPrinButton button
 */
class AddPrinButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog td;
    private boolean editPolicyEntry;

    AddPrinButtonListener(PolicyTool tool, ToolWindow tw,
				ToolDialog td, boolean editPolicyEntry) {
	this.tool = tool;
	this.tw = tw;
	this.td = td;
	this.editPolicyEntry = editPolicyEntry;
    }

    public void actionPerformed(ActionEvent e) {

	// display a dialog box for the user to enter principal info
	td.displayPrincipalDialog(editPolicyEntry, false);
    }
}

/**
 * Event handler for AddPermButton button
 */
class AddPermButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog td;
    private boolean editPolicyEntry;

    AddPermButtonListener(PolicyTool tool, ToolWindow tw,
				ToolDialog td, boolean editPolicyEntry) {
	this.tool = tool;
	this.tw = tw;
	this.td = td;
	this.editPolicyEntry = editPolicyEntry;
    }

    public void actionPerformed(ActionEvent e) {

	// display a dialog box for the user to enter permission info
	td.displayPermissionDialog(editPolicyEntry, false);
    }
}

/**
 * Event handler for AddPrinOKButton button
 */
class NewPolicyPrinOKButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog listDialog;
    private ToolDialog infoDialog;
    private boolean edit;

    NewPolicyPrinOKButtonListener(PolicyTool tool,
				ToolWindow tw,
				ToolDialog listDialog,
				ToolDialog infoDialog,
				boolean edit) {
	this.tool = tool;
	this.tw = tw;
	this.listDialog = listDialog;
	this.infoDialog = infoDialog;
	this.edit = edit;
    }

    public void actionPerformed(ActionEvent e) {

	try {
	    // read in the new principal info from Dialog Box
	    PolicyParser.PrincipalEntry pppe =
			infoDialog.getPrinFromDialog();
	    if (pppe != null) {
	        try {
	            tool.verifyPrincipal(pppe.getPrincipalClass(), pppe.getPrincipalName());
	        } catch (ClassNotFoundException cnfe) {
		    tool.warnings.addElement
			(PolicyTool.rb.getString("Warning: Class not found: ") +
			pppe.getPrincipalClass());
		    tw.displayStatusDialog(infoDialog,
			PolicyTool.rb.getString("Warning: Class not found: ") +
			pppe.getPrincipalClass());
	        }

	        // add the principal to the GUI principal list
	        StringWriter sw = new StringWriter();
	        PrintWriter pw = new PrintWriter(sw);
	        pppe.write(pw);
	        List prinList =
		    (List)listDialog.getComponent(listDialog.PE_PRIN_LIST);

	        String prinString = sw.toString();
	        if (edit) {
		    // if editing, replace the original principal
		    prinList.replaceItem(prinString, prinList.getSelectedIndex());
	        } else {
		    // if adding, just add it to the end
		    prinList.add(prinString);
	        }
	    }
	    infoDialog.dispose();
	} catch (Exception ee) {
	    tw.displayErrorDialog(infoDialog, ee);
	}
    }
}

/**
 * Event handler for AddPermOKButton button
 */
class NewPolicyPermOKButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog listDialog;
    private ToolDialog infoDialog;
    private boolean edit;

    NewPolicyPermOKButtonListener(PolicyTool tool,
				ToolWindow tw,
				ToolDialog listDialog,
				ToolDialog infoDialog,
				boolean edit) {
	this.tool = tool;
	this.tw = tw;
	this.listDialog = listDialog;
	this.infoDialog = infoDialog;
	this.edit = edit;
    }

    public void actionPerformed(ActionEvent e) {

	try {
	    // read in the new permission info from Dialog Box
	    PolicyParser.PermissionEntry pppe =
			infoDialog.getPermFromDialog();

	    /*
	     * XXX	hate to add a special case check here ... but ...
	     *		we need to make sure windows users use
	     *		2 backs slashes in their filenames
	     */
	    if (pppe.permission.equals(listDialog.FILE_PERM_CLASS) &&
		File.separatorChar == '\\')
		pppe.name = listDialog.addSingleBackSlash(pppe.name);

	    try {
		tool.verifyPermission(pppe.permission, pppe.name, pppe.action);
	    } catch (ClassNotFoundException cnfe) {
		tool.warnings.addElement
			(PolicyTool.rb.getString("Warning: Class not found: ") +
			pppe.permission);
		tw.displayStatusDialog(infoDialog,
			PolicyTool.rb.getString("Warning: Class not found: ") +
			pppe.permission);
	    }

	    // add the permission to the GUI permission list
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    pppe.write(pw);
	    List permList =
		(List)listDialog.getComponent(listDialog.PE_PERM_LIST);

	    String permString = sw.toString();
	    if (edit) {
		// if editing, replace the original permission
		permList.replaceItem
			(permString.substring(0, permString.indexOf(";") + 1),
			permList.getSelectedIndex());
	    } else {
		// if adding, just add it to the end
		permList.add
			(permString.substring(0, permString.indexOf(";") + 1));
	    }
	    infoDialog.dispose();

	} catch (InvocationTargetException ite) {
	    tw.displayErrorDialog(infoDialog,
		new Exception
		(PolicyTool.rb.getString("Invalid value for Actions")));
	} catch (Exception ee) {
	    tw.displayErrorDialog(infoDialog, ee);
	}
    }
}

/**
 * Event handler for EditPrinOKButton button
 */
class EditPolicyPrinOKButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog listDialog;
    private ToolDialog infoDialog;
    private boolean edit;

    EditPolicyPrinOKButtonListener(PolicyTool tool,
				ToolWindow tw,
				ToolDialog listDialog,
				ToolDialog infoDialog,
				boolean edit) {
	this.tool = tool;
	this.tw = tw;
	this.listDialog = listDialog;
	this.infoDialog = infoDialog;
	this.edit = edit;
    }

    public void actionPerformed(ActionEvent e) {
	try {
	    // read in the new principal info from Dialog Box
	    PolicyParser.PrincipalEntry pppe =
			infoDialog.getPrinFromDialog();
	    if (pppe != null) {
	        try {
		    tool.verifyPrincipal(pppe.getPrincipalClass(),
		    	pppe.getPrincipalName());
	        } catch (ClassNotFoundException cnfe) {
		    tool.warnings.addElement
			(PolicyTool.rb.getString("Warning: Class not found: ") +
			pppe.getPrincipalClass());
		    tw.displayStatusDialog(infoDialog,
			PolicyTool.rb.getString("Warning: Class not found: ") +
			pppe.getPrincipalClass());
	        }

	        // get the PolicyEntry selected from the main ToolWindow
	        List policyList = (List)tw.getComponent(tw.MW_POLICY_LIST);
	        int index = policyList.getSelectedIndex();
	        PolicyEntry entries[] = tool.getEntry();
	        PolicyEntry entry = entries[index];

	        // add the principal to the policy entry
	        List prinList =
			(List)listDialog.getComponent(listDialog.PE_PRIN_LIST);
	        boolean status = false;
	        if (edit)
		    status = tool.addPrinEntry
			(entry, pppe, prinList.getSelectedIndex());
	        else
		    status = tool.addPrinEntry
			(entry, pppe, -1);
	        if (status == true) {
		    // add the principal to the GUI principal list
		    StringWriter sw = new StringWriter();
		    PrintWriter pw = new PrintWriter(sw);
		    pppe.write(pw);
		    String prinString = sw.toString();
		    if (edit) {
		        prinList.replaceItem
			    (prinString, prinList.getSelectedIndex());
		    } else {
		        prinList.add(prinString);
		    }
	        }
	    }
	    infoDialog.dispose();
	} catch (Exception ee) {
	    tw.displayErrorDialog(infoDialog, ee);
	}
    }
}

/**
 * Event handler for EditPermOKButton button
 */
class EditPolicyPermOKButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog listDialog;
    private ToolDialog infoDialog;
    private boolean edit;

    EditPolicyPermOKButtonListener(PolicyTool tool,
				ToolWindow tw,
				ToolDialog listDialog,
				ToolDialog infoDialog,
				boolean edit) {
	this.tool = tool;
	this.tw = tw;
	this.listDialog = listDialog;
	this.infoDialog = infoDialog;
	this.edit = edit;
    }

    public void actionPerformed(ActionEvent e) {
	try {
	    // read in the new permission info from Dialog Box
	    PolicyParser.PermissionEntry pppe =
			infoDialog.getPermFromDialog();

	    /*
	     * XXX      hate to add a special case check here ... but ...
	     *		we need to make sure windows users use
	     *		2 backs slashes in their filenames
	     */
	    if (pppe.permission.equals(listDialog.FILE_PERM_CLASS) &&
		File.separatorChar == '\\')
		pppe.name = listDialog.addSingleBackSlash(pppe.name);

	    try {
		tool.verifyPermission(pppe.permission, pppe.name, pppe.action);
	    } catch (ClassNotFoundException cnfe) {
		tool.warnings.addElement
			(PolicyTool.rb.getString("Warning: Class not found: ") +
			pppe.permission);
		tw.displayStatusDialog(infoDialog,
			PolicyTool.rb.getString("Warning: Class not found: ") +
			pppe.permission);
	    }

	    // get the PolicyEntry selected from the main ToolWindow
	    List policyList = (List)tw.getComponent(tw.MW_POLICY_LIST);
	    int index = policyList.getSelectedIndex();
	    PolicyEntry entries[] = tool.getEntry();
	    PolicyEntry entry = entries[index];

	    // add the permission to the policy entry
	    List permList =
			(List)listDialog.getComponent(listDialog.PE_PERM_LIST);
	    boolean status = false;
	    if (edit)
		status = tool.addPermEntry
			(entry, pppe, permList.getSelectedIndex());
	    else
		status = tool.addPermEntry
			(entry, pppe, -1);
	    if (status == true) {

		// add the permission to the GUI permission list
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		pppe.write(pw);
		String permString = sw.toString();
		if (edit) {
		    permList.replaceItem
			(permString.substring(0, permString.indexOf(";") + 1),
			permList.getSelectedIndex());
		} else {
		    permList.add
			(permString.substring(0, permString.indexOf(";") + 1));
		}
	    }
	    infoDialog.dispose();

	} catch (InvocationTargetException ite) {
	    tw.displayErrorDialog(infoDialog,
		new Exception
		(PolicyTool.rb.getString("Invalid value for Actions")));
	} catch (Exception ee) {
	    tw.displayErrorDialog(infoDialog, ee);
	}
    }
}

/**
 * Event handler for RemovePrinButton button
 */
class RemovePrinButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog td;
    private boolean edit;

    RemovePrinButtonListener(PolicyTool tool, ToolWindow tw,
				ToolDialog td, boolean edit) {
	this.tool = tool;
	this.tw = tw;
	this.td = td;
	this.edit = edit;
    }

    public void actionPerformed(ActionEvent e) {

	// get the Principal selected from the Principal List
	List prinList = (List)td.getComponent(td.PE_PRIN_LIST);
	int prinIndex = prinList.getSelectedIndex();

	if (prinIndex < 0) {
	    tw.displayErrorDialog(td, new Exception
		(PolicyTool.rb.getString("No principal selected")));
	    return;
	}
	// remove the principal from the display
	prinList.remove(prinIndex);
    }
}

/**
 * Event handler for RemovePermButton button
 */
class RemovePermButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog td;
    private boolean edit;

    RemovePermButtonListener(PolicyTool tool, ToolWindow tw,
				ToolDialog td, boolean edit) {
	this.tool = tool;
	this.tw = tw;
	this.td = td;
	this.edit = edit;
    }

    public void actionPerformed(ActionEvent e) {

	// get the Permission selected from the Permission List
	List permList = (List)td.getComponent(td.PE_PERM_LIST);
	int permIndex = permList.getSelectedIndex();

	if (permIndex < 0) {
	    tw.displayErrorDialog(td, new Exception
		(PolicyTool.rb.getString("No permission selected")));
	    return;
	}
	// remove the permission from the display
	permList.remove(permIndex);

    }
}

/**
 * Event handler for Edit Principal button
 *
 * We need the editPolicyEntry boolean to tell us if the user is
 * adding a new PolicyEntry at this time, or editing an existing entry.
 * If the user is adding a new PolicyEntry, we ONLY update the
 * GUI listing.  If the user is editing an existing PolicyEntry, we
 * update both the GUI listing and the actual PolicyEntry.
 */
class EditPrinButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog td;
    private boolean editPolicyEntry;

    EditPrinButtonListener(PolicyTool tool, ToolWindow tw,
				ToolDialog td, boolean editPolicyEntry) {
	this.tool = tool;
	this.tw = tw;
	this.td = td;
	this.editPolicyEntry = editPolicyEntry;
    }

    public void actionPerformed(ActionEvent e) {

	// get the Principal selected from the Principal List
	List list = (List)td.getComponent(td.PE_PRIN_LIST);
	int prinIndex = list.getSelectedIndex();

	if (prinIndex < 0) {
	    tw.displayErrorDialog(td, new Exception
		(PolicyTool.rb.getString("No principal selected")));
	    return;
	}
	td.displayPrincipalDialog(editPolicyEntry, true);
    }
}

/**
 * Event handler for Edit Permission button
 *
 * We need the editPolicyEntry boolean to tell us if the user is
 * adding a new PolicyEntry at this time, or editing an existing entry.
 * If the user is adding a new PolicyEntry, we ONLY update the
 * GUI listing.  If the user is editing an existing PolicyEntry, we
 * update both the GUI listing and the actual PolicyEntry.
 */
class EditPermButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog td;
    private boolean editPolicyEntry;

    EditPermButtonListener(PolicyTool tool, ToolWindow tw,
				ToolDialog td, boolean editPolicyEntry) {
	this.tool = tool;
	this.tw = tw;
	this.td = td;
	this.editPolicyEntry = editPolicyEntry;
    }

    public void actionPerformed(ActionEvent e) {

	// get the Permission selected from the Permission List
	List list = (List)td.getComponent(td.PE_PERM_LIST);
	int permIndex = list.getSelectedIndex();

	if (permIndex < 0) {
	    tw.displayErrorDialog(td, new Exception
		(PolicyTool.rb.getString("No permission selected")));
	    return;
	}
	td.displayPermissionDialog(editPolicyEntry, true);
    }
}

/**
 * Event handler for Principal Popup Menu
 */
class PrincipalTypeMenuListener implements ItemListener {

    private ToolDialog td;

    PrincipalTypeMenuListener(ToolDialog td) {
	this.td = td;
    }

    public void itemStateChanged(ItemEvent e) {

	Choice prin = (Choice)td.getComponent(td.PRD_PRIN_CHOICE);
	TextField prinField =
			(TextField)td.getComponent(td.PRD_PRIN_TEXTFIELD);
	TextField nameField =
			(TextField)td.getComponent(td.PRD_NAME_TEXTFIELD);

	// if you change the principal, clear the name
	if (prinField.getText().indexOf((String)e.getItem()) == -1) {
	    nameField.setText("");
	}

	// set the text in the textfield and also modify the
	// pull-down choice menus to reflect the correct possible
	// set of names and actions
	if (((String)e.getItem()).equals(td.PRIN_TYPE)) {
	    prinField.setText("");
	} else if (((String)e.getItem()).equals(td.KERBEROS_PRIN)) {
	    prinField.setText(td.KERBEROS_PRIN_CLASS);
	} else if (((String)e.getItem()).equals(td.NTDOM_PRIN)) {
	    prinField.setText(td.NTDOM_PRIN_CLASS);
	} else if (((String)e.getItem()).equals(td.NTSIDDOM_PRIN)) {
	    prinField.setText(td.NTSIDDOM_PRIN_CLASS);
	} else if (((String)e.getItem()).equals(td.NTSIDGRP_PRIN)) {
	    prinField.setText(td.NTSIDGRP_PRIN_CLASS);
	} else if (((String)e.getItem()).equals(td.NTSIDPRIMGRP_PRIN)) {
	    prinField.setText(td.NTSIDPRIMGRP_PRIN_CLASS);
	} else if (((String)e.getItem()).equals(td.NTSIDUSER_PRIN)) {
	    prinField.setText(td.NTSIDUSER_PRIN_CLASS);
	} else if (((String)e.getItem()).equals(td.NTUSER_PRIN)) {
	    prinField.setText(td.NTUSER_PRIN_CLASS);
	} else if (((String)e.getItem()).equals(td.UNIX_PRIN)) {
	    prinField.setText(td.UNIX_PRIN_CLASS);
	} else if (((String)e.getItem()).equals(td.UNIXNUMGRP_PRIN)) {
	    prinField.setText(td.UNIXNUMGRP_PRIN_CLASS);
	} else if (((String)e.getItem()).equals(td.UNIXNUMUSER_PRIN)) {
	    prinField.setText(td.UNIXNUMUSER_PRIN_CLASS);
	} else if (((String)e.getItem()).equals(td.X500_PRIN)) {
	    prinField.setText(td.X500_PRIN_CLASS);
	}
    }
}

/**
 * Event handler for Permission Popup Menu
 */
class PermissionMenuListener implements ItemListener {

    private ToolDialog td;

    PermissionMenuListener(ToolDialog td) {
	this.td = td;
    }

    public void itemStateChanged(ItemEvent e) {

	Choice names = (Choice)td.getComponent(td.PD_NAME_CHOICE);
	Choice actions = (Choice)td.getComponent(td.PD_ACTIONS_CHOICE);
	TextField nameField =
			(TextField)td.getComponent(td.PD_NAME_TEXTFIELD);
	TextField actionsField =
			(TextField)td.getComponent(td.PD_ACTIONS_TEXTFIELD);
	TextField permField = (TextField)td.getComponent(td.PD_PERM_TEXTFIELD);
	TextField signedbyField =
			(TextField)td.getComponent(td.PD_SIGNEDBY_TEXTFIELD);

	// ignore if they choose the 'Permission:' item
	if (PolicyTool.collator.compare((String)e.getItem(), td.PERM) == 0) {
	    if (permField.getText() != null &&
		permField.getText().length() > 0) {

		Choice perms = (Choice)td.getComponent(td.PD_PERM_CHOICE);
		td.setPermissions(permField.getText(), perms);
	    }
	    return;
	}

	// if you change the permission, clear the name, actions, and signedBy
	if (permField.getText().indexOf((String)e.getItem()) == -1) {
	    nameField.setText("");
	    actionsField.setText("");
	    signedbyField.setText("");
	}

	// set the text in the textfield and also modify the
	// pull-down choice menus to reflect the correct possible
	// set of names and actions

	if (((String)e.getItem()).equals(td.AWT_PERM)) {
	    permField.setText(td.AWT_PERM_CLASS);
	    td.setPermissionNames(td.AWT_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.AWT_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.FILE_PERM)) {
	    permField.setText(td.FILE_PERM_CLASS);
	    td.setPermissionNames(td.FILE_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.FILE_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.NET_PERM)) {
	    permField.setText(td.NET_PERM_CLASS);
	    td.setPermissionNames(td.NET_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.NET_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.PROPERTY_PERM)) {
	    permField.setText(td.PROP_PERM_CLASS);
	    td.setPermissionNames(td.PROP_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.PROP_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.RUNTIME_PERM)) {
	    permField.setText(td.RUNTIME_PERM_CLASS);
	    td.setPermissionNames(td.RUNTIME_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.RUNTIME_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.SOCKET_PERM)) {
	    permField.setText(td.SOCK_PERM_CLASS);
	    td.setPermissionNames(td.SOCK_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.SOCK_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.REFLECT_PERM)) {
	    permField.setText(td.REFLECT_PERM_CLASS);
	    td.setPermissionNames(td.REFLECT_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.REFLECT_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.SECURITY_PERM)) {
	    permField.setText(td.SECURITY_PERM_CLASS);
	    td.setPermissionNames(td.SECURITY_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.SECURITY_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.SERIAL_PERM)) {
	    permField.setText(td.SERIAL_PERM_CLASS);
	    td.setPermissionNames(td.SERIAL_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.SERIAL_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.ALL_PERM)) {
	    permField.setText(td.ALL_PERM_CLASS);
	    td.setPermissionNames(td.ALL_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.ALL_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.AUTH_PERM)) {
	    permField.setText(td.AUTH_PERM_CLASS);
	    td.setPermissionNames(td.AUTH_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.AUTH_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.PRIVCRED_PERM)) {
	    permField.setText(td.PRIVCRED_PERM_CLASS);
	    td.setPermissionNames(td.PRIVCRED_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.PRIVCRED_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.SERVICE_PERM)) {
	    permField.setText(td.SERVICE_PERM_CLASS);
	    td.setPermissionNames(td.SERVICE_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.SERVICE_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.DELEGATION_PERM)) {
	    permField.setText(td.DELEGATION_PERM_CLASS);
	    td.setPermissionNames(td.DELEGATION_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.DELEGATION_PERM_CLASS, actions, actionsField);
	}  else if (((String)e.getItem()).equals(td.AUDIO_PERM)) {
	    permField.setText(td.AUDIO_PERM_CLASS);
	    td.setPermissionNames(td.AUDIO_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.AUDIO_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.LOG_PERM)) {
	    permField.setText(td.LOG_PERM_CLASS);
	    td.setPermissionNames(td.LOG_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.LOG_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.SQL_PERM)) {
	    permField.setText(td.SQL_PERM_CLASS);
	    td.setPermissionNames(td.SQL_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.SQL_PERM_CLASS, actions, actionsField);
	} else if (((String)e.getItem()).equals(td.SSL_PERM)) {
	    permField.setText(td.SSL_PERM_CLASS);
	    td.setPermissionNames(td.SSL_PERM_CLASS, names, nameField);
	    td.setPermissionActions(td.SSL_PERM_CLASS, actions, actionsField);
	}
    }
}

/**
 * Event handler for Permission Name Popup Menu
 */
class PermissionNameMenuListener implements ItemListener {

    private ToolDialog td;

    PermissionNameMenuListener(ToolDialog td) {
	this.td = td;
    }

    public void itemStateChanged(ItemEvent e) {

	if (((String)e.getItem()).indexOf(td.PERM_NAME) != -1)
	    return;

	TextField tf = (TextField)td.getComponent(td.PD_NAME_TEXTFIELD);
	tf.setText((String)e.getItem());
    }
}

/**
 * Event handler for Permission Actions Popup Menu
 */
class PermissionActionsMenuListener implements ItemListener {

    private ToolDialog td;

    PermissionActionsMenuListener(ToolDialog td) {
	this.td = td;
    }

    public void itemStateChanged(ItemEvent e) {

	if (((String)e.getItem()).indexOf(td.PERM_ACTIONS) != -1)
	    return;

	TextField tf = (TextField)td.getComponent(td.PD_ACTIONS_TEXTFIELD);
	if (tf.getText() == null || tf.getText().equals("")) {
	    tf.setText((String)e.getItem());
	} else {
	    if (tf.getText().indexOf((String)e.getItem()) == -1)
		tf.setText(tf.getText() + ", " + (String)e.getItem());
	}
    }
}

/**
 * Event handler for all the children dialogs/windows
 */
class ChildWindowListener implements WindowListener {

    private ToolDialog td;

    ChildWindowListener(ToolDialog td) {
	this.td = td;
    }

    public void windowOpened(WindowEvent we) {
    }

    public void windowClosing(WindowEvent we) {
	// same as pressing the "cancel" button
	td.setVisible(false);
	td.dispose();
    }

    public void windowClosed(WindowEvent we) {
    }

    public void windowIconified(WindowEvent we) {
    }

    public void windowDeiconified(WindowEvent we) {
    }

    public void windowActivated(WindowEvent we) {
    }

    public void windowDeactivated(WindowEvent we) {
    }
}

/**
 * Event handler for CancelButton button
 */
class CancelButtonListener implements ActionListener {

    private ToolDialog td;

    CancelButtonListener(ToolDialog td) {
	this.td = td;
    }

    public void actionPerformed(ActionEvent e) {
	td.setVisible(false);
	td.dispose();
    }
}

/**
 * Event handler for ErrorOKButton button
 */
class ErrorOKButtonListener implements ActionListener {

    private ToolDialog ed;

    ErrorOKButtonListener(ToolDialog ed) {
	this.ed = ed;
    }

    public void actionPerformed(ActionEvent e) {
	ed.setVisible(false);
	ed.dispose();
    }
}

/**
 * Event handler for StatusOKButton button
 */
class StatusOKButtonListener implements ActionListener {

    private ToolDialog sd;

    StatusOKButtonListener(ToolDialog sd) {
	this.sd = sd;
    }

    public void actionPerformed(ActionEvent e) {
	sd.setVisible(false);
	sd.dispose();
    }
}

/**
 * Event handler for UserSaveYes button
 */
class UserSaveYesButtonListener implements ActionListener {

    private ToolDialog us;
    private PolicyTool tool;
    private ToolWindow tw;
    private int select;

    UserSaveYesButtonListener(ToolDialog us, PolicyTool tool,
			ToolWindow tw, int select) {
	this.us = us;
	this.tool = tool;
	this.tw = tw;
	this.select = select;
    }

    public void actionPerformed(ActionEvent e) {

	// first get rid of the window
	us.setVisible(false);
	us.dispose();

	try {
	    String filename = ((TextField)
		    tw.getComponent(tw.MW_FILENAME_TEXTFIELD)).getText();
	    if (filename == null || filename.equals("")) {
		us.displaySaveAsDialog(select);

		// the above dialog will continue with the originally
		// requested command if necessary
	    } else {
		// save the policy entries to a file
		tool.savePolicy(filename);

		// display status
		MessageFormat form = new MessageFormat
			(PolicyTool.rb.getString
			("Policy successfully written to filename"));
		Object[] source = {filename};
		tw.displayStatusDialog(null, form.format(source));

		// now continue with the originally requested command
		// (QUIT, NEW, or OPEN)
		us.userSaveContinue(tool, tw, us, select);
	    }
	} catch (Exception ee) {
	    // error -- just report it and bail
	    tw.displayErrorDialog(null, ee);
	}
    }
}

/**
 * Event handler for UserSaveNoButton
 */
class UserSaveNoButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog us;
    private int select;

    UserSaveNoButtonListener(ToolDialog us, PolicyTool tool,
			ToolWindow tw, int select) {
	this.us = us;
	this.tool = tool;
	this.tw = tw;
	this.select = select;
    }

    public void actionPerformed(ActionEvent e) {
	us.setVisible(false);
	us.dispose();

	// now continue with the originally requested command
	// (QUIT, NEW, or OPEN)
	us.userSaveContinue(tool, tw, us, select);
    }
}

/**
 * Event handler for UserSaveCancelButton
 */
class UserSaveCancelButtonListener implements ActionListener {

    private ToolDialog us;

    UserSaveCancelButtonListener(ToolDialog us) {
	this.us = us;
    }

    public void actionPerformed(ActionEvent e) {
	us.setVisible(false);
	us.dispose();

	// do NOT continue with the originally requested command
	// (QUIT, NEW, or OPEN)
    }
}

/**
 * Event handler for ConfirmRemovePolicyEntryOKButtonListener
 */
class ConfirmRemovePolicyEntryOKButtonListener implements ActionListener {

    private PolicyTool tool;
    private ToolWindow tw;
    private ToolDialog us;

    ConfirmRemovePolicyEntryOKButtonListener(PolicyTool tool,
				ToolWindow tw, ToolDialog us) {
	this.tool = tool;
	this.tw = tw;
	this.us = us;
    }

    public void actionPerformed(ActionEvent e) {
	// remove the entry
	List list = (List)tw.getComponent(tw.MW_POLICY_LIST);
	int index = list.getSelectedIndex();
	PolicyEntry entries[] = tool.getEntry();
	tool.removeEntry(entries[index]);

	// redraw the window listing
	list = new List(40, false);
	list.addActionListener(new PolicyListListener(tool, tw));
	entries = tool.getEntry();
	if (entries != null) {
		for (int i = 0; i < entries.length; i++)
		    list.add(entries[i].headerToString());
	}
	tw.replacePolicyList(list);
	us.setVisible(false);
	us.dispose();
    }
}
