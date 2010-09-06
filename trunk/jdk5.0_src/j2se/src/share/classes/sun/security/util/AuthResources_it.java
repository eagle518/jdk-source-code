/*
 * @(#)AuthResources_it.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.util;
/**
 * <p> This class represents the <code>ResourceBundle</code>
 * for the following packages:
 *
 * <ol>
 * <li> javax.security.auth
 * <li> javax.security.auth.callback
 * <li> javax.security.auth.login
 * <li> javax.security.auth.spi
 * <li> com.sun.security.auth
 * <li> com.sun.security.auth.login
 * </ol>
 *
 * @version 1.6, 01/23/01
 */
public class AuthResources_it extends java.util.ListResourceBundle {

    private static final Object[][] contents = {

    // NT principals
    {"invalid null input: value", "input nullo non valido: {0}"},
    {"NTDomainPrincipal: name", "NTDomainPrincipal: {0}"},
    {"NTNumericCredential: name", "NTNumericCredential: {0}"},
    {"Invalid NTSid value", "Valore NTSid non valido"},
    {"NTSid: name", "NTSid: {0}"},
    {"NTSidDomainPrincipal: name", "NTSidDomainPrincipal: {0}"},
    {"NTSidGroupPrincipal: name", "NTSidGroupPrincipal: {0}"},
    {"NTSidPrimaryGroupPrincipal: name", "NTSidPrimaryGroupPrincipal: {0}"},
    {"NTSidUserPrincipal: name", "NTSidUserPrincipal: {0}"},
    {"NTUserPrincipal: name", "NTUserPrincipal: {0}"},

    // UnixPrincipals
    {"UnixNumericGroupPrincipal [Primary Group]: name",
        "UnixNumericGroupPrincipal [gruppo primario]: {0}"},
    {"UnixNumericGroupPrincipal [Supplementary Group]: name",
        "UnixNumericGroupPrincipal [gruppo supplementare]: {0}"},
    {"UnixNumericUserPrincipal: name", "UnixNumericUserPrincipal: {0}"},
    {"UnixPrincipal: name", "UnixPrincipal: {0}"},

    // com.sun.security.auth.login.ConfigFile
    {"Unable to properly expand config", "Impossibile espandere correttamente {0}"},
    {"extra_config (No such file or directory)",
        "{0} (file o directory inesistente)"},
    {"Unable to locate a login configuration",
        "Impossibile trovare una configurazione di login"},
    {"Configuration Error:\n\tInvalid control flag, flag",
        "Errore di configurazione:\n\tflag di controllo non valido, {0}"},
    {"Configuration Error:\n\tCan not specify multiple entries for appName",
        "Errore di configurazione:\n\timpossibile specificare pi\u00f9 valori per {0}"},
    {"Configuration Error:\n\texpected [expect], read [end of file]",
        "Errore di configurazione:\n\tprevisto [{0}], letto [end of file]"},
    {"Configuration Error:\n\tLine line: expected [expect], found [value]",
        "Errore di configurazione:\n\triga {0}: previsto [{1}], trovato [{2}]"},
    {"Configuration Error:\n\tLine line: expected [expect]",
        "Errore di configurazione:\n\triga {0}: previsto [{1}]"},
    {"Configuration Error:\n\tLine line: system property [value] expanded to empty value",
        "Errore di configurazione:\n\triga {0}: propriet\u00e0 di sistema [{1}] espansa a valore vuoto"},

    // com.sun.security.auth.module.JndiLoginModule
    {"username: ","Nome utente: "},
    {"password: ","Password: "},

    // com.sun.security.auth.module.KeyStoreLoginModule
	{"Please enter keystore information",
		"Inserire le informazioni per il keystore"},
    {"Keystore alias: ","Alias keystore: "},
    {"Keystore password: ","Password keystore: "},
    {"Private key password (optional): ",
        "Password chiave privata (opzionale): "},

    // com.sun.security.auth.module.Krb5LoginModule
    {"Kerberos username [[defUsername]]: ",
	        "Nome utente Kerberos [{0}]: "},
	    {"Kerberos password for [username]: ",
	            "Password Kerberos per {0}: "},

    /***    EVERYTHING BELOW IS DEPRECATED  ***/

    // com.sun.security.auth.PolicyFile
    {": error parsing ", ": errore nell'analisi "},
    {": ", ": "},
    {": error adding Permission ", ": errore nell'aggiunta del permesso "},
    {" ", " "},
    {": error adding Entry ", ": errore nell'aggiunta dell'entry "},
    {"(", "("},
    {")", ")"},
    {"attempt to add a Permission to a readonly PermissionCollection",
        "tentativo di aggiungere un permesso a una PermissionCollection di sola lettura"},

    // com.sun.security.auth.PolicyParser
    {"expected keystore type", "tipo di keystore previsto"},
    {"can not specify Principal with a ",
        "impossibile specificare Principal con una "},
    {"wildcard class without a wildcard name",
        "classe wildcard senza un nome wildcard"},
    {"expected codeBase or SignedBy", "previsto codeBase o SignedBy"},
    {"only Principal-based grant entries permitted",
        "sono permessi solo valori garantiti basati su Principal"},
    {"expected permission entry", "prevista entry di permesso"},
    {"number ", "numero "},
    {"expected ", "previsto "},
    {", read end of file", ", letto end of file"},
    {"expected ';', read end of file", "previsto ';', letto end of file"},
    {"line ", "riga "},
    {": expected '", ": previsto '"},
    {"', found '", "', trovato '"},
    {"'", "'"},

    // SolarisPrincipals
    {"SolarisNumericGroupPrincipal [Primary Group]: ",
        "SolarisNumericGroupPrincipal [gruppo primario]: "},
    {"SolarisNumericGroupPrincipal [Supplementary Group]: ",
        "SolarisNumericGroupPrincipal [gruppo supplementare]: "},
    {"SolarisNumericUserPrincipal: ",
        "SolarisNumericUserPrincipal: "},
    {"SolarisPrincipal: ", "SolarisPrincipal: "},
    {"provided null name", "il nome fornito \u00e8 nullo"}

    };

    /**
     * Returns the contents of this <code>ResourceBundle</code>.
     *
     * <p>
     *
     * @return the contents of this <code>ResourceBundle</code>.
     */
    public Object[][] getContents() {
    return contents;
    }
}

