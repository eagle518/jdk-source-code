/*
 * @(#)LoginUtility.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.jgss;

import javax.security.auth.Subject;
import javax.security.auth.login.LoginContext;
import javax.security.auth.login.LoginException;
import java.security.PrivilegedExceptionAction;
import java.security.AccessController;
import sun.security.action.GetPropertyAction;
import com.sun.security.auth.callback.DialogCallbackHandler;
import com.sun.security.auth.callback.TextCallbackHandler;

/*
 * This utility returns a credential that is not part of the current
 * Subject. It does this by first performing a JAAS login, and then
 * searching for the credential in the Subject that is returned by that
 * login. This Subject might be entirely different from the Subject on the
 * current ACC. In fact, the current ACC might not even have a Subject
 * available.
 *
 * @author Mayank Upadhyay
 * @version 1.6, 12/19/03
 * @since 1.4
 */

public class LoginUtility {

    public static final String GSS_INITIATE_ENTRY = 
	"com.sun.security.jgss.initiate";

    public static final String GSS_ACCEPT_ENTRY = 
	"com.sun.security.jgss.accept";

    public static final String JSSE_CLIENT_ENTRY = 
	"com.sun.net.ssl.client";

    public static final String JSSE_SERVER_ENTRY = 
	"com.sun.net.ssl.server";

    private static final String DEFAULT_HANDLER = 
	"auth.login.defaultCallbackHandler";


    /**
     * Default constructor
     */
    private LoginUtility() {  // Cannot create one of these
    }

    /**
     * Authenticate using the login module from the specified
     * configuration entry.
     *
     * @return the authenticated subject
     */
    public static Subject login(String config) throws LoginException {
	// get the default callback handler
	String defaultHandler = 
		java.security.Security.getProperty(DEFAULT_HANDLER);
	LoginContext lc = null;
        if ((defaultHandler != null) && (defaultHandler.length() != 0)) {
	   // use the default callback handler
	   lc = new LoginContext(config);
	} else {
	   // fallback to TextCallBackHandler
	   lc = new LoginContext(config, new TextCallbackHandler());
	}
	lc.login();
	return lc.getSubject();
    }

    /**
     * Determines if the application doesn't mind if the mechanism obtains
     * the required credentials from outside of the current Subject. Our
     * Kerberos v5 mechanism would do a JAAS login on behalf of the
     * application if this were the case.
     *
     * The application indicates this by explicitly setting the system
     * property javax.security.auth.useSubjectCredsOnly to false.
     */
    public static boolean useSubjectCredsOnly() {
	/*
	 * Don't use GetBooleanAction because the default value in the JRE
	 * (when this is unset) has to treated as true.
	 */
	String propValue = 
	    (String) AccessController.doPrivileged(
		new GetPropertyAction("javax.security.auth.useSubjectCredsOnly",
					"true"));
	/*
	 * This property has to be explicitly set to "false". Invalid
	 * values should be ignored and the default "true" assumed.
	 */
	return (!propValue.equalsIgnoreCase("false"));
    }
}
