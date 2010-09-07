/*
 * @(#)WinOperaSupport.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.io.File;
import java.io.IOException;
import java.lang.Exception;
import java.lang.Integer;
import java.lang.NumberFormatException;
import java.lang.Object;
import java.lang.String;
import java.lang.StringBuffer;
import java.lang.System;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.WinRegistry;


/**
 This class provides Java Web Start support for the Opera browser on Windows.

 @author Michael W. Romanchuk
 @version 0.1, 05/28/03
 @since 1.5
 */
public class WinOperaSupport extends OperaSupport
{
    /**
     Checks if Opera is installed.
     <br />
     This checks the Windows Registry for the presence of a specific key
     installed by all known versions of the Opera web browser.

     @return <code>true</code> if Opera is installed; <code>false</code>
             otherwise.
     */
    public boolean isInstalled()
    {
        return (getInstallPath().length() != 0);
    }

    /**
     Enables Opera to handle the <code>application/x-java-jnlp-file</code> mime
     type and <code>jnlp</code> file extension.

     @param jaws      the location of the Java Web Start executable.
     @param override  if <code>true</code>, then the value of <code>jaws</code>
                      is used even if there is an existing application assigned
                      to the JNLP mime type.
     */
    public void enableJnlp(File jaws, boolean override)
    {
        String operaPath = getInstallPath();

        if (operaPath.length() > 0)
        {
            try
            {
                File opera = new File(operaPath);
                File user  = enableSystemJnlp(opera, jaws);

                if (user == null)
                {
                    // If this is Opera 6+ then there is a single preference
                    // file for all users named "Opera6.ini" in the opera
                    // directory
                    user = new File(opera, OPERA_6_PREFERENCES);
                    if (user.exists() == false)
                    {
                        // If this is Opera 5+ then there is a single preference
                        // file for all users named "Opera.ini" in the opera
                        // directory
                        user = new File(opera, OPERA_PREFERENCES);
                        if (user.exists() == false)
                        {
                            // If this is Opera 3 then there is a single
                            // preference file for all users named "Opera.ini"
                            // in the %System% directory
                            user = new File(Config.getOSHome(), OPERA_PREFERENCES);
                        }
                        // no else required; Opera 5+
                    }
                    // no else required; Opera 6+
                }
                // no else required; Opera 7+ multi-user preferences

                enableJnlp(null, user, jaws, override);
            }
            catch (Exception e)
            {
                Trace.ignoredException(e);
            }
        }
        // no else required; Opera not installed
    }

    /**
     Constructs n <code>WinOperaSupport</code> support object.

     @param useDefault  if <code>true</code> then Opera will be configured to
                        use the default installation of Java Web Start, which
                        may not be the one that is currently being setup by
                        the application manager.
     */
    public WinOperaSupport(boolean useDefault)
    {
        super(useDefault);
    }

    /**
     Enables Java Web Start at the System level.
     <br />
     If this is Opera 7+, then there will be an overall preference file named
     'OperaDef6.ini' in the install directory.  When this file is present, check
     the value of the 'Multi User' key in the 'System' section.  If this enabled
     Opera will use Windows profiles to store individual user settings.  In
     which case JaWS needs to be enabled in both the overall preferences and the
     current user preferences.

     @param installDir  the Opera installation directory.
     @param jaws        the location of the Java Web Start executable.

     @return The location of the current user's personal preferences if this is
             enabled.
     */
    private File enableSystemJnlp(File installDir, File jaws) throws IOException
    {
        OperaPreferences prefs  = null;
        File             system = null;
        File             result = null;

        // check for a system level preference file and multi-user support
        system = new File(installDir, SYSTEM_PREFERENCES);
        prefs  = getPreferences(system);

        if (prefs != null)
        {
            boolean multiUser = true;

            // First enable JaWS in the system template.  This is usefull
            // even if multi-user support isn't enabled.
            enableJnlp(prefs, system, jaws, true);

            if (prefs.containsKey(MULTI_USER_SECTION, MULTI_USER_KEY))
            {
                String value = prefs.get(MULTI_USER_SECTION, MULTI_USER_KEY).trim();

                // get rid of any embeded comment
                value = value.substring(0, value.indexOf(' '));

                try
                {
                    int enabled = Integer.decode(value).intValue();
                    if (enabled == 0)
                    {
                        multiUser = false;

                        Trace.println("Multi-user support is turned off in the " +
                                      "Opera system preference file (" +
                                      system.getAbsolutePath() + ").",
                                      TraceLevel.BASIC);
                    }
                    // no else required; multi-user enabled by key
                }
                catch (NumberFormatException nfe)
                {
                    multiUser = false;

                    Trace.println("The Opera system preference file (" +
                                  system.getAbsolutePath() + ") has '" +
                                  MULTI_USER_KEY + "=" + value +"' in the " +
                                  MULTI_USER_SECTION + " section, so multi-user " +
                                  "support is not enabled.",
                                  TraceLevel.BASIC);
                }
            }
            // no else required; the Opera behavior is to treat the
            // absence of the key as multi-user enabled

            if (multiUser == true)
            {
                StringBuffer userPath = new StringBuffer(512);

                userPath.append(System.getProperty(USER_HOME))
                        .append(File.separator)
                        .append(USER_DATA_INFIX)
                        .append(File.separator)
                        .append(installDir.getName())
                        .append(File.separator)
                        .append(USER_DATA_POSTFIX)
                        .append(File.separator)
                        .append(OPERA_6_PREFERENCES);

                result = new File(userPath.toString());
            }
            // no else required; not a multi-user installation, so return null
        }
        // no else required; there was no system level preferences file.

        return (result);
    }

    /**
     Get the directory that Opera is installed in, from the Windows Registry.

     @return The directory that Opera is installed in, or the empty string if
             Opera isn't installed on the machine.
     */
    private String getInstallPath()
    {
        String operDir = WinRegistry.getString(WinRegistry.HKEY_LOCAL_MACHINE,
                                               OPERA_SUBKEY, OPERA_PATH);

        return ((operDir != null) ? operDir : "");
    }

    /**
     The subkey in the Windows registry to use when checking if Opera is
     installed.
     */
    private static final String OPERA_SUBKEY = "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Opera.exe";

    /**
     The name of the value in the Windows registry to use when checking if Opera
     is installed.
     */
    private static final String OPERA_PATH = "Path";

    /** The name of the System property that stores the user's home directory. */
    private static final String USER_HOME = "user.home";

    /**
     This string is used when determining where to find the user's preferences
     file when Opera is installed with multi-user support.
     */
    private static final String USER_DATA_INFIX = "Application Data" + File.separator + "Opera";

    /**
     This string is used when determining where to find the user's preferences
     file when Opera is installed with multi-user support.
     */
    private static final String USER_DATA_POSTFIX = "Profile";

    /** The name of the system level preferences file. */
    private static final String SYSTEM_PREFERENCES = "OperaDef6.ini";

    /**
     The name of the section in the system level preferences file where a key
     can be found that indicates whether or not multi-user support is enabled.
     */
    private static final String MULTI_USER_SECTION = "System";

    /**
     The name of the key that indicates whether or not multi-user support is
     enabled.
     */
    private static final String MULTI_USER_KEY = "Multi User";
}
