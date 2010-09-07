/*
 * @(#)SSLPermission.java	1.9 04/02/16
 *
 * Copyright (c) 2004 Sun Microsystems, Inc. All Rights Reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.net.ssl;

import java.security.*;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.StringTokenizer;
import java.security.Permissions;
import java.lang.SecurityManager;

/** 
 * This class is for various network permissions.
 * An SSLPermission contains a name (also referred to as a "target name") but
 * no actions list; you either have the named permission
 * or you don't.
 * <P>
 * The target name is the name of the network permission (see below). The naming
 * convention follows the  hierarchical property naming convention.
 * Also, an asterisk
 * may appear at the end of the name, following a ".", or by itself, to
 * signify a wildcard match. For example: "foo.*" or "*" is valid,
 * "*foo" or "a*b" is not valid.
 * <P>
 * The following table lists all the possible SSLPermission target names,
 * and for each provides a description of what the permission allows
 * and a discussion of the risks of granting code the permission.
 * <P>
 *
 * <table border=1 cellpadding=5
 *  summary="permission name, what it allows, and associated risks">
 * <tr>
 * <th>Permission Target Name</th>
 * <th>What the Permission Allows</th>
 * <th>Risks of Allowing this Permission</th>
 * </tr>
 *
 * <tr>
 *   <td>setHostnameVerifier</td>
 *   <td>The ability to set a callback which can decide whether to
 * allow a mismatch between the host being connected to by
 * an HttpsURLConnection and the common name field in
 * server certificate.
 *  </td>
 *   <td>Malicious
 * code can set a verifier that monitors host names visited by
 * HttpsURLConnection requests or that allows server certificates
 * with invalid common names.
 * </td>
 * </tr>
 *
 * <tr>
 *   <td>getSSLSessionContext</td>
 *   <td>The ability to get the SSLSessionContext of an SSLSession.
 * </td>
 *   <td>Malicious code may monitor sessions which have been established
 * with SSL peers or might invalidate sessions to slow down performance.
 * </td>
 * </tr>
 *
 * </table>
 *
 * @see java.security.BasicPermission
 * @see java.security.Permission
 * @see java.security.Permissions
 * @see java.security.PermissionCollection
 * @see java.lang.SecurityManager
 *
 * @since 1.4
 * @version 1.9
 * @author Marianne Mueller
 * @author Roland Schemers
 */
public final class SSLPermission extends BasicPermission
{

    /** 
     * Creates a new SSLPermission with the specified name.
     * The name is the symbolic name of the SSLPermission, such as
     * "setDefaultAuthenticator", etc. An asterisk
     * may appear at the end of the name, following a ".", or by itself, to
     * signify a wildcard match.
     *
     * @param name the name of the SSLPermission.
     */
    public SSLPermission(String name) { }

    /** 
     * Creates a new SSLPermission object with the specified name.
     * The name is the symbolic name of the SSLPermission, and the
     * actions String is currently unused and should be null. 
     *
     * @param name the name of the SSLPermission.
     * @param actions should be null.
     */
    public SSLPermission(String name, String actions) { }
}
