/*
 * "@(#)AuthResources_sv.java	1.11 03/12/19
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
public class AuthResources_sv extends java.util.ListResourceBundle {

    private static final Object[][] contents = {

    // NT principals
    {"invalid null input: value", "ogiltiga null-indata: {0}"},
    {"NTDomainPrincipal: name", "NTDomainPrincipal: {0}"},
    {"NTNumericCredential: name", "NTNumericCredential: {0}"},
    {"Invalid NTSid value", "Ogiltigt NTSid-v\u00e4rde"},
    {"NTSid: name", "NTSid: {0}"},
    {"NTSidDomainPrincipal: name", "NTSidDomainPrincipal: {0}"},
    {"NTSidGroupPrincipal: name", "NTSidGroupPrincipal: {0}"},
    {"NTSidPrimaryGroupPrincipal: name", "NTSidPrimaryGroupPrincipal: {0}"},
    {"NTSidUserPrincipal: name", "NTSidUserPrincipal: {0}"},
    {"NTUserPrincipal: name", "NTUserPrincipal: {0}"},

    // UnixPrincipals
    {"UnixNumericGroupPrincipal [Primary Group]: name",
        "UnixNumericGroupPrincipal [prim\u00e4r grupp]: {0}"},
    {"UnixNumericGroupPrincipal [Supplementary Group]: name",
        "UnixNumericGroupPrincipal [till\u00e4ggsgrupp]: {0}"},
    {"UnixNumericUserPrincipal: name", "UnixNumericUserPrincipal: {0}"},
    {"UnixPrincipal: name", "UnixPrincipal: {0}"},

    // com.sun.security.auth.login.ConfigFile
    {"Unable to properly expand config", "Det g\u00e5r inte att utvidga korrekt {0}"},
    {"extra_config (No such file or directory)",
        "{0} (Det finns ingen s\u00e5dan fil eller katalog.)"},
    {"Unable to locate a login configuration",
        "Det g\u00e5r inte att hitta n\u00e5gon inloggningskonfiguration"},
    {"Configuration Error:\n\tInvalid control flag, flag",
        "Konfigurationsfel:\n\tOgiltig kontrollflagga, {0}"},
    {"Configuration Error:\n\tCan not specify multiple entries for appName",
        "Konfigurationsfel:\n\tDet g\u00e5r inte att ange flera poster f\u00f6r {0}"},
    {"Configuration Error:\n\texpected [expect], read [end of file]",
        "Konfigurationsfel:\n\tf\u00f6rv\u00e4ntade [{0}], l\u00e4ste [end of file]"},
    {"Configuration Error:\n\tLine line: expected [expect], found [value]",
        "Konfigurationsfel:\n\tLine {0}: f\u00f6rv\u00e4ntade [{1}], hittade [{2}]"},
    {"Configuration Error:\n\tLine line: expected [expect]",
        "Konfigurationsfel:\n\tLine {0}: f\u00f6rv\u00e4ntade [{1}]"},
    {"Configuration Error:\n\tLine line: system property [value] expanded to empty value",
        "Konfigurationsfel:\n\tLine {0}: systemegenskapen [{1}] utvidgad till tomt v\u00e4rde"},

    // com.sun.security.auth.module.JndiLoginModule
    {"username: ","anv\u00e4ndarnamn: "},
    {"password: ","l\u00f6senord: "},

    // com.sun.security.auth.module.KeyStoreLoginModule
    {"Please enter keystore information","Ange keystore-information"},
    {"Keystore alias: ","Keystore-alias: "},
    {"Keystore password: ","Keystore-l\u00f6senord: "},
    {"Private key password (optional): ",
        "L\u00f6senord f\u00f6r personlig nyckel (valfritt): "},

    // com.sun.security.auth.module.Krb5LoginModule
    {"Kerberos username [[defUsername]]: ",
        "Kerberos-anv\u00e4ndarnamn [{0}]: "},
    {"Kerberos password for [username]: ",
            "Kerberos-l\u00f6senord f\u00f6r {0}: "},

    /***    EVERYTHING BELOW IS DEPRECATED  ***/

    // com.sun.security.auth.PolicyFile
    {": error parsing ", ": analysfel "},
    {": ", ": "},
    {": error adding Permission ", ": fel vid till\u00e4gg av beh\u00f6righet "},
    {" ", " "},
    {": error adding Entry ", ": fel vid till\u00e4gg av post "},
    {"(", "("},
    {")", ")"},
    {"attempt to add a Permission to a readonly PermissionCollection",
        "f\u00f6rs\u00f6k att l\u00e4gga till beh\u00f6righet till skrivskyddad PermissionCollection"},

    // com.sun.security.auth.PolicyParser
    {"expected keystore type", "f\u00f6rv\u00e4ntad keystore-typ"},
    {"can not specify Principal with a ",
        "det g\u00e5r inte att specificera n\u00e5gon principal med "},
    {"wildcard class without a wildcard name",
        "jokertecken f\u00f6r klass men inte f\u00f6r namn"},
    {"expected codeBase or SignedBy", "f\u00f6rv\u00e4ntade codeBase eller SignedBy"},
    {"only Principal-based grant entries permitted",
        "enbart Principal-baserade poster till\u00e5tna"},
    {"expected permission entry", "f\u00f6rv\u00e4ntade beh\u00f6righetspost"},
    {"number ", "antal "},
    {"expected ", "f\u00f6rv\u00e4ntade "},
    {", read end of file", ", l\u00e4ste filslut"},
    {"expected ';', read end of file", "f\u00f6rv\u00e4ntade ';', l\u00e4ste filslut"},
    {"line ", "rad "},
    {": expected '", ": f\u00f6rv\u00e4ntade '"},
    {"', found '", "', hittade '"},
    {"'", "'"},

    // SolarisPrincipals
    {"SolarisNumericGroupPrincipal [Primary Group]: ",
        "SolarisNumericGroupPrincipal [prim\u00e4r grupp]: "},
    {"SolarisNumericGroupPrincipal [Supplementary Group]: ",
        "SolarisNumericGroupPrincipal [till\u00e4ggsgrupp]: "},
    {"SolarisNumericUserPrincipal: ",
        "SolarisNumericUserPrincipal: "},
    {"SolarisPrincipal: ", "SolarisPrincipal: "},
        {"provided null name", "gav null-namn"}

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

